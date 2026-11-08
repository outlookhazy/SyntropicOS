/**
 * @file syn_spinlock.h
 * @brief Scoped spinlock helper for shared resource protection.
 *
 * Convenience wrapper around the spinlock port functions. Provides
 * a scoped lock macro that acquires on entry and releases on normal
 * block completion.
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

#include "../common/syn_defs.h"

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../port/syn_port_spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Well-known spinlock IDs ───────────────────────────────────────────── */

/** @brief Spinlock ID for shared UART peripheral access. */
#ifndef SYN_SPINLOCK_UART
#define SYN_SPINLOCK_UART 0
#endif

/** @brief Spinlock ID for shared flash access. */
#ifndef SYN_SPINLOCK_FLASH
#define SYN_SPINLOCK_FLASH 1
#endif

/** @brief Spinlock ID for application use. */
#ifndef SYN_SPINLOCK_USER0
#define SYN_SPINLOCK_USER0 2
#endif

/** @brief Spinlock ID for application use. */
#ifndef SYN_SPINLOCK_USER1
#define SYN_SPINLOCK_USER1 3
#endif

/* ── Scoped lock macro ─────────────────────────────────────────────────── */

/**
 * @brief Scoped spinlock guard.
 *
 * Acquires the spinlock before executing the guarded block and releases it
 * when control reaches the end of the block normally.
 *
 * @warning Do NOT execute return, break, or goto out of a SYN_SPINLOCK_GUARD
 *          block; doing so skips the post-expression release and leaks both
 *          the spinlock and interrupt mask.
 *
 * @param id  Spinlock index (0 .. SYN_SPINLOCK_COUNT-1).
 *
 * @code
 *   SYN_SPINLOCK_GUARD(SYN_SPINLOCK_FLASH) {
 *       syn_port_flash_write(addr, data, len);
 *   }
 * @endcode
 */
#define SYN_SPINLOCK_GUARD(id)                                         \
    for (int _sl_once = (syn_port_spinlock_acquire(id), 0); !_sl_once; \
         _sl_once = 1, syn_port_spinlock_release(id))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MULTICORE */

#endif /* SYN_SPINLOCK_H */
