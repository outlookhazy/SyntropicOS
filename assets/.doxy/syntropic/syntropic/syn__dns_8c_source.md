

# File syn\_dns.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.c**](syn__dns_8c.md)

[Go to the documentation of this file](syn__dns_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_DNS) || SYN_USE_DNS


#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_dns.h"

#include <string.h>

void syn_dns_init(SYN_DnsResolver *resolver)
{
    if (resolver == NULL) {
        return;
    }
    memset(resolver, 0, sizeof(*resolver));
    resolver->sock = SYN_SOCKET_INVALID;
    resolver->sock_open = false;
}

static void dns_close_socket(SYN_DnsResolver *r)
{
    if (r->sock_open && r->sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(r->sock);
    }
    r->sock = SYN_SOCKET_INVALID;
    r->sock_open = false;
}

static size_t encode_qname(uint8_t *dest, size_t dest_capacity, const char *src)
{
    uint8_t *len_ptr = dest;
    size_t total = 1U;
    dest++;
    uint8_t label_len = 0U;
    while (*src != '\0') {
        if (total + 1U >= dest_capacity) {
            return 0U;
        }
        if (*src == '.') {
            if (label_len == 0U) {
                return 0U;
            }
            *len_ptr = label_len;
            len_ptr = dest++;
            label_len = 0U;
            total++;
        } else {
            if (label_len >= 63U) {
                return 0U;
            }
            *dest++ = (uint8_t)*src;
            label_len++;
            total++;
        }
        src++;
    }
    if (label_len == 0U) {
        return 0U;
    }
    *len_ptr = label_len;
    *dest = 0U;
    return total + 1U;
}

static bool parse_qname(const uint8_t *buf, size_t buf_len, size_t *pos)
{
    size_t p = *pos;
    for (;;) {
        if (p >= buf_len) {
            return false;
        }
        uint8_t len = buf[p];
        if ((len & 0xC0U) == 0xC0U) {
            /* Pointer — 2 bytes total */
            if (p + 1U >= buf_len) {
                return false;
            }
            p += 2U;
            break;
        }
        if ((len & 0xC0U) != 0U) {
            return false;
        }
        if (len == 0U) {
            p++;
            break;
        }
        if ((size_t)len > buf_len - p - 1U) {
            return false;
        }
        p += 1U + (size_t)len;
    }
    *pos = p;
    return true;
}

static bool dns_skip_questions(const uint8_t *buf, size_t rx_len, uint16_t questions, size_t *pos)
{
    for (uint16_t i = 0U; i < questions; i++) {
        if (!parse_qname(buf, rx_len, pos) || *pos + 4U > rx_len) {
            return false;
        }
        *pos += 4U; /* skip QTYPE and QCLASS */
    }
    return true;
}

static SYN_Status parse_response(const uint8_t *buf, size_t rx_len, SYN_SockAddr *addr_out)
{
    if ((buf[2] & 0x80U) == 0U || (buf[3] & 0x0FU) != 0U) {
        return SYN_ERROR; /* QR == 0 (query) or RCODE != 0 */
    }

    uint16_t questions = syn_peek_u16(buf, 4U);
    uint16_t answers = syn_peek_u16(buf, 6U);

    if (answers == 0U) {
        return SYN_ERROR;
    }

    /* Skip questions */
    size_t pos = 12U;
    if (!dns_skip_questions(buf, rx_len, questions, &pos)) {
        return SYN_ERROR;
    }

    /* Parse answers */
    for (uint16_t i = 0U; i < answers; i++) {
        if (!parse_qname(buf, rx_len, &pos) || pos + 10U > rx_len) {
            return SYN_ERROR;
        }

        uint16_t type = syn_peek_u16(buf, pos);
        uint16_t rclass = syn_peek_u16(buf, pos + 2U);
        uint16_t rdlen = syn_peek_u16(buf, pos + 8U);
        pos += 10U;
        if ((size_t)rdlen > rx_len - pos) {
            return SYN_ERROR;
        }

        if (type == 1U && rclass == 1U && rdlen == 4U) {
            /* IPv4 A Record (CLASS IN) */
            memcpy(addr_out->ip, buf + pos, 4U);
            addr_out->port = 0U;
            return SYN_OK;
        }
        pos += rdlen;
    }

    return SYN_ERROR;
}

