

# File syn\_port\_socket.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_socket.h**](syn__port__socket_8h.md)

[Go to the documentation of this file](syn__port__socket_8h.md)


```C++

#ifndef SYN_PORT_SOCKET_H
#define SYN_PORT_SOCKET_H

#include <stdint.h>
#include <stddef.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int SYN_Socket;

#define SYN_SOCKET_INVALID  (-1)

typedef struct {
    uint8_t  ip[4];       
    uint16_t port;        
} SYN_SockAddr;

/* ── Connection ─────────────────────────────────────────────────────────── */

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr);

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port);

/* ── Data transfer ──────────────────────────────────────────────────────── */

int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len);

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len);

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                       uint32_t timeout_ms);

/* ── Server ─────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_sock_listen(uint16_t port, int backlog);

SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms);

/* ── UDP ────────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_udp_open(uint16_t port);

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                        const SYN_SockAddr *to);

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                          SYN_SockAddr *from, uint32_t timeout_ms);

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip);



/* ── Lifecycle ──────────────────────────────────────────────────────────── */



void syn_port_sock_close(SYN_Socket sock);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SOCKET_H */
```


