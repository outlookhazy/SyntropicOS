/**
 * @file syn_tcp.c
 * @brief Zero-Alloc Native TCP State Machine & Segment Engine Implementation.
 */

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
    if (!tcp || port == 0U) {
        return SYN_INVALID_PARAM;
    }

    /* Find first closed slot */
    for (size_t i = 0U; i < SYN_TCP_MAX_CONNS; i++) {
        if (tcp->conns[i].state == SYN_TCP_CLOSED) {
            tcp->listening = true;
            tcp->listen_port = port;
            tcp->listen_ports[i] = port;
            tcp->conns[i].state = SYN_TCP_LISTEN;
            tcp->conns[i].local_port = port;
            tcp->conns[i].local_ip = (tcp->eth != NULL) ? tcp->eth->ip_addr : 0U;
            return SYN_OK;
        }
    }
    return SYN_ERROR;
}

/** @brief Compact control-segment header parameters for 54-byte TCP frame serialization. */
typedef struct {
    uint8_t dst_mac[6]; /**< Destination Ethernet MAC address. */
    uint32_t src_ip;    /**< Source IPv4 address (host byte order). */
    uint32_t dst_ip;    /**< Destination IPv4 address (host byte order). */
    uint16_t src_port;  /**< Source TCP port. */
    uint16_t dst_port;  /**< Destination TCP port. */
    uint32_t seq_num;   /**< Sequence number. */
    uint32_t ack_num;   /**< Acknowledgment number. */
    uint16_t win;       /**< Advertised receive window. */
    uint8_t flags;      /**< TCP control flags bitmask. */
} SYN_TcpCtrlSpec;

/**
 * @brief Serialize a 54-byte 0-payload Ethernet II + IPv4 + TCP control segment.
 * @param tcp    Pointer to TCP stack.
 * @param spec   Control header specification.
 * @param tx_out Output Ethernet frame buffer (>= 54 bytes).
 * @param tx_len Output Ethernet frame length pointer.
 */
static void tcp_emit_ctrl_frame(const SYN_TCP *tcp, const SYN_TcpCtrlSpec *spec, uint8_t *tx_out,
                                size_t *tx_len)
{
    uint16_t total_ip_len = 40U;
    uint16_t total_frame_len = 54U;

    memcpy(&tx_out[0], spec->dst_mac, 6U);
    if (tcp->eth != NULL) {
        memcpy(&tx_out[6], tcp->eth->mac_addr, 6U);
    } else {
        memset(&tx_out[6], 0, 6U);
    }
    tx_out[12] = 0x08U;
    tx_out[13] = 0x00U;

    tx_out[14] = 0x45U;
    tx_out[15] = 0x00U;
    syn_poke_u16(total_ip_len, tx_out, 16U);
    syn_poke_u16(0x1234U, tx_out, 18U);
    syn_poke_u16(0x0000U, tx_out, 20U);
    tx_out[22] = 64U;
    tx_out[23] = 6U;
    tx_out[24] = 0U;
    tx_out[25] = 0U;
    syn_poke_u32(spec->src_ip, tx_out, 26U);
    syn_poke_u32(spec->dst_ip, tx_out, 30U);

    uint32_t ip_sum = 0U;
    for (int i = 14; i < 34; i += 2) {
        ip_sum += ((uint16_t)tx_out[i] << 8U) | tx_out[i + 1];
    }
    while ((ip_sum >> 16U) != 0U) {
        ip_sum = (ip_sum & 0xFFFFU) + (ip_sum >> 16U);
    }
    syn_poke_u16((uint16_t)(~ip_sum), tx_out, 24U);

    syn_poke_u16(spec->src_port, tx_out, 34U);
    syn_poke_u16(spec->dst_port, tx_out, 36U);
    syn_poke_u32(spec->seq_num, tx_out, 38U);
    syn_poke_u32(spec->ack_num, tx_out, 42U);
    tx_out[46] = 0x50U;
    tx_out[47] = spec->flags;
    syn_poke_u16(spec->win, tx_out, 48U);
    tx_out[50] = 0U;
    tx_out[51] = 0U;
    tx_out[52] = 0U;
    tx_out[53] = 0U;

    uint16_t tcp_csum = syn_tcp_checksum(spec->src_ip, spec->dst_ip, &tx_out[34], 20U);
    syn_poke_u16(tcp_csum, tx_out, 50U);

    *tx_len = total_frame_len;
}

