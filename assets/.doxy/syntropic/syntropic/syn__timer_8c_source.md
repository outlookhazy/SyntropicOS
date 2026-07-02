

# File syn\_timer.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer.c**](syn__timer_8c.md)

[Go to the documentation of this file](syn__timer_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER

#include "syn_timer.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_timer_init(SYN_Timer *timer,
                     uint32_t period_ms,
                     bool periodic,
                     SYN_TimerCallback callback,
                     void *user_data)
{
    SYN_ASSERT(timer != NULL);
    SYN_ASSERT(period_ms > 0);

    timer->period_ms   = period_ms;
    timer->target_tick = 0;
    timer->callback    = callback;
    timer->user_data   = user_data;
    timer->periodic    = periodic;
    timer->active      = false;
}

void syn_timer_start(SYN_Timer *timer)
{
    SYN_ASSERT(timer != NULL);

    timer->target_tick = syn_port_get_tick_ms() + timer->period_ms;
    timer->active      = true;
}

void syn_timer_stop(SYN_Timer *timer)
{
    SYN_ASSERT(timer != NULL);
    timer->active = false;
}

void syn_timer_set_period(SYN_Timer *timer, uint32_t period_ms)
{
    SYN_ASSERT(timer != NULL);
    SYN_ASSERT(period_ms > 0);
    timer->period_ms = period_ms;
}

bool syn_timer_expired(SYN_Timer *timer)
{
    SYN_ASSERT(timer != NULL);

    if (!timer->active) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();

    /* Handle tick wrap-around: use unsigned subtraction */
    if ((int32_t)(now - timer->target_tick) >= 0) {
        if (timer->periodic) {
            /* Re-arm relative to the previous target to avoid drift */
            timer->target_tick += timer->period_ms;

            /*
             * If we've fallen behind by more than one period (e.g.,
             * the callback took too long), snap forward to avoid a
             * burst of rapid-fire expirations.
             */
            if ((int32_t)(now - timer->target_tick) >= 0) {
                timer->target_tick = now + timer->period_ms;
            }
        } else {
            timer->active = false;
        }
        return true;
    }

    return false;
}

void syn_timer_service(SYN_Timer *timers, size_t count)
{
    SYN_ASSERT(timers != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        SYN_Timer *t = &timers[i];

        if (!t->active) {
            continue;
        }

        if (syn_timer_expired(t)) {
            if (t->callback != NULL) {
                t->callback(t, t->user_data);
            }
        }
    }
}

uint32_t syn_timer_remaining(const SYN_Timer *timer)
{
    SYN_ASSERT(timer != NULL);

    if (!timer->active) {
        return 0;
    }

    uint32_t now = syn_port_get_tick_ms();
    int32_t diff = (int32_t)(timer->target_tick - now);

    return (diff > 0) ? (uint32_t)diff : 0;
}

uint32_t syn_timer_next_expiry(const SYN_Timer *timers, size_t count)
{
    SYN_ASSERT(timers != NULL || count == 0);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t earliest = UINT32_MAX;
    bool any_ready_now = false;

    for (size_t i = 0; i < count; i++) {
        if (!timers[i].active) continue;

        /* Check if this timer has already expired */
        if ((int32_t)(now - timers[i].target_tick) >= 0) {
            any_ready_now = true;
            continue;
        }

        /* Timer is in the future — track the earliest */
        if ((int32_t)(timers[i].target_tick - earliest) < 0 ||
            earliest == UINT32_MAX) {
            earliest = timers[i].target_tick;
        }
    }

    /* If any timer is ready right now, return 'now' */
    if (any_ready_now) {
        return now;
    }

    return earliest;
}

#endif /* SYN_USE_TIMER */
```


