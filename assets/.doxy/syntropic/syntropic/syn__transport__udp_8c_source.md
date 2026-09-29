

# File syn\_transport\_udp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.c**](syn__transport__udp_8c.md)

[Go to the documentation of this file](syn__transport__udp_8c.md)


```C++

#include "syntropic/net/syn_transport_udp.h"

#include "port/syn_port_eth.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

static SYN_UDP *g_native_udp = NULL;
static uint8_t g_udp_tx_buf[SYN_ETH_MAX_FRAME_LEN];

void syn_transport_udp_set_instance(SYN_UDP *udp)
{
    g_native_udp = udp;
}

SYN_UDP *syn_transport_udp_get_instance(void)
{
    return g_native_udp;
}

/* LCOV_EXCL_START: Defensive bounds check / hardware port fallback */
SYN_WEAK SYN_Socket syn_port_udp_open(uint16_t port)
{
    if (!g_native_udp) {
        return SYN_SOCKET_INVALID;
    }
    SYN_UdpSocket *sock = syn_udp_bind(g_native_udp, port);
    if (!sock) {
        return SYN_SOCKET_INVALID;
    }
    for (int i = 0; i < SYN_UDP_MAX_SOCKETS; i++) {
        if (&g_native_udp->sockets[i] == sock) {
            return i + 100;
        }
    }
    return SYN_SOCKET_INVALID;
}

SYN_WEAK int syn_port_udp_sendto(SYN_Socket sock_handle, const void *data, size_t len,
                                 const SYN_SockAddr *to)
{
    if (!g_native_udp || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS || !to) {
        return -1;
    }
    int idx = sock_handle - 100;
    SYN_UdpSocket *sock = &g_native_udp->sockets[idx];
    if (!sock->is_bound) {
        return -1;
    }

    uint32_t dst_ip = syn_peek_u32(to->ip, 0);
    size_t tx_len = 0;
    int ret = syn_udp_sendto(g_native_udp, sock->local_port, dst_ip, to->port,
                             (const uint8_t *)data, len, g_udp_tx_buf, &tx_len);
    if (ret >= 0 && tx_len > 0) {
        syn_port_eth_tx(g_udp_tx_buf, tx_len);
    }
    return ret;
}

SYN_WEAK int syn_port_udp_recvfrom(SYN_Socket sock_handle, void *buf, size_t max_len,
                                   SYN_SockAddr *from, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (!g_native_udp || sock_handle < 100 || sock_handle >= 100 + SYN_UDP_MAX_SOCKETS) {
        return -1;
    }
    int idx = sock_handle - 100;
    SYN_UdpSocket *sock = &g_native_udp->sockets[idx];
    if (!sock->is_bound || sock->rx_len == 0) {
        return 0;
    }

    size_t copy_len = (sock->rx_len < max_len) ? sock->rx_len : max_len;
    memcpy(buf, sock->rx_buf, copy_len);
    memmove(sock->rx_buf, sock->rx_buf + copy_len, sock->rx_len - copy_len);
    sock->rx_len -= (uint16_t)copy_len;

    if (from) {
        syn_poke_u32(sock->remote_ip, from->ip, 0);
        from->port = sock->remote_port;
    }

    return (int)copy_len;
}
/* LCOV_EXCL_STOP */
```


