#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_STEPPER) || SYN_USE_STEPPER

/**
 * @file syn_stepper.c
 * @brief Stepper motor driver implementation.
 */

#include "../util/syn_assert.h"
#include "syn_stepper.h"

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

/**
 * @brief Update stepper speed ramp state for the elapsed tick interval.
 * @param s       Stepper instance.
 * @param dt_tick Elapsed milliseconds since previous tick.
 * @return Updated step rate in steps per second (at least 1).
 */
static uint32_t stepper_update_speed(SYN_Stepper *s, uint32_t dt_tick)
{
    uint64_t acc_total = (uint64_t)s->accel * (uint64_t)dt_tick + (uint64_t)s->speed_rem;
    uint64_t dv = acc_total / 1000ULL;
    s->speed_rem = (uint32_t)(acc_total % 1000ULL);

    if (s->state == (uint8_t)SYN_STEPPER_ACCEL) {
        uint64_t next_speed = (uint64_t)s->speed + dv;
        if (next_speed >= (uint64_t)s->max_speed) {
            s->speed = s->max_speed;
            s->speed_rem = 0U;
            s->state = (uint8_t)SYN_STEPPER_CRUISE;
        } else {
            s->speed = (uint32_t)next_speed;
        }
        return (s->speed > 0U) ? s->speed : 1U;
    }

    if (s->state == (uint8_t)SYN_STEPPER_CRUISE) {
        return s->max_speed;
    }

    if ((uint64_t)s->speed > dv) {
        s->speed -= (uint32_t)dv;
    } else {
        s->speed = 1U;
    }
    return s->speed;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_stepper_init(SYN_Stepper *s, SYN_GPIO_Pin step_pin, SYN_GPIO_Pin dir_pin)
{
    SYN_ASSERT(s != NULL);
    if (s == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    memset(s, 0, sizeof(*s));
    s->step_pin = step_pin;
    s->dir_pin = dir_pin;
    s->enable_pin = (SYN_GPIO_Pin)-1;
    s->state = (uint8_t)SYN_STEPPER_IDLE;

    syn_port_gpio_write(step_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(dir_pin, SYN_GPIO_LOW);
}

void syn_stepper_set_enable_pin(SYN_Stepper *s, SYN_GPIO_Pin pin, bool active_low)
{
    SYN_ASSERT(s != NULL);
    if (s == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    s->enable_pin = pin;
    s->enable_invert = active_low;
}

void syn_stepper_set_speed(SYN_Stepper *s, uint32_t max_sps, uint32_t accel_sps2)
{
    SYN_ASSERT(s != NULL);
    SYN_ASSERT(max_sps > 0);
    SYN_ASSERT(accel_sps2 > 0);

    if (s == NULL || max_sps == 0 || accel_sps2 == 0) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    s->max_speed = max_sps;
    s->accel = accel_sps2;
}

void syn_stepper_move(SYN_Stepper *s, int32_t steps)
{
    SYN_ASSERT(s != NULL);

    if (steps == 0 || s->accel == 0U || s->max_speed == 0U) {
        return;
    }

    bool forward = (steps > 0);
    int64_t steps64 = forward ? (int64_t)steps : -(int64_t)steps;
    int32_t abs_steps = (steps64 > (int64_t)INT32_MAX) ? INT32_MAX : (int32_t)steps64;

    set_direction(s, forward);

    s->steps_to_go = abs_steps;
    int64_t target64 = (int64_t)s->position + (int64_t)steps;
    if (target64 > (int64_t)INT32_MAX) {
        s->target = INT32_MAX;
    } else if (target64 < (int64_t)INT32_MIN) {
        s->target = INT32_MIN;
    } else {
        s->target = (int32_t)target64;
    }

    /* Compute deceleration start point:
     * For a symmetric trapezoidal profile, decel starts at half the
     * total steps. If max speed can't be reached, it's a triangle. */
    uint64_t raw_accel_steps =
        ((uint64_t)s->max_speed * (uint64_t)s->max_speed) / (2ULL * (uint64_t)s->accel);
    int32_t half_steps = abs_steps / 2;
    int32_t accel_steps =
        (raw_accel_steps > (uint64_t)half_steps) ? half_steps : (int32_t)raw_accel_steps;
    s->decel_start = abs_steps - accel_steps;

    uint32_t now = syn_port_get_tick_ms();
    s->speed = 0U;
    s->speed_rem = 0U;
    s->step_interval = 0U;
    s->last_step_tick = now;
    s->last_tick_ms = now;
    s->state = (uint8_t)SYN_STEPPER_ACCEL;
}

void syn_stepper_move_to(SYN_Stepper *s, int32_t position)
{
    SYN_ASSERT(s != NULL);
    int64_t diff = (int64_t)position - (int64_t)s->position;
    if (diff > (int64_t)INT32_MAX) {
        diff = (int64_t)INT32_MAX;
    } else if (diff < (int64_t)INT32_MIN + 1LL) {
        diff = (int64_t)INT32_MIN + 1LL;
    }
    syn_stepper_move(s, (int32_t)diff);
}

void syn_stepper_tick(SYN_Stepper *s)
{
    SYN_ASSERT(s != NULL);

    if (s->state == (uint8_t)SYN_STEPPER_IDLE || s->accel == 0U || s->max_speed == 0U) {
        s->state = (uint8_t)SYN_STEPPER_IDLE;
        return;
    }

    uint32_t now = syn_port_get_tick_ms();
    uint32_t dt_tick = now - s->last_tick_ms;
    s->last_tick_ms = now;

    uint32_t target_speed = stepper_update_speed(s, dt_tick);
    uint32_t interval_ms = 1000U / target_speed;
    if (interval_ms == 0U) {
        interval_ms = 1U;
    }

    uint32_t step_dt = now - s->last_step_tick;
    if (step_dt >= interval_ms) {
        syn_port_gpio_write(s->step_pin, SYN_GPIO_HIGH);
        syn_port_gpio_write(s->step_pin, SYN_GPIO_LOW);

        if (s->target > s->position) {
            s->position++;
        } else {
            s->position--;
        }
        s->steps_to_go--;
        s->last_step_tick = now;

        if (s->state != (uint8_t)SYN_STEPPER_DECEL) {
            uint64_t stop_steps =
                ((uint64_t)s->speed * (uint64_t)s->speed) / (2ULL * (uint64_t)s->accel);
            if ((uint64_t)s->steps_to_go <= stop_steps) {
                s->state = (uint8_t)SYN_STEPPER_DECEL;
                s->speed_rem = 0U;
            }
        }

        if (s->steps_to_go <= 0) {
            s->state = (uint8_t)SYN_STEPPER_IDLE;
            s->speed = 0U;
            s->speed_rem = 0U;
        }
    }
}

void syn_stepper_stop(SYN_Stepper *s)
{
    SYN_ASSERT(s != NULL);
    s->state = (uint8_t)SYN_STEPPER_IDLE;
    s->speed = 0;
    s->steps_to_go = 0;
}

void syn_stepper_enable(const SYN_Stepper *s, bool enable)
{
    SYN_ASSERT(s != NULL);
    if (s->enable_pin == (SYN_GPIO_Pin)-1)
        return;

    SYN_GPIO_State lvl = enable ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    if (s->enable_invert) {
        lvl = (lvl == SYN_GPIO_HIGH) ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    }
    syn_port_gpio_write(s->enable_pin, lvl);
}

/* ── SYN_MotorOutput adapter ───────────────────────────────────────────── */

/**
 * @brief Set output adapter for SYN_MotorOutput (ticks the stepper).
 * @param ctx     Stepper instance (SYN_Stepper*).
 * @param output  Unused — stepper motion is driven by its own commands.
 */
static void stepper_output_set(void *ctx, int32_t output)
{
    SYN_Stepper *s = (SYN_Stepper *)ctx;
    (void)output;
    syn_stepper_tick(s);
}

/**
 * @brief Coast adapter for SYN_MotorOutput.
 * @param ctx  Stepper instance (SYN_Stepper*).
 */
static void stepper_output_coast(void *ctx)
{
    syn_stepper_stop((SYN_Stepper *)ctx);
}

/**
 * @brief Brake adapter for SYN_MotorOutput.
 * @param ctx  Stepper instance (SYN_Stepper*).
 */
static void stepper_output_brake(void *ctx)
{
    syn_stepper_stop((SYN_Stepper *)ctx);
}

SYN_MotorOutput syn_stepper_output(SYN_Stepper *stepper)
{
    SYN_MotorOutput out = {
        .set_output = stepper_output_set,
        .coast = stepper_output_coast,
        .brake = stepper_output_brake,
        .ctx = stepper,
    };
    return out;
}

#endif /* SYN_USE_STEPPER */
