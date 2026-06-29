/**
 * @file syn_signal.h
 * @brief Signal statistics — sliding window min/max/mean/variance.
 *
 * Maintains a circular buffer of samples and computes statistics
 * incrementally. Designed for sensor data conditioning.
 *
 * All math is integer-only. Mean and variance use Q16.16 for
 * fractional precision without floating point.
 *
 * @par Usage
 * @code
 *   int32_t samples[32];
 *   SYN_Signal sig;
 *   syn_signal_init(&sig, samples, 32);
 *
 *   // Feed samples:
 *   syn_signal_push(&sig, adc_read());
 *
 *   // Query stats:
 *   int32_t min = syn_signal_min(&sig);
 *   int32_t max = syn_signal_max(&sig);
 *   int32_t avg = syn_signal_mean(&sig);       // integer mean
 *   int32_t pp  = syn_signal_peak_to_peak(&sig);
 *   int32_t var = syn_signal_variance_q16(&sig); // Q16.16
 * @endcode
 * @ingroup syn_dsp
 */

#ifndef SYN_SIGNAL_H
#define SYN_SIGNAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Instance ───────────────────────────────────────────────────────────── */

/** @brief Signal statistics — sliding window with incremental min/max/mean. */
typedef struct {
    int32_t  *buf;           /**< Sample buffer (circular)                */
    size_t    capacity;      /**< Buffer size                             */
    size_t    head;          /**< Next write position                     */
    size_t    count;         /**< Number of valid samples (≤ capacity)    */

    /* Running accumulators */
    int64_t   sum;           /**< Sum of all samples in window            */
    int32_t   cached_min;    /**< Cached min (invalidated on push)        */
    int32_t   cached_max;    /**< Cached max (invalidated on push)        */
    bool      cache_valid;   /**< True if min/max caches are current      */
} SYN_Signal;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize with a caller-provided sample buffer.
 * @param sig       Signal instance.
 * @param buf       Sample buffer (caller-owned).
 * @param capacity  Buffer capacity (number of samples).
 */
void syn_signal_init(SYN_Signal *sig, int32_t *buf, size_t capacity);

/**
 * @brief Push a new sample.
 *
 * If the window is full, the oldest sample is evicted.
 *
 * @param sig     Signal instance.
 * @param sample  New sample value.
 */
void syn_signal_push(SYN_Signal *sig, int32_t sample);

/**
 * @brief Clear all samples.
 * @param sig  Signal instance.
 */
void syn_signal_clear(SYN_Signal *sig);

/**
 * @brief Number of samples in the window.
 * @param sig  Signal instance.
 * @return Sample count.
 */
static inline size_t syn_signal_count(const SYN_Signal *sig)
{
    return sig->count;
}

/**
 * @brief True if the window is full.
 * @param sig  Signal instance.
 * @return true if count == capacity.
 */
static inline bool syn_signal_full(const SYN_Signal *sig)
{
    return sig->count == sig->capacity;
}

/**
 * @brief Minimum value in the window.
 * @param sig  Signal instance.
 * @return Min, or 0 if empty.
 */
int32_t syn_signal_min(SYN_Signal *sig);

/**
 * @brief Maximum value in the window.
 * @param sig  Signal instance.
 * @return Max, or 0 if empty.
 */
int32_t syn_signal_max(SYN_Signal *sig);

/**
 * @brief Peak-to-peak = max - min.
 * @param sig  Signal instance.
 * @return Peak-to-peak value, or 0 if empty.
 */
static inline int32_t syn_signal_peak_to_peak(SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    return syn_signal_max(sig) - syn_signal_min(sig);
}

/**
 * @brief Integer mean (truncated).
 * @param sig  Signal instance.
 * @return Truncated mean, or 0 if empty.
 */
static inline int32_t syn_signal_mean(const SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    return (int32_t)(sig->sum / (int64_t)sig->count);
}

/**
 * @brief Sum of all samples in window.
 * @param sig  Signal instance.
 * @return Running sum.
 */
static inline int64_t syn_signal_sum(const SYN_Signal *sig)
{
    return sig->sum;
}

/**
 * @brief Variance in Q16.16 format.
 *
 * Population variance: Σ(x - mean)² / N, returned as Q16.16.
 * Returns 0 if fewer than 2 samples.
 *
 * @param sig  Signal instance.
 * @return Variance in Q16.16.
 */
int32_t syn_signal_variance_q16(const SYN_Signal *sig);

/**
 * @brief Get the most recent sample.
 * @param sig  Signal instance.
 * @return Latest sample, or 0 if empty.
 */
int32_t syn_signal_latest(const SYN_Signal *sig);

/**
 * @brief Get sample at index (0 = oldest, count-1 = newest).
 * @param sig    Signal instance.
 * @param index  Sample index.
 * @return Sample value.
 */
int32_t syn_signal_at(const SYN_Signal *sig, size_t index);

/**
 * @brief Rate of change: latest - previous.
 *
 * Returns 0 if fewer than 2 samples.
 *
 * @param sig  Signal instance.
 * @return Delta between last two samples.
 */
int32_t syn_signal_delta(const SYN_Signal *sig);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SIGNAL_H */
