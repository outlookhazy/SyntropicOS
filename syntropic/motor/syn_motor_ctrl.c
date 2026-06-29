#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MOTOR_CTRL) || SYN_USE_MOTOR_CTRL

#if defined(SYN_USE_PID) && !SYN_USE_PID
  #error "syn_motor_ctrl requires SYN_USE_PID=1"
#endif

/**
 * @file syn_motor_ctrl.c
 * @brief Closed-loop motor controller implementation.
 *
 * Control loop:
 *   1. Read feedback via read_pos() function pointer
 *   2. Compute velocity = delta × update_hz
 *   3. Compute PID error (velocity or position mode)
 *   4. Enforce soft position limits
 *   5. Apply PID output to motor
 *   6. Check stall condition
 */

#include "syn_motor_ctrl.h"
#include "../util/syn_assert.h"
#include "../system/syn_errlog.h"

#include <string.h>

/** @internal Motor controller error codes (for errlog). */
#define SYN_MCTRL_ERR_STALL   0x0100  /**< Stall condition detected.  */
#define SYN_MCTRL_ERR_LIMIT   0x0101  /**< Soft position limit hit.   */

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Apply PID output to a DC motor.
 * @param ctrl    Motor controller.
 * @param output  PID output value.
 */
static void apply_output_dc(SYN_MotorCtrl *ctrl, int32_t output)
{
    if (ctrl->cfg.dc_motor == NULL) return;

    /* Clamp PID output to the DC motor's signed range (-100 to +100) */
    int16_t speed;
    if (output > 100) speed = 100;
    else if (output < -100) speed = -100;
    else speed = (int16_t)output;

    syn_dc_motor_set_speed(ctrl->cfg.dc_motor, speed);
    syn_dc_motor_update(ctrl->cfg.dc_motor);
}

/**
 * @brief Apply PID output to a stepper motor.
 * @param ctrl    Motor controller.
 * @param output  PID output value (unused for stepper tick).
 */
static void apply_output_stepper(SYN_MotorCtrl *ctrl, int32_t output)
{
    if (ctrl->cfg.stepper == NULL) return;
    (void)output;
    syn_stepper_tick(ctrl->cfg.stepper);
}

/**
 * @brief Coast/stop the motor (no active braking).
 * @param ctrl  Motor controller.
 */
static void stop_motor(SYN_MotorCtrl *ctrl)
{
    if (ctrl->cfg.type == SYN_MCTRL_DC && ctrl->cfg.dc_motor != NULL) {
        syn_dc_motor_coast(ctrl->cfg.dc_motor);
    } else if (ctrl->cfg.type == SYN_MCTRL_STEPPER && ctrl->cfg.stepper != NULL) {
        syn_stepper_stop(ctrl->cfg.stepper);
    }
}

/**
 * @brief Active-brake the motor.
 * @param ctrl  Motor controller.
 */
static void brake_motor(SYN_MotorCtrl *ctrl)
{
    if (ctrl->cfg.type == SYN_MCTRL_DC && ctrl->cfg.dc_motor != NULL) {
        syn_dc_motor_brake(ctrl->cfg.dc_motor);
    } else if (ctrl->cfg.type == SYN_MCTRL_STEPPER && ctrl->cfg.stepper != NULL) {
        syn_stepper_stop(ctrl->cfg.stepper);
    }
}

/**
 * @brief Read the current position from the encoder/sensor.
 * @param ctrl  Motor controller.
 * @return Current position.
 */
static int32_t read_position(SYN_MotorCtrl *ctrl)
{
    return ctrl->cfg.read_pos(ctrl->cfg.read_pos_ctx);
}

/**
 * @brief Check if soft position limits are configured.
 * @param ctrl  Motor controller.
 * @return true if limits are set.
 */
static bool limits_enabled(const SYN_MotorCtrl *ctrl)
{
    return (ctrl->cfg.position_min != 0 || ctrl->cfg.position_max != 0);
}

/**
 * @brief Check if position is at a soft limit in the given direction.
 * @param ctrl    Motor controller.
 * @param pos     Current position.
 * @param output  Intended output (sign indicates direction).
 * @return true if movement would exceed a limit.
 */
