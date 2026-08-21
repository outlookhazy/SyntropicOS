

# File syn\_lux.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_lux.h**](syn__lux_8h.md)

[Go to the documentation of this file](syn__lux_8h.md)


```C++

#ifndef SYN_LUX_H
#define SYN_LUX_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_LUX_BH1750 = 0,  
    SYN_LUX_TSL2561 = 1, 
    SYN_LUX_TCS34725 = 2 
} SYN_LuxType;

typedef struct {
    SYN_LuxType type;      
    SYN_SoftI2C i2c;       
    uint8_t i2c_addr;      
    float illuminance_lux; 
    uint16_t color_r;      
    uint16_t color_g;      
    uint16_t color_b;      
    uint16_t color_c;      
    uint16_t color_temp_k; 
} SYN_Lux;

SYN_Status syn_lux_init(SYN_Lux *sensor, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                        SYN_LuxType type);

void syn_lux_feed_lux(SYN_Lux *sensor, float lux);

void syn_lux_feed_rgbc(SYN_Lux *sensor, uint16_t r, uint16_t g, uint16_t b, uint16_t c);

float syn_lux_get_lux(const SYN_Lux *sensor);

uint16_t syn_lux_get_color_temp_k(const SYN_Lux *sensor);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LUX_H */
```


