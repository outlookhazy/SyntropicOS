

# File syn\_distance.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_distance.c**](syn__distance_8c.md)

[Go to the documentation of this file](syn__distance_8c.md)


```C++

#include "syn_distance.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_distance_init(SYN_Distance *sensor, SYN_GPIO_Pin trig_pin, SYN_GPIO_Pin echo_pin,
                             uint32_t min_mm, uint32_t max_mm, SYN_DistanceType type)
{
    SYN_ASSERT(sensor != NULL);
    SYN_ASSERT(max_mm > min_mm);

    if (sensor == NULL || max_mm <= min_mm) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL and boundary check after
                                     SYN_ASSERT macro in release mode */
    }

    memset(sensor, 0, sizeof(*sensor));
    sensor->type = type;
    sensor->trig_pin = trig_pin;
    sensor->echo_pin = echo_pin;
    sensor->min_range_mm = min_mm;
    sensor->max_range_mm = max_mm;
    sensor->proximity_thresh_mm = 100; /* 100mm default */

    return SYN_OK;
}

void syn_distance_feed_pulse(SYN_Distance *sensor, uint32_t pulse_us)
{
    if (sensor == NULL)
        return;

    uint32_t dist_mm = 0;
    if (sensor->type == SYN_DISTANCE_ULTRASONIC) {
        /* Speed of sound = 343 m/s -> dist = (pulse_us * 0.343) / 2 = pulse_us / 5.83 */
        dist_mm = (uint32_t)(((uint64_t)pulse_us * 100u) / 583u);
    } else {
        /* TOF laser or pre-converted mm input */
        dist_mm = pulse_us;
    }

    if (dist_mm < sensor->min_range_mm)
        dist_mm = sensor->min_range_mm;
    if (dist_mm > sensor->max_range_mm)
        dist_mm = sensor->max_range_mm;

    sensor->last_distance_mm = dist_mm;
    sensor->obstacle_detected = (dist_mm <= sensor->proximity_thresh_mm);
}

void syn_distance_set_proximity_threshold(SYN_Distance *sensor, uint32_t thresh_mm)
{
    if (sensor == NULL)
        return;
    sensor->proximity_thresh_mm = thresh_mm;
}

uint32_t syn_distance_get_mm(const SYN_Distance *sensor)
{
    if (sensor == NULL)
        return 0;
    return sensor->last_distance_mm;
}

uint32_t syn_distance_get_cm(const SYN_Distance *sensor)
{
    if (sensor == NULL)
        return 0;
    return sensor->last_distance_mm / 10u;
}

bool syn_distance_is_obstacle_detected(const SYN_Distance *sensor)
{
    if (sensor == NULL)
        return false;
    return sensor->obstacle_detected;
}
```


