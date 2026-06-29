/**
 * @file syn_i2c_dev.h
 * @brief I2C device register helpers — thin layer over port I2C.
 *
 * Provides convenient register-level access for I2C sensors/peripherals.
 * Eliminates boilerplate for reading/writing 8-bit and 16-bit registers
 * and burst reads.
 *
 * @par Usage
 * @code
 *   SYN_I2CDev bmp280;
 *   syn_i2c_dev_init(&bmp280, 0, 0x76);  // bus 0, addr 0x76
 *
 *   uint8_t chip_id = syn_i2c_dev_read8(&bmp280, 0xD0);
 *   syn_i2c_dev_write8(&bmp280, 0xF4, 0x27);  // ctrl_meas
 *
 *   uint8_t burst[6];
 *   syn_i2c_dev_read_burst(&bmp280, 0xF7, burst, 6);
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_I2C_DEV_H
#define SYN_I2C_DEV_H

#include "../common/syn_defs.h"
#include "../port/syn_port_i2c.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Device descriptor ──────────────────────────────────────────────────── */

/** @brief I2C device descriptor — bus number + 7-bit address. */
typedef struct {
    uint8_t  bus;      /**< I2C bus number                              */
    uint8_t  addr;     /**< 7-bit device address                        */
} SYN_I2CDev;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize device descriptor.
 * @param dev   Device instance.
 * @param bus   I2C bus number.
 * @param addr  7-bit device address.
 */
static inline void syn_i2c_dev_init(SYN_I2CDev *dev,
                                     uint8_t bus, uint8_t addr)
{
    dev->bus  = bus;
    dev->addr = addr;
}

/**
 * @brief Read a single 8-bit register.
 * @param dev  Device.
 * @param reg  Register address.
 * @return Register value.
 */
static inline uint8_t syn_i2c_dev_read8(const SYN_I2CDev *dev,
                                         uint8_t reg)
{
    uint8_t val = 0;
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, &val, 1);
    return val;
}

/**
 * @brief Write a single 8-bit register.
 * @param dev  Device.
 * @param reg  Register address.
 * @param val  Value to write.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_i2c_dev_write8(const SYN_I2CDev *dev,
                                               uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return syn_port_i2c_write(dev->bus, dev->addr, buf, 2);
}

/**
 * @brief Read a 16-bit register (big-endian, MSB first).
 * @param dev  Device.
 * @param reg  Register address.
 * @return 16-bit value (MSB first).
 */
static inline uint16_t syn_i2c_dev_read16_be(const SYN_I2CDev *dev,
                                               uint8_t reg)
{
    uint8_t buf[2] = {0, 0};
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, 2);
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

/**
 * @brief Read a 16-bit register (little-endian, LSB first).
 * @param dev  Device.
 * @param reg  Register address.
 * @return 16-bit value (LSB first).
 */
static inline uint16_t syn_i2c_dev_read16_le(const SYN_I2CDev *dev,
                                               uint8_t reg)
{
    uint8_t buf[2] = {0, 0};
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, 2);
    return (uint16_t)((buf[1] << 8) | buf[0]);
}

/**
 * @brief Write a 16-bit register (big-endian).
 * @param dev  Device.
 * @param reg  Register address.
 * @param val  16-bit value to write.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_i2c_dev_write16_be(const SYN_I2CDev *dev,
                                                    uint8_t reg, uint16_t val)
{
    uint8_t buf[3] = { reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
    return syn_port_i2c_write(dev->bus, dev->addr, buf, 3);
}

/**
 * @brief Burst read: read @p len bytes starting from @p reg.
 * @param dev  Device.
 * @param reg  Starting register address.
 * @param buf  Output buffer.
 * @param len  Number of bytes to read.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_i2c_dev_read_burst(const SYN_I2CDev *dev,
                                                    uint8_t reg,
                                                    uint8_t *buf, size_t len)
{
    return syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, len);
}

/**
 * @brief Burst write: write @p len bytes starting from @p reg.
 * @param dev   Device.
 * @param reg   Starting register address.
 * @param data  Data to write.
 * @param len   Number of bytes (max 32).
 * @return SYN_OK on success, SYN_ERROR if len > 32.
 */
static inline SYN_Status syn_i2c_dev_write_burst(const SYN_I2CDev *dev,
                                                     uint8_t reg,
                                                     const uint8_t *data,
                                                     size_t len)
{
    /* Must prepend register address — use stack for small writes */
    uint8_t buf[33]; /* 1 reg + up to 32 data bytes */
    if (len > 32) return SYN_ERROR;
    buf[0] = reg;
    size_t i;
    for (i = 0; i < len; i++) buf[i + 1] = data[i];
    return syn_port_i2c_write(dev->bus, dev->addr, buf, len + 1);
}

/**
 * @brief Modify register: read, mask, set bits, write back.
 * @param dev   Device.
 * @param reg   Register address.
 * @param mask  Bits to modify.
 * @param val   New values for masked bits.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_i2c_dev_modify8(const SYN_I2CDev *dev,
                                                 uint8_t reg,
                                                 uint8_t mask, uint8_t val)
{
    uint8_t cur = syn_i2c_dev_read8(dev, reg);
    cur = (uint8_t)((cur & ~mask) | (val & mask));
    return syn_i2c_dev_write8(dev, reg, cur);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_I2C_DEV_H */
