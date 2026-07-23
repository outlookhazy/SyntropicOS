/**
 * @file syn_filter_design.c
 * @brief Dynamic runtime IIR Biquad filter coefficient generator implementation.
 */

#include "syn_filter_design.h"
#include <string.h>

/**
 * @brief Helper to pre-warp frequency and compute intermediate bilinear transform constants.
 * @param fc_hz  Cutoff frequency in Hz.
 * @param fs_hz  Sampling frequency in Hz.
 * @param Q      Quality factor.
 * @param cos_w0 Output cos(w0).
 * @param sin_w0 Output sin(w0).
 * @param alpha  Output alpha.
 * @param a0_inv Output 1/a0 scale factor.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid limits.
 */
static SYN_Status compute_intermediate_vars(q16_t fc_hz, q16_t fs_hz, q16_t Q,
                                             q16_t *cos_w0, q16_t *sin_w0,
                                             q16_t *alpha, q16_t *a0_inv)
{
    if (fc_hz <= 0 || fs_hz <= 0 || Q <= 0 || fc_hz >= (fs_hz >> 1)) {
        return SYN_INVALID_PARAM;
    }

    q16_t omega0 = q16_mul(Q16_2_PI, q16_div(fc_hz, fs_hz));
    *sin_w0 = q16_sin(omega0);
    *cos_w0 = q16_cos(omega0);

    *alpha = q16_div(*sin_w0, q16_mul(Q16_FROM_INT(2), Q));
    q16_t a0 = Q16_ONE + *alpha;
    *a0_inv = q16_div(Q16_ONE, a0);

    return SYN_OK;
}

SYN_Status syn_filter_design_lowpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL) return SYN_INVALID_PARAM;

    q16_t cos_w0, sin_w0, alpha, a0_inv;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &sin_w0, &alpha, &a0_inv);
    if (status != SYN_OK) return status;

    q16_t num = Q16_ONE - cos_w0;
    out->b0 = q16_mul(num >> 1, a0_inv);
    out->b1 = q16_mul(num, a0_inv);
    out->b2 = out->b0;
    out->a1 = q16_mul(q16_mul(Q16_FROM_INT(-2), cos_w0), a0_inv);
    out->a2 = q16_mul(Q16_ONE - alpha, a0_inv);

    return SYN_OK;
}

SYN_Status syn_filter_design_highpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL) return SYN_INVALID_PARAM;

    q16_t cos_w0, sin_w0, alpha, a0_inv;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &sin_w0, &alpha, &a0_inv);
    if (status != SYN_OK) return status;

    q16_t num = Q16_ONE + cos_w0;
    out->b0 = q16_mul(num >> 1, a0_inv);
    out->b1 = -q16_mul(num, a0_inv);
    out->b2 = out->b0;
    out->a1 = q16_mul(q16_mul(Q16_FROM_INT(-2), cos_w0), a0_inv);
    out->a2 = q16_mul(Q16_ONE - alpha, a0_inv);

    return SYN_OK;
}

SYN_Status syn_filter_design_bandpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL) return SYN_INVALID_PARAM;

    q16_t cos_w0, sin_w0, alpha, a0_inv;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &sin_w0, &alpha, &a0_inv);
    if (status != SYN_OK) return status;

    out->b0 = q16_mul(alpha, a0_inv);
    out->b1 = 0;
    out->b2 = -out->b0;
    out->a1 = q16_mul(q16_mul(Q16_FROM_INT(-2), cos_w0), a0_inv);
    out->a2 = q16_mul(Q16_ONE - alpha, a0_inv);

    return SYN_OK;
}

SYN_Status syn_filter_design_notch(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out)
{
    if (out == NULL) return SYN_INVALID_PARAM;

    q16_t cos_w0, sin_w0, alpha, a0_inv;
    SYN_Status status = compute_intermediate_vars(fc_hz, fs_hz, Q, &cos_w0, &sin_w0, &alpha, &a0_inv);
    if (status != SYN_OK) return status;

    out->b0 = a0_inv;
    out->b1 = q16_mul(q16_mul(Q16_FROM_INT(-2), cos_w0), a0_inv);
    out->b2 = out->b0;
    out->a1 = out->b1;
    out->a2 = q16_mul(Q16_ONE - alpha, a0_inv);

    return SYN_OK;
}

SYN_Status syn_filter_design_apply_to_biquad(const SYN_BiquadCoeffs *coeffs, SYN_FilterBiquad *biquad)
{
    if (coeffs == NULL || biquad == NULL) return SYN_INVALID_PARAM;

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
