#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BIQUAD) || SYN_USE_BIQUAD

/**
 * @file syn_biquad.c
 * @brief Fixed-point Q16.16 Biquad filter implementation.
 */

#include "syn_biquad.h"
#include "../util/syn_assert.h"
#include <stddef.h>


/* ── Biquad Filter API ──────────────────────────────────────────────────── */

void syn_filter_biquad_init(SYN_FilterBiquad *f, q16_t b0, q16_t b1, q16_t b2, q16_t a1, q16_t a2)
{
    SYN_ASSERT(f != NULL);
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
    f->a1 = a1;
    f->a2 = a2;
    syn_filter_biquad_reset(f);
}

void syn_filter_biquad_reset(SYN_FilterBiquad *f)
{
    SYN_ASSERT(f != NULL);
    f->x1 = 0;
    f->x2 = 0;
    f->y1 = 0;
    f->y2 = 0;
}

q16_t syn_filter_biquad_update(SYN_FilterBiquad *f, q16_t sample)
{
    SYN_ASSERT(f != NULL);

    /* y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2] */
    int64_t acc = ((int64_t)f->b0 * sample) +
                  ((int64_t)f->b1 * f->x1) +
                  ((int64_t)f->b2 * f->x2) -
                  ((int64_t)f->a1 * f->y1) -
                  ((int64_t)f->a2 * f->y2);

    /* Shift back to Q16.16 */
    q16_t output = (q16_t)(acc >> 16);

    /* Update history */
    f->x2 = f->x1;
    f->x1 = sample;
    f->y2 = f->y1;
    f->y1 = output;

    return output;
}

void syn_filter_biquad_lowpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);

    /* omega = 2 * PI * fc / fs */
    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);

    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    /* Butterworth Q = 1/sqrt(2) ≈ 0.707107 */
    q16_t q_val = 46341; /* 0.70710678 in Q16.16 */
    q16_t alpha = q16_mul(sin_w, q_val);

    q16_t a0 = Q16_ONE + alpha;
    q16_t b0 = q16_div(Q16_ONE - cos_w, Q16_FROM_INT(2));
    q16_t b1 = Q16_ONE - cos_w;
    q16_t b2 = b0;
    q16_t a1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t a2 = Q16_ONE - alpha;

    /* Normalize by a0 */
    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

#endif /* SYN_USE_BIQUAD */
