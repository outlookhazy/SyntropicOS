/**
 * @file syn_biquad.h
 * @brief Fixed-point Q16.16 Biquad filter (Direct Form I).
 * @ingroup syn_dsp
 */

#ifndef SYN_BIQUAD_H
#define SYN_BIQUAD_H

#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixed-point Q16.16 Biquad filter state (Direct Form I).
 */
typedef struct {
    q16_t b0;  /**< Feedforward coefficient b0 (Q16.16)   */
    q16_t b1;  /**< Feedforward coefficient b1 (Q16.16)   */
    q16_t b2;  /**< Feedforward coefficient b2 (Q16.16)   */
    q16_t a1;  /**< Feedback coefficient a1 (Q16.16, a0 assumed 1.0) */
    q16_t a2;  /**< Feedback coefficient a2 (Q16.16)      */
    q16_t x1;  /**< Input delay line x[n-1]               */
    q16_t x2;  /**< Input delay line x[n-2]               */
    q16_t y1;  /**< Output delay line y[n-1]              */
    q16_t y2;  /**< Output delay line y[n-2]              */
} SYN_FilterBiquad;

/**
 * @brief Initialize a biquad filter with raw coefficients.
 * @param f   Filter instance.
 * @param b0  Feedforward coefficient b0 (Q16.16).
 * @param b1  Feedforward coefficient b1 (Q16.16).
 * @param b2  Feedforward coefficient b2 (Q16.16).
 * @param a1  Feedback coefficient a1 (Q16.16).
 * @param a2  Feedback coefficient a2 (Q16.16).
 */
void syn_filter_biquad_init(SYN_FilterBiquad *f, q16_t b0, q16_t b1, q16_t b2, q16_t a1, q16_t a2);

/**
 * @brief Reset biquad filter delay lines to zero.
 * @param f  Filter instance.
 */
void syn_filter_biquad_reset(SYN_FilterBiquad *f);

/**
 * @brief Process a single sample through the biquad filter.
 *
 * Uses 64-bit intermediate calculations to prevent overflows.
 *
 * @param f      Biquad filter instance.
 * @param sample Input sample in Q16.16.
 * @return Filtered output in Q16.16.
 */
q16_t syn_filter_biquad_update(SYN_FilterBiquad *f, q16_t sample);

/**
 * @brief Initialize a biquad lowpass filter.
 *
 * Computes standard Butterworth coefficients in Q16.16.
 *
 * @param f      Filter instance.
 * @param fc     Cutoff frequency (Hz) in Q16.16.
 * @param fs     Sample rate (Hz) in Q16.16.
 */
void syn_filter_biquad_lowpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BIQUAD_H */
