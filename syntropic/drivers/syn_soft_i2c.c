#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SOFT_I2C) || SYN_USE_SOFT_I2C

#include "syn_soft_i2c.h"
#include "syn_gpio.h"
#include "../util/syn_assert.h"

static void i2c_delay(const SYN_SoftI2C *i2c) {
    for (volatile uint32_t i = 0; i < i2c->delay_loops; i++) {
        // NOP loop
    }
}

static void sda_high(const SYN_SoftI2C *i2c) {
    syn_gpio_write(i2c->sda, SYN_GPIO_HIGH);
    // Alternatively: syn_gpio_init(i2c->sda, SYN_GPIO_INPUT) for pseudo open-drain
}

static void sda_low(const SYN_SoftI2C *i2c) {
    // syn_gpio_init(i2c->sda, SYN_GPIO_OUTPUT);
    syn_gpio_write(i2c->sda, SYN_GPIO_LOW);
}

static void scl_high(const SYN_SoftI2C *i2c) {
    syn_gpio_write(i2c->scl, SYN_GPIO_HIGH);
    // Clock stretching support: wait while SCL is held low by a slave
    // (Requires SCL to be configured as Open-Drain input)
    uint32_t timeout = 10000;
    while (syn_gpio_read(i2c->scl) == SYN_GPIO_LOW && timeout > 0) {
        timeout--;
    }
}

static void scl_low(const SYN_SoftI2C *i2c) {
    syn_gpio_write(i2c->scl, SYN_GPIO_LOW);
}

void syn_soft_i2c_init(SYN_SoftI2C *i2c, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint32_t delay_loops) {
    SYN_ASSERT(i2c != NULL);
    i2c->scl = scl;
    i2c->sda = sda;
    i2c->delay_loops = delay_loops;

    // We assume the HAL supports Open-Drain mode
    syn_gpio_init(i2c->scl, SYN_GPIO_OUTPUT_OD);
    syn_gpio_init(i2c->sda, SYN_GPIO_OUTPUT_OD);

    sda_high(i2c);
    scl_high(i2c);
}

void syn_soft_i2c_start(const SYN_SoftI2C *i2c) {
    if (!i2c) return;
    sda_high(i2c);
    scl_high(i2c);
    i2c_delay(i2c);
    sda_low(i2c);
    i2c_delay(i2c);
    scl_low(i2c);
    i2c_delay(i2c);
}

void syn_soft_i2c_stop(const SYN_SoftI2C *i2c) {
    if (!i2c) return;
    scl_low(i2c);
    i2c_delay(i2c);
    sda_low(i2c);
    i2c_delay(i2c);
    scl_high(i2c);
    i2c_delay(i2c);
    sda_high(i2c);
    i2c_delay(i2c);
}

bool syn_soft_i2c_write(const SYN_SoftI2C *i2c, uint8_t data) {
    if (!i2c) return false;
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        if (data & mask) {
            sda_high(i2c);
        } else {
            sda_low(i2c);
        }
        i2c_delay(i2c);
        scl_high(i2c);
        i2c_delay(i2c);
        scl_low(i2c);
    }
    
    // Read ACK
    sda_high(i2c); // Release SDA
    i2c_delay(i2c);
    scl_high(i2c);
    i2c_delay(i2c);
    bool ack = (syn_gpio_read(i2c->sda) == SYN_GPIO_LOW);
    scl_low(i2c);
    i2c_delay(i2c);
    
    return ack;
}

uint8_t syn_soft_i2c_read(const SYN_SoftI2C *i2c, bool ack) {
    if (!i2c) return 0;
    uint8_t data = 0;
    
    sda_high(i2c); // Release SDA
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        i2c_delay(i2c);
        scl_high(i2c);
        i2c_delay(i2c);
        if (syn_gpio_read(i2c->sda) == SYN_GPIO_HIGH) {
            data |= mask;
        }
        scl_low(i2c);
    }
    
    // Send ACK/NACK
    if (ack) {
        sda_low(i2c);
    } else {
        sda_high(i2c);
    }
    i2c_delay(i2c);
    scl_high(i2c);
    i2c_delay(i2c);
    scl_low(i2c);
    sda_high(i2c);
    
    return data;
}

bool syn_soft_i2c_write_read(SYN_SoftI2C *i2c, uint8_t dev_addr,
                              const uint8_t *tx_data, size_t tx_len,
                              uint8_t *rx_data, size_t rx_len)
{
    SYN_ASSERT(i2c != NULL);

    /* Write phase */
    if (tx_len > 0) {
        syn_soft_i2c_start(i2c);
        if (!syn_soft_i2c_write(i2c, (uint8_t)(dev_addr << 1))) {
            syn_soft_i2c_stop(i2c);
            return false; /* NACK on address */
        }
        for (size_t i = 0; i < tx_len; i++) {
            if (!syn_soft_i2c_write(i2c, tx_data[i])) {
                syn_soft_i2c_stop(i2c);
                return false; /* NACK on data */
            }
        }
    }

    /* Read phase */
    if (rx_len > 0) {
        syn_soft_i2c_start(i2c); /* Repeated start (or first start if tx_len==0) */
        if (!syn_soft_i2c_write(i2c, (uint8_t)((dev_addr << 1) | 1))) {
            syn_soft_i2c_stop(i2c);
            return false; /* NACK on address */
        }
        for (size_t i = 0; i < rx_len; i++) {
            rx_data[i] = syn_soft_i2c_read(i2c, (i < rx_len - 1));
        }
    }

    syn_soft_i2c_stop(i2c);
    return true;
}


#endif /* SYN_USE_SOFT_I2C */
