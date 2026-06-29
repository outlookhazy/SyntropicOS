/**
 * @file syn_autotune.h
 * @brief Motor controller auto-tuner — feedforward ID + relay PID tuning.
 *
 * Safety-first design for heavy machinery:
 * - Hard position limit — abort if displacement exceeds configured bound
 * - Velocity limit — cut power if speed exceeds safe threshold
 * - Motor controller soft limits are respected (checked every update)
 * - Watchdog timeout — auto-abort if update() stops being called
 * - Gradual ramp-up (not a step) to avoid jerking heavy loads
 *
 * Two layers of API:
 *
 * 1. **syn_autotune_start()** (recommended) — one-call auto-tune.
 *    Runs FF identification → braking → relay PID tuning → braking
 *    automatically. The user only provides track limits.
 *    @note Not yet implemented — use the manual API below.
 *
 * 2. **syn_autotune_init()** (advanced) — manual mode selection.
 *    The caller configures and runs each phase (FF ident, relay) separately.
 *
 * Both are non-blocking — call syn_autotune_update() from your main loop.
 *
 * @par Real-World Usage (recommended API — future)
 *
 * **Step 1: Home the encoder.**
 * Zero the encoder at one end of the track. The auto-tuner does NOT
 * require zeroing in the middle — it works with any zero convention.
 * @code
 *   EncoderIn.Position(0);   // or equivalent
 * @endcode
 *
 * **Step 2: Configure the motor controller.**
 * Set position_min/max to your safe operating range (in encoder counts).
 * These are the soft limits the auto-tuner will respect.
 * @code
 *   SYN_MotorCtrl_Config cfg = {
 *       .type          = SYN_MCTRL_DC,
 *       .dc_motor      = &motor,
 *       .read_pos      = encoder_read,
 *       .update_hz     = 100,
 *       .output_min    = -100,
 *       .output_max    = 100,
 *       .position_min  = 5000,     // 0.5m from home end (10000 cts/m)
 *       .position_max  = 495000,   // 0.5m from far end
 *   };
 *   syn_motor_ctrl_init(&ctrl, &cfg);
 * @endcode
 *
 * **Step 3: Jog the mover to somewhere with room.**
 * The auto-tuner needs space to move in both directions. Anywhere more
 * than ~2m from either end is fine. Mid-track is ideal.
 *
 * **Step 4: Start auto-tune.**
 * @code
 *   SYN_AutoTune at;
 *   SYN_AutoTune_Limits limits = {
 *       .position_min = 5000,
 *       .position_max = 495000,
 *   };
 *   syn_autotune_start(&at, &ctrl, &limits);
 * @endcode
 *
 * **Step 5: Poll from your main loop.**
 * @code
 *   SYN_AutoTune_State st = syn_autotune_update(&at);
 *   if (st == SYN_ATUNE_DONE) {
 *       // Gains are applied to the controller. Save to flash if desired.
 *   }
 *   if (st == SYN_ATUNE_ABORTED) {
 *       // Check syn_autotune_abort_reason(&at) for what went wrong.
 *   }
 * @endcode
 *
 * **Step 6: Resume normal operation.**
 * The mover is stopped with tuned gains applied. The controller is in
 * IDLE mode — set your next target with set_position/set_velocity.
 *
 * @par Manual API Usage (available now)
 * @code
 *   // Phase 1: FF identification
 *   SYN_AutoTune at;
 *   SYN_AutoTune_Config cfg = {
 *       .mode             = SYN_ATUNE_MODE_FF_IDENT,
 *       .test_output      = 30,
 *       .position_limit   = 200000,
 *       .watchdog_ms      = 500,
 *   };
 *   syn_autotune_init(&at, &ctrl, &cfg);
 *   // poll update() until DONE, then brake to stop
 *
 *   // Phase 2: Relay PID tuning
 *   cfg = (SYN_AutoTune_Config){
 *       .mode             = SYN_ATUNE_MODE_RELAY,
 *       .test_output      = 25,
 *       .setpoint         = ctrl.measured_position,
 *       .method           = SYN_ATUNE_TYREUS_LUYBEN,
 *       .position_limit   = 50000,
 *       .watchdog_ms      = 500,
 *   };
 *   syn_autotune_init(&at, &ctrl, &cfg);
 *   // poll update() until DONE
 *   syn_autotune_apply(&at);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_AUTOTUNE_H
#define SYN_AUTOTUNE_H

#include "../common/syn_defs.h"
#include "../motor/syn_motor_ctrl.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Mode ──────────────────────────────────────────────────────────────── */

/** @brief Auto-tune operating mode. */
typedef enum {
    SYN_ATUNE_MODE_FF_IDENT = 0,   /**< Feedforward identification only   */
    SYN_ATUNE_MODE_RELAY    = 1,   /**< Relay feedback PID tuning         */
} SYN_AutoTune_Mode;

/* ── PID formula ───────────────────────────────────────────────────────── */

