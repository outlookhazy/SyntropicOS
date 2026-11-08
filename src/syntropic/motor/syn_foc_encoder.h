/**
 * @file syn_foc_encoder.h
 * @brief Sensored FOC Rotor Position & Speed Interface (Encoder / Hall / Absolute).
 * @ingroup syn_motor
 */

#ifndef SYN_FOC_ENCODER_H
#define SYN_FOC_ENCODER_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Sensored Feedback Type */
typedef enum {
    SYN_FOC_ENCODER_QUADRATURE = 0, /**< ABZ Incremental Quadrature Encoder */
    SYN_FOC_ENCODER_HALL = 1,       /**< 3-Phase Digital Hall Sensors (120 deg) */
    SYN_FOC_ENCODER_ABSOLUTE = 2    /**< Absolute SPI / SSI Encoder */
} SYN_FOCEncoderType;

/** @brief Sensored Encoder Configuration */
typedef struct {
    SYN_FOCEncoderType type; /**< Sensored feedback type selection */
    uint8_t pole_pairs;      /**< Motor pole pair count (P) */
    uint32_t cpr;            /**< Counts Per Revolution (4 * pulses for QEI) */
    q16_t zero_offset_rad;   /**< Electrical zero position offset (Q16 rad) */
    uint32_t sample_rate_hz; /**< Position sampling frequency in Hz */
} SYN_FOCEncoderConfig;

/** @brief Sensored Encoder State */
typedef struct {
    SYN_FOCEncoderConfig config; /**< Encoder configuration parameters */
    int32_t count;               /**< Current raw pulse count */
    int32_t prev_count;          /**< Previous raw pulse count */
    q16_t elec_angle_rad;        /**< Calculated electrical angle (0 to 2*PI in Q16) */
    q16_t elec_speed_rad_s;      /**< Calculated electrical speed in Q16 rad/s */
    uint8_t hall_state;          /**< Hall sensor 3-bit state (1 to 6) */
} SYN_FOCEncoder;

/**
 * @brief Initialize sensored encoder module.
 * @param enc Pointer to encoder instance.
 * @param cfg Pointer to encoder configuration.
 * @return SYN_OK on success.
 */
SYN_Status syn_foc_encoder_init(SYN_FOCEncoder *enc, const SYN_FOCEncoderConfig *cfg);

/**
 * @brief Update position for ABZ Incremental Quadrature Encoder.
 * @param enc Pointer to encoder instance.
 * @param raw_count Current timer counter register (QEI count).
 */
void syn_foc_encoder_update_quadrature(SYN_FOCEncoder *enc, int32_t raw_count);

/**
 * @brief Update position for 3-Phase Digital Hall Sensors.
 * @param enc Pointer to encoder instance.
 * @param hall_u Digital pin U state (0 or 1).
 * @param hall_v Digital pin V state (0 or 1).
 * @param hall_w Digital pin W state (0 or 1).
 */
void syn_foc_encoder_update_hall(SYN_FOCEncoder *enc, bool hall_u, bool hall_v, bool hall_w);

/**
 * @brief Update position for Absolute Encoder.
 * @param enc Pointer to encoder instance.
 * @param raw_angle_14bit Raw 14-bit absolute angle (0..16383).
 */
void syn_foc_encoder_update_absolute(SYN_FOCEncoder *enc, uint16_t raw_angle_14bit);

/**
 * @brief Get current electrical angle in Q16 radians [0, 2*PI).
 * @param enc Pointer to encoder instance.
 * @return Current electrical angle in Q16 radians.
 */
q16_t syn_foc_encoder_get_elec_angle(const SYN_FOCEncoder *enc);

/**
 * @brief Get current electrical speed in Q16 rad/s.
 * @param enc Pointer to encoder instance.
 * @return Current electrical speed in Q16 rad/s.
 */
q16_t syn_foc_encoder_get_elec_speed(const SYN_FOCEncoder *enc);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_ENCODER_H */
