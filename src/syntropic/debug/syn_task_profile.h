/**
 * @file syn_task_profile.h
 * @brief Opt-In Task Execution, CPU Jitter & Cooperative Chunk-Budget Profiler.
 * @ingroup syn_debug
 *
 * Provides opt-in execution duration, maximum yield latency, CPU %
 * utilization tracking, and cooperative starvation-budget verification
 * for protothread tasks.
 */

#ifndef SYN_TASK_PROFILE_H
#define SYN_TASK_PROFILE_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "../common/syn_defs.h"
#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
#include "../sched/syn_sched.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Task Execution Profile Metrics Structure.
 */
typedef struct {
    const char *task_name;   /**< Pointer to task name string              */
    uint32_t step_count;     /**< Total number of times task executed      */
    uint32_t total_time_us;  /**< Total accumulated execution time in us   */
    uint32_t window_time_us; /**< Execution time accumulated in window (us)*/
    uint32_t max_yield_us;   /**< Maximum continuous execution time in us  */
    uint32_t last_start_us;  /**< Microsecond timestamp of step start      */
    float cpu_pct;           /**< Computed CPU utilization percentage      */
    bool step_active;        /**< True when step_start was recorded        */
} SYN_TaskProfile;

#ifndef SYN_TASK_PROFILE_MAX_TASKS
#define SYN_TASK_PROFILE_MAX_TASKS 16 /**< Maximum profiled task slots in registry */
#endif

/**
 * @brief Task Profiler Registry / Manager Instance.
 */
typedef struct SYN_TaskProfileManager {
    SYN_TaskProfile profiles[SYN_TASK_PROFILE_MAX_TASKS]; /**< Task profile array  */
    size_t count;                                         /**< Profile count       */
    uint32_t window_start_us;                             /**< Profile window start*/
    bool enabled;                                         /**< Profiler enable flag*/
} SYN_TaskProfileManager;

/**
 * @brief Cooperative chunk latency budget violation record.
 */
typedef struct {
    size_t task_idx;       /**< Index of offending task in scheduler       */
    const char *task_name; /**< Name of offending task                     */
    uint8_t task_priority; /**< Priority of offending task                 */
    uint8_t peer_priority; /**< Priority of tightest constraining peer     */
    uint32_t max_yield_us; /**< Observed peak continuous execution (us)    */
    uint32_t budget_us;    /**< Allowed chunk budget from peer max_wait_ms */
} SYN_TaskBudgetViolation;

/**
 * @brief Initialize the task execution profiler.
 * @param mgr Pointer to task profile manager.
 * @return SYN_OK on success, or SYN_INVALID_PARAM when @p mgr is NULL.
 */
SYN_Status syn_task_profile_init(SYN_TaskProfileManager *mgr);

/**
 * @brief Enable or disable the task profiler.
 * @param mgr Pointer to profile manager.
 * @param enable true to enable, false to disable.
 */
void syn_task_profile_enable(SYN_TaskProfileManager *mgr, bool enable);

/**
 * @brief Record the start of a task step execution.
 * @param mgr Pointer to profile manager.
 * @param task_idx Index of executing task.
 * @param task_name Task identifier name.
 * @param now_us Current microsecond timestamp.
 */
void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx,
                                 const char *task_name, uint32_t now_us);

/**
 * @brief Record the completion / yield of a task step execution.
 * @param mgr Pointer to profile manager.
 * @param task_idx Index of executing task.
 * @param now_us Current microsecond timestamp.
 */
void syn_task_profile_step_end(SYN_TaskProfileManager *mgr, size_t task_idx, uint32_t now_us);

/**
 * @brief Recalculate CPU utilization percentages across active tasks.
 * @param mgr Pointer to profile manager.
 * @param now_us Current microsecond timestamp.
 */
void syn_task_profile_update(SYN_TaskProfileManager *mgr, uint32_t now_us);

/**
 * @brief Get the profile statistics for a specific task.
 * @param mgr Pointer to profile manager.
 * @param task_idx Task index.
 * @param profile Output profile metrics pointer.
 * @return SYN_OK on success, or SYN_INVALID_PARAM.
 */
SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx,
                                SYN_TaskProfile *profile);

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
/**
 * @brief Compute the cooperative chunk budget (us) for @p task_idx in @p sched.
 *
 * Returns the smallest `sched->max_wait_ms[prio] * 1000` among all participating
 * peer tasks `j != task_idx` (any non-SUSPENDED peer that is currently alive or
 * executed at least one profiled step in @p mgr). Returns `UINT32_MAX` when
 * uncontended or out of range.
 *
 * @param mgr            Optional profiler manager (used to include completed peers).
 * @param sched          Scheduler instance.
 * @param task_idx       Task slot index in `sched->tasks`.
 * @param out_peer_prio  Optional output receiving the constraining peer priority (0xFF if none).
 * @return Allowed cooperative chunk duration in microseconds, or UINT32_MAX if unbounded.
 */
uint32_t syn_task_profile_task_budget_us(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                         size_t task_idx, uint8_t *out_peer_prio);

/**
 * @brief Verify profiled cooperative chunk durations against scheduler peer budgets.
 *
 * Scans all profiled tasks with `step_count > 0` up to `min(mgr->count, sched->task_count)`,
 * counting every task whose `max_yield_us` exceeds `syn_task_profile_task_budget_us()`.
 * Populates @p worst_out (when non-NULL) with the violation having the largest
 * overrun (`max_yield_us - budget_us`).
 *
 * @param mgr             Initialized task profile manager.
 * @param sched           Scheduler instance whose tasks were profiled.
 * @param out_violations  Output receiving total number of budget-violating tasks.
 * @param worst_out       Optional output receiving the worst violation (zeroed if 0).
 * @return SYN_OK on success, or SYN_INVALID_PARAM on NULL required arguments.
 */
SYN_Status syn_task_profile_check_budgets(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                          size_t *out_violations,
                                          SYN_TaskBudgetViolation *worst_out);
#endif /* SYN_USE_SCHED */

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_PROFILE_H */
