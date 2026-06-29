#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_TRANSPORT_TCP) || SYN_USE_TRANSPORT_TCP

/**
 * @file syn_transport_tcp.c
 * @brief Bridge for syn_transport over a TCP socket.
 */

#include "syn_transport_tcp.h"
#include "../util/syn_assert.h"
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

    if (tcp->sock == SYN_SOCKET_INVALID) return false;

    uint8_t len_hdr[2];
    len_hdr[0] = (uint8_t)(len >> 8);
    len_hdr[1] = (uint8_t)(len & 0xFF);

    if (syn_port_sock_send_all(tcp->sock, len_hdr, 2) != 2) {
        return false;
    }

    if (len > 0 && data != NULL) {
        if (syn_port_sock_send_all(tcp->sock, data, len) != (int)len) {
            return false;
        }
    }

    return true;
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

    if (tcp->sock == SYN_SOCKET_INVALID) return false;

    /* Poll in a loop until we either run out of available data on socket or finish a packet */
    for (;;) {
        if (tcp->state == 0) {
            uint8_t b;
            int n = syn_port_sock_recv(tcp->sock, &b, 1, 0); /* non-blocking */
            if (n <= 0) return false; /* no data, timeout, or closed */
            tcp->payload_len = (uint16_t)((uint16_t)b << 8);
            tcp->state = 1;
        }
        else if (tcp->state == 1) {
            uint8_t b;
            int n = syn_port_sock_recv(tcp->sock, &b, 1, 0);
            if (n <= 0) return false;
            tcp->payload_len |= b;
            tcp->bytes_read = 0;
            tcp->state = 2;

            if (tcp->payload_len > sizeof(tcp->rx_buf)) {
                /* Framing error: packet too large for transport buffer */
                tcp->state = 0;
                return false;
            }
        }
        else if (tcp->state == 2) {
            uint16_t remaining = tcp->payload_len - tcp->bytes_read;
            if (remaining == 0) {
                /* Empty packet */
                *out_len = 0;
                tcp->state = 0;
                return true;
            }

            int n = syn_port_sock_recv(tcp->sock, tcp->rx_buf + tcp->bytes_read, remaining, 0);
            if (n <= 0) return false;

            tcp->bytes_read += (uint16_t)n;
            if (tcp->bytes_read == tcp->payload_len) {
                if (tcp->payload_len <= max_len) {
                    memcpy(data, tcp->rx_buf, tcp->payload_len);
                    *out_len = tcp->payload_len;
                    tcp->state = 0;
                    return true;
                } else {
                    /* Output buffer too small, drop it */
                    tcp->state = 0;
                    return false;
                }
            }
        }
    }
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
    t->ctx  = tcp;
}

#endif /* SYN_USE_TRANSPORT_TCP */
