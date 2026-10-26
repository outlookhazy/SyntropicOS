

# File syn\_i2c.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_i2c.h**](syn__i2c_8h.md)

[Go to the documentation of this file](syn__i2c_8h.md)


```C++

#ifndef SYN_I2C_H
#define SYN_I2C_H

#include "../common/syn_defs.h"
#include "../port/syn_port_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_I2C_ROLE_MASTER = 0, 
    SYN_I2C_ROLE_SLAVE       
} SYN_I2C_Role;

typedef struct {
    uint8_t i2c_id;          
    uint32_t clock_speed_hz; 
    SYN_I2C_Role role;       
    uint16_t own_address;    
    bool use_dma;            
} SYN_I2C_Config;

typedef struct {
    SYN_I2C_Config cfg; 
    bool initialized;   
} SYN_I2C;

SYN_Status syn_i2c_init(SYN_I2C *i2c, const SYN_I2C_Config *cfg);

SYN_Status syn_i2c_deinit(SYN_I2C *i2c);

SYN_Status syn_i2c_transfer(SYN_I2C *i2c, uint16_t addr, const uint8_t *tx, size_t tx_len,
                            uint8_t *rx, size_t rx_len);

SYN_Status syn_i2c_read_reg(SYN_I2C *i2c, uint16_t addr, uint8_t reg, uint8_t *val);

SYN_Status syn_i2c_write_reg(SYN_I2C *i2c, uint16_t addr, uint8_t reg, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_I2C_H */
```


