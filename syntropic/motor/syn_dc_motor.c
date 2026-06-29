#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DC_MOTOR) || SYN_USE_DC_MOTOR

/**
 * @file syn_dc_motor.c
 * @brief DC motor controller implementation.
 */

#include "syn_dc_motor.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Clamp speed to the valid range (-100 to +100).
 * @param speed  Raw speed value.
 * @return Clamped speed.
 */
static int16_t clamp_speed(int16_t speed)
{
    if (speed > 100)  return 100;
    if (speed < -100) return -100;
    return speed;
}

/**
 * @brief Apply the current speed to the motor outputs.
 * @param m  DC motor instance.
 */
static void apply_speed(SYN_DCMotor *m)
{
    int16_t spd = m->speed;
    bool forward = (spd >= 0);
    if (m->invert) forward = !forward;

    uint8_t duty = (uint8_t)(spd >= 0 ? spd : -spd);

    switch ((SYN_DCMotorMode)m->mode) {
    case SYN_DC_MODE_PWM_DIR:
        /* pin_a = PWM, pin_b = direction */
        syn_port_gpio_write(m->pin_b,
                             forward ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
        if (m->set_duty != NULL) {
            m->set_duty(m->pin_a, duty, m->duty_ctx);
        } else {
            /* Fallback: just set GPIO high/low */
            syn_port_gpio_write(m->pin_a,
                                 duty > 0 ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
        }
        break;

    case SYN_DC_MODE_DUAL_PWM:
        /* pin_a = forward PWM, pin_b = reverse PWM */
        if (forward) {
            if (m->set_duty != NULL) {
                m->set_duty(m->pin_a, duty, m->duty_ctx);
                m->set_duty(m->pin_b, 0,    m->duty_ctx);
            } else {
                syn_port_gpio_write(m->pin_a,
                                     duty > 0 ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
                syn_port_gpio_write(m->pin_b, SYN_GPIO_LOW);
            }
        } else {
            if (m->set_duty != NULL) {
                m->set_duty(m->pin_a, 0,    m->duty_ctx);
                m->set_duty(m->pin_b, duty, m->duty_ctx);
            } else {
                syn_port_gpio_write(m->pin_a, SYN_GPIO_LOW);
                syn_port_gpio_write(m->pin_b,
                                     duty > 0 ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
            }
        }
        break;
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_dc_motor_init(SYN_DCMotor *motor, SYN_GPIO_Pin pin_a,
                        SYN_GPIO_Pin pin_b, SYN_DCMotorMode mode)
{
    SYN_ASSERT(motor != NULL);

    memset(motor, 0, sizeof(*motor));
    motor->pin_a = pin_a;
    motor->pin_b = pin_b;
    motor->mode  = (uint8_t)mode;

    syn_port_gpio_write(pin_a, SYN_GPIO_LOW);
    syn_port_gpio_write(pin_b, SYN_GPIO_LOW);
}

void syn_dc_motor_set_duty_callback(SYN_DCMotor *motor,
                                     void (*cb)(SYN_GPIO_Pin, uint8_t, void *),
                                     void *ctx)
{
    SYN_ASSERT(motor != NULL);
    motor->set_duty = cb;
    motor->duty_ctx = ctx;
}

void syn_dc_motor_set_speed(SYN_DCMotor *motor, int16_t speed)
{
    SYN_ASSERT(motor != NULL);
    speed = clamp_speed(speed);
    motor->speed     = speed;
    motor->target    = speed;
    motor->ramp_rate = 0;
    apply_speed(motor);
}

void syn_dc_motor_ramp_to(SYN_DCMotor *motor, int16_t speed,
                           uint16_t duration)
{
    SYN_ASSERT(motor != NULL);
    speed = clamp_speed(speed);
    motor->target = speed;

    if (duration == 0) {
        motor->speed = speed;
        motor->ramp_rate = 0;
        apply_speed(motor);
        return;
    }

    int32_t delta = (int32_t)speed - (int32_t)motor->speed;
    /* Rate in % per ms, Q8 fixed-point */
    motor->ramp_rate = (delta * 256) / (int32_t)duration;
    if (motor->ramp_rate == 0 && delta != 0) {
        motor->ramp_rate = (delta > 0) ? 1 : -1;
    }
    motor->last_tick = syn_port_get_tick_ms();
}

void syn_dc_motor_update(SYN_DCMotor *motor)
{
    SYN_ASSERT(motor != NULL);

    if (motor->speed == motor->target || motor->ramp_rate == 0) return;

    uint32_t now = syn_port_get_tick_ms();
    uint32_t dt = now - motor->last_tick;
    if (dt == 0) return;

    motor->last_tick = now;

    int32_t delta = (motor->ramp_rate * (int32_t)dt) / 256;
    if (delta == 0) {
        delta = (motor->ramp_rate > 0) ? 1 : -1;
    }

    int32_t new_speed = (int32_t)motor->speed + delta;

    if ((motor->ramp_rate > 0 && new_speed >= (int32_t)motor->target) ||
        (motor->ramp_rate < 0 && new_speed <= (int32_t)motor->target)) {
        motor->speed     = motor->target;
        motor->ramp_rate = 0;
    } else {
        motor->speed = clamp_speed((int16_t)new_speed);
    }

    apply_speed(motor);
}

void syn_dc_motor_coast(SYN_DCMotor *motor)
{
    SYN_ASSERT(motor != NULL);
    motor->speed     = 0;
    motor->target    = 0;
    motor->ramp_rate = 0;
    syn_port_gpio_write(motor->pin_a, SYN_GPIO_LOW);
    syn_port_gpio_write(motor->pin_b, SYN_GPIO_LOW);
}

void syn_dc_motor_brake(SYN_DCMotor *motor)
{
    SYN_ASSERT(motor != NULL);
    motor->speed     = 0;
    motor->target    = 0;
    motor->ramp_rate = 0;
    syn_port_gpio_write(motor->pin_a, SYN_GPIO_HIGH);
    syn_port_gpio_write(motor->pin_b, SYN_GPIO_HIGH);
}

#endif /* SYN_USE_DC_MOTOR */
