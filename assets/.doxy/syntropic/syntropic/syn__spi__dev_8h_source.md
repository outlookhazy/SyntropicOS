

# File syn\_spi\_dev.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi\_dev.h**](syn__spi__dev_8h.md)

[Go to the documentation of this file](syn__spi__dev_8h.md)


```C++


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

typedef struct {
    uint8_t bus;      
    SYN_GPIO_Pin cs;  
    uint8_t read_bit; 
} SYN_SPIDev;

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_spi_dev_init(SYN_SPIDev *dev, uint8_t bus, SYN_GPIO_Pin cs, uint8_t read_bit)
{
    dev->bus = bus;
    dev->cs = cs;
    dev->read_bit = read_bit;
}

static inline uint8_t syn_spi_dev_read8(const SYN_SPIDev *dev, uint8_t reg)
{
    uint8_t tx[2] = {(uint8_t)(reg | dev->read_bit), 0x00};
    uint8_t rx[2] = {0, 0};

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    (void)syn_port_spi_transfer(dev->bus, tx, rx, 2U);
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);

    return rx[1];
}

static inline SYN_Status syn_spi_dev_write8(const SYN_SPIDev *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};

    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_LOW);
    SYN_Status st = syn_port_spi_transfer(dev->bus, tx, NULL, 2U);
    (void)syn_port_gpio_write(dev->cs, SYN_GPIO_HIGH);

    return st;
}

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
```


