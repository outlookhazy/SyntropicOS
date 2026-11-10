#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_FFT) || SYN_USE_FFT

/**
 * @file syn_fft.c
 * @brief Fixed-point Q16.16 Radix-2 Decimation-in-Time Fast Fourier Transform implementation.
 */

#include "../util/syn_assert.h"
#include "syn_fft.h"

/* Quarter-sine lookup table for N=256. Covers i=0..64 (0 to PI/2 radians). */
/** @brief Quarter-sine lookup table for N=256 FFT (Q16 fixed-point). */
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

/**
 * @brief Reverse the low bits of a value.
 * @param x       Value to reverse.
 * @param stages  Number of bits.
 * @return Bit-reversed value.
 */
static uint16_t reverse_bits(uint16_t x, uint16_t stages)
{
    uint16_t rev = 0;
    for (uint16_t i = 0; i < stages; i++) {
        rev = (rev << 1) | (x & 1);
        x >>= 1;
    }
    return rev;
}

/**
 * @brief Reorder samples by bit-reversed index.
 * @param real    Real components.
 * @param imag    Imaginary components.
 * @param n       Number of samples.
 * @param stages  log2(n).
 */
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

/**
 * @brief Compute twiddle factor (cos, -sin) for a butterfly stage.
 * @param k      Butterfly index within group (0 <= k < m/2).
 * @param stage  Current FFT stage (1-based).
 * @param wr     [out] Twiddle real (cosine).
 * @param wi     [out] Twiddle imaginary (-sine).
 *
 * The Cooley-Tukey butterfly calls this with k < 2^(stage-1), so
 * idx = (256*k) >> stage is always < 128. Only quadrants 0 and 1
 * of the sine table are reachable.
 */
