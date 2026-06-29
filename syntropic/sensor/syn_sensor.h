/**
 * @file syn_sensor.h
 * @brief Sensor polling framework — periodic read → filter → threshold → event.
 *
 * Wires up the full sensor pipeline: poll at an interval, filter the
 * reading, compare against thresholds (with hysteresis), and fire
 * callbacks or set event flags.
 *
 * @par Usage
 * @code
 *   static SYN_Sensor temp_sensor;
 *   static SYN_FilterEMA temp_filter;
 *   syn_filter_ema_init(&temp_filter, 64);
 *
 *   syn_sensor_init(&temp_sensor, "temp", read_temperature, NULL);
 *   syn_sensor_set_interval(&temp_sensor, 1000);      // poll every 1s
 *   syn_sensor_set_filter_ema(&temp_sensor, &temp_filter);
 *   syn_sensor_set_threshold(&temp_sensor, 8000, 500,  // 80.00°C ± 5°C
 *                              on_temp_high, on_temp_low, NULL);
 *
 *   // In main loop:
 *   syn_sensor_update(&temp_sensor);
 * @endcode
 * @ingroup syn_io
 */

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

/**
 * @brief Read function — returns the raw sensor value.
 * @param ctx  User context pointer.
 * @return Raw sensor reading.
 */
typedef int16_t (*SYN_SensorReadFunc)(void *ctx);

/**
 * @brief Threshold callback — called when threshold is crossed.
 * @param sensor  The sensor that fired.
 * @param value   Current filtered value.
 * @param ctx     User context pointer.
 */
typedef void (*SYN_SensorThresholdCallback)(struct SYN_Sensor *sensor,
                                              int16_t value, void *ctx);

/* ── Filter type ────────────────────────────────────────────────────────── */

/** @brief Filter type selector for sensor pipeline. */
typedef enum {
    SYN_SENSOR_FILTER_NONE   = 0,  /**< No filter.              */
    SYN_SENSOR_FILTER_MA     = 1,  /**< Moving average.         */
    SYN_SENSOR_FILTER_EMA    = 2,  /**< Exponential moving avg. */
    SYN_SENSOR_FILTER_MEDIAN = 3,  /**< Median filter.          */
} SYN_SensorFilterType;

/* ── Sensor descriptor ──────────────────────────────────────────────────── */

/** @brief Sensor descriptor — owns the full read→filter→threshold pipeline. */
typedef struct SYN_Sensor {
    /* Identity */
    const char         *name;           /**< Human-readable name           */

    /* Read function */
    SYN_SensorReadFunc read_func;       /**< Hardware read callback        */
    void               *read_ctx;       /**< Context for read callback     */

    /* Polling */
    uint32_t            interval_ms;    /**< Poll interval in ms           */
    uint32_t            last_poll_tick;  /**< Tick of last poll             */
    bool                enabled;        /**< Polling enabled flag          */

    /* Values */
    int16_t             raw;            /**< Last raw reading              */
    int16_t             filtered;       /**< Last filtered reading         */

    /* Filter (union would save RAM but complicates API) */
    uint8_t             filter_type;    /**< SYN_SensorFilterType          */
    void               *filter;         /**< Pointer to filter instance    */

    /* Threshold */
    bool                threshold_enabled; /**< Threshold monitoring on    */
    SYN_Hysteresis     hyst;            /**< Hysteresis state              */
    SYN_SensorThresholdCallback on_high; /**< Above-threshold callback    */
    SYN_SensorThresholdCallback on_low;  /**< Below-threshold callback    */
    void               *threshold_ctx;  /**< Context for threshold cbs    */

    /* Statistics (optional — set via syn_sensor_set_stats) */
    SYN_Signal        *stats;           /**< Sliding window stats (if set)*/
} SYN_Sensor;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a sensor.
 *
 * @param sensor     Sensor instance.
 * @param name       Human-readable name (for logging).
 * @param read_func  Function that reads the sensor hardware.
 * @param ctx        Context passed to read_func.
 */
