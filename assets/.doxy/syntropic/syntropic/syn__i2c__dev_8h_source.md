

# File syn\_i2c\_dev.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c\_dev.h**](syn__i2c__dev_8h.md)

[Go to the documentation of this file](syn__i2c__dev_8h.md)


```C++

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

typedef struct {
    uint8_t  bus;      
    uint8_t  addr;     
} SYN_I2CDev;

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_i2c_dev_init(SYN_I2CDev *dev,
                                     uint8_t bus, uint8_t addr)
{
    dev->bus  = bus;
    dev->addr = addr;
}

static inline uint8_t syn_i2c_dev_read8(const SYN_I2CDev *dev,
                                         uint8_t reg)
{
    uint8_t val = 0;
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, &val, 1);
    return val;
}

static inline SYN_Status syn_i2c_dev_write8(const SYN_I2CDev *dev,
                                               uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return syn_port_i2c_write(dev->bus, dev->addr, buf, 2);
}

static inline uint16_t syn_i2c_dev_read16_be(const SYN_I2CDev *dev,
                                               uint8_t reg)
{
    uint8_t buf[2] = {0, 0};
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, 2);
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

static inline uint16_t syn_i2c_dev_read16_le(const SYN_I2CDev *dev,
                                               uint8_t reg)
{
    uint8_t buf[2] = {0, 0};
    syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, 2);
    return (uint16_t)((buf[1] << 8) | buf[0]);
}

static inline SYN_Status syn_i2c_dev_write16_be(const SYN_I2CDev *dev,
                                                    uint8_t reg, uint16_t val)
{
    uint8_t buf[3] = { reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
    return syn_port_i2c_write(dev->bus, dev->addr, buf, 3);
}

static inline SYN_Status syn_i2c_dev_read_burst(const SYN_I2CDev *dev,
                                                    uint8_t reg,
                                                    uint8_t *buf, size_t len)
{
    return syn_port_i2c_write_read(dev->bus, dev->addr, &reg, 1, buf, len);
}

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
```


