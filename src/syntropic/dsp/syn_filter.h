/**
 * @file syn_filter.h
 * @brief Integer-only digital filters for sensor signal conditioning.
 *
 * All filters operate on int16_t values and use no floating-point math.
 * Window backing buffers are caller-provided (no malloc).
 *
 * @par Available filters
 * - **Moving Average (MA)**: Smooth noise, configurable window
 * - **Exponential Moving Average (EMA)**: Low-pass, single coefficient
 * - **Median**: Impulse noise rejection, small windows only
 * @ingroup syn_dsp
 */

#ifndef SYN_FILTER_H
#define SYN_FILTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** Maximum window size for moving average and median filters. */
#ifndef SYN_FILTER_MAX_WINDOW
  #define SYN_FILTER_MAX_WINDOW  32
#endif

/* ── Moving Average ─────────────────────────────────────────────────────── */

/** @brief Moving average filter state. */
typedef struct {
    int16_t   buf[SYN_FILTER_MAX_WINDOW]; /**< Sample buffer              */
    int32_t   sum;      /**< Running sum                 */
    uint8_t   window;   /**< Configured window size      */
    uint8_t   count;    /**< Samples inserted so far     */
    uint8_t   idx;      /**< Circular write index        */
} SYN_FilterMA;

/**
 * @brief Initialize a moving average filter.
 *
 * @param f       Filter instance.
 * @param window  Window size (must be <= SYN_FILTER_MAX_WINDOW).
 */
void syn_filter_ma_init(SYN_FilterMA *f, uint8_t window);

/**
 * @brief Feed a new sample and get the filtered output.
 *
 * @param f       Filter instance.
 * @param sample  New raw sample.
 * @return Filtered value (average of the last @c window samples).
 */
int16_t syn_filter_ma_update(SYN_FilterMA *f, int16_t sample);

/**
 * @brief Reset the filter to its initial state.
 * @param f  Filter instance.
 */
void syn_filter_ma_reset(SYN_FilterMA *f);

/* ── Exponential Moving Average ─────────────────────────────────────────── */

/** @brief Exponential moving average filter state. */
typedef struct {
    int32_t   value;    /**< Current filtered value (Q8 fixed-point)     */
    uint8_t   alpha;    /**< Smoothing factor (0–255), higher = faster   */
    bool      primed;   /**< Has received at least one sample            */
} SYN_FilterEMA;

/**
 * @brief Initialize an EMA filter.
 *
 * @param f      Filter instance.
 * @param alpha  Smoothing factor (0–255). Higher = more responsive.
 *               Equivalent to alpha = value/256.
 *               Example: 64 ≈ 0.25 (slow), 192 ≈ 0.75 (fast).
 */
void syn_filter_ema_init(SYN_FilterEMA *f, uint8_t alpha);

/**
 * @brief Feed a new sample and get the filtered output.
 * @param f       Filter instance.
 * @param sample  New raw sample.
 * @return Filtered value.
 */
int16_t syn_filter_ema_update(SYN_FilterEMA *f, int16_t sample);

/**
 * @brief Reset the filter to its initial state.
 * @param f  Filter instance.
 */
void syn_filter_ema_reset(SYN_FilterEMA *f);

/* ── Median Filter ──────────────────────────────────────────────────────── */

/** @brief Median filter state. */
typedef struct {
    int16_t   buf[SYN_FILTER_MAX_WINDOW]; /**< Sample buffer              */
    uint8_t   window;   /**< Configured window size      */
    uint8_t   count;    /**< Samples inserted so far     */
    uint8_t   idx;      /**< Circular write index        */
} SYN_FilterMedian;

/**
 * @brief Initialize a median filter.
 *
 * @param f       Filter instance.
 * @param window  Window size (should be odd for a true median;
 *                must be <= SYN_FILTER_MAX_WINDOW).
 */
void syn_filter_median_init(SYN_FilterMedian *f, uint8_t window);

/**
 * @brief Feed a new sample and get the filtered output.
 *
 * @param f       Filter instance.
 * @param sample  New raw sample.
 * @return Median of the last @c window samples.
 */
int16_t syn_filter_median_update(SYN_FilterMedian *f, int16_t sample);

/**
 * @brief Reset the filter to its initial state.
 * @param f  Filter instance.
 */
void syn_filter_median_reset(SYN_FilterMedian *f);

/* ── FIR Filter ─────────────────────────────────────────────────────────── */

/** @brief FIR filter state (Q16 fixed-point). */
typedef struct {
    const q16_t *taps;     /**< Filter coefficient array (length num_taps) */
    q16_t       *history;  /**< Sample history buffer (length num_taps)    */
    uint16_t     num_taps; /**< Number of taps / filter length            */
    uint16_t     head;     /**< Circular history write index              */
} SYN_FilterFIR;

/**
 * @brief Initialize an FIR filter.
 * @param f         Filter instance.
 * @param taps      Array of Q16 filter coefficients.
 * @param history   Caller-provided Q16 history buffer of size @c num_taps.
 * @param num_taps  Number of filter taps.
 */
void syn_filter_fir_init(SYN_FilterFIR *f, const q16_t *taps, q16_t *history, uint16_t num_taps);

/**
 * @brief Feed a new Q16 sample and get the FIR filtered output.
 * @param f       Filter instance.
 * @param sample  New Q16 sample.
 * @return Filtered output in Q16.
 */
q16_t syn_filter_fir_update(SYN_FilterFIR *f, q16_t sample);

/**
 * @brief Reset FIR filter sample history to 0.
 * @param f  Filter instance.
 */
void syn_filter_fir_reset(SYN_FilterFIR *f);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FILTER_H */
