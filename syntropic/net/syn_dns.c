#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DNS) || SYN_USE_DNS

/**
 * @file syn_dns.c
 * @brief UDP DNS resolver and mDNS responder implementation.
 */

#include "syn_dns.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"
#include <string.h>

/**
 * @brief Encode a hostname as a DNS QNAME (label-length format).
 * @param dest  [out] Destination buffer.
 * @param src   Dot-separated hostname.
 * @return Total bytes written.
 */
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

/**
 * @brief Skip a QNAME in a DNS packet (handles compression pointers).
 * @param buf      Packet buffer.
 * @param buf_len  Buffer length.
 * @param pos      [in/out] Current parse position.
 * @return true on success.
 */
static bool parse_qname(const uint8_t *buf, size_t buf_len, size_t *pos)
{
    size_t p = *pos;
    for (;;) {
        if (p >= buf_len) return false;
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

/**
 * @brief Parse a DNS response packet to extract the resolved IP.
 *
 * Checks transaction ID, question count, and pulls the first IPv4 A record.
 *
 * @param buf       Received UDP packet buffer.
 * @param rx_len    Received packet length.
 * @param addr_out       [out] Output structure for resolved IPv4 address.
 * @param expected_txid  Transaction ID to verify.
 * @return SYN_OK on success, SYN_ERROR on format/parsing error.
 */
static SYN_Status parse_response(const uint8_t *buf, size_t rx_len, SYN_SockAddr *addr_out, uint16_t expected_txid)
{
    if (rx_len < 12) return SYN_ERROR;
    uint16_t rx_txid = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    if (rx_txid != expected_txid) return SYN_ERROR; /* Bad ID */
    if ((buf[3] & 0x0F) != 0) return SYN_ERROR;             /* RCODE != 0 (error) */

    uint16_t questions = (uint16_t)(((uint16_t)buf[4] << 8) | buf[5]);
    uint16_t answers   = (uint16_t)(((uint16_t)buf[6] << 8) | buf[7]);

    if (answers == 0) return SYN_ERROR;

    /* Skip questions */
    size_t pos = 12;
    for (uint16_t i = 0; i < questions; i++) {
        if (!parse_qname(buf, rx_len, &pos)) return SYN_ERROR;
        pos += 4; /* skip QTYPE and QCLASS */
    }

    /* Parse answers */
    for (uint16_t i = 0; i < answers; i++) {
        if (!parse_qname(buf, rx_len, &pos)) return SYN_ERROR;
        if (pos + 10 > rx_len) return SYN_ERROR;

        uint16_t type  = (uint16_t)(((uint16_t)buf[pos]   << 8) | buf[pos+1]);
        uint16_t rdlen = (uint16_t)(((uint16_t)buf[pos+8]  << 8) | buf[pos+9]);
        pos += 10;

        if (type == 1 && rdlen == 4) {
            /* IPv4 A Record */
            if (pos + 4 > rx_len) return SYN_ERROR;
            memcpy(addr_out->ip, buf + pos, 4);
            addr_out->port = 0;
            return SYN_OK;
        }
        pos += rdlen;
    }

    return SYN_ERROR;
}

/**
 * @brief Cooperative task for resolving a hostname via DNS.
 */
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
    r->buf[0] = (uint8_t)(r->txid >> 8); 
    r->buf[1] = (uint8_t)(r->txid & 0xFF); /* Transaction ID */
    r->buf[2] = 0x01; r->buf[3] = 0x00; /* Flags: Standard query, recursion desired */
    r->buf[5] = 0x01;                   /* Questions = 1 */

    {
        size_t pos = 12;
        pos += encode_qname(r->buf + pos, r->hostname);
        r->buf[pos++] = 0x00; r->buf[pos++] = 0x01; /* QTYPE = A */
        r->buf[pos++] = 0x00; r->buf[pos++] = 0x01; /* QCLASS = IN */
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
            r->status = parse_response(r->buf, (size_t)n, r->addr_out, r->txid);
            if (r->status == SYN_OK) {
                break;
            }
        }

        if ((syn_port_get_tick_ms() - r->start_ms) >= r->timeout_ms) {
            r->status = SYN_TIMEOUT;
            break;
        }

        PT_YIELD(pt);
    }

    syn_port_sock_close(r->sock);
    r->sock = SYN_SOCKET_INVALID;

    PT_END(pt);
}



