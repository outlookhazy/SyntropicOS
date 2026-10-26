

# File syn\_scale.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_scale.c**](syn__scale_8c.md)

[Go to the documentation of this file](syn__scale_8c.md)


```C++

#include "syn_scale.h"

#include "../util/syn_assert.h"

#include <math.h>
#include <string.h>

SYN_Status syn_scale_init(SYN_Scale *scale, SYN_GPIO_Pin dout_pin, SYN_GPIO_Pin sck_pin,
                          SYN_ScaleType type)
{
    SYN_ASSERT(scale != NULL);
    if (scale == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    memset(scale, 0, sizeof(*scale));
    scale->type = type;
    scale->dout_pin = dout_pin;
    scale->sck_pin = sck_pin;
    scale->scale_factor = 1.0f; /* 1.0 default */
    scale->is_stable = true;

    return SYN_OK;
}

void syn_scale_feed_adc(SYN_Scale *scale, int32_t raw_adc24)
{
    if (scale == NULL)
        return;

    int32_t delta = raw_adc24 - scale->tare_offset;
    if (scale->scale_factor != 0.0f) {
        scale->last_weight_grams = (float)delta / scale->scale_factor;
    } else {
        scale->last_weight_grams = 0.0f;
    }
}

void syn_scale_tare(SYN_Scale *scale, int32_t tare_offset)
{
    if (scale == NULL)
        return;
    scale->tare_offset = tare_offset;
}

void syn_scale_set_calibration_factor(SYN_Scale *scale, float factor)
{
    if (scale == NULL || factor == 0.0f)
        return;
    scale->scale_factor = factor;
}

float syn_scale_get_grams(const SYN_Scale *scale)
{
    if (scale == NULL)
        return 0.0f;
    return scale->last_weight_grams;
}

float syn_scale_get_kg(const SYN_Scale *scale)
{
    if (scale == NULL)
        return 0.0f;
    return scale->last_weight_grams / 1000.0f;
}
```


