/**
 * @file syn_scurve.h
 * @brief Jerk-limited S-curve trajectory generator.
 *
 * Generates smooth motion profiles by bounding velocity, acceleration, and jerk.
 * Ideal for stepper motors and servo positioning.
 * @ingroup syn_motor
 */

#ifndef SYN_SCURVE_H
#define SYN_SCURVE_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief S-curve trajectory generator state.
 */
typedef struct {
    int32_t p;        /**< Current position */
    int32_t v;        /**< Current velocity */
    int32_t a;        /**< Current acceleration */
    int32_t j;        /**< Current jerk */

    int32_t target_p; /**< Target position */
    
    int32_t v_max;    /**< Maximum velocity magnitude */
    int32_t a_max;    /**< Maximum acceleration magnitude */
    int32_t j_max;    /**< Maximum jerk magnitude */

    int32_t phase_ticks[7]; /**< Ticks to spend in each of the 7 phases */
    int32_t current_phase;  /**< Current phase (0-6), 7 = done */
    int32_t ticks_in_phase; /**< Ticks spent in current phase */
    int32_t dir;            /**< Direction of motion (1 or -1) */

    bool done;        /**< True if target reached */
} SYN_SCurve;

/**
 * @brief Initialize the S-Curve generator.
 * @param sc       Pointer to generator.
 * @param initial_p Initial position.
 */
void syn_scurve_init(SYN_SCurve *sc, int32_t initial_p);

/**
 * @brief Configure the kinematic constraints.
 * @param sc       Pointer to generator.
 * @param v_max    Maximum velocity (units/tick).
 * @param a_max    Maximum acceleration (units/tick^2).
 * @param j_max    Maximum jerk (units/tick^3).
 */
void syn_scurve_set_constraints(SYN_SCurve *sc, int32_t v_max, int32_t a_max, int32_t j_max);

/**
 * @brief Set a new target position.
 * @param sc       Pointer to generator.
 * @param target   Target position.
 */
void syn_scurve_set_target(SYN_SCurve *sc, int32_t target);

/**
 * @brief Update the trajectory generator by one time step (tick).
 * @param sc       Pointer to generator.
 * @return Current position.
 */
int32_t syn_scurve_update(SYN_SCurve *sc);

#ifdef __cplusplus
}
#endif
#endif // SYN_SCURVE_H
