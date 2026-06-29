/**
 * @file syn_servo.h
 * @brief Hobby servo controller — pulse-width positioning.
 *
 * Standard hobby servos expect a 50 Hz PWM signal with a pulse width
 * of 1000–2000 µs (center = 1500 µs). This module provides angle-based
 * and microsecond-based positioning with optional smooth movement.
 *
 * Requires a hardware timer or syn_soft_pwm for pulse generation.
 * The module computes pulse width; you connect it to your PWM output.
 *
 * @par Usage
 * @code
 *   SYN_Servo servo;
 *   syn_servo_init(&servo, 1000, 2000, 180); // 1000–2000µs, 180° range
 *
 *   syn_servo_set_angle(&servo, 90);   // center
 *   uint16_t pw = syn_servo_get_pulse_us(&servo);  // → 1500µs
 *
 *   // Smooth move:
 *   syn_servo_move_to(&servo, 0, 500);  // move to 0° over 500ms
 *   while (!syn_servo_at_target(&servo)) {
 *       syn_servo_update(&servo);       // call periodically
 *       set_pwm_us(syn_servo_get_pulse_us(&servo));
 *   }
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_SERVO_H
#define SYN_SERVO_H

#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Servo descriptor ───────────────────────────────────────────────────── */

/** @brief Hobby servo instance — pulse range, position, smooth movement. */
typedef struct {
    /* Configuration */
    uint16_t  pulse_min;    /**< Minimum pulse width in µs (e.g., 1000)  */
    uint16_t  pulse_max;    /**< Maximum pulse width in µs (e.g., 2000)  */
    uint16_t  angle_range;  /**< Full range in degrees (e.g., 180)       */

    /* State */
    uint16_t  current_us;   /**< Current pulse width (µs)                */
    uint16_t  target_us;    /**< Target pulse width (µs)                 */
    int32_t   rate;         /**< Movement rate (µs per ms), 0 = instant  */
    uint32_t  last_tick;    /**< Last update tick                        */
} SYN_Servo;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a servo.
 *
 * @param servo       Servo instance.
 * @param pulse_min   Minimum pulse width in µs (0° position).
 * @param pulse_max   Maximum pulse width in µs (max angle position).
 * @param angle_range Full angular range in degrees.
 */
void syn_servo_init(SYN_Servo *servo, uint16_t pulse_min,
                     uint16_t pulse_max, uint16_t angle_range);

/**
 * @brief Set servo position by angle (immediate).
 *
 * @param servo  Servo handle.
 * @param angle  Angle in degrees (0 to angle_range).
 */
void syn_servo_set_angle(SYN_Servo *servo, uint16_t angle);

/**
 * @brief Set servo position by raw pulse width (immediate).
 *
 * @param servo  Servo handle.
 * @param us     Pulse width in microseconds.
 */
void syn_servo_set_pulse(SYN_Servo *servo, uint16_t us);

/**
 * @brief Start a smooth move to an angle.
 *
 * @param servo      Servo.
 * @param angle      Target angle.
 * @param duration   Movement duration in milliseconds.
 */
void syn_servo_move_to(SYN_Servo *servo, uint16_t angle, uint16_t duration);

/**
 * @brief Update servo position for smooth movement.
 *
 * Call periodically (e.g., every 10–20ms) while a smooth move is active.
 *
 * @param servo  Servo instance.
 */
void syn_servo_update(SYN_Servo *servo);

/**
 * @brief Get the current pulse width in microseconds.
 * @param servo  Servo.
 * @return Pulse width in µs.
 */
static inline uint16_t syn_servo_get_pulse_us(const SYN_Servo *servo)
{
    return servo->current_us;
}

/**
 * @brief Get the current angle.
 * @param servo  Servo.
 * @return Angle in degrees.
 */
static inline uint16_t syn_servo_get_angle(const SYN_Servo *servo)
{
    uint32_t range_us = (uint32_t)(servo->pulse_max - servo->pulse_min);
    if (range_us == 0) return 0;
    return (uint16_t)(((uint32_t)(servo->current_us - servo->pulse_min) *
                       servo->angle_range) / range_us);
}

/**
 * @brief Is the servo at its target position?
 * @param servo  Servo.
 * @return true if at target.
 */
static inline bool syn_servo_at_target(const SYN_Servo *servo)
{
    return servo->current_us == servo->target_us;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SERVO_H */