static void get_twiddle(uint16_t k, uint16_t stage, q16_t *wr, q16_t *wi)
{
    uint16_t idx = (uint16_t)((256u * k) >> stage);
    uint16_t theta = idx & 0x3F;       /* idx % 64 */
    uint16_t quad = (idx >> 6) & 0x03; /* idx / 64 */

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

/**
 * @brief Validate FFT length n and compute log2(n) stages.
 * @param n FFT length.
 * @return log2(n) in [1, 8] if n is a power of 2 in [2, 256], else 0.
 */
static uint16_t fft_log2_stages(uint16_t n)
{
    if (n < 2U || n > 256U || (n & (uint16_t)(n - 1U)) != 0U) {
        return 0U;
    }
    uint16_t stages = 0U;
    uint16_t temp = n;
    while (temp > 1U) {
        temp >>= 1U;
        stages++;
    }
    return stages;
}

/**
 * @brief Execute a single Cooley-Tukey Radix-2 butterfly stage in place.
 * @param real  Real sample array (size n).
 * @param imag  Imaginary sample array (size n).
 * @param n     FFT length.
 * @param stage 1-based stage index (1 .. log2(n)).
 */
static void fft_butterfly_stage(q16_t *real, q16_t *imag, uint16_t n, uint16_t stage)
{
    uint16_t m = (uint16_t)(1U << stage);
    uint16_t m2 = (uint16_t)(m >> 1U);

    for (uint16_t k = 0U; k < m2; k++) {
        q16_t wr, wi;
        get_twiddle(k, stage, &wr, &wi);

        for (uint16_t j = 0U; j < n; j = (uint16_t)(j + m)) {
            uint16_t i = (uint16_t)(j + k);
            uint16_t ip = (uint16_t)(i + m2);

            q16_t tr = q16_mul(wr, real[ip]) - q16_mul(wi, imag[ip]);
            q16_t ti = q16_mul(wr, imag[ip]) + q16_mul(wi, real[ip]);

            real[ip] = real[i] - tr;
            imag[ip] = imag[i] - ti;
            real[i] = real[i] + tr;
            imag[i] = imag[i] + ti;
        }
    }
}

SYN_Status syn_dsp_fft(q16_t *real, q16_t *imag, uint16_t n)
{
    SYN_ASSERT(real != NULL);
    SYN_ASSERT(imag != NULL);

    uint16_t stages = fft_log2_stages(n);
    if (stages == 0U) {
        return SYN_ERROR;
    }

    bit_reverse_sort(real, imag, n, stages);

    for (uint16_t stage = 1U; stage <= stages; stage++) {
        fft_butterfly_stage(real, imag, n, stage);
    }

    return SYN_OK;
}

SYN_PT_Status syn_dsp_fft_pt(SYN_PT *pt, q16_t *real, q16_t *imag, uint16_t n,
                             uint16_t *current_stage)
{
    if (pt == NULL || real == NULL || imag == NULL || current_stage == NULL) {
        return PT_EXITED;
    }

    uint16_t stages = fft_log2_stages(n);
    if (stages == 0U) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    *current_stage = 1U;
    bit_reverse_sort(real, imag, n, stages);
    PT_CONTINUE(pt);

    while (*current_stage <= stages) {
        fft_butterfly_stage(real, imag, n, *current_stage);
        (*current_stage)++;
        if (*current_stage <= stages) {
            PT_CONTINUE(pt);
        }
    }

    PT_END(pt);
}

/**
 * @brief Compute window phase angle 2*pi*i/(n-1) in Q16.16 without 32-bit overflow.
 * @param i Sample index in [0, n - 1].
 * @param n Window length (must be > 1).
 * @return Phase angle in [0, Q16_2_PI].
 */
static inline q16_t fft_window_angle(uint16_t i, uint16_t n)
{
    return (q16_t)(((int64_t)Q16_2_PI * (int64_t)i) / (int64_t)((uint32_t)n - 1U));
}

SYN_Status syn_fft_window_hanning(q16_t *out, uint16_t n)
{
    if ((out == NULL) || (n <= 1U)) {
        return SYN_INVALID_PARAM;
    }

    for (uint16_t i = 0U; i < n; i++) {
        q16_t theta = fft_window_angle(i, n);
        q16_t cos_t = q16_cos_fast(theta);
        out[i] = q16_mul(Q16_HALF, Q16_ONE - cos_t);
    }
    return SYN_OK;
}

SYN_Status syn_fft_window_hamming(q16_t *out, uint16_t n)
{
    if ((out == NULL) || (n <= 1U)) {
        return SYN_INVALID_PARAM;
    }

    q16_t a0 = Q16_FROM_FRAC(54, 100);
    q16_t a1 = Q16_FROM_FRAC(46, 100);

    for (uint16_t i = 0U; i < n; i++) {
        q16_t theta = fft_window_angle(i, n);
        q16_t cos_t = q16_cos_fast(theta);
        out[i] = a0 - q16_mul(a1, cos_t);
    }
    return SYN_OK;
}

SYN_Status syn_fft_window_blackman(q16_t *out, uint16_t n)
{
    if ((out == NULL) || (n <= 1U)) {
        return SYN_INVALID_PARAM;
    }

    q16_t a0 = Q16_FROM_FRAC(42, 100);
    q16_t a1 = Q16_FROM_FRAC(50, 100);
    q16_t a2 = Q16_FROM_FRAC(8, 100);

    for (uint16_t i = 0U; i < n; i++) {
        q16_t theta = fft_window_angle(i, n);
        q16_t cos_t1 = q16_cos_fast(theta);
        q16_t cos_t2 = q16_cos_fast((q16_t)((int64_t)theta * 2LL));
        out[i] = a0 - q16_mul(a1, cos_t1) + q16_mul(a2, cos_t2);
    }
    return SYN_OK;
}

SYN_Status syn_fft_magnitude_spectrum(const q16_t *real, const q16_t *imag, q16_t *mag, uint16_t n)
{
    if (real == NULL || imag == NULL || mag == NULL || n == 0)
        return SYN_INVALID_PARAM;

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

    uint8_t count = 0U;
    uint32_t fft_size = ((uint32_t)n_bins - 1U) * 2U;

    for (uint16_t k = 1U; (k < (uint16_t)(n_bins - 1U)) && (count < max_peaks); k++) {
        if ((mag[k] > mag[k - 1U]) && (mag[k] > mag[k + 1U])) {
            peaks[count].bin = k;
            peaks[count].magnitude = mag[k];
            peaks[count].freq_hz =
                (q16_t)(((int64_t)sample_rate_hz * (int64_t)k) / (int64_t)fft_size);
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
    if (mag == NULL || thd_pct == NULL || fundamental_bin == 0 || fundamental_bin >= n_bins ||
        max_harmonics == 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t v1 = mag[fundamental_bin];
    if (v1 == 0)
        return SYN_INVALID_PARAM;

    int64_t sum_h2 = 0;
    for (uint16_t h = 2U; h <= (uint16_t)max_harmonics + 1U; h++) {
        uint32_t h_bin = (uint32_t)fundamental_bin * (uint32_t)h;
        if (h_bin < n_bins) {
            q16_t v_h = mag[h_bin];
            int64_t v_h_q16 = v_h;
            sum_h2 += (v_h_q16 * v_h_q16) >> Q16_SHIFT;
        }
    }

    q16_t sum_q16 = (sum_h2 > (int64_t)INT32_MAX) ? INT32_MAX : (q16_t)sum_h2;
    q16_t harmonic_rms = q16_sqrt(sum_q16);
    /* THD % = (V_harmonics / V_1) * 100 */
    q16_t ratio = q16_div(harmonic_rms, v1);
    *thd_pct = q16_mul(ratio, Q16_FROM_INT(100));

    return SYN_OK;
}

#endif /* SYN_USE_FFT */
