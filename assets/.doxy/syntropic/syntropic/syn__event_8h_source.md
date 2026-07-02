

# File syn\_event.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_event.h**](syn__event_8h.md)

[Go to the documentation of this file](syn__event_8h.md)


```C++

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

typedef struct {
    volatile uint32_t flags;   
} SYN_EventGroup;

/* ── Initialization ─────────────────────────────────────────────────────── */

static inline void syn_event_init(SYN_EventGroup *grp)
{
    grp->flags = 0;
}

/* ── Set / Clear / Get ──────────────────────────────────────────────────── */

static inline void syn_event_set(SYN_EventGroup *grp, uint32_t mask)
{
    grp->flags |= mask;
}

static inline void syn_event_clear(SYN_EventGroup *grp, uint32_t mask)
{
    grp->flags &= ~mask;
}

static inline uint32_t syn_event_get(const SYN_EventGroup *grp)
{
    return grp->flags;
}

/* ── Check ──────────────────────────────────────────────────────────────── */

static inline bool syn_event_check_all(const SYN_EventGroup *grp, uint32_t mask)
{
    return (grp->flags & mask) == mask;
}

static inline bool syn_event_check_any(const SYN_EventGroup *grp, uint32_t mask)
{
    return (grp->flags & mask) != 0;
}

/* ── Protothread integration ────────────────────────────────────────────── */

#define PT_WAIT_EVENT(pt, grp, mask)                          \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_all(grp, mask));    \
        syn_event_clear(grp, mask);                           \
    } while (0)

#define PT_WAIT_EVENT_ANY(pt, grp, mask)                      \
    do {                                                       \
        PT_WAIT_UNTIL(pt, syn_event_check_any(grp, mask));    \
        syn_event_clear(grp, (grp)->flags & (mask));          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_EVENT_H */
```


