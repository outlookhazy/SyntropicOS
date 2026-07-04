#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_AUTOTUNE) || SYN_USE_AUTOTUNE

#include "syn_autotune.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

#include <stdio.h>

static void apply_raw_output(SYN_AutoTune *at, int32_t output)
{
    at->current_output = output;
    if (at->ctrl->cfg.motor.set_output != NULL) {
        at->ctrl->cfg.motor.set_output(at->ctrl->cfg.motor.ctx, output);
    }
}

static void emergency_stop(SYN_AutoTune *at, SYN_AutoTune_AbortReason reason)
{
    if (at == NULL) return;
    at->current_output = 0;
    syn_motor_ctrl_stop(at->ctrl);
    at->state = SYN_ATUNE_ABORTED;
    at->abort_reason = reason;
}

/** Ramp output from 0 toward target over ramp_ms. Returns current output. */
static int32_t ramp_output(SYN_AutoTune *at, int32_t target, uint32_t elapsed)
{
    uint32_t ramp = at->cfg.ramp_ms;
    if (ramp == 0 || elapsed >= ramp) return target;
    return (target * (int32_t)elapsed) / (int32_t)ramp;
}

static void write_log(SYN_AutoTune *at)
{
    if (at->cfg.datalog == NULL) return;

    SYN_AutoTune_LogFrame frame = {
        .state    = (uint8_t)at->state,
        .output   = (int16_t)at->current_output,
        .position = at->ctrl->measured_position,
        .velocity = at->ctrl->measured_velocity
    };

    syn_datalog_write(at->cfg.datalog, SYN_ATUNE_LOG_ID, &frame, sizeof(frame));
}

