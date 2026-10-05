

# File syn\_uart.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_uart.h**](syn__uart_8h.md)

[Go to the documentation of this file](syn__uart_8h.md)


```C++

#ifndef SYN_UART_H
#define SYN_UART_H

#include "../common/syn_defs.h"
#include "../port/syn_port_uart.h"
#include "../util/syn_ringbuf.h"

#if defined(SYN_USE_DMA) && SYN_USE_DMA
#include "syn_dma.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration defaults (override in syn_config.h) ───────────────── */

#ifndef SYN_UART_TX_BUF_SIZE
#define SYN_UART_TX_BUF_SIZE 128
#endif

#ifndef SYN_UART_RX_BUF_SIZE
#define SYN_UART_RX_BUF_SIZE 128
#endif

#ifndef SYN_UART_MAX_INSTANCES
#define SYN_UART_MAX_INSTANCES 2
#endif

/* ── UART Configuration ─────────────────────────────────────────────────── */

typedef struct {
    SYN_UARTInstance instance; 
    uint32_t baudrate;         
    bool use_dma;              
#if defined(SYN_USE_DMA) && SYN_USE_DMA
    uint8_t dma_channel_rx;    
    const void *periph_rx_reg; 
#endif
} SYN_UART_Config;

/* ── UART handle ────────────────────────────────────────────────────────── */

typedef struct {
    SYN_UARTInstance instance;            
    SYN_RingBuf tx_rb;                    
    SYN_RingBuf rx_rb;                    
    uint8_t tx_buf[SYN_UART_TX_BUF_SIZE]; 
    uint8_t rx_buf[SYN_UART_RX_BUF_SIZE]; 
    bool initialized;                     
    bool use_dma;                         
#if defined(SYN_USE_DMA) && SYN_USE_DMA
    SYN_DMA dma_rx;              
    SYN_DMA_RingBuf dma_ring_rx; 
#endif
} SYN_UART;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_uart_init(SYN_UART *uart, SYN_UARTInstance instance, uint32_t baudrate);

SYN_Status syn_uart_init_config(SYN_UART *uart, const SYN_UART_Config *cfg);

SYN_Status syn_uart_deinit(SYN_UART *uart);

SYN_Status syn_uart_write_str(const SYN_UART *uart, const char *str, uint32_t timeout_ms);

SYN_Status syn_uart_write(const SYN_UART *uart, const uint8_t *data, size_t len,
                          uint32_t timeout_ms);

size_t syn_uart_read(SYN_UART *uart, uint8_t *data, size_t max_len);

bool syn_uart_rx_isr_feed(SYN_UART *uart, uint8_t byte);

SYN_Status syn_uart_write_async(SYN_UART *uart, const uint8_t *data, size_t len);

bool syn_uart_tx_isr_flush(SYN_UART *uart);

bool syn_uart_tx_complete(const SYN_UART *uart);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UART_H */
```


