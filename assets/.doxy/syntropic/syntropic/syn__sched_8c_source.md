

# File syn\_sched.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sched.c**](syn__sched_8c.md)

[Go to the documentation of this file](syn__sched_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED

#include "syn_sched.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_event.h"
#include "../util/syn_metrics.h"

#include <limits.h>

#if SYN_USE_METRICS
SYN_METRIC_DECLARE(sched_ticks,    "sched_ticks",    "Total scheduler run calls", SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(sched_switches, "sched_switches", "Total task executions",    SYN_METRIC_TYPE_COUNTER);
#endif

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(tasks != NULL || count == 0);

    sched->tasks      = tasks;
    sched->task_count = count;

    for (size_t i = 0; i < SYN_SCHED_PRIO_LEVELS; i++) {
        sched->rr_per_prio[i] = 0;
    }

    SYN_METRIC_REGISTER(sched_ticks);
    SYN_METRIC_REGISTER(sched_switches);
}

void syn_task_create(SYN_Task *task,
                      const char *name,
                      SYN_TaskFunc func,
                      uint8_t priority,
                      void *user_data)
{
    SYN_ASSERT(task != NULL);
    SYN_ASSERT(func != NULL);
    SYN_ASSERT(priority < SYN_SCHED_PRIO_LEVELS);

    PT_INIT(&task->pt);
    task->func        = func;
    task->name        = name;
    task->priority    = priority;
    task->state       = (uint8_t)SYN_TASK_READY;
    task->delay_until = 0;
    task->user_data   = user_data;
    task->wait_event  = NULL;
    task->wait_mask   = 0;
}


static SYN_PT_Status sched_run_task(SYN_Task *task)
{
    SYN_PT_Status status = task->func(&task->pt, task);

    if (status == PT_EXITED || status == PT_ENDED) {
        task->state = (uint8_t)SYN_TASK_DEAD;
    }
    /* PT_YIELDED and PT_WAITING: caller decides next action */
    return status;
}

/* ── Scheduler tick ─────────────────────────────────────────────────────── */

bool syn_sched_run(SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    const size_t n = sched->task_count;

    if (n == 0) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    bool any_alive = false;
    SYN_METRIC_INC(sched_ticks);

    /*
     * Priority scan with retry on PT_WAITING.
     *
     * When a task returns PT_WAITING its condition was false — it couldn't
     * execute.  We set its state to SYN_TASK_WAITING (skipped for the
     * remainder of this tick only) and immediately re-scan for the next
     * best candidate.  This prevents high-priority waiting tasks from
     * starving lower-priority tasks that CAN do useful work.
     *
     * SYN_TASK_WAITING is separate from SYN_TASK_DEFERRED (used by
     * PT_DEFER) — DEFERRED persists across ticks, WAITING is tick-local.
     *
     * Bounded by task_count so we stop if every task is waiting.
     */
    const SYN_Task *executed_task = NULL;
    size_t    executed_idx  = 0;
    size_t    attempts      = 0;

    while (attempts < n) {
        SYN_Task *best_task = NULL;
        size_t    best_idx  = 0;
        uint8_t   best_prio = 255;
        size_t    best_dist = n;   /* Larger than any valid distance */

        for (size_t i = 0; i < n; i++) {
            SYN_Task *task = &sched->tasks[i];

            if (task->state == (uint8_t)SYN_TASK_DEAD) {
                continue;
            }

            any_alive = true;

            if (task->state == (uint8_t)SYN_TASK_SUSPENDED ||
                task->state == (uint8_t)SYN_TASK_DEFERRED  ||
                task->state == (uint8_t)SYN_TASK_WAITING) {
                continue;
            }

            /* Blocked on event — check if the event has fired */
            if (task->state == (uint8_t)SYN_TASK_BLOCKED) {
                if (task->wait_event != NULL &&
                    (syn_event_flags_get(task->wait_event) & task->wait_mask)) {
                    /* Event fired — transition to READY */
                    task->wait_event = NULL;
                    task->state = (uint8_t)SYN_TASK_READY;
                    /* Fall through to normal priority evaluation */
                } else {
                    continue;  /* Still blocked */
                }
            }

            /* Delay check — signed arithmetic for wraparound safety */
            if (task->delay_until != 0 && (int32_t)(now - task->delay_until) < 0) {
                continue;
            }

            /* Task is ready — compute rotation distance for its priority */
            const uint8_t prio = task->priority;
            size_t rr_start = sched->rr_per_prio[prio];
            if (rr_start >= n) { rr_start = 0; } /* Defensive clamp */

            const size_t dist = (i >= rr_start)
                              ? (i - rr_start)
                              : (n - rr_start + i);

            if (prio < best_prio ||
                (prio == best_prio && dist < best_dist)) {
                best_prio = prio;
                best_dist = dist;
                best_task = task;
                best_idx  = i;
            }
        }

        if (best_task == NULL) {
            break;  /* No eligible tasks this tick */
        }

        SYN_PT_Status status = sched_run_task(best_task);

        if (status == PT_WAITING) {
            /* Task's condition was false — it didn't do useful work.
             * Mark WAITING so it's skipped on re-scan, then try the
             * next candidate immediately within this same tick. */
            best_task->state = (uint8_t)SYN_TASK_WAITING;
            attempts++;
            continue;
        }

        /* Task did real work (or deferred itself via PT_DEFER). */
        executed_task = best_task;
        executed_idx  = best_idx;
        SYN_METRIC_INC(sched_switches);

        /* Advance round-robin unless the task deferred (PT_DEFER sets
         * state to DEFERRED before yielding — no RR advance). */
        if (best_task->state != (uint8_t)SYN_TASK_DEFERRED) {
            const size_t next = executed_idx + 1;
            sched->rr_per_prio[best_prio] = (next >= n) ? 0 : next;
        }
        break;  /* One useful execution per tick */
    }

    /* Clean up task states for the next tick:
     *
     * WAITING  → always cleared (tick-local, used for PT_WAITING retry).
     * DEFERRED → cleared UNLESS it's the task that just ran and deferred
     *            itself (PT_DEFER).  That task must stay DEFERRED so it's
     *            skipped on the NEXT tick — one-tick skip contract. */
    for (size_t i = 0; i < n; i++) {
        const uint8_t st = sched->tasks[i].state;
        if (st == (uint8_t)SYN_TASK_WAITING) {
            sched->tasks[i].state = (uint8_t)SYN_TASK_READY;
        } else if (st == (uint8_t)SYN_TASK_DEFERRED &&
                   &sched->tasks[i] != executed_task) {
            sched->tasks[i].state = (uint8_t)SYN_TASK_READY;
        }
    }

    return any_alive;
}

SYN_NORETURN void syn_sched_run_forever(SYN_Sched *sched)
{
    for (;;) {
        syn_sched_run(sched);
    }
}

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS

uint32_t syn_sched_next_wakeup(const SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t earliest = UINT32_MAX;
    bool any_ready_now = false;

    for (size_t i = 0; i < sched->task_count; i++) {
        const SYN_Task *task = &sched->tasks[i];

        if (task->state == (uint8_t)SYN_TASK_DEAD ||
            task->state == (uint8_t)SYN_TASK_SUSPENDED) {
            continue;
        }

        /* Blocked on event — check if the event has fired since the
         * last syn_sched_run() (e.g. from an ISR or timer service). */
        if (task->state == (uint8_t)SYN_TASK_BLOCKED) {
            if (task->wait_event != NULL &&
                (syn_event_flags_get(task->wait_event) & task->wait_mask)) {
                any_ready_now = true;  /* Event fired — don't sleep */
            }
            continue;
        }

        if (task->delay_until == 0) {
            /* Task is ready immediately (no delay) */
            any_ready_now = true;
            continue;
        }

        /* Check if delay has already passed */
        if ((int32_t)(now - task->delay_until) >= 0) {
            any_ready_now = true;
            continue;
        }

        /* This task is in the future — track the earliest.
         * Cap at UINT32_MAX-1 so a real deadline at counter wrap
         * is never confused with the "no deadlines" sentinel. */
        uint32_t target = task->delay_until;
        if (target == UINT32_MAX) target = UINT32_MAX - 1;

        if ((int32_t)(target - earliest) < 0 ||
            earliest == UINT32_MAX) {
            earliest = target;
        }
    }

    /* If any task is ready right now, return 'now' to indicate no sleep */
    if (any_ready_now) {
        return now;
    }

    return earliest;
}

SYN_NORETURN void syn_sched_run_tickless(SYN_Sched *sched, SYN_Sleep *sleep)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(sleep != NULL);

    for (;;) {
        /* Run the scheduler — returns true if any tasks alive */
        syn_sched_run(sched);

        /* Check if we can sleep */
        uint32_t now = syn_port_get_tick_ms();
        uint32_t wake = syn_sched_next_wakeup(sched);

        /* Only sleep if no tasks are immediately ready */
        if (wake != now && !syn_sleep_any_locked(sleep)) {
            if (wake == UINT32_MAX) {
                /* No deadlines — light sleep until interrupt */
                syn_sleep_enter(sleep);
            } else {
                /* Sleep until the next deadline */
                syn_port_sleep_until(wake);
            }
        }
    }
}