/**
 * @brief Build outbound control TCP segment frame (0-byte payload) into buffer.
 * @param tcp    Pointer to TCP stack.
 * @param conn   Pointer to TCP connection block.
 * @param flags  TCP control flags bitmask.
 * @param tx_out Output Ethernet frame buffer.
 * @param tx_len Output Ethernet frame length pointer.
 */
static void build_tcp_segment(SYN_TCP *tcp, SYN_TcpConn *conn, uint8_t flags, uint8_t *tx_out,
                              size_t *tx_len)
{
    SYN_TcpCtrlSpec spec;
    memcpy(spec.dst_mac, conn->remote_mac, 6U);
    spec.src_ip = conn->local_ip;
    spec.dst_ip = conn->remote_ip;
    spec.src_port = conn->local_port;
    spec.dst_port = conn->remote_port;
    spec.seq_num = conn->seq_nxt;
    spec.ack_num = conn->ack_nxt;
    spec.win = (conn->rx_len < (uint16_t)SYN_TCP_BUF_SIZE)
                   ? (uint16_t)((uint16_t)SYN_TCP_BUF_SIZE - conn->rx_len)
                   : 0U;
    spec.flags = flags;
    tcp_emit_ctrl_frame(tcp, &spec, tx_out, tx_len);
}

/**
 * @brief Emit RFC 793 stateless RST reply for an unmatched inbound unicast non-RST segment.
 * @param tcp         Pointer to TCP stack.
 * @param frame       Inbound Ethernet II + IPv4 + TCP frame.
 * @param tcp_off     Byte offset of TCP header inside frame.
 * @param payload_len Validated TCP payload byte length.
 * @param tx_out      Output Ethernet frame buffer (>= 54 bytes).
 * @param tx_len      Output Ethernet frame length pointer.
 */
static void tcp_send_stateless_rst(const SYN_TCP *tcp, const uint8_t *frame, size_t tcp_off,
                                   size_t payload_len, uint8_t *tx_out, size_t *tx_len)
{
    uint8_t flags = frame[tcp_off + 13U];
    uint32_t dst_ip = syn_peek_u32(frame, 30U);
    if ((flags & SYN_TCP_FLAG_RST) != 0U || (frame[0] & 0x01U) != 0U || dst_ip == 0xFFFFFFFFUL ||
        (dst_ip & 0xF0000000UL) == 0xE0000000UL) {
        return;
    }

    SYN_TcpCtrlSpec spec;
    memcpy(spec.dst_mac, &frame[6], 6U);
    spec.src_ip = dst_ip;
    spec.dst_ip = syn_peek_u32(frame, 26U);
    spec.src_port = syn_peek_u16(frame, tcp_off + 2U);
    spec.dst_port = syn_peek_u16(frame, tcp_off);
    spec.win = 0U;
    if ((flags & SYN_TCP_FLAG_ACK) != 0U) {
        spec.seq_num = syn_peek_u32(frame, tcp_off + 8U);
        spec.ack_num = 0U;
        spec.flags = SYN_TCP_FLAG_RST;
    } else {
        uint32_t seg_len = (uint32_t)payload_len + (((flags & SYN_TCP_FLAG_SYN) != 0U) ? 1U : 0U) +
                           (((flags & SYN_TCP_FLAG_FIN) != 0U) ? 1U : 0U);
        spec.seq_num = 0U;
        spec.ack_num = syn_peek_u32(frame, tcp_off + 4U) + seg_len;
        spec.flags = (uint8_t)(SYN_TCP_FLAG_RST | SYN_TCP_FLAG_ACK);
    }
    tcp_emit_ctrl_frame(tcp, &spec, tx_out, tx_len);
}

