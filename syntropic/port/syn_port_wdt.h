/**
 * @file syn_port_wdt.h
 * @brief Platform port: Hardware Watchdog Timer (WDT).
 *
 * The hardware watchdog resets the MCU if not fed within the timeout
 * window — even if the CPU is hard-locked (unlike the software watchdog
 * in syn_watchdog.h which requires the scheduler to keep running).
 * @ingroup syn_system
 */

#ifndef SYN_PORT_WDT_H
#define SYN_PORT_WDT_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure and start the hardware watchdog.
 *
 * Once started, the watchdog cannot be stopped without a reset on most
 * MCUs.  Call syn_port_wdt_feed() regularly within the timeout window.
 *
 * @param timeout_ms  Desired watchdog timeout in milliseconds.
 *                    The port will round to the nearest supported value.
 * @return SYN_OK on success, SYN_ERROR if the WDT cannot be started.
 */
SYN_Status syn_port_wdt_init(uint32_t timeout_ms);

/**
 * @brief Feed (pet) the hardware watchdog, resetting its counter.
 *
 * Must be called more frequently than the timeout configured in
 * syn_port_wdt_init() to prevent a hardware reset.
 */
void syn_port_wdt_feed(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_WDT_H */
