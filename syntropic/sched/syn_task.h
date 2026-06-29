/**
 * @file syn_task.h
 * @brief Task descriptor for the cooperative scheduler.
 *
 * Defines the task control block (SYN_Task) that pairs a protothread
 * with scheduling metadata: priority, state, name, and delay target.
 *
 * Tasks are caller-owned — you allocate them however you like (static
 * array, global, on the stack). The scheduler just takes a pointer to
 * your array.
 * @ingroup syn_sched
 */

#ifndef SYN_TASK_H
#define SYN_TASK_H

#include "../pt/syn_pt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task states ────────────────────────────────────────────────────────── */

/** @brief Cooperative task lifecycle state. */
typedef enum {
    SYN_TASK_READY     = 0,  /**< Eligible to run on next scheduler tick */
    SYN_TASK_SUSPENDED = 1,  /**< Paused — skipped by scheduler          */
    SYN_TASK_DEAD      = 2,  /**< Exited — will not run again            */
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
 * Typical size: ~20 bytes on a 32-bit target.
 */
typedef struct SYN_Task {
    SYN_PT          pt;           /**< Protothread continuation (2 bytes)      */
    SYN_TaskFunc    func;         /**< The task's protothread function          */
    const char      *name;         /**< Human-readable name (for debug/logging)  */
    uint8_t          priority;     /**< 0 = highest priority                     */
    uint8_t          state;        /**< SYN_TaskState                           */
    uint32_t         delay_until;  /**< Tick deadline for PT_TASK_DELAY_MS       */
    void            *user_data;    /**< Optional pointer to task-private state   */
} SYN_Task;

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_H */
