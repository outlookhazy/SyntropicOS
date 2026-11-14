/**
 * @file syn_transport_udp.c
 * @brief Bridge implementation connecting syn_port_udp_* to native syn_udp stack.
 * @ingroup syn_net
 */

#include "syntropic/net/syn_transport_udp.h"

#if !defined(SYN_USE_TRANSPORT_UDP) || SYN_USE_TRANSPORT_UDP

#include "port/syn_port_eth.h"
#include "syntropic/util/syn_assert.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

/** Global native UDP stack pointer */
static SYN_UDP *g_native_udp = NULL;
/** Active Ethernet transmit function (defaults to syn_port_eth_tx) */
static SYN_TransportEthTxFn g_eth_tx_fn = syn_port_eth_tx;
/** Internal static transmission buffer */
static uint8_t g_udp_tx_buf[SYN_ETH_MAX_FRAME_LEN];

void syn_transport_udp_set_instance(SYN_UDP *udp)
{
    g_native_udp = udp;
}

void syn_transport_udp_set_eth_tx(SYN_TransportEthTxFn tx_fn)
{
    g_eth_tx_fn = (tx_fn != NULL) ? tx_fn : syn_port_eth_tx;
}

SYN_UDP *syn_transport_udp_get_instance(void)
{
    return g_native_udp;
}

SYN_Socket syn_transport_udp_open(uint16_t port)
{
    if (g_native_udp == NULL) {
        return SYN_SOCKET_INVALID;
    }
    SYN_UdpSocket *sock = syn_udp_bind(g_native_udp, port);
    if (sock == NULL) {
        return SYN_SOCKET_INVALID;
    }
    return (SYN_Socket)((sock - &g_native_udp->sockets[0]) + 100);
}

int syn_transport_udp_sendto(SYN_Socket sock_handle, const void *data, size_t len,
                             const SYN_SockAddr *to)
{
    if (g_native_udp == NULL || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS ||
        to == NULL) {
        return -1;
    }
    SYN_UdpSocket *sock = &g_native_udp->sockets[sock_handle - 100];
    if (!sock->is_bound) {
        return -1;
    }

    uint32_t dst_ip = syn_peek_u32(to->ip, 0U);
    size_t tx_len = 0U;
    int ret = syn_udp_sendto(g_native_udp, sock->local_port, dst_ip, to->port,
                             (const uint8_t *)data, len, g_udp_tx_buf, &tx_len);
    if (ret >= 0 && tx_len > 0U) {
        if (g_eth_tx_fn(g_udp_tx_buf, tx_len) != SYN_OK) {
            return -1;
        }
    }
    return ret;
}

int syn_transport_udp_recvfrom(SYN_Socket sock_handle, void *buf, size_t max_len,
                               SYN_SockAddr *from, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (g_native_udp == NULL || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS ||
        buf == NULL || max_len == 0U) {
        return -1;
    }
    SYN_UdpSocket *sock = &g_native_udp->sockets[sock_handle - 100];
    if (!sock->is_bound || (sock->dgram_count == 0U && sock->rx_len == 0U)) {
        return 0;
    }

    size_t bytes_read = 0U;
    uint32_t src_ip = 0U;
    uint16_t src_port = 0U;
    if (syn_udp_recv(sock, buf, max_len, &bytes_read, &src_ip, &src_port) != SYN_OK) {
        return -1; /* LCOV_EXCL_LINE: sock, buf, and &bytes_read are non-NULL above */
    }

    if (from != NULL) {
        syn_poke_u32(src_ip, from->ip, 0U);
        from->port = src_port;
    }

    return (int)bytes_read;
}

bool syn_transport_udp_readable(SYN_Socket sock_handle)
{
    if (g_native_udp == NULL || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS) {
        return false;
    }
    const SYN_UdpSocket *sock = &g_native_udp->sockets[sock_handle - 100];
    return sock->is_bound && (sock->dgram_count > 0U || sock->rx_len > 0U);
}

void syn_transport_udp_close(SYN_Socket sock_handle)
{
    if (g_native_udp == NULL || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS) {
        return;
    }
    syn_udp_unbind(&g_native_udp->sockets[sock_handle - 100]);
}

/* LCOV_EXCL_START: 1-line weak hardware port wrappers overridden by mock_port in unit tests */
SYN_WEAK SYN_Status syn_port_eth_tx(const void *frame, size_t len)
{
    (void)frame;
    (void)len;
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Socket syn_port_udp_open(uint16_t port)
{
    return syn_transport_udp_open(port);
}

SYN_WEAK int syn_port_udp_sendto(SYN_Socket sock_handle, const void *data, size_t len,
                                 const SYN_SockAddr *to)
{
    return syn_transport_udp_sendto(sock_handle, data, len, to);
}

SYN_WEAK int syn_port_udp_recvfrom(SYN_Socket sock_handle, void *buf, size_t max_len,
                                   SYN_SockAddr *from, uint32_t timeout_ms)
{
    return syn_transport_udp_recvfrom(sock_handle, buf, max_len, from, timeout_ms);
}

SYN_WEAK bool syn_port_udp_readable(SYN_Socket sock_handle)
{
    return syn_transport_udp_readable(sock_handle);
}

SYN_WEAK void syn_port_sock_close(SYN_Socket sock_handle)
{
    if (sock_handle >= 100 && sock_handle < 100 + SYN_UDP_MAX_SOCKETS) {
        syn_transport_udp_close(sock_handle);
    } else if (sock_handle >= 0) {
        syn_assert_failed(__FILE__, __LINE__);
    }
}
/* LCOV_EXCL_STOP */

#endif /* SYN_USE_TRANSPORT_UDP */
