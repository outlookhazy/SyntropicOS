#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_WATCHDOG) || SYN_USE_WATCHDOG

/**
 * @file syn_watchdog.c
 * @brief Task-level watchdog implementation.
 */

#include "../util/syn_assert.h"
#include "syn_watchdog.h"
#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
#include "../system/syn_errlog.h"
#endif

#include <string.h>

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
#define SYN_WDT_ERR_TIMEOUT 0x0200 /**< Watchdog timeout detected. */
#endif

void syn_watchdog_init(SYN_Watchdog *wdt, SYN_WDT_Entry *entries, uint8_t capacity,
                       SYN_WDT_TimeoutCallback callback, void *ctx)
{
    SYN_ASSERT(wdt != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0U);

    if (wdt == NULL || entries == NULL ||
        capacity == 0U) { /* LCOV_EXCL_LINE: Defensive NULL/zero check */
        return;           /* LCOV_EXCL_LINE: Defensive NULL/zero fallback */
    }
    if (capacity > SYN_WATCHDOG_MAX_CAPACITY) {
        capacity = SYN_WATCHDOG_MAX_CAPACITY;
    }

    memset(entries, 0, sizeof(*entries) * capacity);
    wdt->entries = entries;
    wdt->capacity = capacity;
    wdt->count = 0;
    wdt->callback = callback;
    wdt->ctx = ctx;
    wdt->errlog = NULL;
}

int8_t syn_watchdog_register(SYN_Watchdog *wdt, const char *name, uint32_t timeout_ms)
{
    if (wdt == NULL) {
        return -1;
    }

    if (timeout_ms == 0U) {
        timeout_ms = 1U;
    } else if (timeout_ms > 0x7FFFFFFFU) {
        timeout_ms = 0x7FFFFFFFU;
    }

    int8_t slot = -1;
    syn_port_enter_critical();
    if (wdt->entries != NULL && wdt->count < wdt->capacity &&
        wdt->capacity <= SYN_WATCHDOG_MAX_CAPACITY) {
        for (uint8_t i = 0U; i < wdt->capacity; i++) {
            if (!wdt->entries[i].active) {
                wdt->entries[i].name = name;
                wdt->entries[i].timeout_ms = timeout_ms;
                wdt->entries[i].last_checkin = syn_port_get_tick_ms();
                wdt->entries[i].active = true;
                wdt->count++;
                slot = (int8_t)i;
                break;
            }
        }
    }
    syn_port_exit_critical();

    return slot;
}

void syn_watchdog_checkin(SYN_Watchdog *wdt, int8_t id)
{
    if (wdt == NULL || id < 0) {
        return;
    }

    syn_port_enter_critical();
    if (wdt->entries != NULL && (uint8_t)id < wdt->capacity && wdt->entries[id].active) {
        wdt->entries[id].last_checkin = syn_port_get_tick_ms();
    }
    syn_port_exit_critical();
}

void syn_watchdog_unregister(SYN_Watchdog *wdt, int8_t id)
{
    if (wdt == NULL || id < 0) {
        return;
    }

    syn_port_enter_critical();
    if (wdt->entries != NULL && (uint8_t)id < wdt->capacity && wdt->entries[id].active) {
        wdt->entries[id].active = false;
        wdt->count--;
    }
    syn_port_exit_critical();
}

void syn_watchdog_update(SYN_Watchdog *wdt)
{
    if (wdt == NULL || wdt->entries == NULL) {
        return;
    }

    for (uint8_t i = 0U; i < wdt->capacity; i++) {
        bool timed_out = false;

        syn_port_enter_critical();
        if (wdt->entries[i].active) {
            uint32_t now = syn_port_get_tick_ms();
            uint32_t elapsed = now - wdt->entries[i].last_checkin;
            if (elapsed >= wdt->entries[i].timeout_ms && elapsed <= 0xEFFFFFFFU) {
                /* Reset check-in atomically so callback check-in/unregister is preserved */
                wdt->entries[i].last_checkin = now;
                timed_out = true;
            }
        }
        syn_port_exit_critical();

        if (!timed_out) {
            continue;
        }

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
        if (wdt->errlog != NULL) {
            syn_errlog_record(wdt->errlog, SYN_WDT_ERR_TIMEOUT, SYN_ERR_ERROR, (uint32_t)i);
        }
#endif
        if (wdt->callback != NULL) {
            wdt->callback(wdt, &wdt->entries[i], wdt->ctx);
        }
    }
}

#endif /* SYN_USE_WATCHDOG */
