

# File syn\_sensor.h

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor.h**](syn__sensor_8h.md)

[Go to the documentation of this file](syn__sensor_8h.md)


```C++

#ifndef SYN_SENSOR_H
#define SYN_SENSOR_H

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"
#include "../dsp/syn_filter.h"
#include "../dsp/syn_signal.h"
#include "../util/syn_hysteresis.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback types ─────────────────────────────────────────────────────── */

struct SYN_Sensor;

typedef int16_t (*SYN_SensorReadFunc)(void *ctx);

typedef void (*SYN_SensorThresholdCallback)(struct SYN_Sensor *sensor,
                                              int16_t value, void *ctx);

/* ── Filter type ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_SENSOR_FILTER_NONE   = 0,  
    SYN_SENSOR_FILTER_MA     = 1,  
    SYN_SENSOR_FILTER_EMA    = 2,  
    SYN_SENSOR_FILTER_MEDIAN = 3,  
} SYN_SensorFilterType;

/* ── Sensor descriptor ──────────────────────────────────────────────────── */

typedef struct SYN_Sensor {
    /* Identity */
    const char         *name;           
    /* Read function */
    SYN_SensorReadFunc read_func;       
    void               *read_ctx;       
    /* Polling */
    uint32_t            interval_ms;    
    uint32_t            last_poll_tick;  
    bool                enabled;        
    /* Values */
    int16_t             raw;            
    int16_t             filtered;       
    /* Filter (union would save RAM but complicates API) */
    uint8_t             filter_type;    
    void               *filter;         
    /* Threshold */
    bool                threshold_enabled; 
    SYN_Hysteresis     hyst;            
    SYN_SensorThresholdCallback on_high; 
    SYN_SensorThresholdCallback on_low;  
    void               *threshold_ctx;  
    /* Statistics (optional — set via syn_sensor_set_stats) */
    SYN_Signal        *stats;           
} SYN_Sensor;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sensor_init(SYN_Sensor *sensor, const char *name,
                      SYN_SensorReadFunc read_func, void *ctx);

void syn_sensor_set_interval(SYN_Sensor *sensor, uint32_t interval_ms);

void syn_sensor_enable(SYN_Sensor *sensor, bool enable);

/* ── Filter setup ───────────────────────────────────────────────────────── */

void syn_sensor_set_filter_ma(SYN_Sensor *sensor, SYN_FilterMA *f);

void syn_sensor_set_filter_ema(SYN_Sensor *sensor, SYN_FilterEMA *f);

void syn_sensor_set_filter_median(SYN_Sensor *sensor, SYN_FilterMedian *f);

void syn_sensor_clear_filter(SYN_Sensor *sensor);

/* ── Threshold setup ────────────────────────────────────────────────────── */

void syn_sensor_set_threshold(SYN_Sensor *sensor,
                               int32_t threshold, int32_t band,
                               SYN_SensorThresholdCallback on_high,
                               SYN_SensorThresholdCallback on_low,
                               void *ctx);

void syn_sensor_clear_threshold(SYN_Sensor *sensor);

/* ── Statistics ─────────────────────────────────────────────────────────── */

void syn_sensor_set_stats(SYN_Sensor *sensor, SYN_Signal *stats);

/* ── Update ─────────────────────────────────────────────────────────────── */

bool syn_sensor_update(SYN_Sensor *sensor);

int16_t syn_sensor_read_now(SYN_Sensor *sensor);

void syn_sensor_service(SYN_Sensor *sensors, size_t count);

static inline int16_t syn_sensor_value(const SYN_Sensor *sensor)
{
    return sensor->filtered;
}

static inline int16_t syn_sensor_raw(const SYN_Sensor *sensor)
{
    return sensor->raw;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SENSOR_H */
```


