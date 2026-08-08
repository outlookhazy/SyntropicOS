

# File syn\_eth.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_eth.c**](syn__eth_8c.md)

[Go to the documentation of this file](syn__eth_8c.md)


```C++

#include "syntropic/net/syn_eth.h"

#include "syntropic/net/syn_icmp.h"
#include "syntropic/net/syn_tcp.h"
#include "syntropic/net/syn_transport_udp.h"
#include "syntropic/net/syn_udp.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/util/syn_crc.h"

#include <string.h>

/* LCOV_EXCL_START: Defensive bounds check / hardware port fallback */
SYN_WEAK SYN_UDP *syn_transport_udp_get_instance(void)
{
    return NULL;
}

SYN_WEAK struct SYN_ICMP *syn_eth_get_icmp_instance(void)
{
    return NULL;
}

SYN_WEAK struct SYN_TCP *syn_eth_get_tcp_instance(void)
{
    return NULL;
}
/* LCOV_EXCL_STOP */

static const uint8_t MAC_BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

SYN_Status syn_eth_generate_mac(const void *uid_bytes, size_t uid_len, uint8_t mac_out[6])
{
    if (!uid_bytes || uid_len == 0 || !mac_out) {
        return SYN_INVALID_PARAM;
    }

    uint32_t hash = syn_crc32(uid_bytes, uid_len);

    mac_out[0] = 0x02; /* Locally Administered Unicast MAC */
    mac_out[1] = (uint8_t)(hash >> 24);
    mac_out[2] = (uint8_t)(hash >> 16);
    mac_out[3] = (uint8_t)(hash >> 8);
    mac_out[4] = (uint8_t)(hash);
    mac_out[5] = (uint8_t)((hash >> 24) ^ (hash & 0xFF));

    return SYN_OK;
}

SYN_Status syn_eth_init(SYN_ETH *eth, const uint8_t mac_addr[6], uint32_t ip_addr)
{
    if (!eth || !mac_addr) {
        return SYN_INVALID_PARAM;
    }
    memset(eth, 0, sizeof(*eth));
    memcpy(eth->mac_addr, mac_addr, 6);
    eth->ip_addr = ip_addr;
    return SYN_OK;
}

SYN_Status syn_eth_arp_update(SYN_ETH *eth, uint32_t ip, const uint8_t mac[6])
{
    if (!eth || !mac || ip == 0) {
        return SYN_INVALID_PARAM;
    }

    uint32_t now = syn_port_get_tick_ms();

    /* Search for existing entry to update */
    for (size_t i = 0; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        if (eth->arp_cache[i].valid && eth->arp_cache[i].ip == ip) {
            memcpy(eth->arp_cache[i].mac, mac, 6);
            eth->arp_cache[i].last_seen_ms = now;
            return SYN_OK;
        }
    }

    /* Insert into first empty entry */
    for (size_t i = 0; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        if (!eth->arp_cache[i].valid) {
            eth->arp_cache[i].ip = ip;
            memcpy(eth->arp_cache[i].mac, mac, 6);
            eth->arp_cache[i].valid = true;
            eth->arp_cache[i].last_seen_ms = now;
            return SYN_OK;
        }
    }

    /* Cache full: evict the least-recently-used entry */
    size_t lru_idx = 0;
    for (size_t i = 1; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        if ((int32_t)(eth->arp_cache[i].last_seen_ms - eth->arp_cache[lru_idx].last_seen_ms) < 0) {
            lru_idx = i;
        }
    }
    eth->arp_cache[lru_idx].ip = ip;
    memcpy(eth->arp_cache[lru_idx].mac, mac, 6);
    eth->arp_cache[lru_idx].valid = true;
    eth->arp_cache[lru_idx].last_seen_ms = now;

    return SYN_OK;
}

SYN_Status syn_eth_arp_lookup(SYN_ETH *eth, uint32_t ip, uint8_t mac_out[6])
{
    if (!eth || !mac_out) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        if (eth->arp_cache[i].valid && eth->arp_cache[i].ip == ip) {
            memcpy(mac_out, eth->arp_cache[i].mac, 6);
            /* Refresh LRU timestamp on each successful lookup */
            eth->arp_cache[i].last_seen_ms = syn_port_get_tick_ms();
            return SYN_OK;
        }
    }

    return SYN_NOT_FOUND;
}

SYN_Status syn_eth_build_frame(SYN_ETH *eth, const uint8_t dst_mac[6], uint16_t ethertype,
                               const uint8_t *payload, size_t payload_len, uint8_t *frame_out,
                               size_t *frame_len)
{
    if (!eth || !dst_mac || !frame_out || !frame_len) {
        return SYN_INVALID_PARAM;
    }

    size_t total_len = SYN_ETH_HEADER_LEN + payload_len;
    if (total_len < SYN_ETH_MIN_FRAME_LEN) {
        total_len = SYN_ETH_MIN_FRAME_LEN;
    }
    if (total_len > SYN_ETH_MAX_FRAME_LEN) {
        return SYN_INVALID_PARAM;
    }

    memcpy(&frame_out[0], dst_mac, 6);
    memcpy(&frame_out[6], eth->mac_addr, 6);
    frame_out[12] = (uint8_t)((ethertype >> 8) & 0xFF);
    frame_out[13] = (uint8_t)(ethertype & 0xFF);

    if (payload && payload_len > 0) {
        memcpy(&frame_out[14], payload, payload_len);
    }

    /* Zero pad frame to 60 bytes minimum */
    if (total_len > SYN_ETH_HEADER_LEN + payload_len) {
        memset(&frame_out[SYN_ETH_HEADER_LEN + payload_len], 0,
               total_len - (SYN_ETH_HEADER_LEN + payload_len));
    }

    eth->frames_tx++;
    *frame_len = total_len;

    return SYN_OK;
}

SYN_Status syn_eth_process_frame(SYN_ETH *eth, const uint8_t *frame, size_t len, uint8_t *tx_buf,
                                 size_t *tx_len)
{
    if (!eth || !frame || len < SYN_ETH_HEADER_LEN) {
        return SYN_INVALID_PARAM;
    }

    if (tx_len) {
        *tx_len = 0;
    }

    eth->frames_rx++;

    const uint8_t *dst_mac = &frame[0];
    uint16_t ethertype = ((uint16_t)frame[12] << 8) | frame[13];

    /* MAC Filter: drop frames not addressed to us or broadcast */
    bool is_unicast = (memcmp(dst_mac, eth->mac_addr, 6) == 0);
    bool is_bcast = (memcmp(dst_mac, MAC_BROADCAST, 6) == 0);
    if (!is_unicast && !is_bcast) {
        return SYN_OK;
    }

    /* Process ARP */
    if (ethertype == SYN_ETHTYPE_ARP && len >= 42) {
        uint16_t htype = ((uint16_t)frame[14] << 8) | frame[15];
        uint16_t ptype = ((uint16_t)frame[16] << 8) | frame[17];
        uint16_t oper = ((uint16_t)frame[20] << 8) | frame[21];

        if (htype == 1 && ptype == SYN_ETHTYPE_IPV4) {
            const uint8_t *sender_mac = &frame[22];
            uint32_t sender_ip = ((uint32_t)frame[28] << 24) | ((uint32_t)frame[29] << 16) |
                                 ((uint32_t)frame[30] << 8) | frame[31];
            uint32_t target_ip = ((uint32_t)frame[38] << 24) | ((uint32_t)frame[39] << 16) |
                                 ((uint32_t)frame[40] << 8) | frame[41];

            syn_eth_arp_update(eth, sender_ip, sender_mac);

            if (oper == SYN_ARP_OP_REQUEST) {
                eth->arp_requests++;
                if (target_ip == eth->ip_addr && tx_buf && tx_len) {
                    /* Build ARP Reply Payload (28 bytes) */
                    uint8_t arp_reply_payload[28];
                    arp_reply_payload[0] = 0;
                    arp_reply_payload[1] = 1; /* HW = Ethernet */
                    arp_reply_payload[2] = 0x08;
                    arp_reply_payload[3] = 0x00; /* Proto = IPv4 */
                    arp_reply_payload[4] = 6;
                    arp_reply_payload[5] = 4; /* HW len, Proto len */
                    arp_reply_payload[6] = 0;
                    arp_reply_payload[7] = 2; /* Oper = Reply */

                    memcpy(&arp_reply_payload[8], eth->mac_addr, 6);
                    arp_reply_payload[14] = (uint8_t)(eth->ip_addr >> 24);
                    arp_reply_payload[15] = (uint8_t)(eth->ip_addr >> 16);
                    arp_reply_payload[16] = (uint8_t)(eth->ip_addr >> 8);
                    arp_reply_payload[17] = (uint8_t)(eth->ip_addr);

                    memcpy(&arp_reply_payload[18], sender_mac, 6);
                    arp_reply_payload[24] = (uint8_t)(sender_ip >> 24);
                    arp_reply_payload[25] = (uint8_t)(sender_ip >> 16);
                    arp_reply_payload[26] = (uint8_t)(sender_ip >> 8);
                    arp_reply_payload[27] = (uint8_t)(sender_ip);

                    syn_eth_build_frame(eth, sender_mac, SYN_ETHTYPE_ARP, arp_reply_payload, 28,
                                        tx_buf, tx_len);
                }
            } else if (oper == SYN_ARP_OP_REPLY) {
                eth->arp_replies++;
            }
        }
    } else if (ethertype == SYN_ETHTYPE_IPV4 && len >= 34) {
        uint8_t ip_proto = frame[23];

        if (ip_proto == 17) { /* UDP */
            SYN_UDP *udp_inst = syn_transport_udp_get_instance();
            if (udp_inst) {
                syn_udp_process_packet(udp_inst, frame, len);
            }
        } else if (ip_proto == 1 && tx_buf && tx_len) { /* ICMP */
            SYN_ICMP *icmp_inst = (SYN_ICMP *)syn_eth_get_icmp_instance();
            if (icmp_inst) {
                syn_icmp_process_packet(icmp_inst, frame, len, tx_buf, tx_len);
            }
        } else if (ip_proto == 6 && tx_buf && tx_len) { /* TCP */
            SYN_TCP *tcp_inst = (SYN_TCP *)syn_eth_get_tcp_instance();
            if (tcp_inst) {
                syn_tcp_process_packet(tcp_inst, frame, len, tx_buf, tx_len);
            }
        }
    }

    return SYN_OK;
}

uint16_t syn_ip_checksum(const void *buf, size_t len)
{
    if (!buf || len == 0) {
        /* LCOV_EXCL_START: Invalid param guard */
        return 0;
        /* LCOV_EXCL_STOP */
    }
    const uint8_t *ptr = (const uint8_t *)buf;
    uint32_t sum = 0;

    while (len > 1) {
        sum += ((uint16_t)ptr[0] << 8) | ptr[1];
        ptr += 2;
        len -= 2;
    }

    if (len == 1) {
        sum += (uint16_t)ptr[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

size_t syn_eth_pack_header(uint8_t *buf, size_t buf_size, const uint8_t dst_mac[6],
                           const uint8_t src_mac[6], uint16_t ethertype)
{
    if (!buf || buf_size < SYN_ETH_HEADER_LEN || !dst_mac || !src_mac) {
        /* LCOV_EXCL_START: Invalid param guard */
        return 0;
        /* LCOV_EXCL_STOP */
    }

    memcpy(&buf[0], dst_mac, 6);
    memcpy(&buf[6], src_mac, 6);
    buf[12] = (uint8_t)(ethertype >> 8);
    buf[13] = (uint8_t)(ethertype & 0xFF);

    return SYN_ETH_HEADER_LEN;
}

size_t syn_ip_pack_header(uint8_t *buf, size_t buf_size, uint32_t src_ip, uint32_t dst_ip,
                          uint8_t proto, uint16_t payload_len, uint16_t id)
{
    if (!buf || buf_size < 20U) {
        /* LCOV_EXCL_START: Invalid param guard */
        return 0;
        /* LCOV_EXCL_STOP */
    }

    uint16_t total_ip_len = (uint16_t)(20U + payload_len);

    buf[0] = 0x45; /* Version 4, IHL 5 */
    buf[1] = 0x00; /* TOS */
    buf[2] = (uint8_t)(total_ip_len >> 8);
    buf[3] = (uint8_t)(total_ip_len & 0xFF);
    buf[4] = (uint8_t)(id >> 8);
    buf[5] = (uint8_t)(id & 0xFF);
    buf[6] = 0x40; /* Flags: Don't Fragment */
    buf[7] = 0x00; /* Fragment Offset */
    buf[8] = 64;   /* TTL = 64 */
    buf[9] = proto;
    buf[10] = 0; /* IP Checksum placeholder */
    buf[11] = 0;

    buf[12] = (uint8_t)(src_ip >> 24);
    buf[13] = (uint8_t)(src_ip >> 16);
    buf[14] = (uint8_t)(src_ip >> 8);
    buf[15] = (uint8_t)(src_ip);

    buf[16] = (uint8_t)(dst_ip >> 24);
    buf[17] = (uint8_t)(dst_ip >> 16);
    buf[18] = (uint8_t)(dst_ip >> 8);
    buf[19] = (uint8_t)(dst_ip);

    uint16_t csum = syn_ip_checksum(buf, 20);
    buf[10] = (uint8_t)(csum >> 8);
    buf[11] = (uint8_t)(csum & 0xFF);

    return 20U;
}
```


