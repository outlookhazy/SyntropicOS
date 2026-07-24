/**
 * @file syn_cia401.h
 * @brief CANopen CiA 401 Device Profile for Generic I/O Modules.
 *
 * Implements CiA 401 Object Dictionary definitions and helper binding state
 * for 8-bit digital inputs/outputs and 16-bit analog inputs/outputs.
 */

#ifndef SYN_CIA401_H
#define SYN_CIA401_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CiA 401 Standard Object Dictionary Indices */
#define SYN_CIA401_OD_DIGITAL_INPUT_8BIT   0x6000U /**< Read 8-bit Digital Inputs */
#define SYN_CIA401_OD_DIGITAL_OUTPUT_8BIT  0x6200U /**< Write 8-bit Digital Outputs */
#define SYN_CIA401_OD_ANALOG_INPUT_16BIT   0x6401U /**< Read 16-bit Analog Inputs */
#define SYN_CIA401_OD_ANALOG_OUTPUT_16BIT  0x6411U /**< Write 16-bit Analog Outputs */
#define SYN_CIA401_OD_ANALOG_INPUT_DELTA   0x6421U /**< Analog Input Delta Interrupt Threshold */
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_L 0x6424U /**< Analog Input Low Limit Alarm */
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_H 0x6425U /**< Analog Input High Limit Alarm */

/** @brief Maximum channel capacities for CiA 401 instance */
#define SYN_CIA401_MAX_DIGITAL_8BIT 4
#define SYN_CIA401_MAX_ANALOG_16BIT  8

/**
 * @brief CiA 401 Generic I/O Device State Instance.
 */
typedef struct {
    uint8_t  digital_in[SYN_CIA401_MAX_DIGITAL_8BIT];   /**< Digital input bytes (channels 1..4) */
    uint8_t  digital_out[SYN_CIA401_MAX_DIGITAL_8BIT];  /**< Digital output bytes (channels 1..4) */
    int16_t  analog_in[SYN_CIA401_MAX_ANALOG_16BIT];    /**< Analog input 16-bit values */
    int16_t  analog_out[SYN_CIA401_MAX_ANALOG_16BIT];   /**< Analog output 16-bit values */
    int16_t  analog_delta[SYN_CIA401_MAX_ANALOG_16BIT]; /**< Change threshold for event PDO */
    int16_t  prev_analog_in[SYN_CIA401_MAX_ANALOG_16BIT]; /**< Saved previous analog values */
    uint8_t  digital_in_count;                          /**< Configured digital input byte count */
    uint8_t  digital_out_count;                         /**< Configured digital output byte count */
    uint8_t  analog_in_count;                           /**< Configured analog input channel count */
    uint8_t  analog_out_count;                          /**< Configured analog output channel count */
} SYN_CiA401_Device;

/**
 * @brief Initialize a CiA 401 device state instance.
 * @param dev Pointer to CiA 401 device handle.
 * @param dig_in_bytes Number of 8-bit digital input blocks (1..4).
 * @param dig_out_bytes Number of 8-bit digital output blocks (1..4).
 * @param analog_in_ch Number of 16-bit analog input channels (1..8).
 * @param analog_out_ch Number of 16-bit analog output channels (1..8).
 */
void syn_cia401_init(SYN_CiA401_Device *dev, uint8_t dig_in_bytes, uint8_t dig_out_bytes,
                     uint8_t analog_in_ch, uint8_t analog_out_ch);

/**
 * @brief Populate a standard CANopen Object Dictionary array with CiA 401 entries.
 * @param dev Pointer to initialized CiA 401 device instance.
 * @param od_table Output array of SYN_CANOpenODEntry elements.
 * @param max_entries Capacity of od_table array.
 * @return Number of OD entries populated.
 */
size_t syn_cia401_populate_od(SYN_CiA401_Device *dev, SYN_CANOpenODEntry *od_table, size_t max_entries);

/**
 * @brief Check if any analog input has exceeded its delta threshold for event-driven TPDO.
 * @param dev Pointer to CiA 401 device instance.
 * @param channel Output channel index (0..analog_in_count-1) that changed.
 * @return True if a change occurred exceeding delta.
 */
bool syn_cia401_check_analog_delta(SYN_CiA401_Device *dev, uint8_t *channel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA401_H */
