

# File syn\_biquad.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_biquad.c**](syn__biquad_8c.md)

[Go to the documentation of this file](syn__biquad_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BIQUAD) || SYN_USE_BIQUAD

#include "../util/syn_assert.h"
#include "syn_biquad.h"

#include <stddef.h>
#include <string.h>

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
    if (f == NULL)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    f->x1 = 0;
    f->x2 = 0;
    f->y1 = 0;
    f->y2 = 0;
}

q16_t syn_filter_biquad_update(SYN_FilterBiquad *f, q16_t sample)
{
    SYN_ASSERT(f != NULL);

    /* y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2] */
    int64_t acc = ((int64_t)f->b0 * sample) + ((int64_t)f->b1 * f->x1) + ((int64_t)f->b2 * f->x2) -
                  ((int64_t)f->a1 * f->y1) - ((int64_t)f->a2 * f->y2);

    /* Shift back to Q16.16 */
    q16_t output = (q16_t)(acc >> 16);

    /* Update history */
    f->x2 = f->x1;
    f->x1 = sample;
    f->y2 = f->y1;
    f->y1 = output;

    return output;
}

void syn_filter_biquad_process_block(SYN_FilterBiquad *f, const q16_t *in, q16_t *out,
                                     uint16_t count)
{
    SYN_ASSERT(f != NULL && in != NULL && out != NULL);
    q16_t b0 = f->b0, b1 = f->b1, b2 = f->b2;
    q16_t a1 = f->a1, a2 = f->a2;
    q16_t x1 = f->x1, x2 = f->x2;
    q16_t y1 = f->y1, y2 = f->y2;

    for (uint16_t i = 0; i < count; i++) {
        q16_t sample = in[i];
        int64_t acc = ((int64_t)b0 * sample) + ((int64_t)b1 * x1) + ((int64_t)b2 * x2) -
                      ((int64_t)a1 * y1) - ((int64_t)a2 * y2);
        q16_t output = (q16_t)(acc >> 16);
        x2 = x1;
        x1 = sample;
        y2 = y1;
        y1 = output;
        out[i] = output;
    }

    f->x1 = x1;
    f->x2 = x2;
    f->y1 = y1;
    f->y2 = y2;
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

void syn_filter_biquad_highpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);

    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);
    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    /* Butterworth Q = 1/sqrt(2) */
    q16_t q_val = 46341; /* 0.70710678 in Q16.16 */
    q16_t alpha = q16_mul(sin_w, q_val);

    q16_t a0 = Q16_ONE + alpha;
    q16_t one_plus_cos = Q16_ONE + cos_w;
    q16_t b0 = q16_div(one_plus_cos, Q16_FROM_INT(2));
    q16_t b1 = -(Q16_ONE + cos_w);
    q16_t b2 = b0;
    q16_t a1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t a2 = Q16_ONE - alpha;

    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

void syn_filter_biquad_bandpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);
    SYN_ASSERT(q > 0);

    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);
    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    /* alpha = sin(w) / (2*Q) */
    q16_t alpha = q16_div(sin_w, q16_mul(Q16_FROM_INT(2), q));

    q16_t a0 = Q16_ONE + alpha;
    q16_t b0 = alpha; /* Peak gain = 1.0 */
    q16_t b1 = 0;
    q16_t b2 = -alpha;
    q16_t a1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t a2 = Q16_ONE - alpha;

    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

void syn_filter_biquad_notch(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);
    SYN_ASSERT(q > 0);

    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);
    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    /* alpha = sin(w) / (2*Q) */
    q16_t alpha = q16_div(sin_w, q16_mul(Q16_FROM_INT(2), q));

    q16_t a0 = Q16_ONE + alpha;
    q16_t b0 = Q16_ONE;
    q16_t b1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t b2 = Q16_ONE;
    q16_t a1 = b1; /* Same as b1 for notch */
    q16_t a2 = Q16_ONE - alpha;

    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

