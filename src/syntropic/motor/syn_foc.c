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

#include "../common/syn_defs.h"
#include "../util/syn_assert.h"
#include "syn_foc.h"

#include <stdint.h>

/** @brief Precomputed √3 in Q16.16 (≈ 1.73205). */
#define Q16_SQRT3 113512

/** @brief Precomputed 1/√3 in Q16.16 (≈ 0.57735). */
#define Q16_INV_SQRT3 37837

/** @brief Precomputed 2/√3 in Q16.16 (≈ 1.15470). */
#define Q16_2_INV_SQRT3 75674

/** @brief Precomputed √3/2 in Q16.16 (≈ 0.86603). */
#define Q16_SQRT3_OVER_2 56756

/**
 * @brief Saturate a 64-bit signed intermediate value to the 32-bit Q16.16 range.
 * @param val 64-bit signed intermediate value.
 * @return Saturated Q16.16 value in [INT32_MIN, INT32_MAX].
 */
static inline q16_t foc_sat_q16(int64_t val)
{
    if (val > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (val < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (q16_t)val;
}

/* ── Clarke transform ───────────────────────────────────────────────────── */

void syn_foc_clarke(const SYN_FOC_ABC *abc, SYN_FOC_AB *ab)
{
    SYN_ASSERT(abc != NULL && ab != NULL);

    /*
     * Equal-amplitude Clarke (simplest form, assumes a + b + c ≈ 0):
     *   α = a
     *   β = (a + 2b) / √3 = (a + 2b) * (1/√3)
     */
    int64_t sum_a2b = (int64_t)abc->a + (2LL * (int64_t)abc->b);

    ab->alpha = abc->a;
    ab->beta = foc_sat_q16((sum_a2b * (int64_t)Q16_INV_SQRT3) >> Q16_SHIFT);
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
    int64_t s3b = ((int64_t)ab->beta * (int64_t)Q16_SQRT3) >> Q16_SHIFT;

    abc->a = ab->alpha;
    abc->b = foc_sat_q16((-(int64_t)ab->alpha + s3b) / 2LL);
    abc->c = foc_sat_q16((-(int64_t)ab->alpha - s3b) / 2LL);
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
    dq->d = foc_sat_q16((int64_t)q16_mul(ab->alpha, cos_t) + (int64_t)q16_mul(ab->beta, sin_t));
    dq->q = foc_sat_q16(-(int64_t)q16_mul(ab->alpha, sin_t) + (int64_t)q16_mul(ab->beta, cos_t));
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
    ab->alpha = foc_sat_q16((int64_t)q16_mul(dq->d, cos_t) - (int64_t)q16_mul(dq->q, sin_t));
    ab->beta = foc_sat_q16((int64_t)q16_mul(dq->d, sin_t) + (int64_t)q16_mul(dq->q, cos_t));
}

void syn_foc_park_fast(const SYN_FOC_AB *ab, q16_t theta, SYN_FOC_DQ *dq)
{
    SYN_ASSERT(ab != NULL && dq != NULL);
    q16_t sin_t, cos_t;
    q16_sincos_fast(theta, &sin_t, &cos_t);

    dq->d = foc_sat_q16((int64_t)q16_mul(ab->alpha, cos_t) + (int64_t)q16_mul(ab->beta, sin_t));
    dq->q = foc_sat_q16(-(int64_t)q16_mul(ab->alpha, sin_t) + (int64_t)q16_mul(ab->beta, cos_t));
}

void syn_foc_inv_park_fast(const SYN_FOC_DQ *dq, q16_t theta, SYN_FOC_AB *ab)
{
    SYN_ASSERT(dq != NULL && ab != NULL);
    q16_t sin_t, cos_t;
    q16_sincos_fast(theta, &sin_t, &cos_t);

    ab->alpha = foc_sat_q16((int64_t)q16_mul(dq->d, cos_t) - (int64_t)q16_mul(dq->q, sin_t));
    ab->beta = foc_sat_q16((int64_t)q16_mul(dq->d, sin_t) + (int64_t)q16_mul(dq->q, cos_t));
}

bool syn_foc_field_weakening(q16_t v_d, q16_t v_q, q16_t v_max, q16_t *id_cmd)
{
    if (id_cmd == NULL || v_max <= 0) {
        return false;
    }
    q16_t v_mag = q16_hypot(v_d, v_q);
    if (v_mag > v_max) {
        q16_t v_err = v_mag - v_max;
        /* Inject negative d-axis current proportional to voltage excess (saturated) */
        *id_cmd = foc_sat_q16((int64_t)(*id_cmd) - (int64_t)q16_mul(v_err, Q16_FROM_FRAC(1, 2)));
        return true;
    }
    return false;
}

/* ── SVPWM ──────────────────────────────────────────────────────────────── */

/**
 * @brief Normalize a zero-sequence-shifted phase voltage to PWM duty in [0, Q16_ONE].
 * @param v_phase  Phase reference voltage (Q16.16).
 * @param v_offset Neutral zero-sequence offset widened to 64 bits.
 * @param v_bus    DC bus voltage (Q16.16, > 0).
 * @return Clamped duty cycle in [0, Q16_ONE].
 */
static inline q16_t foc_svpwm_duty(q16_t v_phase, int64_t v_offset, q16_t v_bus)
{
    q16_t v_shifted = foc_sat_q16((int64_t)v_phase + v_offset);
    int64_t duty = (int64_t)q16_div(v_shifted, v_bus) + (int64_t)Q16_HALF;
    return q16_clamp(foc_sat_q16(duty), 0, Q16_ONE);
}

void syn_foc_svpwm(const SYN_FOC_AB *ab, q16_t v_bus, q16_t *duty_a, q16_t *duty_b, q16_t *duty_c)
{
    SYN_ASSERT(ab != NULL && duty_a != NULL && duty_b != NULL && duty_c != NULL);
    SYN_ASSERT(v_bus > 0);
    if (v_bus <= 0)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    SYN_FOC_ABC v_ref;
    syn_foc_inv_clarke(ab, &v_ref);
    q16_t va = v_ref.a;
    q16_t vb = v_ref.b;
    q16_t vc = v_ref.c;

    q16_t v_min = SYN_MIN(va, SYN_MIN(vb, vc));
    q16_t v_max = SYN_MAX(va, SYN_MAX(vb, vc));

    int64_t v_offset = -(((int64_t)v_max + (int64_t)v_min) / 2LL);

    *duty_a = foc_svpwm_duty(va, v_offset, v_bus);
    *duty_b = foc_svpwm_duty(vb, v_offset, v_bus);
    *duty_c = foc_svpwm_duty(vc, v_offset, v_bus);
}

/**
 * @brief Compute sign-adjusted dead-time duty cycle offset with zero-crossing interpolation.
 * @param current  Measured phase current (Q16.16).
 * @param dt_comp  Base dead-time compensation offset (Q16.16).
 * @return Scaled compensation offset (Q16.16).
 */
static inline q16_t dt_sign_scale(q16_t current, q16_t dt_comp)
{
    q16_t thresh = Q16_FROM_FRAC(1, 10); /* 0.1A zero-crossing threshold */
    if (current >= thresh) {
        return dt_comp;
    }
    if (current <= -thresh) {
        return q16_sub_sat(0, dt_comp);
    }
    /* Smooth linear interpolation across zero-crossing band */
    return q16_mul_sat(q16_div(current, thresh), dt_comp);
}

void syn_foc_deadtime_comp(q16_t *duty_a, q16_t *duty_b, q16_t *duty_c, const SYN_FOC_ABC *i_abc,
                           q16_t dt_comp)
{
    SYN_ASSERT(duty_a != NULL && duty_b != NULL && duty_c != NULL && i_abc != NULL);
    if (duty_a == NULL || duty_b == NULL || duty_c == NULL || i_abc == NULL || dt_comp == 0) {
        return;
    }

    *duty_a = q16_clamp(q16_add_sat(*duty_a, dt_sign_scale(i_abc->a, dt_comp)), 0, Q16_ONE);
    *duty_b = q16_clamp(q16_add_sat(*duty_b, dt_sign_scale(i_abc->b, dt_comp)), 0, Q16_ONE);
    *duty_c = q16_clamp(q16_add_sat(*duty_c, dt_sign_scale(i_abc->c, dt_comp)), 0, Q16_ONE);
}

#endif /* SYN_USE_FOC */
