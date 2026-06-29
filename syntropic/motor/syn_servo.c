#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SERVO) || SYN_USE_SERVO

/**
 * @file syn_servo.c
 * @brief Servo controller implementation.
 */

#include "syn_servo.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Convert angle to pulse width in microseconds.
 * @param s      Servo instance.
 * @param angle  Angle (0 to angle_range).
 * @return Pulse width in µs.
 */
static uint16_t angle_to_us(const SYN_Servo *s, uint16_t angle)
{
    if (angle > s->angle_range) angle = s->angle_range;
    uint32_t range_us = (uint32_t)(s->pulse_max - s->pulse_min);
    return (uint16_t)(s->pulse_min +
                      (range_us * angle) / s->angle_range);
}

/**
 * @brief Clamp pulse width to min/max range.
 * @param s   Servo instance.
 * @param us  Pulse width in µs.
 * @return Clamped pulse width.
 */
static uint16_t clamp_us(const SYN_Servo *s, uint16_t us)
{
    if (us < s->pulse_min) return s->pulse_min;
    if (us > s->pulse_max) return s->pulse_max;
    return us;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_servo_init(SYN_Servo *servo, uint16_t pulse_min,
                     uint16_t pulse_max, uint16_t angle_range)
{
    SYN_ASSERT(servo != NULL);
    SYN_ASSERT(pulse_max > pulse_min);
    SYN_ASSERT(angle_range > 0);

    memset(servo, 0, sizeof(*servo));
    servo->pulse_min   = pulse_min;
    servo->pulse_max   = pulse_max;
    servo->angle_range = angle_range;

    /* Default to center */
    uint16_t center = (uint16_t)(pulse_min + (pulse_max - pulse_min) / 2);
    servo->current_us = center;
    servo->target_us  = center;
}

void syn_servo_set_angle(SYN_Servo *servo, uint16_t angle)
{
    SYN_ASSERT(servo != NULL);
    uint16_t us = angle_to_us(servo, angle);
    servo->current_us = us;
    servo->target_us  = us;
    servo->rate       = 0;
}

void syn_servo_set_pulse(SYN_Servo *servo, uint16_t us)
{
    SYN_ASSERT(servo != NULL);
    us = clamp_us(servo, us);
    servo->current_us = us;
    servo->target_us  = us;
    servo->rate       = 0;
}

void syn_servo_move_to(SYN_Servo *servo, uint16_t angle, uint16_t duration)
{
    SYN_ASSERT(servo != NULL);

    uint16_t target = angle_to_us(servo, angle);
    servo->target_us = target;

    if (duration == 0) {
        servo->current_us = target;
        servo->rate = 0;
        return;
    }

    int32_t delta = (int32_t)target - (int32_t)servo->current_us;
    /* Rate in µs per ms (can be negative for reverse movement).
     * Scale up by 256 for fixed-point precision. */
    servo->rate = (delta * 256) / (int32_t)duration;
    if (servo->rate == 0 && delta != 0) {
        servo->rate = (delta > 0) ? 1 : -1;
    }
    servo->last_tick = syn_port_get_tick_ms();
}

void syn_servo_update(SYN_Servo *servo)
{
    SYN_ASSERT(servo != NULL);

    if (servo->current_us == servo->target_us || servo->rate == 0) return;

    uint32_t now = syn_port_get_tick_ms();
    uint32_t dt = now - servo->last_tick;
    if (dt == 0) return;

    servo->last_tick = now;

    int32_t delta = (servo->rate * (int32_t)dt) / 256;
    if (delta == 0) {
        delta = (servo->rate > 0) ? 1 : -1;
    }

    int32_t new_us = (int32_t)servo->current_us + delta;

    /* Check if we've reached or passed the target */
    if ((servo->rate > 0 && new_us >= (int32_t)servo->target_us) ||
        (servo->rate < 0 && new_us <= (int32_t)servo->target_us)) {
        servo->current_us = servo->target_us;
        servo->rate = 0;
    } else {
        servo->current_us = clamp_us(servo, (uint16_t)new_us);
    }
}

#endif /* SYN_USE_SERVO */
