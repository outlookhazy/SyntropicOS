/**
 * @file syn_soft_i2c.h
 * @brief Software I2C (bit-banging) driver.
 *
 * Implements I2C master functionality using any two GPIO pins.
 * Relies on syn_port_gpio.h for pin manipulation.
 * Uses a simple NOP loop for clock stretching/delays.
 * @ingroup syn_drivers
 */

#ifndef SYN_SOFT_I2C_H
#define SYN_SOFT_I2C_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Software I2C instance.
 */
typedef struct {
    SYN_GPIO_Pin scl;              /**< SCL GPIO pin identifier */
    SYN_GPIO_Pin sda;              /**< SDA GPIO pin identifier */
    uint32_t delay_loops;          /**< Iteration count for half-clock software delay */
} SYN_SoftI2C;

/**
 * @brief Initialize the soft I2C pins.
 * @param i2c          Pointer to I2C instance.
 * @param scl          SCL pin.
 * @param sda          SDA pin.
 * @param delay_loops  Number of iterations for a half-clock delay.
 */
void syn_soft_i2c_init(SYN_SoftI2C *i2c, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint32_t delay_loops);

/**
 * @brief Generate an I2C START condition.
 * @param i2c Pointer to I2C instance.
 */
void syn_soft_i2c_start(const SYN_SoftI2C *i2c);

/**
 * @brief Generate an I2C STOP condition.
 * @param i2c Pointer to I2C instance.
 */
void syn_soft_i2c_stop(const SYN_SoftI2C *i2c);

/**
 * @brief Write a byte to the I2C bus.
 * @param i2c  Pointer to I2C instance.
 * @param data Byte to write.
 * @return true if the slave ACKed, false if NACKed.
 */
bool syn_soft_i2c_write(const SYN_SoftI2C *i2c, uint8_t data);

/**
 * @brief Read a byte from the I2C bus.
 * @param i2c Pointer to I2C instance.
 * @param ack true to ACK the byte, false to NACK (end of read).
 * @return The byte read.
 */
uint8_t syn_soft_i2c_read(const SYN_SoftI2C *i2c, bool ack);

/**
 * @brief Perform a write-then-read I2C transaction.
 *
 * Sends a START, writes the device address + tx_data, then issues a
 * repeated START, reads rx_len bytes into rx_data, and sends a STOP.
 * This covers the common "write register address, read data" pattern.
 *
 * @param i2c       I2C instance.
 * @param dev_addr  7-bit device address (will be left-shifted internally).
 * @param tx_data   Data to write (e.g., register address). Can be NULL if tx_len is 0.
 * @param tx_len    Number of bytes to write.
 * @param rx_data   Buffer to read into. Can be NULL if rx_len is 0.
 * @param rx_len    Number of bytes to read.
 * @return true if all bytes were ACKed, false on NACK.
 */
bool syn_soft_i2c_write_read(SYN_SoftI2C *i2c, uint8_t dev_addr,
                              const uint8_t *tx_data, size_t tx_len,
                              uint8_t *rx_data, size_t rx_len);

#ifdef __cplusplus
}
#endif
#endif // SYN_SOFT_I2C_H
