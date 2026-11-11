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
#include "../port/syn_port_gpio.h"
#include "../port/syn_port_spi.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Device descriptor ──────────────────────────────────────────────────── */

/** @brief SPI device descriptor — bus, CS, read-bit convention. */
typedef struct {
    uint8_t bus;      /**< SPI bus number                              */
    SYN_GPIO_Pin cs;  /**< Chip-select GPIO pin                        */
    uint8_t read_bit; /**< OR'd into register addr for reads (e.g., 0x80) */
} SYN_SPIDev;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize an SPI device descriptor.
 * @param dev       Device instance.
 * @param bus       SPI bus number.
 * @param cs        Chip-select GPIO pin.
 * @param read_bit  Bit OR'd into register address for reads.
 */
static inline void syn_spi_dev_init(SYN_SPIDev *dev, uint8_t bus, SYN_GPIO_Pin cs, uint8_t read_bit)
{
    dev->bus = bus;
    dev->cs = cs;
    dev->read_bit = read_bit;
}

/**
 * @brief Read a single 8-bit register.
 * @param dev  SPI device.
 * @param reg  Register address.
 * @return Register value.
 */
static inline uint8_t syn_spi_dev_read8(const SYN_SPIDev *dev, uint8_t reg)
{
    uint8_t tx[2] = {(uint8_t)(reg | dev->read_bit), 0x00};
    uint8_t rx[2] = {0, 0};

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    (void)syn_port_spi_transfer(dev->bus, tx, rx, 2U);
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);

    return rx[1];
}

/**
 * @brief Write a single 8-bit register.
 * @param dev  SPI device.
 * @param reg  Register address.
 * @param val  Value to write.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_write8(const SYN_SPIDev *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    SYN_Status st = syn_port_spi_transfer(dev->bus, tx, NULL, 2U);
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);

    return st;
}

/**
 * @brief Burst-read multiple registers.
 * @param dev  SPI device.
 * @param reg  Starting register address.
 * @param buf  Buffer to read into.
 * @param len  Number of bytes to read.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_read_burst(const SYN_SPIDev *dev, uint8_t reg, uint8_t *buf,
                                                size_t len)
{
    uint8_t tx_reg = (uint8_t)(reg | dev->read_bit);

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    SYN_Status st = syn_port_spi_transfer(dev->bus, &tx_reg, NULL, 1U);
    if (st == SYN_OK) {
        /* Subsequent clocks to read data */
        uint8_t zeros[32];
        memset(zeros, 0, sizeof(zeros));
        while (len > 0U) {
            size_t chunk = (len > 32U) ? 32U : len;
            st = syn_port_spi_transfer(dev->bus, zeros, buf, chunk);
            if (st != SYN_OK) {
                break;
            }
            buf += chunk;
            len -= chunk;
        }
    }

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);
    return st;
}

/**
 * @brief Burst-write multiple registers.
 * @param dev   SPI device.
 * @param reg   Starting register address.
 * @param data  Data to write.
 * @param len   Number of bytes.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_spi_dev_write_burst(const SYN_SPIDev *dev, uint8_t reg,
                                                 const uint8_t *data, size_t len)
{
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    SYN_Status st = syn_port_spi_transfer(dev->bus, &reg, NULL, 1U);
    if (st == SYN_OK && len > 0U) {
        st = syn_port_spi_transfer(dev->bus, data, NULL, len);
    }
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);

    return st;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_DEV_H */
