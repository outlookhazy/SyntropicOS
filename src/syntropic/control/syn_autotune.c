#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AUTOTUNE) || SYN_USE_AUTOTUNE

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_autotune.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static int32_t autotune_clamp_i64(int64_t val)
{
    if (val > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (val < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t)val;
}

static bool autotune_is_relay_phase(const SYN_AutoTune *at)
{
    return (at->cfg.mode == SYN_ATUNE_MODE_RELAY) ||
           (at->cfg.mode == SYN_ATUNE_MODE_AUTO && at->ff_phase_done);
}

static void apply_raw_output(SYN_AutoTune *at, int32_t output)
{
    at->current_output = output;
    if (at->ctrl->cfg.motor.set_output != NULL) {
        at->ctrl->cfg.motor.set_output(at->ctrl->cfg.motor.ctx, output);
    }
}

static void emergency_stop(SYN_AutoTune *at, SYN_AutoTune_AbortReason reason)
{
    if (at == NULL)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    at->current_output = 0;
    syn_motor_ctrl_stop(at->ctrl);
    at->state = SYN_ATUNE_ABORTED;
    at->abort_reason = reason;
}

/** Ramp output from 0 toward target over ramp_ms. Returns current output. */
static int32_t ramp_output(SYN_AutoTune *at, int32_t target, uint32_t elapsed)
{
    uint32_t ramp = at->cfg.ramp_ms;
    if (ramp == 0 || elapsed >= ramp)
        return target;
    return autotune_clamp_i64(((int64_t)target * (int64_t)elapsed) / (int64_t)ramp);
}

static void write_log(SYN_AutoTune *at)
{
    if (at->cfg.datalog == NULL)
        return;

    SYN_AutoTune_LogFrame frame = {.state = (uint8_t)at->state,
                                   .output = (int16_t)at->current_output,
                                   .position = at->ctrl->measured_position,
                                   .velocity = at->ctrl->measured_velocity};

    syn_datalog_write(at->cfg.datalog, SYN_ATUNE_LOG_ID, &frame, sizeof(frame));
}

/** Check all safety conditions. Returns true if safe. */
static bool safety_ok(SYN_AutoTune *at, int32_t pos, int32_t velocity)
{
    uint32_t now = syn_port_get_tick_ms();

    /* Watchdog: update() must be called regularly */
    uint32_t watchdog = at->cfg.watchdog_ms;
    if (watchdog == 0)
        watchdog = at->cfg.limits.watchdog_ms;
    if (watchdog == 0)
        watchdog = 500;

    if (at->last_update_tick > 0) {
        if ((now - at->last_update_tick) > watchdog) {
            emergency_stop(at, SYN_ATUNE_ABORT_WATCHDOG);
            return false;
        }
    }

    /* Track limits */
    if (at->cfg.limits.position_min != 0 || at->cfg.limits.position_max != 0) {
        if (pos < at->cfg.limits.position_min || pos > at->cfg.limits.position_max) {
            emergency_stop(at, SYN_ATUNE_ABORT_POSITION);
            return false;
        }
    }

    /* Velocity limit */
    if (at->cfg.limits.max_velocity > 0) {
        int64_t abs_vel = (velocity < 0) ? -(int64_t)velocity : (int64_t)velocity;
        if (abs_vel > (int64_t)at->cfg.limits.max_velocity) {
            emergency_stop(at, SYN_ATUNE_ABORT_VELOCITY);
            return false;
        }
    }

    /* Respect motor controller soft limits */
    if (at->ctrl->cfg.position_min != 0 || at->ctrl->cfg.position_max != 0) {
        if (pos <= at->ctrl->cfg.position_min || pos >= at->ctrl->cfg.position_max) {
            emergency_stop(at, SYN_ATUNE_ABORT_SOFT_LIMIT);
            return false;
        }
    }

    return true;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_autotune_init(SYN_AutoTune *at, SYN_MotorCtrl *ctrl, const SYN_AutoTune_Config *cfg)
{
    SYN_ASSERT(at != NULL);
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(cfg != NULL);

    memset(at, 0, sizeof(*at));
    at->cfg = *cfg;
    at->ctrl = ctrl;

    /* Apply defaults */
    if (at->cfg.settle_ms == 0)
        at->cfg.settle_ms = 2000;
    if (at->cfg.measure_ms == 0)
        at->cfg.measure_ms = 2000;
    if (at->cfg.relay_cycles == 0)
        at->cfg.relay_cycles = 3;
    if (at->cfg.watchdog_ms == 0)
        at->cfg.watchdog_ms = 500;
    if (at->cfg.ramp_ms == 0)
        at->cfg.ramp_ms = 500;
    if (at->cfg.gain_multiplier_pct == 0)
        at->cfg.gain_multiplier_pct = 100;

    /* Snapshot starting position and set default setpoint if 0 */
    at->start_position = ctrl->measured_position;
    if (at->cfg.setpoint == 0) {
        at->cfg.setpoint = at->start_position;
    }
    at->phase_start_tick = syn_port_get_tick_ms();
    at->last_update_tick = at->phase_start_tick;
    at->result.pid_scale =
        (ctrl->cfg.pid_scale == 0U || ctrl->cfg.pid_scale > 20U) ? 8U : ctrl->cfg.pid_scale;
    at->result.ff_scale =
        (ctrl->cfg.ff_scale == 0U || ctrl->cfg.ff_scale > 24U) ? 8U : ctrl->cfg.ff_scale;

    bool skip_ff = (cfg->mode == SYN_ATUNE_MODE_RELAY) ||
                   (cfg->mode == SYN_ATUNE_MODE_AUTO &&
                    (cfg->flags & (SYN_ATUNE_FLAG_IDENT_KV | SYN_ATUNE_FLAG_IDENT_KA)) == 0 &&
                    (cfg->flags & SYN_ATUNE_FLAG_TUNE_PID) != 0);
    if (skip_ff) {
        at->result.ff_kv = ctrl->cfg.ff_kv;
        at->result.ff_ka = ctrl->cfg.ff_ka;
        at->ff_phase_done = true;
    }
    if (cfg->mode == SYN_ATUNE_MODE_FF_IDENT ||
        (cfg->mode == SYN_ATUNE_MODE_AUTO && (cfg->flags & SYN_ATUNE_FLAG_TUNE_PID) == 0)) {
        at->result.kp = ctrl->cfg.pid_kp;
        at->result.ki = ctrl->cfg.pid_ki;
        at->result.kd = ctrl->cfg.pid_kd;
    }

    /* Stop the motor controller — we're taking over */
    syn_motor_ctrl_stop(ctrl);

    at->ka_p1_captured = false;
    at->ka_p2_captured = false;
    at->history_v = 0;
    at->last_check_tick = 0;

    if (at->cfg.test_output == 0) {
        at->state = SYN_ATUNE_PROBE;
        at->current_output = 5; /* Start probing at 5% */
    } else {
        at->state = SYN_ATUNE_RAMP_UP;
    }

    if (skip_ff) {
        at->above_setpoint = (ctrl->measured_position >= at->cfg.setpoint);
        at->osc_peak_pos = ctrl->measured_position;
        at->osc_peak_neg = ctrl->measured_position;
    }

    return SYN_OK;
}

static void autotune_step_measuring_2(SYN_AutoTune *at, int32_t velocity, uint32_t elapsed,
                                      uint32_t now)
{
    apply_raw_output(at, autotune_clamp_i64(2LL * (int64_t)at->cfg.test_output));
    at->velocity_sum += velocity;
    at->velocity_samples++;

    if (elapsed < at->cfg.measure_ms) {
        return;
    }

    if (at->velocity_samples > 0) {
        at->result.steady_velocity_2 = (int32_t)(at->velocity_sum / (int64_t)at->velocity_samples);

        int32_t v1 = at->result.steady_velocity_1;
        int32_t v2 = at->result.steady_velocity_2;
        int64_t dv = (v2 > v1) ? ((int64_t)v2 - (int64_t)v1) : ((int64_t)v1 - (int64_t)v2);
        int64_t dout = (at->cfg.test_output < 0) ? -(int64_t)at->cfg.test_output
                                                 : (int64_t)at->cfg.test_output;
        if (dv > 2 && dout > 0) {
            uint8_t ff_scale = at->ctrl->cfg.ff_scale;
            if (ff_scale == 0U || ff_scale > 24U) {
                ff_scale = 12U;
            }

            while (ff_scale < 24U && ((dout * (1LL << ff_scale)) / dv) < 100LL) {
                ff_scale++;
            }
            at->result.ff_scale = ff_scale;
            at->result.ff_kv = autotune_clamp_i64((dout * (1LL << ff_scale)) / dv);
        }
    }

    if (at->result.ff_kv > 0 && (at->cfg.flags & SYN_ATUNE_FLAG_IDENT_KA) != 0) {
        if (at->ka_t2 > at->ka_t1) {
            int64_t dt_ms = (int64_t)(at->ka_t2 - at->ka_t1);
            int64_t dv = (int64_t)at->ka_v2 - (int64_t)at->ka_v1;
            int64_t accel = (dt_ms > 0) ? ((dv * 1000LL) / dt_ms) : 0LL;
            if (dv > 10 && accel > 0) {
                int64_t v_avg = ((int64_t)at->ka_v1 + (int64_t)at->ka_v2) / 2LL;
                int64_t kv_out = (v_avg * (int64_t)at->result.ff_kv) >> at->result.ff_scale;
                int64_t accel_out =
                    (int64_t)autotune_clamp_i64((int64_t)at->cfg.test_output - kv_out);

                if (accel_out > 0) {
                    at->result.ff_ka =
                        autotune_clamp_i64((accel_out * (1LL << at->result.ff_scale)) / accel);
                }
            }
        }
    }

    at->ff_phase_done = true;
    at->phase_start_tick = now;
    at->start_output = at->current_output;
    if (at->cfg.mode == SYN_ATUNE_MODE_AUTO && (at->cfg.flags & SYN_ATUNE_FLAG_TUNE_PID) != 0) {
        at->state = SYN_ATUNE_BRAKING;
    } else {
        at->state = SYN_ATUNE_RAMP_DOWN;
    }
}

static void autotune_finish_relay(SYN_AutoTune *at, uint32_t now)
{
    at->phase_start_tick = now;
    at->start_output = at->current_output;
    at->state = SYN_ATUNE_RAMP_DOWN;

    uint32_t Tu = (at->period_count > 0) ? (at->period_sum / at->period_count) : 0U;
    if (Tu == 0U) {
        Tu = 1U;
    }
    int32_t avg_amp = (at->amplitude_count > 0) ? at->amplitude_sum / at->amplitude_count : 1;
    int32_t half_amp = avg_amp / 2;
    if (half_amp == 0) {
        half_amp = 1;
    }

    at->result.Tu_ms = Tu;
    uint8_t scale = at->ctrl->cfg.pid_scale;
    if (scale == 0U || scale > 20U) {
        scale = 8U;
    }

    int64_t amp_out =
        (at->cfg.test_output < 0) ? -(int64_t)at->cfg.test_output : (int64_t)at->cfg.test_output;
    int32_t Ku = 0;
    while (scale < 24U) {
        int64_t Ku_num = 4LL * amp_out * 113LL * (1LL << scale);
        int64_t Ku_den = 355LL * (int64_t)half_amp;
        Ku = autotune_clamp_i64(Ku_num / Ku_den);
        if (Ku >= 100 || scale >= 20U) {
            break;
        }
        scale++;
    }
    at->result.pid_scale = scale;
    at->result.Ku = Ku;

    syn_autotune_calc_relay_gains(Ku, Tu, at->cfg.method, at->cfg.gain_multiplier_pct,
                                  &at->result.kp, &at->result.ki, &at->result.kd);
}

static void autotune_step_relay(SYN_AutoTune *at, int32_t pos, uint32_t now)
{
    int64_t hysteresis = 2LL * (int64_t)at->ctrl->cfg.position_deadband;
    if (hysteresis < 10LL) {
        hysteresis = 10LL;
    }

    bool now_above = at->above_setpoint;
    if (at->above_setpoint) {
        if ((int64_t)pos < (int64_t)at->cfg.setpoint - hysteresis) {
            now_above = false;
        }
    } else if ((int64_t)pos > (int64_t)at->cfg.setpoint + hysteresis) {
        now_above = true;
    }

    if (pos > at->osc_peak_pos) {
        at->osc_peak_pos = pos;
    }
    if (pos < at->osc_peak_neg) {
        at->osc_peak_neg = pos;
    }

    if (now_above != at->above_setpoint) {
        at->half_cycles++;

        if (at->half_cycles >= 2) {
            int64_t amp = (int64_t)at->osc_peak_pos - (int64_t)at->osc_peak_neg;
            at->amplitude_sum = autotune_clamp_i64((int64_t)at->amplitude_sum + amp);
            at->amplitude_count++;
        }

        if ((at->half_cycles & 1) == 0 && at->last_cross_tick > 0) {
            at->period_sum += (now - at->last_cross_tick);
            at->period_count++;
        }
        if ((at->half_cycles & 1) == 0) {
            at->last_cross_tick = now;
        }

        at->osc_peak_pos = pos;
        at->osc_peak_neg = pos;
        at->above_setpoint = now_above;
        at->relay_output =
            now_above ? autotune_clamp_i64(-(int64_t)at->cfg.test_output) : at->cfg.test_output;
    }

    apply_raw_output(at, at->relay_output);

    if (at->period_count >= at->cfg.relay_cycles) {
        autotune_finish_relay(at, now);
    }
}

SYN_AutoTune_State syn_autotune_update(SYN_AutoTune *at)
{
    SYN_ASSERT(at != NULL);

    if (at->state == SYN_ATUNE_DONE || at->state == SYN_ATUNE_ABORTED ||
        at->state == SYN_ATUNE_IDLE) {
        return at->state;
    }

    /* Read feedback manually (we bypassed the controller) */
    int32_t pos = at->ctrl->cfg.read_pos(at->ctrl->cfg.read_pos_ctx);
    int64_t delta64 = (int64_t)pos - (int64_t)at->ctrl->last_position;
    int32_t velocity = autotune_clamp_i64(delta64 * (int64_t)at->ctrl->cfg.update_hz);
    at->ctrl->measured_position = pos;
    at->ctrl->measured_velocity = velocity;
    at->ctrl->last_position = pos;

    uint32_t now = syn_port_get_tick_ms();

    /* Safety checks first */
    if (!safety_ok(at, pos, velocity)) {
        write_log(at);
        return at->state; /* already aborted inside safety_ok */
    }
    at->last_update_tick = now;

    write_log(at);

    uint32_t elapsed = now - at->phase_start_tick;

    switch (at->state) {
    /* ── Probe for minimum motion output ────────────────────────── */
    case SYN_ATUNE_PROBE: {
        apply_raw_output(at, at->current_output);
        int64_t drift = (int64_t)pos - (int64_t)at->start_position;
        if (drift < 0)
            drift = -drift;

        if (drift > 50) {
            at->cfg.test_output = (at->current_output * 120) / 100;
            at->state = SYN_ATUNE_BRAKING;
            at->phase_start_tick = now;
        } else if (elapsed >= 300) {
            at->current_output += 1;
            at->phase_start_tick = now;
            if (at->current_output > 50) {
                emergency_stop(at, SYN_ATUNE_ABORT_NO_MOTION);
            }
        }
        break;
    }

    /* ── Braking to stop between phases ────────────────────────── */
    case SYN_ATUNE_BRAKING:
        apply_raw_output(at, 0);
        if (velocity == 0 || elapsed >= 2000) {
            at->state = SYN_ATUNE_RAMP_UP;
            at->phase_start_tick = now;
            at->start_position = pos;
            if (autotune_is_relay_phase(at)) {
                at->cfg.setpoint = pos;
                at->above_setpoint = true;
                at->osc_peak_pos = pos;
                at->osc_peak_neg = pos;
            }
        }
        break;

    /* ── Ramp up to test output ────────────────────────────────── */
    case SYN_ATUNE_RAMP_UP: {
        bool is_relay = autotune_is_relay_phase(at);
        int32_t target = (is_relay && at->above_setpoint)
                             ? autotune_clamp_i64(-(int64_t)at->cfg.test_output)
                             : at->cfg.test_output;
        int32_t out = ramp_output(at, target, elapsed);
        apply_raw_output(at, out);

        if (elapsed >= at->cfg.ramp_ms) {
            at->phase_start_tick = now;
            if (is_relay) {
                at->state = SYN_ATUNE_RELAY;
                at->relay_output = target;
            } else {
                at->state = SYN_ATUNE_SETTLING;
            }
        }
        break;
    }

    /* ── FF ident: settling phase ──────────────────────────────── */
    case SYN_ATUNE_SETTLING:
        apply_raw_output(at, at->cfg.test_output);

        if (!at->ka_p1_captured) {
            at->ka_t1 = now;
            at->ka_v1 = velocity;
            at->ka_p1_captured = true;
        } else if (!at->ka_p2_captured) {
            int64_t dv = (velocity > at->ka_v1) ? ((int64_t)velocity - (int64_t)at->ka_v1)
                                                : ((int64_t)at->ka_v1 - (int64_t)velocity);
            if (dv > 100 || elapsed >= 1000) {
                at->ka_t2 = now;
                at->ka_v2 = velocity;
                at->ka_p2_captured = true;
                at->last_check_tick = now;
                at->history_v = velocity;
            }
        }

        if (at->ka_p2_captured && (now - at->last_check_tick >= 500)) {
            int64_t dv = (velocity > at->history_v) ? ((int64_t)velocity - (int64_t)at->history_v)
                                                    : ((int64_t)at->history_v - (int64_t)velocity);
            at->history_v = velocity;
            at->last_check_tick = now;

            if (dv < 5 || (uint64_t)elapsed >= (uint64_t)at->cfg.settle_ms * 10ULL) {
                at->state = SYN_ATUNE_MEASURING;
                at->phase_start_tick = now;
                at->velocity_sum = 0;
                at->velocity_samples = 0;
                at->last_check_tick = 0;
            }
        }
        break;

    /* ── FF ident: measuring phase 1 ────────────────────────────── */
    case SYN_ATUNE_MEASURING:
        apply_raw_output(at, at->cfg.test_output);
        at->velocity_sum += velocity;
        at->velocity_samples++;

        if (elapsed >= at->cfg.measure_ms) {
            if (at->velocity_samples > 0) {
                at->result.steady_velocity_1 =
                    (int32_t)(at->velocity_sum / (int64_t)at->velocity_samples);
            }

            at->state = SYN_ATUNE_SETTLING_2;
            at->phase_start_tick = now;
            at->velocity_sum = 0;
            at->velocity_samples = 0;
            at->last_check_tick = now;
            at->history_v = velocity;
        }
        break;

    /* ── FF ident: settling phase 2 (2x output) ────────────────── */
    case SYN_ATUNE_SETTLING_2:
        apply_raw_output(at, autotune_clamp_i64(2LL * (int64_t)at->cfg.test_output));
        if (elapsed >= 200 && (now - at->last_check_tick >= 500)) {
            int64_t dv = (velocity > at->history_v) ? ((int64_t)velocity - (int64_t)at->history_v)
                                                    : ((int64_t)at->history_v - (int64_t)velocity);
            at->history_v = velocity;
            at->last_check_tick = now;

            if (dv < 5 || (uint64_t)elapsed >= (uint64_t)at->cfg.settle_ms * 10ULL) {
                at->state = SYN_ATUNE_MEASURING_2;
                at->phase_start_tick = now;
            }
        }
        break;

    /* ── FF ident: measuring phase 2 (2x output) ────────────────── */
    case SYN_ATUNE_MEASURING_2:
        autotune_step_measuring_2(at, velocity, elapsed, now);
        break;

    /* ── Relay feedback oscillation ────────────────────────────── */
    case SYN_ATUNE_RELAY:
        autotune_step_relay(at, pos, now);
        break;

    /* ── Ramp down to zero ─────────────────────────────────────── */
    case SYN_ATUNE_RAMP_DOWN: {
        uint32_t ramp = at->cfg.ramp_ms;
        if (ramp == 0) {
            apply_raw_output(at, 0);
            syn_motor_ctrl_stop(at->ctrl);
            at->state = SYN_ATUNE_DONE;
            break;
        }

        uint32_t e = (elapsed > ramp) ? ramp : elapsed;
        int32_t out =
            autotune_clamp_i64(((int64_t)at->start_output * (int64_t)(ramp - e)) / (int64_t)ramp);

        if (elapsed >= ramp || (at->start_output > 0 && out <= 0) ||
            (at->start_output < 0 && out >= 0)) {
            apply_raw_output(at, 0);
            syn_motor_ctrl_stop(at->ctrl);
            at->state = SYN_ATUNE_DONE;
        } else {
            apply_raw_output(at, out);
        }
        break;
    }

    default:
        break;
    }

    return at->state;
}

void syn_autotune_apply(SYN_AutoTune *at)
{
    SYN_ASSERT(at != NULL);
    if (at->state != SYN_ATUNE_DONE) {
        return;
    }

    uint8_t pid_scale = (at->result.pid_scale < 31U) ? at->result.pid_scale : 30U;
    uint8_t ff_scale = (at->result.ff_scale < 31U) ? at->result.ff_scale : 30U;

    at->ctrl->cfg.pid_kp = at->result.kp;
    at->ctrl->cfg.pid_ki = at->result.ki;
    at->ctrl->cfg.pid_kd = at->result.kd;
    at->ctrl->cfg.pid_scale = pid_scale;
    at->ctrl->cfg.ff_kv = at->result.ff_kv;
    at->ctrl->cfg.ff_ka = at->result.ff_ka;
    at->ctrl->cfg.ff_scale = ff_scale;

    SYN_PID_Config pid_cfg = at->ctrl->pid.cfg;
    pid_cfg.kp = at->result.kp;
    pid_cfg.ki = at->result.ki;
    pid_cfg.kd = at->result.kd;
    pid_cfg.scale = (int32_t)(1ULL << pid_scale);
    pid_cfg.integral_max = 0;
    syn_pid_init(&at->ctrl->pid, &pid_cfg);
}

void syn_autotune_abort(SYN_AutoTune *at)
{
    if (at == NULL) {
        return;
    }
    SYN_ASSERT(at != NULL);
    emergency_stop(at, SYN_ATUNE_ABORT_USER);
}

SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                              const SYN_AutoTune_Limits *limits, SYN_AutoTune_Flags flags,
                              uint16_t gain_multiplier)
{
    SYN_ASSERT(at != NULL);
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(limits != NULL);

    SYN_AutoTune_Config cfg = {
        .mode = SYN_ATUNE_MODE_AUTO,
        .flags = flags,
        .test_output = 0, /* Auto-probe */
        .limits = *limits,
        .relay_cycles = 4,
        .method = SYN_ATUNE_TYREUS_LUYBEN,
        .watchdog_ms = limits->watchdog_ms,
        .gain_multiplier_pct = gain_multiplier,
    };

    return syn_autotune_init(at, ctrl, &cfg);
}

void syn_autotune_calc_relay_gains(int32_t Ku, uint32_t Tu_ms, SYN_AutoTune_Method method,
                                   uint16_t multiplier_pct, int32_t *kp, int32_t *ki, int32_t *kd)
{
    if (kp == NULL || ki == NULL || kd == NULL || Tu_ms == 0U) {
        return;
    }

    int64_t ku = (int64_t)Ku;
    int64_t tu = (int64_t)Tu_ms;
    int64_t p = 0;
    int64_t i = 0;
    int64_t d = 0;

    switch (method) {
    case SYN_ATUNE_ZN_CLASSIC:
        p = (ku * 60LL) / 100LL;
        i = (p * 2000LL) / tu;
        d = (p * tu) / 8000LL;
        break;
    case SYN_ATUNE_TYREUS_LUYBEN:
        p = (ku * 10LL) / 22LL;
        i = (p * 10000LL) / (22LL * tu);
        d = (p * tu) / 6300LL;
        break;
    case SYN_ATUNE_ZN_NO_OVERSHOOT:
    default:
        p = (ku * 20LL) / 100LL;
        i = (p * 2000LL) / tu;
        d = (p * tu) / 3000LL;
        break;
    }

    if (multiplier_pct > 0U && multiplier_pct != 100U) {
        p = (p * (int64_t)multiplier_pct) / 100LL;
        i = (i * (int64_t)multiplier_pct) / 100LL;
        d = (d * (int64_t)multiplier_pct) / 100LL;
    }

    *kp = autotune_clamp_i64(p);
    *ki = autotune_clamp_i64(i);
    *kd = autotune_clamp_i64(d);
}

#endif /* SYN_USE_AUTOTUNE */