#if defined(SYN_USE_TIMER) && SYN_USE_TIMER

SYN_NORETURN void syn_sched_run_tickless_ex(SYN_Sched *sched,
                                             SYN_Sleep *sleep,
                                             SYN_Timer *timers,
                                             size_t timer_count)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(sleep != NULL);

    for (;;) {
        /* Run the scheduler */
        syn_sched_run(sched);

        /* Service software timers */
        syn_timer_service(timers, timer_count);

        /* Compute sleep duration: min of task deadlines and timer expiries */
        uint32_t now = syn_port_get_tick_ms();
        uint32_t task_wake  = syn_sched_next_wakeup(sched);
        uint32_t timer_wake = syn_timer_next_expiry(timers, timer_count);

        /* Pick the earlier deadline */
        uint32_t wake = task_wake;
        if (timer_wake != UINT32_MAX &&
            (wake == UINT32_MAX || (int32_t)(timer_wake - wake) < 0)) {
            wake = timer_wake;
        }

        /* Only sleep if nothing is immediately ready */
        if (wake != now && !syn_sleep_any_locked(sleep)) {
            if (wake == UINT32_MAX) {
                /* No deadlines — light sleep until interrupt */
                syn_sleep_enter(sleep);
            } else {
                /* Sleep until the next deadline */
                syn_port_sleep_until(wake);
            }
        }
    }
}

#endif /* SYN_USE_TIMER */

#endif /* SYN_USE_TICKLESS */

/* ── Task control ───────────────────────────────────────────────────────── */

void syn_task_suspend(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    if (task->state != (uint8_t)SYN_TASK_DEAD) {
        task->state = (uint8_t)SYN_TASK_SUSPENDED;
    }
}

void syn_task_resume(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    if (task->state == (uint8_t)SYN_TASK_SUSPENDED) {
        task->state = (uint8_t)SYN_TASK_READY;
    }
}

void syn_task_restart(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    PT_INIT(&task->pt);
    task->delay_until = 0;
    task->wait_event  = NULL;
    task->wait_mask   = 0;
    task->state = (uint8_t)SYN_TASK_READY;
}

size_t syn_sched_alive_count(const SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    size_t count = 0;
    for (size_t i = 0; i < sched->task_count; i++) {
        if (sched->tasks[i].state != (uint8_t)SYN_TASK_DEAD) {
            count++;
        }
    }
    return count;
}

#endif /* SYN_USE_SCHED */
```


