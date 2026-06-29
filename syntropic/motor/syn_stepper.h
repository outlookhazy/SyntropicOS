/**
 * @file syn_stepper.h
 * @brief Stepper motor driver — step/direction with acceleration ramps.
 *
 * Generates step pulses for step/dir drivers (A4988, DRV8825, TMC2209,
 * etc.). Supports trapezoidal acceleration profiles and non-blocking
 * operation.
 *
 * @par Usage
 * @code
 *   static SYN_Stepper stepper;
 *   syn_stepper_init(&stepper, STEP_PIN, DIR_PIN);
 *   syn_stepper_set_speed(&stepper, 1000, 500);   // max 1000 sps, accel 500 sps²
 *   syn_stepper_move(&stepper, 2000);               // move 2000 steps forward
 *
 *   // Call periodically (from timer ISR or fast loop):
 *   syn_stepper_tick(&stepper);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_STEPPER_H
#define SYN_STEPPER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Stepper state ──────────────────────────────────────────────────────── */

/**
 * @brief Stepper motor operational states for the ramp generator.
 */
typedef enum {
    SYN_STEPPER_IDLE    = 0,         /**< Motor is stopped */
    SYN_STEPPER_ACCEL   = 1,         /**< Motor is accelerating */
    SYN_STEPPER_CRUISE  = 2,         /**< Motor is running at maximum velocity */
    SYN_STEPPER_DECEL   = 3,         /**< Motor is decelerating to stop */
} SYN_StepperState;

/** @brief Stepper motor direction. */
typedef enum {
    SYN_STEPPER_CW  = 0,  /**< Clockwise / forward   */
    SYN_STEPPER_CCW = 1,  /**< Counter-clockwise / reverse */
} SYN_StepperDir;

/* ── Stepper descriptor ─────────────────────────────────────────────────── */

/**
 * @brief Stepper motor controller context.
 */
typedef struct {
    /* Configuration */
    SYN_GPIO_Pin  step_pin;        /**< Step signal GPIO pin */
    SYN_GPIO_Pin  dir_pin;         /**< Direction control GPIO pin */
    SYN_GPIO_Pin  enable_pin;     /**< Optional enable pin (set to -1 if unused) */
    bool           dir_invert;     /**< Invert direction logic                     */
    bool           enable_invert;  /**< Enable active-low                          */

    /* Motion parameters */
    uint32_t       max_speed;      /**< Maximum speed in steps/sec                 */
    uint32_t       accel;          /**< Acceleration in steps/sec²                 */

    /* Internal state */
    uint8_t        state;          /**< SYN_StepperState                          */
    int32_t        position;       /**< Current position (steps, signed)           */
    int32_t        target;         /**< Target position                            */
    uint32_t       speed;          /**< Current speed (steps/sec, fixed-point Q16) */
    uint32_t       step_interval;  /**< Current step interval (µs)                 */
    uint32_t       last_step_tick; /**< Tick (µs) of last step                     */
    int32_t        steps_to_go;    /**< Remaining steps in current move            */
    int32_t        decel_start;    /**< Step count at which to start decelerating  */
    bool           step_state;     /**< Current step pin level (for pulse gen)     */
} SYN_Stepper;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize stepper motor driver.
 *
 * @param s         Stepper instance.
 * @param step_pin  Step pulse GPIO.
 * @param dir_pin   Direction GPIO.
 */
void syn_stepper_init(SYN_Stepper *s,
                       SYN_GPIO_Pin step_pin,
                       SYN_GPIO_Pin dir_pin);

/**
 * @brief Set optional enable pin.
 *
 * @param s           Stepper instance.
 * @param pin         Enable signal GPIO pin.
 * @param active_low  True if enabling requires driving pin LOW.
 */
void syn_stepper_set_enable_pin(SYN_Stepper *s, SYN_GPIO_Pin pin,
                                 bool active_low);

/**
 * @brief Set maximum speed and acceleration.
 *
 * @param s          Stepper.
 * @param max_sps    Maximum speed in steps per second.
 * @param accel_sps2 Acceleration in steps per second².
 */
void syn_stepper_set_speed(SYN_Stepper *s, uint32_t max_sps,
                            uint32_t accel_sps2);

/**
 * @brief Start a relative move.
 *
 * @param s      Stepper handle.
 * @param steps  Number of steps (positive = forward, negative = reverse).
 */
void syn_stepper_move(SYN_Stepper *s, int32_t steps);

/**
 * @brief Start a move to an absolute position.
 * @param s         Stepper.
 * @param position  Target absolute position in steps.
 */
void syn_stepper_move_to(SYN_Stepper *s, int32_t position);

/**
 * @brief Advance the stepper state machine by one tick.
 *
 * Call from a timer ISR or high-frequency loop. The tick rate should
 * be at least 2× the maximum step rate for proper pulse generation.
 *
 * @param s  Stepper instance.
 */
void syn_stepper_tick(SYN_Stepper *s);

/**
 * @brief Emergency stop — immediately halt with no deceleration.
 * @param s  Stepper instance.
 */
void syn_stepper_stop(SYN_Stepper *s);

/**
 * @brief Enable or disable the motor driver.
 * @param s       Stepper instance.
 * @param enable  true to enable, false to disable.
 */
void syn_stepper_enable(const SYN_Stepper *s, bool enable);

/**
 * @brief Is a move in progress?
 * @param s  Stepper.
 * @return true if moving.
 */
static inline bool syn_stepper_is_moving(const SYN_Stepper *s)
{
    return s->state != (uint8_t)SYN_STEPPER_IDLE;
}

/**
 * @brief Get current position (steps).
 * @param s  Stepper.
 * @return Position in steps.
 */
static inline int32_t syn_stepper_position(const SYN_Stepper *s)
{
    return s->position;
}

/**
 * @brief Set current position without moving.
 * @param s    Stepper.
 * @param pos  New position.
 */
static inline void syn_stepper_set_position(SYN_Stepper *s, int32_t pos)
{
    s->position = pos;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_STEPPER_H */
