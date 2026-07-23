/**
 * @file syn_filter_design.h
 * @brief Dynamic runtime IIR Biquad filter coefficient generator (Q16.16 fixed-point).
 */

#ifndef SYN_FILTER_DESIGN_H
#define SYN_FILTER_DESIGN_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"
#include "syn_biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Normalized Biquad filter coefficients (where a0 = 1.0). */
typedef struct {
    q16_t b0; /**< Numerator x[n] coefficient     */
    q16_t b1; /**< Numerator x[n-1] coefficient   */
    q16_t b2; /**< Numerator x[n-2] coefficient   */
    q16_t a1; /**< Denominator y[n-1] coefficient */
    q16_t a2; /**< Denominator y[n-2] coefficient */
} SYN_BiquadCoeffs;

/**
 * @brief Calculate Butterworth Lowpass filter coefficients.
 * @param fc_hz Cutoff frequency in Hz (Q16.16).
 * @param fs_hz Sampling frequency in Hz (Q16.16).
 * @param Q     Quality factor (Q16.16, e.g., Q16_FROM_FLOAT(0.7071) for Butterworth).
 * @param out   Pointer to output coefficient structure.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_filter_design_lowpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

/**
 * @brief Calculate Butterworth Highpass filter coefficients.
 * @param fc_hz Cutoff frequency in Hz (Q16.16).
 * @param fs_hz Sampling frequency in Hz (Q16.16).
 * @param Q     Quality factor (Q16.16, e.g., Q16_FROM_FLOAT(0.7071) for Butterworth).
 * @param out   Pointer to output coefficient structure.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_filter_design_highpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

/**
 * @brief Calculate Bandpass filter coefficients (constant skirt gain).
 * @param fc_hz Center frequency in Hz (Q16.16).
 * @param fs_hz Sampling frequency in Hz (Q16.16).
 * @param Q     Quality factor / bandwidth selectivity (Q16.16, e.g., Q16_FROM_FLOAT(2.0)).
 * @param out   Pointer to output coefficient structure.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_filter_design_bandpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

/**
 * @brief Calculate Notch / Bandstop filter coefficients.
 * @param fc_hz Rejection frequency in Hz (Q16.16).
 * @param fs_hz Sampling frequency in Hz (Q16.16).
 * @param Q     Quality factor / notch depth selectivity (Q16.16, e.g., Q16_FROM_FLOAT(5.0)).
 * @param out   Pointer to output coefficient structure.
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_filter_design_notch(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

/**
 * @brief Apply calculated coefficients to a SYN_FilterBiquad instance.
 * @param coeffs Calculated biquad coefficients.
 * @param biquad Target biquad filter instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_filter_design_apply_to_biquad(const SYN_BiquadCoeffs *coeffs, SYN_FilterBiquad *biquad);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FILTER_DESIGN_H */