void syn_filter_biquad_allpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);
    SYN_ASSERT(q > 0);

    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);
    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    q16_t alpha = q16_div(sin_w, q16_mul(Q16_FROM_INT(2), q));

    q16_t a0 = Q16_ONE + alpha;
    q16_t b0 = Q16_ONE - alpha;
    q16_t b1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t b2 = Q16_ONE + alpha;
    q16_t a1 = b1;
    q16_t a2 = Q16_ONE - alpha;

    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

void syn_filter_biquad_peaking_eq(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t gain_db, q16_t q)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(fs > 0);
    SYN_ASSERT(q > 0);

    q16_t omega = q16_div(q16_mul(Q16_2_PI, fc), fs);
    q16_t sin_w = q16_sin(omega);
    q16_t cos_w = q16_cos(omega);

    /* A = 10^(gain_db / 40) ≈ 1 + gain_db / 35 (fixed-point linear approximation) */
    q16_t a_gain = Q16_ONE + q16_div(gain_db, Q16_FROM_INT(35));
    if (a_gain < Q16_FROM_FRAC(1, 10)) {
        a_gain = Q16_FROM_FRAC(1, 10);
    }

    q16_t alpha = q16_div(sin_w, q16_mul(Q16_FROM_INT(2), q));
    q16_t alpha_a = q16_mul(alpha, a_gain);
    q16_t alpha_div_a = q16_div(alpha, a_gain);

    q16_t a0 = Q16_ONE + alpha_div_a;
    q16_t b0 = Q16_ONE + alpha_a;
    q16_t b1 = -q16_mul(Q16_FROM_INT(2), cos_w);
    q16_t b2 = Q16_ONE - alpha_a;
    q16_t a1 = b1;
    q16_t a2 = Q16_ONE - alpha_div_a;

    f->b0 = q16_div(b0, a0);
    f->b1 = q16_div(b1, a0);
    f->b2 = q16_div(b2, a0);
    f->a1 = q16_div(a1, a0);
    f->a2 = q16_div(a2, a0);

    syn_filter_biquad_reset(f);
}

/* ── Cascaded Biquad Filter Bank ─────────────────────────────────────────── */

SYN_Status syn_filter_biquad_cascade_init(SYN_FilterBiquadCascade *c, uint8_t num_stages)
{
    SYN_ASSERT(c != NULL);
    if (num_stages == 0 || num_stages > SYN_BIQUAD_CASCADE_MAX_STAGES) {
        return SYN_INVALID_PARAM;
    }

    memset(c, 0, sizeof(*c));
    c->num_stages = num_stages;

    for (uint8_t i = 0; i < num_stages; i++) {
        c->stages[i].b0 = Q16_ONE; /* Passthrough by default */
    }

    return SYN_OK;
}

void syn_filter_biquad_cascade_reset(SYN_FilterBiquadCascade *c)
{
    SYN_ASSERT(c != NULL);
    for (uint8_t i = 0; i < c->num_stages; i++) {
        syn_filter_biquad_reset(&c->stages[i]);
    }
}

q16_t syn_filter_biquad_cascade_update(SYN_FilterBiquadCascade *c, q16_t sample)
{
    SYN_ASSERT(c != NULL);
    q16_t s = sample;
    for (uint8_t i = 0; i < c->num_stages; i++) {
        s = syn_filter_biquad_update(&c->stages[i], s);
    }
    return s;
}

void syn_filter_biquad_cascade_process_block(SYN_FilterBiquadCascade *c, const q16_t *in,
                                             q16_t *out, uint16_t count)
{
    SYN_ASSERT(c != NULL);
    SYN_ASSERT(in != NULL);
    SYN_ASSERT(out != NULL);

    for (uint16_t n = 0; n < count; n++) {
        out[n] = syn_filter_biquad_cascade_update(c, in[n]);
    }
}

#endif /* SYN_USE_BIQUAD */
```


