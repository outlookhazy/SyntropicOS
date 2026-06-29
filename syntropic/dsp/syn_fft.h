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

#ifdef __cplusplus
}
#endif

#endif /* SYN_FFT_H */
