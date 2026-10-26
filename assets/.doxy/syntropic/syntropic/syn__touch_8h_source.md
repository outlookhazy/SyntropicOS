

# File syn\_touch.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_touch.h**](syn__touch_8h.md)

[Go to the documentation of this file](syn__touch_8h.md)


```C++

#ifndef SYN_TOUCH_H
#define SYN_TOUCH_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SYN_GPIO_Pin pin;     
    uint16_t baseline;    
    uint16_t threshold;   
    uint16_t current_val; 
    uint16_t hysteresis;  
    bool is_pressed;      
    uint32_t press_count; 
} SYN_Touch;

SYN_Status syn_touch_init(SYN_Touch *touch, SYN_GPIO_Pin pin, uint16_t threshold);

void syn_touch_feed_sample(SYN_Touch *touch, uint16_t raw_sample);

void syn_touch_calibrate(SYN_Touch *touch, uint16_t baseline);

bool syn_touch_is_pressed(const SYN_Touch *touch);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TOUCH_H */
```


