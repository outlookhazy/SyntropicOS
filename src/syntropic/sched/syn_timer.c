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

    uint32_t clamped = timer_clamp_period(period_ms);
    syn_port_enter_critical();
    timer->period_ms = clamped;
    timer->target_tick = 0U;
    timer->callback = callback;
    timer->user_data = user_data;
    timer->periodic = periodic;
    timer->active = false;
    syn_port_exit_critical();
}

void syn_timer_start(SYN_Timer *timer)
{
    if (timer == NULL) {
        return;
    }

    syn_port_enter_critical();
    uint32_t period = timer->period_ms;
    timer->target_tick = syn_port_get_tick_ms() + period;
    timer->active = true;
    syn_port_exit_critical();
}

void syn_timer_stop(SYN_Timer *timer)
{
    if (timer == NULL) {
        return;
    }
    syn_port_enter_critical();
    timer->active = false;
    syn_port_exit_critical();
}

void syn_timer_set_period(SYN_Timer *timer, uint32_t period_ms)
{
    if (timer == NULL) {
        return;
    }
    uint32_t clamped = timer_clamp_period(period_ms);
    syn_port_enter_critical();
    timer->period_ms = clamped;
    syn_port_exit_critical();
}

/**
 * @brief Atomically check expiry, re-arm or disarm timer, and snapshot callback pointers.
 * @param timer  Pointer to timer descriptor.
 * @param out_cb Optional output pointer receiving snapshot of callback function.
 * @param out_ud Optional output pointer receiving snapshot of callback user_data.
 * @return true if timer expired on this check, false otherwise.
 */
static bool timer_check_and_rearm(SYN_Timer *timer, SYN_TimerCallback *out_cb, void **out_ud)
{
    bool fired = false;
    syn_port_enter_critical();
    if (timer->active) {
        uint32_t now = syn_port_get_tick_ms();
        uint32_t target = timer->target_tick;
        if ((int32_t)(now - target) >= 0) {
            if (timer->periodic) {
                uint32_t period = timer->period_ms;
                target += period;
                if ((int32_t)(now - target) >= 0) {
                    target = now + period;
                }
                timer->target_tick = target;
            } else {
                timer->active = false;
            }
            if (out_cb != NULL) {
                *out_cb = timer->callback;
                *out_ud = timer->user_data;
            }
            fired = true;
        }
    }
    syn_port_exit_critical();
    return fired;
}

bool syn_timer_expired(SYN_Timer *timer)
{
    if (timer == NULL) {
        return false;
    }
    return timer_check_and_rearm(timer, NULL, NULL);
}

void syn_timer_service(SYN_Timer *timers, size_t count)
{
    if (timers == NULL || count == 0U) {
        return;
    }

    for (size_t i = 0U; i < count; i++) {
        SYN_Timer *t = &timers[i];
        SYN_TimerCallback cb = NULL;
        void *ud = NULL;

        if (timer_check_and_rearm(t, &cb, &ud) && cb != NULL) {
            cb(t, ud);
        }
    }
}

uint32_t syn_timer_remaining(const SYN_Timer *timer)
{
    if (timer == NULL) {
        return 0U;
    }

    syn_port_enter_critical();
    bool is_active = timer->active;
    uint32_t target = timer->target_tick;
    uint32_t now = syn_port_get_tick_ms();
    syn_port_exit_critical();

    if (!is_active) {
        return 0U;
    }

    int32_t diff = (int32_t)(target - now);

    return (diff > 0) ? (uint32_t)diff : 0U;
}

uint32_t syn_timer_next_expiry(const SYN_Timer *timers, size_t count)
{
    if (timers == NULL || count == 0U) {
        return UINT32_MAX;
    }

    syn_port_enter_critical();
    uint32_t now = syn_port_get_tick_ms();
    uint32_t earliest = 0U;
    bool has_earliest = false;
    bool any_ready_now = false;

    for (size_t i = 0U; i < count; i++) {
        if (!timers[i].active) {
            continue;
        }

        uint32_t raw_target = timers[i].target_tick;
        /* Check if this timer has already expired */
        if ((int32_t)(now - raw_target) >= 0) {
            any_ready_now = true;
            continue;
        }

        if (!has_earliest || (int32_t)(raw_target - earliest) < 0) {
            earliest = raw_target;
            has_earliest = true;
        }
    }
    syn_port_exit_critical();

    /* If any timer is ready right now, return 'now' */
    if (any_ready_now) {
        return now;
    }
    if (!has_earliest) {
        return UINT32_MAX;
    }

    return (earliest == UINT32_MAX) ? (UINT32_MAX - 1U) : earliest;
}

#endif /* SYN_USE_TIMER */
