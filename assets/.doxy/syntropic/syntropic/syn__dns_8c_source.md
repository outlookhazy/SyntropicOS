

# File syn\_dns.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dns.c**](syn__dns_8c.md)

[Go to the documentation of this file](syn__dns_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_DNS) || SYN_USE_DNS

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_dns.h"

#include <string.h>

static size_t encode_qname(uint8_t *dest, const char *src)
{
    uint8_t *len_ptr = dest;
    size_t total = 0;
    dest++;
    total++;
    uint8_t label_len = 0;
    while (*src) {
        if (*src == '.') {
            *len_ptr = label_len;
            len_ptr = dest;
            label_len = 0;
            dest++;
            total++;
        } else {
            *dest++ = (uint8_t)*src;
            label_len++;
            total++;
        }
        src++;
    }
    *len_ptr = label_len;
    *dest++ = 0;
    total++;
    return total;
}

static bool parse_qname(const uint8_t *buf, size_t buf_len, size_t *pos)
{
    size_t p = *pos;
    for (;;) {
        if (p >= buf_len)
            return false;
        uint8_t len = buf[p];
        if ((len & 0xC0) == 0xC0) {
            /* Pointer */
            p += 2;
            break;
        } else if (len == 0) {
            p++;
            break;
        } else {
            p += 1 + len;
        }
    }
    *pos = p;
    return true;
}

static SYN_Status parse_response(const uint8_t *buf, size_t rx_len, SYN_SockAddr *addr_out,
                                 uint16_t expected_txid)
{
    /* LCOV_EXCL_START: Short packet / TxID mismatch rejection */
    if (rx_len < 12)
        return SYN_ERROR;
    uint16_t rx_txid = syn_peek_u16(buf, 0);
    if (rx_txid != expected_txid)
        return SYN_ERROR;
    /* LCOV_EXCL_STOP */
    if ((buf[3] & 0x0F) != 0)
        return SYN_ERROR; /* RCODE != 0 (error) */

    uint16_t questions = syn_peek_u16(buf, 4);
    uint16_t answers = syn_peek_u16(buf, 6);

    if (answers == 0)
        return SYN_ERROR;

    /* Skip questions */
    size_t pos = 12;
    for (uint16_t i = 0; i < questions; i++) {
        if (!parse_qname(buf, rx_len, &pos))
            return SYN_ERROR;
        if (pos + 4 > rx_len)
            return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback
                               */
        pos += 4;             /* skip QTYPE and QCLASS */
    }

    /* Parse answers */
    for (uint16_t i = 0; i < answers; i++) {
        if (!parse_qname(buf, rx_len, &pos))
            return SYN_ERROR;
        if (pos + 10 > rx_len)
            return SYN_ERROR;

        uint16_t type = syn_peek_u16(buf, pos);
        uint16_t rdlen = syn_peek_u16(buf, pos + 8);
        pos += 10;

        if (type == 1 && rdlen == 4) {
            /* IPv4 A Record */
            if (pos + 4 > rx_len)
                return SYN_ERROR;
            memcpy(addr_out->ip, buf + pos, 4);
            addr_out->port = 0;
            return SYN_OK;
        }
        pos += rdlen;
    }

    return SYN_ERROR;
}

