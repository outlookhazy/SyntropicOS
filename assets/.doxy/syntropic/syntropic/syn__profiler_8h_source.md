

# File syn\_profiler.h

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_profiler.h**](syn__profiler_8h.md)

[Go to the documentation of this file](syn__profiler_8h.md)


```C++

#ifndef SYN_PROFILER_H
#define SYN_PROFILER_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Profile entry (per task) ───────────────────────────────────────────── */

typedef struct {
    const char *name;           
    uint32_t    total_us;       
    uint32_t    peak_us;        
    uint32_t    run_count;      
    uint16_t    cpu_percent_x10; 
    /* Internal */
    uint32_t    _start_tick;    
} SYN_ProfileEntry;

/* ── Profiler ───────────────────────────────────────────────────────────── */

typedef struct {
    SYN_ProfileEntry *entries;     
    uint8_t            capacity;    
    uint32_t           period_start; 
    uint32_t           period_ms;    
    bool               enabled;      
} SYN_Profiler;

typedef void (*SYN_ProfilerPrintFunc)(const char *str);

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_profiler_init(SYN_Profiler *prof,
                        SYN_ProfileEntry *entries,
                        uint8_t capacity);

void syn_profiler_register(SYN_Profiler *prof, uint8_t index,
                            const char *name);

void syn_profiler_task_begin(SYN_Profiler *prof, uint8_t index);

void syn_profiler_task_end(SYN_Profiler *prof, uint8_t index);

void syn_profiler_update(SYN_Profiler *prof);

void syn_profiler_dump(const SYN_Profiler *prof,
                        SYN_ProfilerPrintFunc print);

void syn_profiler_enable(SYN_Profiler *prof, bool enable);

static inline const SYN_ProfileEntry *
syn_profiler_get(const SYN_Profiler *prof, uint8_t index)
{
    return (index < prof->capacity) ? &prof->entries[index] : (const SYN_ProfileEntry *)0;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PROFILER_H */
```


