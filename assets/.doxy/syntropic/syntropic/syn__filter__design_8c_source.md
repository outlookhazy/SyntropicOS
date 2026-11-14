

# File syn\_filter\_design.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter\_design.c**](syn__filter__design_8c.md)

[Go to the documentation of this file](syn__filter__design_8c.md)


```C++


#include "syn_filter_design.h"

#include <string.h>

static inline q16_t filter_design_div_a0(int64_t num, int64_t a0)
{
    return (q16_t)((num * (int64_t)Q16_ONE) / a0);
}

static SYN_Status compute_intermediate_vars(q16_t fc_hz, q16_t fs_hz, q16_t Q, q16_t *cos_w0,
                                            int64_t *alpha64, int64_t *a0)
{
    if (fc_hz <= 0 || fs_hz <= 0 || Q <= 0 || fc_hz >= (fs_hz >> 1)) {
        return SYN_INVALID_PARAM;
    }

    q16_t omega0 = (q16_t)(((int64_t)Q16_2_PI * (int64_t)fc_hz) / (int64_t)fs_hz);
    if (omega0 <= 0) {
        omega0 = 1;
    }
    q16_t sin_w0 = q16_sin(omega0);
    *cos_w0 = q16_cos(omega0);

    *alpha64 = ((int64_t)sin_w0 * (int64_t)Q16_ONE) / (2LL * (int64_t)Q);
    *a0 = (int64_t)Q16_ONE + *alpha64;

    return SYN_OK;
}

SYN_Status syn_filter_design_lowpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL)
        return SYN_INVALID_PARAM;

    q16_t cos_w0;
    int64_t alpha64, a0;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &alpha64, &a0);
    if (status != SYN_OK)
        return status;

    int64_t num = (int64_t)Q16_ONE - (int64_t)cos_w0;
    out->b0 = filter_design_div_a0(num >> 1, a0);
    out->b1 = filter_design_div_a0(num, a0);
    out->b2 = out->b0;
    out->a1 = filter_design_div_a0(-2LL * (int64_t)cos_w0, a0);
    out->a2 = filter_design_div_a0((int64_t)Q16_ONE - alpha64, a0);

    return SYN_OK;
}

SYN_Status syn_filter_design_highpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL)
        return SYN_INVALID_PARAM;

    q16_t cos_w0;
    int64_t alpha64, a0;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &alpha64, &a0);
    if (status != SYN_OK)
        return status;

    int64_t num = (int64_t)Q16_ONE + (int64_t)cos_w0;
    out->b0 = filter_design_div_a0(num >> 1, a0);
    out->b1 = -filter_design_div_a0(num, a0);
    out->b2 = out->b0;
    out->a1 = filter_design_div_a0(-2LL * (int64_t)cos_w0, a0);
    out->a2 = filter_design_div_a0((int64_t)Q16_ONE - alpha64, a0);

    return SYN_OK;
}

SYN_Status syn_filter_design_bandpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL)
        return SYN_INVALID_PARAM;

    q16_t cos_w0;
    int64_t alpha64, a0;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &alpha64, &a0);
    if (status != SYN_OK)
        return status;

    out->b0 = filter_design_div_a0(alpha64, a0);
    out->b1 = 0;
    out->b2 = -out->b0;
    out->a1 = filter_design_div_a0(-2LL * (int64_t)cos_w0, a0);
    out->a2 = filter_design_div_a0((int64_t)Q16_ONE - alpha64, a0);

    return SYN_OK;
}

SYN_Status syn_filter_design_notch(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL)
        return SYN_INVALID_PARAM;

    q16_t cos_w0;
    int64_t alpha64, a0;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &alpha64, &a0);
    if (status != SYN_OK)
        return status;

    out->b0 = filter_design_div_a0((int64_t)Q16_ONE, a0);
    out->b1 = filter_design_div_a0(-2LL * (int64_t)cos_w0, a0);
    out->b2 = out->b0;
    out->a1 = out->b1;
    out->a2 = filter_design_div_a0((int64_t)Q16_ONE - alpha64, a0);

    return SYN_OK;
}

SYN_Status syn_filter_design_apply_to_biquad(const SYN_BiquadCoeffs *coeffs,
                                             SYN_FilterBiquad *biquad)
{
    if (coeffs == NULL || biquad == NULL)
        return SYN_INVALID_PARAM;

    biquad->b0 = coeffs->b0;
    biquad->b1 = coeffs->b1;
    biquad->b2 = coeffs->b2;
    biquad->a1 = coeffs->a1;
    biquad->a2 = coeffs->a2;
    biquad->x1 = 0;
    biquad->x2 = 0;
    biquad->y1 = 0;
    biquad->y2 = 0;

    return SYN_OK;
}
```


