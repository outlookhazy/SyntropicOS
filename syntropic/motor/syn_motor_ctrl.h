/**
 * @file syn_motor_ctrl.h
 * @brief Closed-loop motor controller — generic feedback + PID + motor.
 *
 * Combines any position/velocity feedback source with PID control to
 * drive DC or stepper motors in closed loop. The feedback source is a
 * function pointer, so it works equally well with:
 *   - Rotary encoder (syn_encoder)
 *   - Potentiometer / linear pot on ADC (syn_adc)
 *   - Hall sensor, absolute encoder, or any other position source
 *
 * @par Modes
 * - **Velocity**: PID maintains target units/second
 * - **Position**: PID drives to target position value
 * - **Trajectory**: PID + feedforward tracks a trajectory from an external
 *                   generator (syn_ramp, syn_scurve, or application code)
 *
 * @par Example — DC motor + encoder (velocity control)
 * @code
 *   // Feedback: read encoder position
 *   int32_t encoder_feedback(void *ctx) {
 *       return syn_encoder_position((SYN_Encoder *)ctx);
 *   }
 *
 *   SYN_MotorCtrl ctrl;
 *   SYN_MotorCtrl_Config cfg = {
 *       .type        = SYN_MCTRL_DC,
 *       .read_pos    = encoder_feedback,
 *       .read_pos_ctx = &my_encoder,
 *       .dc_motor    = &my_dc,
 *       .pid_kp = 200, .pid_ki = 50, .pid_kd = 10, .pid_scale = 8,
 *       .update_hz   = 100,
 *       .output_min  = -255, .output_max = 255,
 *   };
 *   syn_motor_ctrl_init(&ctrl, &cfg);
 *   syn_motor_ctrl_set_velocity(&ctrl, 500);  // 500 counts/sec
 * @endcode
 *
 * @par Example — Trajectory tracking with feedforward
 * @code
 *   SYN_MotorCtrl_Config cfg = {
 *       // ... feedback, motor, PID as above ...
 *       .ff_kv    = 128,   // velocity feedforward gain
 *       .ff_ka    = 64,    // acceleration feedforward gain
 *       .ff_scale = 8,     // feedforward divisor = 1 << 8 = 256
 *   };
 *   syn_motor_ctrl_init(&ctrl, &cfg);
 *
 *   // In your control loop, feed trajectory from any generator:
 *   SYN_MotorCtrl_Trajectory traj = {
 *       .position     = profile_position,
 *       .velocity     = profile_velocity,
 *       .acceleration = profile_acceleration,
 *   };
 *   syn_motor_ctrl_set_trajectory(&ctrl, &traj);
 *   syn_motor_ctrl_update(&ctrl);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_MOTOR_CTRL_H
#define SYN_MOTOR_CTRL_H

#include "../common/syn_defs.h"
#include "../control/syn_pid.h"
#include "../motor/syn_dc_motor.h"
#include "../motor/syn_stepper.h"
#include "../port/syn_port_system.h"
#include "../system/syn_errlog.h"
#include "../log/syn_datalog.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Feedback function ──────────────────────────────────────────────────── */

/**
 * @brief Position feedback function.
 *
 * Returns the current position in whatever units the sensor provides
 * (encoder counts, ADC ticks, millivolts — the PID doesn't care).
 * The same units must be used for all targets and deadbands.
 *
 * @param ctx  User context.
 * @return Current position in feedback units.
 */
typedef int32_t (*SYN_MotorCtrl_ReadPos)(void *ctx);

/* ── Motor type ─────────────────────────────────────────────────────────── */

/** @brief Motor driver hardware type. */
typedef enum {
    SYN_MCTRL_DC      = 0,   /**< DC motor (PWM output)                 */
    SYN_MCTRL_STEPPER = 1,   /**< Stepper motor (step rate output)      */
} SYN_MotorCtrl_Type;

/* ── Control mode ───────────────────────────────────────────────────────── */

/** @brief Control loop operating mode. */
typedef enum {
    SYN_MCTRL_MODE_IDLE     = 0,  /**< Motor stopped, controller off     */
    SYN_MCTRL_MODE_VELOCITY = 1,  /**< Maintain target velocity          */
    SYN_MCTRL_MODE_POSITION = 2,  /**< Move to target position           */
} SYN_MotorCtrl_Mode;