/**
 * @brief Detach and resume a task blocked on a TCP connection, if any.
 * @param conn TCP connection pointer.
 */
static void tcp_wake_blocked(SYN_TcpConn *conn)
{
    if (conn->blocked_task != NULL) {
        SYN_Task *t = conn->blocked_task;
        conn->blocked_task = NULL;
        syn_task_resume(t);
    }
}

/**
 * @brief Check whether a local TCP port is registered as a passive listener on this stack.
 * @param tcp  TCP stack container.
 * @param port Local port number.
 * @return true if port is an active listener port.
 */
static bool tcp_is_listen_port(const SYN_TCP *tcp, uint16_t port)
{
    if (!tcp->listening || port == 0U) {
        return false;
    }
    if (port == tcp->listen_port) {
        return true;
    }
    for (size_t i = 0U; i < SYN_TCP_MAX_CONNS; i++) {
        if (tcp->listen_ports[i] == port) {
            return true;
        }
    }
    return false;
}

SYN_Status syn_tcp_close(SYN_TCP *tcp, SYN_TcpConn *conn)
{
    if (!tcp || !conn) {
        return SYN_INVALID_PARAM;
    }
    conn->rx_len = 0U;
    conn->tx_len = 0U;
    if (tcp_is_listen_port(tcp, conn->local_port)) {
        conn->state = SYN_TCP_LISTEN;
    } else {
        conn->state = SYN_TCP_CLOSED;
    }
    tcp_wake_blocked(conn);
    return SYN_OK;
}

/**
 * @brief Re-arm a drained closed or expired half-open listener slot on SYN.
 * @param tcp      TCP stack container with listening == true.
 * @param dst_port Matched passive listener destination port.
 * @return Re-armed connection slot, or NULL if no eligible slot exists.
 */
static SYN_TcpConn *tcp_rearm_closed_listener(SYN_TCP *tcp, uint16_t dst_port)
{
    uint32_t now = syn_port_get_tick_ms();
    for (size_t i = 0U; i < SYN_TCP_MAX_CONNS; i++) {
        SYN_TcpConn *c = &tcp->conns[i];
        bool reclaim_closed = (c->state == SYN_TCP_CLOSED && c->rx_len == 0U);
        bool reclaim_syn_rcvd =
            (c->state == SYN_TCP_SYN_RCVD && c->local_port == dst_port &&
             (int32_t)(now - c->syn_rcvd_ms) >= (int32_t)SYN_TCP_SYN_TIMEOUT_MS);
        if (reclaim_closed || reclaim_syn_rcvd) {
            c->state = SYN_TCP_LISTEN;
            c->local_port = dst_port;
            c->local_ip = (tcp->eth != NULL) ? tcp->eth->ip_addr : 0U;
            c->rx_len = 0U;
            c->tx_len = 0U;
            return c;
        }
    }
    return NULL;
}

/**
 * @brief Check whether TCP flags represent a pure connection-initiating SYN segment.
 *
 * @param flags TCP header flags byte.
 * @return true if SYN is set and ACK, FIN, and RST are clear.
 */
static bool tcp_is_pure_syn(uint8_t flags)
{
    const uint8_t mask =
        (uint8_t)(SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_ACK | SYN_TCP_FLAG_FIN | SYN_TCP_FLAG_RST);
    return (flags & mask) == SYN_TCP_FLAG_SYN;
}