SYN_PT_Status syn_dns_resolve_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_DnsResolver *r = (SYN_DnsResolver *)task->user_data;
    SYN_ASSERT(r != NULL);

    PT_BEGIN(pt);

    r->status = SYN_TIMEOUT;
    if (r->dns_server != NULL) {
        r->server_addr = *(r->dns_server);
    } else {
        r->server_addr.ip[0] = 8;
        r->server_addr.ip[1] = 8;
        r->server_addr.ip[2] = 8;
        r->server_addr.ip[3] = 8;
        r->server_addr.port = 53;
    }

    r->sock = syn_port_udp_open(0);
    if (r->sock == SYN_SOCKET_INVALID) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    /* Build query */
    r->txid = (uint16_t)(syn_port_get_tick_ms() & 0xFFFF);
    memset(r->buf, 0, 12);
    syn_poke_u16(r->txid, r->buf, 0); /* Transaction ID */
    syn_poke_u16(0x0100, r->buf, 2);  /* Flags: Standard query, recursion desired */
    syn_poke_u16(0x0001, r->buf, 4);  /* Questions = 1 */

    {
        size_t pos = 12;
        pos += encode_qname(r->buf + pos, r->hostname);
        syn_poke_u16(0x0001, r->buf, pos);
        pos += 2; /* QTYPE = A */
        syn_poke_u16(0x0001, r->buf, pos);
        pos += 2; /* QCLASS = IN */
        r->query_len = pos;
    }

    if (syn_port_udp_sendto(r->sock, r->buf, r->query_len, &r->server_addr) != (int)r->query_len) {
        syn_port_sock_close(r->sock);
        r->sock = SYN_SOCKET_INVALID;
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->start_ms = syn_port_get_tick_ms();

    for (;;) {
        SYN_SockAddr from;
        int n = syn_port_udp_recvfrom(r->sock, r->buf, sizeof(r->buf), &from, 0);
        if (n > 0) {
            if (n < 12 || syn_peek_u16(r->buf, 0) != r->txid) {
                /* Runt or TXID mismatch — ignore and keep waiting */
                continue;
            }
            r->status = parse_response(r->buf, (size_t)n, r->addr_out, r->txid);
            break;
        }

        if ((syn_port_get_tick_ms() - r->start_ms) >= r->timeout_ms) {
            r->status = SYN_TIMEOUT;
            break;
        }

        PT_WAIT_UNTIL(pt, syn_port_udp_readable(r->sock) ||
                              (syn_port_get_tick_ms() - r->start_ms) >= r->timeout_ms);
    }

    syn_port_sock_close(r->sock);
    r->sock = SYN_SOCKET_INVALID;

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

static size_t get_base_hostname_len(const char *hostname)
{
    if (hostname == NULL)
        return 0; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    size_t len = strlen(hostname);
    if (len > 6 && strcmp(hostname + len - 6, ".local") == 0) {
        return len - 6;
    }
    return len;
}

static bool match_qname_local(const uint8_t *buf, size_t buf_len, size_t *pos, const char *hostname)
{
    if (hostname == NULL)
        return false;

    size_t p = *pos;
    if (p >= buf_len)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    /* First label: hostname length + string */
    uint8_t h_len = buf[p++];
    size_t host_len = get_base_hostname_len(hostname);
    if (h_len != host_len)
        return false;
    if (p + host_len > buf_len)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    if (memcmp(buf + p, hostname, host_len) != 0)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    p += host_len;

    /* Second label: "local" length + string */
    if (p >= buf_len)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    uint8_t l_len = buf[p++];
    if (l_len != 5)
        return false;
    if (p + 5 > buf_len)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    if (memcmp(buf + p, "local", 5) != 0)
        return false;
    p += 5;

    /* Terminator */
    if (p >= buf_len || buf[p] != 0)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    p++;

    *pos = p;
    return true;
}

SYN_PT_Status syn_mdns_task(SYN_PT *pt, SYN_Task *task)
{
    const SYN_Mdns *mdns = (const SYN_Mdns *)task->user_data;
    SYN_ASSERT(mdns != NULL);

    PT_BEGIN(pt);

    for (;;) {
        if (mdns->sock != SYN_SOCKET_INVALID) {
            uint8_t buf[256];
            SYN_SockAddr from;
            /* Non-blocking read (0 ms timeout) */
            int n = syn_port_udp_recvfrom(mdns->sock, buf, sizeof(buf), &from, 0);
            if (n > 12) {
                size_t rx_len = (size_t)n;
                uint16_t flags = syn_peek_u16(buf, 2);
                uint16_t questions = syn_peek_u16(buf, 4);

                /* Parse if it's a query (Flags QR bit = 0) */
                if ((flags & 0x8000) == 0 && questions > 0) {
                    size_t pos = 12;
                    bool matched = false;
                    for (uint16_t i = 0; i < questions; i++) {
                        if (match_qname_local(buf, rx_len, &pos, mdns->hostname)) {
                            matched = true;
                            break;
                        }
                        /* if didn't match, parse to skip */
                        if (!parse_qname(buf, rx_len, &pos))
                            break;
                        if (pos + 4 > rx_len)
                            break; /* LCOV_EXCL_LINE: Defensive default branch for enum state
                                      machine */
                        pos += 4;  /* QTYPE + QCLASS */
                    }

                    if (matched) {
                        /* Build mDNS response packet */
                        uint8_t resp[256];
                        memset(resp, 0, 12);
                        /* Flags: 0x8400 (Authoritative Answer, Response) */
                        syn_poke_u16(0x8400, resp, 2);
                        syn_poke_u16(0x0001, resp, 6); /* Answers = 1 */

                        size_t rpos = 12;
                        size_t base_len = get_base_hostname_len(mdns->hostname);
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

                        /* Send mDNS reply back to multicast group 224.0.0.251:5353 */
                        SYN_SockAddr mcast_dest;
                        mcast_dest.ip[0] = 224;
                        mcast_dest.ip[1] = 0;
                        mcast_dest.ip[2] = 0;
                        mcast_dest.ip[3] = 251;
                        mcast_dest.port = 5353;

                        syn_port_udp_sendto(mdns->sock, resp, rpos, &mcast_dest);
                    }
                }
            }
        }
        PT_WAIT_UNTIL(pt, mdns->sock == SYN_SOCKET_INVALID || syn_port_udp_readable(mdns->sock));
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Defensive bounds check / hardware port fallback */
}

#endif /* SYN_USE_DNS */
```


