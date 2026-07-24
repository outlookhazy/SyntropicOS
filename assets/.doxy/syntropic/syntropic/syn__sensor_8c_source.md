

# File syn\_sensor.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor.c**](syn__sensor_8c.md)

[Go to the documentation of this file](syn__sensor_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SENSOR) || SYN_USE_SENSOR

#include "syn_sensor.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sensor_init(SYN_Sensor *sensor, const char *name,
                      SYN_SensorReadFunc read_func, void *ctx)
{
    SYN_ASSERT(sensor != NULL);
    SYN_ASSERT(read_func != NULL);

    memset(sensor, 0, sizeof(*sensor));
    sensor->name      = name;
    sensor->read_func = read_func;
    sensor->read_ctx  = ctx;
    sensor->enabled   = true;
}

void syn_sensor_set_interval(SYN_Sensor *sensor, uint32_t interval_ms)
{
    SYN_ASSERT(sensor != NULL);
    sensor->interval_ms    = interval_ms;
    sensor->last_poll_tick = syn_port_get_tick_ms();
}

void syn_sensor_enable(SYN_Sensor *sensor, bool enable)
{
    SYN_ASSERT(sensor != NULL);
    sensor->enabled = enable;
    if (enable) {
        sensor->last_poll_tick = syn_port_get_tick_ms();
    }
}

/* ── Filter setup ───────────────────────────────────────────────────────── */

void syn_sensor_set_filter_ma(SYN_Sensor *sensor, SYN_FilterMA *f)
{
    SYN_ASSERT(sensor != NULL);
    sensor->filter_type = (uint8_t)SYN_SENSOR_FILTER_MA;
    sensor->filter      = f;
}

void syn_sensor_set_filter_ema(SYN_Sensor *sensor, SYN_FilterEMA *f)
{
    SYN_ASSERT(sensor != NULL);
    sensor->filter_type = (uint8_t)SYN_SENSOR_FILTER_EMA;
    sensor->filter      = f;
}

void syn_sensor_set_filter_median(SYN_Sensor *sensor, SYN_FilterMedian *f)
{
    SYN_ASSERT(sensor != NULL);
    sensor->filter_type = (uint8_t)SYN_SENSOR_FILTER_MEDIAN;
    sensor->filter      = f;
}

void syn_sensor_clear_filter(SYN_Sensor *sensor)
{
    SYN_ASSERT(sensor != NULL);
    sensor->filter_type = (uint8_t)SYN_SENSOR_FILTER_NONE;
    sensor->filter      = NULL;
}

/* ── Threshold setup ────────────────────────────────────────────────────── */

void syn_sensor_set_threshold(SYN_Sensor *sensor,
                               int32_t threshold, int32_t band,
                               SYN_SensorThresholdCallback on_high,
                               SYN_SensorThresholdCallback on_low,
                               void *ctx)
{
    SYN_ASSERT(sensor != NULL);
    sensor->threshold_enabled = true;
    syn_hyst_init(&sensor->hyst, threshold, band, false);
    sensor->on_high       = on_high;
    sensor->on_low        = on_low;
    sensor->threshold_ctx = ctx;
}

void syn_sensor_clear_threshold(SYN_Sensor *sensor)
{
    SYN_ASSERT(sensor != NULL);
    sensor->threshold_enabled = false;
}

static int16_t apply_filter(SYN_Sensor *sensor, int16_t raw)
{
    switch ((SYN_SensorFilterType)sensor->filter_type) {
    case SYN_SENSOR_FILTER_MA:
        return syn_filter_ma_update((SYN_FilterMA *)sensor->filter, raw);
    case SYN_SENSOR_FILTER_EMA:
        return syn_filter_ema_update((SYN_FilterEMA *)sensor->filter, raw);
    case SYN_SENSOR_FILTER_MEDIAN:
        return syn_filter_median_update((SYN_FilterMedian *)sensor->filter, raw);
    default:
        return raw;
    }
}

/* ── Update ─────────────────────────────────────────────────────────────── */

static void do_read(SYN_Sensor *sensor)
{
    sensor->raw = sensor->read_func(sensor->read_ctx);

    /* Filter */
    if (sensor->filter != NULL) {
        sensor->filtered = apply_filter(sensor, sensor->raw);
    } else {
        sensor->filtered = sensor->raw;
    }

    /* Threshold check */
    if (sensor->threshold_enabled) {
        bool was_high = syn_hyst_state(&sensor->hyst);
        bool now_high = syn_hyst_update(&sensor->hyst, sensor->filtered);

        if (now_high && !was_high) {
            if (sensor->on_high != NULL) {
                sensor->on_high(sensor, sensor->filtered,
                                sensor->threshold_ctx);
            }
        } else if (!now_high && was_high) {
            if (sensor->on_low != NULL) {
                sensor->on_low(sensor, sensor->filtered,
                               sensor->threshold_ctx);
            }
        }
    }

    /* Push to statistics window (if attached) */
    if (sensor->stats != NULL) {
        syn_signal_push(sensor->stats, (int32_t)sensor->filtered);
    }
}

bool syn_sensor_update(SYN_Sensor *sensor)
{
    SYN_ASSERT(sensor != NULL);

    if (!sensor->enabled) return false;

    uint32_t now = syn_port_get_tick_ms();
    if ((now - sensor->last_poll_tick) < sensor->interval_ms) {
        return false;
    }

    sensor->last_poll_tick = now;
    do_read(sensor);
    return true;
}

int16_t syn_sensor_read_now(SYN_Sensor *sensor)
{
    SYN_ASSERT(sensor != NULL);
    do_read(sensor);
    sensor->last_poll_tick = syn_port_get_tick_ms();
    return sensor->filtered;
}

void syn_sensor_service(SYN_Sensor *sensors, size_t count)
{
    SYN_ASSERT(sensors != NULL || count == 0);
    for (size_t i = 0; i < count; i++) {
        syn_sensor_update(&sensors[i]);
    }
}

void syn_sensor_set_stats(SYN_Sensor *sensor, SYN_Signal *stats)
{
    SYN_ASSERT(sensor != NULL);
    sensor->stats = stats;
}

#endif /* SYN_USE_SENSOR */
```


