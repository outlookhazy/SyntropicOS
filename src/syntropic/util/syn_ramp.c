#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_RAMP) || SYN_USE_RAMP

/**
 * @file syn_ramp.c
 * @brief Ramp / motion profile generator implementation.
 */

#include "syn_ramp.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_ramp_init(SYN_Ramp *ramp, int32_t initial)
{
    SYN_ASSERT(ramp != NULL);

    memset(ramp, 0, sizeof(*ramp));
    ramp->current = initial;
    ramp->target  = initial;
    ramp->done    = true;
}

void syn_ramp_set_target(SYN_Ramp *ramp, int32_t target, int32_t rate)
{
    SYN_ASSERT(ramp != NULL);
    SYN_ASSERT(rate > 0);

    ramp->target   = target;
    ramp->rate     = rate;
    ramp->velocity = 0;
    ramp->accel    = 0;
    ramp->mode     = (uint8_t)SYN_RAMP_LINEAR;
    ramp->done     = (ramp->current == target);
}

void syn_ramp_set_target_trapezoid(SYN_Ramp *ramp, int32_t target,
                                    int32_t max_rate, int32_t accel)
{
    SYN_ASSERT(ramp != NULL);
    SYN_ASSERT(max_rate > 0);
    SYN_ASSERT(accel > 0);

    ramp->target     = target;
    ramp->rate       = max_rate;
    ramp->accel      = accel;
    ramp->velocity   = 0;
    ramp->frac_accum = 0;
    ramp->frac_bits  = 0;
    ramp->mode       = (uint8_t)SYN_RAMP_TRAPEZOID;
    ramp->done       = (ramp->current == target);
}

void syn_ramp_set_target_trapezoid_fp(SYN_Ramp *ramp, int32_t target,
                                      int32_t max_rate, int32_t accel,
                                      uint8_t frac_bits)
{
    SYN_ASSERT(ramp != NULL);
    SYN_ASSERT(max_rate > 0);
    SYN_ASSERT(accel > 0);
    SYN_ASSERT(frac_bits <= 16);

    ramp->target     = target;
    ramp->rate       = max_rate;
    ramp->accel      = accel;
    ramp->velocity   = 0;
    ramp->frac_accum = 0;
    ramp->frac_bits  = frac_bits;
    ramp->mode       = (uint8_t)SYN_RAMP_TRAPEZOID;
    ramp->done       = (ramp->current == target);
}


/**
 * @brief Linear ramp step toward target.
 * @param ramp  Ramp instance.
 * @return Current ramp value.
 */
static int32_t update_linear(SYN_Ramp *ramp)
{
    int32_t diff = ramp->target - ramp->current;

    if (diff == 0) {
        ramp->done = true;
        return ramp->current;
    }

    if (diff > 0) {
        ramp->current += (diff > ramp->rate) ? ramp->rate : diff;
    } else {
        ramp->current += (diff < -ramp->rate) ? -ramp->rate : diff;
    }

    ramp->done = (ramp->current == ramp->target);
    return ramp->current;
}

/**
 * @brief Trapezoid ramp step toward target.
 *
 * Supports fixed-point velocity/accel when frac_bits > 0.
 * Velocity and accel are in Q`frac_bits` format. Position is integer;
 * a fractional accumulator carries sub-unit remainder.
 *
 * @param ramp  Ramp instance.
 * @return Current ramp value (integer position).
 */
static int32_t update_trapezoid(SYN_Ramp *ramp)
{
    int32_t diff = ramp->target - ramp->current;

    if (diff == 0 && ramp->velocity == 0) {
        ramp->frac_accum = 0;
        ramp->done = true;
        return ramp->current;
    }

    /* Determine direction */
    int32_t dir = (diff > 0) ? 1 : -1;

    /*
     * Deceleration distance: v²/(2*a)
     * When using fixed-point, velocity is in Qn. The decel distance
     * must be in integer units, so: decel_dist = v² / (2*a) >> frac_bits.
     * Use 64-bit to avoid overflow in v².
     */
    int32_t abs_vel  = SYN_ABS(ramp->velocity);
    int32_t abs_diff = SYN_ABS(diff);
    int32_t decel_dist = 0;
    if (ramp->accel > 0) {
        int64_t v2 = (int64_t)abs_vel * abs_vel;
        int64_t denom = 2 * (int64_t)ramp->accel;
        if (ramp->frac_bits > 0) {
            decel_dist = (int32_t)((v2 / denom) >> ramp->frac_bits);
        } else {
            decel_dist = (int32_t)(v2 / denom);
        }
    }

    if (abs_diff <= decel_dist || (dir * ramp->velocity < 0)) {
        /* Decelerate (or reverse) */
        if (ramp->velocity > 0) {
            ramp->velocity -= ramp->accel;
            if (ramp->velocity < 0) ramp->velocity = 0;
        } else if (ramp->velocity < 0) {
            ramp->velocity += ramp->accel;
            if (ramp->velocity > 0) ramp->velocity = 0;
        }
    } else {
        /* Accelerate toward target */
        ramp->velocity += dir * ramp->accel;
        ramp->velocity = SYN_CLAMP(ramp->velocity, -ramp->rate, ramp->rate);
    }

    /* Apply velocity to position.
     * In Q-mode: accumulate fractional bits, carry whole units to current. */
    if (ramp->frac_bits > 0) {
        ramp->frac_accum += ramp->velocity;
        int32_t whole = ramp->frac_accum >> ramp->frac_bits;
        ramp->frac_accum -= whole << ramp->frac_bits;
        ramp->current += whole;
    } else {
        ramp->current += ramp->velocity;
    }

    /* Snap to target if we've overshot or are very close */
    int32_t new_diff = ramp->target - ramp->current;
    if ((diff > 0 && new_diff <= 0) || (diff < 0 && new_diff >= 0)) {
        ramp->current    = ramp->target;
        ramp->velocity   = 0;
        ramp->frac_accum = 0;
        ramp->done       = true;
    }

    return ramp->current;
}

int32_t syn_ramp_update(SYN_Ramp *ramp)
{
    SYN_ASSERT(ramp != NULL);

    if (ramp->done) return ramp->current;

    switch ((SYN_RampMode)ramp->mode) {
    case SYN_RAMP_TRAPEZOID:
        return update_trapezoid(ramp);
    default:
        return update_linear(ramp);
    }
}

void syn_ramp_jump(SYN_Ramp *ramp, int32_t value)
{
    SYN_ASSERT(ramp != NULL);
    ramp->current    = value;
    ramp->target     = value;
    ramp->velocity   = 0;
    ramp->frac_accum = 0;
    ramp->done       = true;
}

#endif /* SYN_USE_RAMP */
