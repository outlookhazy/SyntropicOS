

# File syn\_watchdog.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_watchdog.c**](syn__watchdog_8c.md)

[Go to the documentation of this file](syn__watchdog_8c.md)


```C++
#include "../common/syn_defs.h"

#if !defined(SYN_USE_WATCHDOG) || SYN_USE_WATCHDOG


#include "../util/syn_assert.h"
#include "syn_watchdog.h"
#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
#include "../system/syn_errlog.h"
#endif

#include <string.h>

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
#define SYN_WDT_ERR_TIMEOUT 0x0200 
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

    syn_port_enter_critical();
    wdt->entries = NULL;
    wdt->capacity = 0U;
    wdt->count = 0U;
    wdt->in_update = false;
    memset(entries, 0, sizeof(*entries) * capacity);
    wdt->entries = entries;
    wdt->capacity = capacity;
    wdt->callback = callback;
    wdt->ctx = ctx;
    wdt->errlog = NULL;
    syn_port_exit_critical();
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

static bool wdt_poll_slot_locked(SYN_Watchdog *wdt, uint8_t idx, SYN_WDT_Entry *out_snap,
                                 SYN_WDT_TimeoutCallback *out_cb, void **out_ctx,
                                 SYN_ErrLog **out_errlog)
{
    if (!wdt->entries[idx].active) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    uint32_t elapsed = now - wdt->entries[idx].last_checkin;
    if (elapsed < wdt->entries[idx].timeout_ms || elapsed > 0xEFFFFFFFU) {
        return false;
    }

    /* Reset check-in and snapshot slot/callback atomically before leaving critical section */
    wdt->entries[idx].last_checkin = now;
    *out_snap = wdt->entries[idx];
    *out_cb = wdt->callback;
    *out_ctx = wdt->ctx;
    *out_errlog = wdt->errlog;
    return true;
}

void syn_watchdog_update(SYN_Watchdog *wdt)
{
    if (wdt == NULL) {
        return;
    }

    syn_port_enter_critical();
    if (wdt->entries == NULL || wdt->in_update) {
        syn_port_exit_critical();
        return;
    }
    wdt->in_update = true;
    uint8_t cap = wdt->capacity;

    for (uint8_t i = 0U; i < cap; i++) {
        SYN_WDT_Entry snap;
        SYN_WDT_TimeoutCallback cb = NULL;
        void *cb_ctx = NULL;
        SYN_ErrLog *errlog = NULL;

        if (!wdt_poll_slot_locked(wdt, i, &snap, &cb, &cb_ctx, &errlog)) {
            continue;
        }
        syn_port_exit_critical();

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
        if (errlog != NULL) {
            syn_errlog_record(errlog, SYN_WDT_ERR_TIMEOUT, SYN_ERR_ERROR, (uint32_t)i);
        }
#endif
        if (cb != NULL) {
            cb(wdt, &snap, cb_ctx);
        }

        syn_port_enter_critical();
        if (!wdt->in_update) {
            syn_port_exit_critical();
            return;
        }
    }

    wdt->in_update = false;
    syn_port_exit_critical();
}

#endif /* SYN_USE_WATCHDOG */
```


