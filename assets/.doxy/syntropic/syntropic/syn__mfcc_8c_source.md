

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

SYN_Status syn_mfcc_process_frame(SYN_MFCC *mfcc, const int16_t *pcm_in,
                                  q7_t mfcc_out[SYN_MFCC_NUM_COEFFS])
{
    if ((mfcc == NULL) || (pcm_in == NULL) || (mfcc_out == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t N = mfcc->fft_size;

    /* 1. Windowing and Q16 conversion */
    for (uint16_t i = 0U; i < N; i++) {
        /* Convert int16 PCM to Q16.16 */
        mfcc->fft_real[i] = (q16_t)pcm_in[i] * 2;
        mfcc->fft_imag[i] = 0;
    }

    /* 2. In-place FFT */
    if (syn_dsp_fft(mfcc->fft_real, mfcc->fft_imag, N) != SYN_OK) {
        return SYN_ERROR;
    }

    /* 3. Power Spectrum calculation (magnitude squared) & Mel filterbank integration */
    uint16_t num_bins = N >> 1;
    q7_t mel_input_q7[SYN_MFCC_NUM_MEL_FILTERS];

    for (uint16_t m = 0U; m < SYN_MFCC_NUM_MEL_FILTERS; m++) {
        int64_t sum_power = 0;
        /* Triangular filter approximation over FFT power spectrum */
        uint16_t bin_start = (uint16_t)((m * num_bins) / (SYN_MFCC_NUM_MEL_FILTERS + 2U));
        uint16_t bin_end = (uint16_t)(((m + 2U) * num_bins) / (SYN_MFCC_NUM_MEL_FILTERS + 2U));

        for (uint16_t k = bin_start; k < bin_end && k < num_bins; k++) {
            int64_t re = mfcc->fft_real[k];
            int64_t im = mfcc->fft_imag[k];
            int64_t power = ((re * re) + (im * im)) >> Q16_SHIFT;
            sum_power += power;
        }

        /* Log compression to Q7 */
        int32_t log_val = (int32_t)(sum_power >> 8);
        if (log_val > 127) {
            log_val = 127;
        }
        mel_input_q7[m] = (q7_t)log_val;
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


