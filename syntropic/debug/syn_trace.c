#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_TRACE) || SYN_USE_TRACE

/**
 * @file syn_trace.c
 * @brief Trace buffer implementation.
 */

#include "syn_trace.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"

#include <string.h>

void syn_trace_init(SYN_Trace *trace,
                     SYN_TraceEntry *entries,
                     uint16_t capacity)
{
    SYN_ASSERT(trace != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0);

    memset(entries, 0, sizeof(*entries) * capacity);
    trace->entries  = entries;
    trace->capacity = capacity;
    trace->head     = 0;
    trace->count    = 0;
    trace->enabled  = true;
}

void syn_trace_record(SYN_Trace *trace, uint16_t event_id, uint16_t value)
{
    SYN_ASSERT(trace != NULL);

    if (!trace->enabled) return;

    SYN_TraceEntry *e = &trace->entries[trace->head];
    e->timestamp = syn_port_get_tick_ms();
    e->event_id  = event_id;
    e->value     = value;

    trace->head = (uint16_t)((trace->head + 1) % trace->capacity);
    trace->count++;
}

void syn_trace_enable(SYN_Trace *trace, bool enable)
{
    SYN_ASSERT(trace != NULL);
    trace->enabled = enable;
}

void syn_trace_clear(SYN_Trace *trace)
{
    SYN_ASSERT(trace != NULL);
    trace->head  = 0;
    trace->count = 0;
}

bool syn_trace_read(const SYN_Trace *trace, uint32_t index,
                     SYN_TraceEntry *entry)
{
    SYN_ASSERT(trace != NULL);
    SYN_ASSERT(entry != NULL);

    uint32_t available = (trace->count < trace->capacity) ?
                         trace->count : trace->capacity;
    if (index >= available) return false;

    uint16_t start;
    if (trace->count >= trace->capacity) {
        start = trace->head; /* oldest is at head (just wrapped) */
    } else {
        start = 0;
    }

    uint16_t pos = (uint16_t)((start + index) % trace->capacity);
    *entry = trace->entries[pos];
    return true;
}

void syn_trace_dump(const SYN_Trace *trace, SYN_TracePrintFunc print)
{
    SYN_ASSERT(trace != NULL);

    if (print == NULL) return;

    uint32_t available = (trace->count < trace->capacity) ?
                         trace->count : trace->capacity;

    char buf[64];
    SYN_TraceEntry entry;

    for (uint32_t i = 0; i < available; i++) {
        if (syn_trace_read(trace, i, &entry)) {
            /* Format: "[  timestamp] evt=0xXXXX val=0xXXXX\n" */
            char num[12];
            size_t pos = 0;

            buf[pos++] = '[';

            /* Right-align timestamp in 10-char field */
            syn_fmt_uint(num, sizeof(num), entry.timestamp);
            size_t nl = strlen(num);
            size_t pad = (10 > nl) ? 10 - nl : 0;
            while (pad-- > 0) buf[pos++] = ' ';
            memcpy(buf + pos, num, nl); pos += nl;

            buf[pos++] = ']';
            buf[pos++] = ' ';

            /* evt=0xXXXX */
            memcpy(buf + pos, "evt=0x", 6); pos += 6;
            syn_fmt_hex(num, sizeof(num), entry.event_id, 4);
            memcpy(buf + pos, num, 4); pos += 4;

            /* val=0xXXXX */
            memcpy(buf + pos, " val=0x", 7); pos += 7;
            syn_fmt_hex(num, sizeof(num), entry.value, 4);
            memcpy(buf + pos, num, 4); pos += 4;

            buf[pos++] = '\n';
            buf[pos] = '\0';
            print(buf);
        }
    }
}

#endif /* SYN_USE_TRACE */