/**
 * @brief Locate an active or listening TCP connection matching packet tuple.
 * @param tcp      TCP stack container.
 * @param src_ip   Peer IPv4 address.
 * @param src_port Peer TCP port.
 * @param dst_port Local destination TCP port.
 * @param flags    TCP segment flags bitmask.
 * @return Matching connection slot, or NULL if none.
 */
static SYN_TcpConn *tcp_find_conn(SYN_TCP *tcp, uint32_t src_ip, uint16_t src_port,
                                  uint16_t dst_port, uint8_t flags)
{
    for (size_t i = 0U; i < SYN_TCP_MAX_CONNS; i++) {
        SYN_TcpConn *c = &tcp->conns[i];
        if ((c->state == SYN_TCP_ESTABLISHED || c->state == SYN_TCP_SYN_RCVD) &&
            c->remote_ip == src_ip && c->remote_port == src_port && c->local_port == dst_port) {
            return c;
        }
    }
    for (size_t i = 0U; i < SYN_TCP_MAX_CONNS; i++) {
        if (tcp->conns[i].state == SYN_TCP_LISTEN && tcp->conns[i].local_port == dst_port) {
            return &tcp->conns[i];
        }
    }
    if (tcp_is_pure_syn(flags) && tcp_is_listen_port(tcp, dst_port)) {
        return tcp_rearm_closed_listener(tcp, dst_port);
    }
    return NULL;
}

/**
 * @brief Handle payload and/or FIN on an ESTABLISHED TCP connection.
 * @param tcp         TCP stack container.
 * @param conn        Established connection slot.
 * @param seq_num     Segment sequence number.
 * @param flags       TCP flags bitmask.
 * @param payload     Pointer to segment payload bytes.
 * @param payload_len Segment payload byte count.
 * @param tx_out      Outbound Ethernet frame buffer.
 * @param tx_len      [out] Outbound Ethernet frame length.
 */
static void tcp_handle_established(SYN_TCP *tcp, SYN_TcpConn *conn, uint32_t seq_num, uint8_t flags,
                                   const uint8_t *payload, size_t payload_len, uint8_t *tx_out,
                                   size_t *tx_len)
{
    if ((flags & SYN_TCP_FLAG_SYN) != 0U) {
        build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK, tx_out, tx_len);
        return;
    }

    if (seq_num != conn->ack_nxt) {
        if (payload_len > 0U || (flags & SYN_TCP_FLAG_FIN) != 0U) {
            build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK, tx_out, tx_len);
        }
        return;
    }

    size_t accepted = 0U;
    if (payload_len > 0U) {
        if (conn->rx_len < SYN_TCP_BUF_SIZE) {
            size_t space = (size_t)SYN_TCP_BUF_SIZE - conn->rx_len;
            accepted = (payload_len <= space) ? payload_len : space;
            memcpy(&conn->rx_buf[conn->rx_len], payload, accepted);
            conn->rx_len += (uint16_t)accepted;
        }
        conn->ack_nxt = seq_num + (uint32_t)accepted;
    }

    if ((flags & SYN_TCP_FLAG_FIN) != 0U && accepted == payload_len) {
        conn->ack_nxt = seq_num + (uint32_t)accepted + 1U;
        conn->state = SYN_TCP_CLOSED;
        build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK | SYN_TCP_FLAG_FIN, tx_out, tx_len);
        tcp_wake_blocked(conn);
        return;
    }

    if (payload_len > 0U) {
        build_tcp_segment(tcp, conn, SYN_TCP_FLAG_ACK, tx_out, tx_len);
        tcp_wake_blocked(conn);
    }
}

/**
 * @brief Validate Ethernet/IPv4/TCP header lengths and compute segment offsets.
 * @param frame           Raw Ethernet frame bytes.
 * @param len             Ethernet frame byte length (>= 54).
 * @param tcp_off_out     [out] Byte offset of TCP header inside frame.
 * @param data_off_out    [out] TCP header length in bytes.
 * @param payload_len_out [out] TCP payload byte count.
 * @return SYN_OK if valid TCP-over-IPv4 frame, SYN_ERROR otherwise.
 */
