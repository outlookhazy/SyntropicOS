/**
 * @file syn_rtc.h
 * @brief RTC (Real-Time Clock) driver.
 *
 * Thin wrapper over syn_port_rtc.h that adds input validation and
 * Unix-epoch conversion utilities.  No heap allocation; no state struct
 * is required — calls delegate directly to the port layer.
 *
 * Epoch base: 1970-01-01 00:00:00 UTC (standard Unix time).
 * Valid date range: 1970-01-01 through 2105-12-31.
 *
 * Usage:
 * @code
 *   SYN_RTC_DateTime dt = { .year=2025, .month=6, .day=1,
 *                            .hour=12, .minute=0, .second=0 };
 *   syn_rtc_init();
 *   syn_rtc_set(&dt);
 *   // ... later ...
 *   syn_rtc_get(&dt);
 *   uint32_t t = syn_rtc_to_epoch(&dt);
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_RTC_H
#define SYN_RTC_H

#include "../port/syn_port_rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* syn_port_rtc.h re-exports SYN_RTC_DateTime; no need to redefine. */

/**
 * @brief Initialize the RTC peripheral.
 * @return SYN_OK on success, SYN_ERROR on failure.
 */
SYN_Status syn_rtc_init(void);

/**
 * @brief Read the current date/time.
 * @param dt  Output, must not be NULL.
 * @return SYN_OK on success, SYN_ERROR on hardware failure.
 */
SYN_Status syn_rtc_get(SYN_RTC_DateTime *dt);

/**
 * @brief Set the RTC to a new date/time.
 *
 * Validates all fields before writing to hardware.
 *
 * @param dt  New time.  Must not be NULL and must pass syn_rtc_is_valid().
 * @return SYN_OK on success, SYN_INVALID_PARAM if fields are out of range,
 *         SYN_ERROR on hardware failure.
 */
SYN_Status syn_rtc_set(const SYN_RTC_DateTime *dt);

/**
 * @brief Check whether all fields of a SYN_RTC_DateTime are in range.
 * @param dt  Date/time to validate.  Must not be NULL.
 * @return true if valid, false if any field is out of range.
 */
bool syn_rtc_is_valid(const SYN_RTC_DateTime *dt);

/**
 * @brief Convert a date/time to a Unix epoch timestamp.
 *
 * Epoch = seconds since 1970-01-01 00:00:00 UTC.  Leap seconds are
 * not accounted for (standard POSIX convention).
 *
 * @param dt  Date/time to convert.  Must be valid.
 * @return Seconds since Unix epoch.
 */
uint32_t syn_rtc_to_epoch(const SYN_RTC_DateTime *dt);

/**
 * @brief Convert a Unix epoch timestamp to a date/time.
 * @param epoch  Seconds since 1970-01-01 00:00:00 UTC.
 * @param dt     Output.  Must not be NULL.
 */
void syn_rtc_from_epoch(uint32_t epoch, SYN_RTC_DateTime *dt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RTC_H */
