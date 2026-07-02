

# File syn\_port\_can.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_can.h**](syn__port__can_8h.md)

[Go to the documentation of this file](syn__port__can_8h.md)


```C++

#ifndef SYN_PORT_CAN_H
#define SYN_PORT_CAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool syn_port_can_init(uint8_t port, uint32_t bitrate);

bool syn_port_can_send(uint8_t port, uint32_t id, bool extended,
                         const uint8_t *data, uint8_t dlc);

bool syn_port_can_receive(uint8_t port, uint32_t *id, bool *extended,
                            uint8_t *data, uint8_t *dlc);

void syn_port_can_set_filter(uint8_t port, uint32_t id, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_CAN_H */
```


