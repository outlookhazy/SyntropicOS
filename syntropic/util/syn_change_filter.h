/**
 * @file syn_change_filter.h
 * @brief Dead-band + rate-limited change detector (header-only).
 *
 * Reports a value as "changed" only when the difference from the last
 * reported value exceeds a dead-band AND a minimum time interval has
 * elapsed.  Useful for suppressing noise in telemetry, sensor logging,
 * display updates, or any push-on-change pattern.
 *
 * Header-only — zero code size if unused.
 *
 * @par Usage
 * @code
 *   SYN_ChangeFilter cf;
 *   syn_change_filter_init(&cf, 0.5f, 1000);  // dead_band=0.5, min_ms=1000
 *
 *   // In sensor loop:
 *   float temp = read_temperature();
 *   if (syn_change_filter_update(&cf, temp, now_ms)) {
 *       // Value changed significantly — transmit / log / display
 *       send_update(temp);
 *   }
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_CHANGE_FILTER_H
#define SYN_CHANGE_FILTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Dead-band + rate-limited change filter. */
typedef struct {
    float    last_value;      /**< Last reported value                    */
    uint32_t last_time_ms;    /**< Tick when last reported                */
    float    dead_band;       /**< Minimum |delta| to count as changed   */
    uint32_t min_interval_ms; /**< Minimum ms between reports (0=no limit) */
    bool     initialized;     /**< false until first update               */
} SYN_ChangeFilter;

/**
 * @brief Initialize a change filter.
 *
 * @param cf               Instance.
 * @param dead_band        Minimum absolute change to report.
 *                         Use 0.0f to report any change.
 * @param min_interval_ms  Minimum milliseconds between reports.
 *                         Use 0 to disable rate limiting.
 */
static inline void syn_change_filter_init(SYN_ChangeFilter *cf,
                                           float dead_band,
                                           uint32_t min_interval_ms)
{
    cf->last_value      = 0.0f;
    cf->last_time_ms    = 0;
    cf->dead_band       = dead_band;
    cf->min_interval_ms = min_interval_ms;
    cf->initialized     = false;
}

/**
 * @brief Feed a new value and check if it should be reported.
 *
 * The first call always returns true (initial value).
 *
 * @param cf       Instance.
 * @param value    New value to test.
 * @param now_ms   Current tick in milliseconds.
 * @return true if the value should be reported.
 */
static inline bool syn_change_filter_update(SYN_ChangeFilter *cf,
                                             float value,
                                             uint32_t now_ms)
{
    /* First value is always reported */
    if (!cf->initialized) {
        cf->last_value   = value;
        cf->last_time_ms = now_ms;
        cf->initialized  = true;
        return true;
    }

    /* Rate limit check */
    uint32_t elapsed = now_ms - cf->last_time_ms;
    if (cf->min_interval_ms > 0 && elapsed < cf->min_interval_ms) {
        return false;
    }

    /* Dead-band check */
    float delta = value - cf->last_value;
    if (delta < 0.0f) delta = -delta;

    if (delta <= cf->dead_band) {
        return false;
    }

    /* Changed enough and enough time passed */
    cf->last_value   = value;
    cf->last_time_ms = now_ms;
    return true;
}

/**
 * @brief Force the filter to report on next update.
 *
 * Resets the initialized flag so the next call to update()
 * returns true regardless of dead-band or interval.
 *
 * @param cf  Filter instance.
 */
static inline void syn_change_filter_reset(SYN_ChangeFilter *cf)
{
    cf->initialized = false;
}

/**
 * @brief Get the last reported value.
 * @param cf  Filter instance.
 * @return Last value that passed the filter.
 */
static inline float syn_change_filter_last(const SYN_ChangeFilter *cf)
{
    return cf->last_value;
}

/**
 * @brief Update dead-band and interval at runtime.
 * @param cf               Filter instance.
 * @param dead_band        New dead-band threshold.
 * @param min_interval_ms  New minimum reporting interval (ms).
 */
static inline void syn_change_filter_set(SYN_ChangeFilter *cf,
                                          float dead_band,
                                          uint32_t min_interval_ms)
{
    cf->dead_band       = dead_band;
    cf->min_interval_ms = min_interval_ms;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CHANGE_FILTER_H */
