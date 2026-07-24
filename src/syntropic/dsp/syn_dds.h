/**
 * @file syn_dds.h
 * @brief Direct Digital Synthesis (DDS) Waveform Synthesizer.
 * @ingroup syn_dsp
 *
 * Implements a zero-malloc Q16.16 fixed-point DDS signal generator supporting
 * Sine, Triangle, Sawtooth, Square/PWM, and Noise waveforms.
 */

#ifndef SYN_DDS_H
#define SYN_DDS_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DDS Waveform Types */
typedef enum {
    SYN_DDS_SINE     = 0, /**< Sine wave (via Taylor/Polynomial approximation) */
    SYN_DDS_TRIANGLE = 1, /**< Symmetric Triangle wave                        */
    SYN_DDS_SAWTOOTH = 2, /**< Ramp / Sawtooth wave                            */
    SYN_DDS_SQUARE   = 3, /**< Square / PWM wave                               */
    SYN_DDS_NOISE    = 4  /**< Pseudorandom White Noise                        */
} SYN_DDS_Waveform;

/**
 * @brief DDS Generator State Instance.
 */
typedef struct {
    SYN_DDS_Waveform type;         /**< Waveform type                          */
    uint32_t         phase;        /**< 32-bit Phase accumulator (0 to 2^32-1)  */
    uint32_t         phase_step;   /**< Phase increment per sample step        */
    q16_t            amplitude;    /**< Q16.16 Peak Amplitude (e.g. 1.0)       */
    q16_t            offset;       /**< Q16.16 DC Offset                       */
    uint16_t         duty_q16;     /**< PWM Duty cycle (0 to 65536 for 0-100%) */
    uint32_t         rand_state;   /**< PRNG state for noise generator         */
} SYN_DDS;

/**
 * @brief Initialize a DDS Generator instance.
 * @param dds Pointer to DDS instance.
 * @param type Waveform type.
 * @param freq_hz Target output frequency in Hz.
 * @param sample_rate_hz Sample rate in Hz.
 * @return SYN_OK on success, or SYN_INVALID_PARAM.
 */
SYN_Status syn_dds_init(SYN_DDS *dds, SYN_DDS_Waveform type, uint32_t freq_hz, uint32_t sample_rate_hz);

/**
 * @brief Set the output frequency of the DDS generator.
 * @param dds Pointer to DDS instance.
 * @param freq_hz Output frequency in Hz.
 * @param sample_rate_hz Sample rate in Hz.
 * @return SYN_OK on success.
 */
SYN_Status syn_dds_set_freq(SYN_DDS *dds, uint32_t freq_hz, uint32_t sample_rate_hz);

/**
 * @brief Set the peak amplitude and DC offset.
 * @param dds Pointer to DDS instance.
 * @param amplitude Q16.16 peak amplitude.
 * @param offset Q16.16 DC offset.
 * @return SYN_OK on success.
 */
SYN_Status syn_dds_set_gain(SYN_DDS *dds, q16_t amplitude, q16_t offset);

/**
 * @brief Set PWM duty cycle (for SYN_DDS_SQUARE mode).
 * @param dds Pointer to DDS instance.
 * @param duty_pct Duty cycle percentage (0.0 to 100.0%).
 * @return SYN_OK on success.
 */
SYN_Status syn_dds_set_duty(SYN_DDS *dds, float duty_pct);

/**
 * @brief Compute and advance the DDS generator by one sample step.
 * @param dds Pointer to DDS instance.
 * @return Generated sample in Q16.16 fixed-point format.
 */
q16_t syn_dds_step(SYN_DDS *dds);

/**
 * @brief Fill a Q16.16 buffer with consecutive DDS samples.
 * @param dds Pointer to DDS instance.
 * @param buf Output Q16.16 buffer.
 * @param count Number of samples to generate.
 * @return SYN_OK on success.
 */
SYN_Status syn_dds_fill_q16(SYN_DDS *dds, q16_t *buf, size_t count);

/**
 * @brief Fill a unsigned 16-bit integer buffer for DAC DMA output.
 * @param dds Pointer to DDS instance.
 * @param buf Output 16-bit DAC buffer.
 * @param count Number of samples.
 * @param dac_center Center DAC value (e.g. 2048 for 12-bit DAC).
 * @param dac_span Maximum peak-to-peak DAC range (e.g. 2047).
 * @return SYN_OK on success.
 */
SYN_Status syn_dds_fill_u16(SYN_DDS *dds, uint16_t *buf, size_t count, uint16_t dac_center, uint16_t dac_span);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DDS_H */
