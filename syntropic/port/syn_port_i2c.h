/**
 * @file syn_port_i2c.h
 * @brief I2C port interface — implement these for your platform.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_I2C_H
#define SYN_PORT_I2C_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── I2C configuration ──────────────────────────────────────────────────── */

/** @brief I2C bus configuration. */
typedef struct {
    uint8_t   bus;            /**< I2C bus index (0, 1, ...)             */
    uint32_t  clock_hz;       /**< I2C clock frequency (100k, 400k, etc) */
} SYN_I2C_Config;

/* ── Port functions (user implements) ───────────────────────────────────── */

/**
 * @brief Initialize an I2C bus.
 * @param cfg  I2C configuration.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_i2c_init(const SYN_I2C_Config *cfg);

/**
 * @brief Deinitialize an I2C bus.
 * @param bus  I2C bus index.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_i2c_deinit(uint8_t bus);

/**
 * @brief Write data to an I2C device.
 *
 * @param bus     I2C bus index.
 * @param addr    7-bit device address.
 * @param data    Data to write.
 * @param len     Number of bytes.
 * @return SYN_OK on ACK, SYN_ERROR on NACK/timeout.
 */
SYN_Status syn_port_i2c_write(uint8_t bus, uint8_t addr,
                                const uint8_t *data, size_t len);

/**
 * @brief Read data from an I2C device.
 *
 * @param bus     I2C bus index.
 * @param addr    7-bit device address.
 * @param data    Buffer to receive data.
 * @param len     Number of bytes to read.
 * @return SYN_OK on ACK, SYN_ERROR on NACK/timeout.
 */
SYN_Status syn_port_i2c_read(uint8_t bus, uint8_t addr,
                               uint8_t *data, size_t len);

/**
 * @brief Write then read (register access pattern).
 *
 * Sends a write (typically a register address) followed by a repeated
 * start and read. This is the most common I2C transaction pattern.
 *
 * @param bus      I2C bus index.
 * @param addr     7-bit device address.
 * @param tx_data  Data to write (e.g., register address).
 * @param tx_len   Write length.
 * @param rx_data  Buffer for read data.
 * @param rx_len   Read length.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_i2c_write_read(uint8_t bus, uint8_t addr,
                                     const uint8_t *tx_data, size_t tx_len,
                                     uint8_t *rx_data, size_t rx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_I2C_H */
