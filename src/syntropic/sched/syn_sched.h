/**
 * @file syn_sched.h
 * @brief Cooperative scheduler for protothread tasks.
 *
 * The scheduler manages an array of SYN_Task descriptors. Each call to
 * syn_sched_run() dispatches one eligible task using a ranked policy:
 * Overdue (starvation bound reached) -> Work (PT_MORE) -> Poll (unpolled
 * this round) -> Round reset (all ready tasks passed). Within every rank,
 * tasks are ordered by priority and then round-robin.
 *
 * Every eligible task runs within max_wait_ms[prio] plus the duration of
 * the currently executing cooperative chunk plus any higher-priority overdue
 * tasks ahead of it (soft bound; hard latency belongs in ISRs).
 *
 * @par Usage
 * @code
 *   static SYN_Task tasks[3];
 *   static SYN_Sched sched;
 *
 *   syn_task_create(&tasks[0], "blink",   blink_fn,   SYN_PRIO_HIGH,     NULL);
 *   syn_task_create(&tasks[1], "serial",  serial_fn,  SYN_PRIO_CRITICAL, NULL);
 *   syn_task_create(&tasks[2], "monitor", monitor_fn, SYN_PRIO_NORMAL,   NULL);
 *
 *   syn_sched_init(&sched, tasks, 3);
 *   syn_sched_run_forever(&sched);
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_SCHED_H
#define SYN_SCHED_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "../common/syn_compiler.h"
#include "syn_task.h"

#include <stdbool.h>

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS && (!defined(SYN_USE_TIMER) || SYN_USE_TIMER)
#include "syn_timer.h"
#endif
#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS
#include "../system/syn_sleep.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief Maximum number of priority levels (0 to N-1).
 *  Override in syn_config.h if more are needed (valid range: 1..255). */
#ifndef SYN_SCHED_PRIO_LEVELS
#define SYN_SCHED_PRIO_LEVELS 4
#endif

#if (SYN_SCHED_PRIO_LEVELS < 1) || (SYN_SCHED_PRIO_LEVELS > 255)
#error "SYN_SCHED_PRIO_LEVELS must be between 1 and 255"
#endif

/** @brief Default maximum wait window for SYN_PRIO_CRITICAL (ms). */
#ifndef SYN_SCHED_MAX_WAIT_CRITICAL_MS
#define SYN_SCHED_MAX_WAIT_CRITICAL_MS 1U
#endif

/** @brief Default maximum wait window for SYN_PRIO_HIGH (ms). */
#ifndef SYN_SCHED_MAX_WAIT_HIGH_MS
#define SYN_SCHED_MAX_WAIT_HIGH_MS 5U
#endif

/** @brief Default maximum wait window for SYN_PRIO_NORMAL (ms). */
#ifndef SYN_SCHED_MAX_WAIT_NORMAL_MS
#define SYN_SCHED_MAX_WAIT_NORMAL_MS 15U
#endif

/** @brief Default maximum wait window for SYN_PRIO_LOW (ms). */
#ifndef SYN_SCHED_MAX_WAIT_LOW_MS
#define SYN_SCHED_MAX_WAIT_LOW_MS 50U
#endif

/* ── Scheduler struct ───────────────────────────────────────────────────── */

/**
 * @brief Scheduler control block.
 *
 * The scheduler does not own the task array — you allocate it and pass
 * a pointer. This means zero hidden allocation.
 */
