/**
 * @file syn_timeout.h
 * @brief Non-blocking timeout helper — tick wrap-around safe.
 *
 * The "has N ms elapsed?" pattern is used everywhere in embedded code
 * and is easy to get wrong with tick counter wrap-around. This header
 * provides a tiny inline API that handles it correctly.
 *
 * Header-only — zero code size if unused.
 *
 * @par Usage
 * @code
 *   SYN_Timeout to;
 *   syn_timeout_start(&to, 1000);
 *
 *   while (!syn_timeout_expired(&to)) {
 *       // try something...
 *   }
 *
 *   // In a protothread:
 *   PT_WAIT_TIMEOUT(pt, &to);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_TIMEOUT_H
#define SYN_TIMEOUT_H

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timeout struct ─────────────────────────────────────────────────────── */

/** @brief Non-blocking timeout — tick wrap-around safe. */
typedef struct {
    uint32_t start;       /**< Tick at which the timeout was started */
    uint32_t duration;    /**< Timeout duration in milliseconds      */
} SYN_Timeout;

/* ── API (all inline) ───────────────────────────────────────────────────── */

/**
 * @brief Start (or restart) a timeout.
 *
 * @param to        Timeout to start.
 * @param duration  Duration in milliseconds.
 */
static inline void syn_timeout_start(SYN_Timeout *to, uint32_t duration)
{
    to->start    = syn_port_get_tick_ms();
    to->duration = duration;
}

/**
 * @brief Check if the timeout has expired.
 *
 * Uses unsigned subtraction, which is wrap-around safe for a single
 * wrap of the 32-bit tick counter (~49 days at 1 kHz).
 *
 * @param to  Timeout to check.
 * @return true if the timeout has expired.
 */
static inline bool syn_timeout_expired(const SYN_Timeout *to)
{
    return (syn_port_get_tick_ms() - to->start) >= to->duration;
}

/**
 * @brief Get the remaining time until expiry.
 *
 * @param to  Timeout.
 * @return Remaining milliseconds, or 0 if already expired.
 */
static inline uint32_t syn_timeout_remaining(const SYN_Timeout *to)
{
    uint32_t elapsed = syn_port_get_tick_ms() - to->start;
    return (elapsed < to->duration) ? (to->duration - elapsed) : 0;
}

/**
 * @brief Get the elapsed time since the timeout was started.
 * @param to  Timeout.
 * @return Elapsed milliseconds.
 */
static inline uint32_t syn_timeout_elapsed(const SYN_Timeout *to)
{
    return syn_port_get_tick_ms() - to->start;
}

/**
 * @brief Restart the timeout from *now* with the same duration.
 * @param to  Timeout.
 */
static inline void syn_timeout_restart(SYN_Timeout *to)
{
    to->start = syn_port_get_tick_ms();
}

/**
 * @brief Check expiry and auto-restart if expired (periodic use).
 *
 * Useful in main loops: "every N ms, do X".
 *
 * @param to  Timeout.
 * @return true if the timeout expired (and was restarted).
 */
static inline bool syn_timeout_periodic(SYN_Timeout *to)
{
    if (syn_timeout_expired(to)) {
        /* Re-arm relative to the old start to avoid drift */
        to->start += to->duration;
        return true;
    }
    return false;
}

/* ── Protothread integration ────────────────────────────────────────────── */

/**
 * @brief Block a protothread until the timeout expires.
 */
#define PT_WAIT_TIMEOUT(pt, to) \
    PT_WAIT_UNTIL(pt, syn_timeout_expired(to))

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMEOUT_H */
