/**
 * @file syn_event.h
 * @brief Event flag groups — 32-bit bitmask signaling.
 * @ingroup syn_sched
 *
 * Unified event signaling delegating directly to SYN_EventFlags.
 */

#ifndef SYN_EVENT_H
#define SYN_EVENT_H

#include "../common/syn_defs.h"
#include "../util/syn_bits.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_event_flags.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event flag group — alias for SYN_EventFlags.
 */
typedef SYN_EventFlags SYN_EventGroup;

/**
 * @brief Initialize an event group.
 * @param grp Event group.
 */
static inline void syn_event_init(SYN_EventGroup *grp)
{
    syn_event_flags_init(grp);
}

/**
 * @brief Set one or more event flags.
 * @param grp Event group.
 * @param mask Bitmask of flags to set.
 */
static inline void syn_event_set(SYN_EventGroup *grp, uint32_t mask)
{
    syn_event_flags_set(grp, mask);
}

/**
 * @brief Clear one or more event flags.
 * @param grp Event group.
 * @param mask Bitmask of flags to clear.
 */
static inline void syn_event_clear(SYN_EventGroup *grp, uint32_t mask)
{
    syn_event_flags_clear(grp, mask);
}

/**
 * @brief Get the current value of all flags.
 * @param grp Event group.
 * @return Current flags.
 */
static inline uint32_t syn_event_get(const SYN_EventGroup *grp)
{
    return syn_event_flags_get(grp);
}

/**
 * @brief Check if ALL bits in @p mask are set.
 * @param grp Event group.
 * @param mask Bitmask to test.
 * @return true if every bit in @p mask is set in the event group.
 */
static inline bool syn_event_check_all(const SYN_EventGroup *grp, uint32_t mask)
{
    return (syn_event_flags_get(grp) & mask) == mask;
}

/**
 * @brief Check if ANY bit in @p mask is set.
 * @param grp Event group.
 * @param mask Bitmask to test.
 * @return true if at least one bit in @p mask is set.
 */
static inline bool syn_event_check_any(const SYN_EventGroup *grp, uint32_t mask)
{
    return (syn_event_flags_get(grp) & mask) != 0;
}

/* ── Protothread integration ────────────────────────────────────────────── */

/**
 * @brief Block the protothread until ALL bits in @p mask are set, then auto-clear.
 */
#define PT_WAIT_EVENT(pt, grp, mask)                          \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_all(grp, mask));    \
        syn_event_clear(grp, mask);                           \
    } while (0)

/**
 * @brief Block the protothread until ANY bit in @p mask is set, then auto-clear.
 */
#define PT_WAIT_EVENT_ANY(pt, grp, mask)                      \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_any(grp, mask));    \
        syn_event_clear(grp, syn_event_get(grp) & (mask));    \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_EVENT_H */
