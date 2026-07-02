

# File syn\_port\_i2c.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c.h**](syn__port__i2c_8h.md)

[Go to the documentation of this file](syn__port__i2c_8h.md)


```C++

#ifndef SYN_PORT_I2C_H
#define SYN_PORT_I2C_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── I2C configuration ──────────────────────────────────────────────────── */

typedef struct {
    uint8_t   bus;            
    uint32_t  clock_hz;       
} SYN_I2C_Config;

/* ── Port functions (user implements) ───────────────────────────────────── */

SYN_Status syn_port_i2c_init(const SYN_I2C_Config *cfg);

SYN_Status syn_port_i2c_deinit(uint8_t bus);

SYN_Status syn_port_i2c_write(uint8_t bus, uint8_t addr,
                                const uint8_t *data, size_t len);

SYN_Status syn_port_i2c_read(uint8_t bus, uint8_t addr,
                               uint8_t *data, size_t len);

SYN_Status syn_port_i2c_write_read(uint8_t bus, uint8_t addr,
                                     const uint8_t *tx_data, size_t tx_len,
                                     uint8_t *rx_data, size_t rx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_I2C_H */
```


