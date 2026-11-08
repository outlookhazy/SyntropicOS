/**
 * @file syn_dnssd.c
 * @brief DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762) implementation.
 */

#include "syn_dnssd.h"

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"

#include <stdio.h>
#include <string.h>

/** @brief Multicast DNS IPv4 address */
#define DNSSD_MCAST_ADDR "224.0.0.251"
/** @brief Default TTL for PTR records (75 minutes) */
#define DNSSD_TTL_PTR 4500U
/** @brief Default TTL for SRV/A records (2 minutes) */
#define DNSSD_TTL_SRV 120U

/**
 * @brief Append raw bytes into a bounded output buffer.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @param src Source bytes.
 * @param len Byte count to append.
 * @return true if bytes fit and were written, false on truncation.
 */
static bool dnssd_put_bytes(uint8_t *buf, size_t cap, size_t *pos, const void *src, size_t len)
{
    if (len > cap - *pos) {
        return false;
    }
    if (len > 0U) {
        memcpy(buf + *pos, src, len);
        *pos += len;
    }
    return true;
}

/**
 * @brief Append a single byte into a bounded output buffer.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @param val Byte value.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_u8(uint8_t *buf, size_t cap, size_t *pos, uint8_t val)
{
    return dnssd_put_bytes(buf, cap, pos, &val, 1U);
}

/**
 * @brief Append a 16-bit big-endian integer into a bounded output buffer.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @param val 16-bit value.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_u16(uint8_t *buf, size_t cap, size_t *pos, uint16_t val)
{
    uint8_t b[2] = {(uint8_t)(val >> 8U), (uint8_t)(val & 0xFFU)};
    return dnssd_put_bytes(buf, cap, pos, b, 2U);
}

/**
 * @brief Append a 32-bit big-endian integer into a bounded output buffer.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @param val 32-bit value.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_u32(uint8_t *buf, size_t cap, size_t *pos, uint32_t val)
{
    uint8_t b[4] = {(uint8_t)(val >> 24U), (uint8_t)(val >> 16U), (uint8_t)(val >> 8U),
                    (uint8_t)(val & 0xFFU)};
    return dnssd_put_bytes(buf, cap, pos, b, 4U);
}

/**
 * @brief Write DNS label (clamped to 63 octets) with length prefix into bounded buffer.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @param str Label string.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_label(uint8_t *buf, size_t cap, size_t *pos, const char *str)
{
    size_t len = strlen(str);
    if (len > 63U) {
        len = 63U;
    }
    return dnssd_put_u8(buf, cap, pos, (uint8_t)len) && dnssd_put_bytes(buf, cap, pos, str, len);
}

/**
 * @brief Write DNS service type domain name (`_svc._proto.local.`) into bounded buffer.
 * @param buf          Destination buffer.
 * @param cap          Buffer capacity.
 * @param pos          [in,out] Write cursor.
 * @param service_type Service type string.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_service_type_name(uint8_t *buf, size_t cap, size_t *pos,
                                        const char *service_type)
{
    char temp[64];
    strncpy(temp, service_type, sizeof(temp) - 1U);
    temp[sizeof(temp) - 1U] = '\0';

    char *dot = strchr(temp, '.');
    bool ok = true;
    if (dot != NULL) {
        *dot = '\0';
        ok = dnssd_put_label(buf, cap, pos, temp) && dnssd_put_label(buf, cap, pos, dot + 1);
    } else {
        ok = dnssd_put_label(buf, cap, pos, temp);
    }
    return ok && dnssd_put_label(buf, cap, pos, "local") && dnssd_put_u8(buf, cap, pos, 0x00U);
}

/**
 * @brief Write DNS instance FQDN into bounded buffer.
 * @param buf          Destination buffer.
 * @param cap          Buffer capacity.
 * @param pos          [in,out] Write cursor.
 * @param instance     Instance name.
 * @param service_type Service type.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_instance_fqdn(uint8_t *buf, size_t cap, size_t *pos, const char *instance,
                                    const char *service_type)
{
    return dnssd_put_label(buf, cap, pos, instance) &&
           dnssd_put_service_type_name(buf, cap, pos, service_type);
}

/**
 * @brief Write DNS hostname FQDN (`host.local.`) into bounded buffer.
 * @param buf      Destination buffer.
 * @param cap      Buffer capacity.
 * @param pos      [in,out] Write cursor.
 * @param hostname Hostname string.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_hostname_fqdn(uint8_t *buf, size_t cap, size_t *pos, const char *hostname)
{
    return dnssd_put_label(buf, cap, pos, hostname) && dnssd_put_label(buf, cap, pos, "local") &&
           dnssd_put_u8(buf, cap, pos, 0x00U);
}

/**
 * @brief Write fixed 10-byte DNS RR metadata and record RDLENGTH offset.
 * @param buf       Destination buffer.
 * @param cap       Buffer capacity.
 * @param pos       [in,out] Write cursor.
 * @param type      RR type code.
 * @param cls       RR class + cache-flush bits.
 * @param ttl       TTL in seconds.
 * @param rdlen_pos [out] Offset of 16-bit RDLENGTH field.
 * @return true on success, false on truncation.
 */