static bool dns_build_and_send_query(SYN_DnsResolver *r)
{
    if (r->dns_server != NULL) {
        r->server_addr = *(r->dns_server);
    } else {
        r->server_addr.ip[0] = 8;
        r->server_addr.ip[1] = 8;
        r->server_addr.ip[2] = 8;
        r->server_addr.ip[3] = 8;
        r->server_addr.port = 53;
    }

    r->txid = (uint16_t)(syn_port_get_tick_ms() & 0xFFFFU);
    memset(r->buf, 0, 12);
    syn_poke_u16(r->txid, r->buf, 0); /* Transaction ID */
    syn_poke_u16(0x0100, r->buf, 2);  /* Flags: Standard query, recursion desired */
    syn_poke_u16(0x0001, r->buf, 4);  /* Questions = 1 */

    size_t pos = 12;
    size_t qname_len = encode_qname(r->buf + pos, sizeof(r->buf) - pos - 4U, r->hostname);
    if (qname_len == 0U) {
        return false;
    }
    pos += qname_len;
    syn_poke_u16(0x0001, r->buf, pos);
    pos += 2; /* QTYPE = A */
    syn_poke_u16(0x0001, r->buf, pos);
    pos += 2; /* QCLASS = IN */
    r->query_len = pos;

    return syn_port_udp_sendto(r->sock, r->buf, r->query_len, &r->server_addr) == (int)r->query_len;
}

static bool dns_packet_matches_txid(const SYN_DnsResolver *r, int n)
{
    return (n >= 12) && (syn_peek_u16(r->buf, 0) == r->txid);
}

static bool dns_wait_ready(const SYN_DnsResolver *r)
{
    return syn_port_udp_readable(r->sock) ||
           ((syn_port_get_tick_ms() - r->start_ms) >= r->timeout_ms);
}

SYN_PT_Status syn_dns_resolve_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_DnsResolver *r = (SYN_DnsResolver *)task->user_data;

    PT_BEGIN(pt);

    dns_close_socket(r);

    if (r->hostname == NULL || r->addr_out == NULL) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->status = SYN_TIMEOUT;
    r->sock = syn_port_udp_open(0);
    if (r->sock == SYN_SOCKET_INVALID) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }
    r->sock_open = true;

    if (!dns_build_and_send_query(r)) {
        dns_close_socket(r);
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->start_ms = syn_port_get_tick_ms();

    for (;;) {
        if ((syn_port_get_tick_ms() - r->start_ms) >= r->timeout_ms) {
            r->status = SYN_TIMEOUT;
            break;
        }

        SYN_SockAddr from;
        int n = syn_port_udp_recvfrom(r->sock, r->buf, sizeof(r->buf), &from, 0);
        if (n > 0) {
            if (!dns_packet_matches_txid(r, n)) {
                /* Runt or TXID mismatch — ignore and keep waiting */
                PT_CONTINUE_IF_NEEDED(pt, task);
                continue;
            }
            r->status = parse_response(r->buf, (size_t)n, r->addr_out);
            break;
        }

        if (syn_port_udp_readable(r->sock)) {
            PT_PASS(pt);
            continue;
        }

        PT_WAIT_UNTIL(pt, dns_wait_ready(r));
    }

    dns_close_socket(r);

    PT_END(pt);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  mDNS Responder Task                                                    */
/* ════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_mdns_init(SYN_Mdns *mdns, const char *hostname, const uint8_t ip[4])
{
    SYN_ASSERT(mdns != NULL);
    SYN_ASSERT(hostname != NULL);
    SYN_ASSERT(ip != NULL);

    memset(mdns, 0, sizeof(*mdns));
    mdns->hostname = hostname;
    memcpy(mdns->ip, ip, 4);
    mdns->sock = SYN_SOCKET_INVALID;

    mdns->sock = syn_port_udp_open(5353);
    if (mdns->sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    if (syn_port_udp_join_multicast(mdns->sock, "224.0.0.251") != SYN_OK) {
        syn_port_sock_close(mdns->sock);
        mdns->sock = SYN_SOCKET_INVALID;
        return SYN_ERROR;
    }

    return SYN_OK;
}

static uint8_t dns_ascii_lower(uint8_t c)
{
    if (c >= (uint8_t)'A' && c <= (uint8_t)'Z') {
        return (uint8_t)(c + 32U);
    }
    return c;
}

static bool dns_memeq_ci(const void *a, const void *b, size_t len)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    for (size_t i = 0U; i < len; i++) {
        if (dns_ascii_lower(pa[i]) != dns_ascii_lower(pb[i])) {
            return false;
        }
    }
    return true;
}

static size_t get_base_hostname_len(const char *hostname)
{
    size_t len = strlen(hostname);
    if (len > 6U && dns_memeq_ci(hostname + len - 6U, ".local", 6U)) {
        return len - 6U;
    }
    return len;
}

static bool mdns_deref_pointers(const uint8_t *buf, size_t buf_len, size_t *p, size_t *end_pos,
                                uint8_t *hops)
{
    size_t cur = *p;
    while (cur < buf_len && (buf[cur] & 0xC0U) == 0xC0U) {
        if (cur + 1U >= buf_len || *hops >= 8U) {
            return false;
        }
        size_t target = (((size_t)(buf[cur] & 0x3FU)) << 8U) | (size_t)buf[cur + 1U];
        if (target < 12U || target >= cur) {
            return false;
        }
        if (*hops == 0U) {
            *end_pos = cur + 2U;
        }
        cur = target;
        (*hops)++;
    }
    *p = cur;
    return (cur < buf_len);
}

static bool match_dns_label(const uint8_t *buf, size_t buf_len, size_t *pos, size_t *end_pos,
                            uint8_t *hops, const char *label, size_t label_len)
{
    if (!mdns_deref_pointers(buf, buf_len, pos, end_pos, hops)) {
        return false;
    }
    size_t p = *pos;
    if ((size_t)buf[p] != label_len || (buf_len - p - 1U) < label_len) {
        return false;
    }
    if (!dns_memeq_ci(buf + p + 1U, label, label_len)) {
        return false;
    }
    *pos = p + 1U + label_len;
    return true;
}

static bool match_qname_local(const uint8_t *buf, size_t buf_len, size_t *pos, const char *hostname)
{
    if (hostname == NULL) {
        return false;
    }

    size_t p = *pos;
    size_t end_pos = 0U;
    uint8_t hops = 0U;
    size_t host_len = get_base_hostname_len(hostname);
    if (!match_dns_label(buf, buf_len, &p, &end_pos, &hops, hostname, host_len) ||
        !match_dns_label(buf, buf_len, &p, &end_pos, &hops, "local", 5U) ||
        !mdns_deref_pointers(buf, buf_len, &p, &end_pos, &hops) || buf[p] != 0U) {
        return false;
    }
    *pos = (hops > 0U) ? end_pos : (p + 1U);
    return true;
}

static void mdns_send_response(const SYN_Mdns *mdns)
{
    uint8_t resp[256];
    size_t base_len = get_base_hostname_len(mdns->hostname);
    if (base_len == 0U || base_len > 63U) {
        return;
    }

    memset(resp, 0, 12);
    syn_poke_u16(0x8400, resp, 2); /* Authoritative Answer, Response */
    syn_poke_u16(0x0001, resp, 6); /* Answers = 1 */

    size_t rpos = 12;
    resp[rpos++] = (uint8_t)base_len;
    memcpy(resp + rpos, mdns->hostname, base_len);
    rpos += base_len;
    resp[rpos++] = 5;
    memcpy(resp + rpos, "local", 5);
    rpos += 5;
    resp[rpos++] = 0;

    syn_poke_u16(0x0001, resp, rpos);
    rpos += 2; /* TYPE = A */
    syn_poke_u16(0x8001, resp, rpos);
    rpos += 2; /* CLASS = IN + cache flush */
    syn_poke_u32(120, resp, rpos);
    rpos += 4; /* TTL = 120 */
    syn_poke_u16(4, resp, rpos);
    rpos += 2;                        /* RDLEN = 4 */
    memcpy(resp + rpos, mdns->ip, 4); /* Address */
    rpos += 4;

    SYN_SockAddr mcast_dest;
    mcast_dest.ip[0] = 224;
    mcast_dest.ip[1] = 0;
    mcast_dest.ip[2] = 0;
    mcast_dest.ip[3] = 251;
    mcast_dest.port = 5353;

    syn_port_udp_sendto(mdns->sock, resp, rpos, &mcast_dest);
}

