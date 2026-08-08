

# File syn\_uart.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_uart.c**](syn__uart_8c.md)

[Go to the documentation of this file](syn__uart_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_UART) || SYN_USE_UART

#include "../util/syn_assert.h"
#include "syn_uart.h"

#include <string.h>

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_uart_init(SYN_UART *uart, SYN_UARTInstance instance, uint32_t baudrate)
{
    SYN_UART_Config cfg = {.instance = instance, .baudrate = baudrate, .use_dma = false};
    return syn_uart_init_config(uart, &cfg);
}

SYN_Status syn_uart_init_config(SYN_UART *uart, const SYN_UART_Config *cfg)
{
    if (uart == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(uart, 0, sizeof(*uart));
    uart->instance = cfg->instance;
    uart->initialized = false;
    uart->use_dma = cfg->use_dma;

    syn_ringbuf_init(&uart->tx_rb, uart->tx_buf, sizeof(uart->tx_buf));
    syn_ringbuf_init(&uart->rx_rb, uart->rx_buf, sizeof(uart->rx_buf));

    SYN_Status status = syn_port_uart_init(cfg->instance, cfg->baudrate);
    if (status != SYN_OK) {
        return status;
    }

#if defined(SYN_USE_DMA) && SYN_USE_DMA
    if (cfg->use_dma) {
        SYN_DMA_Config dma_cfg = {.channel_id = cfg->dma_channel_rx,
                                  .dir = SYN_DMA_DIR_PERIPH_TO_MEM,
                                  .data_size = SYN_DMA_SIZE_8BIT,
                                  .src_inc = false,
                                  .dst_inc = true,
                                  .callback = NULL,
                                  .user_ctx = NULL};
        (void)syn_dma_init(&uart->dma_rx, &dma_cfg);
        (void)syn_dma_ringbuf_init(&uart->dma_ring_rx, &uart->dma_rx, uart->rx_buf,
                                   sizeof(uart->rx_buf));

        status = syn_dma_ringbuf_start(&uart->dma_ring_rx, cfg->periph_rx_reg);
        if (status != SYN_OK) {
            syn_port_uart_deinit(cfg->instance);
            return status;
        }
    }
#endif

    uart->initialized = true;
    return SYN_OK;
}

SYN_Status syn_uart_deinit(SYN_UART *uart)
{
    SYN_ASSERT(uart != NULL);

    if (!uart->initialized) {
        return SYN_OK;
    }

#if defined(SYN_USE_DMA) && SYN_USE_DMA
    if (uart->use_dma) {
        syn_dma_stop(&uart->dma_rx);
    }
#endif

    SYN_Status status = syn_port_uart_deinit(uart->instance);
    uart->initialized = false;
    syn_ringbuf_reset(&uart->tx_rb);
    syn_ringbuf_reset(&uart->rx_rb);

    return status;
}

SYN_Status syn_uart_write_str(const SYN_UART *uart, const char *str, uint32_t timeout_ms)
{
    SYN_ASSERT(uart != NULL);
    SYN_ASSERT(str != NULL);
    SYN_ASSERT(uart->initialized);

    size_t len = strlen(str);
    if (len == 0) {
        return SYN_OK;
    }

    return syn_port_uart_transmit(uart->instance, (const uint8_t *)str, len, timeout_ms);
}

SYN_Status syn_uart_write(const SYN_UART *uart, const uint8_t *data, size_t len,
                          uint32_t timeout_ms)
{
    SYN_ASSERT(uart != NULL);
    SYN_ASSERT(data != NULL || len == 0);
    SYN_ASSERT(uart->initialized);

    if (len == 0) {
        return SYN_OK;
    }

    return syn_port_uart_transmit(uart->instance, data, len, timeout_ms);
}

size_t syn_uart_read(SYN_UART *uart, uint8_t *data, size_t max_len)
{
    SYN_ASSERT(uart != NULL);
    SYN_ASSERT(data != NULL || max_len == 0);

#if defined(SYN_USE_DMA) && SYN_USE_DMA
    if (uart->use_dma) {
        return syn_dma_ringbuf_read(&uart->dma_ring_rx, data, max_len);
    }
#endif

    size_t count = 0;
    while (count < max_len) {
        uint8_t byte;
        if (!syn_ringbuf_get(&uart->rx_rb, &byte)) {
            break;
        }
        data[count++] = byte;
    }
    return count;
}

bool syn_uart_rx_isr_feed(SYN_UART *uart, uint8_t byte)
{
    SYN_ASSERT(uart != NULL);
    return syn_ringbuf_put(&uart->rx_rb, byte);
}

SYN_Status syn_uart_write_async(SYN_UART *uart, const uint8_t *data, size_t len)
{
    SYN_ASSERT(uart != NULL);
    SYN_ASSERT(data != NULL || len == 0);

    if (uart == NULL || (!uart->initialized)) { /* LCOV_EXCL_LINE: Defensive fallback */
        return SYN_INVALID_PARAM;               /* LCOV_EXCL_LINE */
    }

    if (len == 0) {
        return SYN_OK;
    }

    /* All-or-nothing atomic check: reject write if insufficient free buffer space */
    if (syn_ringbuf_free(&uart->tx_rb) < len) {
        return SYN_BUSY;
    }

    /* Bulk copy data into TX ring buffer */
    (void)syn_ringbuf_write(&uart->tx_rb, data, len);

    /* Kickstart hardware TX interrupt chain */
    syn_port_uart_enable_txe_irq(uart->instance);
    return SYN_OK;
}

bool syn_uart_tx_isr_flush(SYN_UART *uart)
{
    SYN_ASSERT(uart != NULL);

    if (uart == NULL) { /* LCOV_EXCL_LINE: Defensive fallback */
        return false;   /* LCOV_EXCL_LINE */
    }

    uint8_t byte;
    if (syn_ringbuf_get(&uart->tx_rb, &byte)) {
        syn_port_uart_write_dr(uart->instance, byte);
        return true;
    }

    /* Ring buffer empty — disable TXE interrupt */
    syn_port_uart_disable_txe_irq(uart->instance);
    return false;
}

bool syn_uart_tx_complete(const SYN_UART *uart)
{
    SYN_ASSERT(uart != NULL);

    if (uart == NULL) { /* LCOV_EXCL_LINE: Defensive fallback */
        return true;    /* LCOV_EXCL_LINE */
    }

    /* Complete ONLY when software tx_rb is empty AND hardware shift register is clear */
    return syn_ringbuf_empty(&uart->tx_rb) && syn_port_uart_is_tc_set(uart->instance);
}

#endif /* SYN_USE_UART */
```


