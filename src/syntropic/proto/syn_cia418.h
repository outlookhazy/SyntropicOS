/**
 * @file syn_cia418.h
 * @brief CANopen CiA 418 Device Profile for Batteries & Energy Management.
 * @ingroup syn_protocol
 *
 * Implements CANopen CiA 418 Object Dictionary entries (0x6000-0x6009) for
 * Battery Management Systems (BMS), including SOC, SOH, voltage, current,
 * temperature, time-to-go, and cell min/max extremes.
 */

#ifndef SYN_CIA418_H
#define SYN_CIA418_H

#include "../common/syn_defs.h"
#include "syn_canopen.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup cia418_od CiA 418 Object Dictionary Indices
 *  @{ */
#define SYN_CIA418_OD_SOC                 0x6000U /**< State of Charge in 0.1% (0..1000) */
#define SYN_CIA418_OD_SOH                 0x6001U /**< State of Health in 0.1% (0..1000) */
#define SYN_CIA418_OD_VOLTAGE             0x6002U /**< Battery Voltage in mV */
#define SYN_CIA418_OD_CURRENT             0x6003U /**< Battery Current in mA (signed) */
#define SYN_CIA418_OD_TEMPERATURE         0x6004U /**< Battery Temp in 0.1 °C (signed) */
#define SYN_CIA418_OD_TIME_TO_GO          0x6005U /**< Time Remaining in seconds */
#define SYN_CIA418_OD_MIN_CELL_VOLTAGE    0x6006U /**< Minimum Cell Voltage in mV */
#define SYN_CIA418_OD_MAX_CELL_VOLTAGE    0x6007U /**< Maximum Cell Voltage in mV */
#define SYN_CIA418_OD_MIN_CELL_TEMP       0x6008U /**< Minimum Cell Temp in 0.1 °C */
#define SYN_CIA418_OD_MAX_CELL_TEMP       0x6009U /**< Maximum Cell Temp in 0.1 °C */
/** @} */

/**
 * @brief CiA 418 Battery State Context.
 */
typedef struct {
    uint16_t soc_0_1pct;          /**< State of Charge (0..1000 = 0.0..100.0%) */
    uint16_t soh_0_1pct;          /**< State of Health (0..1000 = 0.0..100.0%) */
    uint32_t voltage_mv;          /**< Battery Voltage in mV */
    int32_t  current_ma;          /**< Battery Current in mA (positive=charging, negative=discharging) */
    int16_t  temperature_0_1c;    /**< Battery Temperature in 0.1 °C */
    uint32_t time_to_go_sec;      /**< Time-To-Go in seconds */
    uint16_t min_cell_voltage_mv; /**< Minimum Cell Voltage in mV */
    uint16_t max_cell_voltage_mv; /**< Maximum Cell Voltage in mV */
    int16_t  min_cell_temp_0_1c;  /**< Minimum Cell Temp in 0.1 °C */
    int16_t  max_cell_temp_0_1c;  /**< Maximum Cell Temp in 0.1 °C */
} SYN_CiA418_State;

/**
 * @brief Initialize a CiA 418 Battery State instance.
 * @param state Pointer to state structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_cia418_init(SYN_CiA418_State *state);

/**
 * @brief Handle SDO Read request for CiA 418 Object Dictionary indices (0x6000-0x6009).
 * @param state Pointer to state structure.
 * @param index OD Index (0x6000 to 0x6009).
 * @param subindex OD Subindex (typically 0).
 * @param out_data Output pointer receiving read bytes.
 * @param out_len Output pointer receiving length in bytes.
 * @return SYN_OK on success, or SYN_ERROR.
 */
SYN_Status syn_cia418_od_read(const SYN_CiA418_State *state, uint16_t index, uint8_t subindex, uint8_t *out_data, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA418_H */