/** @brief PID gain formula selection for relay auto-tune. */
typedef enum {
    SYN_ATUNE_ZN_CLASSIC       = 0,  /**< Ziegler-Nichols classic          */
    SYN_ATUNE_ZN_NO_OVERSHOOT  = 1,  /**< Ziegler-Nichols (some overshoot) */
    SYN_ATUNE_TYREUS_LUYBEN    = 2,  /**< Tyreus-Luyben (conservative)     */
} SYN_AutoTune_Method;

/* ── State ─────────────────────────────────────────────────────────────── */

/** @brief Auto-tuner state machine states. */
typedef enum {
    SYN_ATUNE_IDLE     = 0,   /**< Not started or finished               */
    SYN_ATUNE_RAMP_UP  = 1,   /**< Ramping output to test level           */
    SYN_ATUNE_SETTLING = 2,   /**< FF ident: waiting for steady-state     */
    SYN_ATUNE_MEASURING = 3,  /**< FF ident: measuring velocity           */
    SYN_ATUNE_RELAY    = 4,   /**< Relay: oscillation in progress         */
    SYN_ATUNE_RAMP_DOWN = 5,  /**< Ramping output back to zero            */
    SYN_ATUNE_DONE     = 6,   /**< Tuning complete, results available     */
    SYN_ATUNE_ABORTED  = 7,   /**< Safety limit hit or user abort         */
} SYN_AutoTune_State;

/* ── Abort reason ──────────────────────────────────────────────────────── */

/** @brief Reason for auto-tune abort. */
typedef enum {
    SYN_ATUNE_OK               = 0,  /**< No abort — tuning succeeded      */
    SYN_ATUNE_ABORT_POSITION   = 1,  /**< Position limit exceeded          */
    SYN_ATUNE_ABORT_VELOCITY   = 2,  /**< Velocity limit exceeded          */
    SYN_ATUNE_ABORT_SOFT_LIMIT = 3,  /**< Motor controller soft limit hit  */
    SYN_ATUNE_ABORT_WATCHDOG   = 4,  /**< update() not called in time      */
    SYN_ATUNE_ABORT_USER       = 5,  /**< User called abort()              */
    SYN_ATUNE_ABORT_STALL      = 6,  /**< Motor not responding to output   */
} SYN_AutoTune_AbortReason;

/* ── Configuration ─────────────────────────────────────────────────────── */

/** @brief Auto-tuner configuration. */
typedef struct {
    SYN_AutoTune_Mode   mode;        /**< Operating mode (FF or relay)      */

    /** Motor output during test (% of output_max, always positive).
     *  For a 300 lb mover, 15-25 is a good starting range. */
    int32_t             test_output;  /**< % of output during test          */

    /* FF identification timing */
    uint32_t            settle_ms;       /**< Time to reach steady-state    */
    uint32_t            measure_ms;      /**< Duration to average velocity  */

    /* Relay feedback config */
    int32_t             setpoint;        /**< Position to oscillate around  */
    uint8_t             relay_cycles;    /**< Oscillation cycles to measure */
    SYN_AutoTune_Method method;          /**< PID gain formula              */

    /* ── Safety (all mandatory for heavy machinery) ─────────────── */

    /** Abort if displacement from start exceeds this (units).
     *  MUST be set — there is no default. */
    int32_t             position_limit;

    /** Abort if velocity exceeds this (units/sec). 0 = no limit. */
    int32_t             velocity_limit;

    /** Abort if update() not called within this time (ms).
     *  Protects against application crashes. Default: 200 ms. */
    uint32_t            watchdog_ms;

    /** Ramp time — ms to ramp from 0 to test_output. Default: 500 ms.
     *  Prevents jerking heavy loads with a step input. */
    uint32_t            ramp_ms;
} SYN_AutoTune_Config;

/* ── Result ────────────────────────────────────────────────────────────── */

/** @brief Auto-tune results (valid when state == DONE). */
typedef struct {
    /* Feedforward identification results */
    int32_t  ff_kv;           /**< Computed velocity feedforward gain      */
    uint8_t  ff_scale;        /**< Feedforward scale (same as ctrl cfg)    */
    int32_t  steady_velocity; /**< Measured steady-state velocity          */

    /* Relay feedback results */
    int32_t  Ku;              /**< Ultimate gain (scaled by pid_scale)     */
    uint32_t Tu_ms;           /**< Ultimate period (ms)                    */
    int32_t  kp;              /**< Computed proportional gain              */
    int32_t  ki;              /**< Computed integral gain                  */
    int32_t  kd;              /**< Computed derivative gain                */
    uint8_t  pid_scale;       /**< PID gain scale                         */
} SYN_AutoTune_Result;

/* ── Auto-tuner instance ───────────────────────────────────────────────── */

