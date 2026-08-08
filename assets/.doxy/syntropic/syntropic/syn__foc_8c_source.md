

# File syn\_foc.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc.c**](syn__foc_8c.md)

[Go to the documentation of this file](syn__foc_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_FOC) || SYN_USE_FOC

#include "../common/syn_defs.h"
#include "../util/syn_assert.h"
#include "syn_foc.h"

#define Q16_SQRT3 113512

#define Q16_INV_SQRT3 37837

#define Q16_2_INV_SQRT3 75674

#define Q16_SQRT3_OVER_2 56756

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
    ab->beta = q16_mul(dq->d, sin_t) + q16_mul(dq->q, cos_t);
}

void syn_foc_park_fast(const SYN_FOC_AB *ab, q16_t theta, SYN_FOC_DQ *dq)
{
    SYN_ASSERT(ab != NULL && dq != NULL);
    q16_t sin_t, cos_t;
    q16_sincos_fast(theta, &sin_t, &cos_t);

    dq->d = q16_mul(ab->alpha, cos_t) + q16_mul(ab->beta, sin_t);
    dq->q = -q16_mul(ab->alpha, sin_t) + q16_mul(ab->beta, cos_t);
}

void syn_foc_inv_park_fast(const SYN_FOC_DQ *dq, q16_t theta, SYN_FOC_AB *ab)
{
    SYN_ASSERT(dq != NULL && ab != NULL);
    q16_t sin_t, cos_t;
    q16_sincos_fast(theta, &sin_t, &cos_t);

    ab->alpha = q16_mul(dq->d, cos_t) - q16_mul(dq->q, sin_t);
    ab->beta = q16_mul(dq->d, sin_t) + q16_mul(dq->q, cos_t);
}

bool syn_foc_field_weakening(q16_t v_d, q16_t v_q, q16_t v_max, q16_t *id_cmd)
{
    SYN_ASSERT(id_cmd != NULL && v_max > 0);
    q16_t v_mag = q16_hypot(v_d, v_q);
    if (v_mag > v_max) {
        q16_t v_err = v_mag - v_max;
        /* Ingest negative d-axis current proportional to voltage excess */
        *id_cmd -= q16_mul(v_err, Q16_FROM_FRAC(1, 2));
        return true;
    }
    return false;
}

/* ── SVPWM ──────────────────────────────────────────────────────────────── */

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

    q16_t v_offset = -(v_max + v_min) / 2;

    /* Use single reciprocal division for 3-phase PWM normalization */
    q16_t inv_v_bus = q16_inv(v_bus);
    *duty_a = q16_clamp(q16_mul(va + v_offset, inv_v_bus) + Q16_HALF, 0, Q16_ONE);
    *duty_b = q16_clamp(q16_mul(vb + v_offset, inv_v_bus) + Q16_HALF, 0, Q16_ONE);
    *duty_c = q16_clamp(q16_mul(vc + v_offset, inv_v_bus) + Q16_HALF, 0, Q16_ONE);
}

static inline q16_t dt_sign_scale(q16_t current, q16_t dt_comp)
{
    q16_t thresh = Q16_FROM_FRAC(1, 10); /* 0.1A zero-crossing threshold */
    if (current >= thresh) {
        return dt_comp;
    }
    if (current <= -thresh) {
        return -dt_comp;
    }
    /* Smooth linear interpolation across zero-crossing band */
    return q16_mul(q16_div(current, thresh), dt_comp);
}

void syn_foc_deadtime_comp(q16_t *duty_a, q16_t *duty_b, q16_t *duty_c, const SYN_FOC_ABC *i_abc,
                           q16_t dt_comp)
{
    SYN_ASSERT(duty_a != NULL && duty_b != NULL && duty_c != NULL && i_abc != NULL);
    if (duty_a == NULL || duty_b == NULL || duty_c == NULL || i_abc == NULL || dt_comp == 0) {
        return;
    }

    *duty_a = q16_clamp(*duty_a + dt_sign_scale(i_abc->a, dt_comp), 0, Q16_ONE);
    *duty_b = q16_clamp(*duty_b + dt_sign_scale(i_abc->b, dt_comp), 0, Q16_ONE);
    *duty_c = q16_clamp(*duty_c + dt_sign_scale(i_abc->c, dt_comp), 0, Q16_ONE);
}

#endif /* SYN_USE_FOC */
```


