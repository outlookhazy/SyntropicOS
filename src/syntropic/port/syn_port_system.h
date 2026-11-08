/**
 * @file syn_port_system.h
 * @brief System-level port interface — functions the user must implement.
 *
 * Provides critical-section management, a millisecond tick source, delay,
 * interrupt priority management, and system reset.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_SYSTEM_H
#define SYN_PORT_SYSTEM_H

#if defined(__has_include) && __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enter a critical section (disable interrupts).
 */
void syn_port_enter_critical(void);

/**
 * @brief Exit a critical section (re-enable interrupts).
 */
void syn_port_exit_critical(void);

/**
 * @brief Return the current system tick in milliseconds.
 * @return Current system tick count in milliseconds.
 */
uint32_t syn_port_get_tick_ms(void);

/**
 * @brief Return the current system tick in microseconds.
 * @return Current system tick count in microseconds.
 */
uint32_t syn_port_get_tick_us(void);

/**
 * @brief Blocking delay for the specified number of milliseconds.
 * @param ms Delay duration in milliseconds.
 */
void syn_port_delay_ms(uint32_t ms);

/**
 * @brief Configure interrupt preemption and subpriority in hardware controller.
 *
 * @param irq_num      Interrupt vector number.
 * @param preempt_prio Preemption priority (0 = highest).
 * @param sub_prio     Subpriority level (0 = highest).
 */
void syn_port_nvic_set_priority(uint8_t irq_num, uint8_t preempt_prio, uint8_t sub_prio);

/**
 * @brief Enable a specific interrupt line in the hardware interrupt controller.
 *
 * @param irq_num Interrupt vector number to enable.
 */
void syn_port_nvic_enable_irq(uint8_t irq_num);

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS
/**
 * @brief Program low-power sleep mode until the designated target tick.
 *
 * @note The tickless scheduler invokes this function inside a critical section
 *       with interrupts masked. Implementations must wake on pending-but-masked
 *       interrupts (such as WFI on Cortex-M) or re-mask interrupts before
 *       returning so the caller exits its critical section cleanly.
 *
 * @param wake_tick_ms Target tick time in milliseconds to wake up.
 */
void syn_port_sleep_until(uint32_t wake_tick_ms);
#endif

/**
 * @brief Perform a system reset.
 */
SYN_NORETURN void syn_port_system_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SYSTEM_H */