static void mdns_handle_packet(const SYN_Mdns *mdns, const uint8_t *buf, size_t rx_len)
{
    uint16_t flags = syn_peek_u16(buf, 2U);
    uint16_t questions = syn_peek_u16(buf, 4U);
    if ((flags & 0x8000U) != 0U || questions == 0U) {
        return;
    }

    size_t pos = 12U;
    for (uint16_t i = 0U; i < questions; i++) {
        size_t qpos = pos;
        bool matched = match_qname_local(buf, rx_len, &pos, mdns->hostname);
        if (!matched && !parse_qname(buf, rx_len, &qpos)) {
            return;
        }
        if (!matched) {
            pos = qpos;
        }
        if (pos + 4U > rx_len) {
            return;
        }
        uint16_t qtype = syn_peek_u16(buf, pos);
        uint16_t qclass = (uint16_t)(syn_peek_u16(buf, pos + 2U) & 0x7FFFU);
        pos += 4U; /* QTYPE + QCLASS */
        if (matched && (qtype == 1U || qtype == 255U) && (qclass == 1U || qclass == 255U)) {
            mdns_send_response(mdns);
            return;
        }
    }
}

SYN_PT_Status syn_mdns_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    const SYN_Mdns *mdns = (const SYN_Mdns *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        if (mdns->sock == SYN_SOCKET_INVALID) {
            PT_EXIT(pt);
        }

        uint8_t buf[256];
        SYN_SockAddr from;
        int n = syn_port_udp_recvfrom(mdns->sock, buf, sizeof(buf), &from, 0);
        if (n > 12) {
            mdns_handle_packet(mdns, buf, (size_t)n);
        }
        if (n > 0 && syn_port_udp_readable(mdns->sock)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
            continue;
        }
        if (n <= 0 && syn_port_udp_readable(mdns->sock)) {
            PT_PASS(pt);
            continue;
        }
        PT_WAIT_UNTIL(pt, mdns->sock == SYN_SOCKET_INVALID || syn_port_udp_readable(mdns->sock));
    }

    PT_END(pt);
}

#endif /* SYN_USE_DNS */
```


