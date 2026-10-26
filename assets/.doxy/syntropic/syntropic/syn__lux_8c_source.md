

# File syn\_lux.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_lux.c**](syn__lux_8c.md)

[Go to the documentation of this file](syn__lux_8c.md)


```C++

#include "syn_lux.h"

#include "../util/syn_assert.h"

#include <math.h>
#include <string.h>

SYN_Status syn_lux_init(SYN_Lux *sensor, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                        SYN_LuxType type)
{
    SYN_ASSERT(sensor != NULL);
    if (sensor == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    memset(sensor, 0, sizeof(*sensor));
    sensor->type = type;
    sensor->i2c_addr = i2c_addr;

    syn_soft_i2c_init(&sensor->i2c, scl, sda, 10);

    return SYN_OK;
}

void syn_lux_feed_lux(SYN_Lux *sensor, float lux)
{
    if (sensor == NULL)
        return;
    sensor->illuminance_lux = lux;
}

void syn_lux_feed_rgbc(SYN_Lux *sensor, uint16_t r, uint16_t g, uint16_t b, uint16_t c)
{
    if (sensor == NULL)
        return;

    sensor->color_r = r;
    sensor->color_g = g;
    sensor->color_b = b;
    sensor->color_c = c;

    /* Calculate Lux from Clear channel */
    sensor->illuminance_lux = (float)c * 0.5f;

    /* Calculate Color Temperature in Kelvin (McCamy formula) */
    if (c > 0) {
        float rf = (float)r / (float)c;
        float gf = (float)g / (float)c;
        float bf = (float)b / (float)c;

        float x = (-0.14282f * rf) + (0.15492f * gf) + (-0.00000f * bf);
        float y = (-0.32466f * rf) + (1.57837f * gf) + (-0.73191f * bf);

        if (fabsf(0.1858f - y) > 0.0001f) {
            float n = (x - 0.3320f) / (0.1858f - y);
            float cct = (449.0f * n * n * n) + (3525.0f * n * n) + (6823.3f * n) + 5520.33f;
            if (cct < 1000.0f)
                cct = 1000.0f;
            if (cct > 20000.0f)
                cct = 20000.0f;
            sensor->color_temp_k = (uint16_t)cct;
        }
    }
}

float syn_lux_get_lux(const SYN_Lux *sensor)
{
    if (sensor == NULL)
        return 0.0f;
    return sensor->illuminance_lux;
}

uint16_t syn_lux_get_color_temp_k(const SYN_Lux *sensor)
{
    if (sensor == NULL)
        return 0;
    return sensor->color_temp_k;
}
```