/** @brief Auto-tuner instance. */
typedef struct {
    SYN_AutoTune_Config   cfg;            /**< Configuration snapshot       */
    SYN_AutoTune_Result   result;         /**< Computed results             */
    SYN_AutoTune_State    state;          /**< Current state machine state  */
    SYN_AutoTune_AbortReason abort_reason;/**< Reason if aborted            */

    SYN_MotorCtrl        *ctrl;           /**< Controlled motor            */

    /* Internal state */
    int32_t               start_position; /**< Position when tuning started */
    uint32_t              phase_start_tick;/**< Tick at phase start         */
    uint32_t              last_update_tick;/**< For watchdog                */
    int32_t               current_output; /**< Current applied output      */

    /* FF ident internals */
    int64_t               velocity_sum;   /**< Accumulated velocity sum    */
    uint32_t              velocity_samples;/**< Number of velocity samples */

    /* Relay internals */
    int32_t               relay_output;    /**< Current relay sign × amp   */
    uint8_t               half_cycles;     /**< Half-cycle counter         */
    uint32_t              last_cross_tick;  /**< Last zero-crossing tick    */
    uint32_t              period_sum;       /**< Sum of full periods (ms)   */
    uint8_t               period_count;    /**< Number of full periods     */
    int32_t               osc_peak_pos;    /**< Max position in half-cycle */
    int32_t               osc_peak_neg;    /**< Min position in half-cycle */
    int32_t               amplitude_sum;   /**< Sum of oscillation amps    */
    uint8_t               amplitude_count; /**< Number of amplitude samples*/
    bool                  above_setpoint;  /**< Current side of setpoint   */
} SYN_AutoTune;

/* ── API ───────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the auto-tuner.
 *
 * @param at    Auto-tuner instance.
 * @param ctrl  Motor controller to tune (tuner takes control while active).
 * @param cfg   Configuration. position_limit MUST be set.
 * @return SYN_OK, or SYN_ERROR if position_limit is 0.
 */
SYN_Status syn_autotune_init(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                              const SYN_AutoTune_Config *cfg);

/**
 * @brief Update the auto-tuner — call from main loop.
 *
 * @param at   Auto-tuner instance.
 * @return Current state. When DONE, results are in syn_autotune_result().
 */
SYN_AutoTune_State syn_autotune_update(SYN_AutoTune *at);

/**
 * @brief Get the tuning results (valid when state == DONE).
 * @param at  Autotuner instance.
 * @return Pointer to results struct.
 */
static inline const SYN_AutoTune_Result *
syn_autotune_result(const SYN_AutoTune *at) { return &at->result; }

/**
 * @brief Get the reason for abort (valid when state == ABORTED).
 * @param at  Autotuner instance.
 * @return Abort reason code.
 */
static inline SYN_AutoTune_AbortReason
syn_autotune_abort_reason(const SYN_AutoTune *at) { return at->abort_reason; }

/**
 * @brief Apply computed gains to the motor controller.
 *
 * Copies ff_kv, ff_scale, and PID gains into the motor controller.
 * Only valid when state == DONE.
 *
 * @param at   Auto-tuner instance.
 */
void syn_autotune_apply(SYN_AutoTune *at);

/**
 * @brief Abort the auto-tune and stop the motor immediately.
 * @param at   Auto-tuner instance.
 */
void syn_autotune_abort(SYN_AutoTune *at);

/* ── One-call auto-tune API (future) ───────────────────────────────────── */

/**
 * @brief Physical constraints for the one-call auto-tune.
 *
 * The auto-tuner uses these limits to decide how far it can move,
 * how fast it can go, and when to abort. Positions are in encoder
 * counts using the same coordinate system as the motor controller.
 *
 * You do NOT need to zero the encoder in the middle of the track.
 * Zero it wherever you home (typically one end), then set position_min
 * and position_max relative to that home position.
 *
 * Example: 50m track, 10000 counts/m, homed at left end:
 * @code
 *   .position_min = 5000,     // 0.5m safety margin from home end
 *   .position_max = 495000,   // 0.5m safety margin from far end
 * @endcode
 */
typedef struct {
    int32_t  position_min;     /**< Min safe position (encoder counts).    */
    int32_t  position_max;     /**< Max safe position (encoder counts).    */
    int32_t  max_velocity;     /**< Max safe velocity (counts/sec). 0=none.*/
    uint32_t watchdog_ms;      /**< Abort if update() gap exceeds this.
                                    0 = default (500 ms).                  */
} SYN_AutoTune_Limits;

/**
 * @brief Start a fully automatic tune sequence (NOT YET IMPLEMENTED).
 *
 * Runs probe → FF identification → braking → relay PID tune → braking
 * as a single self-sequencing state machine. The user only provides
 * physical constraints.
 *
 * @param at      Auto-tuner instance.
 * @param ctrl    Motor controller (must be stopped).
 * @param limits  Physical constraints (track limits, max velocity).
 * @return SYN_ERROR — not yet implemented. Use syn_autotune_init() for now.
 *
 * @note This API is defined but not yet implemented. It will be built
 *       when real firmware integration provides the context needed to
 *       make the right design decisions (triggering, e-stop interaction,
 *       gain persistence). Use the manual syn_autotune_init() API for now.
 */
SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Limits *limits);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUTOTUNE_H */
