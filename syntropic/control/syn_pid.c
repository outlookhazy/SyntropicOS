#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_PID) || SYN_USE_PID

/**
 * @file syn_pid.c
 * @brief PID controller implementation.
 */

#include "syn_pid.h"
#include "../common/syn_defs.h"
#include "../util/syn_assert.h"

#include <string.h>


/* ── API ────────────────────────────────────────────────────────────────── */

void syn_pid_init(SYN_PID *pid, const SYN_PID_Config *cfg)
{
    SYN_ASSERT(pid != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(cfg->scale != 0);

    memset(pid, 0, sizeof(*pid));
    pid->cfg   = *cfg;
    pid->first = true;

    /* Default integral limit if not set */
    if (pid->cfg.integral_max == 0) {
        pid->cfg.integral_max = pid->cfg.out_max * pid->cfg.scale;
    }
}

int32_t syn_pid_update(SYN_PID *pid, int32_t setpoint,
                        int32_t measured, uint32_t dt_ms)
{
    SYN_ASSERT(pid != NULL);

    if (dt_ms == 0) dt_ms = 1;

    int32_t error = setpoint - measured;

    /* ── Proportional ──────────────────────────────────────────────────── */
    int32_t p_term = (pid->cfg.kp * error) / pid->cfg.scale;

    /* ── Integral (with anti-windup) ───────────────────────────────────── */
    pid->integral += error * (int32_t)dt_ms;
    pid->integral = SYN_CLAMP(pid->integral,
                          -pid->cfg.integral_max,
                           pid->cfg.integral_max);

    int32_t i_term = (pid->cfg.ki * pid->integral) / (pid->cfg.scale * 1000);

    /* ── Derivative ────────────────────────────────────────────────────── */
    int32_t d_raw;
    if (pid->first) {
        d_raw = 0;
        pid->first = false;
    } else {
        d_raw = ((error - pid->prev_error) * 1000) / (int32_t)dt_ms;
    }
    pid->prev_error = error;

    /* Optional EMA filter on derivative */
    if (pid->cfg.d_filter_alpha > 0 && pid->cfg.d_filter_alpha < 255) {
        int32_t alpha = pid->cfg.d_filter_alpha;
        pid->prev_d_filtered += (alpha * (d_raw - pid->prev_d_filtered)) >> 8;
        d_raw = pid->prev_d_filtered;
    }

    int32_t d_term = (pid->cfg.kd * d_raw) / pid->cfg.scale;

    /* ── Sum and clamp ─────────────────────────────────────────────────── */
    int32_t output = p_term + i_term + d_term;
    output = SYN_CLAMP(output, pid->cfg.out_min, pid->cfg.out_max);

    /* Anti-windup: if output is saturated, freeze integral */
    if ((output == pid->cfg.out_max && error > 0) ||
        (output == pid->cfg.out_min && error < 0)) {
        pid->integral -= error * (int32_t)dt_ms;
    }

    pid->output = output;
    return output;
}

void syn_pid_reset(SYN_PID *pid)
{
    SYN_ASSERT(pid != NULL);
    pid->integral        = 0;
    pid->prev_error      = 0;
    pid->prev_d_filtered = 0;
    pid->output          = 0;
    pid->first           = true;
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
