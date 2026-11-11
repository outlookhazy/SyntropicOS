#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG

/**
 * @file syn_errlog.c
 * @brief Persistent error registry implementation.
 */

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_errlog.h"

#include <string.h>

void syn_errlog_init(SYN_ErrLog *log, SYN_ErrEntry *entries, size_t capacity, uint32_t boot_count)
{
    SYN_ASSERT(log != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0);

    memset(log, 0, sizeof(*log));
    log->entries = entries;
    log->capacity = capacity;
    log->boot_count = boot_count;
    log->enabled = true;

    memset(entries, 0, sizeof(SYN_ErrEntry) * capacity);
}

void syn_errlog_record(SYN_ErrLog *log, uint16_t code, SYN_ErrSeverity severity, uint32_t context)
{
    if (log == NULL) {
        return;
    }

    uint32_t ts = syn_port_get_tick_ms();

    syn_port_enter_critical();
    if (!log->enabled || log->entries == NULL || log->capacity == 0U) {
        syn_port_exit_critical();
        return;
    }

    SYN_ErrEntry *e = &log->entries[log->head];
    e->code = code;
    e->severity = (uint8_t)severity;
    e->context = context;
    e->timestamp = ts;
    e->boot_count = log->boot_count;

    log->head++;
    if (log->head >= log->capacity) {
        log->head = 0U;
    }
    log->total_count++;
    syn_port_exit_critical();
}

bool syn_errlog_read(const SYN_ErrLog *log, size_t index, SYN_ErrEntry *out)
{
    if (log == NULL) {
        return false;
    }
    SYN_ASSERT(out != NULL);

    syn_port_enter_critical();
    size_t avail = syn_errlog_available(log);
    if (index >= avail) {
        syn_port_exit_critical();
        return false;
    }

    size_t start;
    if (log->total_count <= log->capacity) {
        start = 0U;
    } else {
        start = log->head; /* oldest entry */
    }

    size_t actual = (start + index) % log->capacity;
    *out = log->entries[actual];
    syn_port_exit_critical();
    return true;
}

void syn_errlog_clear(SYN_ErrLog *log)
{
    if (log == NULL) {
        return;
    }
    syn_port_enter_critical();
    log->head = 0U;
    log->total_count = 0U;
    if ((log->entries != NULL) && (log->capacity > 0U)) {
        memset(log->entries, 0, sizeof(SYN_ErrEntry) * log->capacity);
    }
    syn_port_exit_critical();
}

const SYN_ErrEntry *syn_errlog_latest(const SYN_ErrLog *log)
{
    const SYN_ErrEntry *entry = NULL;

    if (log == NULL) {
        return NULL;
    }

    syn_port_enter_critical();
    if ((log->entries != NULL) && (log->capacity > 0U) && (log->total_count > 0U)) {
        size_t idx = (log->head == 0U) ? (log->capacity - 1U) : (log->head - 1U);
        entry = &log->entries[idx];
    }
    syn_port_exit_critical();
    return entry;
}

size_t syn_errlog_count_severity(const SYN_ErrLog *log, SYN_ErrSeverity severity)
{
    size_t count = 0U;

    if (log == NULL) {
        return 0U;
    }

    syn_port_enter_critical();
    if ((log->entries != NULL) && (log->capacity > 0U)) {
        size_t avail = syn_errlog_available(log);
        size_t start = (log->total_count <= log->capacity) ? 0U : log->head;
        size_t i;

        for (i = 0U; i < avail; i++) {
            size_t idx = (start + i) % log->capacity;
            if (log->entries[idx].severity == (uint8_t)severity) {
                count++;
            }
        }
    }
    syn_port_exit_critical();
    return count;
}

#endif /* SYN_USE_ERRLOG */