SYN_Status syn_mdns_init(SYN_Mdns *mdns, const char *hostname, const uint8_t ip[4])
{
    SYN_ASSERT(mdns != NULL);
    SYN_ASSERT(hostname != NULL);

    memset(mdns, 0, sizeof(*mdns));
    mdns->hostname = hostname;
    memcpy(mdns->ip, ip, 4);

    mdns->sock = syn_port_udp_open(5353);
    if (mdns->sock == SYN_SOCKET_INVALID) return SYN_ERROR;

    if (syn_port_udp_join_multicast(mdns->sock, "224.0.0.251") != SYN_OK) {
        syn_port_sock_close(mdns->sock);
        mdns->sock = SYN_SOCKET_INVALID;
        return SYN_ERROR;
    }

    return SYN_OK;
}

/**
 * @brief Check if QNAME matches hostname.local.
 * @param buf       Packet buffer.
 * @param buf_len   Buffer length.
 * @param pos       [in/out] Parse position.
 * @param hostname  Expected hostname.
 * @return true if match.
 */
static bool match_qname_local(const uint8_t *buf, size_t buf_len, size_t *pos, const char *hostname)
{
    size_t p = *pos;
    if (p >= buf_len) return false;
    
    /* First label: hostname length + string */
    uint8_t h_len = buf[p++];
    size_t host_len = strlen(hostname);
    if (h_len != host_len) return false;
    if (p + host_len > buf_len) return false;
    if (memcmp(buf + p, hostname, host_len) != 0) return false;
    p += host_len;

    /* Second label: "local" length + string */
    if (p >= buf_len) return false;
    uint8_t l_len = buf[p++];
    if (l_len != 5) return false;
    if (p + 5 > buf_len) return false;
    if (memcmp(buf + p, "local", 5) != 0) return false;
    p += 5;

    /* Terminator */
    if (p >= buf_len || buf[p] != 0) return false;
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
                uint16_t flags     = (uint16_t)(((uint16_t)buf[2] << 8) | buf[3]);
                uint16_t questions = (uint16_t)(((uint16_t)buf[4] << 8) | buf[5]);
                
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
                        if (!parse_qname(buf, rx_len, &pos)) break;
                        pos += 4; /* QTYPE + QCLASS */
                    }

                    if (matched) {
                        /* Build mDNS response packet */
                        uint8_t resp[256];
                        memset(resp, 0, 12);
                        /* Flags: 0x8400 (Authoritative Answer, Response) */
                        resp[2] = 0x84; resp[3] = 0x00;
                        resp[7] = 0x01; /* Answers = 1 */

                        size_t rpos = 12;
                        rpos += encode_qname(resp + rpos, mdns->hostname);
                        /* Replace trailing 0 in qname with pointer to ".local" */
                        rpos--;
                        resp[rpos++] = 5;
                        memcpy(resp + rpos, "local", 5);
                        rpos += 5;
                        resp[rpos++] = 0;

                        resp[rpos++] = 0x00; resp[rpos++] = 0x01; /* TYPE = A */
                        resp[rpos++] = 0x80; resp[rpos++] = 0x01; /* CLASS = IN + cache flush */
                        resp[rpos++] = 0x00; resp[rpos++] = 0x00; resp[rpos++] = 0x00; resp[rpos++] = 0x78; /* TTL = 120 */
                        resp[rpos++] = 0x00; resp[rpos++] = 0x04; /* RDLEN = 4 */
                        memcpy(resp + rpos, mdns->ip, 4);          /* Address */
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
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_DNS */
