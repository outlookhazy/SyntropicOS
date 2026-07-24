/**
 * @file syn_task_profile.h
 * @brief Opt-In Task Execution & CPU Jitter Profiler.
 * @ingroup syn_debug
 *
 * Provides opt-in execution duration, maximum yield latency, and CPU %
 * utilization tracking for cooperative protothread tasks.
 */

#ifndef SYN_TASK_PROFILE_H
#define SYN_TASK_PROFILE_H

#include "../common/syn_defs.h"
#include "../sched/syn_sched.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Task Execution Profile Metrics Structure.
 */
typedef struct {
    const char *task_name;      /**< Pointer to task name string            */
    uint32_t    step_count;     /**< Total number of times task executed    */
    uint32_t    total_time_us;  /**< Total accumulated execution time in us */
    uint32_t    max_yield_us;   /**< Maximum continuous execution time in us*/
    uint32_t    last_start_us;  /**< Microsecond timestamp of step start    */
    float       cpu_pct;        /**< Computed CPU utilization percentage    */
} SYN_TaskProfile;

#ifndef SYN_TASK_PROFILE_MAX_TASKS
  #define SYN_TASK_PROFILE_MAX_TASKS 16
#endif

/**
 * @brief Task Profiler Registry / Manager Instance.
 */
typedef struct {
    SYN_TaskProfile profiles[SYN_TASK_PROFILE_MAX_TASKS]; /**< Task profile array  */
    size_t          count;                                 /**< Profile count       */
    uint32_t        window_start_us;                       /**< Profile window start*/
    bool            enabled;                               /**< Profiler enable flag*/
} SYN_TaskProfileManager;

/**
 * @brief Initialize the task execution profiler.
 * @param mgr Pointer to task profile manager.
 * @return SYN_OK on success.
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
void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx, const char *task_name, uint32_t now_us);

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
SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx, SYN_TaskProfile *profile);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_PROFILE_H */
