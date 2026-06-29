/**
 * @file syn_port_dac.h
 * @brief Platform port: Digital-to-Analog Converter (DAC).
 *
 * Mirrors syn_port_adc.h.  Implementors provide these four functions
 * for the target hardware DAC channels.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_DAC_H
#define SYN_PORT_DAC_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a DAC channel.
 * @param channel  Platform-specific channel index.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_dac_init(uint8_t channel);

/**
 * @brief Write a raw value to a DAC channel.
 * @param channel  Platform-specific channel index.
 * @param value    Raw counts (0 .. (1 << resolution) - 1).
 * @return SYN_OK on success.
 */
SYN_Status syn_port_dac_write(uint8_t channel, uint16_t value);

/**
 * @brief Return the DAC resolution in bits (e.g. 12 for a 12-bit DAC).
 * @return Resolution in bits.
 */
uint8_t syn_port_dac_resolution(void);

/**
 * @brief Return the DAC reference voltage in millivolts.
 * @return Reference voltage in mV.
 */
uint16_t syn_port_dac_reference_mv(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_DAC_H */
