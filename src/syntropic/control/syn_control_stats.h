/**
 * @file syn_control_stats.h
 * @brief Performance monitoring and "Tuning Scorecard" for control loops.
 *
 * This module provides a way to quantify the performance of a control loop
 * beyond simple error tracking. It measures precision, efficiency, and
 * mechanical stress (jitter).
 */

#ifndef SYN_CONTROL_STATS_H
#define SYN_CONTROL_STATS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Raw accumulators for control performance analysis. */
typedef struct {
    int64_t sum_sq_err;    /**< Accumulator for RMS Error                */
    int64_t sum_sq_out;    /**< Accumulator for Control Effort (RMS Out)  */
    int64_t sum_abs_delta; /**< Accumulator for Jitter (Output Slew)     */
    int64_t sum_itae;      /**< Integral of Time-weighted Absolute Error */

    int32_t max_error;   /**< Maximum absolute error observed          */
    int32_t last_output; /**< Previous output for delta calculation     */
    uint32_t samples;    /**< Number of samples collected              */
} SYN_ControlStats;

/** @brief Human-readable performance report. */
typedef struct {
    int32_t rms_error;      /**< Root Mean Square error (counts)          */
    int32_t max_error;      /**< Maximum absolute error (counts)          */
    int32_t control_effort; /**< RMS of output (scaled 0-10000 for 0-100%) */
    int32_t jitter;         /**< Average absolute output delta (noise)     */
    int32_t itae;           /**< ITAE score (lower is better)              */
} SYN_ControlReport;

/**
 * @brief Reset all accumulators.
 * @param stats Stats instance to reset.
 */
void syn_control_stats_reset(SYN_ControlStats *stats);

/**
 * @brief Update stats with a new sample.
 * @param stats   Stats instance.
 * @param error   Current servo error (target - measured).
 * @param output  Current motor output (-100 to 100, or raw counts).
 */
void syn_control_stats_update(SYN_ControlStats *stats, int32_t error, int32_t output);

/**
 * @brief Calculate a report from the current accumulators.
 * @param stats   Stats instance.
 * @param report  Output report structure.
 */
void syn_control_stats_report(const SYN_ControlStats *stats, SYN_ControlReport *report);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CONTROL_STATS_H */
