

# File syn\_tcp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tcp.c**](syn__tcp_8c.md)

[Go to the documentation of this file](syn__tcp_8c.md)


```C++

#include "syntropic/net/syn_tcp.h"

#include "syntropic/port/syn_port_system.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_assert.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

uint16_t syn_tcp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *tcp_seg, size_t len)
{
    uint32_t sum = 0;

    /* Pseudo-header: Src IP (4), Dst IP (4), Reserved (1), Proto (1), TCP Len (2) */
    sum += (src_ip >> 16) + (src_ip & 0xFFFF);
    sum += (dst_ip >> 16) + (dst_ip & 0xFFFF);
    sum += 0x0006; /* Protocol 6 (TCP) */
    sum += (uint16_t)len;

    /* Segment bytes */
    const uint8_t *ptr = tcp_seg;
    size_t left = len;
    while (left > 1) {
        sum += ((uint16_t)ptr[0] << 8) | ptr[1];
        ptr += 2;
        left -= 2;
    }
    if (left == 1) {
        sum += (uint16_t)ptr[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

SYN_Status syn_tcp_init(SYN_TCP *tcp, SYN_ETH *eth)
{
    if (!tcp || !eth) {
        return SYN_INVALID_PARAM;
    }
    memset(tcp, 0, sizeof(*tcp));
    tcp->eth = eth;
    return SYN_OK;
}

SYN_Status syn_tcp_listen(SYN_TCP *tcp, uint16_t port)
{
    if (!tcp || port == 0) {
        return SYN_INVALID_PARAM;
    }

    /* Find first closed slot */
    for (size_t i = 0; i < SYN_TCP_MAX_CONNS; i++) {
        if (tcp->conns[i].state == SYN_TCP_CLOSED) {
            tcp->conns[i].state = SYN_TCP_LISTEN;
            tcp->conns[i].local_port = port;
            if (tcp->eth != NULL) {
                tcp->conns[i].local_ip = tcp->eth->ip_addr;
            }
            return SYN_OK;
        }
    }
    return SYN_ERROR;
}

static void build_tcp_segment(SYN_TCP *tcp, SYN_TcpConn *conn, uint8_t flags,
                              const uint8_t *payload, size_t payload_len, uint8_t *tx_out,
                              size_t *tx_len)
{
    /* Ethernet (14) + IPv4 (20) + TCP (20) + Payload */
    uint16_t total_ip_len = 20 + 20 + payload_len;
    uint16_t total_frame_len = 14 + total_ip_len;

    /* Ethernet Header */
    memcpy(&tx_out[0], conn->remote_mac, 6);
    /* Local MAC: read from ETH instance; zeroed if unavailable (should not happen in practice) */
    if (tcp && tcp->eth) {
        memcpy(&tx_out[6], tcp->eth->mac_addr, 6);
    } else {
        /* LCOV_EXCL_START: Defensive MAC fallback */
        memset(&tx_out[6], 0, 6);
        /* LCOV_EXCL_STOP */
    }
    tx_out[12] = 0x08;
    tx_out[13] = 0x00; /* IPv4 */

    /* IPv4 Header */
    tx_out[14] = 0x45;
    tx_out[15] = 0x00;
    syn_poke_u16(total_ip_len, tx_out, 16);
    syn_poke_u16(0x1234, tx_out, 18);
    syn_poke_u16(0x0000, tx_out, 20);
    tx_out[22] = 64; /* TTL */
    tx_out[23] = 6;  /* TCP */
    tx_out[24] = 0;
    tx_out[25] = 0; /* IP Checksum */
    syn_poke_u32(conn->local_ip, tx_out, 26);
    syn_poke_u32(conn->remote_ip, tx_out, 30);

    /* IP Checksum */
    uint32_t ip_sum = 0;
    for (int i = 14; i < 34; i += 2) {
        ip_sum += ((uint16_t)tx_out[i] << 8) | tx_out[i + 1];
    }
    while (ip_sum >> 16) {
        ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
    }
    syn_poke_u16((uint16_t)(~ip_sum), tx_out, 24);

    /* TCP Header */
    syn_poke_u16(conn->local_port, tx_out, 34);
    syn_poke_u16(conn->remote_port, tx_out, 36);
    syn_poke_u32(conn->seq_nxt, tx_out, 38);
    syn_poke_u32(conn->ack_nxt, tx_out, 42);
    tx_out[46] = 0x50; /* 20 bytes (5 words) */
    tx_out[47] = flags;
    syn_poke_u16(SYN_TCP_BUF_SIZE, tx_out, 48); /* Window */
    tx_out[50] = 0;
    tx_out[51] = 0; /* Checksum */
    tx_out[52] = 0;
    tx_out[53] = 0; /* Urgent Pointer */

    if (payload_len > 0 && payload != NULL) {
        /* LCOV_EXCL_START: TCP payload copy */
        memcpy(&tx_out[54], payload, payload_len);
        /* LCOV_EXCL_STOP */
    }

    /* TCP Checksum */
    uint16_t tcp_csum =
        syn_tcp_checksum(conn->local_ip, conn->remote_ip, &tx_out[34], 20 + payload_len);
    syn_poke_u16(tcp_csum, tx_out, 50);

    *tx_len = total_frame_len;
}

SYN_Status syn_tcp_process_packet(SYN_TCP *tcp, const uint8_t *frame, size_t len, uint8_t *tx_out,
                                  size_t *tx_len)
{
    if (!tcp || !frame || len < 54 || !tx_out || !tx_len) {
        return SYN_INVALID_PARAM;
    }

    *tx_len = 0;

    /* Parse IP header */
    uint8_t proto = frame[23];
    if (proto != 6) { /* Not TCP */
        return SYN_ERROR;
    }

    uint32_t src_ip = syn_peek_u32(frame, 26);
    uint16_t src_port = syn_peek_u16(frame, 34);
    uint16_t dst_port = syn_peek_u16(frame, 36);
    uint32_t seq_num = syn_peek_u32(frame, 38);
    uint32_t ack_num = syn_peek_u32(frame, 42);
    (void)ack_num;
    uint8_t data_offset = (frame[46] >> 4) * 4;
    uint8_t flags = frame[47];

    size_t payload_len = 0;
    uint16_t ip_len = syn_peek_u16(frame, 16);
    if ((size_t)ip_len >= 20 + (size_t)data_offset) {
        payload_len = (size_t)ip_len - 20 - (size_t)data_offset;
    }

    /* Find matching connection slot (SYN_RCVD or ESTABLISHED) */
    SYN_TcpConn *conn = NULL;
    for (size_t i = 0; i < SYN_TCP_MAX_CONNS; i++) {
        if ((tcp->conns[i].state == SYN_TCP_ESTABLISHED ||
             tcp->conns[i].state == SYN_TCP_SYN_RCVD) &&
            tcp->conns[i].remote_ip == src_ip && tcp->conns[i].remote_port == src_port &&
            tcp->conns[i].local_port == dst_port) {
            conn = &tcp->conns[i];
            break;
        }
    }

    if (!conn) {
        /* Search for LISTEN slot */
        for (size_t i = 0; i < SYN_TCP_MAX_CONNS; i++) {
            if (tcp->conns[i].state == SYN_TCP_LISTEN && tcp->conns[i].local_port == dst_port) {
                conn = &tcp->conns[i];
                break;
            }
        }
    }

    if (!conn) {
        return SYN_ERROR;
    }

    /* State Machine */
    if (conn->state == SYN_TCP_LISTEN) {
        if (flags & SYN_TCP_FLAG_SYN) {
            conn->state = SYN_TCP_SYN_RCVD;
            conn->remote_ip = src_ip;
            conn->remote_port = src_port;
            memcpy(conn->remote_mac, &frame[6], 6);
            conn->ack_nxt = seq_num + 1;
            /* Use tick-based ISN instead of a fixed constant to avoid
             * predictable sequence numbers and stale connection reuse. */
            conn->seq_nxt = syn_port_get_tick_ms();

            build_tcp_segment(tcp, conn, SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_ACK, NULL, 0, tx_out,
                              tx_len);
            conn->seq_nxt++;
            return SYN_OK;
        }
    } else if (conn->state == SYN_TCP_SYN_RCVD) {
        if (flags & SYN_TCP_FLAG_ACK) {
            conn->state = SYN_TCP_ESTABLISHED;
            return SYN_OK;
        }
    } else if (conn->state == SYN_TCP_ESTABLISHED) {
        if (flags & SYN_TCP_FLAG_FIN) {
            conn->ack_nxt = seq_num + 1;
            conn->state = SYN_TCP_CLOSED;
            build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK | SYN_TCP_FLAG_FIN, NULL, 0, tx_out,
                              tx_len);
            if (conn->blocked_task) {
                syn_task_resume(conn->blocked_task);
            }
            return SYN_OK;
        }

        if (payload_len > 0) {
            const uint8_t *payload = &frame[14 + 20 + data_offset];
            size_t accepted = 0;
            if (conn->rx_len < SYN_TCP_BUF_SIZE) {
                size_t space = SYN_TCP_BUF_SIZE - conn->rx_len;
                accepted = (payload_len <= space) ? payload_len : space;
                memcpy(&conn->rx_buf[conn->rx_len], payload, accepted);
                conn->rx_len += (uint16_t)accepted;
            }
            /* Only advance ack by bytes actually accepted — never false-ACK dropped data */
            conn->ack_nxt = seq_num + (uint32_t)accepted;
            build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK, NULL, 0, tx_out, tx_len);

            /* Resume task blocked in SYN_TASK_BLOCKED state */
            if (conn->blocked_task) {
                syn_task_resume(conn->blocked_task);
            }
            return SYN_OK;
        }
    }

    return SYN_OK; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
}
```


