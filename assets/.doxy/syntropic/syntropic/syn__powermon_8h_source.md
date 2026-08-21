

# File syn\_powermon.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_powermon.h**](syn__powermon_8h.md)

[Go to the documentation of this file](syn__powermon_8h.md)


```C++

#ifndef SYN_POWERMON_H
#define SYN_POWERMON_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_POWERMON_INA219 = 0, 
    SYN_POWERMON_INA226 = 1  
} SYN_PowerMonType;

typedef struct {
    SYN_PowerMonType type;     
    SYN_SoftI2C i2c;           
    uint8_t i2c_addr;          
    float shunt_resistor_ohms; 
    float bus_voltage_v;       
    float shunt_current_ma;    
    float power_mw;            
    float energy_mwh;          
} SYN_PowerMon;

SYN_Status syn_powermon_init(SYN_PowerMon *pm, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                             float shunt_resistor_ohms, SYN_PowerMonType type);

void syn_powermon_feed_raw(SYN_PowerMon *pm, uint16_t raw_bus_v, float raw_shunt_mv);

float syn_powermon_get_bus_voltage(const SYN_PowerMon *pm);

float syn_powermon_get_current_ma(const SYN_PowerMon *pm);

float syn_powermon_get_power_mw(const SYN_PowerMon *pm);

#ifdef __cplusplus
}
#endif

#endif /* SYN_POWERMON_H */
```


