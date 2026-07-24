

# File syn\_task\_profile.h

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.h**](syn__task__profile_8h.md)

[Go to the documentation of this file](syn__task__profile_8h.md)


```C++

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

typedef struct {
    const char *task_name;      
    uint32_t    step_count;     
    uint32_t    total_time_us;  
    uint32_t    max_yield_us;   
    uint32_t    last_start_us;  
    float       cpu_pct;        
} SYN_TaskProfile;

#ifndef SYN_TASK_PROFILE_MAX_TASKS
  #define SYN_TASK_PROFILE_MAX_TASKS 16
#endif

typedef struct {
    SYN_TaskProfile profiles[SYN_TASK_PROFILE_MAX_TASKS]; 
    size_t          count;                                 
    uint32_t        window_start_us;                       
    bool            enabled;                               
} SYN_TaskProfileManager;

SYN_Status syn_task_profile_init(SYN_TaskProfileManager *mgr);

void syn_task_profile_enable(SYN_TaskProfileManager *mgr, bool enable);

void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx, const char *task_name, uint32_t now_us);

void syn_task_profile_step_end(SYN_TaskProfileManager *mgr, size_t task_idx, uint32_t now_us);

void syn_task_profile_update(SYN_TaskProfileManager *mgr, uint32_t now_us);

SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx, SYN_TaskProfile *profile);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_PROFILE_H */
```


