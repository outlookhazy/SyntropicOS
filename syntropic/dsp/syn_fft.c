#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FFT) || SYN_USE_FFT

/**
 * @file syn_fft.c
 * @brief Fixed-point Q16.16 Radix-2 Decimation-in-Time Fast Fourier Transform implementation.
 */

#include "syn_fft.h"
#include "../util/syn_assert.h"

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
 * @param k      Butterfly index within group.
 * @param stage  Current FFT stage.
 * @param wr     [out] Twiddle real (cosine).
 * @param wi     [out] Twiddle imaginary (-sine).
 */
static void get_twiddle(uint16_t k, uint16_t stage, q16_t *wr, q16_t *wi)
{
    /* Map phase angle to range [0..255] for standard N=256 lookup.
     * m = 1 << stage, so (256 * k) / m == (256 * k) >> stage. */
    uint16_t idx = (uint16_t)((256u * k) >> stage);
    uint16_t theta = idx & 0x3F;       /* idx % 64 */
    uint16_t quad  = (idx >> 6) & 0x03; /* idx / 64 */

    q16_t s, c;
    if (quad == 0) {
        s = g_sin_table[theta];
        c = g_sin_table[64 - theta];
    } else if (quad == 1) {
        s = g_sin_table[64 - theta];
        c = -g_sin_table[theta];
    } else if (quad == 2) {
        s = -g_sin_table[theta];
        c = -g_sin_table[64 - theta];
    } else {
        s = -g_sin_table[64 - theta];
        c = g_sin_table[theta];
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

#endif /* SYN_USE_FFT */
