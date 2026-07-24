

# File syn\_soft\_i2c.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_i2c.h**](syn__soft__i2c_8h.md)

[Go to the documentation of this file](syn__soft__i2c_8h.md)


```C++

#ifndef SYN_SOFT_I2C_H
#define SYN_SOFT_I2C_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SYN_GPIO_Pin scl;              
    SYN_GPIO_Pin sda;              
    uint32_t delay_loops;          
} SYN_SoftI2C;

void syn_soft_i2c_init(SYN_SoftI2C *i2c, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint32_t delay_loops);

void syn_soft_i2c_start(const SYN_SoftI2C *i2c);

void syn_soft_i2c_stop(const SYN_SoftI2C *i2c);

bool syn_soft_i2c_write(const SYN_SoftI2C *i2c, uint8_t data);

uint8_t syn_soft_i2c_read(const SYN_SoftI2C *i2c, bool ack);

bool syn_soft_i2c_write_read(SYN_SoftI2C *i2c, uint8_t dev_addr,
                              const uint8_t *tx_data, size_t tx_len,
                              uint8_t *rx_data, size_t rx_len);

#ifdef __cplusplus
}
#endif
#endif // SYN_SOFT_I2C_H
```


