

# File syn\_rc\_curve.c

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_curve.c**](syn__rc__curve_8c.md)

[Go to the documentation of this file](syn__rc__curve_8c.md)


```C++

#include "syntropic/control/syn_rc_curve.h"

uint16_t syn_rc_curve_apply(uint16_t input_us, const SYN_RCCurve_Config *config)
{
    if (!config) {
        return input_us;
    }

    if (input_us < 1000U) {
        input_us = 1000U;
    }
    if (input_us > 2000U) {
        input_us = 2000U;
    }

    /* Center offset x in range [-500, +500] us */
    int32_t offset = (int32_t)input_us - 1500;

    /* Apply deadband filtering */
    if (offset > 0) {
        if (offset <= (int32_t)config->deadband_us) {
            offset = 0;
        } else {
            offset -= (int32_t)config->deadband_us;
        }
    } else if (offset < 0) {
        if (-offset <= (int32_t)config->deadband_us) {
            offset = 0;
        } else {
            offset += (int32_t)config->deadband_us;
        }
    }

    if (offset == 0) {
        return 1500U;
    }

    /* Convert offset to normalized Q16.16 in range [-1.0, +1.0] */
    int32_t span = 500 - (int32_t)config->deadband_us;
    if (span <= 0) {
        return 1500U; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    q16_t x = Q16_FROM_INT(offset) / span;

    /* Polynomial: y = (1 - expo) * x + expo * x^3 */
    q16_t x_cube = q16_mul(q16_mul(x, x), x);
    q16_t linear_term = q16_mul(Q16_ONE - config->expo, x);
    q16_t cubic_term = q16_mul(config->expo, x_cube);
    q16_t y = linear_term + cubic_term;

    /* Apply dual-rate scaling */
    q16_t y_scaled = q16_mul(y, config->dual_rate);

    /* Convert back to microsecond offset */
    int32_t out_offset = Q16_TO_INT(q16_mul(y_scaled, Q16_FROM_INT(500)));
    int32_t final_us = 1500 + out_offset;

    if (final_us < 1000) {
        final_us = 1000;
    }
    if (final_us > 2000) {
        final_us = 2000;
    }

    return (uint16_t)final_us;
}
```


