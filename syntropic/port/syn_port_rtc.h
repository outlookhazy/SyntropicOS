/**
 * @file syn_port_rtc.h
 * @brief Platform port: Real-Time Clock (RTC).
 *
 * Implementors provide these three functions for the target hardware RTC.
 * On host (tests) they are backed by mock_port.c.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_RTC_H
#define SYN_PORT_RTC_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calendar date and time representation.
 *
 * All fields are in natural units (year = 4-digit, month 1-12, etc.).
 */
typedef struct {
    uint16_t year;    /**< Full year, e.g. 2025              */
    uint8_t  month;   /**< Month: 1 = January, 12 = December */
    uint8_t  day;     /**< Day of month: 1 – 31              */
    uint8_t  hour;    /**< Hour: 0 – 23                      */
    uint8_t  minute;  /**< Minute: 0 – 59                    */
    uint8_t  second;  /**< Second: 0 – 59                    */
} SYN_RTC_DateTime;

/**
 * @brief Initialize the RTC peripheral.
 * @return SYN_OK on success, SYN_ERROR if the peripheral cannot be started.
 */
SYN_Status syn_port_rtc_init(void);

/**
 * @brief Read the current date/time from the hardware RTC.
 * @param dt  Output parameter, filled by the port.  Must not be NULL.
 * @return SYN_OK on success, SYN_ERROR on read failure.
 */
SYN_Status syn_port_rtc_get(SYN_RTC_DateTime *dt);

/**
 * @brief Set the hardware RTC to the given date/time.
 * @param dt  New time to program.  Must not be NULL.
 * @return SYN_OK on success, SYN_ERROR on write failure.
 */
SYN_Status syn_port_rtc_set(const SYN_RTC_DateTime *dt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_RTC_H */
