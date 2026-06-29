/**
 * @file syn_uart.h
 * @brief UART driver — buffered I/O and formatted output.
 *
 * Provides interrupt-driven buffered UART on top of the port layer.
 * Each UART instance gets a TX and RX ring buffer. The buffer sizes
 * are configurable in syn_config.h.
 * @ingroup syn_drivers
 */

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
  /** @brief UART transmit buffer size (bytes). */
  #define SYN_UART_TX_BUF_SIZE   128
#endif

#ifndef SYN_UART_RX_BUF_SIZE
  /** @brief UART receive buffer size (bytes). */
  #define SYN_UART_RX_BUF_SIZE   128
#endif

#ifndef SYN_UART_MAX_INSTANCES
  /** @brief Maximum UART instances supported. */
  #define SYN_UART_MAX_INSTANCES  2
#endif

/* ── UART handle ────────────────────────────────────────────────────────── */

/**
 * @brief UART driver handle.
 *
 * The user allocates this struct (stack or static) and passes it to
 * syn_uart_init(). The backing buffers are embedded so no dynamic
 * allocation is needed.
 */
typedef struct {
    SYN_UARTInstance instance;     /**< Hardware UART peripheral index */
    SYN_RingBuf     tx_rb;         /**< Transmit ring buffer control block */
    SYN_RingBuf     rx_rb;         /**< Receive ring buffer control block */
    uint8_t          tx_buf[SYN_UART_TX_BUF_SIZE]; /**< Physical storage memory for TX ring buffer */
    uint8_t          rx_buf[SYN_UART_RX_BUF_SIZE]; /**< Physical storage memory for RX ring buffer */
    bool             initialized;   /**< Initialization flag status */
} SYN_UART;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a UART instance with buffered I/O.
 *
 * @param uart      Pointer to a caller-owned SYN_UART struct.
 * @param instance  UART peripheral number (0, 1, …).
 * @param baudrate  Desired baud rate.
 * @return SYN_OK on success.
 */
SYN_Status syn_uart_init(SYN_UART *uart,
                            SYN_UARTInstance instance,
                            uint32_t baudrate);

/**
 * @brief De-initialize a UART instance.
 *
 * @param uart UART handle to deinitialize.
 * @return SYN_OK on success.
 */
SYN_Status syn_uart_deinit(SYN_UART *uart);

/**
 * @brief Write a string to the UART (blocking).
 *
 * @param uart       UART handle.
 * @param str        Null-terminated string.
 * @param timeout_ms Timeout in milliseconds (0 = no timeout).
 * @return SYN_OK on success.
 */
SYN_Status syn_uart_write_str(const SYN_UART *uart,
                                const char *str,
                                uint32_t timeout_ms);

/**
 * @brief Write a buffer of bytes to the UART (blocking).
 *
 * @param uart       UART handle.
 * @param data       Data to transmit.
 * @param len        Number of bytes.
 * @param timeout_ms Timeout in milliseconds (0 = no timeout).
 * @return SYN_OK on success.
 */
SYN_Status syn_uart_write(const SYN_UART *uart,
                             const uint8_t *data,
                             size_t len,
                             uint32_t timeout_ms);

/**
 * @brief Read bytes from the UART RX ring buffer.
 *
 * Reads up to @p max_len bytes that have been received. Non-blocking:
 * returns immediately with however many bytes are available.
 *
 * @param uart     UART handle.
 * @param data     Buffer to read into.
 * @param max_len  Maximum number of bytes to read.
 * @return Number of bytes actually read.
 */
size_t syn_uart_read(SYN_UART *uart, uint8_t *data, size_t max_len);

/**
 * @brief Feed a received byte into the UART RX ring buffer.
 *
 * Call this from your UART RX ISR to push incoming data into the
 * driver's buffer.
 *
 * @param uart  UART handle.
 * @param byte  The received byte.
 * @return true if the byte was stored, false if the RX buffer is full.
 */
bool syn_uart_rx_isr_feed(SYN_UART *uart, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UART_H */
