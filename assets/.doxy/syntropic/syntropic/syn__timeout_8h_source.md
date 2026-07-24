

# File syn\_timeout.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_timeout.h**](syn__timeout_8h.md)

[Go to the documentation of this file](syn__timeout_8h.md)


```C++

#ifndef SYN_TIMEOUT_H
#define SYN_TIMEOUT_H

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timeout struct ─────────────────────────────────────────────────────── */

typedef struct {
    uint32_t start;       
    uint32_t duration;    
} SYN_Timeout;

/* ── API (all inline) ───────────────────────────────────────────────────── */

static inline void syn_timeout_start(SYN_Timeout *to, uint32_t duration)
{
    to->start    = syn_port_get_tick_ms();
    to->duration = duration;
}

static inline bool syn_timeout_expired(const SYN_Timeout *to)
{
    return (syn_port_get_tick_ms() - to->start) >= to->duration;
}

static inline uint32_t syn_timeout_remaining(const SYN_Timeout *to)
{
    uint32_t elapsed = syn_port_get_tick_ms() - to->start;
    return (elapsed < to->duration) ? (to->duration - elapsed) : 0;
}

static inline uint32_t syn_timeout_elapsed(const SYN_Timeout *to)
{
    return syn_port_get_tick_ms() - to->start;
}

static inline void syn_timeout_restart(SYN_Timeout *to)
{
    to->start = syn_port_get_tick_ms();
}

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

#define PT_WAIT_TIMEOUT(pt, to) \
    PT_WAIT_UNTIL(pt, syn_timeout_expired(to))

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMEOUT_H */
```


