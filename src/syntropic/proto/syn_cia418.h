/**
 * @file syn_cia418.h
 * @brief CANopen CiA 418 Device Profile Object Dictionary Definitions.
 * @ingroup syn_protocol
 *
 * Defines standard CANopen CiA 418 Object Dictionary indices (0x6000-0x6009)
 * for Battery & Energy Storage Systems.
 */

#ifndef SYN_CIA418_H
#define SYN_CIA418_H

#include <stdint.h>

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

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA418_H */
