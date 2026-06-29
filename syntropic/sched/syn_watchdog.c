#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WATCHDOG) || SYN_USE_WATCHDOG

/**
 * @file syn_watchdog.c
 * @brief Task-level watchdog implementation.
 */

#include "syn_watchdog.h"
#include "../util/syn_assert.h"
#include "../system/syn_errlog.h"

#include <string.h>

#define SYN_WDT_ERR_TIMEOUT  0x0200  /**< Watchdog timeout detected. */

void syn_watchdog_init(SYN_Watchdog *wdt,
                        SYN_WDT_Entry *entries, uint8_t capacity,
                        SYN_WDT_TimeoutCallback callback, void *ctx)
{
    SYN_ASSERT(wdt != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0);

    memset(entries, 0, sizeof(*entries) * capacity);
    wdt->entries  = entries;
    wdt->capacity = capacity;
    wdt->count    = 0;
    wdt->callback = callback;
    wdt->ctx      = ctx;
    wdt->errlog   = NULL;
}

int8_t syn_watchdog_register(SYN_Watchdog *wdt, const char *name,
                              uint32_t timeout_ms)
{
    SYN_ASSERT(wdt != NULL);

    if (wdt->count >= wdt->capacity) return -1;

    /* Find free slot */
    for (uint8_t i = 0; i < wdt->capacity; i++) {
        if (!wdt->entries[i].active) {
            wdt->entries[i].name         = name;
            wdt->entries[i].timeout_ms   = timeout_ms;
            wdt->entries[i].last_checkin = syn_port_get_tick_ms();
            wdt->entries[i].active       = true;
            wdt->count++;
            return (int8_t)i;
        }
    }
    return -1;
}

void syn_watchdog_checkin(SYN_Watchdog *wdt, int8_t id)
{
    SYN_ASSERT(wdt != NULL);
    SYN_ASSERT(id >= 0 && (uint8_t)id < wdt->capacity);
    SYN_ASSERT(wdt->entries[id].active);

    wdt->entries[id].last_checkin = syn_port_get_tick_ms();
}

void syn_watchdog_unregister(SYN_Watchdog *wdt, int8_t id)
{
    SYN_ASSERT(wdt != NULL);
    SYN_ASSERT(id >= 0 && (uint8_t)id < wdt->capacity);

    wdt->entries[id].active = false;
    wdt->count--;
}

void syn_watchdog_update(SYN_Watchdog *wdt)
{
    SYN_ASSERT(wdt != NULL);

    uint32_t now = syn_port_get_tick_ms();

    for (uint8_t i = 0; i < wdt->capacity; i++) {
        if (!wdt->entries[i].active) continue;

        uint32_t elapsed = now - wdt->entries[i].last_checkin;
        if (elapsed >= wdt->entries[i].timeout_ms) {
            if (wdt->errlog != NULL) {
                syn_errlog_record(wdt->errlog, SYN_WDT_ERR_TIMEOUT,
                                   SYN_ERR_ERROR, (uint32_t)i);
            }
            if (wdt->callback != NULL) {
                wdt->callback(wdt, &wdt->entries[i], wdt->ctx);
            }
            /* Reset check-in to avoid repeated callbacks */
            wdt->entries[i].last_checkin = now;
        }
    }
}

#endif /* SYN_USE_WATCHDOG */
