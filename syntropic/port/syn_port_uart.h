/**
 * @file syn_port_uart.h
 * @brief UART port interface — functions the user must implement.
 *
 * Provides the low-level byte/buffer transmit and receive primitives that
 * the higher-level syn_uart driver builds upon.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_UART_H
#define SYN_PORT_UART_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a UART peripheral.
 *
 * @param instance  UART instance number (0, 1, 2, …).
 * @param baudrate  Desired baud rate.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate);

/**
 * @brief De-initialize a UART peripheral.
 *
 * @param instance  UART instance to de-initialize.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance);

/**
 * @brief Transmit a buffer of bytes (blocking).
 *
 * @param instance   UART instance.
 * @param data       Pointer to data to transmit.
 * @param len        Number of bytes to transmit.
 * @param timeout_ms Timeout in milliseconds (0 = no timeout).
 * @return SYN_OK on success, SYN_TIMEOUT if the timeout elapsed.
 */
SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                    const uint8_t *data,
                                    size_t len,
                                    uint32_t timeout_ms);

/**
 * @brief Receive bytes into a buffer (blocking).
 *
 * @param instance   UART instance.
 * @param data       Buffer to receive into.
 * @param len        Maximum number of bytes to receive.
 * @param received   [out] Actual number of bytes received.
 * @param timeout_ms Timeout in milliseconds (0 = no timeout).
 * @return SYN_OK on success, SYN_TIMEOUT if the timeout elapsed.
 */
SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                   uint8_t *data,
                                   size_t len,
                                   size_t *received,
                                   uint32_t timeout_ms);

/**
 * @brief Transmit a single byte (blocking).
 *
 * @param instance  UART instance.
 * @param byte      Byte to transmit.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance,
                                         uint8_t byte);

/**
 * @brief Receive a single byte (blocking).
 *
 * @param instance   UART instance.
 * @param byte       [out] Received byte.
 * @param timeout_ms Timeout in milliseconds (0 = no timeout).
 * @return SYN_OK if a byte was received, SYN_TIMEOUT otherwise.
 */
SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance,
                                        uint8_t *byte,
                                        uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_UART_H */
