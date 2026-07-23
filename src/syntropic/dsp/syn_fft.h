/**
 * @file syn_fft.h
 * @brief Fixed-point Q16.16 Fast Fourier Transform (FFT).
 * @ingroup syn_dsp
 */

#ifndef SYN_FFT_H
#define SYN_FFT_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Computes the in-place Radix-2 Decimation-in-Time FFT.
 *
 * n must be a power of 2 (e.g., 8, 16, 32, 64, 128, 256). Max supported
 * size is 256.
 *
 * @param real Array of real parts (size n).
 * @param imag Array of imaginary parts (size n).
 * @param n    Size of the FFT (must be power of 2).
 * @return SYN_OK on success, SYN_ERROR on invalid parameters (e.g. not power of 2).
 */
SYN_Status syn_dsp_fft(q16_t *real, q16_t *imag, uint16_t n);

/**
 * @brief Generate Hanning window: w[i] = 0.5 * (1 - cos(2*pi*i / (n-1))).
 * @param out Output buffer of size n.
 * @param n   Window length.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL or n <= 1.
 */
SYN_Status syn_fft_window_hanning(q16_t *out, uint16_t n);

/**
 * @brief Generate Hamming window: w[i] = 0.54 - 0.46 * cos(2*pi*i / (n-1)).
 * @param out Output buffer of size n.
 * @param n   Window length.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL or n <= 1.
 */
SYN_Status syn_fft_window_hamming(q16_t *out, uint16_t n);

/**
 * @brief Generate Blackman-Harris window: w[i] = 0.42 - 0.5*cos(2*pi*i/(n-1)) + 0.08*cos(4*pi*i/(n-1)).
 * @param out Output buffer of size n.
 * @param n   Window length.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL or n <= 1.
 */
SYN_Status syn_fft_window_blackman(q16_t *out, uint16_t n);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FFT_H */
