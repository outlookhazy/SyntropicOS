/**
 * @file syn_task.h
 * @brief Task descriptor for the cooperative scheduler.
 *
 * Defines the task control block (SYN_Task) that pairs a protothread
 * with scheduling metadata: priority, state, name, delay target, and
 * optional event-wait fields for true blocking.
 *
 * Tasks are caller-owned — you allocate them however you like (static
 * array, global, on the stack). The scheduler just takes a pointer to
 * your array.
 *
 * @par Task restart and static local state
 * syn_task_restart() resets the protothread continuation (lc), delay
 * deadlines, and event wait masks. It does NOT reset static local variables
 * inside the task function. Place all state initialization after PT_BEGIN()
 * or reset static state inside task-private user_data structs to ensure clean
 * restarts.
 * @ingroup syn_sched
 */

#ifndef SYN_TASK_H
#define SYN_TASK_H

#include "../pt/syn_pt.h"
#include "syn_event_flags.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task priorities ────────────────────────────────────────────────────── */

/** @brief Cooperative task priority levels (0 = highest). */
typedef enum {
    SYN_PRIO_CRITICAL = 0,   /**< Latency-critical polling/control (1 ms default max wait) */
    SYN_PRIO_HIGH = 1,       /**< Protocol I/O and event handlers (5 ms default max wait)  */
    SYN_PRIO_NORMAL = 2,     /**< Standard application tasks (15 ms default max wait)      */
    SYN_PRIO_LOW = 3,        /**< Background / batch processing (50 ms default max wait)   */
    SYN_PRIO_LEVEL_MAX = 255 /**< Upper bound sentinel for custom SYN_SCHED_PRIO_LEVELS    */
} SYN_Prio;

/* ── Task states ────────────────────────────────────────────────────────── */

/** @brief Cooperative task lifecycle state. */
typedef enum {
    SYN_TASK_READY = 0,     /**< Eligible to run when not delayed       */
    SYN_TASK_SUSPENDED = 1, /**< Paused — skipped by scheduler          */
    SYN_TASK_DEAD = 2,      /**< Exited — will not run again            */
    SYN_TASK_BLOCKED = 3,   /**< Blocked on event — skipped until fired */
} SYN_TaskState;

/* ── Scheduler internal flags (stored in SYN_Task.sched_flags) ──────────── */

/** @brief Task reported PT_MORE on last run, woke from sleep, or was boosted. */
#define SYN_SCHED_FLAG_MORE 0x01U

/** @brief Task reported PT_IDLE during the current polling round. */
#define SYN_SCHED_FLAG_PASSED 0x02U

/** @brief Task was delayed, blocked, or suspended after its last dispatch. */
#define SYN_SCHED_FLAG_SLEEPING 0x04U

/** @brief Task due_ms has been stamped by the scheduler. */
#define SYN_SCHED_FLAG_DUE_VALID 0x08U

/** @brief Task was resumed asynchronously during its current dispatch. */
#define SYN_SCHED_FLAG_ISR_WAKE 0x10U

/** @brief Task is actively contended in a SYN_Mutex waiter list. */
#define SYN_SCHED_FLAG_MUTEX_WAIT 0x20U

/** @brief Task called syn_task_restart() on itself during its current dispatch. */
#define SYN_SCHED_FLAG_RESTARTED 0x40U

/** @brief Sentinel value for SYN_Task.manual_priority when no manual boost is active. */
#define SYN_TASK_NO_MANUAL_PRIO 0xFFU

/* ── Forward declarations ───────────────────────────────────────────────── */

struct SYN_Task;
struct SYN_Sched;
struct SYN_Mutex;

/* ── Task function signature ────────────────────────────────────────────── */

/**
 * @brief Protothread task function.
 *
 * A task function receives its own protothread and task descriptor.
 * It must follow the PT_BEGIN / PT_END pattern.
 *
 * @param pt    Pointer to the task's protothread (same as &task->pt).
 * @param task  Pointer to the task descriptor (for user_data, delay, etc.).
 * @return Protothread status (PT_IDLE, PT_MORE, PT_EXITED, or PT_ENDED).
 */
typedef SYN_PT_Status (*SYN_TaskFunc)(SYN_PT *pt, struct SYN_Task *task);

/* ── Task control block ─────────────────────────────────────────────────── */

/**
 * @brief Task descriptor — binds a protothread to scheduler metadata.
 *
 * Size: 44 bytes on a 32-bit target (80 bytes on 64-bit).
 */
typedef struct SYN_Task {
    SYN_PT pt;                      /**< Protothread continuation (2 bytes)      */
    uint8_t manual_priority;        /**< Manual boost priority (0xFF = none)     */
    SYN_TaskFunc func;              /**< The task's protothread function          */
    const char *name;               /**< Human-readable name (for debug/logging)  */
    uint8_t priority;               /**< Active priority (0 = highest)            */
    uint8_t base_priority;          /**< Configured base priority                 */
    volatile uint8_t state;         /**< SYN_TaskState                           */
    volatile uint8_t sched_flags;   /**< Internal scheduler rank/round flags      */
    uint32_t delay_until;           /**< Tick deadline for PT_TASK_DELAY_MS       */
    uint32_t due_ms;                /**< Starvation deadline tick for scheduler   */
    struct SYN_Sched *sched;        /**< Owning scheduler instance (or NULL)      */
    struct SYN_Mutex *held_mutexes; /**< Intrusive list of owned SYN_Mutex locks  */
    void *user_data;                /**< Optional pointer to task-private state   */
    SYN_EventFlags *wait_event;     /**< Event flags task blocks on (NULL if not blocking) */
    uint32_t wait_mask;             /**< Bitmask of event flags to wait for       */
} SYN_Task;

/** @cond INTERNAL */
static inline bool syn_task_arm_event_wait(SYN_Task *task, SYN_EventFlags *grp, uint32_t mask,
                                           bool use_timeout, uint32_t timeout_ms,
                                           uint32_t *out_matched)
{
    if (out_matched != NULL) {
        *out_matched = 0U;
    }
    if ((task == NULL) || (grp == NULL) || (mask == 0U)) {
        return false;
    }

    syn_port_enter_critical();
    if (syn_event_flags_wait(
            grp, mask, (uint32_t)SYN_EVENT_FLAGS_WAIT_ANY | (uint32_t)SYN_EVENT_FLAGS_AUTO_CLEAR,
            out_matched) == SYN_OK) {
        syn_port_exit_critical();
        return false;
    }

    if (use_timeout &&
        !syn_pt_arm_ms_deadline(&task->delay_until, syn_port_get_tick_ms(), timeout_ms)) {
        syn_port_exit_critical();
        return false;
    }

    task->wait_event = grp;
    task->wait_mask = mask;
    SYN_PT_STATE_BARRIER();
    syn_port_exit_critical();
    return true;
}

static inline void syn_task_park_blocked(SYN_Task *task)
{
    syn_port_enter_critical();
    if (task->state == (uint8_t)SYN_TASK_READY) {
        task->state = (uint8_t)SYN_TASK_BLOCKED;
        SYN_PT_STATE_BARRIER();
    }
    syn_port_exit_critical();
}
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_H */
