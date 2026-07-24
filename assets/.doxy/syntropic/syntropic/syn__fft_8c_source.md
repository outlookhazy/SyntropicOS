

# File syn\_fft.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_fft.c**](syn__fft_8c.md)

[Go to the documentation of this file](syn__fft_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FFT) || SYN_USE_FFT

#include "syn_fft.h"
#include "../util/syn_assert.h"

/* Quarter-sine lookup table for N=256. Covers i=0..64 (0 to PI/2 radians). */
static const q16_t g_sin_table[65] = {
    0x00000000, 0x00000648, 0x00000C90, 0x000012D5, 0x00001918, 0x00001F56, 0x00002590, 0x00002BC4,
    0x000031F1, 0x00003817, 0x00003E34, 0x00004447, 0x00004A50, 0x0000504D, 0x0000563E, 0x00005C22,
    0x000061F8, 0x000067BE, 0x00006D74, 0x0000731A, 0x000078AD, 0x00007E2F, 0x0000839C, 0x000088F6,
    0x00008E3A, 0x00009368, 0x00009880, 0x00009D80, 0x0000A268, 0x0000A736, 0x0000ABEB, 0x0000B086,
    0x0000B505, 0x0000B968, 0x0000BDAF, 0x0000C1D8, 0x0000C5E4, 0x0000C9D1, 0x0000CD9F, 0x0000D14D,
    0x0000D4DB, 0x0000D848, 0x0000DB94, 0x0000DEBE, 0x0000E1C6, 0x0000E4AA, 0x0000E76C, 0x0000EA0A,
    0x0000EC83, 0x0000EED9, 0x0000F109, 0x0000F314, 0x0000F4FA, 0x0000F6BA, 0x0000F854, 0x0000F9C8,
    0x0000FB15, 0x0000FC3B, 0x0000FD3B, 0x0000FE13, 0x0000FEC4, 0x0000FF4E, 0x0000FFB1, 0x0000FFEC,
    0x00010000,
};

static uint16_t reverse_bits(uint16_t x, uint16_t stages)
{
    uint16_t rev = 0;
    for (uint16_t i = 0; i < stages; i++) {
        rev = (rev << 1) | (x & 1);
        x >>= 1;
    }
    return rev;
}

static void bit_reverse_sort(q16_t *real, q16_t *imag, uint16_t n, uint16_t stages)
{
    for (uint16_t i = 0; i < n; i++) {
        uint16_t rev = reverse_bits(i, stages);
        if (i < rev) {
            /* Swap real */
            q16_t temp = real[i];
            real[i] = real[rev];
            real[rev] = temp;
            /* Swap imaginary */
            temp = imag[i];
            imag[i] = imag[rev];
            imag[rev] = temp;
        }
    }
}

static void get_twiddle(uint16_t k, uint16_t stage, q16_t *wr, q16_t *wi)
{
    uint16_t idx = (uint16_t)((256u * k) >> stage);
    uint16_t theta = idx & 0x3F;       /* idx % 64 */
    uint16_t quad  = (idx >> 6) & 0x03; /* idx / 64 */

    SYN_ASSERT(quad <= 1); /* k < m/2 guarantees idx < 128 */

    q16_t s, c;
    if (quad == 0) {
        s = g_sin_table[theta];
        c = g_sin_table[64 - theta];
    } else {
        s = g_sin_table[64 - theta];
        c = -g_sin_table[theta];
    }

    *wr = c;
    *wi = -s; /* e^(-i*theta) = cos(theta) - i*sin(theta) */
}


SYN_Status syn_dsp_fft(q16_t *real, q16_t *imag, uint16_t n)
{
    SYN_ASSERT(real != NULL);
    SYN_ASSERT(imag != NULL);

    /* Validate n: must be a power of 2, 8 <= n <= 256 */
    if (n < 2 || n > 256 || (n & (n - 1)) != 0) {
        return SYN_ERROR;
    }

    /* Compute stages = log2(n) */
    uint16_t stages = 0;
    uint16_t temp = n;
    while (temp > 1) {
        temp >>= 1;
        stages++;
    }

    /* 1. Bit-reversal sorting */
    bit_reverse_sort(real, imag, n, stages);

    /* 2. Cooley-Tukey Radix-2 butterflies */
    for (uint16_t stage = 1; stage <= stages; stage++) {
        uint16_t m = 1 << stage;
        uint16_t m2 = m >> 1;

        for (uint16_t k = 0; k < m2; k++) {
            q16_t wr, wi;
            get_twiddle(k, stage, &wr, &wi);

            for (uint16_t j = 0; j < n; j += m) {
                uint16_t i = j + k;
                uint16_t ip = i + m2;

                /* complex multiply: t = w * input[ip] */
                q16_t tr = q16_mul(wr, real[ip]) - q16_mul(wi, imag[ip]);
                q16_t ti = q16_mul(wr, imag[ip]) + q16_mul(wi, real[ip]);

                /* input[ip] = input[i] - t */
                real[ip] = real[i] - tr;
                imag[ip] = imag[i] - ti;

                /* input[i] = input[i] + t */
                real[i] = real[i] + tr;
                imag[i] = imag[i] + ti;
            }
        }
    }

    return SYN_OK;
}

