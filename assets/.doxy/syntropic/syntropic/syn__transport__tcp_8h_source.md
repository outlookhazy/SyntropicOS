

# File syn\_transport\_tcp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_tcp.h**](syn__transport__tcp_8h.md)

[Go to the documentation of this file](syn__transport__tcp_8h.md)


```C++

#ifndef SYN_TRANSPORT_TCP_H
#define SYN_TRANSPORT_TCP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "syn_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SYN_Socket sock;                 
    uint8_t    state;                
    uint16_t   payload_len;          
    uint16_t   bytes_read;           
    uint8_t    rx_buf[128];          
} SYN_TransportTcp;

void syn_transport_tcp_init(SYN_Transport *t, SYN_TransportTcp *tcp, SYN_Socket sock);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_TCP_H */
```


