

# File syn\_trace.h

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_trace.h**](syn__trace_8h.md)

[Go to the documentation of this file](syn__trace_8h.md)


```C++

#ifndef SYN_TRACE_H
#define SYN_TRACE_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Trace entry ────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t  timestamp;  
    uint16_t  event_id;   
    uint16_t  value;      
} SYN_TraceEntry;

/* ── Trace buffer ───────────────────────────────────────────────────────── */

typedef struct {
    SYN_TraceEntry *entries;   
    uint16_t         capacity;  
    uint16_t         head;      
    uint32_t         count;     
    bool             enabled;   
} SYN_Trace;

typedef void (*SYN_TracePrintFunc)(const char *str);

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_trace_init(SYN_Trace *trace,
                     SYN_TraceEntry *entries,
                     uint16_t capacity);

void syn_trace_record(SYN_Trace *trace, uint16_t event_id, uint16_t value);

#define SYN_TRACE(trace, id, val) syn_trace_record((trace), (id), (val))

void syn_trace_enable(SYN_Trace *trace, bool enable);

void syn_trace_clear(SYN_Trace *trace);

static inline uint32_t syn_trace_count(const SYN_Trace *trace)
{
    return trace->count;
}

bool syn_trace_read(const SYN_Trace *trace, uint32_t index,
                     SYN_TraceEntry *entry);

void syn_trace_dump(const SYN_Trace *trace, SYN_TracePrintFunc print);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRACE_H */
```


