/**
 * @file syn_event_flags.h
 * @brief 32-bit Event Flag Group Synchronization.
 * @ingroup syn_sched
 *
 * Provides a bitmask event group mechanism for tasks and protothreads
 * to set, clear, and wait on single or multiple event bit combinations
 * (WAIT_ALL, WAIT_ANY, auto-clear).
 */

#ifndef SYN_EVENT_FLAGS_H
#define SYN_EVENT_FLAGS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Wait mode flags. */
typedef enum {
    SYN_EVENT_FLAGS_WAIT_ANY      = 0x01U, /**< Satisfied when ANY bit in mask is set */
    SYN_EVENT_FLAGS_WAIT_ALL      = 0x02U, /**< Satisfied when ALL bits in mask are set */
    SYN_EVENT_FLAGS_AUTO_CLEAR    = 0x04U  /**< Automatically clear matched bits on read */
} SYN_EventFlagsMode;

/**
 * @brief Event Flags Control Structure.
 */
typedef struct {
    volatile uint32_t flags; /**< 32-bit atomic event flags register */
} SYN_EventFlags;

/**
 * @brief Initialize an Event Flags instance.
 * @param ef Event flags context.
 * @return SYN_OK on success.
 */
SYN_Status syn_event_flags_init(SYN_EventFlags *ef);

/**
 * @brief Set event flags (Bitwise OR).
 * @param ef Event flags context.
 * @param flags_mask Bitmask of flags to set.
 * @return SYN_OK on success.
 */
SYN_Status syn_event_flags_set(SYN_EventFlags *ef, uint32_t flags_mask);

/**
 * @brief Clear event flags (Bitwise AND NOT).
 * @param ef Event flags context.
 * @param flags_mask Bitmask of flags to clear.
 * @return SYN_OK on success.
 */
SYN_Status syn_event_flags_clear(SYN_EventFlags *ef, uint32_t flags_mask);

/**
 * @brief Get current event flags value without clearing.
 * @param ef Event flags context.
 * @return Current 32-bit flags value.
 */
uint32_t syn_event_flags_get(const SYN_EventFlags *ef);

/**
 * @brief Non-blocking evaluate/wait for event flags condition.
 * @param ef Event flags context.
 * @param wait_mask Bitmask of flags to evaluate.
 * @param mode Combination of SYN_EVENT_FLAGS_WAIT_ANY/ALL/AUTO_CLEAR.
 * @param out_flags Optional pointer receiving matched flags.
 * @return SYN_OK if condition is satisfied, or SYN_BUSY if not met.
 */
SYN_Status syn_event_flags_wait(SYN_EventFlags *ef, uint32_t wait_mask, uint32_t mode, uint32_t *out_flags);

#ifdef __cplusplus
}
#endif

#endif /* SYN_EVENT_FLAGS_H */
