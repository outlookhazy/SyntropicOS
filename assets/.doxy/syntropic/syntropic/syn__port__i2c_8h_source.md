

# File syn\_port\_i2c.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c.h**](syn__port__i2c_8h.md)

[Go to the documentation of this file](syn__port__i2c_8h.md)


```C++

#ifndef SYN_PORT_I2C_H
#define SYN_PORT_I2C_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_i2c_init(uint8_t i2c_id, uint32_t clock_speed_hz, uint8_t role,
                             uint16_t own_addr);

SYN_Status syn_port_i2c_deinit(uint8_t i2c_id);

SYN_Status syn_port_i2c_transfer(uint8_t i2c_id, uint16_t addr, const uint8_t *tx, size_t tx_len,
                                 uint8_t *rx, size_t rx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_I2C_H */
```


