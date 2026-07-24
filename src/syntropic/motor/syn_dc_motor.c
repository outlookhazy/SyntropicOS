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
#include "../util/syn_ramp.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Clamp speed to the valid range [-duty_max, +duty_max].
 * @param motor  Motor instance (for duty_max).
 * @param speed  Raw speed value.
 * @return Clamped speed.
 */
static int32_t clamp_speed(const SYN_DCMotor *motor, int32_t speed)
{
    if (speed > motor->duty_max)  return motor->duty_max;
    if (speed < -motor->duty_max) return -motor->duty_max;
    return speed;
}

/**
 * @brief Apply the current speed to the motor outputs.
 * @param m  DC motor instance.
 */
static void apply_speed(SYN_DCMotor *m)
{
    int32_t spd = m->speed;
    bool forward = (spd >= 0);
    if (m->invert) forward = !forward;

    uint16_t duty = (uint16_t)(spd >= 0 ? spd : -spd);

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

/* ── SYN_MotorOutput adapter ───────────────────────────────────────────── */

/**
 * @brief Set output adapter for SYN_MotorOutput.
 * @param ctx     DC motor instance (SYN_DCMotor*).
 * @param output  Signed output level.
 */
static void dc_output_set(void *ctx, int32_t output)
{
    SYN_DCMotor *m = (SYN_DCMotor *)ctx;
    syn_dc_motor_set_speed(m, output);
}

/**
 * @brief Coast adapter for SYN_MotorOutput.
 * @param ctx  DC motor instance (SYN_DCMotor*).
 */
static void dc_output_coast(void *ctx)
{
    syn_dc_motor_coast((SYN_DCMotor *)ctx);
}

/**
 * @brief Brake adapter for SYN_MotorOutput.
 * @param ctx  DC motor instance (SYN_DCMotor*).
 */
static void dc_output_brake(void *ctx)
{
    syn_dc_motor_brake((SYN_DCMotor *)ctx);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_dc_motor_init(SYN_DCMotor *motor, SYN_GPIO_Pin pin_a,
                        SYN_GPIO_Pin pin_b, SYN_DCMotorMode mode)
{
    SYN_ASSERT(motor != NULL);

    memset(motor, 0, sizeof(*motor));
    motor->pin_a    = pin_a;
    motor->pin_b    = pin_b;
    motor->mode     = (uint8_t)mode;
    motor->duty_max = SYN_DC_MOTOR_DUTY_MAX_DEFAULT;
    syn_ramp_init(&motor->ramp, 0);

    syn_port_gpio_write(pin_a, SYN_GPIO_LOW);
    syn_port_gpio_write(pin_b, SYN_GPIO_LOW);
}

void syn_dc_motor_set_duty_callback(SYN_DCMotor *motor,
                                     void (*cb)(SYN_GPIO_Pin, uint16_t, void *),
                                     void *ctx)
{
    SYN_ASSERT(motor != NULL);
    motor->set_duty = cb;
    motor->duty_ctx = ctx;
}

void syn_dc_motor_set_speed(SYN_DCMotor *motor, int32_t speed)
{
    SYN_ASSERT(motor != NULL);
    speed = clamp_speed(motor, speed);
    motor->speed     = speed;
    motor->target    = speed;
    motor->ramp_rate = 0;
    syn_ramp_jump(&motor->ramp, speed);
    apply_speed(motor);
}

void syn_dc_motor_ramp_to(SYN_DCMotor *motor, int32_t speed,
                           uint16_t duration)
{
    SYN_ASSERT(motor != NULL);
    speed = clamp_speed(motor, speed);
    motor->target = speed;

    if (duration == 0) {
        motor->speed = speed;
        motor->ramp_rate = 0;
        syn_ramp_jump(&motor->ramp, speed);
        apply_speed(motor);
        return;
    }

    int32_t delta = speed - motor->speed;
    /* Rate in units per ms, Q8 fixed-point */
    motor->ramp_rate = (delta * 256) / (int32_t)duration;
    if (motor->ramp_rate == 0 && delta != 0) {
        motor->ramp_rate = (delta > 0) ? 1 : -1;
    }

    int32_t step_rate = (delta >= 0 ? delta : -delta) / (int32_t)duration;
    if (step_rate == 0 && delta != 0) {
        step_rate = 1;
    }
    syn_ramp_init(&motor->ramp, motor->speed);
    syn_ramp_set_target(&motor->ramp, speed, step_rate);

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

    int32_t new_speed = motor->speed + delta;

    if ((motor->ramp_rate > 0 && new_speed >= motor->target) ||
        (motor->ramp_rate < 0 && new_speed <= motor->target)) {
        motor->speed     = motor->target;
        motor->ramp_rate = 0;
        syn_ramp_jump(&motor->ramp, motor->target);
    } else {
        motor->speed = clamp_speed(motor, new_speed);
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

void syn_dc_motor_set_duty_max(SYN_DCMotor *motor, int32_t duty_max)
{
    SYN_ASSERT(motor != NULL);
    SYN_ASSERT(duty_max > 0);
    motor->duty_max = duty_max;
    motor->speed    = 0;
    motor->target   = 0;
}

SYN_MotorOutput syn_dc_motor_output(SYN_DCMotor *motor)
{
    SYN_MotorOutput out = {
        .set_output = dc_output_set,
        .coast      = dc_output_coast,
        .brake      = dc_output_brake,
        .ctx        = motor,
    };
    return out;
}

#endif /* SYN_USE_DC_MOTOR */
