#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED

/**
 * @file syn_sched.c
 * @brief Cooperative scheduler implementation.
 */

#include "syn_sched.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

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


/**
 * @brief Execute a single task's protothread function.
 * @param task  Task to run.
 */
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
