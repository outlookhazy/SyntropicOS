

# File syn\_task\_profile.h

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.h**](syn__task__profile_8h.md)

[Go to the documentation of this file](syn__task__profile_8h.md)


```C++


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

typedef struct {
    const char *task_name;   
    uint32_t step_count;     
    uint32_t total_time_us;  
    uint32_t window_time_us; 
    uint32_t max_yield_us;   
    uint32_t last_start_us;  
    float cpu_pct;           
    bool step_active;        
} SYN_TaskProfile;

#ifndef SYN_TASK_PROFILE_MAX_TASKS
#define SYN_TASK_PROFILE_MAX_TASKS 16 
#endif

typedef struct SYN_TaskProfileManager {
    SYN_TaskProfile profiles[SYN_TASK_PROFILE_MAX_TASKS]; 
    size_t count;                                         
    uint32_t window_start_us;                             
    bool enabled;                                         
} SYN_TaskProfileManager;

typedef struct {
    size_t task_idx;       
    const char *task_name; 
    uint8_t task_priority; 
    uint8_t peer_priority; 
    uint32_t max_yield_us; 
    uint32_t budget_us;    
} SYN_TaskBudgetViolation;

SYN_Status syn_task_profile_init(SYN_TaskProfileManager *mgr);

void syn_task_profile_enable(SYN_TaskProfileManager *mgr, bool enable);

void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx,
                                 const char *task_name, uint32_t now_us);

void syn_task_profile_step_end(SYN_TaskProfileManager *mgr, size_t task_idx, uint32_t now_us);

void syn_task_profile_update(SYN_TaskProfileManager *mgr, uint32_t now_us);

SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx,
                                SYN_TaskProfile *profile);

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
uint32_t syn_task_profile_task_budget_us(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                         size_t task_idx, uint8_t *out_peer_prio);

SYN_Status syn_task_profile_check_budgets(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                          size_t *out_violations,
                                          SYN_TaskBudgetViolation *worst_out);
#endif /* SYN_USE_SCHED */

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_PROFILE_H */
```