static SYN_Status tcp_validate_frame(const uint8_t *frame, size_t len, size_t *tcp_off_out,
                                     uint8_t *data_off_out, size_t *payload_len_out)
{
    uint8_t ip_ihl = (uint8_t)((frame[14] & 0x0FU) * 4U);
    if (ip_ihl < 20U || (14U + (size_t)ip_ihl + 20U) > len || frame[23] != 6U) {
        return SYN_ERROR;
    }

    size_t tcp_off = 14U + (size_t)ip_ihl;
    uint16_t ip_len = syn_peek_u16(frame, 16U);
    uint8_t data_offset = (uint8_t)((frame[tcp_off + 12U] >> 4U) * 4U);
    if (data_offset < 20U || ip_len < (uint16_t)ip_ihl + (uint16_t)data_offset ||
        (14U + (size_t)ip_len) > len) {
        return SYN_ERROR;
    }

    uint32_t src_ip = syn_peek_u32(frame, 26U);
    uint32_t dst_ip = syn_peek_u32(frame, 30U);
    if (syn_tcp_checksum(src_ip, dst_ip, &frame[tcp_off], (size_t)ip_len - (size_t)ip_ihl) != 0U) {
        return SYN_ERROR;
    }

    *tcp_off_out = tcp_off;
    *data_off_out = data_offset;
    *payload_len_out = (size_t)ip_len - (size_t)ip_ihl - (size_t)data_offset;
    return SYN_OK;
}

/**
 * @brief Process an inbound segment on a connection in SYN_TCP_SYN_RCVD state.
 *
 * @param tcp         TCP stack container.
 * @param conn        Connection slot in SYN_TCP_SYN_RCVD.
 * @param frame       Raw Ethernet frame bytes.
 * @param tcp_off     Byte offset of TCP header inside frame.
 * @param seq_num     Segment sequence number.
 * @param ack_num     Segment acknowledgment number.
 * @param flags       TCP flags bitmask.
 * @param payload     Pointer to segment payload bytes.
 * @param payload_len Segment payload byte count.
 * @param tx_out      Outbound Ethernet frame buffer.
 * @param tx_len      [out] Outbound Ethernet frame length.
 */
static void tcp_handle_syn_rcvd(SYN_TCP *tcp, SYN_TcpConn *conn, const uint8_t *frame,
                                size_t tcp_off, uint32_t seq_num, uint32_t ack_num, uint8_t flags,
                                const uint8_t *payload, size_t payload_len, uint8_t *tx_out,
                                size_t *tx_len)
{
    if (tcp_is_pure_syn(flags) && seq_num == (conn->ack_nxt - 1U)) {
        conn->seq_nxt--;
        build_tcp_segment(tcp, conn, SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_ACK, tx_out, tx_len);
        conn->seq_nxt++;
        return;
    }

    if ((flags & (uint8_t)(SYN_TCP_FLAG_ACK | SYN_TCP_FLAG_SYN)) == SYN_TCP_FLAG_ACK) {
        if (ack_num != conn->seq_nxt) {
            tcp_send_stateless_rst(tcp, frame, tcp_off, payload_len, tx_out, tx_len);
            return;
        }
        if (seq_num == conn->ack_nxt) {
            conn->state = SYN_TCP_ESTABLISHED;
            if (payload_len > 0U || (flags & SYN_TCP_FLAG_FIN) != 0U) {
                tcp_handle_established(tcp, conn, seq_num, flags, payload, payload_len, tx_out,
                                       tx_len);
            }
        }
    }
}

/**
 * @brief Dispatch a validated TCP segment to a matched connection state machine.
 * @param tcp         TCP stack container.
 * @param conn        Matched connection slot.
 * @param frame       Raw Ethernet frame bytes.
 * @param tcp_off     Byte offset of TCP header inside frame.
 * @param data_offset TCP header byte length.
 * @param payload_len TCP payload byte count.
 * @param tx_out      Outbound Ethernet frame buffer.
 * @param tx_len      [out] Outbound Ethernet frame length.
 */
