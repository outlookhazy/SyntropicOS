#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PID) || SYN_USE_PID

/**
 * @file syn_pid.c
 * @brief PID controller implementation.
 */

#include "../common/syn_defs.h"
#include "../util/syn_assert.h"
#include "syn_pid.h"

#include <string.h>

/**
 * @brief Clamp signed 64-bit integer to signed 32-bit range.
 * @param v Input 64-bit value.
 * @return Clamped 32-bit value.
 */
static int32_t pid_clamp_i64(int64_t v)
{
    if (v > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (v < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t)v;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_pid_init(SYN_PID *pid, const SYN_PID_Config *cfg)
{
    SYN_ASSERT(pid != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(cfg->scale != 0);

    memset(pid, 0, sizeof(*pid));
    pid->cfg = *cfg;
    pid->first = true;

    /* Auto-compute integral_max if not explicitly set.
     * I-term = (ki * integral) / (1000 * scale).
     * For I-term to reach out_max:
     *   integral_max = out_max * scale * 1000 / ki
     * Use 64-bit to avoid overflow in the multiply. */
    if (pid->cfg.integral_max == 0) {
        if (pid->cfg.ki > 0) {
            pid->cfg.integral_max =
                pid_clamp_i64(((int64_t)pid->cfg.out_max * (int64_t)pid->cfg.scale * 1000LL) /
                              (int64_t)pid->cfg.ki);
        } else {
            pid->cfg.integral_max =
                pid_clamp_i64((int64_t)pid->cfg.out_max * (int64_t)pid->cfg.scale);
        }
    }
}

int32_t syn_pid_update(SYN_PID *pid, int32_t setpoint, int32_t measured, uint32_t dt_ms)
{
    SYN_ASSERT(pid != NULL);

    if (dt_ms == 0)
        dt_ms = 1;

    int32_t error = pid_clamp_i64((int64_t)setpoint - (int64_t)measured);

    /* ── Proportional ──────────────────────────────────────────────────── */
    int32_t p_term =
        pid_clamp_i64(((int64_t)pid->cfg.kp * (int64_t)error) / (int64_t)pid->cfg.scale);

    /* ── Integral (with anti-windup) ───────────────────────────────────── */
    int64_t dt64 = (int64_t)dt_ms;
    int64_t new_integral = pid->integral + (int64_t)error * dt64;
    if (pid->cfg.integral_max > 0) {
        if (new_integral > pid->cfg.integral_max) {
            new_integral = pid->cfg.integral_max;
        } else if (new_integral < -pid->cfg.integral_max) {
            new_integral = -pid->cfg.integral_max;
        }
    }
    pid->integral = new_integral;

    /* Time normalization + scaling using 64-bit int arithmetic to prevent overflow */
    int32_t i_term =
        pid_clamp_i64((((int64_t)pid->cfg.ki * pid->integral) / 1000LL) / (int64_t)pid->cfg.scale);

    /* ── Derivative ────────────────────────────────────────────────────── */
    int32_t d_raw;
    if (pid->first) {
        d_raw = 0;
        pid->first = false;
    } else {
        d_raw = pid_clamp_i64((((int64_t)error - (int64_t)pid->prev_error) * 1000LL) / dt64);
    }
    pid->prev_error = error;

    /* Optional EMA filter on derivative */
    if (pid->cfg.d_filter_alpha > 0 && pid->cfg.d_filter_alpha < 255) {
        int64_t alpha = pid->cfg.d_filter_alpha;
        int64_t diff = (int64_t)d_raw - pid->prev_d_filtered;
        pid->prev_d_filtered = pid_clamp_i64((int64_t)pid->prev_d_filtered + ((alpha * diff) >> 8));
        d_raw = pid->prev_d_filtered;
    }

    int32_t d_term =
        pid_clamp_i64(((int64_t)pid->cfg.kd * (int64_t)d_raw) / (int64_t)pid->cfg.scale);

    /* ── Sum and clamp ─────────────────────────────────────────────────── */
    int64_t sum64 = (int64_t)p_term + (int64_t)i_term + (int64_t)d_term;
    if (sum64 < (int64_t)pid->cfg.out_min) {
        sum64 = (int64_t)pid->cfg.out_min;
    } else if (sum64 > (int64_t)pid->cfg.out_max) {
        sum64 = (int64_t)pid->cfg.out_max;
    }
    int32_t output = (int32_t)sum64;

    /* Anti-windup: if output is saturated, freeze integral */
    if ((output == pid->cfg.out_max && error > 0) || (output == pid->cfg.out_min && error < 0)) {
        pid->integral -= (int64_t)error * dt64;
    }

    pid->output = output;
    return output;
}

void syn_pid_reset(SYN_PID *pid)
{
    SYN_ASSERT(pid != NULL);
    pid->integral = 0;
    pid->prev_error = 0;
    pid->prev_d_filtered = 0;
    pid->output = 0;
    pid->first = true;
}

void syn_pid_set_gains(SYN_PID *pid, int32_t kp, int32_t ki, int32_t kd)
{
    SYN_ASSERT(pid != NULL);
    pid->cfg.kp = kp;
    pid->cfg.ki = ki;
    pid->cfg.kd = kd;
}

void syn_pid_set_limits(SYN_PID *pid, int32_t out_min, int32_t out_max)
{
    SYN_ASSERT(pid != NULL);
    pid->cfg.out_min = out_min;
    pid->cfg.out_max = out_max;
}

#endif /* SYN_USE_PID */
