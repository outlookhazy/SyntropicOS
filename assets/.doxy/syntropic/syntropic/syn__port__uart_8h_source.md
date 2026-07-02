

# File syn\_port\_uart.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_uart.h**](syn__port__uart_8h.md)

[Go to the documentation of this file](syn__port__uart_8h.md)


```C++

#ifndef SYN_PORT_UART_H
#define SYN_PORT_UART_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate);

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance);

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                    const uint8_t *data,
                                    size_t len,
                                    uint32_t timeout_ms);

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                   uint8_t *data,
                                   size_t len,
                                   size_t *received,
                                   uint32_t timeout_ms);

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance,
                                         uint8_t byte);

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance,
                                        uint8_t *byte,
                                        uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_UART_H */
```


