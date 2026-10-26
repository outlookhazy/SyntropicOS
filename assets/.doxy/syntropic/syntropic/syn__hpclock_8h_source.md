

# File syn\_hpclock.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hpclock.h**](syn__hpclock_8h.md)

[Go to the documentation of this file](syn__hpclock_8h.md)


```C++

#ifndef SYN_HPCLOCK_H
#define SYN_HPCLOCK_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HPCLOCK) || SYN_USE_HPCLOCK

#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"
#include "../port/syn_port_hpclock.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timestamp ─────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t msb_1; 
    uint32_t lsb;   
    uint32_t msb_2; 
} SYN_HPTimestamp;

#define SYN_HPTIMESTAMP_INIT {0, 0, 0}

/* ── Capture macro ─────────────────────────────────────────────────────── */

#define SYN_HPCLOCK_CAPTURE(ts)                                        \
    do {                                                               \
        volatile uint32_t *syn_hp_lsb_p_ = syn_port_hpclock_lsb_ptr(); \
        (ts).msb_1 = syn_hpclock_msb;                                  \
        SYN_COMPILER_BARRIER();                                        \
        (ts).lsb = *syn_hp_lsb_p_;                                     \
        SYN_COMPILER_BARRIER();                                        \
        (ts).msb_2 = syn_hpclock_msb;                                  \
    } while (0)

/* ── Resolution ────────────────────────────────────────────────────────── */

uint64_t syn_hpclock_resolve(const SYN_HPTimestamp *ts);

uint64_t syn_hpclock_ticks_to_ns(uint64_t ticks);

uint64_t syn_hpclock_elapsed(const SYN_HPTimestamp *start, const SYN_HPTimestamp *end);

static inline bool syn_hpclock_is_zero(const SYN_HPTimestamp *ts)
{
    return (ts->msb_1 == 0) && (ts->lsb == 0) && (ts->msb_2 == 0);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_HPCLOCK */

#endif /* SYN_HPCLOCK_H */
```