/* ── Controller state ───────────────────────────────────────────────────── */

/** @brief Motor controller runtime state. */
typedef enum {
    SYN_MCTRL_STOPPED    = 0,  /**< Motor off, controller idle           */
    SYN_MCTRL_RUNNING    = 1,  /**< Actively driving toward target       */
    SYN_MCTRL_ON_TARGET  = 2,  /**< Position mode: within deadband       */
    SYN_MCTRL_STALLED    = 3,  /**< Feedback not changing despite output  */
    SYN_MCTRL_LIMIT      = 4,  /**< Hit a soft position limit            */
} SYN_MotorCtrl_State;

/* ── Callbacks ──────────────────────────────────────────────────────────── */

struct SYN_MotorCtrl;

/**
 * @brief Callback invoked when a motor stall is detected.
 * @param ctrl  Controller that detected the stall.
 * @param ctx   User context.
 */
typedef void (*SYN_MotorCtrl_StallCallback)(struct SYN_MotorCtrl *ctrl,
                                              void *ctx);

/**
 * @brief Callback invoked when position reaches the target deadband.
 * @param ctrl  Controller that reached target.
 * @param ctx   User context.
 */
typedef void (*SYN_MotorCtrl_TargetCallback)(struct SYN_MotorCtrl *ctrl,
                                               void *ctx);

/* ── Trajectory input ───────────────────────────────────────────────────── */

/**
 * @brief Trajectory setpoint for feedforward control.
 *
 * Produced by an external trajectory generator (syn_ramp, syn_scurve,
 * or application code) and fed to the controller each update.
 */
typedef struct {
    int32_t position;       /**< Target position (feedback units)         */
    int32_t velocity;       /**< Target velocity (units/sec)              */
    int32_t acceleration;   /**< Target acceleration (units/sec²)         */
} SYN_MotorCtrl_Trajectory;

/* ── Tuning capture sample ──────────────────────────────────────────────── */

/** Datalog stream ID for motor controller telemetry. */
#define SYN_MCTRL_DATALOG_ID  0x4D43  /* 'MC' */

/**
 * @brief One sample of control-loop telemetry.
 *
 * Written to the attached SYN_DataLog every update() call.
 * At 100 Hz with a 4 KB buffer you get ~1.2 seconds of capture.
 */
typedef struct {
    uint32_t tick_ms;         /**< Timestamp                              */
    int32_t  target_pos;      /**< Trajectory/target position             */
    int32_t  measured_pos;    /**< Measured position (from feedback)       */
    int32_t  target_vel;      /**< Trajectory/target velocity             */
    int32_t  measured_vel;    /**< Measured velocity                      */
    int32_t  ff_output;       /**< Feedforward contribution               */
    int32_t  pid_output;      /**< PID feedback contribution              */
    int32_t  total_output;    /**< Final clamped output to motor          */
} SYN_MotorCtrl_Sample;

/* ── Move metrics (computed on-the-fly, zero buffer cost) ───────────────── */

/**
 * @brief Accumulated metrics for a single move/trajectory.
 *
 * Updated every update() call. Read back after a move completes to
 * evaluate tracking performance without storing per-sample data.
 * Call syn_motor_ctrl_reset_metrics() before a move to start fresh.
 */
