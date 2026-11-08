#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TRANSPORT_TCP) || SYN_USE_TRANSPORT_TCP

/**
 * @file syn_transport_tcp.c
 * @brief Bridge for syn_transport over a TCP socket.
 */

#include "../util/syn_assert.h"
#include "syn_transport_tcp.h"

#include <string.h>

/**
 * @brief TCP transport send callback — length-prefixed framing.
 * @param data  Packet data.
 * @param len   Packet length.
 * @param ctx   Transport context (SYN_TransportTcp *).
 * @return true if sent.
 */
static bool tcp_send(const uint8_t *data, size_t len, void *ctx)
{
    const SYN_TransportTcp *tcp = (const SYN_TransportTcp *)ctx;
    SYN_ASSERT(tcp != NULL);
    SYN_ASSERT(len <= (size_t)0xFFFF);

    if (tcp->sock == SYN_SOCKET_INVALID || (len > 0U && data == NULL)) {
        return false;
    }

    uint8_t len_hdr[2];
    len_hdr[0] = (uint8_t)(len >> 8);
    len_hdr[1] = (uint8_t)(len & 0xFF);

    if (syn_port_sock_send_all(tcp->sock, len_hdr, 2) != 2) {
        return false;
    }

    if (len > 0U) {
        if (syn_port_sock_send_all(tcp->sock, data, len) != (int)len) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Read available bytes from TCP socket and close on EOF or readable error.
 * @param tcp Transport TCP context.
 * @param buf Destination buffer.
 * @param len Maximum bytes to read.
 * @return Bytes read (> 0), or <= 0 when incomplete or closed.
 */
static int tcp_read_chunk(SYN_TransportTcp *tcp, void *buf, size_t len)
{
    bool was_readable = syn_port_sock_readable(tcp->sock);
    int n = syn_port_sock_recv(tcp->sock, buf, len, 0);
    if (n == 0 || (n < 0 && was_readable)) {
        syn_port_sock_close(tcp->sock);
        tcp->sock = SYN_SOCKET_INVALID;
        tcp->state = 0;
    }
    return n;
}

/**
 * @brief TCP transport receive callback — length-prefixed framing.
 * @param data     [out] Buffer for received data.
 * @param max_len  Buffer capacity.
 * @param out_len  [out] Received length.
 * @param ctx      Transport context (SYN_TransportTcp *).
 * @return true if a complete packet was received.
 */
static bool tcp_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_TransportTcp *tcp = (SYN_TransportTcp *)ctx;
    SYN_ASSERT(tcp != NULL);
    SYN_ASSERT(data != NULL);
    SYN_ASSERT(out_len != NULL);

    if (tcp->sock == SYN_SOCKET_INVALID)
        return false;

    /* Poll in a loop until we either run out of available data on socket or finish a packet */
    for (;;) {
        if (tcp->state == 0) {
            uint8_t b;
            int n = tcp_read_chunk(tcp, &b, 1U);
            if (n <= 0)
                return false;
            tcp->payload_len = (uint16_t)((uint16_t)b << 8);
            tcp->state = 1;
        } else if (tcp->state == 1) {
            uint8_t b;
            int n = tcp_read_chunk(tcp, &b, 1U);
            if (n <= 0)
                return false;
            tcp->payload_len |= b;
            tcp->bytes_read = 0;

            if (tcp->payload_len == 0U) {
                *out_len = 0U;
                tcp->state = 0;
                return true;
            }
            if (tcp->payload_len > sizeof(tcp->rx_buf)) {
                /* Framing error: packet too large for transport buffer */
                syn_port_sock_close(tcp->sock);
                tcp->sock = SYN_SOCKET_INVALID;
                tcp->state = 0;
                return false;
            }
            tcp->state = 2;
        } else {
            uint16_t remaining = tcp->payload_len - tcp->bytes_read;
            int n = tcp_read_chunk(tcp, tcp->rx_buf + tcp->bytes_read, remaining);
            if (n <= 0)
                return false;

            tcp->bytes_read += (uint16_t)n;
            if (tcp->bytes_read == tcp->payload_len) {
                tcp->state = 0;
                if (tcp->payload_len <= max_len) {
                    memcpy(data, tcp->rx_buf, tcp->payload_len);
                    *out_len = tcp->payload_len;
                    return true;
                }
                return false;
            }
        }
    }
}

/**
 * @brief Check if underlying TCP socket currently has unread bytes or EOF/error pending.
 * @param ctx Pointer to SYN_TransportTcp instance.
 * @return True if socket is readable, false otherwise.
 */
static bool tcp_has_data(const void *ctx)
{
    const SYN_TransportTcp *tcp = (const SYN_TransportTcp *)ctx;
    return (tcp != NULL) && (tcp->sock != SYN_SOCKET_INVALID) && syn_port_sock_readable(tcp->sock);
}

void syn_transport_tcp_init(SYN_Transport *t, SYN_TransportTcp *tcp, SYN_Socket sock)
{
    SYN_ASSERT(t != NULL);
    SYN_ASSERT(tcp != NULL);

    memset(tcp, 0, sizeof(*tcp));
    tcp->sock = sock;
    tcp->state = 0;

    t->send = tcp_send;
    t->recv = tcp_recv;
    t->has_data = tcp_has_data;
    t->ctx = tcp;
}

#endif /* SYN_USE_TRANSPORT_TCP */