static void tcp_dispatch_segment(SYN_TCP *tcp, SYN_TcpConn *conn, const uint8_t *frame,
                                 size_t tcp_off, uint8_t data_offset, size_t payload_len,
                                 uint8_t *tx_out, size_t *tx_len)
{
    uint32_t src_ip = syn_peek_u32(frame, 26U);
    uint16_t src_port = syn_peek_u16(frame, tcp_off);
    uint32_t seq_num = syn_peek_u32(frame, tcp_off + 4U);
    uint32_t ack_num = syn_peek_u32(frame, tcp_off + 8U);
    uint8_t flags = frame[tcp_off + 13U];
    const uint8_t *payload = &frame[tcp_off + (size_t)data_offset];

    if ((flags & SYN_TCP_FLAG_RST) != 0U) {
        if (conn->state != SYN_TCP_LISTEN && seq_num == conn->ack_nxt) {
            conn->rx_len = 0U;
            conn->tx_len = 0U;
            conn->state = SYN_TCP_CLOSED;
            tcp_wake_blocked(conn);
        }
        return;
    }

    if (conn->state == SYN_TCP_LISTEN) {
        if ((flags & SYN_TCP_FLAG_ACK) != 0U) {
            tcp_send_stateless_rst(tcp, frame, tcp_off, payload_len, tx_out, tx_len);
        } else if (tcp_is_pure_syn(flags)) {
            uint32_t now = syn_port_get_tick_ms();
            conn->state = SYN_TCP_SYN_RCVD;
            conn->syn_rcvd_ms = now;
            conn->remote_ip = src_ip;
            conn->remote_port = src_port;
            memcpy(conn->remote_mac, &frame[6], 6U);
            conn->ack_nxt = seq_num + 1U;
            conn->seq_nxt = now;
            build_tcp_segment(tcp, conn, SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_ACK, tx_out, tx_len);
            conn->seq_nxt++;
        }
    } else if (conn->state == SYN_TCP_SYN_RCVD) {
        tcp_handle_syn_rcvd(tcp, conn, frame, tcp_off, seq_num, ack_num, flags, payload,
                            payload_len, tx_out, tx_len);
    } else {
        tcp_handle_established(tcp, conn, seq_num, flags, payload, payload_len, tx_out, tx_len);
    }
}

SYN_Status syn_tcp_process_packet(SYN_TCP *tcp, const uint8_t *frame, size_t len, uint8_t *tx_out,
                                  size_t *tx_len)
{
    if (!tcp || !frame || len < 54U || !tx_out || !tx_len) {
        return SYN_INVALID_PARAM;
    }

    *tx_len = 0U;

    size_t tcp_off = 0U;
    uint8_t data_offset = 0U;
    size_t payload_len = 0U;
    if (tcp_validate_frame(frame, len, &tcp_off, &data_offset, &payload_len) != SYN_OK) {
        return SYN_ERROR;
    }

    uint32_t src_ip = syn_peek_u32(frame, 26U);
    uint16_t src_port = syn_peek_u16(frame, tcp_off);
    uint16_t dst_port = syn_peek_u16(frame, tcp_off + 2U);
    uint8_t flags = frame[tcp_off + 13U];

    SYN_TcpConn *conn = tcp_find_conn(tcp, src_ip, src_port, dst_port, flags);
    if (conn == NULL) {
        tcp_send_stateless_rst(tcp, frame, tcp_off, payload_len, tx_out, tx_len);
        return SYN_ERROR;
    }

    tcp_dispatch_segment(tcp, conn, frame, tcp_off, data_offset, payload_len, tx_out, tx_len);
    return SYN_OK;
}
