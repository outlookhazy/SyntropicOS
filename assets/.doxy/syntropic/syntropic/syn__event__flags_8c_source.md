

# File syn\_event\_flags.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_event\_flags.c**](syn__event__flags_8c.md)

[Go to the documentation of this file](syn__event__flags_8c.md)


```C++

#include "syn_event_flags.h"

#include "../port/syn_port_system.h"

SYN_Status syn_event_flags_init(SYN_EventFlags *ef)
{
    if (!ef)
        return SYN_INVALID_PARAM;
    ef->flags = 0;
    return SYN_OK;
}

/*
 * set/clear/wait are callable from both ISR and task context. `volatile`
 * alone does not make |= / &= atomic (LDR+ORR/BIC+STR on Cortex-M), so a
 * preempting ISR's flag update can be lost to a stale read-modify-write.
 * Critical sections make the RMW atomic; syn_port_enter_critical() nests.
 */
SYN_Status syn_event_flags_set(SYN_EventFlags *ef, uint32_t flags_mask)
{
    if (!ef)
        return SYN_INVALID_PARAM;
    syn_port_enter_critical();
    ef->flags |= flags_mask;
    syn_port_exit_critical();
    return SYN_OK;
}

SYN_Status syn_event_flags_clear(SYN_EventFlags *ef, uint32_t flags_mask)
{
    if (!ef)
        return SYN_INVALID_PARAM;
    syn_port_enter_critical();
    ef->flags &= ~flags_mask;
    syn_port_exit_critical();
    return SYN_OK;
}

uint32_t syn_event_flags_get(const SYN_EventFlags *ef)
{
    if (!ef)
        return 0;
    return ef->flags;
}

SYN_Status syn_event_flags_wait(SYN_EventFlags *ef, uint32_t wait_mask, uint32_t mode,
                                uint32_t *out_flags)
{
    if (!ef || wait_mask == 0)
        return SYN_INVALID_PARAM;

    /* Read and auto-clear must be one atomic unit: an ISR setting an
     * unrelated bit between them would be clobbered by the stale RMW. */
    syn_port_enter_critical();

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
        syn_port_exit_critical();
        return SYN_BUSY; /* Condition not yet satisfied */
    }

    if (mode & SYN_EVENT_FLAGS_AUTO_CLEAR) {
        ef->flags &= ~matched;
    }

    syn_port_exit_critical();

    if (out_flags) {
        *out_flags = matched;
    }

    return SYN_OK;
}
```


