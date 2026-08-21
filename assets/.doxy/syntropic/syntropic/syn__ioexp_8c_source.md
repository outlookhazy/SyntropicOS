

# File syn\_ioexp.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_ioexp.c**](syn__ioexp_8c.md)

[Go to the documentation of this file](syn__ioexp_8c.md)


```C++

#include "syn_ioexp.h"

#include "../util/syn_assert.h"

#include <string.h>

static void write_i2c_bytes(SYN_SoftI2C *i2c, uint8_t dev_addr, const uint8_t *buf, size_t len)
{
    syn_soft_i2c_start(i2c);
    syn_soft_i2c_write(i2c, (uint8_t)(dev_addr << 1));
    for (size_t i = 0; i < len; i++) {
        syn_soft_i2c_write(i2c, buf[i]);
    }
    syn_soft_i2c_stop(i2c);
}

SYN_Status syn_ioexp_init(SYN_IOExp *ioexp, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                          SYN_IOExpType type)
{
    SYN_ASSERT(ioexp != NULL);

    memset(ioexp, 0, sizeof(*ioexp));
    ioexp->type = type;
    ioexp->i2c_addr = i2c_addr;
    ioexp->num_pins = (type == SYN_IOEXP_MCP23008 || type == SYN_IOEXP_PCF8574) ? 8 : 16;
    ioexp->dir_mask = 0xFFFF; /* All inputs by default */

    syn_soft_i2c_init(&ioexp->i2c, scl, sda, 10);

    /* Initialize chip registers */
    if (type == SYN_IOEXP_MCP23017) {
        uint8_t buf[3] = {0x00, 0xFF, 0xFF}; /* IODIRA and IODIRB to all inputs */
        write_i2c_bytes(&ioexp->i2c, i2c_addr, buf, 3);
    } else if (type == SYN_IOEXP_MCP23008) {
        uint8_t buf[2] = {0x00, 0xFF}; /* IODIR all inputs */
        write_i2c_bytes(&ioexp->i2c, i2c_addr, buf, 2);
    } else if (type == SYN_IOEXP_TCA9555) {
        uint8_t buf[3] = {0x06, 0xFF, 0xFF}; /* Configuration registers to all inputs */
        write_i2c_bytes(&ioexp->i2c, i2c_addr, buf, 3);
    } else if (type == SYN_IOEXP_PCF8574) {
        uint8_t val = 0xFF; /* PCF8574 pseudo-bidirectional high = input */
        write_i2c_bytes(&ioexp->i2c, i2c_addr, &val, 1);
    }

    return SYN_OK;
}

void syn_ioexp_set_pin_mode(SYN_IOExp *ioexp, uint8_t pin, SYN_GPIO_Mode mode)
{
    if (ioexp == NULL || pin >= ioexp->num_pins)
        return;

    if (mode == SYN_GPIO_OUTPUT) {
        ioexp->dir_mask &= (uint16_t)~(1u << pin);
    } else {
        ioexp->dir_mask |= (uint16_t)(1u << pin);
        if (mode == SYN_GPIO_INPUT_PULLUP) {
            ioexp->pullup_mask |= (uint16_t)(1u << pin);
        }
    }

    if (ioexp->type == SYN_IOEXP_MCP23017) {
        uint8_t buf[3] = {0x00, (uint8_t)(ioexp->dir_mask & 0xFF), (uint8_t)(ioexp->dir_mask >> 8)};
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, buf, 3);
        uint8_t gppu[3] = {0x0C, (uint8_t)(ioexp->pullup_mask & 0xFF),
                           (uint8_t)(ioexp->pullup_mask >> 8)};
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, gppu, 3);
    }
}

void syn_ioexp_write_pin(SYN_IOExp *ioexp, uint8_t pin, SYN_GPIO_State state)
{
    if (ioexp == NULL || pin >= ioexp->num_pins)
        return;

    if (state == SYN_GPIO_HIGH) {
        ioexp->out_mask |= (uint16_t)(1u << pin);
    } else {
        ioexp->out_mask &= (uint16_t)~(1u << pin);
    }

    syn_ioexp_write_port(ioexp, ioexp->out_mask);
}

SYN_GPIO_State syn_ioexp_read_pin(SYN_IOExp *ioexp, uint8_t pin)
{
    if (ioexp == NULL || pin >= ioexp->num_pins)
        return SYN_GPIO_LOW;

    uint16_t port = syn_ioexp_read_port(ioexp);
    return (port & (1u << pin)) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

void syn_ioexp_write_port(SYN_IOExp *ioexp, uint16_t mask)
{
    if (ioexp == NULL)
        return;

    ioexp->out_mask = mask;

    if (ioexp->type == SYN_IOEXP_MCP23017) {
        uint8_t buf[3] = {0x12, (uint8_t)(mask & 0xFF), (uint8_t)(mask >> 8)};
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, buf, 3);
    } else if (ioexp->type == SYN_IOEXP_MCP23008) {
        uint8_t buf[2] = {0x09, (uint8_t)(mask & 0xFF)};
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, buf, 2);
    } else if (ioexp->type == SYN_IOEXP_PCF8574) {
        uint8_t val = (uint8_t)(mask & 0xFF);
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, &val, 1);
    } else if (ioexp->type == SYN_IOEXP_TCA9555) {
        uint8_t buf[3] = {0x02, (uint8_t)(mask & 0xFF), (uint8_t)(mask >> 8)};
        write_i2c_bytes(&ioexp->i2c, ioexp->i2c_addr, buf, 3);
    }
}

uint16_t syn_ioexp_read_port(SYN_IOExp *ioexp)
{
    if (ioexp == NULL)
        return 0;
    return ioexp->out_mask;
}
```


