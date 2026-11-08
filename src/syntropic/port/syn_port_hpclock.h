/**
 * @file syn_port_hpclock.h
 * @brief High-precision clock port interface — implement for your platform.
 *
 * Provides the hardware timer counter register and overflow counter
 * that the syn_hpclock driver uses to construct 64-bit system-clock-
 * precision timestamps.
 *
 * ## Platform implementation checklist
 *
 *   1. Configure a free-running hardware timer at the system clock rate
 *      (or as fast as practical).  For example, STM32 TIM2 in free-run
 *      mode, or AVR Timer1 with no prescaler.
 *
 *   2. Define `syn_port_hpclock_lsb_ptr()` to return a pointer to the
 *      timer's count register.
 *
 *   3. Enable the timer overflow interrupt and call
 *      `SYN_HPCLOCK_OVERFLOW_TICK()` from the ISR.
 *
 *   4. Define `syn_port_hpclock_freq_hz()` to return the timer's clock
 *      frequency.
 *
 *   5. **The overflow ISR MUST have the highest interrupt priority in the
 *      system.**  The resolve algorithm assumes the overflow ISR can
 *      preempt any context that calls SYN_HPCLOCK_CAPTURE().  If a
 *      higher-priority ISR captures a timestamp while the overflow ISR
 *      is pended, the resolved tick will be silently wrong by one full
 *      counter period.  There is no runtime detection of this error.
 *
 * @par Example — STM32 TIM2 at 84 MHz
 * @code
 *   // In your platform port file:
 *   volatile uint32_t *syn_port_hpclock_lsb_ptr(void) {
 *       return &TIM2->CNT;
 *   }
 *
 *   uint32_t syn_port_hpclock_freq_hz(void) {
 *       return 84000000UL;
 *   }
 *
 *   // Priority 0 = highest on Cortex-M NVIC
 *   void TIM2_IRQHandler(void) {
 *       SYN_HPCLOCK_OVERFLOW_TICK();
 *       LL_TIM_ClearFlag_UPDATE(TIM2);
 *   }
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_PORT_HPCLOCK_H
#define SYN_PORT_HPCLOCK_H

#include "../common/syn_defs.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Overflow counter ──────────────────────────────────────────────────── */

/**
 * @brief Global overflow counter — incremented by the platform ISR.
 *
 * Declared here so both the port ISR and the driver can access it.
 * Defined in syn_hpclock.c.
 */
extern volatile uint32_t syn_hpclock_msb;

/**
 * @brief Macro for the platform overflow ISR — one line, nothing else.
 *
 * Call this from your timer update/overflow interrupt handler.
 * Do not add any other logic before this macro in the ISR.
 */
#define SYN_HPCLOCK_OVERFLOW_TICK() (syn_hpclock_msb++)

/* ── Port functions ────────────────────────────────────────────────────── */

/**
 * @brief Return a pointer to the hardware timer count register.
 *
 * The returned pointer must be volatile and point to a memory-mapped
 * register that increments at the rate returned by
 * syn_port_hpclock_freq_hz().
 *
 * @return Pointer to the timer counter register.
 */
volatile uint32_t *syn_port_hpclock_lsb_ptr(void);

/**
 * @brief Return the timer clock frequency in Hz.
 *
 * Used for tick-to-nanosecond conversion.  Must be a compile-time
 * constant or a value that does not change after initialization.
 *
 * @return Timer frequency in Hz (e.g. 16000000 for 16 MHz).
 */
uint32_t syn_port_hpclock_freq_hz(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_HPCLOCK_H */
