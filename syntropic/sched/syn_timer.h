/**
 * @file syn_timer.h
 * @brief Software timer service — one-shot and periodic timers.
 *
 * Timers are caller-owned structs with a callback. They can be used
 * with or without the scheduler. In a bare main loop, call
 * syn_timer_service() each iteration. With the scheduler, run it
 * from a high-priority task.
 *
 * Timers are driven by syn_port_get_tick_ms() — resolution depends
 * on how often you call syn_timer_service().
 *
 * @par Usage
 * @code
 *   static SYN_Timer heartbeat;
 *
 *   void on_heartbeat(SYN_Timer *t, void *ctx) {
 *       syn_gpio_toggle(LED_PIN);
 *   }
 *
 *   syn_timer_init(&heartbeat, 1000, true, on_heartbeat, NULL);
 *   syn_timer_start(&heartbeat);
 *
 *   while (1) {
 *       syn_timer_service(&heartbeat, 1);
 *   }
 * @endcode
 * @ingroup syn_sched
 */

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

/**
 * @brief Timer expiry callback.
 *
 * @param timer     The timer that expired.
 * @param user_data User-provided context pointer.
 */
typedef void (*SYN_TimerCallback)(struct SYN_Timer *timer, void *user_data);

/* ── Timer struct ───────────────────────────────────────────────────────── */

/**
 * @brief Software timer descriptor.
 */
typedef struct SYN_Timer {
    uint32_t            period_ms;    /**< Timer period in milliseconds      */
    uint32_t            target_tick;  /**< Next expiry tick                   */
    SYN_TimerCallback  callback;     /**< Called on expiry (may be NULL)     */
    void               *user_data;    /**< User context for callback          */
    bool                periodic;     /**< true = auto-repeat, false = one-shot */
    bool                active;       /**< Currently running?                 */
} SYN_Timer;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a software timer.
 *
 * The timer is created in the stopped state. Call syn_timer_start()
 * to begin counting.
 *
 * @param timer      Timer to initialize.
 * @param period_ms  Period in milliseconds.
 * @param periodic   true for repeating timer, false for one-shot.
 * @param callback   Function called on expiry (may be NULL if using polling).
 * @param user_data  User context pointer passed to callback.
 */
void syn_timer_init(SYN_Timer *timer,
                     uint32_t period_ms,
                     bool periodic,
                     SYN_TimerCallback callback,
                     void *user_data);

/**
 * @brief Start (or restart) the timer.
 *
 * Sets the target tick to now + period_ms and marks the timer active.
 *
 * @param timer  Timer to start.
 */
void syn_timer_start(SYN_Timer *timer);

/**
 * @brief Stop the timer. It will not fire until started again.
 * @param timer  Timer to stop.
 */
void syn_timer_stop(SYN_Timer *timer);

/**
 * @brief Change the timer's period. Takes effect on next start or
 *        next periodic reload.
 * @param timer      Timer.
 * @param period_ms  New period in milliseconds.
 */
void syn_timer_set_period(SYN_Timer *timer, uint32_t period_ms);

/**
 * @brief Poll whether the timer has expired.
 *
 * If expired and periodic, the timer is automatically re-armed.
 * If expired and one-shot, the timer is stopped.
 *
 * This can be used instead of (or in addition to) the callback.
 *
 * @param timer  Timer to check.
 * @return true if the timer has expired since the last check.
 */
bool syn_timer_expired(SYN_Timer *timer);

/**
 * @brief Service an array of timers.
 *
 * Checks each timer for expiry and calls its callback if set.
 * Call this from your main loop, a scheduler task, or a periodic ISR.
 *
 * @param timers  Array of timers.
 * @param count   Number of timers in the array.
 */
void syn_timer_service(SYN_Timer *timers, size_t count);

/**
 * @brief Check if a timer is currently active (running).
 * @param timer  Timer to query.
 * @return true if active.
 */
static inline bool syn_timer_is_active(const SYN_Timer *timer)
{
    return timer->active;
}

/**
 * @brief Get the remaining time until the next expiry.
 *
 * @param timer  Timer to query.
 * @return Remaining milliseconds, or 0 if expired/stopped.
 */
uint32_t syn_timer_remaining(const SYN_Timer *timer);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMER_H */
