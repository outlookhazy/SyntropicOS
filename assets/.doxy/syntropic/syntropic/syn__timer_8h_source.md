

# File syn\_timer.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer.h**](syn__timer_8h.md)

[Go to the documentation of this file](syn__timer_8h.md)


```C++

#ifndef SYN_TIMER_H
#define SYN_TIMER_H

#include "../common/syn_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Forward declaration ────────────────────────────────────────────────── */

struct SYN_Timer;

/* ── Callback type ──────────────────────────────────────────────────────── */

typedef void (*SYN_TimerCallback)(struct SYN_Timer *timer, void *user_data);

/* ── Timer struct ───────────────────────────────────────────────────────── */

typedef struct SYN_Timer {
    uint32_t            period_ms;    
    uint32_t            target_tick;  
    SYN_TimerCallback  callback;     
    void               *user_data;    
    bool                periodic;     
    bool                active;       
} SYN_Timer;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_timer_init(SYN_Timer *timer,
                     uint32_t period_ms,
                     bool periodic,
                     SYN_TimerCallback callback,
                     void *user_data);

void syn_timer_start(SYN_Timer *timer);

void syn_timer_stop(SYN_Timer *timer);

void syn_timer_set_period(SYN_Timer *timer, uint32_t period_ms);

bool syn_timer_expired(SYN_Timer *timer);

void syn_timer_service(SYN_Timer *timers, size_t count);

static inline bool syn_timer_is_active(const SYN_Timer *timer)
{
    return timer->active;
}

uint32_t syn_timer_remaining(const SYN_Timer *timer);

uint32_t syn_timer_next_expiry(const SYN_Timer *timers, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMER_H */
```


