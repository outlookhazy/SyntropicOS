/**
 * @file syn_ramp.h
 * @brief Ramp / motion profile generator.
 *
 * Generates smooth setpoint transitions for any controlled variable
 * (valves, lights, temperature, motor speed, etc.).
 *
 * Supports two modes:
 * - LINEAR:  constant slew rate
 * - SCURVE:  acceleration/deceleration for jerk-free motion
 *
 * Usage:
 * @code
 *   static SYN_Ramp ramp;
 *   syn_ramp_init(&ramp, 0);
 *   syn_ramp_set_target(&ramp, 1000, 10);  // ramp to 1000 at 10/tick
 *
 *   // In your update loop:
 *   int32_t output = syn_ramp_update(&ramp);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_RAMP_H
#define SYN_RAMP_H

#include "../common/syn_defs.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Ramp mode ──────────────────────────────────────────────────────────── */

/** @brief Ramp profile mode. */
typedef enum {
    SYN_RAMP_LINEAR = 0,   /**< Constant rate                             */
    SYN_RAMP_SCURVE = 1,   /**< Smooth acceleration/deceleration          */
} SYN_RampMode;

/* ── Ramp instance ──────────────────────────────────────────────────────── */

/** @brief Ramp generator instance — current value, target, rate. */
typedef struct {
    int32_t  current;       /**< Current output value                      */
    int32_t  target;        /**< Desired final value                       */
    int32_t  rate;          /**< Max units per update (slew rate)           */
    int32_t  velocity;      /**< Current velocity (for S-curve)            */
    int32_t  accel;         /**< Acceleration (for S-curve, units/tick²)   */
    uint8_t  mode;          /**< SYN_RampMode                             */
    bool     done;          /**< true when current == target               */
} SYN_Ramp;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize ramp generator.
 *
 * @param ramp     Ramp instance.
 * @param initial  Starting value.
 */
void syn_ramp_init(SYN_Ramp *ramp, int32_t initial);

/**
 * @brief Set new target with linear ramp.
 *
 * @param ramp    Ramp instance.
 * @param target  Desired final value.
 * @param rate    Max change per update call (always positive).
 */
void syn_ramp_set_target(SYN_Ramp *ramp, int32_t target, int32_t rate);

/**
 * @brief Set new target with S-curve ramp.
 *
 * @param ramp     Ramp instance.
 * @param target   Desired final value.
 * @param max_rate Max velocity (units/tick).
 * @param accel    Acceleration (units/tick²). Controls smoothness.
 */
void syn_ramp_set_target_scurve(SYN_Ramp *ramp, int32_t target,
                                  int32_t max_rate, int32_t accel);

/**
 * @brief Update the ramp — call once per tick.
 *
 * @param ramp  Ramp instance.
 * @return Current output value.
 */
int32_t syn_ramp_update(SYN_Ramp *ramp);

/**
 * @brief Jump immediately to a value (no ramp).
 * @param ramp   Ramp instance.
 * @param value  Value to jump to.
 */
void syn_ramp_jump(SYN_Ramp *ramp, int32_t value);

/**
 * @brief Check if ramp has reached its target.
 * @param ramp  Ramp instance.
 * @return true if done.
 */
static inline bool syn_ramp_done(const SYN_Ramp *ramp)
{
    return ramp->done;
}

/**
 * @brief Get current value without updating.
 * @param ramp  Ramp instance.
 * @return Current value.
 */
static inline int32_t syn_ramp_value(const SYN_Ramp *ramp)
{
    return ramp->current;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_RAMP_H */