static bool at_limit(const SYN_MotorCtrl *ctrl, int32_t pos, int32_t output)
{
    if (!limits_enabled(ctrl)) return false;

    /* At min limit and trying to go further negative */
    if (pos <= ctrl->cfg.position_min && output < 0) return true;
    /* At max limit and trying to go further positive */
    if (pos >= ctrl->cfg.position_max && output > 0) return true;

    return false;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_motor_ctrl_init(SYN_MotorCtrl *ctrl,
                                 const SYN_MotorCtrl_Config *cfg)
{
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(cfg->read_pos != NULL);
    SYN_ASSERT(cfg->update_hz > 0);

    memset(ctrl, 0, sizeof(*ctrl));
    ctrl->cfg = *cfg;

    SYN_PID_Config pid_cfg = {
        .kp         = cfg->pid_kp,
        .ki         = cfg->pid_ki,
        .kd         = cfg->pid_kd,
        .scale      = (int32_t)1 << cfg->pid_scale,
        .out_min    = cfg->output_min,
        .out_max    = cfg->output_max,
        .integral_max = cfg->output_max * 4,
        .d_filter_alpha = 200,
    };
    syn_pid_init(&ctrl->pid, &pid_cfg);

    ctrl->mode  = SYN_MCTRL_MODE_IDLE;
    ctrl->state = SYN_MCTRL_STOPPED;
    ctrl->last_position    = read_position(ctrl);
    ctrl->measured_position = ctrl->last_position;
    ctrl->last_update_tick = syn_port_get_tick_ms();
    ctrl->enabled          = true;
    ctrl->trajectory_active = false;
    ctrl->ff_output        = 0;
    ctrl->total_output     = 0;
    ctrl->datalog          = NULL;

    return SYN_OK;
}

void syn_motor_ctrl_set_velocity(SYN_MotorCtrl *ctrl, int32_t units_per_sec)
{
    SYN_ASSERT(ctrl != NULL);

    ctrl->target_velocity = units_per_sec;
    ctrl->mode  = SYN_MCTRL_MODE_VELOCITY;
    ctrl->state = SYN_MCTRL_RUNNING;
    ctrl->stall_active = false;
    ctrl->trajectory_active = false;

    syn_pid_reset(&ctrl->pid);
    ctrl->last_position    = read_position(ctrl);
    ctrl->last_update_tick = syn_port_get_tick_ms();
}

void syn_motor_ctrl_set_position(SYN_MotorCtrl *ctrl, int32_t target)
{
    SYN_ASSERT(ctrl != NULL);

    /* Clamp target to soft limits */
    if (limits_enabled(ctrl)) {
        if (target < ctrl->cfg.position_min) target = ctrl->cfg.position_min;
        if (target > ctrl->cfg.position_max) target = ctrl->cfg.position_max;
    }

    ctrl->target_position = target;
    ctrl->mode  = SYN_MCTRL_MODE_POSITION;
    ctrl->state = SYN_MCTRL_RUNNING;
    ctrl->stall_active = false;
    ctrl->trajectory_active = false;

    syn_pid_reset(&ctrl->pid);
    ctrl->last_position    = read_position(ctrl);
    ctrl->last_update_tick = syn_port_get_tick_ms();
}

void syn_motor_ctrl_set_trajectory(SYN_MotorCtrl *ctrl,
                                    const SYN_MotorCtrl_Trajectory *traj)
{
    SYN_ASSERT(ctrl != NULL);
    SYN_ASSERT(traj != NULL);

    ctrl->trajectory = *traj;
    ctrl->target_position = traj->position;
    ctrl->trajectory_active = true;

    /* First call: switch mode and reset PID */
    if (ctrl->mode != SYN_MCTRL_MODE_POSITION || ctrl->state == SYN_MCTRL_STOPPED) {
        ctrl->mode  = SYN_MCTRL_MODE_POSITION;
        ctrl->state = SYN_MCTRL_RUNNING;
        ctrl->stall_active = false;
        syn_pid_reset(&ctrl->pid);
        ctrl->last_position    = read_position(ctrl);
        ctrl->last_update_tick = syn_port_get_tick_ms();
    }
}

void syn_motor_ctrl_stop(SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->mode = SYN_MCTRL_MODE_IDLE;
    ctrl->state = SYN_MCTRL_STOPPED;
    ctrl->pid_output = 0;
    ctrl->ff_output = 0;
    ctrl->total_output = 0;
    ctrl->trajectory_active = false;
    stop_motor(ctrl);
    syn_pid_reset(&ctrl->pid);
}

void syn_motor_ctrl_estop(SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->mode = SYN_MCTRL_MODE_IDLE;
    ctrl->state = SYN_MCTRL_STOPPED;
    ctrl->pid_output = 0;
    ctrl->ff_output = 0;
    ctrl->total_output = 0;
    ctrl->trajectory_active = false;
    brake_motor(ctrl);
    syn_pid_reset(&ctrl->pid);
}

SYN_MotorCtrl_State syn_motor_ctrl_update(SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);

    if (ctrl->mode == SYN_MCTRL_MODE_IDLE || !ctrl->enabled) {
        return ctrl->state;
    }

    uint32_t now = syn_port_get_tick_ms();
    uint32_t dt_ms = now - ctrl->last_update_tick;
    if (dt_ms == 0) dt_ms = 1;

    /* ── Read feedback ──────────────────────────────────────────── */
    int32_t current_pos = read_position(ctrl);
    int32_t delta = current_pos - ctrl->last_position;
    ctrl->last_position     = current_pos;
    ctrl->measured_position = current_pos;

    /* Velocity = delta × update_hz (units per second) */
    ctrl->measured_velocity = delta * (int32_t)ctrl->cfg.update_hz;

    /* ── Compute feedforward ────────────────────────────────────── */
    int32_t ff = 0;
    if (ctrl->trajectory_active &&
        (ctrl->cfg.ff_kv != 0 || ctrl->cfg.ff_ka != 0)) {
        int32_t ff_scale = (ctrl->cfg.ff_scale > 0)
                         ? (int32_t)1 << ctrl->cfg.ff_scale
                         : 1;
        ff = (ctrl->cfg.ff_kv * ctrl->trajectory.velocity
            + ctrl->cfg.ff_ka * ctrl->trajectory.acceleration) / ff_scale;

        /* Clamp FF to leave headroom for PID correction.
         * FF gets at most 90% of the output range — PID needs room to work. */
        int32_t ff_max = (ctrl->cfg.output_max * 9) / 10;
        int32_t ff_min = (ctrl->cfg.output_min * 9) / 10;
        if (ff > ff_max) ff = ff_max;
        if (ff < ff_min) ff = ff_min;
    }
    ctrl->ff_output = ff;

    /* ── Compute PID ────────────────────────────────────────────── */
    int32_t pid_out;

    if (ctrl->mode == SYN_MCTRL_MODE_VELOCITY) {
        /* Velocity: setpoint is target velocity, measured is actual */
        pid_out = syn_pid_update(&ctrl->pid, ctrl->target_velocity,
                                 ctrl->measured_velocity, dt_ms);
    } else {
        /* Position mode (includes trajectory tracking) */
        int32_t error = ctrl->target_position - current_pos;

        /* Check deadband (only when trajectory is not actively driving) */
        if (!ctrl->trajectory_active) {
            int32_t abs_error = (error < 0) ? -error : error;
            if (abs_error <= ctrl->cfg.position_deadband) {
                if (ctrl->state != SYN_MCTRL_ON_TARGET) {
                    ctrl->state = SYN_MCTRL_ON_TARGET;
                    ctrl->pid_output = 0;
                    ctrl->ff_output = 0;
                    ctrl->total_output = 0;
                    stop_motor(ctrl);
                    syn_pid_reset(&ctrl->pid);

                    if (ctrl->on_target != NULL) {
                        ctrl->on_target(ctrl, ctrl->on_target_ctx);
                    }
                }
                return ctrl->state;
            }
        }

        ctrl->state = SYN_MCTRL_RUNNING;
        pid_out = syn_pid_update(&ctrl->pid, ctrl->target_position,
                                 current_pos, dt_ms);
    }

    ctrl->pid_output = pid_out;

    /* ── Combine feedforward + feedback ─────────────────────────── */
    int32_t output = pid_out + ff;

    /* Clamp combined output — with FF-aware anti-windup.
     * If the combined output saturates, the PID integrator is frozen
     * to prevent windup. This is critical when FF consumes most of
     * the output headroom (e.g., during high-speed cruise). */
    bool saturated = false;
    if (output > ctrl->cfg.output_max) {
        output = ctrl->cfg.output_max;
        saturated = true;
    }
    if (output < ctrl->cfg.output_min) {
        output = ctrl->cfg.output_min;
        saturated = true;
    }

    if (saturated) {
        /* Freeze integrator — don't let it accumulate while we're clipping */
        int32_t max_pid = ctrl->cfg.output_max - ff;
        int32_t min_pid = ctrl->cfg.output_min - ff;
        if (max_pid < min_pid) { int32_t t = max_pid; max_pid = min_pid; min_pid = t; }
        /* Clamp the integrator to keep PID output within available headroom */
        if (ctrl->pid.integral > 0 && pid_out > max_pid) {
            ctrl->pid.integral -= (pid_out - max_pid) * ctrl->pid.cfg.scale;
        }
        if (ctrl->pid.integral < 0 && pid_out < min_pid) {
            ctrl->pid.integral -= (pid_out - min_pid) * ctrl->pid.cfg.scale;
        }
    }

    ctrl->total_output = output;

    /* ── Enforce soft position limits ───────────────────────────── */
    if (at_limit(ctrl, current_pos, output)) {
        ctrl->state = SYN_MCTRL_LIMIT;
        ctrl->pid_output = 0;
        ctrl->ff_output = 0;
        ctrl->total_output = 0;
        stop_motor(ctrl);
        syn_pid_reset(&ctrl->pid);
        if (ctrl->cfg.errlog != NULL) {
            syn_errlog_record(ctrl->cfg.errlog, SYN_MCTRL_ERR_LIMIT,
                               SYN_ERR_WARNING, (uint32_t)current_pos);
        }
        return ctrl->state;
    }

    if (ctrl->state != SYN_MCTRL_STALLED) {
        ctrl->state = SYN_MCTRL_RUNNING;
    }

    /* ── Apply to motor ─────────────────────────────────────────── */
    if (ctrl->cfg.type == SYN_MCTRL_DC) {
        apply_output_dc(ctrl, output);
    } else {
        apply_output_stepper(ctrl, output);
    }

    /* ── Tuning capture ─────────────────────────────────────────── */
    if (ctrl->datalog != NULL) {
        SYN_MotorCtrl_Sample sample = {
            .tick_ms      = now,
            .target_pos   = ctrl->target_position,
            .measured_pos = current_pos,
            .target_vel   = ctrl->trajectory_active
                          ? ctrl->trajectory.velocity
                          : ctrl->target_velocity,
            .measured_vel = ctrl->measured_velocity,
            .ff_output    = ff,
            .pid_output   = pid_out,
            .total_output = output,
        };
        syn_datalog_write(ctrl->datalog, SYN_MCTRL_DATALOG_ID,
                           &sample, sizeof(sample));
    }

    /* ── Accumulate move metrics ────────────────────────────────── */
    {
        int32_t pos_error = ctrl->target_position - current_pos;
        int32_t abs_err = (pos_error < 0) ? -pos_error : pos_error;
        int32_t abs_out = (output < 0) ? -output : output;

        if (abs_err > ctrl->metrics.max_error)
            ctrl->metrics.max_error = abs_err;

        ctrl->metrics.error_sq_sum += (int64_t)pos_error * pos_error;

        if (abs_out > ctrl->metrics.peak_output)
            ctrl->metrics.peak_output = abs_out;

        /* Overshoot: position went past target */
        if (ctrl->mode == SYN_MCTRL_MODE_POSITION) {
            int32_t beyond = current_pos - ctrl->target_position;
            /* Overshoot direction depends on which side we approached from.
             * Use abs(beyond) if error has changed sign (we crossed target). */
            int32_t abs_beyond = (beyond < 0) ? -beyond : beyond;
            if (abs_err < ctrl->metrics.max_error && abs_beyond > ctrl->metrics.overshoot) {
                /* Only count as overshoot once we've been closer before */
                ctrl->metrics.overshoot = abs_beyond;
            }
        }

        /* Settle time: first time within deadband */
        if (ctrl->metrics.settle_tick == 0 &&
            abs_err <= ctrl->cfg.position_deadband) {
            ctrl->metrics.settle_tick = now;
        }

        ctrl->metrics.sample_count++;
    }

    /* ── Stall detection ────────────────────────────────────────── */
    if (ctrl->cfg.stall_timeout_ms > 0) {
        int32_t abs_delta = (delta < 0) ? -delta : delta;
        int32_t abs_output = (output < 0) ? -output : output;

        if (abs_output > (ctrl->cfg.output_max / 4) &&
            abs_delta <= ctrl->cfg.stall_threshold) {
            if (!ctrl->stall_active) {
                ctrl->stall_active = true;
                ctrl->stall_start_tick = now;
            } else if ((now - ctrl->stall_start_tick) >= ctrl->cfg.stall_timeout_ms) {
                ctrl->state = SYN_MCTRL_STALLED;
                ctrl->mode  = SYN_MCTRL_MODE_IDLE;
                ctrl->trajectory_active = false;
                stop_motor(ctrl);
                syn_pid_reset(&ctrl->pid);

                if (ctrl->cfg.errlog != NULL) {
                    syn_errlog_record(ctrl->cfg.errlog, SYN_MCTRL_ERR_STALL,
                                       SYN_ERR_ERROR, (uint32_t)current_pos);
                }

                if (ctrl->on_stall != NULL) {
                    ctrl->on_stall(ctrl, ctrl->on_stall_ctx);
                }
            }
        } else {
            ctrl->stall_active = false;
        }
    }

    ctrl->last_update_tick = now;
    return ctrl->state;
}

