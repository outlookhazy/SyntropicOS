#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_UART) || SYN_USE_UART

/**
 * @file syn_uart.c
 * @brief UART driver implementation.
 */

#include "syn_uart.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_uart_init(SYN_UART *uart,
                            SYN_UARTInstance instance,
                            uint32_t baudrate)
{
    SYN_ASSERT(uart != NULL);

    uart->instance = instance;
    uart->initialized = false;

    syn_ringbuf_init(&uart->tx_rb, uart->tx_buf, sizeof(uart->tx_buf));
    syn_ringbuf_init(&uart->rx_rb, uart->rx_buf, sizeof(uart->rx_buf));

    SYN_Status status = syn_port_uart_init(instance, baudrate);
    if (status != SYN_OK) {
        return status;
    }

    uart->initialized = true;
    return SYN_OK;
}

SYN_Status syn_uart_deinit(SYN_UART *uart)
{
    SYN_ASSERT(uart != NULL);

    if (!uart->initialized) {
        return SYN_OK;
    }

    SYN_Status status = syn_port_uart_deinit(uart->instance);
    uart->initialized = false;
    syn_ringbuf_reset(&uart->tx_rb);
    syn_ringbuf_reset(&uart->rx_rb);

    return status;
}

SYN_Status syn_uart_write_str(const SYN_UART *uart,
                                const char *str,
                                uint32_t timeout_ms)
{
    SYN_ASSERT(uart != NULL);
    SYN_ASSERT(str != NULL);
    SYN_ASSERT(uart->initialized);

    size_t len = strlen(str);
    if (len == 0) {
        return SYN_OK;
    }

    return syn_port_uart_transmit(uart->instance,
                                   (const uint8_t *)str,
                                   len,
                                   timeout_ms);
}

SYN_Status syn_uart_write(const SYN_UART *uart,
                             const uint8_t *data,
                             size_t len,
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

#endif /* SYN_USE_UART */
