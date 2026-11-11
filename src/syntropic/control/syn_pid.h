/**
 * @file syn_pid.h
 * @brief General-purpose PID controller — integer arithmetic.
 *
 * A discrete-time PID with anti-windup, output clamping, derivative
 * filtering, and configurable gain scaling. Works for motor speed,
 * temperature, position, or any closed-loop control.
 *
 * @par Gain Scaling
 * All gains are integers divided by @c scale. This allows fractional
 * control without floating-point:
 *
 * | Parameter | Effective value | Notes |
 * |-----------|-----------------|-------|
 * | kp        | kp / scale      | Proportional |
 * | ki        | ki / (scale × 1000) | **Note the ×1000 for time normalization** |
 * | kd        | kd / scale      | Derivative (with dt normalization) |
 *
 * The I-term accumulates error×dt_ms, so the division by 1000 converts
 * millisecond-accumulation into seconds. This means `ki` must be ~1000×
 * larger than you'd expect compared to kp/kd for equivalent effect.
 *
 * @par Convenience Macro
 * Use SYN_PID_GAINS() to set gains from intuitive values:
 * @code
 *   // Instead of manually computing scaled values:
 *   SYN_PID_Config cfg = SYN_PID_GAINS(
 *       1.5,    // kp (proportional)
 *       0.5,    // ki (integral, per-second — NOT per-millisecond)
 *       0.1,    // kd (derivative)
 *       256,    // scale
 *       -1000, 1000  // output range
 *   );
 *   // Produces: kp=384, ki=128000, kd=25, scale=256
 * @endcode
 *
 * @par Usage (manual)
 * @code
 *   SYN_PID pid;
 *   SYN_PID_Config cfg = {
 *       .kp = 100, .ki = 10000, .kd = 50,
 *       .scale = 100,        // gains are /100
 *       .out_min = -1000, .out_max = 1000,
 *       .d_filter_alpha = 128,
 *   };
 *   syn_pid_init(&pid, &cfg);
 *
 *   // In control loop (e.g., every 10ms):
 *   int32_t output = syn_pid_update(&pid, setpoint, measured, dt_ms);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_PID_H
#define SYN_PID_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief PID controller configuration. */
typedef struct {
    int32_t kp;    /**< Proportional gain (÷ scale)            */
    int32_t ki;    /**< Integral gain     (÷ scale × 1000)     */
    int32_t kd;    /**< Derivative gain   (÷ scale)            */
    int32_t scale; /**< Gain divisor (e.g., 256 for 8-bit)     */

    int32_t out_min;      /**< Minimum output value                   */
    int32_t out_max;      /**< Maximum output value                   */
    int32_t integral_max; /**< Max integral accumulator (0 = auto)    */

    uint8_t d_filter_alpha; /**< EMA alpha for D-term (0=off, 255=no filter) */
} SYN_PID_Config;

/**
 * @brief Convenience macro to create a PID config from human-readable gains.
 *
 * Accepts floating-point gain values and a scale factor. Computes the
 * integer ki with the ×1000 factor built in, so you specify ki as a
 * normal per-second integral gain.
 *
 * @param kp_f       Proportional gain (float).
 * @param ki_f       Integral gain per second (float).
 * @param kd_f       Derivative gain (float).
 * @param scale_val  Integer scale divisor (e.g., 256).
 * @param omin       Minimum output.
 * @param omax       Maximum output.
 * @return SYN_PID_Config struct initializer.
 *
 * @par Example
 * @code
 *   SYN_PID_Config cfg = SYN_PID_GAINS(1.5, 0.5, 0.1, 256, -1000, 1000);
 *   // kp = (int)(1.5 * 256)       = 384
 *   // ki = (int)(0.5 * 256 * 1000) = 128000
 *   // kd = (int)(0.1 * 256)       = 25
 * @endcode
 */
#define SYN_PID_GAINS(kp_f, ki_f, kd_f, scale_val, omin, omax) \
    ((SYN_PID_Config){                                         \
        .kp = (int32_t)((kp_f) * (scale_val)),                 \
        .ki = (int32_t)((ki_f) * (scale_val) * 1000),          \
        .kd = (int32_t)((kd_f) * (scale_val)),                 \
        .scale = (scale_val),                                  \
        .out_min = (omin),                                     \
        .out_max = (omax),                                     \
        .integral_max = 0,                                     \
        .d_filter_alpha = 0U,                                  \
    })

/* ── PID instance ───────────────────────────────────────────────────────── */

/** @brief PID controller instance — config + accumulated state. */
typedef struct {
    SYN_PID_Config cfg; /**< Configuration snapshot                 */

    int32_t integral;        /**< Accumulated integral                   */
    int32_t prev_error;      /**< Previous error for derivative          */
    int32_t prev_d_filtered; /**< Filtered derivative (Q0)               */
    int32_t output;          /**< Last computed output                   */
    bool first;              /**< True before first update               */
} SYN_PID;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize PID controller.
 *
 * If integral_max is 0, it is auto-computed from ki, scale, and out_max
 * to allow full integral authority without windup.
 *
 * @param pid  PID instance.
 * @param cfg  Configuration (copied internally).
 */
void syn_pid_init(SYN_PID *pid, const SYN_PID_Config *cfg);

/**
 * @brief Compute one PID update step.
 *
 * @param pid       PID instance.
 * @param setpoint  Desired value.
 * @param measured  Actual measured value.
 * @param dt_ms     Time since last update in milliseconds.
 *                  (0 is treated as 1 to avoid division by zero.)
 * @return Clamped output value.
 */
int32_t syn_pid_update(SYN_PID *pid, int32_t setpoint, int32_t measured, uint32_t dt_ms);

/**
 * @brief Reset integral and derivative state.
 * @param pid  PID instance.
 */
void syn_pid_reset(SYN_PID *pid);

/**
 * @brief Update gains at runtime.
 * @param pid  PID instance.
 * @param kp   Proportional gain.
 * @param ki   Integral gain.
 * @param kd   Derivative gain.
 */
void syn_pid_set_gains(SYN_PID *pid, int32_t kp, int32_t ki, int32_t kd);

/**
 * @brief Set output clamping limits.
 * @param pid      PID instance.
 * @param out_min  Minimum output.
 * @param out_max  Maximum output.
 */
void syn_pid_set_limits(SYN_PID *pid, int32_t out_min, int32_t out_max);

/**
 * @brief Get the last computed output.
 * @param pid  PID instance.
 * @return Last output value.
 */
static inline int32_t syn_pid_output(const SYN_PID *pid)
{
    return pid->output;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PID_H */
