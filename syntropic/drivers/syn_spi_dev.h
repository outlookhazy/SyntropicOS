/**
 * @file syn_spi_dev.h
 * @brief SPI device register helpers — thin layer over port SPI.
 *
 * Provides register-level access for SPI devices. Handles CS
 * assertion, register address framing, and read/write bit.
 *
 * @par Usage
 * @code
 *   SYN_SPIDev mpu;
 *   syn_spi_dev_init(&mpu, 0, 0, 0x80);  // bus 0, CS 0, read bit = 0x80
 *
 *   uint8_t who = syn_spi_dev_read8(&mpu, 0x75);  // WHO_AM_I
 *   syn_spi_dev_write8(&mpu, 0x6B, 0x00);         // wake up
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_SPI_DEV_H
#define SYN_SPI_DEV_H

#include "../common/syn_defs.h"
#include "../port/syn_port_spi.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Device descriptor ──────────────────────────────────────────────────── */

/** @brief SPI device descriptor — bus, CS, read-bit convention. */
typedef struct {
    uint8_t  bus;       /**< SPI bus number                              */
    uint8_t  cs;        /**< Chip select index                           */
    uint8_t  read_bit;  /**< OR'd into register addr for reads (e.g., 0x80) */
} SYN_SPIDev;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize an SPI device descriptor.
 * @param dev       Device instance.
 * @param bus       SPI bus number.
 * @param cs        Chip-select index.
 * @param read_bit  Bit OR'd into register address for reads.
 */
static inline void syn_spi_dev_init(SYN_SPIDev *dev,
                                     uint8_t bus, uint8_t cs,
                                     uint8_t read_bit)
{
    dev->bus      = bus;
    dev->cs       = cs;
    dev->read_bit = read_bit;
}

/**
 * @brief Read a single 8-bit register.
 * @param dev  SPI device.
 * @param reg  Register address.
 * @return Register value.
 */
static inline uint8_t syn_spi_dev_read8(const SYN_SPIDev *dev,
                                         uint8_t reg)
{
    uint8_t tx[2] = { (uint8_t)(reg | dev->read_bit), 0x00 };
    uint8_t rx[2] = { 0, 0 };

    syn_port_spi_cs_assert(dev->bus, dev->cs);
    syn_port_spi_transfer(dev->bus, tx, rx, 2);
    syn_port_spi_cs_deassert(dev->bus, dev->cs);

    return rx[1];
}

/**
 * @brief Write a single 8-bit register.
 * @param dev  SPI device.
 * @param reg  Register address.
 * @param val  Value to write.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_write8(const SYN_SPIDev *dev,
                                               uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = { reg, val };

    syn_port_spi_cs_assert(dev->bus, dev->cs);
    syn_port_spi_transfer(dev->bus, tx, NULL, 2);
    syn_port_spi_cs_deassert(dev->bus, dev->cs);

    return SYN_OK;
}

/**
 * @brief Burst-read multiple registers.
 * @param dev  SPI device.
 * @param reg  Starting register address.
 * @param buf  Buffer to read into.
 * @param len  Number of bytes to read.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_read_burst(const SYN_SPIDev *dev,
                                                    uint8_t reg,
                                                    uint8_t *buf, size_t len)
{
    uint8_t tx_reg = (uint8_t)(reg | dev->read_bit);

    syn_port_spi_cs_assert(dev->bus, dev->cs);
    syn_port_spi_transfer(dev->bus, &tx_reg, NULL, 1);

    /* Subsequent clocks to read data */
    uint8_t zeros[32];
    memset(zeros, 0, sizeof(zeros));
    while (len > 0) {
        size_t chunk = (len > 32) ? 32 : len;
        syn_port_spi_transfer(dev->bus, zeros, buf, chunk);
        buf += chunk;
        len -= chunk;
    }

    syn_port_spi_cs_deassert(dev->bus, dev->cs);
    return SYN_OK;
}

/**
 * @brief Burst-write multiple registers.
 * @param dev   SPI device.
 * @param reg   Starting register address.
 * @param data  Data to write.
 * @param len   Number of bytes.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_write_burst(const SYN_SPIDev *dev,
                                                     uint8_t reg,
                                                     const uint8_t *data,
                                                     size_t len)
{
    syn_port_spi_cs_assert(dev->bus, dev->cs);
    syn_port_spi_transfer(dev->bus, &reg, NULL, 1);
    syn_port_spi_transfer(dev->bus, data, NULL, len);
    syn_port_spi_cs_deassert(dev->bus, dev->cs);

    return SYN_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_DEV_H */