typedef struct {
    int32_t  max_error;        /**< Peak |position_error| during move       */
    int64_t  error_sq_sum;     /**< Sum of error² (for RMS computation)     */
    int32_t  overshoot;        /**< Max overshoot past target (position)    */
    int32_t  peak_output;      /**< Peak |output| applied to motor          */
    uint32_t sample_count;     /**< Number of update() calls in this move   */
    uint32_t move_start_tick;  /**< Tick when move started                  */
    uint32_t settle_tick;      /**< Tick when first entered deadband (0=never) */
} SYN_MotorCtrl_Metrics;

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief Motor controller configuration (passed to init, copied internally). */
typedef struct {
    SYN_MotorCtrl_Type type;           /**< Motor hardware type              */

    /* Feedback source (mandatory) */
    SYN_MotorCtrl_ReadPos read_pos;     /**< Position read function       */
    void                  *read_pos_ctx; /**< Context for read_pos         */

    /* Output (set one based on type) */
    SYN_DCMotor       *dc_motor;     /**< For SYN_MCTRL_DC              */
    SYN_Stepper       *stepper;      /**< For SYN_MCTRL_STEPPER         */

    /* PID gains (integer, divided by 1 << pid_scale) */
/** @brief PID proportional gain (÷ 1 << pid_scale). */
    int32_t             pid_kp;
    /** @brief PID integral gain (÷ 1 << pid_scale). */
    int32_t             pid_ki;
    /** @brief PID derivative gain (÷ 1 << pid_scale). */
    int32_t             pid_kd;
    uint8_t             pid_scale;    /**< Gain divisor = 1 << pid_scale   */

    /* Feedforward gains (optional — 0 = disabled)
     * Applied when using set_trajectory(). Output contribution:
     *   ff_output = (ff_kv × velocity + ff_ka × acceleration) >> ff_scale
     */
    int32_t             ff_kv;        /**< Velocity feedforward gain       */
    int32_t             ff_ka;        /**< Acceleration feedforward gain   */
    uint8_t             ff_scale;     /**< FF divisor = 1 << ff_scale      */

    /* Loop rate */
    uint16_t            update_hz;    /**< Control loop frequency          */

    /* Output limits */
    int32_t             output_min;   /**< Min PID output (e.g., -255)     */
    int32_t             output_max;   /**< Max PID output (e.g., +255)     */

    /* Position mode */
    int32_t             position_deadband; /**< Units within target = done */

    /* Soft position limits (0, 0 = disabled) */
    int32_t             position_min; /**< Min allowed position (soft limit)*/
    int32_t             position_max; /**< Max allowed position (soft limit)*/

    /* Stall detection */
    uint16_t            stall_timeout_ms;  /**< 0 = disabled              */
    int32_t             stall_threshold;   /**< Min units/period for "not stalled" */

    /* Error logging (optional) */
    SYN_ErrLog            *errlog;            /**< If set, stall/limit events logged */
} SYN_MotorCtrl_Config;

/* ── Controller instance ────────────────────────────────────────────────── */

/** @brief Motor controller instance (opaque — use API to access). */
typedef struct SYN_MotorCtrl {
    SYN_MotorCtrl_Config  cfg;      /**< Configuration snapshot             */
    SYN_PID               pid;      /**< Embedded PID controller            */

    SYN_MotorCtrl_Mode    mode;       /**< Current operating mode            */
    SYN_MotorCtrl_State   state;      /**< Current runtime state             */

    /* Targets */
    int32_t                target_velocity;  /**< Units per second         */
    int32_t                target_position;  /**< Target position value    */

    /* Trajectory (feedforward) */
    SYN_MotorCtrl_Trajectory trajectory;     /**< Current trajectory point */
    bool                   trajectory_active; /**< True when using set_trajectory */
    int32_t                ff_output;        /**< Last feedforward output  */

    /* Measurements */
    int32_t                measured_velocity; /**< Current velocity        */
    int32_t                measured_position; /**< Current position        */
    int32_t                last_position;     /**< Position at last update */
    int32_t                pid_output;        /**< Last PID output         */
    int32_t                total_output;      /**< Last combined output    */

    /* Timing */
    /** @brief Tick of last update() call. */
    uint32_t               last_update_tick;

    /** @brief Tick when stall condition was first detected. */
    uint32_t               stall_start_tick;
    /** @brief True while output is nonzero but position isn't changing. */
    bool                   stall_active;

    /** @brief Registered stall callback (NULL = none). */
    SYN_MotorCtrl_StallCallback  on_stall;
    /** @brief User context passed to stall callback. */
    void                         *on_stall_ctx;
    /** @brief Registered on-target callback (NULL = none). */
    SYN_MotorCtrl_TargetCallback on_target;
    /** @brief User context passed to on-target callback. */
    void                         *on_target_ctx;

    /** @brief True when the controller is enabled and driving output. */
    bool                   enabled;

    /* Tuning capture (optional — NULL to disable) */
    SYN_DataLog           *datalog;          /**< Attached datalog for telemetry */

    /** @brief Move metrics — accumulated during each move, zero buffer cost. */
    SYN_MotorCtrl_Metrics  metrics;
} SYN_MotorCtrl;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the closed-loop controller.
 * @param ctrl  Controller instance to initialize.
 * @param cfg   Configuration (copied internally).
 * @return SYN_OK on success.
 */
SYN_Status syn_motor_ctrl_init(SYN_MotorCtrl *ctrl,
                                 const SYN_MotorCtrl_Config *cfg);

