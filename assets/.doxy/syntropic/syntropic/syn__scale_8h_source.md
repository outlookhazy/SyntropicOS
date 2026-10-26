

# File syn\_scale.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_scale.h**](syn__scale_8h.md)

[Go to the documentation of this file](syn__scale_8h.md)


```C++

#ifndef SYN_SCALE_H
#define SYN_SCALE_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_SCALE_HX711 = 0,  
    SYN_SCALE_NAU7802 = 1 
} SYN_ScaleType;

typedef struct {
    SYN_ScaleType type;      
    SYN_GPIO_Pin dout_pin;   
    SYN_GPIO_Pin sck_pin;    
    int32_t tare_offset;     
    float scale_factor;      
    float last_weight_grams; 
    bool is_stable;          
} SYN_Scale;

SYN_Status syn_scale_init(SYN_Scale *scale, SYN_GPIO_Pin dout_pin, SYN_GPIO_Pin sck_pin,
                          SYN_ScaleType type);

void syn_scale_feed_adc(SYN_Scale *scale, int32_t raw_adc24);

void syn_scale_tare(SYN_Scale *scale, int32_t tare_offset);

void syn_scale_set_calibration_factor(SYN_Scale *scale, float factor);

float syn_scale_get_grams(const SYN_Scale *scale);

float syn_scale_get_kg(const SYN_Scale *scale);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SCALE_H */
```


