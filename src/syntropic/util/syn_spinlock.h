/**
 * @file syn_spinlock.h
 * @brief Scoped spinlock helper for shared resource protection.
 *
 * Convenience wrapper around the spinlock port functions. Provides
 * an RAII-style scoped lock that guarantees release on scope exit.
 *
 * @par Usage
 * @code
 *   // Protect a shared UART across two cores
 *   SYN_SPINLOCK_GUARD(SYN_SPINLOCK_UART) {
 *       syn_port_uart_transmit(0, data, len, 10);
 *   }
 * @endcode
 *
 * @par Well-known lock IDs
 * Define your own in syn_config.h or use the defaults:
 * - `SYN_SPINLOCK_UART`  (0) — Shared UART access
 * - `SYN_SPINLOCK_FLASH` (1) — Shared flash access
 * - `SYN_SPINLOCK_USER0` (2) — Application use
 * - `SYN_SPINLOCK_USER1` (3) — Application use
 *
 * @ingroup syn_core
 */

#ifndef SYN_SPINLOCK_H
#define SYN_SPINLOCK_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../port/syn_port_spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Well-known spinlock IDs ───────────────────────────────────────────── */

/** @brief Spinlock ID for shared UART peripheral access. */
#ifndef SYN_SPINLOCK_UART
  #define SYN_SPINLOCK_UART   0
#endif

/** @brief Spinlock ID for shared flash access. */
#ifndef SYN_SPINLOCK_FLASH
  #define SYN_SPINLOCK_FLASH  1
#endif

/** @brief Spinlock ID for application use. */
#ifndef SYN_SPINLOCK_USER0
  #define SYN_SPINLOCK_USER0  2
#endif

/** @brief Spinlock ID for application use. */
#ifndef SYN_SPINLOCK_USER1
  #define SYN_SPINLOCK_USER1  3
#endif

/* ── Scoped lock macro ─────────────────────────────────────────────────── */

/**
 * @brief Scoped spinlock guard.
 *
 * Acquires the spinlock at scope entry, releases at scope exit.
 * Uses a for-loop trick to guarantee release even on early return
 * or break statements within the guarded block.
 *
 * @param id  Spinlock index (0 .. SYN_SPINLOCK_COUNT-1).
 *
 * @code
 *   SYN_SPINLOCK_GUARD(SYN_SPINLOCK_FLASH) {
 *       syn_port_flash_write(addr, data, len);
 *   }
 * @endcode
 */
#define SYN_SPINLOCK_GUARD(id)                                     \
    for (int _sl_once = (syn_port_spinlock_acquire(id), 0);        \
         !_sl_once;                                                 \
         _sl_once = 1, syn_port_spinlock_release(id))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MULTICORE */

#endif /* SYN_SPINLOCK_H */
