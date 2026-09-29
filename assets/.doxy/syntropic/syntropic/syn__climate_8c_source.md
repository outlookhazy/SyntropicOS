

# File syn\_climate.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_climate.c**](syn__climate_8c.md)

[Go to the documentation of this file](syn__climate_8c.md)


```C++

#include "syn_climate.h"

#include "../util/syn_assert.h"

#include <math.h>
#include <string.h>

SYN_Status syn_climate_init(SYN_Climate *sensor, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                            uint8_t i2c_addr, SYN_ClimateType type)
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

void syn_climate_feed_sample(SYN_Climate *sensor, float temp_c, float humidity_rh, float press_hpa)
{
    if (sensor == NULL)
        return;

    sensor->temperature_c = temp_c;
    sensor->humidity_rh = humidity_rh;
    sensor->pressure_hpa = press_hpa;

    /* Calculate Dew Point using Magnus-Tetens Formula */
    float a = 17.27f;
    float b = 237.7f;
    if (humidity_rh > 0.0f && (b + temp_c) != 0.0f) {
        float alpha = ((a * temp_c) / (b + temp_c)) + logf(humidity_rh / 100.0f);
        if (a != alpha) {
            sensor->dew_point_c = (b * alpha) / (a - alpha);
        } else {
            sensor->dew_point_c = temp_c; /* LCOV_EXCL_LINE: Singularity division-by-zero guard for
                                             Magnus-Tetens formula */
        }
    } else {
        sensor->dew_point_c = temp_c;
    }
}

float syn_climate_get_temp_c(const SYN_Climate *sensor)
{
    if (sensor == NULL)
        return 0.0f;
    return sensor->temperature_c;
}

float syn_climate_get_temp_f(const SYN_Climate *sensor)
{
    if (sensor == NULL)
        return 32.0f;
    return (sensor->temperature_c * 1.8f) + 32.0f;
}

float syn_climate_get_humidity(const SYN_Climate *sensor)
{
    if (sensor == NULL)
        return 0.0f;
    return sensor->humidity_rh;
}

float syn_climate_get_dew_point(const SYN_Climate *sensor)
{
    if (sensor == NULL)
        return 0.0f;
    return sensor->dew_point_c;
}
```


