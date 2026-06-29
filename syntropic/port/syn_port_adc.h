/**
 * @file syn_port_adc.h
 * @brief ADC port interface — implement these for your platform.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_ADC_H
#define SYN_PORT_ADC_H

#include "../common/syn_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize an ADC channel.
 *
 * @param channel  ADC channel number.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_adc_init(uint8_t channel);

/**
 * @brief Read a single ADC sample.
 *
 * @param channel  ADC channel number.
 * @return Raw ADC value (resolution depends on platform).
 */
uint16_t syn_port_adc_read(uint8_t channel);

/**
 * @brief Get the ADC resolution in bits.
 *
 * @return Resolution (e.g., 10, 12, 16).
 */
uint8_t syn_port_adc_resolution(void);

/**
 * @brief Get the ADC reference voltage in millivolts.
 *
 * @return Reference voltage (e.g., 3300 for 3.3V).
 */
uint16_t syn_port_adc_reference_mv(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_ADC_H */
