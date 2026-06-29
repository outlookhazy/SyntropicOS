/**
 * @file syn_hwwdt.h
 * @brief Hardware Watchdog Timer driver.
 *
 * Wraps syn_port_wdt.h with a minimal driver interface.  The hardware
 * watchdog runs independently of the CPU and will reset the system if
 * syn_hwwdt_feed() is not called within the configured timeout — even
 * if the CPU is hard-locked.
 *
 * Complementary to syn_watchdog.h (software task-stall detector).
 *
 * Usage:
 * @code
 *   syn_hwwdt_init(2000);     // 2-second hardware timeout
 *   // In main loop or scheduler tick:
 *   syn_hwwdt_feed();
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_HWWDT_H
#define SYN_HWWDT_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the hardware watchdog with the given timeout.
 *
 * On most MCUs the watchdog cannot be disabled once started.
 * syn_hwwdt_feed() must be called more frequently than @p timeout_ms.
 *
 * @param timeout_ms  Desired timeout in milliseconds.
 * @return SYN_OK on success, SYN_ERROR if the hardware WDT cannot be started.
 */
SYN_Status syn_hwwdt_init(uint32_t timeout_ms);

/**
 * @brief Feed (reload) the hardware watchdog counter.
 *
 * Call this function from your main loop or scheduler idle hook to
 * prevent the hardware from resetting the system.
 */
void syn_hwwdt_feed(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HWWDT_H */
