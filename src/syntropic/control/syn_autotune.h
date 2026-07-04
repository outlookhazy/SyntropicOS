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
 *    Runs PROBE → FF identification → braking → relay PID tuning → braking
 *    automatically. The user only provides track limits.
 *
 * 2. **syn_autotune_init()** (advanced) — manual mode selection.
 *    The caller configures and runs each phase (FF ident, relay) separately.
 *
 * Both are non-blocking — call syn_autotune_update() from your main loop.
 *
 * @par Real-World Usage (recommended API)
 *
 * **Step 1: Home the encoder.**
 * Zero the encoder at one end of the track. The auto-tuner does NOT
 * require zeroing in the middle — it works with any zero convention.
 *
 * **Step 2: Configure the motor controller.**
 * Set position_min/max to your safe operating range (in encoder counts).
 * These are the soft limits the auto-tuner will respect.
 * @code
 *   SYN_MotorCtrl_Config cfg = SYN_MOTOR_CTRL_DEFAULTS(
 *       syn_dc_motor_output(&motor), encoder_read, NULL, 1000, 1000
 *   );
 *   cfg.position_min  = 5000;     // 0.5m from home end (10000 cts/m)
 *   cfg.position_max  = 495000;   // 0.5m from far end
 *   syn_motor_ctrl_init(&ctrl, &cfg);
 * @endcode
 *
 * **Step 3: Start auto-tune.**
 * @code
 *   SYN_AutoTune at;
 *   SYN_AutoTune_Limits limits = {
 *       .position_min = 5000,
 *       .position_max = 495000,
 *       .watchdog_ms  = 1000,    // abort if update() stops for 1s
 *   };
 *   syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL);
 * @endcode
 *
 * **Step 4: Poll from your main loop.**
 * @code
 *   while (true) {
 *       SYN_AutoTune_State st = syn_autotune_update(&at);
 *       if (st == SYN_ATUNE_DONE) {
 *           syn_autotune_apply(&at); // Copy gains to controller
 *           break;
 *       }
 *       if (st == SYN_ATUNE_ABORTED) {
 *           // Error handling...
 *           break;
 *       }
 *       syn_port_sleep_ms(1); // or run from 1kHz interrupt
 *   }
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
    SYN_ATUNE_MODE_AUTO     = 2,   /**< Automatic sequence: FF + Relay    */
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
    SYN_ATUNE_IDLE,
    SYN_ATUNE_PROBE,
    SYN_ATUNE_RAMP_UP,
    SYN_ATUNE_SETTLING,
    SYN_ATUNE_MEASURING,
    SYN_ATUNE_SETTLING_2,
    SYN_ATUNE_MEASURING_2,
    SYN_ATUNE_RELAY,
    SYN_ATUNE_BRAKING,
    SYN_ATUNE_RAMP_DOWN,
    SYN_ATUNE_DONE,
    SYN_ATUNE_ABORTED
} SYN_AutoTune_State;

/** @brief Auto-tuner feature flags. */
typedef enum {
    SYN_ATUNE_FLAG_NONE     = 0,
    SYN_ATUNE_FLAG_IDENT_KV = (1 << 0), /**< Identify velocity feedforward (ff_kv) */
    SYN_ATUNE_FLAG_IDENT_KA = (1 << 1), /**< Identify inertia feedforward (ff_ka) */
    SYN_ATUNE_FLAG_TUNE_PID = (1 << 2), /**< Identify PID gains (Kp, Ki, Kd)      */
    
    /** Default: Identify everything */
    SYN_ATUNE_FLAG_ALL = (SYN_ATUNE_FLAG_IDENT_KV | 
                          SYN_ATUNE_FLAG_IDENT_KA | 
                          SYN_ATUNE_FLAG_TUNE_PID)
} SYN_AutoTune_Flags;

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
    SYN_ATUNE_ABORT_NO_MOTION  = 7,  /**< Could not detect motion in PROBE */
} SYN_AutoTune_AbortReason;

/** @brief Telemetry frame ID for Auto-Tune data. */
#define SYN_ATUNE_LOG_ID  0x4154  /* 'AT' */

/** @brief Telemetry frame for Auto-Tune capture. */
typedef struct {
    uint8_t  state;      /**< SYN_AutoTune_State                       */
    int16_t  output;     /**< Applied output percentage                */
    int32_t  position;   /**< Current position                         */
    int32_t  velocity;   /**< Current velocity                         */
} SYN_AutoTune_LogFrame;

