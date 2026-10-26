

# File syn\_transport\_udp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport\_udp.h**](syn__transport__udp_8h.md)

[Go to the documentation of this file](syn__transport__udp_8h.md)


```C++

#ifndef SYN_TRANSPORT_UDP_H
#define SYN_TRANSPORT_UDP_H

#include "syntropic/net/syn_udp.h"
#include "syntropic/port/syn_port_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

void syn_transport_udp_set_instance(SYN_UDP *udp);

SYN_UDP *syn_transport_udp_get_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_UDP_H */
```


