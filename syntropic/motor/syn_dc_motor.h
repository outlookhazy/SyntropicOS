/**
 * @file syn_dc_motor.h
 * @brief DC motor controller for H-bridge drivers.
 *
 * Controls direction + speed via two GPIO pins (or one PWM + one dir pin).
 * Supports soft start/stop ramps to limit inrush current and mechanical
 * stress, and integrates with PID for closed-loop speed control.
 *
 * @par Usage
 * @code
 *   static SYN_DCMotor motor;
 *   syn_dc_motor_init(&motor, PWM_PIN, DIR_PIN, IN_B_PIN);
 *   syn_dc_motor_set_speed(&motor, 75);   // 75% forward
 *   syn_dc_motor_set_speed(&motor, -50);  // 50% reverse
 *
 *   // With ramp:
 *   syn_dc_motor_ramp_to(&motor, 100, 500);  // ramp to 100% over 500ms
 *   while (!syn_dc_motor_at_target(&motor)) {
 *       syn_dc_motor_update(&motor);
 *   }
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_DC_MOTOR_H
#define SYN_DC_MOTOR_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"

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

/* ── DC motor descriptor ────────────────────────────────────────────────── */

/** @brief DC motor instance — pins, speed, ramp state. */
typedef struct {
    /* Configuration */
    SYN_GPIO_Pin   pin_a;       /**< PWM pin (or IN_A for dual mode)       */
    SYN_GPIO_Pin   pin_b;       /**< DIR pin (or IN_B for dual mode)       */
    uint8_t         mode;        /**< SYN_DCMotorMode                      */
    bool            invert;      /**< Invert direction                      */

    /* State */
    int16_t         speed;       /**< Current speed (-100 to +100)          */
    int16_t         target;      /**< Target speed for ramping              */
    int32_t         ramp_rate;   /**< Rate of speed change (% per ms, Q8)   */
    uint32_t        last_tick;   /**< Last ramp update tick                 */

    /* Duty output callback — user provides this to set actual PWM duty */
    void          (*set_duty)(SYN_GPIO_Pin pin, uint8_t duty_pct, void *ctx); /**< PWM callback */
    void           *duty_ctx;   /**< Context for set_duty                  */
} SYN_DCMotor;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize DC motor controller.
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
 * The callback is called with the pin and the duty percentage (0–100)
 * whenever the motor speed changes.
 *
 * @param motor  Motor instance.
 * @param cb     Duty callback.
 * @param ctx    User context.
 */
void syn_dc_motor_set_duty_callback(SYN_DCMotor *motor,
                                     void (*cb)(SYN_GPIO_Pin, uint8_t, void *),
                                     void *ctx);

/**
 * @brief Set motor speed immediately.
 *
 * @param motor  DC motor handle.
 * @param speed  Speed as a percentage: -100 (full reverse) to +100 (full forward).
 *               0 = stop (brake).
 */
void syn_dc_motor_set_speed(SYN_DCMotor *motor, int16_t speed);

/**
 * @brief Ramp to a target speed over a duration.
 *
 * @param motor     Motor.
 * @param speed     Target speed (-100 to +100).
 * @param duration  Ramp duration in milliseconds.
 */
void syn_dc_motor_ramp_to(SYN_DCMotor *motor, int16_t speed,
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
 * @return Speed percentage (-100 to +100).
 */
static inline int16_t syn_dc_motor_get_speed(const SYN_DCMotor *motor)
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

#ifdef __cplusplus
}
#endif

#endif /* SYN_DC_MOTOR_H */
