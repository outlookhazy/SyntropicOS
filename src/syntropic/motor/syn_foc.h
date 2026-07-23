/**
 * @file syn_foc.h
 * @brief Fixed-point Field-Oriented Control transforms (Clarke & Park).
 *
 * Provides the mathematical transforms needed for BLDC/PMSM motor control:
 *
 * - **Clarke transform**: 3-phase (a, b, c) → 2-phase stationary (α, β)
 * - **Inverse Clarke**:   (α, β) → (a, b, c)
 * - **Park transform**:   Stationary (α, β) → rotating (d, q) using rotor angle θ
 * - **Inverse Park**:     Rotating (d, q) → stationary (α, β)
 *
 * All values are Q16.16 fixed-point. No floating point, no heap allocation.
 *
 * @par Usage — FOC current loop
 * @code
 *   SYN_FOC_AB  ab;
 *   SYN_FOC_DQ  dq;
 *   SYN_FOC_ABC phase_currents = { ia, ib, ic };  // measured currents
 *   q16_t theta = electrical_angle;                // rotor angle
 *
 *   // Forward: ABC → αβ → dq
 *   syn_foc_clarke(&phase_currents, &ab);
 *   syn_foc_park(&ab, theta, &dq);
 *
 *   // Now dq.d and dq.q are the field-aligned currents.
 *   // Run PID on dq, then inverse transform for PWM:
 *   syn_foc_inv_park(&dq_cmd, theta, &ab);
 *   syn_foc_inv_clarke(&ab, &phase_voltages);
 * @endcode
 * @ingroup syn_motor
 */

#ifndef SYN_FOC_H
#define SYN_FOC_H

#include "../util/syn_qmath.h"
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

/** @brief 3-phase values (a, b, c). */
typedef struct {
    q16_t a;   /**< Phase A (Q16.16)  */
    q16_t b;   /**< Phase B (Q16.16)  */
    q16_t c;   /**< Phase C (Q16.16)  */
} SYN_FOC_ABC;

/** @brief 2-phase stationary frame (α, β). */
typedef struct {
    q16_t alpha;  /**< Alpha component (Q16.16)  */
    q16_t beta;   /**< Beta component (Q16.16)   */
} SYN_FOC_AB;

/** @brief 2-phase rotating frame (d, q). */
typedef struct {
    q16_t d;   /**< Direct-axis component (Q16.16)     */
    q16_t q;   /**< Quadrature-axis component (Q16.16) */
} SYN_FOC_DQ;

/* ── Clarke transform ───────────────────────────────────────────────────── */

/**
 * @brief Clarke transform: 3-phase (a,b,c) → 2-phase (α,β).
 *
 * Power-invariant form:
 *   α = a
 *   β = (a + 2b) / √3
 *
 * Assumes balanced 3-phase: a + b + c = 0.
 *
 * @param abc  Input 3-phase values.
 * @param ab   Output (α, β).
 */
void syn_foc_clarke(const SYN_FOC_ABC *abc, SYN_FOC_AB *ab);

/**
 * @brief Inverse Clarke: 2-phase (α,β) → 3-phase (a,b,c).
 *
 * @param ab   Input (α, β).
 * @param abc  Output 3-phase values.
 */
void syn_foc_inv_clarke(const SYN_FOC_AB *ab, SYN_FOC_ABC *abc);

/* ── Park transform ─────────────────────────────────────────────────────── */

/**
 * @brief Park transform: stationary (α,β) → rotating (d,q).
 *
 *   d =  α·cos(θ) + β·sin(θ)
 *   q = −α·sin(θ) + β·cos(θ)
 *
 * @param ab     Input stationary-frame values.
 * @param theta  Electrical rotor angle in Q16 radians.
 * @param dq     Output rotating-frame values.
 */
void syn_foc_park(const SYN_FOC_AB *ab, q16_t theta, SYN_FOC_DQ *dq);

/**
 * @brief Inverse Park: rotating (d,q) → stationary (α,β).
 *
 *   α = d·cos(θ) − q·sin(θ)
 *   β = d·sin(θ) + q·cos(θ)
 *
 * @param dq     Input rotating-frame values.
 * @param theta  Electrical rotor angle in Q16 radians.
 * @param ab     Output stationary-frame values.
 */
void syn_foc_inv_park(const SYN_FOC_DQ *dq, q16_t theta, SYN_FOC_AB *ab);

/* ── Utility ────────────────────────────────────────────────────────────── */

/**
 * @brief Compute Space Vector PWM duty cycles from (α, β).
 *
 * Maps (α, β) voltages to three-phase PWM duty cycles in the range
 * [0, Q16_ONE]. Uses standard 7-segment SVPWM pattern.
 *
 * @param ab       Input (α, β) voltage commands (Q16).
 * @param v_bus    DC bus voltage in Q16 (for normalization).
 * @param duty_a   Output duty cycle for phase A [0, Q16_ONE].
 * @param duty_b   Output duty cycle for phase B [0, Q16_ONE].
 * @param duty_c   Output duty cycle for phase C [0, Q16_ONE].
 */
void syn_foc_svpwm(const SYN_FOC_AB *ab, q16_t v_bus,
                   q16_t *duty_a, q16_t *duty_b, q16_t *duty_c);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_H */
