/**
 * @file syn_dsp.c
 * @brief Fixed-Point DSP implementation for Discrete Cosine Transform.
 */

#include "syn_dsp.h"

/**
 * @brief Scale Q16.16 value to Q7 with truncation toward zero and signed 8-bit clamping.
 * @param val_q16 Input Q16.16 fixed-point value.
 * @return Clamped Q7 output sample.
 */
static q7_t dsp_q16_to_q7_trunc(int64_t val_q16)
{
    int64_t scaled = val_q16 / 512LL;
    if (scaled > 127LL) {
        return (q7_t)127;
    }
    if (scaled < -128LL) {
        return (q7_t)-128;
    }
    return (q7_t)scaled;
}

SYN_Status syn_dsp_dct2_q7(const q7_t *time_series, size_t num_samples, q7_t *dct_coeffs,
                           size_t num_coeffs)
{
    if (!time_series || !dct_coeffs || num_samples == 0U || num_coeffs == 0U ||
        num_coeffs > num_samples) {
        return SYN_INVALID_PARAM;
    }

    uint64_t n_u64 = (uint64_t)num_samples;
    q16_t inv_n = (q16_t)(((uint64_t)Q16_ONE) / n_u64);
    q16_t two_inv_n = (q16_t)((2ULL * (uint64_t)Q16_ONE) / n_u64);
    q16_t scale_dc = q16_sqrt(inv_n);
    q16_t scale_ac = q16_sqrt(two_inv_n);
    uint64_t two_n = 2ULL * n_u64;
    uint64_t four_n = 4ULL * n_u64;

    for (size_t k = 0U; k < num_coeffs; k++) {
        int64_t acc_q16 = 0;
        q16_t scale = (k == 0U) ? scale_dc : scale_ac;
        uint64_t k_mod = (uint64_t)k % four_n;

        for (size_t n = 0U; n < num_samples; n++) {
            uint64_t step_mod = (2ULL * (uint64_t)n + 1ULL) % four_n;
            uint64_t phase_idx = (k_mod * step_mod) % four_n;
            q16_t angle = (q16_t)(((uint64_t)Q16_PI * phase_idx) / two_n);
            q16_t cos_val = q16_cos(angle);
            acc_q16 += ((int64_t)time_series[n] * (int64_t)cos_val) >> 7;
        }

        int64_t scaled_q16 = (acc_q16 * (int64_t)scale) >> Q16_SHIFT;
        dct_coeffs[k] = dsp_q16_to_q7_trunc(scaled_q16);
    }

    return SYN_OK;
}
