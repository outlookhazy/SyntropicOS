

# File syn\_port\_eth.h

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**syn\_port\_eth.h**](syn__port__eth_8h.md)

[Go to the documentation of this file](syn__port__eth_8h.md)


```C++

#ifndef SYN_PORT_ETH_H
#define SYN_PORT_ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

SYN_Status syn_port_eth_init(const uint8_t mac_addr[6]);

SYN_Status syn_port_eth_tx(const void *frame, size_t len);

SYN_Status syn_port_eth_rx(void *buf, size_t max_len, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_ETH_H */
```


