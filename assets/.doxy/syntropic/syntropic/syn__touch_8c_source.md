

# File syn\_touch.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_touch.c**](syn__touch_8c.md)

[Go to the documentation of this file](syn__touch_8c.md)


```C++

#include "syn_touch.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_touch_init(SYN_Touch *touch, SYN_GPIO_Pin pin, uint16_t threshold)
{
    SYN_ASSERT(touch != NULL);

    memset(touch, 0, sizeof(*touch));
    touch->pin = pin;
    touch->threshold = threshold;
    touch->hysteresis = threshold / 4;

    syn_port_gpio_init(pin, SYN_GPIO_INPUT);

    return SYN_OK;
}

void syn_touch_calibrate(SYN_Touch *touch, uint16_t baseline)
{
    if (touch == NULL)
        return;
    touch->baseline = baseline;
}

void syn_touch_feed_sample(SYN_Touch *touch, uint16_t raw_sample)
{
    if (touch == NULL)
        return;

    touch->current_val = raw_sample;

    /* Initialize baseline on first sample */
    if (touch->baseline == 0) {
        touch->baseline = raw_sample;
    }

    /* Compute capacitance delta relative to baseline */
    int32_t delta = (int32_t)raw_sample - (int32_t)touch->baseline;
    if (delta < 0)
        delta = -delta;

    if (!touch->is_pressed) {
        if (delta >= (int32_t)touch->threshold) {
            touch->is_pressed = true;
            touch->press_count++;
        } else {
            /* Slow baseline drift tracking when idle */
            touch->baseline = (uint16_t)(((uint32_t)touch->baseline * 15 + raw_sample) / 16);
        }
    } else {
        int32_t release_thresh = (int32_t)touch->threshold - (int32_t)touch->hysteresis;
        /* LCOV_EXCL_START: Defensive release threshold clamp */
        if (release_thresh < 0)
            release_thresh = 0;
        /* LCOV_EXCL_STOP */
        if (delta < release_thresh) {
            touch->is_pressed = false;
        }
    }
}

bool syn_touch_is_pressed(const SYN_Touch *touch)
{
    return touch != NULL && touch->is_pressed;
}
```