void syn_motor_ctrl_on_stall(SYN_MotorCtrl *ctrl,
                               SYN_MotorCtrl_StallCallback cb, void *ctx)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->on_stall     = cb;
    ctrl->on_stall_ctx = ctx;
}

void syn_motor_ctrl_on_target(SYN_MotorCtrl *ctrl,
                                SYN_MotorCtrl_TargetCallback cb, void *ctx)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->on_target     = cb;
    ctrl->on_target_ctx = ctx;
}

void syn_motor_ctrl_set_gains(SYN_MotorCtrl *ctrl,
                                int32_t kp, int32_t ki, int32_t kd)
{
    SYN_ASSERT(ctrl != NULL);
    syn_pid_set_gains(&ctrl->pid, kp, ki, kd);
}

void syn_motor_ctrl_set_ff_gains(SYN_MotorCtrl *ctrl,
                                  int32_t ff_kv, int32_t ff_ka)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->cfg.ff_kv = ff_kv;
    ctrl->cfg.ff_ka = ff_ka;
}

void syn_motor_ctrl_clear_stall(SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);
    if (ctrl->state == SYN_MCTRL_STALLED) {
        ctrl->state            = SYN_MCTRL_STOPPED;
        ctrl->stall_start_tick = 0;
        ctrl->stall_active     = false;
        ctrl->trajectory_active = false;
        syn_pid_reset(&ctrl->pid);
    }
}

