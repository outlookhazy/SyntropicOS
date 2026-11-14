

# File syn\_mfcc.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_mfcc.c**](syn__mfcc_8c.md)

[Go to the documentation of this file](syn__mfcc_8c.md)


```C++


#include "syn_mfcc.h"

#include <string.h>

SYN_Status syn_mfcc_init(SYN_MFCC *mfcc, uint16_t sample_rate_hz, uint16_t fft_size)
{
    if (mfcc == NULL) {
        return SYN_INVALID_PARAM;
    }
    if ((fft_size != 64U) && (fft_size != 128U)) {
        return SYN_INVALID_PARAM;
    }
    if (sample_rate_hz == 0U) {
        return SYN_INVALID_PARAM;
    }

    memset(mfcc, 0, sizeof(*mfcc));
    mfcc->sample_rate_hz = sample_rate_hz;
    mfcc->fft_size = fft_size;

    return SYN_OK;
}

static q16_t mfcc_log2_u64_q16(uint64_t x)
{
    uint32_t msb = 0U;
    uint64_t tmp = x;
    while (tmp > 1ULL) {
        tmp >>= 1U;
        msb++;
    }

    uint32_t m_q16 = (msb >= 16U) ? (uint32_t)(x >> (msb - 16U)) : (uint32_t)(x << (16U - msb));
    int64_t f = (int64_t)(m_q16 - 65536U);
    q16_t frac = (q16_t)((f * (93238LL - ((27702LL * f) >> 16))) >> 16);
    return Q16_FROM_INT((int32_t)msb) + frac;
}

static void mfcc_compute_mel_edges(uint16_t sample_rate_hz, uint16_t fft_size,
                                   q16_t edges[SYN_MFCC_NUM_MEL_FILTERS + 2U])
{
    q16_t x_max = Q16_ONE + (q16_t)(((int64_t)sample_rate_hz << (Q16_SHIFT - 1)) / 700LL);
    q16_t ln_max = q16_log(x_max);

    for (uint16_t p = 0U; p < (SYN_MFCC_NUM_MEL_FILTERS + 2U); p++) {
        q16_t ln_p =
            (q16_t)(((int64_t)ln_max * (int64_t)p) / (int64_t)(SYN_MFCC_NUM_MEL_FILTERS + 1U));
        q16_t exp_m1 = q16_exp(ln_p) - Q16_ONE;
        edges[p] = (q16_t)(((int64_t)exp_m1 * 700LL * (int64_t)fft_size) / (int64_t)sample_rate_hz);
    }
}

static uint64_t mfcc_integrate_band(const SYN_MFCC *mfcc, uint16_t num_bins, q16_t left,
                                    q16_t center, q16_t right)
{
    uint16_t k_start = (uint16_t)Q16_TO_INT(left);
    uint16_t k_right = (uint16_t)Q16_TO_INT(right);
    uint16_t k_end = (k_right < num_bins) ? (uint16_t)(k_right + 1U) : num_bins;

    uint64_t sum_power = 0ULL;
    bool hit_bin = false;
    for (uint16_t k = k_start; k < k_end; k++) {
        q16_t k_q16 = Q16_FROM_INT((int32_t)k);
        q16_t weight = 0;
        if ((k_q16 > left) && (k_q16 <= center) && (center > left)) {
            weight = (q16_t)(((int64_t)(k_q16 - left) << Q16_SHIFT) / (int64_t)(center - left));
        } else if ((k_q16 > center) && (k_q16 < right) && (right > center)) {
            weight = (q16_t)(((int64_t)(right - k_q16) << Q16_SHIFT) / (int64_t)(right - center));
        }
        if (weight > 0) {
            int64_t re = mfcc->fft_real[k];
            int64_t im = mfcc->fft_imag[k];
            uint64_t power = (uint64_t)(((re * re) + (im * im)) >> Q16_SHIFT);
            sum_power += (power * (uint64_t)weight) >> Q16_SHIFT;
            hit_bin = true;
        }
    }

    if (!hit_bin) {
        uint16_t k_round = (uint16_t)Q16_TO_INT_ROUND(center);
        uint16_t k_near = (k_round < num_bins) ? k_round : (uint16_t)(num_bins - 1U);
        int64_t re = mfcc->fft_real[k_near];
        int64_t im = mfcc->fft_imag[k_near];
        sum_power = (uint64_t)(((re * re) + (im * im)) >> Q16_SHIFT);
    }

    return sum_power;
}

SYN_Status syn_mfcc_process_frame(SYN_MFCC *mfcc, const int16_t *pcm_in,
                                  q7_t mfcc_out[SYN_MFCC_NUM_COEFFS])
{
    if ((mfcc == NULL) || (pcm_in == NULL) || (mfcc_out == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t N = mfcc->fft_size;

    /* 1. Windowing and Q16 conversion */
    for (uint16_t i = 0U; i < N; i++) {
        mfcc->fft_real[i] = (q16_t)((int32_t)pcm_in[i] * 2);
        mfcc->fft_imag[i] = 0;
    }

    /* 2. In-place FFT */
    if (syn_dsp_fft(mfcc->fft_real, mfcc->fft_imag, N) != SYN_OK) {
        return SYN_ERROR;
    }

    /* 3. Mel-warped power spectrum integration & log2 compression */
    uint16_t num_bins = N >> 1;
    q16_t mel_edges[SYN_MFCC_NUM_MEL_FILTERS + 2U];
    q7_t mel_input_q7[SYN_MFCC_NUM_MEL_FILTERS];
    mfcc_compute_mel_edges(mfcc->sample_rate_hz, N, mel_edges);

    for (uint16_t m = 0U; m < SYN_MFCC_NUM_MEL_FILTERS; m++) {
        uint64_t sum_power =
            mfcc_integrate_band(mfcc, num_bins, mel_edges[m], mel_edges[m + 1U], mel_edges[m + 2U]);
        q16_t log2_energy = mfcc_log2_u64_q16(1ULL + sum_power);
        mfcc->mel_energies[m] = log2_energy;

        int32_t log_val = (int32_t)(((int64_t)log2_energy * 9LL) >> 17);
        mel_input_q7[m] = (log_val > 127) ? (q7_t)127 : (q7_t)log_val;
    }

    /* 4. Discrete Cosine Transform (DCT-II) to extract 13 MFCC coefficients */
    if (syn_dsp_dct2_q7(mel_input_q7, SYN_MFCC_NUM_MEL_FILTERS, mfcc->mfcc_coeffs,
                        SYN_MFCC_NUM_COEFFS) != SYN_OK) {
        return SYN_ERROR; /* LCOV_EXCL_LINE: Defensive DCT check */
    }

    memcpy(mfcc_out, mfcc->mfcc_coeffs, SYN_MFCC_NUM_COEFFS * sizeof(q7_t));
    return SYN_OK;
}
```


