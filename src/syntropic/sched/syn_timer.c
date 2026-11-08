#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER

/**
 * @file syn_timer.c
 * @brief Software timer implementation.
 */

#include "../port/syn_port_system.h"
#include "syn_timer.h"

/**
 * @brief Clamp software timer period into valid range [1, INT32_MAX].
 * @param period_ms Requested period in milliseconds.
 * @return Clamped period in milliseconds.
 */
static uint32_t timer_clamp_period(uint32_t period_ms)
{
    if (period_ms == 0U) {
        return 1U;
    }
    if (period_ms > 0x7FFFFFFFU) {
        return 0x7FFFFFFFU;
    }
    return period_ms;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_timer_init(SYN_Timer *timer, uint32_t period_ms, bool periodic, SYN_TimerCallback callback,
                    void *user_data)
{
    if (timer == NULL) {
        return;
    }

    timer->period_ms = timer_clamp_period(period_ms);
    timer->target_tick = 0U;
    timer->callback = callback;
    timer->user_data = user_data;
    timer->periodic = periodic;
    timer->active = false;
}

void syn_timer_start(SYN_Timer *timer)
{
    if (timer == NULL) {
        return;
    }

    timer->target_tick = syn_port_get_tick_ms() + timer->period_ms;
    timer->active = true;
}

void syn_timer_stop(SYN_Timer *timer)
{
    if (timer == NULL) {
        return;
    }
    timer->active = false;
}

void syn_timer_set_period(SYN_Timer *timer, uint32_t period_ms)
{
    if (timer == NULL) {
        return;
    }
    timer->period_ms = timer_clamp_period(period_ms);
}

bool syn_timer_expired(SYN_Timer *timer)
{
    if (timer == NULL || !timer->active) {
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
    if (timers == NULL || count == 0U) {
        return;
    }

    for (size_t i = 0U; i < count; i++) {
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
    if (timer == NULL || !timer->active) {
        return 0U;
    }

    uint32_t now = syn_port_get_tick_ms();
    int32_t diff = (int32_t)(timer->target_tick - now);

    return (diff > 0) ? (uint32_t)diff : 0U;
}

uint32_t syn_timer_next_expiry(const SYN_Timer *timers, size_t count)
{
    if (timers == NULL || count == 0U) {
        return UINT32_MAX;
    }

    uint32_t now = syn_port_get_tick_ms();
    uint32_t earliest = UINT32_MAX;
    bool any_ready_now = false;

    for (size_t i = 0U; i < count; i++) {
        if (!timers[i].active) {
            continue;
        }

        /* Check if this timer has already expired */
        if ((int32_t)(now - timers[i].target_tick) >= 0) {
            any_ready_now = true;
            continue;
        }

        /* Timer is in the future — track the earliest (avoid UINT32_MAX sentinel alias) */
        uint32_t target =
            (timers[i].target_tick == UINT32_MAX) ? (UINT32_MAX - 1U) : timers[i].target_tick;
        if (earliest == UINT32_MAX || (int32_t)(target - earliest) < 0) {
            earliest = target;
        }
    }

    /* If any timer is ready right now, return 'now' */
    if (any_ready_now) {
        return now;
    }

    return earliest;
}

#endif /* SYN_USE_TIMER */
