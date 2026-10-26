

# File syn\_climate.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_climate.h**](syn__climate_8h.md)

[Go to the documentation of this file](syn__climate_8h.md)


```C++

#ifndef SYN_CLIMATE_H
#define SYN_CLIMATE_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_CLIMATE_SHT3X = 0,  
    SYN_CLIMATE_BME280 = 1, 
    SYN_CLIMATE_HTU21D = 2  
} SYN_ClimateType;

typedef struct {
    SYN_ClimateType type; 
    SYN_SoftI2C i2c;      
    uint8_t i2c_addr;     
    float temperature_c;  
    float humidity_rh;    
    float pressure_hpa;   
    float dew_point_c;    
} SYN_Climate;

SYN_Status syn_climate_init(SYN_Climate *sensor, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                            uint8_t i2c_addr, SYN_ClimateType type);

void syn_climate_feed_sample(SYN_Climate *sensor, float temp_c, float humidity_rh, float press_hpa);

float syn_climate_get_temp_c(const SYN_Climate *sensor);

float syn_climate_get_temp_f(const SYN_Climate *sensor);

float syn_climate_get_humidity(const SYN_Climate *sensor);

float syn_climate_get_dew_point(const SYN_Climate *sensor);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CLIMATE_H */
```