typedef struct SYN_Sched {
    SYN_Task *tasks;                             /**< Pointer to caller-owned task array    */
    size_t task_count;                           /**< Number of tasks in the array          */
    uint16_t rr_per_prio[SYN_SCHED_PRIO_LEVELS]; /**< Per-priority round-robin indices      */
    uint16_t max_wait_ms[SYN_SCHED_PRIO_LEVELS]; /**< Per-priority starvation bounds (ms)   */
    uint32_t share_deadline_ms;                  /**< Earliest peer due_ms during dispatch  */
    SYN_Task *running;                           /**< Task currently executing (or NULL)    */
    struct SYN_Mutex *standalone_mutexes;        /**< Locked ownerless mutexes with waiters */
    bool share_contended;                        /**< Equal/higher-prio MORE peer waiting   */
    bool share_has_deadline;                     /**< True when share_deadline_ms is valid  */
    bool share_watch_wakeups;                    /**< Watch blocked/suspended urgent peers  */
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
 * @brief Configure the maximum starvation wait window for a priority level.
 *
 * Values below 1 are clamped to 1 ms because tick resolution is 1 ms.
 *
 * @param sched  Scheduler instance.
 * @param prio   Priority level to configure.
 * @param ms     Maximum wait window in milliseconds (clamped to >= 1).
 */
void syn_sched_set_max_wait(SYN_Sched *sched, SYN_Prio prio, uint16_t ms);

/**
 * @brief Initialize a single task descriptor.
 *
 * Sets the task to READY state with the protothread reset to the top.
 *
 * @param task       Task to initialize.
 * @param name       Human-readable name (stored by pointer, not copied).
 * @param func       Protothread function.
 * @param priority   Priority level (SYN_PRIO_CRITICAL = 0 is highest).
 * @param user_data  Optional pointer to task-private data (or NULL).
 */
void syn_task_create(SYN_Task *task, const char *name, SYN_TaskFunc func, SYN_Prio priority,
                     void *user_data);

/* ── Cooperative CPU sharing ────────────────────────────────────────────── */

/**
 * @brief Query whether the running task should yield to share CPU time.
 *
 * Returns true when another eligible task has active work (PT_MORE) at equal
 * or higher priority, or when any other eligible task has reached its
 * starvation deadline. Returns false when called outside @p task's dispatch.
 *
 * @param task  Task descriptor of the currently running task.
 * @return true if @p task should yield now, false otherwise.
 */
bool syn_task_should_share(const SYN_Task *task);

/**
 * @brief Yield with PT_MORE only when another task needs the CPU.
 *
 * @param pt    Protothread control block.
 * @param task  Pointer to the owning SYN_Task.
 */
#define PT_CONTINUE_IF_NEEDED(pt, task)    \
    do {                                   \
        if (syn_task_should_share(task)) { \
            PT_CONTINUE(pt);               \
        }                                  \
    } while (0)

/* ── Scheduler execution ────────────────────────────────────────────────── */

/**
 * @brief Run one scheduler dispatch.
 *
 * Selects and executes a single eligible task using ranked order
 * (Overdue -> Work -> Poll -> Round reset), ordered by priority and
 * per-priority round-robin within every rank.
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

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS

/**
 * @brief Compute the earliest tick at which a task becomes ready.
 *
 * Scans all non-dead, non-suspended tasks. Returns the current tick (`now`)
 * immediately if any task is currently runnable (including any READY task
 * parked in polled `PT_PASS` or `PT_WAIT_UNTIL` with `delay_until == 0`, or
 * any BLOCKED task whose event/timeout has fired). Otherwise returns the
 * minimum future `delay_until` tick (clamping `UINT32_MAX` to `UINT32_MAX - 1`),
 * or `UINT32_MAX` when all tasks are dead, suspended, or blocked without timeout.
 *
 * @note Any READY task using polled `PT_PASS` / `PT_WAIT_UNTIL` causes this
 *       function to return `now`, preventing tickless sleep; use `PT_BLOCK_*`
 *       or `PT_TASK_DELAY_MS` for tickless-compatible tasks.
 *
 * @param sched  Scheduler to query.
 * @return Current tick if any task is ready now, earliest future wakeup tick,
 *         or UINT32_MAX if no timed wakeups are pending.
 */
uint32_t syn_sched_next_wakeup(const SYN_Sched *sched);

/**
 * @brief Run the scheduler with tickless idle support.
 *
 * Like syn_sched_run_forever(), but enters low-power sleep when no
 * tasks are immediately ready and `sleep` is enabled, unlocked, and not
 * configured for `SYN_SLEEP_NONE`.
 *
 * @param sched  Scheduler to run.
 * @param sleep  Sleep coordinator (for wake-lock and mode checking).
 */
SYN_NORETURN void syn_sched_run_tickless(SYN_Sched *sched, SYN_Sleep *sleep);

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER
/**
 * @brief Run the scheduler with tickless idle + software timer awareness.
 *
 * Like syn_sched_run_tickless(), but also services software timers and sleeps
 * until the earlier of the next task deadline or the next timer expiry.
 *
 * @param sched        Scheduler to run.
 * @param sleep        Sleep coordinator (for wake-lock and mode checking).
 * @param timers       Array of software timers.
 * @param timer_count  Number of timers in the array.
 */
SYN_NORETURN void syn_sched_run_tickless_ex(SYN_Sched *sched, SYN_Sleep *sleep, SYN_Timer *timers,
                                            size_t timer_count);
#endif /* SYN_USE_TIMER */

#endif /* SYN_USE_TICKLESS */

/* ── Task control ───────────────────────────────────────────────────────── */

/**
 * @brief Suspend a task. It will be skipped by the scheduler until resumed.
 * @note Main-loop / task context only (not intended for arbitrary ISR use).
 * @param task  Task to suspend.
 */
void syn_task_suspend(SYN_Task *task);

/**
 * @brief Resume a suspended or condition-blocked task, making it eligible to run.
 * @note ISR-safe. Calling this from an ISR or driver wakes tasks parked in
 *       `PT_BLOCK_CONDITION` (and wrapper macros) or `syn_task_suspend()`.
 *       Tasks actively waiting on `PT_BLOCK_EVENT*` remain blocked until their
 *       event flag or timeout fires.
 * @param task  Task to resume.
 */
void syn_task_resume(SYN_Task *task);

/**
 * @brief Restart a task from the beginning of its protothread.
 *
 * Resets the protothread continuation, releases any held mutexes, restores base
 * priority, and sets the task to READY.
 *
 * @note Main-loop / task context only (not ISR-safe).
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

/**
 * @brief Temporarily boost a task's execution priority.
 *
 * Monotonically ratchets `task->priority` upward when `temp_prio < task->priority`
 * (`SYN_PRIO_CRITICAL = 0` is highest); calls with `temp_prio >= task->priority`
 * are ignored so active higher boosts are never lowered. Promotes the task to
 * Work rank and tightens `due_ms` to the boosted priority's starvation window
 * when bound to a scheduler.
 *
 * @note Main-loop / task context only (not ISR-safe).
 * @param task       Task to boost.
 * @param temp_prio  Temporary elevated priority.
 */
void syn_task_boost_priority(SYN_Task *task, SYN_Prio temp_prio);

/**
 * @brief Restore a task's priority back to its configured base priority
 *        (preserving any active mutex priority-inheritance boost).
 *
 * @note Main-loop / task context only (not ISR-safe).
 * @param task  Task to restore.
 */
void syn_task_restore_priority(SYN_Task *task);

/**
 * @brief Dynamically set a task's base priority and recompute active priority.
 *
 * Preserves any higher active mutex boost, and promotes Work rank / tightens
 * `due_ms` when raising effective priority.
 *
 * @note Main-loop / task context only (not ISR-safe).
 * @param task      Task to modify.
 * @param new_prio  New base priority.
 */
void syn_task_set_base_priority(SYN_Task *task, SYN_Prio new_prio);

/** @cond INTERNAL */
void syn_sched_boost_task_prio(SYN_Task *task);
void syn_sched_recompute_task_prio(SYN_Task *task);
void syn_sched_clear_mutex_wait(SYN_Task *task);
void syn_sched_refresh_share(SYN_Sched *sched);
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* SYN_SCHED_H */