/**
 * @brief Set velocity target (units/second).
 *
 * Switches to velocity mode. Positive = forward, negative = reverse.
 *
 * @param ctrl            Controller instance.
 * @param units_per_sec   Target velocity in feedback units per second.
 */
void syn_motor_ctrl_set_velocity(SYN_MotorCtrl *ctrl, int32_t units_per_sec);

/**
 * @brief Set position target (absolute feedback units).
 *
 * Switches to position mode. Controller drives toward target and
 * enters ON_TARGET state when within deadband.
 *
 * @param ctrl    Controller instance.
 * @param target  Absolute position in feedback units.
 */
void syn_motor_ctrl_set_position(SYN_MotorCtrl *ctrl, int32_t target);

/**
 * @brief Feed a trajectory point for feedforward + PID tracking.
 *
 * Call this each update with the current output of your trajectory
 * generator (syn_ramp, syn_scurve, or application code). The controller
 * uses PID on position error plus feedforward from the trajectory's
 * velocity and acceleration.
 *
 * @param ctrl  Controller instance.
 * @param traj  Trajectory point (position, velocity, acceleration).
 */
void syn_motor_ctrl_set_trajectory(SYN_MotorCtrl *ctrl,
                                    const SYN_MotorCtrl_Trajectory *traj);

/**
 * @brief Stop the motor and enter idle mode.
 * @param ctrl  Controller instance.
 */
void syn_motor_ctrl_stop(SYN_MotorCtrl *ctrl);

/**
 * @brief Emergency stop — immediate brake/coast.
 * @param ctrl  Controller instance.
 */
void syn_motor_ctrl_estop(SYN_MotorCtrl *ctrl);

/**
 * @brief Run the control loop. Call at cfg.update_hz rate.
 *
 * Reads feedback, computes PID, drives motor output.
 * If a datalog is attached, writes a SYN_MotorCtrl_Sample each call.
 *
 * @param ctrl  Controller instance.
 * @return Current controller state.
 */
SYN_MotorCtrl_State syn_motor_ctrl_update(SYN_MotorCtrl *ctrl);

/**
 * @brief Register stall callback.
 * @param ctrl  Controller instance.
 * @param cb    Callback function, or NULL to unregister.
 * @param ctx   User context passed to callback.
 */
void syn_motor_ctrl_on_stall(SYN_MotorCtrl *ctrl,
                               SYN_MotorCtrl_StallCallback cb, void *ctx);

/**
 * @brief Register on-target callback (position mode).
 * @param ctrl  Controller instance.
 * @param cb    Callback function, or NULL to unregister.
 * @param ctx   User context passed to callback.
 */
void syn_motor_ctrl_on_target(SYN_MotorCtrl *ctrl,
                                SYN_MotorCtrl_TargetCallback cb, void *ctx);

/**
 * @brief Update PID gains at runtime.
 * @param ctrl  Controller instance.
 * @param kp    Proportional gain (÷ 1 << pid_scale).
 * @param ki    Integral gain (÷ 1 << pid_scale).
 * @param kd    Derivative gain (÷ 1 << pid_scale).
 */
void syn_motor_ctrl_set_gains(SYN_MotorCtrl *ctrl,
                                int32_t kp, int32_t ki, int32_t kd);

/**
 * @brief Update feedforward gains at runtime.
 * @param ctrl   Controller instance.
 * @param ff_kv  Velocity feedforward gain (÷ 1 << ff_scale).
 * @param ff_ka  Acceleration feedforward gain (÷ 1 << ff_scale).
 */
void syn_motor_ctrl_set_ff_gains(SYN_MotorCtrl *ctrl,
                                  int32_t ff_kv, int32_t ff_ka);

/**
 * @brief Clear a stall condition and return to idle.
 *
 * After a stall, the controller locks in STALLED state until this is called.
 *
 * @param ctrl  Controller instance.
 */
void syn_motor_ctrl_clear_stall(SYN_MotorCtrl *ctrl);

/**
 * @brief Attach a datalog for tuning capture.
 *
 * When attached, every update() writes a SYN_MotorCtrl_Sample frame
 * to the datalog at full control-loop rate.
 *
 * @param ctrl  Controller.
 * @param log   Datalog instance (caller-owned), or NULL to detach.
 */