void syn_sensor_init(SYN_Sensor *sensor, const char *name,
                      SYN_SensorReadFunc read_func, void *ctx);

/**
 * @brief Set polling interval in milliseconds.
 * @param sensor       Sensor.
 * @param interval_ms  Poll interval.
 */
void syn_sensor_set_interval(SYN_Sensor *sensor, uint32_t interval_ms);

/**
 * @brief Enable or disable polling.
 * @param sensor  Sensor.
 * @param enable  true to enable, false to disable.
 */
void syn_sensor_enable(SYN_Sensor *sensor, bool enable);

/* ── Filter setup ───────────────────────────────────────────────────────── */

/**
 * @brief Attach a moving-average filter.
 * @param sensor  Sensor.
 * @param f       Initialized MA filter.
 */
void syn_sensor_set_filter_ma(SYN_Sensor *sensor, SYN_FilterMA *f);

/**
 * @brief Attach an exponential moving-average filter.
 * @param sensor  Sensor.
 * @param f       Initialized EMA filter.
 */
void syn_sensor_set_filter_ema(SYN_Sensor *sensor, SYN_FilterEMA *f);

/**
 * @brief Attach a median filter.
 * @param sensor  Sensor.
 * @param f       Initialized median filter.
 */
void syn_sensor_set_filter_median(SYN_Sensor *sensor, SYN_FilterMedian *f);

/**
 * @brief Remove any attached filter.
 * @param sensor  Sensor.
 */
void syn_sensor_clear_filter(SYN_Sensor *sensor);

/* ── Threshold setup ────────────────────────────────────────────────────── */

/**
 * @brief Set a threshold with hysteresis and callbacks.
 *
 * @param sensor     Sensor.
 * @param threshold  Center threshold value.
 * @param band       Hysteresis half-width.
 * @param on_high    Called when value crosses above (threshold + band).
 * @param on_low     Called when value drops below (threshold - band).
 * @param ctx        Context for callbacks.
 */
void syn_sensor_set_threshold(SYN_Sensor *sensor,
                               int32_t threshold, int32_t band,
                               SYN_SensorThresholdCallback on_high,
                               SYN_SensorThresholdCallback on_low,
                               void *ctx);

/**
 * @brief Disable threshold monitoring.
 * @param sensor  Sensor.
 */
void syn_sensor_clear_threshold(SYN_Sensor *sensor);

/* ── Statistics ─────────────────────────────────────────────────────────── */

/**
 * @brief Attach a signal statistics window.
 *
 * Each update pushes the filtered value into the signal window,
 * giving you min/max/mean/variance/delta automatically.
 *
 * @param sensor  Sensor.
 * @param stats   Initialized SYN_Signal instance, or NULL to detach.
 */
void syn_sensor_set_stats(SYN_Sensor *sensor, SYN_Signal *stats);

/* ── Update ─────────────────────────────────────────────────────────────── */

/**
 * @brief Update the sensor — poll if interval elapsed, filter, check threshold.
 *
 * Call from your main loop or scheduler task.
 *
 * @param sensor  Sensor.
 * @return true if a new reading was taken this call.
 */
bool syn_sensor_update(SYN_Sensor *sensor);

/**
 * @brief Force an immediate reading (ignoring interval).
 *
 * @param sensor  Sensor.
 * @return The filtered value.
 */
int16_t syn_sensor_read_now(SYN_Sensor *sensor);

/**
 * @brief Service an array of sensors (calls syn_sensor_update on each).
 * @param sensors  Array of sensors.
 * @param count    Number of sensors.
 */
void syn_sensor_service(SYN_Sensor *sensors, size_t count);

/**
 * @brief Get the last filtered value.
 * @param sensor  Sensor.
 * @return Filtered reading.
 */
static inline int16_t syn_sensor_value(const SYN_Sensor *sensor)
{
    return sensor->filtered;
}

/**
 * @brief Get the last raw value.
 * @param sensor  Sensor.
 * @return Raw reading.
 */
static inline int16_t syn_sensor_raw(const SYN_Sensor *sensor)
{
    return sensor->raw;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SENSOR_H */
