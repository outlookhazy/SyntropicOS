

# File syn\_udp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_udp.c**](syn__udp_8c.md)

[Go to the documentation of this file](syn__udp_8c.md)


```C++


#include "syntropic/net/syn_udp.h"

#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

#define SYN_UDP_EPHEMERAL_MIN 49152U

static bool udp_port_in_use(const SYN_UDP *udp, uint16_t port)
{
    for (size_t i = 0U; i < SYN_UDP_MAX_SOCKETS; i++) {
        if (udp->sockets[i].is_bound && udp->sockets[i].local_port == port) {
            return true;
        }
    }
    return false;
}

static uint16_t udp_alloc_ephemeral_port(SYN_UDP *udp)
{
    if (udp->next_ephemeral_port < SYN_UDP_EPHEMERAL_MIN) {
        udp->next_ephemeral_port = SYN_UDP_EPHEMERAL_MIN;
    }
    uint16_t candidate = udp->next_ephemeral_port;
    while (udp_port_in_use(udp, candidate)) {
        candidate = (candidate == 65535U) ? SYN_UDP_EPHEMERAL_MIN : (uint16_t)(candidate + 1U);
    }
    udp->next_ephemeral_port =
        (candidate == 65535U) ? SYN_UDP_EPHEMERAL_MIN : (uint16_t)(candidate + 1U);
    return candidate;
}

SYN_Status syn_udp_init(SYN_UDP *udp, SYN_ETH *eth)
{
    if (!udp) {
        return SYN_INVALID_PARAM;
    }
    memset(udp, 0, sizeof(*udp));
    udp->eth = eth;
    udp->next_ephemeral_port = SYN_UDP_EPHEMERAL_MIN;
    return SYN_OK;
}

SYN_UdpSocket *syn_udp_bind(SYN_UDP *udp, uint16_t local_port)
{
    if (!udp) {
        return NULL;
    }

    if (local_port != 0U) {
        for (size_t i = 0U; i < SYN_UDP_MAX_SOCKETS; i++) {
            if (udp->sockets[i].is_bound && udp->sockets[i].local_port == local_port) {
                return &udp->sockets[i];
            }
        }
    }

    for (size_t i = 0U; i < SYN_UDP_MAX_SOCKETS; i++) {
        if (!udp->sockets[i].is_bound) {
            uint16_t assigned = (local_port == 0U) ? udp_alloc_ephemeral_port(udp) : local_port;
            memset(&udp->sockets[i], 0, sizeof(SYN_UdpSocket));
            udp->sockets[i].is_bound = true;
            udp->sockets[i].local_port = assigned;
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
    SYN_Task *t = sock->blocked_task;
    sock->blocked_task = NULL;
    sock->is_bound = false;
    memset(sock, 0, sizeof(*sock));
    if (t != NULL) {
        syn_task_resume(t);
    }
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

static void udp_sync_head_view(SYN_UdpSocket *sock)
{
    if (sock->dgram_count > 0U) {
        const SYN_UdpDatagram *head = &sock->dgrams[sock->dgram_head];
        sock->rx_len = head->len;
        sock->remote_ip = head->src_ip;
        sock->remote_port = head->src_port;
    } else {
        sock->rx_len = 0U;
    }
}

static void udp_sync_legacy_drain(SYN_UdpSocket *sock)
{
    if (sock->dgram_count == 0U) {
        return;
    }
    uint16_t head_len = sock->dgrams[sock->dgram_head].len;
    if (sock->rx_len == 0U && head_len > 0U) {
        size_t rem = (size_t)sock->rx_total_len - (size_t)head_len;
        if (rem > 0U) {
            memmove(sock->rx_buf, &sock->rx_buf[head_len], rem);
        }
        sock->rx_total_len = (uint16_t)rem;
        sock->dgram_head = (uint8_t)((sock->dgram_head + 1U) % SYN_UDP_MAX_DGRAMS);
        sock->dgram_count--;
        udp_sync_head_view(sock);
    }
}

SYN_Status syn_udp_recv(SYN_UdpSocket *sock, void *buf, size_t max_len, size_t *bytes_read,
                        uint32_t *src_ip, uint16_t *src_port)
{
    if (!sock || !buf || !bytes_read) {
        return SYN_INVALID_PARAM;
    }

    udp_sync_legacy_drain(sock);
    if (sock->dgram_count == 0U) {
        if (sock->rx_len > 0U) {
            size_t copy_len = ((size_t)sock->rx_len < max_len) ? (size_t)sock->rx_len : max_len;
            if (copy_len > 0U) {
                memcpy(buf, sock->rx_buf, copy_len);
            }
            sock->rx_len = 0U;
            sock->rx_total_len = 0U;
            *bytes_read = copy_len;
            if (src_ip != NULL) {
                *src_ip = sock->remote_ip;
            }
            if (src_port != NULL) {
                *src_port = sock->remote_port;
            }
            return SYN_OK;
        }
        *bytes_read = 0U;
        return SYN_OK;
    }

    SYN_UdpDatagram d = sock->dgrams[sock->dgram_head];
    size_t copy_len = ((size_t)d.len < max_len) ? (size_t)d.len : max_len;
    if (copy_len > 0U) {
        memcpy(buf, sock->rx_buf, copy_len);
    }

    size_t rem_total = (size_t)sock->rx_total_len - (size_t)d.len;
    if (rem_total > 0U) {
        memmove(sock->rx_buf, &sock->rx_buf[d.len], rem_total);
    }
    sock->rx_total_len = (uint16_t)rem_total;
    sock->dgram_head = (uint8_t)((sock->dgram_head + 1U) % SYN_UDP_MAX_DGRAMS);
    sock->dgram_count--;
    udp_sync_head_view(sock);

    *bytes_read = copy_len;
    if (src_ip != NULL) {
        *src_ip = d.src_ip;
    }
    if (src_port != NULL) {
        *src_port = d.src_port;
    }
    return SYN_OK;
}

static void udp_deliver_to_socket(SYN_UdpSocket *sock, uint32_t src_ip, uint16_t src_port,
                                  const uint8_t *payload, size_t payload_len)
{
    udp_sync_legacy_drain(sock);
    size_t used =
        (sock->rx_total_len > sock->rx_len) ? (size_t)sock->rx_total_len : (size_t)sock->rx_len;

    if (sock->dgram_count < SYN_UDP_MAX_DGRAMS &&
        payload_len <= ((size_t)SYN_UDP_BUF_SIZE - used)) {
        if (payload_len > 0U) {
            memcpy(&sock->rx_buf[used], payload, payload_len);
        }
        sock->rx_total_len = (uint16_t)(used + payload_len);
        uint8_t tail = (uint8_t)((sock->dgram_head + sock->dgram_count) % SYN_UDP_MAX_DGRAMS);
        sock->dgrams[tail].src_ip = src_ip;
        sock->dgrams[tail].src_port = src_port;
        sock->dgrams[tail].len = (uint16_t)payload_len;
        sock->dgram_count++;
        if (sock->dgram_count == 1U) {
            sock->rx_len = (uint16_t)payload_len;
            sock->remote_ip = src_ip;
            sock->remote_port = src_port;
        }
        if (sock->blocked_task != NULL) {
            SYN_Task *t = sock->blocked_task;
            sock->blocked_task = NULL;
            syn_task_resume(t);
        }
    }
}

static bool udp_rx_header_valid(const uint8_t *ip_packet, size_t len, size_t *udp_off_out,
                                uint16_t *udp_len_out)
{
    uint16_t ethertype = syn_peek_u16(ip_packet, 12U);
    size_t ihl = (size_t)(ip_packet[14] & 0x0FU) * 4U;
    if (ethertype != 0x0800U || ihl < 20U || (14U + ihl + 8U) > len || ip_packet[23] != 17U) {
        return false;
    }

    uint16_t ip_total_len = syn_peek_u16(ip_packet, 16U);
    if (ip_total_len < (uint16_t)(ihl + 8U) || (14U + (size_t)ip_total_len) > len) {
        return false;
    }

    size_t udp_off = 14U + ihl;
    const uint8_t *udp_head = &ip_packet[udp_off];
    uint16_t udp_len = syn_peek_u16(udp_head, 4U);
    if (udp_len < 8U || (size_t)udp_len > ((size_t)ip_total_len - ihl)) {
        return false;
    }

    uint16_t rx_csum = syn_peek_u16(udp_head, 6U);
    if (rx_csum != 0U) {
        uint32_t src_ip = syn_peek_u32(ip_packet, 26U);
        uint32_t dst_ip = syn_peek_u32(ip_packet, 30U);
        if (syn_udp_checksum(src_ip, dst_ip, udp_head, udp_len) != rx_csum) {
            return false;
        }
    }

    *udp_off_out = udp_off;
    *udp_len_out = udp_len;
    return true;
}

static void udp_resolve_dst_mac(SYN_ETH *eth, uint32_t dst_ip, uint8_t mac_out[6])
{
    if ((dst_ip & 0xF0000000UL) == 0xE0000000UL) {
        uint32_t low23 = dst_ip & 0x007FFFFFUL;
        mac_out[0] = 0x01U;
        mac_out[1] = 0x00U;
        mac_out[2] = 0x5EU;
        mac_out[3] = (uint8_t)(low23 >> 16U);
        mac_out[4] = (uint8_t)(low23 >> 8U);
        mac_out[5] = (uint8_t)(low23);
        return;
    }

    if (eth != NULL && dst_ip != 0xFFFFFFFFUL) {
        uint32_t next_hop = dst_ip;
        if (eth->netmask != 0U && eth->gateway != 0U &&
            ((dst_ip & eth->netmask) != (eth->ip_addr & eth->netmask))) {
            next_hop = eth->gateway;
        }
        if (syn_eth_arp_lookup(eth, next_hop, mac_out) == SYN_OK) {
            return;
        }
    }

    memset(mac_out, 0xFF, 6U);
}

SYN_Status syn_udp_process_packet(SYN_UDP *udp, const uint8_t *ip_packet, size_t len)
{
    if (udp == NULL || ip_packet == NULL || len < (14U + 20U + 8U)) {
        return SYN_INVALID_PARAM;
    }

    size_t udp_off = 0U;
    uint16_t udp_len = 0U;
    if (!udp_rx_header_valid(ip_packet, len, &udp_off, &udp_len)) {
        return SYN_ERROR;
    }

    uint32_t src_ip = syn_peek_u32(ip_packet, 26U);
    const uint8_t *udp_head = &ip_packet[udp_off];
    uint16_t src_port = syn_peek_u16(udp_head, 0U);
    uint16_t dst_port = syn_peek_u16(udp_head, 2U);
    size_t payload_len = (size_t)udp_len - 8U;
    const uint8_t *payload = &udp_head[8];

    for (size_t i = 0U; i < SYN_UDP_MAX_SOCKETS; i++) {
        SYN_UdpSocket *sock = &udp->sockets[i];
        if (sock->is_bound && sock->local_port == dst_port) {
            udp_deliver_to_socket(sock, src_ip, src_port, payload, payload_len);
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

int syn_udp_sendto(SYN_UDP *udp, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                   const uint8_t *data, size_t data_len, uint8_t *tx_out, size_t *tx_len)
{
    if (udp == NULL || tx_out == NULL || tx_len == NULL || (data == NULL && data_len > 0U) ||
        data_len > (SYN_ETH_MAX_FRAME_LEN - 42U)) {
        return -1;
    }

    size_t total_len = 42U + data_len;

    /* Ethernet Header (14) */
    uint8_t dst_mac[6];
    udp_resolve_dst_mac(udp->eth, dst_ip, dst_mac);
    static const uint8_t bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t *src_mac = (udp->eth != NULL) ? udp->eth->mac_addr : bcast_mac;

    (void)syn_eth_pack_header(tx_out, total_len, dst_mac, src_mac, SYN_ETHTYPE_IPV4);

    /* IPv4 Header (20) */
    uint32_t src_ip = (udp->eth != NULL) ? udp->eth->ip_addr : 0U;
    uint16_t udp_payload_len = (uint16_t)(8U + data_len);
    (void)syn_ip_pack_header(&tx_out[14], total_len - 14U, src_ip, dst_ip, 17U, udp_payload_len,
                             0x1234U);

    /* UDP Header (8) */
    syn_poke_u16(src_port, tx_out, 34U);
    syn_poke_u16(dst_port, tx_out, 36U);
    syn_poke_u16(udp_payload_len, tx_out, 38U);
    syn_poke_u16(0x0000U, tx_out, 40U);

    if (data != NULL && data_len > 0U) {
        memcpy(&tx_out[42], data, data_len);
    }

    uint16_t csum = syn_udp_checksum(src_ip, dst_ip, &tx_out[34], 8U + data_len);
    syn_poke_u16(csum, tx_out, 40U);

    *tx_len = total_len;
    return (int)data_len;
}
```


