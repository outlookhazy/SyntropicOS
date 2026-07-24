

# File syn\_uart.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_uart.h**](syn__uart_8h.md)

[Go to the documentation of this file](syn__uart_8h.md)


```C++

#ifndef SYN_UART_H
#define SYN_UART_H

#include "../common/syn_defs.h"
#include "../util/syn_ringbuf.h"
#include "../port/syn_port_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration defaults (override in syn_config.h) ───────────────── */

#ifndef SYN_UART_TX_BUF_SIZE
  #define SYN_UART_TX_BUF_SIZE   128
#endif

#ifndef SYN_UART_RX_BUF_SIZE
  #define SYN_UART_RX_BUF_SIZE   128
#endif

#ifndef SYN_UART_MAX_INSTANCES
  #define SYN_UART_MAX_INSTANCES  2
#endif

/* ── UART handle ────────────────────────────────────────────────────────── */

typedef struct {
    SYN_UARTInstance instance;     
    SYN_RingBuf     tx_rb;         
    SYN_RingBuf     rx_rb;         
    uint8_t          tx_buf[SYN_UART_TX_BUF_SIZE]; 
    uint8_t          rx_buf[SYN_UART_RX_BUF_SIZE]; 
    bool             initialized;   
} SYN_UART;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_uart_init(SYN_UART *uart,
                            SYN_UARTInstance instance,
                            uint32_t baudrate);

SYN_Status syn_uart_deinit(SYN_UART *uart);

SYN_Status syn_uart_write_str(const SYN_UART *uart,
                                const char *str,
                                uint32_t timeout_ms);

SYN_Status syn_uart_write(const SYN_UART *uart,
                             const uint8_t *data,
                             size_t len,
                             uint32_t timeout_ms);

size_t syn_uart_read(SYN_UART *uart, uint8_t *data, size_t max_len);

bool syn_uart_rx_isr_feed(SYN_UART *uart, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UART_H */
```