void syn_motor_ctrl_set_datalog(SYN_MotorCtrl *ctrl, SYN_DataLog *log);

/* ── Getters ────────────────────────────────────────────────────────────── */

/**
 * @brief Get the current controller state.
 * @param ctrl  Controller.
 * @return Runtime state.
 */
static inline SYN_MotorCtrl_State
syn_motor_ctrl_state(const SYN_MotorCtrl *ctrl)     { return ctrl->state; }

/**
 * @brief Get the current operating mode.
 * @param ctrl  Controller.
 * @return Operating mode.
 */
static inline SYN_MotorCtrl_Mode
syn_motor_ctrl_mode(const SYN_MotorCtrl *ctrl)      { return ctrl->mode; }

/**
 * @brief Get measured velocity.
 * @param ctrl  Controller.
 * @return Velocity in feedback units/second.
 */
static inline int32_t
syn_motor_ctrl_velocity(const SYN_MotorCtrl *ctrl)  { return ctrl->measured_velocity; }

/**
 * @brief Get measured position.
 * @param ctrl  Controller.
 * @return Position in feedback units.
 */
static inline int32_t
syn_motor_ctrl_position(const SYN_MotorCtrl *ctrl)  { return ctrl->measured_position; }

/**
 * @brief Get total output (PID + feedforward).
 * @param ctrl  Controller.
 * @return Combined output.
 */
static inline int32_t
syn_motor_ctrl_output(const SYN_MotorCtrl *ctrl)    { return ctrl->total_output; }

/**
 * @brief Get last PID output component.
 * @param ctrl  Controller.
 * @return PID output.
 */
static inline int32_t
syn_motor_ctrl_pid_output(const SYN_MotorCtrl *ctrl) { return ctrl->pid_output; }

/**
 * @brief Get last feedforward output component.
 * @param ctrl  Controller.
 * @return Feedforward output.
 */
static inline int32_t
syn_motor_ctrl_ff_output(const SYN_MotorCtrl *ctrl)  { return ctrl->ff_output; }

/**
 * @brief Get last position error.
 * @param ctrl  Controller.
 * @return Position error in feedback units.
 */
static inline int32_t
syn_motor_ctrl_error(const SYN_MotorCtrl *ctrl)     { return ctrl->pid.prev_error; }

/* ── Metrics ───────────────────────────────────────────────────────────── */

/**
 * @brief Reset move metrics. Call before starting a move you want to measure.
 * @param ctrl  Controller instance.
 */
void syn_motor_ctrl_reset_metrics(SYN_MotorCtrl *ctrl);

/**
 * @brief Get accumulated move metrics.
 *
 * Read this after a move completes to evaluate performance.
 * Contains max error, RMS error data, overshoot, peak output,
 * and timing — everything needed for tuning, in a single read.
 *
 * @param ctrl  Controller.
 * @return Pointer to metrics struct.
 */
static inline const SYN_MotorCtrl_Metrics *
syn_motor_ctrl_get_metrics(const SYN_MotorCtrl *ctrl) { return &ctrl->metrics; }

/**
 * @brief Compute RMS tracking error from accumulated metrics.
 *
 * Returns the integer square root of (error_sq_sum / sample_count).
 * Only valid if sample_count > 0.
 *
 * @param ctrl  Controller instance.
 * @return RMS tracking error in feedback units.
 */
int32_t syn_motor_ctrl_rms_error(const SYN_MotorCtrl *ctrl);

/**
 * @brief Get move duration in milliseconds.
 * @param ctrl  Controller.
 * @return Duration in ms, or 0 if no move started.
 */
static inline uint32_t
syn_motor_ctrl_move_duration(const SYN_MotorCtrl *ctrl)
{
    if (ctrl->metrics.move_start_tick == 0) return 0;
    return syn_port_get_tick_ms() - ctrl->metrics.move_start_tick;
}

/**
 * @brief Get settling time in milliseconds (0 if never settled).
 * @param ctrl  Controller.
 * @return Settle time in ms.
 */
static inline uint32_t
syn_motor_ctrl_settle_time(const SYN_MotorCtrl *ctrl)
{
    if (ctrl->metrics.settle_tick == 0 || ctrl->metrics.move_start_tick == 0)
        return 0;
    return ctrl->metrics.settle_tick - ctrl->metrics.move_start_tick;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_MOTOR_CTRL_H */
