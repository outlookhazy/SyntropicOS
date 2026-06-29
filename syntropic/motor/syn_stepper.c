#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_STEPPER) || SYN_USE_STEPPER

/**
 * @file syn_stepper.c
 * @brief Stepper motor driver implementation.
 */

#include "syn_stepper.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Set stepper direction pin.
 * @param s        Stepper instance.
 * @param forward  true for forward, false for reverse.
 */
static void set_direction(const SYN_Stepper *s, bool forward)
{
    SYN_GPIO_State lvl = forward ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    if (s->dir_invert) {
        lvl = (lvl == SYN_GPIO_HIGH) ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    }
    syn_port_gpio_write(s->dir_pin, lvl);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_stepper_init(SYN_Stepper *s,
                       SYN_GPIO_Pin step_pin,
                       SYN_GPIO_Pin dir_pin)
{
    SYN_ASSERT(s != NULL);

    memset(s, 0, sizeof(*s));
    s->step_pin    = step_pin;
    s->dir_pin     = dir_pin;
    s->enable_pin  = (SYN_GPIO_Pin)-1;
    s->state       = (uint8_t)SYN_STEPPER_IDLE;

    syn_port_gpio_write(step_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(dir_pin, SYN_GPIO_LOW);
}

void syn_stepper_set_enable_pin(SYN_Stepper *s, SYN_GPIO_Pin pin,
                                 bool active_low)
{
    SYN_ASSERT(s != NULL);
    s->enable_pin    = pin;
    s->enable_invert = active_low;
}

void syn_stepper_set_speed(SYN_Stepper *s, uint32_t max_sps,
                            uint32_t accel_sps2)
{
    SYN_ASSERT(s != NULL);
    SYN_ASSERT(max_sps > 0);
    SYN_ASSERT(accel_sps2 > 0);

    s->max_speed = max_sps;
    s->accel     = accel_sps2;
}

void syn_stepper_move(SYN_Stepper *s, int32_t steps)
{
    SYN_ASSERT(s != NULL);

    if (steps == 0) return;

    bool forward = (steps > 0);
    int32_t abs_steps = forward ? steps : -steps;

    set_direction(s, forward);

    s->steps_to_go = abs_steps;
    s->target      = s->position + steps;

    /* Compute deceleration start point:
     * For a symmetric trapezoidal profile, decel starts at half the
     * total steps. If max speed can't be reached, it's a triangle. */
    int32_t accel_steps = 0;
    if (s->accel > 0) {
        /* v² = 2 * a * d → d = v² / (2a) */
        accel_steps = (int32_t)(((uint64_t)s->max_speed * s->max_speed) /
                                (2u * s->accel));
        if (accel_steps > abs_steps / 2) {
            accel_steps = abs_steps / 2;
        }
    }
    s->decel_start = abs_steps - accel_steps;

    s->speed          = 0;
    s->step_interval  = 0;
    s->last_step_tick = syn_port_get_tick_ms();
    s->state          = (uint8_t)SYN_STEPPER_ACCEL;
}

void syn_stepper_move_to(SYN_Stepper *s, int32_t position)
{
    syn_stepper_move(s, position - s->position);
}

void syn_stepper_tick(SYN_Stepper *s)
{
    SYN_ASSERT(s != NULL);

    if (s->state == (uint8_t)SYN_STEPPER_IDLE) return;

    uint32_t now = syn_port_get_tick_ms();
    uint32_t dt = now - s->last_step_tick;

    /* Compute current speed based on state */
    uint32_t target_speed = 0;

    switch ((SYN_StepperState)s->state) {
    case SYN_STEPPER_ACCEL:
        /* Accelerating — increase speed by accel * dt */
        s->speed += (s->accel * dt) / 1000u;
        if (s->speed >= s->max_speed) {
            s->speed = s->max_speed;
            s->state = (uint8_t)SYN_STEPPER_CRUISE;
        }
        target_speed = s->speed;
        break;

    case SYN_STEPPER_CRUISE:
        target_speed = s->max_speed;
        break;

    case SYN_STEPPER_DECEL:
        if (s->speed > (s->accel * dt) / 1000u) {
            s->speed -= (s->accel * dt) / 1000u;
        } else {
            s->speed = 1; /* minimum speed to finish */
        }
        target_speed = s->speed;
        break;

    case SYN_STEPPER_IDLE:
        return;
    }

    /* Compute step interval from speed */
    if (target_speed == 0) target_speed = 1;
    uint32_t interval_ms = 1000u / target_speed;
    if (interval_ms == 0) interval_ms = 1;

    /* Time to step? */
    if (dt >= interval_ms) {
        /* Generate step pulse */
        syn_port_gpio_write(s->step_pin, SYN_GPIO_HIGH);
        /* In a real system you'd want a tiny delay here,
         * but for compatibility we just toggle */
        syn_port_gpio_write(s->step_pin, SYN_GPIO_LOW);

        /* Update position */
        if (s->target > s->position) {
            s->position++;
        } else {
            s->position--;
        }
        s->steps_to_go--;
        s->last_step_tick = now;

        /* Check if we should start decelerating */
        if (s->state != (uint8_t)SYN_STEPPER_DECEL &&
            s->steps_to_go <= (s->decel_start > 0 ?
            (int32_t)(((uint64_t)s->speed * s->speed) / (2u * s->accel)) : 0)) {
            s->state = (uint8_t)SYN_STEPPER_DECEL;
        }

        /* Check if move is complete */
        if (s->steps_to_go <= 0) {
            s->state = (uint8_t)SYN_STEPPER_IDLE;
            s->speed = 0;
        }
    }
}

void syn_stepper_stop(SYN_Stepper *s)
{
    SYN_ASSERT(s != NULL);
    s->state      = (uint8_t)SYN_STEPPER_IDLE;
    s->speed      = 0;
    s->steps_to_go = 0;
}

void syn_stepper_enable(const SYN_Stepper *s, bool enable)
{
    SYN_ASSERT(s != NULL);
    if (s->enable_pin == (SYN_GPIO_Pin)-1) return;

    SYN_GPIO_State lvl = enable ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    if (s->enable_invert) {
        lvl = (lvl == SYN_GPIO_HIGH) ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    }
    syn_port_gpio_write(s->enable_pin, lvl);
}

#endif /* SYN_USE_STEPPER */
