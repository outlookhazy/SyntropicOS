#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG

/**
 * @file syn_errlog.c
 * @brief Persistent error registry implementation.
 */

#include "syn_errlog.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"

#include <string.h>

void syn_errlog_init(SYN_ErrLog *log, SYN_ErrEntry *entries,
                      size_t capacity, uint32_t boot_count)
{
    SYN_ASSERT(log != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0);

    memset(log, 0, sizeof(*log));
    log->entries    = entries;
    log->capacity   = capacity;
    log->boot_count = boot_count;
    log->enabled    = true;

    memset(entries, 0, sizeof(SYN_ErrEntry) * capacity);
}

void syn_errlog_record(SYN_ErrLog *log, uint16_t code,
                        SYN_ErrSeverity severity, uint32_t context)
{
    if (!log->enabled) return;

    SYN_ErrEntry *e = &log->entries[log->head];
    e->code       = code;
    e->severity   = (uint8_t)severity;
    e->context    = context;
    e->timestamp  = syn_port_get_tick_ms();
    e->boot_count = log->boot_count;

    log->head++;
    if (log->head >= log->capacity) log->head = 0;
    log->total_count++;
}

bool syn_errlog_read(const SYN_ErrLog *log, size_t index,
                      SYN_ErrEntry *out)
{
    SYN_ASSERT(out != NULL);

    size_t avail = syn_errlog_available(log);
    if (index >= avail) return false;

    size_t start;
    if (log->total_count <= log->capacity) {
        start = 0;
    } else {
        start = log->head; /* oldest entry */
    }

    size_t actual = (start + index) % log->capacity;
    *out = log->entries[actual];
    return true;
}

void syn_errlog_clear(SYN_ErrLog *log)
{
    log->head        = 0;
    log->total_count = 0;
    memset(log->entries, 0, sizeof(SYN_ErrEntry) * log->capacity);
}

const SYN_ErrEntry *syn_errlog_latest(const SYN_ErrLog *log)
{
    if (log->total_count == 0) return NULL;

    size_t idx = (log->head == 0) ? log->capacity - 1 : log->head - 1;
    return &log->entries[idx];
}

size_t syn_errlog_count_severity(const SYN_ErrLog *log,
                                  SYN_ErrSeverity severity)
{
    size_t avail = syn_errlog_available(log);
    size_t count = 0;
    size_t i;

    size_t start = (log->total_count <= log->capacity) ? 0 : log->head;

    for (i = 0; i < avail; i++) {
        size_t idx = (start + i) % log->capacity;
        if (log->entries[idx].severity == (uint8_t)severity) {
            count++;
        }
    }
    return count;
}

#endif /* SYN_USE_ERRLOG */
