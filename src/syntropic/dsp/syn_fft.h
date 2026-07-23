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

/** @brief FFT peak information structure. */
typedef struct {
    uint16_t bin;          /**< FFT bin index */
    q16_t    magnitude;    /**< Peak magnitude in Q16.16 */
    q16_t    freq_hz;      /**< Peak frequency in Hz in Q16.16 */
} SYN_FFTPeak;

/**
 * @brief Compute magnitude spectrum: mag[k] = sqrt(real[k]^2 + imag[k]^2).
 * @param real Real part array (size n).
 * @param imag Imaginary part array (size n).
 * @param mag  Output magnitude array (size n/2 + 1).
 * @param n    FFT size (number of points).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_fft_magnitude_spectrum(const q16_t *real, const q16_t *imag, q16_t *mag, uint16_t n);

/**
 * @brief Find dominant peaks in magnitude spectrum.
 * @param mag               Magnitude array (size n_bins).
 * @param n_bins            Number of bins (typically n/2 + 1).
 * @param sample_rate_hz    Sampling rate in Hz (Q16.16).
 * @param peaks             Output peak array.
 * @param max_peaks         Capacity of peaks array.
 * @param num_peaks_found   Output count of detected peaks.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_fft_find_peaks(const q16_t *mag, uint16_t n_bins, q16_t sample_rate_hz,
                              SYN_FFTPeak *peaks, uint8_t max_peaks, uint8_t *num_peaks_found);

/**
 * @brief Compute Total Harmonic Distortion (THD) percentage.
 *
 * THD = sqrt(sum(V_h^2)) / V_1 * 100%
 *
 * @param mag              Magnitude array (size n_bins).
 * @param n_bins           Number of bins.
 * @param fundamental_bin  Bin index of the fundamental frequency.
 * @param max_harmonics    Number of harmonics to sum (e.g. 5).
 * @param thd_pct          Output THD percentage in Q16.16 (e.g., 5.2% = Q16_FROM_FLOAT(5.2)).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_fft_thd(const q16_t *mag, uint16_t n_bins, uint16_t fundamental_bin,
                       uint8_t max_harmonics, q16_t *thd_pct);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FFT_H */