/** @brief Physical constraints for the one-call auto-tune. */
typedef struct {
    int32_t  position_min;     /**< Min safe position (encoder counts).    */
    int32_t  position_max;     /**< Max safe position (encoder counts).    */
    int32_t  max_velocity;     /**< Max safe velocity (counts/sec). 0=none.*/
    uint32_t watchdog_ms;      /**< Abort if update() gap exceeds this.    */
} SYN_AutoTune_Limits;

/* ── Configuration ─────────────────────────────────────────────────────── */

/** @brief Auto-tuner configuration. */
typedef struct {
    SYN_AutoTune_Mode   mode;        /**< Operating mode                    */

    /** Motor output during test (% of output_max).
     *  If 0, the tuner will probe for the minimum motion output. */
    int32_t             test_output;

    /* FF identification timing */
    uint32_t            settle_ms;       /**< Time to reach steady-state    */
    uint32_t            measure_ms;      /**< Duration to average velocity  */

    /* Relay feedback config */
    int32_t             setpoint;        /**< Position to oscillate around  */
    uint8_t             relay_cycles;    /**< Oscillation cycles to measure */
    SYN_AutoTune_Method method;          /**< PID gain formula              */

    /* Telemetry (optional) */
    SYN_DataLog        *datalog;         /**< If set, capture tuning telemetry */

    SYN_AutoTune_Flags  flags;           /**< Feature enablement flags      */

    /* ── Safety (all mandatory for heavy machinery) ─────────────── */
    SYN_AutoTune_Limits limits;          /**< Physical constraints          */

    /** Abort if update() not called within this time (ms).
     *  Protects against application crashes. Default: 500 ms. */
    uint32_t            watchdog_ms;

    /** Ramp time — ms to ramp from 0 to test_output. Default: 500 ms.
     *  Prevents jerking heavy loads with a step input. */
    uint32_t            ramp_ms;

    /** Gain multiplier percentage (1-200). Default: 100.
     *  Allows applying a safety margin to calculated PID gains (e.g., 80 for 80%). */
    uint16_t            gain_multiplier_pct;
} SYN_AutoTune_Config;

/* ── Result ────────────────────────────────────────────────────────────── */

/** @brief Auto-tune results (valid when state == DONE). */
typedef struct {
    /* Feedforward identification results */
    int32_t  ff_kv;           /**< Computed velocity feedforward gain      */
    int32_t  ff_ka;           /**< Computed inertia feedforward gain       */
    uint8_t  ff_scale;        /**< Feedforward scale (same as ctrl cfg)    */
    int32_t  steady_velocity_1; /**< Measured velocity at point 1            */
    int32_t  steady_velocity_2; /**< Measured velocity at point 2            */

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
    uint32_t              phase_start_tick;/**< Tick count when current phase started */
    uint32_t              last_update_tick;/**< Tick count of last update() call      */
    int32_t               current_output;  /**< Current applied output percentage     */
    int32_t               start_output;    /**< Output at start of ramp-down          */

    /* Steady-state monitoring */
    int32_t               history_v;       /**< Velocity sample from previous check   */
    uint32_t              last_check_tick; /**< Tick of last steady-state check       */

    /* FF ident internals */
    int64_t               velocity_sum;   /**< Accumulated velocity sum    */
    uint32_t              velocity_samples;/**< Number of velocity samples */
    int32_t               ka_v1;           /**< Initial velocity for Ka identification */
    int32_t               ka_v2;           /**< Final velocity for Ka identification   */
    uint32_t              ka_t1;           /**< Initial tick for Ka identification     */
    uint32_t              ka_t2;           /**< Final tick for Ka identification       */
    bool                  ka_p1_captured; /**< Point 1 captured            */
    bool                  ka_p2_captured; /**< Point 2 captured            */

    /* Oscillation tracking */
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

/* ── One-call auto-tune API ────────────────────────────────────────────── */

/**
 * @brief Start a fully automatic tune sequence.
 *
 * Runs probe → FF identification → braking → relay PID tune → braking
 * as a single self-sequencing state machine. The user only provides
 * physical constraints.
 *
 * @param at               Auto-tuner instance.
 * @param ctrl             Motor controller to tune.
 * @param limits           Physical constraints (track limits, max velocity).
 * @param flags            Feature flags (e.g., SYN_ATUNE_FLAG_ALL).
 * @param gain_multiplier  Safety margin for PID gains (percentage, e.g., 80).
 * @return SYN_OK on success, or error code.
 */
SYN_Status syn_autotune_start(SYN_AutoTune *at, SYN_MotorCtrl *ctrl,
                               const SYN_AutoTune_Limits *limits,
                               SYN_AutoTune_Flags flags,
                               uint16_t gain_multiplier);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUTOTUNE_H */
