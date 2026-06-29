/**
 * @file syn_actuator.h
 * @brief Linear actuator with potentiometer feedback.
 *
 * Wraps motor_ctrl + sensor into a turnkey position-controlled linear
 * actuator. The potentiometer reading is mapped to a percentage position
 * (0.0% – 100.0%, stored as 0–1000).
 *
 * Usage:
 * @code
 *   static SYN_Actuator act;
 *   static SYN_DCMotor motor;
 *   static SYN_ADC pot_adc;
 *
 *   SYN_Actuator_Config cfg = {
 *       .dc_motor    = &motor,
 *       .read_pos    = my_pot_read,     // returns ADC value
 *       .read_ctx    = &pot_adc,
 *       .stroke_min  = 100,             // ADC at fully retracted
 *       .stroke_max  = 3900,            // ADC at fully extended
 *       .update_hz   = 50,
 *   };
 *   syn_actuator_init(&act, &cfg);
 *   syn_actuator_set_position(&act, 500);  // move to 50.0%
 *
 *   // In your loop:
 *   syn_actuator_update(&act);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_ACTUATOR_H
#define SYN_ACTUATOR_H

#include "../common/syn_defs.h"
#include "../motor/syn_motor_ctrl.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief Actuator configuration. */
typedef struct {
    /* Motor */
    SYN_DCMotor    *dc_motor;       /**< DC motor for actuation          */

    /* Position feedback */
    SYN_MotorCtrl_ReadPos read_pos;  /**< Read potentiometer (returns int32_t) */
    void            *read_ctx;        /**< Context for read_pos            */
    int32_t          stroke_min;      /**< ADC value at fully retracted    */
    int32_t          stroke_max;      /**< ADC value at fully extended     */

    /* Control */
    uint16_t         update_hz;       /**< Control loop frequency          */
    int32_t          pid_kp;          /**< PID proportional gain           */
    int32_t          pid_ki;          /**< PID integral gain               */
    int32_t          pid_kd;          /**< PID derivative gain             */
    uint8_t          pid_scale;       /**< Gain divisor = 1 << pid_scale   */
    int32_t          deadband;        /**< Position deadband (ADC units)   */

    /* Safety */
    uint16_t         stall_timeout_ms; /**< 0 = disabled                  */
    int32_t          stall_threshold;  /**< Min motion for "not stalled"  */
    SYN_ErrLog     *errlog;          /**< Optional error logging          */
} SYN_Actuator_Config;

/* ── Actuator instance ──────────────────────────────────────────────────── */

/** @brief Linear actuator instance — motor controller + stroke mapping. */
typedef struct {
    SYN_MotorCtrl   ctrl;            /**< Underlying motor controller     */
    int32_t          stroke_min;      /**< ADC at retracted                */
    int32_t          stroke_max;      /**< ADC at extended                 */
    int32_t          stroke_range;    /**< max - min (precomputed)         */
    int16_t          target_pct;      /**< Target position 0-1000          */
    int16_t          current_pct;     /**< Current position 0-1000         */
} SYN_Actuator;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize linear actuator.
 * @param act  Actuator instance.
 * @param cfg  Configuration.
 */
void syn_actuator_init(SYN_Actuator *act, const SYN_Actuator_Config *cfg);

/**
 * @brief Set desired position.
 *
 * @param act      Actuator instance.
 * @param pct_x10  Position in 0.1% units (0 = retracted, 1000 = extended).
 */
void syn_actuator_set_position(SYN_Actuator *act, int16_t pct_x10);

/**
 * @brief Update — read pot, run PID, drive motor.
 *
 * Call from your scheduler task at the configured update_hz.
 *
 * @param act  Actuator instance.
 * @return Current position (0-1000).
 */
int16_t syn_actuator_update(SYN_Actuator *act);

/**
 * @brief Stop the actuator immediately.
 * @param act  Actuator instance.
 */
void syn_actuator_stop(SYN_Actuator *act);

/**
 * @brief Get current position (0-1000, i.e. 0.0-100.0%).
 * @param act  Actuator.
 * @return Position in 0.1% units.
 */
static inline int16_t syn_actuator_position(const SYN_Actuator *act)
{
    return act->current_pct;
}

/**
 * @brief Check if actuator has reached its target.
 * @param act  Actuator.
 * @return true if within ±0.5% of target.
 */
static inline bool syn_actuator_at_target(const SYN_Actuator *act)
{
    int16_t diff = act->target_pct - act->current_pct;
    if (diff < 0) diff = (int16_t)(-diff);
    return diff <= 5;  /* ±0.5% tolerance */
}

/**
 * @brief Check if stalled.
 * @param act  Actuator.
 * @return true if motor is stalled.
 */
static inline bool syn_actuator_is_stalled(const SYN_Actuator *act)
{
    return act->ctrl.state == SYN_MCTRL_STALLED;
}

/**
 * @brief Clear stall condition and re-enable.
 * @param act  Actuator.
 */
void syn_actuator_clear_stall(SYN_Actuator *act);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ACTUATOR_H */
