

# File syn\_udp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_udp.c**](syn__udp_8c.md)

[Go to the documentation of this file](syn__udp_8c.md)


```C++

#include "syntropic/net/syn_udp.h"

#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

SYN_Status syn_udp_init(SYN_UDP *udp, SYN_ETH *eth)
{
    if (!udp) {
        return SYN_INVALID_PARAM;
    }
    memset(udp, 0, sizeof(*udp));
    udp->eth = eth;
    return SYN_OK;
}

SYN_UdpSocket *syn_udp_bind(SYN_UDP *udp, uint16_t local_port)
{
    if (!udp || local_port == 0) {
        return NULL;
    }

    /* Check if already bound */
    for (size_t i = 0; i < SYN_UDP_MAX_SOCKETS; i++) {
        if (udp->sockets[i].is_bound && udp->sockets[i].local_port == local_port) {
            return &udp->sockets[i];
        }
    }

    /* Allocate free slot */
    for (size_t i = 0; i < SYN_UDP_MAX_SOCKETS; i++) {
        if (!udp->sockets[i].is_bound) {
            memset(&udp->sockets[i], 0, sizeof(SYN_UdpSocket));
            udp->sockets[i].is_bound = true;
            udp->sockets[i].local_port = local_port;
            return &udp->sockets[i];
        }
    }

    return NULL;
}

void syn_udp_unbind(SYN_UdpSocket *sock)
{
    if (!sock) {
        return;
    }
    if (sock->blocked_task) {
        syn_task_resume(sock->blocked_task);
    }
    memset(sock, 0, sizeof(*sock));
}

uint16_t syn_udp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *udp_seg, size_t len)
{
    if (!udp_seg || len < 8) {
        return 0;
    }

    uint32_t sum = 0;

    /* Pseudo Header */
    sum += (src_ip >> 16) & 0xFFFF;
    sum += src_ip & 0xFFFF;
    sum += (dst_ip >> 16) & 0xFFFF;
    sum += dst_ip & 0xFFFF;
    sum += 17; /* Protocol UDP = 17 */
    sum += (uint16_t)len;

    /* UDP Header & Payload (checksum field at offset 6 zeroed during calc) */
    for (size_t i = 0; i < len; i += 2) {
        if (i == 6)
            continue; /* Skip checksum field */
        if (i + 1 < len) {
            sum += ((uint16_t)udp_seg[i] << 8) | udp_seg[i + 1];
        } else {
            sum += ((uint16_t)udp_seg[i] << 8);
        }
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    uint16_t csum = (uint16_t)(~sum);
    return (csum == 0) ? 0xFFFF : csum;
}

SYN_Status syn_udp_process_packet(SYN_UDP *udp, const uint8_t *ip_packet, size_t len)
{
    if (!udp || !ip_packet || len < 14 + 20 + 8) {
        return SYN_INVALID_PARAM;
    }

    /* Check EtherType IPv4 (0x0800) */
    uint16_t ethertype = syn_peek_u16(ip_packet, 12);
    if (ethertype != 0x0800) {
        return SYN_ERROR;
    }

    /* Check Protocol = 17 (UDP) */
    uint8_t proto = ip_packet[23];
    if (proto != 17) {
        return SYN_ERROR;
    }

    uint32_t src_ip = syn_peek_u32(ip_packet, 26);
    const uint8_t *udp_head = &ip_packet[34];
    uint16_t src_port = syn_peek_u16(udp_head, 0);
    uint16_t dst_port = syn_peek_u16(udp_head, 2);
    uint16_t udp_len = syn_peek_u16(udp_head, 4);

    if (udp_len < 8 || (size_t)(14 + 20 + udp_len) > len) {
        return SYN_ERROR;
    }

    size_t payload_len = udp_len - 8;
    const uint8_t *payload = &udp_head[8];

    /* Find matching socket by dst_port */
    for (size_t i = 0; i < SYN_UDP_MAX_SOCKETS; i++) {
        SYN_UdpSocket *sock = &udp->sockets[i];
        if (sock->is_bound && sock->local_port == dst_port) {
            sock->remote_ip = src_ip;
            sock->remote_port = src_port;

            if (payload_len > 0) {
                if (sock->rx_len + payload_len <= SYN_UDP_BUF_SIZE) {
                    memcpy(&sock->rx_buf[sock->rx_len], payload, payload_len);
                    sock->rx_len += (uint16_t)payload_len;
                }
            }

            /* Targeted wake: wake ONLY the task sleeping on this specific UDP port */
            if (sock->blocked_task) {
                syn_task_resume(sock->blocked_task);
            }

            return SYN_OK;
        }
    }

    return SYN_ERROR; /* No socket bound on dst_port */
}

int syn_udp_sendto(SYN_UDP *udp, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                   const uint8_t *data, size_t data_len, uint8_t *tx_out, size_t *tx_len)
{
    if (!udp || !tx_out || !tx_len) {
        return -1;
    }

    size_t total_len = 14 + 20 + 8 + data_len;
    if (total_len > SYN_ETH_MAX_FRAME_LEN) {
        return -1;
    }

    /* Ethernet Header (14) */
    uint8_t resolved_mac[6];
    bool has_mac = false;
    if (udp->eth && syn_eth_arp_lookup(udp->eth, dst_ip, resolved_mac) == SYN_OK) {
        has_mac = true;
    }
    static const uint8_t bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t *dst_mac = has_mac ? resolved_mac : bcast_mac;
    const uint8_t *src_mac = udp->eth ? udp->eth->mac_addr : bcast_mac;

    /* LCOV_EXCL_START: Unreachable parameter bounds check */
    if (syn_eth_pack_header(tx_out, total_len, dst_mac, src_mac, SYN_ETHTYPE_IPV4) == 0) {
        return -1;
    }

    /* IPv4 Header (20) */
    uint32_t src_ip = udp->eth ? udp->eth->ip_addr : 0;
    uint16_t udp_payload_len = (uint16_t)(8 + data_len);
    if (syn_ip_pack_header(&tx_out[14], total_len - 14, src_ip, dst_ip, 17, udp_payload_len,
                           0x1234) == 0) {
        return -1;
    }
    /* LCOV_EXCL_STOP */

    /* UDP Header (8) */
    syn_poke_u16(src_port, tx_out, 34);
    syn_poke_u16(dst_port, tx_out, 36);
    syn_poke_u16((uint16_t)(8 + data_len), tx_out, 38);
    syn_poke_u16(0x0000, tx_out, 40);

    if (data && data_len > 0) {
        memcpy(&tx_out[42], data, data_len);
    }

    uint16_t csum = syn_udp_checksum(src_ip, dst_ip, &tx_out[34], 8 + data_len);
    syn_poke_u16(csum, tx_out, 40);

    *tx_len = total_len;
    return (int)data_len;
    return (int)data_len;
}
```


