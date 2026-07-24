/**
 * @file syn_event_flags.c
 * @brief 32-bit Event Flag Group Synchronization implementation.
 */

#include "syn_event_flags.h"

SYN_Status syn_event_flags_init(SYN_EventFlags *ef)
{
    if (!ef) return SYN_INVALID_PARAM;
    ef->flags = 0;
    return SYN_OK;
}

SYN_Status syn_event_flags_set(SYN_EventFlags *ef, uint32_t flags_mask)
{
    if (!ef) return SYN_INVALID_PARAM;
    ef->flags |= flags_mask;
    return SYN_OK;
}

SYN_Status syn_event_flags_clear(SYN_EventFlags *ef, uint32_t flags_mask)
{
    if (!ef) return SYN_INVALID_PARAM;
    ef->flags &= ~flags_mask;
    return SYN_OK;
}

uint32_t syn_event_flags_get(const SYN_EventFlags *ef)
{
    if (!ef) return 0;
    return ef->flags;
}

SYN_Status syn_event_flags_wait(SYN_EventFlags *ef, uint32_t wait_mask, uint32_t mode, uint32_t *out_flags)
{
    if (!ef || wait_mask == 0) return SYN_INVALID_PARAM;

    uint32_t current = ef->flags;
    uint32_t matched = current & wait_mask;
    bool satisfied = false;

    if (mode & SYN_EVENT_FLAGS_WAIT_ALL) {
        satisfied = (matched == wait_mask);
    } else {
        /* Default to WAIT_ANY */
        satisfied = (matched != 0);
    }

    if (!satisfied) {
        return SYN_BUSY; /* Condition not yet satisfied */
    }

    if (out_flags) {
        *out_flags = matched;
    }

    if (mode & SYN_EVENT_FLAGS_AUTO_CLEAR) {
        ef->flags &= ~matched;
    }

    return SYN_OK;
}
