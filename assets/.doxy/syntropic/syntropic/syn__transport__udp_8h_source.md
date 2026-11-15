

# File syn\_transport\_udp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.h**](syn__transport__udp_8h.md)

[Go to the documentation of this file](syn__transport__udp_8h.md)


```C++


#ifndef SYN_TRANSPORT_UDP_H
#define SYN_TRANSPORT_UDP_H

#include "syntropic/common/syn_defs.h"

#if !defined(SYN_USE_TRANSPORT_UDP) || SYN_USE_TRANSPORT_UDP

#include "syntropic/net/syn_udp.h"
#include "syntropic/port/syn_port_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef SYN_Status (*SYN_TransportEthTxFn)(const void *frame, size_t len);

void syn_transport_udp_set_instance(SYN_UDP *udp);

void syn_transport_udp_set_eth_tx(SYN_TransportEthTxFn tx_fn);

SYN_UDP *syn_transport_udp_get_instance(void);

SYN_Socket syn_transport_udp_open(uint16_t port);

int syn_transport_udp_sendto(SYN_Socket sock_handle, const void *data, size_t len,
                             const SYN_SockAddr *to);

int syn_transport_udp_recvfrom(SYN_Socket sock_handle, void *buf, size_t max_len,
                               SYN_SockAddr *from, uint32_t timeout_ms);

bool syn_transport_udp_readable(SYN_Socket sock_handle);

void syn_transport_udp_close(SYN_Socket sock_handle);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_TRANSPORT_UDP */

#endif /* SYN_TRANSPORT_UDP_H */
```


