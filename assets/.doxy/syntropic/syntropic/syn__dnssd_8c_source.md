

# File syn\_dnssd.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dnssd.c**](syn__dnssd_8c.md)

[Go to the documentation of this file](syn__dnssd_8c.md)


```C++

#include "syn_dnssd.h"

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

#include <stdio.h>
#include <string.h>

#define DNSSD_MCAST_ADDR "224.0.0.251"
#define DNSSD_TTL_PTR 4500U
#define DNSSD_TTL_SRV 120U

static void write_u16(uint8_t *buf, size_t *pos, uint16_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 8U);
    buf[(*pos)++] = (uint8_t)(val & 0xFFU);
}

static void write_u32(uint8_t *buf, size_t *pos, uint32_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 24U);
    buf[(*pos)++] = (uint8_t)(val >> 16U);
    buf[(*pos)++] = (uint8_t)(val >> 8U);
    buf[(*pos)++] = (uint8_t)(val & 0xFFU);
}

static void write_label(uint8_t *buf, size_t *pos, const char *str)
{
    size_t len = strlen(str);
    if (len > 63)
        len = 63; /* LCOV_EXCL_LINE: Defensive clamp */
    buf[(*pos)++] = (uint8_t)len;
    memcpy(&buf[*pos], str, len);
    *pos += len;
}

static void write_service_type_name(uint8_t *buf, size_t *pos, const char *service_type)
{
    /* Split service_type (e.g. "_http._tcp") */
    char temp[64];
    strncpy(temp, service_type, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *dot = strchr(temp, '.');
    if (dot != NULL) {
        *dot = '\0';
        write_label(buf, pos, temp);
        write_label(buf, pos, dot + 1);
    } else {
        write_label(buf, pos, temp);
    }
    write_label(buf, pos, "local");
    buf[(*pos)++] = 0x00; /* Null terminator */
}

static void write_instance_fqdn(uint8_t *buf, size_t *pos, const char *instance,
                                const char *service_type)
{
    write_label(buf, pos, instance);
    write_service_type_name(buf, pos, service_type);
}

static void write_hostname_fqdn(uint8_t *buf, size_t *pos, const char *hostname)
{
    write_label(buf, pos, hostname);
    write_label(buf, pos, "local");
    buf[(*pos)++] = 0x00;
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

SYN_Status syn_dnssd_register(SYN_DnsSd *sd, const SYN_DnsSd_Service *svc)
{
    if (sd == NULL || svc == NULL || svc->instance_name == NULL || svc->service_type == NULL ||
        svc->hostname == NULL || svc->port == 0) {
        return SYN_ERROR;
    }

    if (sd->service_count >= SYN_DNSSD_MAX_SERVICES) {
        return SYN_ERROR;
    }

    sd->services[sd->service_count++] = *svc;
    return SYN_OK;
}

static bool match_qname_service(const uint8_t *buf, size_t buf_len, size_t *offset,
                                const char *service_type)
{
    /* Match query against _service._proto.local */
    size_t pos = *offset;
    char q_name[128] = {0};
    size_t q_pos = 0;

    while (pos < buf_len && buf[pos] != 0) {
        uint8_t label_len = buf[pos++];
        if (label_len > 63 || pos + label_len > buf_len) {
            return false;
        }
        if (q_pos > 0 && q_pos < sizeof(q_name) - 1) {
            q_name[q_pos++] = '.';
        }
        if (q_pos + label_len < sizeof(q_name) - 1) {
            memcpy(&q_name[q_pos], &buf[pos], label_len);
            q_pos += label_len;
        }
        pos += label_len;
    }
    if (pos < buf_len && buf[pos] == 0) {
        pos++;
    }
    q_name[q_pos] = '\0';
    *offset = pos;

    char expected[128];
    snprintf(expected, sizeof(expected), "%s.local", service_type);

    return (strcmp(q_name, expected) == 0);
}

static SYN_Status pack_service_response(const SYN_DnsSd_Service *svc, uint8_t *resp_buf,
                                        size_t max_resp_len, size_t *resp_len)
{
    if (max_resp_len < 512) {
        return SYN_ERROR;
    }

    /* Build DNS-SD Response Packet */
    size_t rpos = 0;

    /* Header: ID=0, Flags=0x8400 (Response + Authoritative), QD=0, AN=1 (PTR), NS=0, AR=3 (SRV,
     * TXT, A) */
    write_u16(resp_buf, &rpos, 0x0000);
    write_u16(resp_buf, &rpos, 0x8400);
    write_u16(resp_buf, &rpos, 0x0000); /* QDCOUNT */
    write_u16(resp_buf, &rpos, 0x0001); /* ANCOUNT (1 PTR) */
    write_u16(resp_buf, &rpos, 0x0000); /* NSCOUNT */
    write_u16(resp_buf, &rpos, 0x0003); /* ARCOUNT (SRV, TXT, A) */

    /* 1. Answer Record: PTR */
    write_service_type_name(resp_buf, &rpos, svc->service_type);
    write_u16(resp_buf, &rpos, 0x000C); /* TYPE: PTR (12) */
    write_u16(resp_buf, &rpos, 0x0001); /* CLASS: IN (1) */
    write_u32(resp_buf, &rpos, DNSSD_TTL_PTR);

    size_t ptr_rdlen_pos = rpos;
    rpos += 2; /* Reserve 2 bytes for RDLENGTH */
    size_t ptr_rdata_start = rpos;
    write_instance_fqdn(resp_buf, &rpos, svc->instance_name, svc->service_type);
    uint16_t ptr_rdlen = (uint16_t)(rpos - ptr_rdata_start);
    resp_buf[ptr_rdlen_pos] = (uint8_t)(ptr_rdlen >> 8U);
    resp_buf[ptr_rdlen_pos + 1] = (uint8_t)(ptr_rdlen & 0xFFU);

    /* 2. Additional Record 1: SRV */
    write_instance_fqdn(resp_buf, &rpos, svc->instance_name, svc->service_type);
    write_u16(resp_buf, &rpos, 0x0021); /* TYPE: SRV (33) */
    write_u16(resp_buf, &rpos, 0x8001); /* CLASS: IN + Cache-Flush */
    write_u32(resp_buf, &rpos, DNSSD_TTL_SRV);

    size_t srv_rdlen_pos = rpos;
    rpos += 2;
    size_t srv_rdata_start = rpos;
    write_u16(resp_buf, &rpos, 0x0000); /* Priority 0 */
    write_u16(resp_buf, &rpos, 0x0000); /* Weight 0 */
    write_u16(resp_buf, &rpos, svc->port);
    write_hostname_fqdn(resp_buf, &rpos, svc->hostname);
    uint16_t srv_rdlen = (uint16_t)(rpos - srv_rdata_start);
    resp_buf[srv_rdlen_pos] = (uint8_t)(srv_rdlen >> 8U);
    resp_buf[srv_rdlen_pos + 1] = (uint8_t)(srv_rdlen & 0xFFU);

    /* 3. Additional Record 2: TXT */
    write_instance_fqdn(resp_buf, &rpos, svc->instance_name, svc->service_type);
    write_u16(resp_buf, &rpos, 0x0010); /* TYPE: TXT (16) */
    write_u16(resp_buf, &rpos, 0x8001); /* CLASS: IN + Cache-Flush */
    write_u32(resp_buf, &rpos, DNSSD_TTL_PTR);

    size_t txt_rdlen_pos = rpos;
    rpos += 2;
    size_t txt_rdata_start = rpos;
    if (svc->txt_count > 0) {
        for (size_t t = 0; t < svc->txt_count; t++) {
            write_label(resp_buf, &rpos, svc->txt_records[t]);
        }
    } else {
        resp_buf[rpos++] = 0x00; /* Empty TXT */
    }
    uint16_t txt_rdlen = (uint16_t)(rpos - txt_rdata_start);
    resp_buf[txt_rdlen_pos] = (uint8_t)(txt_rdlen >> 8U);
    resp_buf[txt_rdlen_pos + 1] = (uint8_t)(txt_rdlen & 0xFFU);

    /* 4. Additional Record 3: A (IPv4 address) */
    write_hostname_fqdn(resp_buf, &rpos, svc->hostname);
    write_u16(resp_buf, &rpos, 0x0001); /* TYPE: A (1) */
    write_u16(resp_buf, &rpos, 0x8001); /* CLASS: IN + Cache-Flush */
    write_u32(resp_buf, &rpos, DNSSD_TTL_SRV);
    write_u16(resp_buf, &rpos, 0x0004); /* RDLENGTH 4 */
    memcpy(&resp_buf[rpos], svc->ip, 4);
    rpos += 4;

    *resp_len = rpos;
    return SYN_OK;
}

SYN_Status syn_dnssd_process_query(const SYN_DnsSd *sd, const uint8_t *query_buf, size_t query_len,
                                   uint8_t *resp_buf, size_t max_resp_len, size_t *resp_len)
{
    if (sd == NULL || query_buf == NULL || resp_buf == NULL || resp_len == NULL) {
        return SYN_ERROR;
    }
    *resp_len = 0;

    if (query_len < 12) {
        return SYN_ERROR;
    }

    /* Check flags: QR bit must be 0 (Query) */
    uint16_t flags = ((uint16_t)query_buf[2] << 8U) | query_buf[3];
    if ((flags & 0x8000U) != 0) {
        return SYN_ERROR;
    }

    uint16_t qdcount = ((uint16_t)query_buf[4] << 8U) | query_buf[5];
    if (qdcount == 0) {
        return SYN_ERROR;
    }

    size_t pos = 12;
    const SYN_DnsSd_Service *matched_svc = NULL;

    for (uint16_t q = 0; q < qdcount; q++) {
        size_t name_start = pos;
        for (size_t s = 0; s < sd->service_count; s++) {
            size_t temp_pos = name_start;
            if (match_qname_service(query_buf, query_len, &temp_pos,
                                    sd->services[s].service_type)) {
                matched_svc = &sd->services[s];
                pos = temp_pos;
                break;
            }
        }
        if (matched_svc != NULL) {
            break;
        }
        /* Skip unknown QNAME */
        while (pos < query_len && query_buf[pos] != 0) {
            pos += 1 + query_buf[pos];
        }
        if (pos < query_len && query_buf[pos] == 0) {
            pos++;
        }
        pos += 4; /* Skip QTYPE (2) + QCLASS (2) */
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

    SYN_Status st =
        pack_service_response(&sd->services[service_index], resp_buf, max_resp_len, resp_len);
    if (st == SYN_OK && sd->sock != SYN_SOCKET_INVALID && *resp_len > 0) {
        SYN_SockAddr mcast_dest;
        mcast_dest.ip[0] = 224;
        mcast_dest.ip[1] = 0;
        mcast_dest.ip[2] = 0;
        mcast_dest.ip[3] = 251;
        mcast_dest.port = SYN_DNSSD_PORT;
        syn_port_udp_sendto(sd->sock, resp_buf, *resp_len, &mcast_dest);
    }
    return st;
}

SYN_Status syn_dnssd_build_query(const char *service_type, uint8_t *query_buf, size_t max_len,
                                 size_t *query_len)
{
    if (service_type == NULL || query_buf == NULL || query_len == NULL) {
        return SYN_ERROR;
    }
    if (max_len < 128) {
        return SYN_ERROR;
    }

    size_t pos = 0;
    /* Header: ID=0, Flags=0x0000 (Query), QDCOUNT=1, ANCOUNT=0, NSCOUNT=0, ARCOUNT=0 */
    write_u16(query_buf, &pos, 0x0000);
    write_u16(query_buf, &pos, 0x0000);
    write_u16(query_buf, &pos, 0x0001); /* QDCOUNT */
    write_u16(query_buf, &pos, 0x0000);
    write_u16(query_buf, &pos, 0x0000);
    write_u16(query_buf, &pos, 0x0000);

    /* Question: QNAME = _service._proto.local, QTYPE = PTR (12), QCLASS = IN (1) */
    write_service_type_name(query_buf, &pos, service_type);
    write_u16(query_buf, &pos, 0x000C); /* PTR */
    write_u16(query_buf, &pos, 0x0001); /* IN */

    *query_len = pos;
    return SYN_OK;
}

static void skip_or_read_name(const uint8_t *buf, size_t buf_len, size_t *pos, char *out,
                              size_t max_out)
{
    size_t p = *pos;
    size_t o = 0;
    if (out != NULL && max_out > 0) {
        out[0] = '\0';
    }

    while (p < buf_len) {
        uint8_t len = buf[p];
        if (len == 0) {
            p++;
            break;
        }
        if ((len & 0xC0U) == 0xC0U) {
            p += 2;
            break;
        }
        p++;
        if (p + len > buf_len) {
            break;
        }
        if (out != NULL && max_out > 0) {
            if (o > 0 && o + 1 < max_out) {
                out[o++] = '.';
            }
            size_t copy_len = len < (max_out - 1 - o) ? len : (max_out - 1 - o);
            memcpy(&out[o], &buf[p], copy_len);
            o += copy_len;
            out[o] = '\0';
        }
        p += len;
    }
    *pos = p;
}

SYN_Status syn_dnssd_parse_response(const uint8_t *resp_buf, size_t resp_len,
                                    SYN_DnsSd_Discovered *out_disc)
{
    if (resp_buf == NULL || out_disc == NULL || resp_len < 12) {
        return SYN_ERROR;
    }
    memset(out_disc, 0, sizeof(*out_disc));

    uint16_t flags = ((uint16_t)resp_buf[2] << 8U) | resp_buf[3];
    if ((flags & 0x8000U) == 0) {
        return SYN_ERROR; /* Not a response */
    }

    uint16_t ancount = ((uint16_t)resp_buf[6] << 8U) | resp_buf[7];
    uint16_t arcount = ((uint16_t)resp_buf[10] << 8U) | resp_buf[11];
    uint16_t total_rr = ancount + arcount;

    if (total_rr == 0) {
        return SYN_ERROR;
    }

    size_t pos = 12;

    /* Parse Resource Records */
    for (uint16_t i = 0; i < total_rr && pos < resp_len; i++) {
        char rr_name[128] = {0};
        skip_or_read_name(resp_buf, resp_len, &pos, rr_name, sizeof(rr_name));

        if (pos + 10 > resp_len) {
            break;
        }
        uint16_t type = ((uint16_t)resp_buf[pos] << 8U) | resp_buf[pos + 1];
        /* skip class (2), ttl (4) */
        uint16_t rdlength = ((uint16_t)resp_buf[pos + 8] << 8U) | resp_buf[pos + 9];
        pos += 10;

        if (pos + rdlength > resp_len) {
            break;
        }

        size_t rdata_start = pos;
        if (type == 0x000C) { /* PTR */
            char target_fqdn[128] = {0};
            size_t p_rd = rdata_start;
            skip_or_read_name(resp_buf, resp_len, &p_rd, target_fqdn, sizeof(target_fqdn));
            /* Extract instance name (first label) */
            char *first_dot = strchr(target_fqdn, '.');
            if (first_dot != NULL) {
                size_t inst_len = (size_t)(first_dot - target_fqdn);
                if (inst_len < sizeof(out_disc->instance_name)) {
                    memcpy(out_disc->instance_name, target_fqdn, inst_len);
                    out_disc->instance_name[inst_len] = '\0';
                }
                /* Service type is remainder without .local */
                strncpy(out_disc->service_type, first_dot + 1, sizeof(out_disc->service_type) - 1);
                char *loc = strstr(out_disc->service_type, ".local");
                if (loc != NULL) {
                    *loc = '\0';
                }
            }
        } else if (type == 0x0021 && rdlength >= 6) { /* SRV */
            out_disc->port =
                ((uint16_t)resp_buf[rdata_start + 4] << 8U) | resp_buf[rdata_start + 5];
            size_t p_host = rdata_start + 6;
            char host_fqdn[128] = {0};
            skip_or_read_name(resp_buf, resp_len, &p_host, host_fqdn, sizeof(host_fqdn));
            char *first_dot = strchr(host_fqdn, '.');
            if (first_dot != NULL) {
                *first_dot = '\0';
            }
            strncpy(out_disc->hostname, host_fqdn, sizeof(out_disc->hostname) - 1);
        } else if (type == 0x0010 && rdlength > 0) { /* TXT */
            uint8_t txt_len = resp_buf[rdata_start];
            if (txt_len > 0 && txt_len < sizeof(out_disc->txt) && (size_t)txt_len < rdlength) {
                memcpy(out_disc->txt, &resp_buf[rdata_start + 1], txt_len);
                out_disc->txt[txt_len] = '\0';
            }
        } else if (type == 0x0001 && rdlength == 4) { /* A */
            memcpy(out_disc->ip, &resp_buf[rdata_start], 4);
        }

        pos = rdata_start + rdlength;
    }

    return SYN_OK;
}

SYN_Status syn_dnssd_discover(const SYN_DnsSd *sd, const char *service_type)
{
    if (sd == NULL || service_type == NULL || sd->sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    uint8_t query_buf[256];
    size_t query_len = 0;
    (void)syn_dnssd_build_query(service_type, query_buf, sizeof(query_buf), &query_len);

    SYN_SockAddr mcast_dest;
    mcast_dest.ip[0] = 224;
    mcast_dest.ip[1] = 0;
    mcast_dest.ip[2] = 0;
    mcast_dest.ip[3] = 251;
    mcast_dest.port = SYN_DNSSD_PORT;

    return (syn_port_udp_sendto(sd->sock, query_buf, query_len, &mcast_dest) > 0) ? SYN_OK
                                                                                  : SYN_ERROR;
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
    browser->active = true;

    return syn_dnssd_discover(sd, service_type);
}

SYN_PT_Status syn_dnssd_browse_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_DnsSd_Browser *browser = (SYN_DnsSd_Browser *)task->user_data;
    SYN_ASSERT(browser != NULL);
    SYN_ASSERT(browser->sd != NULL);

    PT_BEGIN(pt);

    while (browser->active) {
        uint32_t now = syn_port_get_tick_ms();
        if (browser->timeout_ms > 0 && (now - browser->start_time) >= browser->timeout_ms) {
            browser->active = false;
            break;
        }

        if (browser->sd->sock != SYN_SOCKET_INVALID && syn_port_udp_readable(browser->sd->sock)) {
            uint8_t buf[1024];
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(browser->sd->sock, buf, sizeof(buf), &from, 0);
            if (n > 12) {
                SYN_DnsSd_Discovered disc;
                if (syn_dnssd_parse_response(buf, (size_t)n, &disc) == SYN_OK) {
                    if (disc.instance_name[0] != '\0' &&
                        (browser->service_type[0] == '\0' ||
                         strcmp(disc.service_type, browser->service_type) == 0)) {
                        browser->discovered_count++;
                        if (browser->callback != NULL) {
                            browser->callback(&disc, browser->user_data);
                        }
                    }
                }
            }
        }

        PT_WAIT_UNTIL(pt,
                      !browser->active ||
                          (browser->timeout_ms > 0 &&
                           (syn_port_get_tick_ms() - browser->start_time) >= browser->timeout_ms) ||
                          syn_port_udp_readable(browser->sd->sock));
    }

    PT_END(pt);
}

SYN_PT_Status syn_dnssd_task(SYN_PT *pt, SYN_Task *task)
{
    const SYN_DnsSd *sd = (const SYN_DnsSd *)task->user_data;
    SYN_ASSERT(sd != NULL);

    PT_BEGIN(pt);

    while (1) {
        if (sd->sock != SYN_SOCKET_INVALID) {
            uint8_t query[512];
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(sd->sock, query, sizeof(query), &from, 0);
            if (n > 0) {
                uint8_t resp[1024];
                size_t resp_len = 0;
                if (syn_dnssd_process_query(sd, query, (size_t)n, resp, sizeof(resp), &resp_len) ==
                        SYN_OK &&
                    resp_len > 0) {
                    SYN_SockAddr mcast_dest;
                    mcast_dest.ip[0] = 224;
                    mcast_dest.ip[1] = 0;
                    mcast_dest.ip[2] = 0;
                    mcast_dest.ip[3] = 251;
                    mcast_dest.port = SYN_DNSSD_PORT;
                    syn_port_udp_sendto(sd->sock, resp, resp_len, &mcast_dest);
                }
            }
        }
        PT_WAIT_UNTIL(pt, sd->sock == SYN_SOCKET_INVALID || syn_port_udp_readable(sd->sock));
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Infinite task loop end */
}
```


