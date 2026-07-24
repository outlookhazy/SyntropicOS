#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FOC) || SYN_USE_FOC

/**
 * @file syn_foc.c
 * @brief Fixed-point Field-Oriented Control transforms.
 *
 * All transforms use Q16.16 arithmetic with int64_t intermediates.
 * No floating point, no heap allocation.
 */

#include "syn_foc.h"
#include "../util/syn_assert.h"
#include "../common/syn_defs.h"

/** @brief Precomputed √3 in Q16.16 (≈ 1.73205). */
#define Q16_SQRT3         113512

/** @brief Precomputed 1/√3 in Q16.16 (≈ 0.57735). */
#define Q16_INV_SQRT3     37837

/** @brief Precomputed 2/√3 in Q16.16 (≈ 1.15470). */
#define Q16_2_INV_SQRT3   75674

/** @brief Precomputed √3/2 in Q16.16 (≈ 0.86603). */
#define Q16_SQRT3_OVER_2  56756

/* ── Clarke transform ───────────────────────────────────────────────────── */

void syn_foc_clarke(const SYN_FOC_ABC *abc, SYN_FOC_AB *ab)
{
    SYN_ASSERT(abc != NULL && ab != NULL);

    /*
     * Equal-amplitude Clarke (simplest form, assumes a + b + c ≈ 0):
     *   α = a
     *   β = (a + 2b) / √3 = (a + 2b) * (1/√3)
     */
    ab->alpha = abc->a;
    ab->beta = q16_mul(abc->a + 2 * abc->b, Q16_INV_SQRT3);
}

void syn_foc_inv_clarke(const SYN_FOC_AB *ab, SYN_FOC_ABC *abc)
{
    SYN_ASSERT(ab != NULL && abc != NULL);

    /*
     * Inverse Clarke:
     *   a = α
     *   b = (-α + √3·β) / 2
     *   c = (-α - √3·β) / 2
     *   where √3 ≈ 1.73205 (113512 in Q16.16)
     */
    q16_t s3b = q16_mul(ab->beta, Q16_SQRT3);

    abc->a = ab->alpha;
    abc->b = (-ab->alpha + s3b) / 2;
    abc->c = (-ab->alpha - s3b) / 2;
}

/* ── Park transform ─────────────────────────────────────────────────────── */

void syn_foc_park(const SYN_FOC_AB *ab, q16_t theta, SYN_FOC_DQ *dq)
{
    SYN_ASSERT(ab != NULL && dq != NULL);

    q16_t cos_t = q16_cos(theta);
    q16_t sin_t = q16_sin(theta);

    /*
     * d =  α·cos(θ) + β·sin(θ)
     * q = −α·sin(θ) + β·cos(θ)
     */
    dq->d = q16_mul(ab->alpha, cos_t) + q16_mul(ab->beta, sin_t);
    dq->q = -q16_mul(ab->alpha, sin_t) + q16_mul(ab->beta, cos_t);
}

void syn_foc_inv_park(const SYN_FOC_DQ *dq, q16_t theta, SYN_FOC_AB *ab)
{
    SYN_ASSERT(dq != NULL && ab != NULL);

    q16_t cos_t = q16_cos(theta);
    q16_t sin_t = q16_sin(theta);

    /*
     * α = d·cos(θ) − q·sin(θ)
     * β = d·sin(θ) + q·cos(θ)
     */
    ab->alpha = q16_mul(dq->d, cos_t) - q16_mul(dq->q, sin_t);
    ab->beta  = q16_mul(dq->d, sin_t) + q16_mul(dq->q, cos_t);
}

/* ── SVPWM ──────────────────────────────────────────────────────────────── */

void syn_foc_svpwm(const SYN_FOC_AB *ab, q16_t v_bus,
                   q16_t *duty_a, q16_t *duty_b, q16_t *duty_c)
{
    SYN_ASSERT(ab != NULL && duty_a != NULL && duty_b != NULL && duty_c != NULL);
    SYN_ASSERT(v_bus > 0);
    if (v_bus <= 0) return;

    /*
     * Standard 7-segment SVPWM via inverse Clarke with center-aligned PWM.
     *
     * Step 1: Compute 3-phase reference voltages from (α, β).
     *   va = α
     *   vb = (-α + √3·β) / 2
     *   vc = (-α - √3·β) / 2
     */
    q16_t s3b = q16_mul(ab->beta, Q16_SQRT3);  /* √3·β */

    q16_t va = ab->alpha;
    q16_t vb = (-ab->alpha + s3b) / 2;
    q16_t vc = (-ab->alpha - s3b) / 2;

    /*
     * Step 2: Find min and max for center-clamping.
     * The center offset shifts all three voltages so the midpoint
     * aligns with v_bus/2, maximizing linear modulation range.
     */
    q16_t v_min = SYN_MIN(va, SYN_MIN(vb, vc));
    q16_t v_max = SYN_MAX(va, SYN_MAX(vb, vc));

    q16_t v_offset = -(v_max + v_min) / 2;

    /*
     * Step 3: Normalize to [0, 1] duty cycle.
     *   duty = (v + offset) / v_bus + 0.5
     */
    *duty_a = q16_clamp(q16_div(va + v_offset, v_bus) + Q16_HALF, 0, Q16_ONE);
    *duty_b = q16_clamp(q16_div(vb + v_offset, v_bus) + Q16_HALF, 0, Q16_ONE);
    *duty_c = q16_clamp(q16_div(vc + v_offset, v_bus) + Q16_HALF, 0, Q16_ONE);
}

#endif /* SYN_USE_FOC */