SYN_Status syn_fft_window_hanning(q16_t *out, uint16_t n)
{
    if (out == NULL || n <= 1) return SYN_INVALID_PARAM;

    q16_t two_pi = Q16_2_PI;
    q16_t n_minus_1 = Q16_FROM_INT(n - 1);

    for (uint16_t i = 0; i < n; i++) {
        q16_t theta = q16_div(q16_mul(two_pi, Q16_FROM_INT(i)), n_minus_1);
        q16_t cos_t = q16_cos(theta);
        out[i] = q16_mul(Q16_HALF, Q16_ONE - cos_t);
    }
    return SYN_OK;
}

SYN_Status syn_fft_window_hamming(q16_t *out, uint16_t n)
{
    if (out == NULL || n <= 1) return SYN_INVALID_PARAM;

    q16_t two_pi = Q16_2_PI;
    q16_t n_minus_1 = Q16_FROM_INT(n - 1);
    q16_t a0 = Q16_FROM_FRAC(54, 100);
    q16_t a1 = Q16_FROM_FRAC(46, 100);

    for (uint16_t i = 0; i < n; i++) {
        q16_t theta = q16_div(q16_mul(two_pi, Q16_FROM_INT(i)), n_minus_1);
        q16_t cos_t = q16_cos(theta);
        out[i] = a0 - q16_mul(a1, cos_t);
    }
    return SYN_OK;
}

SYN_Status syn_fft_window_blackman(q16_t *out, uint16_t n)
{
    if (out == NULL || n <= 1) return SYN_INVALID_PARAM;

    q16_t two_pi = Q16_2_PI;
    q16_t n_minus_1 = Q16_FROM_INT(n - 1);
    q16_t a0 = Q16_FROM_FRAC(42, 100);
    q16_t a1 = Q16_HALF;
    q16_t a2 = Q16_FROM_FRAC(8, 100);

    for (uint16_t i = 0; i < n; i++) {
        q16_t theta = q16_div(q16_mul(two_pi, Q16_FROM_INT(i)), n_minus_1);
        q16_t cos_t1 = q16_cos(theta);
        q16_t cos_t2 = q16_cos(q16_mul(Q16_FROM_INT(2), theta));
        out[i] = a0 - q16_mul(a1, cos_t1) + q16_mul(a2, cos_t2);
    }
    return SYN_OK;
}

SYN_Status syn_fft_magnitude_spectrum(const q16_t *real, const q16_t *imag, q16_t *mag, uint16_t n)
{
    if (real == NULL || imag == NULL || mag == NULL || n == 0) return SYN_INVALID_PARAM;

    uint16_t n_bins = (n >> 1) + 1;
    for (uint16_t k = 0; k < n_bins; k++) {
        mag[k] = q16_hypot(real[k], imag[k]);
    }
    return SYN_OK;
}

SYN_Status syn_fft_find_peaks(const q16_t *mag, uint16_t n_bins, q16_t sample_rate_hz,
                              SYN_FFTPeak *peaks, uint8_t max_peaks, uint8_t *num_peaks_found)
{
    if (mag == NULL || peaks == NULL || num_peaks_found == NULL || n_bins <= 2 || max_peaks == 0) {
        return SYN_INVALID_PARAM;
    }

    uint8_t count = 0;
    q16_t bin_resolution = q16_div(sample_rate_hz, Q16_FROM_INT((n_bins - 1) * 2));

    for (uint16_t k = 1; k < n_bins - 1 && count < max_peaks; k++) {
        if (mag[k] > mag[k - 1] && mag[k] > mag[k + 1]) {
            peaks[count].bin       = k;
            peaks[count].magnitude = mag[k];
            peaks[count].freq_hz   = q16_mul(Q16_FROM_INT(k), bin_resolution);
            count++;
        }
    }

    /* Sort detected peaks descending by magnitude */
    for (uint8_t i = 0; i < count; i++) {
        for (uint8_t j = i + 1; j < count; j++) {
            if (peaks[j].magnitude > peaks[i].magnitude) {
                SYN_FFTPeak tmp = peaks[i];
                peaks[i] = peaks[j];
                peaks[j] = tmp;
            }
        }
    }

    *num_peaks_found = count;
    return SYN_OK;
}

SYN_Status syn_fft_thd(const q16_t *mag, uint16_t n_bins, uint16_t fundamental_bin,
                       uint8_t max_harmonics, q16_t *thd_pct)
{
    if (mag == NULL || thd_pct == NULL || fundamental_bin == 0 || fundamental_bin >= n_bins || max_harmonics == 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t v1 = mag[fundamental_bin];
    if (v1 == 0) return SYN_INVALID_PARAM;

    int64_t sum_h2 = 0;
    for (uint8_t h = 2; h <= max_harmonics + 1; h++) {
        uint32_t h_bin = (uint32_t)fundamental_bin * h;
        if (h_bin < n_bins) {
            q16_t v_h = mag[h_bin];
            int64_t v_h_q16 = v_h;
            sum_h2 += (v_h_q16 * v_h_q16) >> Q16_SHIFT;
        }
    }

    q16_t harmonic_rms = q16_sqrt((q16_t)sum_h2);
    /* THD % = (V_harmonics / V_1) * 100 */
    q16_t ratio = q16_div(harmonic_rms, v1);
    *thd_pct = q16_mul(ratio, Q16_FROM_INT(100));

    return SYN_OK;
}

#endif /* SYN_USE_FFT */
```


