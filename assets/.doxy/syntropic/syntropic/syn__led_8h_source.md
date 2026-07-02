

# File syn\_led.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_led.h**](syn__led_8h.md)

[Go to the documentation of this file](syn__led_8h.md)


```C++

#ifndef SYN_LED_H
#define SYN_LED_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── LED polarity ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_LED_ACTIVE_HIGH = 0,  
    SYN_LED_ACTIVE_LOW  = 1,  
} SYN_LEDPolarity;

/* ── LED mode ───────────────────────────────────────────────────────────── */

typedef enum {
    SYN_LED_MODE_OFF     = 0,  
    SYN_LED_MODE_ON      = 1,  
    SYN_LED_MODE_BLINK   = 2,  
    SYN_LED_MODE_FLASH   = 3,  
    SYN_LED_MODE_PATTERN = 4,  
} SYN_LEDMode;

/* ── LED descriptor ─────────────────────────────────────────────────────── */

typedef struct {
    /* Configuration */
    SYN_GPIO_Pin       pin;            
    uint8_t             polarity;      
    /* State */
    uint8_t             mode;          
    bool                lit;           
    uint32_t            tick;          
    /* Blink / flash */
    uint16_t            on_ms;         
    uint16_t            off_ms;        
    uint8_t             flash_remain;  
    /* Pattern */
    const char         *pattern;       
    uint8_t             pattern_idx;   
    uint16_t            pattern_unit;  
} SYN_LED;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_led_init(SYN_LED *led, SYN_GPIO_Pin pin, SYN_LEDPolarity polarity);

void syn_led_on(SYN_LED *led);

void syn_led_off(SYN_LED *led);

void syn_led_toggle(SYN_LED *led);

void syn_led_blink(SYN_LED *led, uint16_t on_ms, uint16_t off_ms);

void syn_led_flash(SYN_LED *led, uint16_t on_ms, uint16_t off_ms,
                    uint8_t count);

void syn_led_pattern(SYN_LED *led, const char *pattern, uint16_t unit_ms);

void syn_led_update(SYN_LED *led);

void syn_led_service(SYN_LED *leds, size_t count);

static inline bool syn_led_is_on(const SYN_LED *led)
{
    return led->lit;
}

uint32_t syn_led_next_ms(const SYN_LED *led);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LED_H */
```


