/**
 * @file syn_filter_design.c
 * @brief Dynamic runtime IIR Biquad filter coefficient generator implementation.
 */

#include "syn_filter_design.h"

#include <string.h>

/**
 * @brief Divide a 64-bit Q16.16 numerator by positive 64-bit Q16.16 a0 into Q16.16.
 * @param num Numerator in Q16.16.
 * @param a0  Positive denominator a0 (>= Q16_ONE) in Q16.16.
 * @return Normalized Q16.16 coefficient.
 */
static inline q16_t filter_design_div_a0(int64_t num, int64_t a0)
{
    return (q16_t)((num * (int64_t)Q16_ONE) / a0);
}

/**
 * @brief Pre-warp frequency and compute 64-bit intermediate bilinear transform constants.
 * @param fc_hz   Cutoff frequency in Hz (Q16.16).
 * @param fs_hz   Sampling frequency in Hz (Q16.16).
 * @param Q       Quality factor (Q16.16).
 * @param cos_w0  Output cos(w0) in Q16.16.
 * @param alpha64 Output alpha in 64-bit Q16.16.
 * @param a0      Output (1 + alpha) in 64-bit Q16.16.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid limits.
 */
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
