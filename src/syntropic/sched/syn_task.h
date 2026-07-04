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
 * @ingroup syn_sched
 */

#ifndef SYN_TASK_H
#define SYN_TASK_H

#include "../pt/syn_pt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task states ────────────────────────────────────────────────────────── */

/* Forward-declare SYN_EventGroup so tasks can hold a pointer to one
 * without pulling in the full event header.                          */
struct SYN_EventGroup;

/** @brief Cooperative task lifecycle state. */
typedef enum {
    SYN_TASK_READY     = 0,  /**< Eligible to run on next scheduler tick */
    SYN_TASK_SUSPENDED = 1,  /**< Paused — skipped by scheduler          */
    SYN_TASK_DEAD      = 2,  /**< Exited — will not run again            */
    SYN_TASK_DEFERRED  = 3,  /**< Deferred — skipped for one pass        */
    SYN_TASK_BLOCKED   = 4,  /**< Blocked on event — skipped until fired */
    SYN_TASK_WAITING   = 5,  /**< PT_WAIT condition false — skip this tick */
} SYN_TaskState;

/* ── Forward declaration ────────────────────────────────────────────────── */

struct SYN_Task;

/* ── Task function signature ────────────────────────────────────────────── */

/**
 * @brief Protothread task function.
 *
 * A task function receives its own protothread and task descriptor.
 * It must follow the PT_BEGIN / PT_END pattern.
 *
 * @param pt    Pointer to the task's protothread (same as &task->pt).
 * @param task  Pointer to the task descriptor (for user_data, delay, etc.).
 * @return PT status indicating whether the thread yielded, is waiting, or exited.
 */
typedef SYN_PT_Status (*SYN_TaskFunc)(SYN_PT *pt, struct SYN_Task *task);

/* ── Task control block ─────────────────────────────────────────────────── */

/**
 * @brief Task descriptor — binds a protothread to scheduler metadata.
 *
 * Typical size: ~28 bytes on a 32-bit target.
 */
typedef struct SYN_Task {
    SYN_PT          pt;           /**< Protothread continuation (2 bytes)      */
    SYN_TaskFunc    func;         /**< The task's protothread function          */
    const char      *name;         /**< Human-readable name (for debug/logging)  */
    uint8_t          priority;     /**< 0 = highest priority                     */
    uint8_t          state;        /**< SYN_TaskState                           */
    uint32_t         delay_until;  /**< Tick deadline for PT_TASK_DELAY_MS       */
    void            *user_data;    /**< Optional pointer to task-private state   */
    struct SYN_EventGroup *wait_event;  /**< Event group task blocks on (NULL if not blocking) */
    uint32_t         wait_mask;    /**< Bitmask of event flags to wait for       */
} SYN_Task;

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_H */
