#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_AUTOTUNE) || SYN_USE_AUTOTUNE

#include "syn_autotune.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static void apply_raw_output(SYN_AutoTune *at, int32_t output)
{
    at->current_output = output;
    if (at->ctrl->cfg.type == SYN_MCTRL_DC && at->ctrl->cfg.dc_motor != NULL) {
        int16_t speed;
        if (output > 100) speed = 100;
        else if (output < -100) speed = -100;
        else speed = (int16_t)output;
        syn_dc_motor_set_speed(at->ctrl->cfg.dc_motor, speed);
        syn_dc_motor_update(at->ctrl->cfg.dc_motor);
    }
}

static void emergency_stop(SYN_AutoTune *at, SYN_AutoTune_AbortReason reason)
{
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

/** Check all safety conditions. Returns true if safe. */
static bool safety_ok(SYN_AutoTune *at, int32_t pos, int32_t velocity)
{
    uint32_t now = syn_port_get_tick_ms();

    /* Watchdog: update() must be called regularly */
    if (at->last_update_tick > 0 && at->cfg.watchdog_ms > 0) {
        if ((now - at->last_update_tick) > at->cfg.watchdog_ms) {
            emergency_stop(at, SYN_ATUNE_ABORT_WATCHDOG);
            return false;
        }
    }

    /* Position limit */
    int32_t drift = pos - at->start_position;
    if (drift < 0) drift = -drift;
    if (drift > at->cfg.position_limit) {
        emergency_stop(at, SYN_ATUNE_ABORT_POSITION);
        return false;
    }

    /* Velocity limit */
    if (at->cfg.velocity_limit > 0) {
        int32_t abs_vel = (velocity < 0) ? -velocity : velocity;
        if (abs_vel > at->cfg.velocity_limit) {
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
    SYN_ASSERT(cfg->test_output > 0);

    /* position_limit is mandatory — refuse to start without it */
    if (cfg->position_limit == 0) return SYN_ERROR;

    memset(at, 0, sizeof(*at));
    at->cfg  = *cfg;
    at->ctrl = ctrl;

    /* Apply defaults */
    if (at->cfg.settle_ms == 0)    at->cfg.settle_ms = 2000;
    if (at->cfg.measure_ms == 0)   at->cfg.measure_ms = 2000;
    if (at->cfg.relay_cycles == 0) at->cfg.relay_cycles = 3;
    if (at->cfg.watchdog_ms == 0)  at->cfg.watchdog_ms = 200;
    if (at->cfg.ramp_ms == 0)      at->cfg.ramp_ms = 500;

    /* Snapshot starting position */
    at->start_position = ctrl->measured_position;
    at->phase_start_tick = syn_port_get_tick_ms();
    at->last_update_tick = at->phase_start_tick;

    /* Stop the motor controller — we're taking over */
    syn_motor_ctrl_stop(ctrl);

    /* Start with ramp-up phase */
    at->state = SYN_ATUNE_RAMP_UP;

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
        return at->state; /* already aborted inside safety_ok */
    }
    at->last_update_tick = now;

    uint32_t elapsed = now - at->phase_start_tick;

    switch (at->state) {

    /* ── Ramp up to test output ────────────────────────────────── */
    case SYN_ATUNE_RAMP_UP: {
        int32_t target;
        if (at->cfg.mode == SYN_ATUNE_MODE_RELAY) {
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
            if (at->cfg.mode == SYN_ATUNE_MODE_FF_IDENT) {
                at->state = SYN_ATUNE_SETTLING;
            } else {
                at->state = SYN_ATUNE_RELAY;
                at->relay_output = target;
            }
        }
        break;
    }

    /* ── FF ident: settling phase ──────────────────────────────── */
    case SYN_ATUNE_SETTLING:
        apply_raw_output(at, at->cfg.test_output);
        if (elapsed >= at->cfg.settle_ms) {
            at->state = SYN_ATUNE_MEASURING;
            at->phase_start_tick = now;
            at->velocity_sum = 0;
            at->velocity_samples = 0;
        }
        break;

    /* ── FF ident: measuring phase ─────────────────────────────── */
    case SYN_ATUNE_MEASURING:
        apply_raw_output(at, at->cfg.test_output);
        at->velocity_sum += velocity;
        at->velocity_samples++;

        if (elapsed >= at->cfg.measure_ms) {
            /* Ramp down before computing results */
            at->phase_start_tick = now;
            at->state = SYN_ATUNE_RAMP_DOWN;

            /* Compute ff_kv */
            if (at->velocity_samples > 0 && at->velocity_sum != 0) {
                int32_t steady_vel = (int32_t)(at->velocity_sum
                                    / (int64_t)at->velocity_samples);
                at->result.steady_velocity = steady_vel;

                int32_t abs_vel = (steady_vel < 0) ? -steady_vel : steady_vel;
                if (abs_vel > 0) {
                    /* Auto-compute ff_scale for adequate precision.
                     * We want ff_kv >= 16 for at least 4 bits of precision.
                     * ff_kv = (test_output << scale) / abs_vel
                     * So scale = ceil(log2(abs_vel * 16 / test_output)) */
                    uint8_t ff_scale = at->ctrl->cfg.ff_scale;
                    if (ff_scale == 0) ff_scale = 8;

                    /* Increase scale until ff_kv would be >= 16 */
                    while (ff_scale < 24 &&
                           (((int64_t)at->cfg.test_output << ff_scale) / abs_vel) < 16) {
                        ff_scale++;
                    }

                    at->result.ff_scale = ff_scale;
                    at->result.ff_kv = (int32_t)(((int64_t)at->cfg.test_output << ff_scale)
                                               / abs_vel);
                }
            }
        }
        break;

    /* ── Relay feedback oscillation ────────────────────────────── */
    case SYN_ATUNE_RELAY: {
        /* Hysteresis: require position to be at least this far past setpoint
         * before registering a zero-crossing. Prevents encoder noise from
         * creating fake crossings on heavy/slow systems. */
        int32_t hysteresis = at->ctrl->cfg.position_deadband * 4;
        if (hysteresis < 20) hysteresis = 20;  /* minimum 20 counts */

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
            at->result.pid_scale = scale;

            /* Ku = 4 * d / (π * a)  where d = relay amplitude (output %),
             * a = half peak-to-peak oscillation amplitude (position counts).
             *
             * We compute Ku pre-scaled by pid_scale so the resulting PID
             * gains are directly usable:
             *   Ku_scaled = (4 * d * (1 << scale)) / (π * a)
             *
             * π ≈ 355/113 (accurate to 2.7e-7) */
            int64_t Ku_num = (int64_t)4 * at->cfg.test_output * 113
                           * ((int64_t)1 << scale);
            int64_t Ku_den = (int64_t)355 * half_amp;
            if (Ku_den == 0) Ku_den = 1;
            int32_t Ku = (int32_t)(Ku_num / Ku_den);
            at->result.Ku = Ku;

            int32_t Tu_i = (int32_t)Tu;
            switch (at->cfg.method) {
            case SYN_ATUNE_ZN_CLASSIC:
                /* Kp = 0.6*Ku, Ki = 1.2*Ku/Tu, Kd = 3*Ku*Tu/40 */
                at->result.kp = (int32_t)((int64_t)Ku * 60 / 100);
                at->result.ki = (int32_t)((int64_t)Ku * 120 / Tu_i);
                at->result.kd = (int32_t)((int64_t)Ku * 3 * Tu_i / (40 * 1000));
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
        }
        break;
    }

    /* ── Ramp down to zero ─────────────────────────────────────── */
    case SYN_ATUNE_RAMP_DOWN: {
        int32_t out = ramp_output(at, at->current_output,
                                   at->cfg.ramp_ms - elapsed);
        if (elapsed >= at->cfg.ramp_ms || out == 0) {
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
    SYN_ASSERT(at->state == SYN_ATUNE_DONE);

    if (at->result.ff_kv != 0) {
        at->ctrl->cfg.ff_kv = at->result.ff_kv;
        at->ctrl->cfg.ff_scale = at->result.ff_scale;
    }

    if (at->cfg.mode == SYN_ATUNE_MODE_RELAY) {
        syn_motor_ctrl_set_gains(at->ctrl,
                                  at->result.kp,
                                  at->result.ki,
                                  at->result.kd);
    }
}

void syn_autotune_abort(SYN_AutoTune *at)
{
    SYN_ASSERT(at != NULL);
    emergency_stop(at, SYN_ATUNE_ABORT_USER);
}

SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Limits *limits)
{
    /* Not yet implemented — use syn_autotune_init() for manual control. */
    (void)at;
    (void)ctrl;
    (void)limits;
    return SYN_ERROR;
}

#endif /* SYN_USE_AUTOTUNE */
