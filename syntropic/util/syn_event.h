/**
 * @file syn_event.h
 * @brief Event flag groups — 32-bit bitmask signaling.
 *
 * Event flags provide a lightweight way to signal between tasks and ISRs.
 * Each event group is a 32-bit word where each bit represents a distinct
 * event. Tasks can wait for any combination of bits to be set.
 *
 * Setting flags is ISR-safe (single atomic write on 32-bit targets).
 *
 * @par Usage
 * @code
 *   #define EVT_DATA_READY   SYN_BIT(0)
 *   #define EVT_TIMEOUT      SYN_BIT(1)
 *
 *   static SYN_EventGroup events;
 *   syn_event_init(&events);
 *
 *   // ISR or another task:
 *   syn_event_set(&events, EVT_DATA_READY);
 *
 *   // In a protothread:
 *   PT_WAIT_EVENT(pt, &events, EVT_DATA_READY);
 *   // EVT_DATA_READY is now auto-cleared, process data...
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_EVENT_H
#define SYN_EVENT_H

#include "../common/syn_defs.h"
#include "../util/syn_bits.h"
#include "../pt/syn_pt.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Event group struct ─────────────────────────────────────────────────── */

/**
 * @brief Event flag group — a 32-bit bitmask of event flags.
 */
typedef struct {
    volatile uint32_t flags;   /**< Bitmask of active event flags         */
} SYN_EventGroup;

/* ── Initialization ─────────────────────────────────────────────────────── */

/**
 * @brief Initialize an event group (clear all flags).
 * @param grp  Event group.
 */
static inline void syn_event_init(SYN_EventGroup *grp)
{
    grp->flags = 0;
}

/* ── Set / Clear / Get ──────────────────────────────────────────────────── */

/**
 * @brief Set one or more event flags.
 *
 * ISR-safe on 32-bit targets (single word write).
 *
 * @param grp   Event group.
 * @param mask  Bitmask of flags to set.
 */
static inline void syn_event_set(SYN_EventGroup *grp, uint32_t mask)
{
    grp->flags |= mask;
}

/**
 * @brief Clear one or more event flags.
 *
 * @param grp   Event group.
 * @param mask  Bitmask of flags to clear.
 */
static inline void syn_event_clear(SYN_EventGroup *grp, uint32_t mask)
{
    grp->flags &= ~mask;
}

/**
 * @brief Get the current value of all flags.
 * @param grp  Event group.
 * @return Current flags.
 */
static inline uint32_t syn_event_get(const SYN_EventGroup *grp)
{
    return grp->flags;
}

/* ── Check ──────────────────────────────────────────────────────────────── */

/**
 * @brief Check if ALL bits in @p mask are set.
 *
 * @param grp   Event group.
 * @param mask  Bitmask to test.
 * @return true if every bit in @p mask is set in the event group.
 */
static inline bool syn_event_check_all(const SYN_EventGroup *grp, uint32_t mask)
{
    return (grp->flags & mask) == mask;
}

/**
 * @brief Check if ANY bit in @p mask is set.
 *
 * @param grp   Event group.
 * @param mask  Bitmask to test.
 * @return true if at least one bit in @p mask is set.
 */
static inline bool syn_event_check_any(const SYN_EventGroup *grp, uint32_t mask)
{
    return (grp->flags & mask) != 0;
}

/* ── Protothread integration ────────────────────────────────────────────── */

/**
 * @brief Block the protothread until ALL bits in @p mask are set,
 *        then auto-clear them.
 *
 * @param pt    Protothread.
 * @param grp   Event group.
 * @param mask  Bitmask of required flags.
 */
#define PT_WAIT_EVENT(pt, grp, mask)                          \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_all(grp, mask));    \
        syn_event_clear(grp, mask);                           \
    } while (0)

/**
 * @brief Block the protothread until ANY bit in @p mask is set,
 *        then auto-clear the bits that were set.
 *
 * @param pt    Protothread.
 * @param grp   Event group.
 * @param mask  Bitmask of flags to wait for (any of them).
 */
#define PT_WAIT_EVENT_ANY(pt, grp, mask)                      \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_any(grp, mask));    \
        syn_event_clear(grp, (grp)->flags & (mask));          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_EVENT_H */
