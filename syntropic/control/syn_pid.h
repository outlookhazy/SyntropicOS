/**
 * @file syn_pid.h
 * @brief General-purpose PID controller — integer or floating-point.
 *
 * A discrete-time PID with anti-windup, output clamping, derivative
 * filtering, and configurable gain scaling. Works for motor speed,
 * temperature, position, or any closed-loop control.
 *
 * @par Usage
 * @code
 *   SYN_PID pid;
 *   SYN_PID_Config cfg = {
 *       .kp = 100, .ki = 10, .kd = 50,
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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief PID controller configuration. */
typedef struct {
    int32_t  kp;              /**< Proportional gain (scaled by @c scale)  */
    int32_t  ki;              /**< Integral gain     (scaled by @c scale)  */
    int32_t  kd;              /**< Derivative gain   (scaled by @c scale)  */
    int32_t  scale;           /**< Gain divisor (e.g., 100 means kp=100→1.0) */

    int32_t  out_min;         /**< Minimum output value                   */
    int32_t  out_max;         /**< Maximum output value                   */
    int32_t  integral_max;    /**< Max integral accumulator (anti-windup) */

    uint8_t  d_filter_alpha;  /**< EMA alpha for D-term (0=off, 255=no filter) */
} SYN_PID_Config;

/* ── PID instance ───────────────────────────────────────────────────────── */

/** @brief PID controller instance — config + accumulated state. */
typedef struct {
    SYN_PID_Config cfg;       /**< Configuration snapshot                 */

    int32_t  integral;        /**< Accumulated integral                   */
    int32_t  prev_error;      /**< Previous error for derivative          */
    int32_t  prev_d_filtered; /**< Filtered derivative (Q0)               */
    int32_t  output;          /**< Last computed output                   */
    bool     first;           /**< True before first update               */
} SYN_PID;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize PID controller.
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
int32_t syn_pid_update(SYN_PID *pid, int32_t setpoint,
                        int32_t measured, uint32_t dt_ms);

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
