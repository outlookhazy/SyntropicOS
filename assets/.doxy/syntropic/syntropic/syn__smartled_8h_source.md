

# File syn\_smartled.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_smartled.h**](syn__smartled_8h.md)

[Go to the documentation of this file](syn__smartled_8h.md)


```C++

#ifndef SYN_SMARTLED_H
#define SYN_SMARTLED_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_SMARTLED_ORDER_GRB = 0, 
    SYN_SMARTLED_ORDER_RGB = 1, 
    SYN_SMARTLED_ORDER_RGBW = 2 
} SYN_SmartLEDOrder;

typedef struct {
    uint8_t r; 
    uint8_t g; 
    uint8_t b; 
    uint8_t w; 
} SYN_SmartLEDColor;

typedef struct {
    SYN_GPIO_Pin data_pin;        
    SYN_SmartLEDOrder order;      
    uint16_t num_leds;            
    uint8_t brightness;           
    SYN_SmartLEDColor *pixel_buf; 
} SYN_SmartLED;

SYN_Status syn_smartled_init(SYN_SmartLED *led, SYN_GPIO_Pin data_pin, uint16_t num_leds,
                             SYN_SmartLEDColor *pixel_buf, SYN_SmartLEDOrder order);

void syn_smartled_set_brightness(SYN_SmartLED *led, uint8_t brightness);

void syn_smartled_set_pixel_rgb(SYN_SmartLED *led, uint16_t index, uint8_t r, uint8_t g, uint8_t b);

void syn_smartled_set_pixel_hsv(SYN_SmartLED *led, uint16_t index, uint8_t h, uint8_t s, uint8_t v);

void syn_smartled_fill_rgb(SYN_SmartLED *led, uint8_t r, uint8_t g, uint8_t b);

void syn_smartled_clear(SYN_SmartLED *led);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SMARTLED_H */
```


