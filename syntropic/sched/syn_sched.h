/**
 * @file syn_sched.h
 * @brief Cooperative scheduler for protothread tasks.
 *
 * The scheduler manages an array of SYN_Task descriptors. On each
 * tick it selects the highest-priority ready task and calls its
 * protothread function. Equal-priority tasks are served round-robin.
 *
 * @par Usage
 * @code
 *   static SYN_Task tasks[3];
 *   static SYN_Sched sched;
 *
 *   syn_task_create(&tasks[0], "blink",   blink_fn,   1, NULL);
 *   syn_task_create(&tasks[1], "serial",  serial_fn,  0, NULL);
 *   syn_task_create(&tasks[2], "monitor", monitor_fn, 2, NULL);
 *
 *   syn_sched_init(&sched, tasks, 3);
 *   syn_sched_run_forever(&sched);
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_SCHED_H
#define SYN_SCHED_H

#include "syn_task.h"
#include "../common/syn_compiler.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Scheduler struct ───────────────────────────────────────────────────── */

/**
 * @brief Scheduler control block.
 *
 * The scheduler does not own the task array — you allocate it and pass
 * a pointer. This means zero hidden allocation.
 */
typedef struct {
    SYN_Task  *tasks;         /**< Pointer to caller-owned task array    */
    size_t      task_count;    /**< Number of tasks in the array          */
    size_t      rr_index;      /**< Round-robin index for equal priority  */
} SYN_Sched;

/* ── Initialization ─────────────────────────────────────────────────────── */

/**
 * @brief Initialize the scheduler with a task array.
 *
 * @param sched  Scheduler to initialize.
 * @param tasks  Pointer to an array of SYN_Task structs. Each task
 *               should already be initialized via syn_task_create().
 * @param count  Number of tasks in the array.
 */
void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count);

/**
 * @brief Initialize a single task descriptor.
 *
 * Sets the task to READY state with the protothread reset to the top.
 *
 * @param task       Task to initialize.
 * @param name       Human-readable name (stored by pointer, not copied).
 * @param func       Protothread function.
 * @param priority   Priority level (0 = highest).
 * @param user_data  Optional pointer to task-private data (or NULL).
 */
void syn_task_create(SYN_Task *task,
                      const char *name,
                      SYN_TaskFunc func,
                      uint8_t priority,
                      void *user_data);

/* ── Scheduler execution ────────────────────────────────────────────────── */

/**
 * @brief Run one scheduler tick.
 *
 * Scans all tasks and runs every ready task once, in priority order
 * (highest priority first, round-robin among equal priorities).
 *
 * @param sched  Scheduler to run.
 * @return true if at least one task is still alive (not DEAD).
 */
bool syn_sched_run(SYN_Sched *sched);

/**
 * @brief Run the scheduler forever.
 *
 * Equivalent to `while (1) { syn_sched_run(sched); }`. This function
 * never returns.
 *
 * @param sched  Scheduler to run.
 */
SYN_NORETURN void syn_sched_run_forever(SYN_Sched *sched);

/* ── Task control ───────────────────────────────────────────────────────── */

/**
 * @brief Suspend a task. It will be skipped by the scheduler until
 *        resumed.
 * @param task  Task to suspend.
 */
void syn_task_suspend(SYN_Task *task);

/**
 * @brief Resume a suspended task, making it eligible to run again.
 * @param task  Task to resume.
 */
void syn_task_resume(SYN_Task *task);

/**
 * @brief Restart a task from the beginning of its protothread.
 *
 * Resets the protothread continuation and sets the task to READY.
 *
 * @param task  Task to restart.
 */
void syn_task_restart(SYN_Task *task);

/**
 * @brief Check if a task is still alive (not DEAD).
 * @param task  Task to check.
 * @return true if alive.
 */
static inline bool syn_task_is_alive(const SYN_Task *task)
{
    return task->state != (uint8_t)SYN_TASK_DEAD;
}

/**
 * @brief Get the number of tasks that are still alive in the scheduler.
 * @param sched  Scheduler.
 * @return Alive task count.
 */
size_t syn_sched_alive_count(const SYN_Sched *sched);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SCHED_H */