/** Check all safety conditions. Returns true if safe. */
static bool safety_ok(SYN_AutoTune *at, int32_t pos, int32_t velocity)
{
    uint32_t now = syn_port_get_tick_ms();

    /* Watchdog: update() must be called regularly */
    uint32_t watchdog = at->cfg.watchdog_ms;
    if (watchdog == 0) watchdog = at->cfg.limits.watchdog_ms;
    if (watchdog == 0) watchdog = 500;

    if (at->last_update_tick > 0) {
        if ((now - at->last_update_tick) > watchdog) {
            emergency_stop(at, SYN_ATUNE_ABORT_WATCHDOG);
            return false;
        }
    }

    /* Track limits */
    if (at->cfg.limits.position_min != 0 || at->cfg.limits.position_max != 0) {
        if (pos < at->cfg.limits.position_min ||
            pos > at->cfg.limits.position_max) {
            emergency_stop(at, SYN_ATUNE_ABORT_POSITION);
            return false;
        }
    }

    /* Velocity limit */
    if (at->cfg.limits.max_velocity > 0) {
        int32_t abs_vel = (velocity < 0) ? -velocity : velocity;
        if (abs_vel > at->cfg.limits.max_velocity) {
            emergency_stop(at, SYN_ATUNE_ABORT_VELOCITY);
            return false;
        }
    }

    /* Respect motor controller soft limits */
    if (at->ctrl->cfg.position_min != 0 || at->ctrl->cfg.position_max != 0) {
        if (pos <= at->ctrl->cfg.position_min ||
            pos >= at->ctrl->cfg.position_max) {
            emergency_stop(at, SYN_ATUNE_ABORT_SOFT_LIMIT);
            return false;
        }
    }

    return true;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_autotune_init(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Config *cfg)
{
    SYN_ASSERT(at != NULL);
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(cfg != NULL);

    memset(at, 0, sizeof(*at));
    at->cfg  = *cfg;
    at->ctrl = ctrl;

    /* Apply defaults */
    if (at->cfg.settle_ms == 0)    at->cfg.settle_ms = 2000;
    if (at->cfg.measure_ms == 0)   at->cfg.measure_ms = 2000;
    if (at->cfg.relay_cycles == 0) at->cfg.relay_cycles = 3;
    if (at->cfg.watchdog_ms == 0)  at->cfg.watchdog_ms = 500;
    if (at->cfg.ramp_ms == 0)      at->cfg.ramp_ms = 500;
    if (at->cfg.gain_multiplier_pct == 0) at->cfg.gain_multiplier_pct = 100;

    /* Snapshot starting position and set default setpoint if 0 */
    at->start_position = ctrl->measured_position;
    if (at->cfg.setpoint == 0) {
        at->cfg.setpoint = at->start_position;
    }
    at->phase_start_tick = syn_port_get_tick_ms();
    at->last_update_tick = at->phase_start_tick;
    at->result.pid_scale = ctrl->cfg.pid_scale;
    if (at->result.pid_scale == 0) at->result.pid_scale = 8;
    at->result.ff_scale = ctrl->cfg.ff_scale;
    if (at->result.ff_scale == 0) at->result.ff_scale = 8;

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

    if (cfg->mode == SYN_ATUNE_MODE_RELAY) {
        at->above_setpoint = (ctrl->measured_position >= cfg->setpoint);
        at->osc_peak_pos = ctrl->measured_position;
        at->osc_peak_neg = ctrl->measured_position;
    }

    return SYN_OK;
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
    int32_t delta = pos - at->ctrl->last_position;
    int32_t velocity = delta * (int32_t)at->ctrl->cfg.update_hz;
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
        int32_t drift = pos - at->start_position;
        if (drift < 0) drift = -drift;

        /* If moved significantly, we found the motion threshold.
         * Using 50 counts to be sure it's not just noise/slack. */
        if (drift > 50) {
            at->cfg.test_output = (at->current_output * 120) / 100; /* 20% margin instead of 100% */
            at->state = SYN_ATUNE_BRAKING;
            at->phase_start_tick = now;
        } else if (elapsed >= 300) {
            /* Increment output every 300ms by 1% */
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
            if (at->cfg.mode == SYN_ATUNE_MODE_RELAY ||
                (at->cfg.mode == SYN_ATUNE_MODE_AUTO && at->result.ff_kv != 0)) {
                at->cfg.setpoint = pos;
                at->above_setpoint = true;
                at->osc_peak_pos = pos;
                at->osc_peak_neg = pos;
            }
        }
        break;

    /* ── Ramp up to test output ────────────────────────────────── */
    case SYN_ATUNE_RAMP_UP: {
        int32_t target;
        bool is_relay = (at->cfg.mode == SYN_ATUNE_MODE_RELAY) ||
                        (at->cfg.mode == SYN_ATUNE_MODE_AUTO && at->result.ff_kv != 0);

        if (is_relay) {
            target = at->above_setpoint
                   ? -at->cfg.test_output
                   :  at->cfg.test_output;
        } else {
            target = at->cfg.test_output;
        }
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

        /* Adaptive point capture for Ka identification */
        if (!at->ka_p1_captured) {
            /* Trigger P1 immediately as the baseline for this phase */
            at->ka_t1 = now;
            at->ka_v1 = velocity;
            at->ka_p1_captured = true;
        } else if (!at->ka_p2_captured) {
            /* Trigger P2 when velocity has changed meaningfully from baseline
             * or after a fallback timeout. */
            int32_t dv = (velocity > at->ka_v1) ? (velocity - at->ka_v1) : (at->ka_v1 - velocity);
            if (dv > 100 || elapsed >= 1000) {
                at->ka_t2 = now;
                at->ka_v2 = velocity;
                at->ka_p2_captured = true;
                at->last_check_tick = now;
                at->history_v = velocity;
            }
        }

        /* Wait for steady-state: compare velocity with previous check */
        if (at->ka_p2_captured) {
            if (now - at->last_check_tick >= 500) {
                int32_t dv = (velocity > at->history_v) ? (velocity - at->history_v) : (at->history_v - velocity);
                at->history_v = velocity;
                at->last_check_tick = now;
                
                /* Steady-state reached when dv < 5 over 500ms */
                if (dv < 5 || elapsed >= at->cfg.settle_ms * 10) {
                    at->state = SYN_ATUNE_MEASURING;
                    at->phase_start_tick = now;
                    at->velocity_sum = 0;
                    at->velocity_samples = 0;
                    at->last_check_tick = 0;
                }
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
                at->result.steady_velocity_1 = (int32_t)(at->velocity_sum / (int64_t)at->velocity_samples);
            }
            
            /* Transition to second measurement point (2x output) */
            at->state = SYN_ATUNE_SETTLING_2;
            at->phase_start_tick = now;
            at->velocity_sum = 0;
            at->velocity_samples = 0;
            at->last_check_tick = 0;
            at->history_v = velocity;
        }
        break;

    /* ── FF ident: settling phase 2 (2x output) ────────────────── */
    case SYN_ATUNE_SETTLING_2:
        apply_raw_output(at, at->cfg.test_output * 2);
        /* Wait for new steady-state after 2x output */
        if (elapsed >= 200) {
            if (now - at->last_check_tick >= 500) {
                int32_t dv = (velocity > at->history_v) ? (velocity - at->history_v) : (at->history_v - velocity);
                at->history_v = velocity;
                at->last_check_tick = now;
                
                if (dv < 5 || elapsed >= at->cfg.settle_ms * 10) {
                    at->state = SYN_ATUNE_MEASURING_2;
                    at->phase_start_tick = now;
                }
            }
        }
        break;

    /* ── FF ident: measuring phase 2 (2x output) ────────────────── */
    case SYN_ATUNE_MEASURING_2:
        apply_raw_output(at, at->cfg.test_output * 2);
        at->velocity_sum += velocity;
        at->velocity_samples++;

        if (elapsed >= at->cfg.measure_ms) {
            if (at->velocity_samples > 0) {
                at->result.steady_velocity_2 = (int32_t)(at->velocity_sum / (int64_t)at->velocity_samples);
                
                int32_t v1 = at->result.steady_velocity_1;
                int32_t v2 = at->result.steady_velocity_2;
                int32_t dv = (v2 > v1) ? (v2 - v1) : (v1 - v2);
                if (dv > 2) {
                    /* Slope kv = (Out2 - Out1) / (V2 - V1) */
                    int32_t dout = at->cfg.test_output; /* (2x - 1x) = 1x */
                    uint8_t ff_scale = at->ctrl->cfg.ff_scale;
                    if (ff_scale == 0) ff_scale = 12;

                    while (ff_scale < 24 && (((int64_t)dout << ff_scale) / dv) < 100) {
                        ff_scale++;
                    }
                    at->result.ff_scale = ff_scale;
                    at->result.ff_kv = (int32_t)(((int64_t)dout << ff_scale) / dv);
                } else {
                }
            }

            /* Compute ff_ka if enabled */
            if (at->result.ff_kv > 0 && at->cfg.flags & SYN_ATUNE_FLAG_IDENT_KA) {
                if (at->ka_t2 > at->ka_t1) {
                    int32_t dt_ms = (int32_t)(at->ka_t2 - at->ka_t1);
                    int32_t dv = at->ka_v2 - at->ka_v1;
                    if (dt_ms > 0 && dv > 10) {
                        int32_t accel = (dv * 1000) / dt_ms; /* units/s^2 */
                        int32_t v_avg = (at->ka_v1 + at->ka_v2) / 2;
                        
                        /* Use the identified slope to find friction offset at 0 speed */
                        /* Out = kv * v + offset → offset = Out - kv * v */
                        int32_t kv_out = (int32_t)(((int64_t)v_avg * at->result.ff_kv) >> at->result.ff_scale);
                        int32_t accel_out = at->cfg.test_output - kv_out;
                        
                        if (accel_out > 0) {
                            at->result.ff_ka = (int32_t)(((int64_t)accel_out << at->result.ff_scale) / accel);
                        }
                    }
                }
            }

            /* Transition */
            at->phase_start_tick = now;
            at->start_output = at->current_output;
            if (at->cfg.flags & SYN_ATUNE_FLAG_TUNE_PID) {
                at->state = SYN_ATUNE_BRAKING;
            } else {
                at->state = SYN_ATUNE_RAMP_DOWN;
            }
        }
        break;

    /* ── Relay feedback oscillation ────────────────────────────── */
    case SYN_ATUNE_RELAY: {
        /* Hysteresis: require position to be at least this far past setpoint
         * before registering a zero-crossing. Prevents encoder noise from
         * creating fake crossings on heavy/slow systems. */
        int32_t hysteresis = at->ctrl->cfg.position_deadband * 2;
        if (hysteresis < 10) hysteresis = 10;  /* minimum 10 counts */

        bool now_above = at->above_setpoint;
        if (at->above_setpoint) {
            if (pos < at->cfg.setpoint - hysteresis) now_above = false;
        } else {
            if (pos > at->cfg.setpoint + hysteresis) now_above = true;
        }

        /* Track peaks */
        if (pos > at->osc_peak_pos) at->osc_peak_pos = pos;
        if (pos < at->osc_peak_neg) at->osc_peak_neg = pos;

        /* Detect zero-crossing (with hysteresis) */
        if (now_above != at->above_setpoint) {
            at->half_cycles++;

            if (at->half_cycles >= 2) {
                int32_t amp = at->osc_peak_pos - at->osc_peak_neg;
                if (amp < 0) amp = -amp;
                at->amplitude_sum += amp;
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

            at->relay_output = now_above
                             ? -at->cfg.test_output
                             :  at->cfg.test_output;
        }

        apply_raw_output(at, at->relay_output);

        if (at->period_count >= at->cfg.relay_cycles) {
            at->phase_start_tick = now;
            at->start_output = at->current_output;
            at->state = SYN_ATUNE_RAMP_DOWN;

            /* Compute PID gains */
            uint32_t Tu = at->period_sum / at->period_count;
            int32_t avg_amp = (at->amplitude_count > 0)
                            ? at->amplitude_sum / at->amplitude_count
                            : 1;
            int32_t half_amp = avg_amp / 2;
            if (half_amp == 0) half_amp = 1;


            at->result.Tu_ms = Tu;
            uint8_t scale = at->ctrl->cfg.pid_scale;
            if (scale == 0) scale = 8;

            /* Auto-scale Ku to avoid underflow */
            int32_t Ku = 0;
            while (scale < 24) {
                int64_t Ku_num = (int64_t)4 * at->cfg.test_output * 113 * ((int64_t)1 << scale);
                int64_t Ku_den = (int64_t)355 * half_amp;
                if (Ku_den == 0) Ku_den = 1;
                Ku = (int32_t)(Ku_num / Ku_den);
                if (Ku >= 100 || scale >= 20) break;
                scale++;
            }
            at->result.pid_scale = scale;
            at->result.Ku = Ku;
            

            int32_t Tu_i = (int32_t)Tu;
            switch (at->cfg.method) {
            case SYN_ATUNE_ZN_CLASSIC:
                /* Kp = 0.6*Ku, Ki = 1.2*Ku/Tu, Kd = 3*Ku*Tu/40 */
                at->result.kp = (int32_t)((int64_t)Ku * 60 / 100);
                at->result.ki = (int32_t)((int64_t)Ku * 120 / Tu_i);
                at->result.kd = (int32_t)((int64_t)Ku * 3 * Tu_i / (40L * 1000L));
                break;
            case SYN_ATUNE_ZN_NO_OVERSHOOT:
                at->result.kp = (int32_t)((int64_t)Ku * 33 / 100);
                at->result.ki = (int32_t)((int64_t)Ku * 66 / Tu_i);
                at->result.kd = (int32_t)((int64_t)Ku * 11 * Tu_i / 100000);
                break;
            case SYN_ATUNE_TYREUS_LUYBEN:
                /* Kp = Ku/2.2, Ki = Ku/(2.2*Tu), Kd = Ku*Tu/6.3
                 * For Ki with large Tu: multiply first to avoid truncation */
                at->result.kp = (int32_t)((int64_t)Ku * 100 / 220);
                if (Tu_i > 0) {
                    at->result.ki = (int32_t)((int64_t)Ku * 10000
                                           / ((int64_t)220 * Tu_i));
                }
                at->result.kd = (int32_t)((int64_t)Ku * Tu_i / 6300);
                break;
            }

            /* Apply safety multiplier */
            if (at->cfg.gain_multiplier_pct != 100) {
                at->result.kp = (int32_t)((int64_t)at->result.kp * at->cfg.gain_multiplier_pct / 100);
                at->result.ki = (int32_t)((int64_t)at->result.ki * at->cfg.gain_multiplier_pct / 100);
                at->result.kd = (int32_t)((int64_t)at->result.kd * at->cfg.gain_multiplier_pct / 100);
            }
        }
        break;
    }

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
        int32_t out = (at->start_output * (int32_t)(ramp - e)) / (int32_t)ramp;
        
        if (elapsed >= ramp || (at->start_output > 0 && out <= 0) || (at->start_output < 0 && out >= 0)) {
            apply_raw_output(at, 0);
            syn_motor_ctrl_stop(at->ctrl);
            at->state = SYN_ATUNE_DONE;
        } else {
            apply_raw_output(at, out);
            if (elapsed % 100 == 0) {
            }
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
    if (at->state != SYN_ATUNE_DONE) return;

    at->ctrl->cfg.pid_kp = at->result.kp;
    at->ctrl->cfg.pid_ki = at->result.ki;
    at->ctrl->cfg.pid_kd = at->result.kd;
    at->ctrl->cfg.ff_kv  = at->result.ff_kv;
    at->ctrl->cfg.ff_ka  = at->result.ff_ka;
    at->ctrl->cfg.pid_scale = at->result.pid_scale;
}

void syn_autotune_abort(SYN_AutoTune *at)
{
    if (at == NULL) return;
    SYN_ASSERT(at != NULL);
    emergency_stop(at, SYN_ATUNE_ABORT_USER);
}

SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Limits *limits,
                               SYN_AutoTune_Flags flags,
                               uint16_t gain_multiplier)
{
    SYN_ASSERT(at != NULL);
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(limits != NULL);

    SYN_AutoTune_Config cfg = {
        .mode           = SYN_ATUNE_MODE_AUTO,
        .flags          = flags,
        .test_output    = 0, /* Auto-probe */
        .limits         = *limits,
        .relay_cycles   = 4,
        .method         = SYN_ATUNE_TYREUS_LUYBEN,
        .watchdog_ms    = limits->watchdog_ms,
        .gain_multiplier_pct = gain_multiplier,
    };

    return syn_autotune_init(at, ctrl, &cfg);
}

#endif /* SYN_USE_AUTOTUNE */
