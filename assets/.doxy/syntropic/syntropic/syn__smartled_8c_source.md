

# File syn\_smartled.c

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_smartled.c**](syn__smartled_8c.md)

[Go to the documentation of this file](syn__smartled_8c.md)


```C++

#include "syn_smartled.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_smartled_init(SYN_SmartLED *led, SYN_GPIO_Pin data_pin, uint16_t num_leds,
                             SYN_SmartLEDColor *pixel_buf, SYN_SmartLEDOrder order)
{
    SYN_ASSERT(led != NULL);
    SYN_ASSERT(num_leds > 0);
    SYN_ASSERT(pixel_buf != NULL);

    memset(led, 0, sizeof(*led));
    led->data_pin = data_pin;
    led->num_leds = num_leds;
    led->pixel_buf = pixel_buf;
    led->order = order;
    led->brightness = 255; /* Default full brightness */

    memset(pixel_buf, 0, sizeof(SYN_SmartLEDColor) * num_leds);
    syn_port_gpio_init(data_pin, SYN_GPIO_OUTPUT);

    return SYN_OK;
}

void syn_smartled_set_brightness(SYN_SmartLED *led, uint8_t brightness)
{
    if (led == NULL)
        return;
    led->brightness = brightness;
}

void syn_smartled_set_pixel_rgb(SYN_SmartLED *led, uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (led == NULL || led->pixel_buf == NULL || index >= led->num_leds)
        return;

    /* Scale by global brightness */
    uint16_t br = (uint16_t)led->brightness + 1;
    led->pixel_buf[index].r = (uint8_t)(((uint16_t)r * br) >> 8);
    led->pixel_buf[index].g = (uint8_t)(((uint16_t)g * br) >> 8);
    led->pixel_buf[index].b = (uint8_t)(((uint16_t)b * br) >> 8);
    led->pixel_buf[index].w = 0;
}

void syn_smartled_set_pixel_hsv(SYN_SmartLED *led, uint16_t index, uint8_t h, uint8_t s, uint8_t v)
{
    if (led == NULL || led->pixel_buf == NULL || index >= led->num_leds)
        return;

    uint8_t r, g, b;
    if (s == 0) {
        r = g = b = v;
    } else {
        uint8_t region = h / 43;
        uint8_t remainder = (uint8_t)((h - (region * 43)) * 6);

        uint8_t p = (uint8_t)((v * (255 - s)) >> 8);
        uint8_t q = (uint8_t)((v * (255 - ((s * remainder) >> 8))) >> 8);
        uint8_t t = (uint8_t)((v * (255 - ((s * (255 - remainder)) >> 8))) >> 8);

        switch (region) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:
            r = v;
            g = p;
            b = q;
            break;
        }
    }

    syn_smartled_set_pixel_rgb(led, index, r, g, b);
}

void syn_smartled_fill_rgb(SYN_SmartLED *led, uint8_t r, uint8_t g, uint8_t b)
{
    if (led == NULL || led->pixel_buf == NULL)
        return;

    for (uint16_t i = 0; i < led->num_leds; i++) {
        syn_smartled_set_pixel_rgb(led, i, r, g, b);
    }
}

void syn_smartled_clear(SYN_SmartLED *led)
{
    if (led == NULL || led->pixel_buf == NULL)
        return;
    syn_smartled_fill_rgb(led, 0, 0, 0);
}
```


