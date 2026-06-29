/**
 * @file syn_port_system.h
 * @brief System-level port interface — functions the user must implement.
 *
 * Provides critical-section management, a millisecond tick source, delay,
 * and system reset. These are required by several SyntropicOS modules.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_SYSTEM_H
#define SYN_PORT_SYSTEM_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enter a critical section (disable interrupts).
 *
 * Calls may be nested; the implementation must track nesting depth and
 * only re-enable interrupts when the outermost critical section exits.
 */
void syn_port_enter_critical(void);

/**
 * @brief Exit a critical section (re-enable interrupts).
 *
 * Must be called once for each corresponding syn_port_enter_critical().
 */
void syn_port_exit_critical(void);

/**
 * @brief Return the current system tick in milliseconds.
 *
 * This value must be monotonically increasing and should wrap
 * naturally at UINT32_MAX. Typical sources: SysTick, a hardware
 * timer, or an RTOS tick.
 *
 * @return Milliseconds since system start (or last wrap).
 */
uint32_t syn_port_get_tick_ms(void);

/**
 * @brief Blocking delay for the specified number of milliseconds.
 *
 * The implementation may busy-wait or yield to an RTOS. The delay
 * must be at least @p ms milliseconds.
 *
 * @param ms  Number of milliseconds to delay.
 */
void syn_port_delay_ms(uint32_t ms);

/**
 * @brief Perform a system reset.
 *
 * This function should not return. On Cortex-M, this is typically
 * NVIC_SystemReset().
 */
SYN_NORETURN void syn_port_system_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SYSTEM_H */