void syn_motor_ctrl_set_datalog(SYN_MotorCtrl *ctrl, SYN_DataLog *log)
{
    SYN_ASSERT(ctrl != NULL);
    ctrl->datalog = log;
}

void syn_motor_ctrl_reset_metrics(SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);
    memset(&ctrl->metrics, 0, sizeof(ctrl->metrics));
    ctrl->metrics.move_start_tick = syn_port_get_tick_ms();
}

/**
 * @brief Integer square root via binary search.
 * @param n  Input value.
 * @return Floor of sqrt(n).
 */
static int32_t isqrt(int64_t n)
{
    if (n <= 0) return 0;
    int64_t x = 1;
    while (x * x <= n) x <<= 1;
    int64_t lo = x >> 1, hi = x;
    while (lo <= hi) {
        int64_t mid = (lo + hi) / 2;
        if (mid * mid <= n) lo = mid + 1;
        else                hi = mid - 1;
    }
    return (int32_t)hi;
}

int32_t syn_motor_ctrl_rms_error(const SYN_MotorCtrl *ctrl)
{
    SYN_ASSERT(ctrl != NULL);
    if (ctrl->metrics.sample_count == 0) return 0;
    int64_t mean_sq = ctrl->metrics.error_sq_sum
                    / (int64_t)ctrl->metrics.sample_count;
    return isqrt(mean_sq);
}

#endif /* SYN_USE_MOTOR_CTRL */
