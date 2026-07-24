/**
 * @file syn_dc_motor.h
 * @brief DC motor controller for H-bridge drivers.
 *
 * Controls direction + speed via two GPIO pins (or one PWM + one dir pin).
 * Supports soft start/stop ramps to limit inrush current and mechanical
 * stress. Provides a SYN_MotorOutput interface for use with syn_motor_ctrl.
 *
 * @par Usage
 * @code
 *   static SYN_DCMotor motor;
 *   syn_dc_motor_init(&motor, PWM_PIN, DIR_PIN, SYN_DC_MODE_PWM_DIR);
 *   syn_dc_motor_set_speed(&motor, 750);   // 75% of duty_max (default 1000)
 *   syn_dc_motor_set_speed(&motor, -500);  // 50% reverse
 *
 *   // With ramp:
 *   syn_dc_motor_ramp_to(&motor, 1000, 500);  // ramp to full over 500ms
 *   while (!syn_dc_motor_at_target(&motor)) {
 *       syn_dc_motor_update(&motor);
 *   }
 *
 *   // With motor controller:
 *   cfg.motor = syn_dc_motor_output(&motor);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_DC_MOTOR_H
#define SYN_DC_MOTOR_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../motor/syn_motor_output.h"
#include "../port/syn_port_system.h"
#include "../util/syn_ramp.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DC motor control mode ──────────────────────────────────────────────── */

/** @brief DC motor control wiring mode. */
typedef enum {
    /**< One PWM pin + one direction pin (most common) */
    SYN_DC_MODE_PWM_DIR    = 0,
    /**< Two pins: IN_A (PWM forward), IN_B (PWM reverse) like L298N */
    SYN_DC_MODE_DUAL_PWM   = 1,
} SYN_DCMotorMode;

/** @brief Default duty cycle range (0.1% resolution). */
#define SYN_DC_MOTOR_DUTY_MAX_DEFAULT  1000

/* ── DC motor descriptor ────────────────────────────────────────────────── */

/** @brief DC motor instance — pins, speed, ramp state. */
typedef struct {
    /* Configuration */
    SYN_GPIO_Pin   pin_a;       /**< PWM pin (or IN_A for dual mode)       */
    SYN_GPIO_Pin   pin_b;       /**< DIR pin (or IN_B for dual mode)       */
    uint8_t         mode;        /**< SYN_DCMotorMode                      */
    bool            invert;      /**< Invert direction                      */

    /* State */
    int32_t         speed;       /**< Current speed (-duty_max to +duty_max) */
    int32_t         target;      /**< Target speed for ramping              */
    int32_t         ramp_rate;   /**< Rate of speed change (per ms, Q8)     */
    uint32_t        last_tick;   /**< Last ramp update tick                 */
    SYN_Ramp        ramp;        /**< Internal velocity ramp generator      */

    /** Maximum duty cycle value. Output range is [-duty_max, +duty_max].
     *  Set to match your PWM timer resolution (e.g., 255 for 8-bit,
     *  1000 for 0.1%, 4095 for 12-bit, 65535 for 16-bit).
     *  Default: SYN_DC_MOTOR_DUTY_MAX_DEFAULT (1000). */
    int32_t         duty_max;

    /** Duty output callback — user provides this to set actual PWM duty.
     *  @param pin   GPIO pin to set duty on.
     *  @param duty  Duty cycle value in range [0, duty_max].
     *  @param ctx   User context. */
    void          (*set_duty)(SYN_GPIO_Pin pin, uint16_t duty, void *ctx);
    void           *duty_ctx;   /**< Context for set_duty                  */
} SYN_DCMotor;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize DC motor controller.
 *
 * Sets duty_max to SYN_DC_MOTOR_DUTY_MAX_DEFAULT (1000).
 *
 * @param motor   Motor instance.
 * @param pin_a   PWM pin (or IN_A).
 * @param pin_b   Direction pin (or IN_B).
 * @param mode    Control mode.
 */
void syn_dc_motor_init(SYN_DCMotor *motor, SYN_GPIO_Pin pin_a,
                        SYN_GPIO_Pin pin_b, SYN_DCMotorMode mode);

/**
 * @brief Set the PWM duty callback.
 *
 * The callback is called with the pin and a duty value in [0, duty_max]
 * whenever the motor speed changes.
 *
 * @param motor  Motor instance.
 * @param cb     Duty callback.
 * @param ctx    User context.
 */
void syn_dc_motor_set_duty_callback(SYN_DCMotor *motor,
                                     void (*cb)(SYN_GPIO_Pin, uint16_t, void *),
                                     void *ctx);

/**
 * @brief Set motor speed immediately.
 *
 * @param motor  DC motor handle.
 * @param speed  Speed in range [-duty_max, +duty_max]. Positive = forward,
 *               negative = reverse, 0 = stop. Clamped to ±duty_max.
 */
void syn_dc_motor_set_speed(SYN_DCMotor *motor, int32_t speed);

/**
 * @brief Ramp to a target speed over a duration.
 *
 * @param motor     Motor.
 * @param speed     Target speed [-duty_max, +duty_max].
 * @param duration  Ramp duration in milliseconds.
 */
void syn_dc_motor_ramp_to(SYN_DCMotor *motor, int32_t speed,
                           uint16_t duration);

/**
 * @brief Update motor ramp. Call periodically.
 * @param motor  Motor instance.
 */
void syn_dc_motor_update(SYN_DCMotor *motor);

/**
 * @brief Stop the motor (coast — both pins low).
 * @param motor  Motor instance.
 */
void syn_dc_motor_coast(SYN_DCMotor *motor);

/**
 * @brief Brake the motor (both pins high, if driver supports it).
 * @param motor  Motor instance.
 */
void syn_dc_motor_brake(SYN_DCMotor *motor);

/**
 * @brief Get current speed.
 * @param motor  Motor instance.
 * @return Speed in range [-duty_max, +duty_max].
 */
static inline int32_t syn_dc_motor_get_speed(const SYN_DCMotor *motor)
{
    return motor->speed;
}

/**
 * @brief Is the ramp complete?
 * @param motor  Motor instance.
 * @return true if at target speed.
 */
static inline bool syn_dc_motor_at_target(const SYN_DCMotor *motor)
{
    return motor->speed == motor->target;
}

/**
 * @brief Set the maximum duty cycle value.
 *
 * Call after init to change from the default (1000). Resets speed to 0.
 *
 * @param motor     Motor instance.
 * @param duty_max  Maximum duty value (e.g., 255, 1000, 4095, 65535).
 */
void syn_dc_motor_set_duty_max(SYN_DCMotor *motor, int32_t duty_max);

/**
 * @brief Create a SYN_MotorOutput interface for this DC motor.
 *
 * Returns a motor output vtable that maps set_output() to
 * syn_dc_motor_set_speed(), coast() to syn_dc_motor_coast(), etc.
 * The output range maps directly to [-duty_max, +duty_max].
 *
 * @param motor  DC motor instance (must outlive the returned output).
 * @return Motor output interface.
 */
SYN_MotorOutput syn_dc_motor_output(SYN_DCMotor *motor);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DC_MOTOR_H */
