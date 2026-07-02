

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

#include <limits.h>

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(tasks != NULL || count == 0);

    sched->tasks      = tasks;
    sched->task_count = count;
    sched->rr_index   = 0;
}

void syn_task_create(SYN_Task *task,
                      const char *name,
                      SYN_TaskFunc func,
                      uint8_t priority,
                      void *user_data)
{
    SYN_ASSERT(task != NULL);
    SYN_ASSERT(func != NULL);

    PT_INIT(&task->pt);
    task->func        = func;
    task->name        = name;
    task->priority    = priority;
    task->state       = (uint8_t)SYN_TASK_READY;
    task->delay_until = 0;
    task->user_data   = user_data;
}


static void sched_run_task(SYN_Task *task)
{
    SYN_PT_Status status = task->func(&task->pt, task);

    if (status == PT_EXITED || status == PT_ENDED) {
        task->state = (uint8_t)SYN_TASK_DEAD;
    }
    /* PT_YIELDED and PT_WAITING: task stays READY */
}

/* ── Scheduler tick ─────────────────────────────────────────────────────── */

bool syn_sched_run(SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    if (sched->task_count == 0) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    bool any_alive = false;

    /*
     * Priority scan: iterate all tasks starting from rr_index.
     * Find the ready task with the highest priority (lowest value).
     * Round-robin fairness among equal-priority tasks is achieved by
     * starting the search at rr_index.
     */
    size_t start = sched->rr_index;
    
    SYN_Task *best_task = NULL;
    size_t best_idx = 0;
    uint8_t best_prio = 255;

    for (size_t i = 0; i < sched->task_count; i++) {
        size_t idx = start + i;
        if (idx >= sched->task_count) idx -= sched->task_count;
        SYN_Task *task = &sched->tasks[idx];

        if (task->state == (uint8_t)SYN_TASK_DEAD) {
            continue;
        }

        any_alive = true;

        if (task->state == (uint8_t)SYN_TASK_SUSPENDED) {
            continue;
        }

        /* Use signed arithmetic to handle timer wraparound safely */
        if (task->delay_until != 0 && (int32_t)(now - task->delay_until) < 0) {
            continue;  /* still waiting */
        }

        /* Task is READY */
        if (task->priority < best_prio) {
            best_prio = task->priority;
            best_task = task;
            best_idx = idx;
        }
    }

    if (best_task != NULL) {
        sched_run_task(best_task);
        size_t next_rr = best_idx + 1;
        sched->rr_index = (next_rr >= sched->task_count) ? 0 : next_rr;
    } else {
        /* Advance round-robin index even if no task ran */
        size_t next_rr = start + 1;
        sched->rr_index = (next_rr >= sched->task_count) ? 0 : next_rr;
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

        /* This task is in the future — track the earliest */
        if ((int32_t)(task->delay_until - earliest) < 0 ||
            earliest == UINT32_MAX) {
            earliest = task->delay_until;
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


