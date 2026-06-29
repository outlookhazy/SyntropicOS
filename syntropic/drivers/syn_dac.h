/**
 * @file syn_dac.h
 * @brief DAC (Digital-to-Analog Converter) driver.
 *
 * Thin wrapper over syn_port_dac.h, mirroring the pattern of syn_adc.h.
 * Provides raw-count, millivolt, and percent write helpers.
 *
 * Usage:
 * @code
 *   SYN_DAC dac;
 *   syn_dac_init(&dac, 0);           // channel 0
 *   syn_dac_write_mv(&dac, 1650);    // 1.65 V (half of 3.3 V ref)
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_DAC_H
#define SYN_DAC_H

#include <stdint.h>
#include "../common/syn_defs.h"
#include "../port/syn_port_dac.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DAC channel handle.  Caller allocates; zero heap. */
typedef struct {
    uint8_t channel;  /**< Platform DAC channel index */
} SYN_DAC;

/**
 * @brief Initialize a DAC channel.
 * @param dac      Handle to initialize.  Must not be NULL.
 * @param channel  Platform DAC channel index.
 * @return SYN_OK on success, SYN_ERROR on hardware failure.
 */
SYN_Status syn_dac_init(SYN_DAC *dac, uint8_t channel);

/**
 * @brief Write a raw count value to the DAC.
 * @param dac    Initialized DAC handle.
 * @param raw    Value in range [0, syn_dac_max_raw(dac)].
 * @return SYN_OK on success.
 */
SYN_Status syn_dac_write_raw(const SYN_DAC *dac, uint16_t raw);

/**
 * @brief Write a millivolt target to the DAC.
 *
 * Clamps to the reference voltage automatically.
 *
 * @param dac  Initialized DAC handle.
 * @param mv   Desired output voltage in millivolts.
 * @return SYN_OK on success.
 */
SYN_Status syn_dac_write_mv(const SYN_DAC *dac, uint16_t mv);

/**
 * @brief Write a percentage of full scale to the DAC.
 *
 * @param dac      Initialized DAC handle.
 * @param percent  0 = 0 V, 100 = full reference voltage.
 *                 Values > 100 are clamped to 100.
 * @return SYN_OK on success.
 */
SYN_Status syn_dac_write_percent(const SYN_DAC *dac, uint8_t percent);

/**
 * @brief Return the maximum raw value for this DAC (2^resolution - 1).
 * @param dac  Initialized DAC handle (channel field used to query port).
 * @return Maximum raw count (e.g. 4095 for 12-bit).
 */
uint16_t syn_dac_max_raw(const SYN_DAC *dac);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DAC_H */
