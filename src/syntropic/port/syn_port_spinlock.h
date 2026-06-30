/**
 * @file syn_port_spinlock.h
 * @brief Multicore spinlock and core-identity port functions.
 *
 * Implement these for your platform to enable AMP multicore support.
 * On RP2040, map to the hardware spinlock peripheral. On other MCUs,
 * implement using compare-and-swap (CAS) or test-and-set.
 *
 * @par Spinlock contract
 *
 * - `acquire()` MUST disable interrupts on the calling core before
 *   spinning. This prevents a deadlock where an ISR on the same core
 *   tries to acquire a lock held by the interrupted main context.
 *
 * - `release()` MUST restore the interrupt state to what it was
 *   before the matching `acquire()`.
 *
 * - Spinlocks are NOT recursive. Acquiring a lock you already hold
 *   is undefined behavior (deadlock on most implementations).
 *
 * - Spinlocks should be held for the shortest possible time. They
 *   are intended for protecting hardware registers and short
 *   critical sections — NOT for guarding long computations.
 *
 * @par Usage
 * @code
 *   // Protect shared UART across two cores
 *   syn_port_spinlock_acquire(SYN_SPINLOCK_UART);
 *   syn_port_uart_transmit(0, data, len, 10);
 *   syn_port_spinlock_release(SYN_SPINLOCK_UART);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_PORT_SPINLOCK_H
#define SYN_PORT_SPINLOCK_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Spinlock configuration ────────────────────────────────────────────── */

/** @brief Number of spinlock IDs available. Override in syn_config.h. */
#ifndef SYN_SPINLOCK_COUNT
  #define SYN_SPINLOCK_COUNT  4
#endif

/* ── Port functions (user implements) ──────────────────────────────────── */

/**
 * @brief Acquire a spinlock (blocking).
 *
 * Disables interrupts on the calling core, then spins until the lock
 * is available. Must NOT be called from ISR context.
 *
 * @param id  Spinlock index (0 .. SYN_SPINLOCK_COUNT-1).
 */
void syn_port_spinlock_acquire(uint8_t id);

/**
 * @brief Release a spinlock.
 *
 * Releases the lock and restores interrupts to the state prior to
 * the matching acquire().
 *
 * @param id  Spinlock index.
 */
void syn_port_spinlock_release(uint8_t id);

/**
 * @brief Try to acquire a spinlock (non-blocking).
 *
 * If the lock is free, acquires it (with interrupts disabled) and
 * returns true. If contended, returns false without spinning.
 *
 * @param id  Spinlock index.
 * @return true if the lock was acquired.
 */
bool syn_port_spinlock_try_acquire(uint8_t id);

/**
 * @brief Return the current CPU core index.
 * @return 0 for the primary core, 1 for the secondary, etc.
 */
uint8_t syn_port_core_id(void);

/**
 * @brief Send an inter-core notification.
 *
 * Wakes the other core from WFE / low-power idle. On ARM Cortex-M,
 * this is typically a SEV instruction. On platforms without hardware
 * IPC, this may be a no-op (the other core will notice at its next
 * scheduler tick).
 */
void syn_port_ipc_notify(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MULTICORE */

#endif /* SYN_PORT_SPINLOCK_H */