static bool dnssd_put_rr_meta(uint8_t *buf, size_t cap, size_t *pos, uint16_t type, uint16_t cls,
                              uint32_t ttl, size_t *rdlen_pos)
{
    if (!dnssd_put_u16(buf, cap, pos, type) || !dnssd_put_u16(buf, cap, pos, cls) ||
        !dnssd_put_u32(buf, cap, pos, ttl)) {
        return false;
    }
    *rdlen_pos = *pos;
    return dnssd_put_u16(buf, cap, pos, 0U);
}

/**
 * @brief Backpatch a 16-bit big-endian RDLENGTH field after writing RDATA.
 * @param buf       Packet buffer.
 * @param rdlen_pos Offset of RDLENGTH field.
 * @param end_pos   Offset immediately following RDATA.
 */
static void dnssd_patch_rdlen(uint8_t *buf, size_t rdlen_pos, size_t end_pos)
{
    uint16_t rdlen = (uint16_t)(end_pos - (rdlen_pos + 2U));
    buf[rdlen_pos] = (uint8_t)(rdlen >> 8U);
    buf[rdlen_pos + 1U] = (uint8_t)(rdlen & 0xFFU);
}

SYN_Status syn_dnssd_init(SYN_DnsSd *sd)
{
    if (sd == NULL) {
        return SYN_ERROR;
    }
    memset(sd, 0, sizeof(*sd));

    sd->sock = syn_port_udp_open(SYN_DNSSD_PORT);
    if (sd->sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    if (syn_port_udp_join_multicast(sd->sock, DNSSD_MCAST_ADDR) != SYN_OK) {
        syn_port_sock_close(sd->sock);
        sd->sock = SYN_SOCKET_INVALID;
        return SYN_ERROR;
    }

    return SYN_OK;
}

/**
 * @brief Validate a DNS-SD service descriptor including TXT array bounds and non-NULL entries.
 * @param svc Service configuration.
 * @return true if valid.
 */
static bool dnssd_valid_service(const SYN_DnsSd_Service *svc)
{
    if (svc == NULL || svc->instance_name == NULL || svc->service_type == NULL ||
        svc->hostname == NULL || svc->port == 0U || svc->txt_count > SYN_DNSSD_MAX_TXT_RECORDS) {
        return false;
    }
    for (size_t i = 0U; i < svc->txt_count; i++) {
        if (svc->txt_records[i] == NULL) {
            return false;
        }
    }
    return true;
}

SYN_Status syn_dnssd_register(SYN_DnsSd *sd, const SYN_DnsSd_Service *svc)
{
    if (sd == NULL || !dnssd_valid_service(svc) || sd->service_count >= SYN_DNSSD_MAX_SERVICES) {
        return SYN_ERROR;
    }

    sd->services[sd->service_count++] = *svc;
    return SYN_OK;
}

/**
 * @brief Append one DNS label (with leading dot if non-empty) into a bounded C string.
 * @param out     Output string buffer.
 * @param max_out Output buffer capacity.
 * @param out_len [in,out] Current string length.
 * @param label   Label bytes.
 * @param len     Label byte length.
 */
static void dnssd_append_label(char *out, size_t max_out, size_t *out_len, const uint8_t *label,
                               size_t len)
{
    size_t o = *out_len;
    if (o > 0U && o + 1U < max_out) {
        out[o++] = '.';
    }
    size_t avail = max_out - 1U - o;
    size_t copy_len = (len < avail) ? len : avail;
    memcpy(&out[o], label, copy_len);
    o += copy_len;
    out[o] = '\0';
    *out_len = o;
}

/**
 * @brief Extract a DNS domain name (following up to 8 0xC0 compression pointers) into a C string.
 * @param buf        Packet start buffer.
 * @param msg_len    Total packet byte length.
 * @param stream_end End offset of inline section (`<= msg_len`).
 * @param pos        [in,out] Inline cursor; updated past inline name on success.
 * @param out        [out] Output string buffer (capacity >= 2).
 * @param max_out    Capacity of out buffer.
 * @return true if name was well-formed within bounds, false otherwise.
 */
static bool skip_or_read_name(const uint8_t *buf, size_t msg_len, size_t stream_end, size_t *pos,
                              char *out, size_t max_out)
{
    size_t p = *pos;
    size_t next_pos = 0U;
    size_t out_len = 0U;
    uint8_t hops = 0U;
    bool jumped = false;
    out[0] = '\0';

    while (true) {
        size_t limit = jumped ? msg_len : stream_end;
        if (p >= limit) {
            return false;
        }
        uint8_t len = buf[p];
        if (len == 0U) {
            *pos = jumped ? next_pos : (p + 1U);
            return true;
        }
        if ((len & 0xC0U) == 0xC0U) {
            if (limit - p < 2U || hops >= 8U) {
                return false;
            }
            uint16_t target = (uint16_t)(((uint16_t)(len & 0x3FU) << 8U) | buf[p + 1U]);
            if ((size_t)target >= msg_len) {
                return false;
            }
            if (!jumped) {
                next_pos = p + 2U;
                jumped = true;
            }
            p = (size_t)target;
            hops++;
            continue;
        }
        if ((len & 0xC0U) != 0U || limit - p - 1U < (size_t)len) {
            return false;
        }
        dnssd_append_label(out, max_out, &out_len, &buf[p + 1U], (size_t)len);
        p += 1U + (size_t)len;
    }
}

/**
 * @brief Fold an ASCII uppercase letter to lowercase.
 * @param c Input byte.
 * @return Lowercase ASCII byte.
 */
static uint8_t dnssd_ascii_lower(uint8_t c)
{
    if (c >= (uint8_t)'A' && c <= (uint8_t)'Z') {
        return (uint8_t)(c + 32U);
    }
    return c;
}

/**
 * @brief Compare two NUL-terminated strings for ASCII case-insensitive equality.
 * @param a First string.
 * @param b Second string.
 * @return true if strings match ignoring ASCII case.
 */
static bool dnssd_streq_ci(const char *a, const char *b)
{
    size_t i = 0U;
    while (a[i] != '\0' && b[i] != '\0') {
        if (dnssd_ascii_lower((uint8_t)a[i]) != dnssd_ascii_lower((uint8_t)b[i])) {
            return false;
        }
        i++;
    }
    return (a[i] == b[i]);
}

/**
 * @brief Match DNS question name against registered service type (case-insensitive).
 * @param[in] buf Input buffer containing DNS question.
 * @param[in] buf_len Total buffer length.
 * @param[in,out] offset Offset within buffer.
 * @param[in] service_type Expected service type.
 * @return True if question matches service type, false otherwise.
 */
static bool match_qname_service(const uint8_t *buf, size_t buf_len, size_t *offset,
                                const char *service_type)
{
    char q_name[128];
    if (!skip_or_read_name(buf, buf_len, buf_len, offset, q_name, sizeof(q_name))) {
        return false;
    }

    char expected[128];
    (void)snprintf(expected, sizeof(expected), "%s.local", service_type);

    return dnssd_streq_ci(q_name, expected);
}

/**
 * @brief Pack PTR answer record into bounded DNS-SD response buffer.
 * @param svc Service definition.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @return true on success, false on truncation.
 */
static bool dnssd_pack_ptr_rr(const SYN_DnsSd_Service *svc, uint8_t *buf, size_t cap, size_t *pos)
{
    size_t rdlen_pos = 0U;
    if (!dnssd_put_service_type_name(buf, cap, pos, svc->service_type) ||
        !dnssd_put_rr_meta(buf, cap, pos, 0x000CU, 0x0001U, DNSSD_TTL_PTR, &rdlen_pos) ||
        !dnssd_put_instance_fqdn(buf, cap, pos, svc->instance_name, svc->service_type)) {
        return false;
    }
    dnssd_patch_rdlen(buf, rdlen_pos, *pos);
    return true;
}

/**
 * @brief Pack SRV additional record into bounded DNS-SD response buffer.
 * @param svc Service definition.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @return true on success, false on truncation.
 */
static bool dnssd_pack_srv_rr(const SYN_DnsSd_Service *svc, uint8_t *buf, size_t cap, size_t *pos)
{
    size_t rdlen_pos = 0U;
    if (!dnssd_put_instance_fqdn(buf, cap, pos, svc->instance_name, svc->service_type) ||
        !dnssd_put_rr_meta(buf, cap, pos, 0x0021U, 0x8001U, DNSSD_TTL_SRV, &rdlen_pos) ||
        !dnssd_put_u16(buf, cap, pos, 0x0000U) || !dnssd_put_u16(buf, cap, pos, 0x0000U) ||
        !dnssd_put_u16(buf, cap, pos, svc->port) ||
        !dnssd_put_hostname_fqdn(buf, cap, pos, svc->hostname)) {
        return false;
    }
    dnssd_patch_rdlen(buf, rdlen_pos, *pos);
    return true;
}

/**
 * @brief Pack TXT additional record into bounded DNS-SD response buffer.
 * @param svc Service definition.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @return true on success, false on truncation.
 */
static bool dnssd_pack_txt_rr(const SYN_DnsSd_Service *svc, uint8_t *buf, size_t cap, size_t *pos)
{
    size_t rdlen_pos = 0U;
    if (!dnssd_put_instance_fqdn(buf, cap, pos, svc->instance_name, svc->service_type) ||
        !dnssd_put_rr_meta(buf, cap, pos, 0x0010U, 0x8001U, DNSSD_TTL_PTR, &rdlen_pos)) {
        return false;
    }
    if (svc->txt_count > 0U) {
        for (size_t t = 0U; t < svc->txt_count; t++) {
            if (!dnssd_put_label(buf, cap, pos, svc->txt_records[t])) {
                return false;
            }
        }
    } else if (!dnssd_put_u8(buf, cap, pos, 0x00U)) {
        return false;
    }
    dnssd_patch_rdlen(buf, rdlen_pos, *pos);
    return true;
}

/**
 * @brief Pack IPv4 A additional record into bounded DNS-SD response buffer.
 * @param svc Service definition.
 * @param buf Destination buffer.
 * @param cap Buffer capacity.
 * @param pos [in,out] Write cursor.
 * @return true on success, false on truncation.
 */
static bool dnssd_pack_a_rr(const SYN_DnsSd_Service *svc, uint8_t *buf, size_t cap, size_t *pos)
{
    size_t rdlen_pos = 0U;
    if (!dnssd_put_hostname_fqdn(buf, cap, pos, svc->hostname) ||
        !dnssd_put_rr_meta(buf, cap, pos, 0x0001U, 0x8001U, DNSSD_TTL_SRV, &rdlen_pos) ||
        !dnssd_put_bytes(buf, cap, pos, svc->ip, 4U)) {
        return false;
    }
    dnssd_patch_rdlen(buf, rdlen_pos, *pos);
    return true;
}

/**
 * @brief Pack DNS-SD response packet for a service (PTR, SRV, TXT, A records).
 * @param[in] svc Service definition.
 * @param[out] resp_buf Buffer to receive response packet.
 * @param[in] max_resp_len Capacity of resp_buf.
 * @param[out] resp_len Bytes written.
 * @return SYN_OK on success, SYN_ERROR on buffer overflow.
 */
static SYN_Status pack_service_response(const SYN_DnsSd_Service *svc, uint8_t *resp_buf,
                                        size_t max_resp_len, size_t *resp_len)
{
    *resp_len = 0U;
    if (!dnssd_valid_service(svc)) {
        return SYN_ERROR;
    }

    static const uint8_t hdr[12] = {0x00U, 0x00U, 0x84U, 0x00U, 0x00U, 0x00U,
                                    0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x03U};
    size_t rpos = 0U;
    if (!dnssd_put_bytes(resp_buf, max_resp_len, &rpos, hdr, sizeof(hdr)) ||
        !dnssd_pack_ptr_rr(svc, resp_buf, max_resp_len, &rpos) ||
        !dnssd_pack_srv_rr(svc, resp_buf, max_resp_len, &rpos) ||
        !dnssd_pack_txt_rr(svc, resp_buf, max_resp_len, &rpos) ||
        !dnssd_pack_a_rr(svc, resp_buf, max_resp_len, &rpos)) {
        return SYN_ERROR;
    }

    *resp_len = rpos;
    return SYN_OK;
}

/**
 * @brief Skip a DNS question entry (QNAME with optional 0xC0 pointer + QTYPE + QCLASS).
 * @param buf     DNS query buffer.
 * @param buf_len Total query byte length.
 * @param pos     [in,out] Read cursor.
 * @return true if question entry was well-formed within bounds, false otherwise.
 */
static bool dnssd_skip_question(const uint8_t *buf, size_t buf_len, size_t *pos)
{
    size_t p = *pos;
    bool terminated = false;

    while (p < buf_len) {
        uint8_t len = buf[p];
        if (len == 0U) {
            p++;
            terminated = true;
            break;
        }
        if ((len & 0xC0U) == 0xC0U) {
            if (buf_len - p < 2U) {
                return false;
            }
            p += 2U;
            terminated = true;
            break;
        }
        if (len > 63U || buf_len - p - 1U < (size_t)len) {
            return false;
        }
        p += 1U + (size_t)len;
    }

    if (!terminated || buf_len - p < 4U) {
        return false;
    }
    *pos = p + 4U;
    return true;
}

/**
 * @brief Validate QTYPE and QCLASS for a DNS-SD service query.
 * @param qtype  16-bit DNS QTYPE.
 * @param qclass 16-bit DNS QCLASS (may include top unicast-response QU bit).
 * @return true if QTYPE is PTR/SRV/TXT/ANY and class is IN/ANY.
 */
static bool dnssd_qtype_qclass_valid(uint16_t qtype, uint16_t qclass)
{
    uint16_t cls = (uint16_t)(qclass & 0x7FFFU);
    bool type_ok = (qtype == 12U) || (qtype == 33U) || (qtype == 16U) || (qtype == 255U);
    bool cls_ok = (cls == 1U) || (cls == 255U);
    return type_ok && cls_ok;
}

/**
 * @brief Match a DNS question at name_start against any registered service.
 * @param sd         DNS-SD instance.
 * @param query_buf  Query packet buffer.
 * @param query_len  Query packet length.
 * @param name_start Offset of QNAME.
 * @param next_pos   [out] Offset after QNAME + QTYPE + QCLASS when matched.
 * @return Pointer to matching service, or NULL.
 */
static const SYN_DnsSd_Service *dnssd_match_registered(const SYN_DnsSd *sd,
                                                       const uint8_t *query_buf, size_t query_len,
                                                       size_t name_start, size_t *next_pos)
{
    for (size_t s = 0U; s < sd->service_count; s++) {
        size_t temp_pos = name_start;
        if (match_qname_service(query_buf, query_len, &temp_pos, sd->services[s].service_type) &&
            query_len - temp_pos >= 4U) {
            uint16_t qtype = ((uint16_t)query_buf[temp_pos] << 8U) | query_buf[temp_pos + 1U];
            uint16_t qclass = ((uint16_t)query_buf[temp_pos + 2U] << 8U) | query_buf[temp_pos + 3U];
            if (dnssd_qtype_qclass_valid(qtype, qclass)) {
                *next_pos = temp_pos + 4U;
                return &sd->services[s];
            }
        }
    }
    return NULL;
}

SYN_Status syn_dnssd_process_query(const SYN_DnsSd *sd, const uint8_t *query_buf, size_t query_len,
                                   uint8_t *resp_buf, size_t max_resp_len, size_t *resp_len)
{
    if (sd == NULL || query_buf == NULL || resp_buf == NULL || resp_len == NULL) {
        return SYN_ERROR;
    }
    *resp_len = 0U;

    if (query_len < 12U) {
        return SYN_ERROR;
    }

    uint16_t flags = ((uint16_t)query_buf[2] << 8U) | query_buf[3];
    uint16_t qdcount = ((uint16_t)query_buf[4] << 8U) | query_buf[5];
    if ((flags & 0x8000U) != 0U || qdcount == 0U) {
        return SYN_ERROR;
    }

    size_t pos = 12U;
    const SYN_DnsSd_Service *matched_svc = NULL;

    for (uint16_t q = 0U; q < qdcount; q++) {
        matched_svc = dnssd_match_registered(sd, query_buf, query_len, pos, &pos);
        if (matched_svc != NULL) {
            break;
        }
        if (!dnssd_skip_question(query_buf, query_len, &pos)) {
            break;
        }
    }

    if (matched_svc == NULL) {
        return SYN_NOT_FOUND;
    }

    return pack_service_response(matched_svc, resp_buf, max_resp_len, resp_len);
}

SYN_Status syn_dnssd_announce(const SYN_DnsSd *sd, size_t service_index, uint8_t *resp_buf,
                              size_t max_resp_len, size_t *resp_len)
{
    if (sd == NULL || resp_buf == NULL || resp_len == NULL || service_index >= sd->service_count) {
        return SYN_ERROR;
    }

    const SYN_DnsSd_Service *svc = &sd->services[service_index];
    SYN_Status st = pack_service_response(svc, resp_buf, max_resp_len, resp_len);
    if (st == SYN_OK && sd->sock != SYN_SOCKET_INVALID) {
        SYN_SockAddr mcast_dest = {.ip = {224U, 0U, 0U, 251U}, .port = SYN_DNSSD_PORT};
        if (syn_port_udp_sendto(sd->sock, resp_buf, *resp_len, &mcast_dest) != (int)*resp_len) {
            return SYN_ERROR;
        }
    }
    return st;
}

SYN_Status syn_dnssd_build_query(const char *service_type, uint8_t *query_buf, size_t max_len,
                                 size_t *query_len)
{
    if (service_type == NULL || service_type[0] == '\0' || query_buf == NULL || query_len == NULL) {
        return SYN_ERROR;
    }
    *query_len = 0U;

    static const uint8_t qhdr[12] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U,
                                     0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
    size_t pos = 0U;
    if (!dnssd_put_bytes(query_buf, max_len, &pos, qhdr, sizeof(qhdr)) ||
        !dnssd_put_service_type_name(query_buf, max_len, &pos, service_type) ||
        !dnssd_put_u16(query_buf, max_len, &pos, 0x000CU) ||
        !dnssd_put_u16(query_buf, max_len, &pos, 0x0001U)) {
        return SYN_ERROR;
    }

    *query_len = pos;
    return SYN_OK;
}

/**
 * @brief Read the first DNS label (following up to 8 compression pointers) as raw bytes.
 * @param buf        Packet buffer.
 * @param msg_len    Total packet length.
 * @param stream_end End offset of inline RDATA (`<= msg_len`).
 * @param start_pos  Starting offset of the domain name.
 * @param out_label  [out] Copy of first label (NUL-terminated, max 63 chars).
 * @param rest_pos   [out] Offset immediately after first label.
 * @param rest_limit [out] Stream limit (`msg_len` if jumped, else `stream_end`).
 * @return true if a non-empty first label was extracted within bounds, false otherwise.
 */
static bool dnssd_read_first_label(const uint8_t *buf, size_t msg_len, size_t stream_end,
                                   size_t start_pos, char out_label[64], size_t *rest_pos,
                                   size_t *rest_limit)
{
    size_t p = start_pos;
    uint8_t hops = 0U;
    bool jumped = false;

    while (true) {
        size_t limit = jumped ? msg_len : stream_end;
        if (p >= limit) {
            return false;
        }
        uint8_t len = buf[p];
        if ((len & 0xC0U) == 0xC0U) {
            if (limit - p < 2U || hops >= 8U) {
                return false;
            }
            uint16_t target = (uint16_t)(((uint16_t)(len & 0x3FU) << 8U) | buf[p + 1U]);
            if ((size_t)target >= msg_len) {
                return false;
            }
            jumped = true;
            p = (size_t)target;
            hops++;
            continue;
        }
        if (len == 0U || (len & 0xC0U) != 0U || limit - p - 1U < (size_t)len) {
            return false;
        }
        memcpy(out_label, &buf[p + 1U], (size_t)len);
        out_label[len] = '\0';
        *rest_pos = p + 1U + (size_t)len;
        *rest_limit = limit;
        return true;
    }
}

/**
 * @brief Parse PTR RDATA into instance_name and service_type fields.
 * @param resp_buf    DNS response packet buffer.
 * @param resp_len    Total packet length.
 * @param rdata_start Offset of PTR RDATA.
 * @param rdlength    RDATA byte length.
 * @param out_disc    [out] Discovered service output struct.
 */
static void dnssd_parse_ptr_rdata(const uint8_t *resp_buf, size_t resp_len, size_t rdata_start,
                                  uint16_t rdlength, SYN_DnsSd_Discovered *out_disc)
{
    char inst_buf[64];
    char svc_fqdn[128];
    size_t rest_pos = 0U;
    size_t rest_limit = 0U;
    size_t rdata_end = rdata_start + (size_t)rdlength;

    if (!dnssd_read_first_label(resp_buf, resp_len, rdata_end, rdata_start, inst_buf, &rest_pos,
                                &rest_limit)) {
        return;
    }
    if (!skip_or_read_name(resp_buf, resp_len, rest_limit, &rest_pos, svc_fqdn, sizeof(svc_fqdn)) ||
        svc_fqdn[0] == '\0') {
        return;
    }
    memcpy(out_disc->instance_name, inst_buf, strlen(inst_buf) + 1U);
    strncpy(out_disc->service_type, svc_fqdn, sizeof(out_disc->service_type) - 1U);
    out_disc->service_type[sizeof(out_disc->service_type) - 1U] = '\0';
    char *loc = strstr(out_disc->service_type, ".local");
    if (loc != NULL) {
        *loc = '\0';
    }
}

/**
 * @brief Parse SRV RDATA into port and hostname fields.
 * @param resp_buf    DNS response packet buffer.
 * @param resp_len    Total packet length.
 * @param rdata_start Offset of SRV RDATA.
 * @param rdlength    RDATA byte length.
 * @param out_disc    [out] Discovered service output struct.
 */
static void dnssd_parse_srv_rdata(const uint8_t *resp_buf, size_t resp_len, size_t rdata_start,
                                  uint16_t rdlength, SYN_DnsSd_Discovered *out_disc)
{
    size_t p_host = rdata_start + 6U;
    size_t rdata_end = rdata_start + (size_t)rdlength;
    char host_fqdn[128];
    if (!skip_or_read_name(resp_buf, resp_len, rdata_end, &p_host, host_fqdn, sizeof(host_fqdn))) {
        return;
    }
    out_disc->port = ((uint16_t)resp_buf[rdata_start + 4U] << 8U) | resp_buf[rdata_start + 5U];
    char *first_dot = strchr(host_fqdn, '.');
    if (first_dot != NULL) {
        *first_dot = '\0';
    }
    strncpy(out_disc->hostname, host_fqdn, sizeof(out_disc->hostname) - 1U);
}

/**
 * @brief Dispatch a single DNS resource record into a discovered service descriptor.
 * @param resp_buf    DNS response packet buffer.
 * @param resp_len    Total packet length.
 * @param type        RR type code.
 * @param rdata_start Offset of RDATA.
 * @param rdlength    RDATA byte length.
 * @param out_disc    [out] Discovered service output struct.
 */
static void dnssd_parse_rr(const uint8_t *resp_buf, size_t resp_len, uint16_t type,
                           size_t rdata_start, uint16_t rdlength, SYN_DnsSd_Discovered *out_disc)
{
    if (type == 0x000CU) {
        dnssd_parse_ptr_rdata(resp_buf, resp_len, rdata_start, rdlength, out_disc);
    } else if (type == 0x0021U && rdlength >= 6U) {
        dnssd_parse_srv_rdata(resp_buf, resp_len, rdata_start, rdlength, out_disc);
    } else if (type == 0x0010U && rdlength > 0U) {
        uint8_t txt_len = resp_buf[rdata_start];
        if (txt_len > 0U && txt_len < sizeof(out_disc->txt) && (size_t)txt_len < rdlength) {
            memcpy(out_disc->txt, &resp_buf[rdata_start + 1U], txt_len);
            out_disc->txt[txt_len] = '\0';
        }
    } else if (type == 0x0001U && rdlength == 4U) {
        memcpy(out_disc->ip, &resp_buf[rdata_start], 4U);
    }
}

SYN_Status syn_dnssd_parse_response(const uint8_t *resp_buf, size_t resp_len,
                                    SYN_DnsSd_Discovered *out_disc)
{
    if (resp_buf == NULL || out_disc == NULL || resp_len < 12U) {
        return SYN_ERROR;
    }
    memset(out_disc, 0, sizeof(*out_disc));

    uint16_t flags = ((uint16_t)resp_buf[2] << 8U) | resp_buf[3];
    uint16_t qdcount = ((uint16_t)resp_buf[4] << 8U) | resp_buf[5];
    uint16_t ancount = ((uint16_t)resp_buf[6] << 8U) | resp_buf[7];
    uint16_t nscount = ((uint16_t)resp_buf[8] << 8U) | resp_buf[9];
    uint16_t arcount = ((uint16_t)resp_buf[10] << 8U) | resp_buf[11];
    uint32_t total_rr = (uint32_t)ancount + (uint32_t)nscount + (uint32_t)arcount;
    if ((flags & 0x8000U) == 0U || total_rr == 0U) {
        return SYN_ERROR;
    }

    size_t pos = 12U;
    for (uint16_t q = 0U; q < qdcount; q++) {
        if (!dnssd_skip_question(resp_buf, resp_len, &pos)) {
            return SYN_ERROR;
        }
    }

    for (uint32_t i = 0U; i < total_rr && pos < resp_len; i++) {
        char rr_name[128];
        if (!skip_or_read_name(resp_buf, resp_len, resp_len, &pos, rr_name, sizeof(rr_name))) {
            break;
        }

        if (resp_len - pos < 10U) {
            break;
        }
        uint16_t type = ((uint16_t)resp_buf[pos] << 8U) | resp_buf[pos + 1U];
        uint16_t rdlength = ((uint16_t)resp_buf[pos + 8U] << 8U) | resp_buf[pos + 9U];
        pos += 10U;

        if (rdlength > resp_len - pos) {
            break;
        }

        dnssd_parse_rr(resp_buf, resp_len, type, pos, rdlength, out_disc);
        pos += rdlength;
    }

    return SYN_OK;
}

SYN_Status syn_dnssd_discover(const SYN_DnsSd *sd, const char *service_type)
{
    if (sd == NULL || service_type == NULL || sd->sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    uint8_t query_buf[256];
    size_t query_len = 0U;
    if (syn_dnssd_build_query(service_type, query_buf, sizeof(query_buf), &query_len) != SYN_OK) {
        return SYN_ERROR;
    }

    SYN_SockAddr mcast_dest = {.ip = {224U, 0U, 0U, 251U}, .port = SYN_DNSSD_PORT};
    int sent = syn_port_udp_sendto(sd->sock, query_buf, query_len, &mcast_dest);
    return (sent == (int)query_len) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_dnssd_browser_init(SYN_DnsSd_Browser *browser, const SYN_DnsSd *sd,
                                  const char *service_type, SYN_DnsSd_DiscoverCallback cb,
                                  void *user_data, uint32_t timeout_ms)
{
    if (browser == NULL || sd == NULL || service_type == NULL || cb == NULL) {
        return SYN_ERROR;
    }
    memset(browser, 0, sizeof(*browser));
    browser->sd = sd;
    browser->service_type = service_type;
    browser->callback = cb;
    browser->user_data = user_data;
    browser->timeout_ms = timeout_ms;
    browser->start_time = syn_port_get_tick_ms();

    SYN_Status st = syn_dnssd_discover(sd, service_type);
    browser->active = (st == SYN_OK);
    return st;
}

/**
 * @brief Parse one received mDNS browser packet and invoke callback on service match.
 * @param browser Browser context.
 * @param buf     Received datagram bytes.
 * @param len     Received datagram length.
 */
static void dnssd_handle_browse_pkt(SYN_DnsSd_Browser *browser, const uint8_t *buf, size_t len)
{
    SYN_DnsSd_Discovered disc;
    if (syn_dnssd_parse_response(buf, len, &disc) != SYN_OK || disc.instance_name[0] == '\0') {
        return;
    }
    if (browser->service_type[0] == '\0' || strcmp(disc.service_type, browser->service_type) == 0) {
        browser->discovered_count++;
        if (browser->callback != NULL) {
            browser->callback(&disc, browser->user_data);
        }
    }
}

/**
 * @brief Check whether a DNS-SD browser task should wake from PT_WAIT_UNTIL.
 * @param browser Browser context.
 * @return true if inactive, socket closed, timed out, or UDP socket readable.
 */
static bool dnssd_browse_wait_done(const SYN_DnsSd_Browser *browser)
{
    if (!browser->active || browser->sd->sock == SYN_SOCKET_INVALID) {
        return true;
    }
    if (browser->timeout_ms > 0U &&
        (syn_port_get_tick_ms() - browser->start_time) >= browser->timeout_ms) {
        return true;
    }
    return syn_port_udp_readable(browser->sd->sock);
}

SYN_PT_Status syn_dnssd_browse_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_DnsSd_Browser *browser = (SYN_DnsSd_Browser *)task->user_data;
    if (browser->sd == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    while (browser->active) {
        if (browser->sd->sock == SYN_SOCKET_INVALID) {
            browser->active = false;
            PT_EXIT(pt);
        }

        if (browser->timeout_ms > 0U &&
            (syn_port_get_tick_ms() - browser->start_time) >= browser->timeout_ms) {
            browser->active = false;
            break;
        }

        if (syn_port_udp_readable(browser->sd->sock)) {
            uint8_t buf[512];
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(browser->sd->sock, buf, sizeof(buf), &from, 0);
            if (n <= 0) {
                PT_PASS(pt);
                continue;
            }
            if (n > 12) {
                dnssd_handle_browse_pkt(browser, buf, (size_t)n);
            }
            if (browser->active && syn_port_udp_readable(browser->sd->sock)) {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        }

        PT_WAIT_UNTIL(pt, dnssd_browse_wait_done(browser));
    }

    PT_END(pt);
}

/**
 * @brief Process one received mDNS query datagram in-place and multicast any matching response.
 * @param sd   DNS-SD daemon context.
 * @param pkt  Shared packet buffer containing query on entry and response on match.
 * @param cap  Total capacity of pkt buffer.
 * @param qlen Received query byte length.
 */
static void dnssd_handle_query_pkt(const SYN_DnsSd *sd, uint8_t *pkt, size_t cap, size_t qlen)
{
    size_t resp_len = 0U;
    if (syn_dnssd_process_query(sd, pkt, qlen, pkt, cap, &resp_len) == SYN_OK && resp_len > 0U) {
        SYN_SockAddr mcast_dest = {.ip = {224U, 0U, 0U, 251U}, .port = SYN_DNSSD_PORT};
        (void)syn_port_udp_sendto(sd->sock, pkt, resp_len, &mcast_dest);
    }
}

SYN_PT_Status syn_dnssd_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    const SYN_DnsSd *sd = (const SYN_DnsSd *)task->user_data;

    PT_BEGIN(pt);

    while (1) {
        if (sd->sock == SYN_SOCKET_INVALID) {
            PT_EXIT(pt);
        }

        uint8_t pkt[512];
        SYN_SockAddr from;
        int n = syn_port_udp_recvfrom(sd->sock, pkt, sizeof(pkt), &from, 0);
        if (n > 0) {
            dnssd_handle_query_pkt(sd, pkt, sizeof(pkt), (size_t)n);
            if (sd->sock != SYN_SOCKET_INVALID && syn_port_udp_readable(sd->sock)) {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        } else if (syn_port_udp_readable(sd->sock)) {
            PT_PASS(pt);
            continue;
        }

        PT_WAIT_UNTIL(pt, sd->sock == SYN_SOCKET_INVALID || syn_port_udp_readable(sd->sock));
    }

    PT_END(pt);
}
