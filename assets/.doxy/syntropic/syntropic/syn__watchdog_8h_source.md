

# File syn\_watchdog.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_watchdog.h**](syn__watchdog_8h.md)

[Go to the documentation of this file](syn__watchdog_8h.md)


```C++

#ifndef SYN_WATCHDOG_H
#define SYN_WATCHDOG_H

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Watchdog entry ─────────────────────────────────────────────────────── */

typedef struct {
    const char  *name;          
    uint32_t     timeout_ms;    
    uint32_t     last_checkin;  
    bool         active;        
} SYN_WDT_Entry;

/* ── Callback ───────────────────────────────────────────────────────────── */

struct SYN_Watchdog;

typedef void (*SYN_WDT_TimeoutCallback)(struct SYN_Watchdog *wdt,
                                         const SYN_WDT_Entry *entry,
                                         void *ctx);

/* ── Watchdog instance ──────────────────────────────────────────────────── */

typedef struct SYN_Watchdog {
    SYN_WDT_Entry          *entries;    
    uint8_t                  capacity;   
    uint8_t                  count;      
    SYN_WDT_TimeoutCallback callback;   
    void                    *ctx;        
    SYN_ErrLog             *errlog;     
} SYN_Watchdog;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_watchdog_init(SYN_Watchdog *wdt,
                        SYN_WDT_Entry *entries, uint8_t capacity,
                        SYN_WDT_TimeoutCallback callback, void *ctx);

int8_t syn_watchdog_register(SYN_Watchdog *wdt, const char *name,
                              uint32_t timeout_ms);

void syn_watchdog_checkin(SYN_Watchdog *wdt, int8_t id);

void syn_watchdog_unregister(SYN_Watchdog *wdt, int8_t id);

void syn_watchdog_update(SYN_Watchdog *wdt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WATCHDOG_H */
```


