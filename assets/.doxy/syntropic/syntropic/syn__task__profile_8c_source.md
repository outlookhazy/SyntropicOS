

# File syn\_task\_profile.c

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.c**](syn__task__profile_8c.md)

[Go to the documentation of this file](syn__task__profile_8c.md)


```C++

#include "syn_task_profile.h"
#include <string.h>

SYN_Status syn_task_profile_init(SYN_TaskProfileManager *mgr)
{
    if (!mgr) return SYN_INVALID_PARAM;

    memset(mgr, 0, sizeof(*mgr));
    mgr->enabled = true;
    mgr->window_start_us = 0;

    return SYN_OK;
}

void syn_task_profile_enable(SYN_TaskProfileManager *mgr, bool enable)
{
    if (!mgr) return;
    mgr->enabled = enable;
}

void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx, const char *task_name, uint32_t now_us)
{
    if (!mgr || !mgr->enabled || task_idx >= SYN_TASK_PROFILE_MAX_TASKS) return;

    SYN_TaskProfile *p = &mgr->profiles[task_idx];
    if (!p->task_name) {
        p->task_name = task_name ? task_name : "unnamed";
        if (task_idx >= mgr->count) {
            mgr->count = task_idx + 1;
        }
    }

    p->last_start_us = now_us;
}

void syn_task_profile_step_end(SYN_TaskProfileManager *mgr, size_t task_idx, uint32_t now_us)
{
    if (!mgr || !mgr->enabled || task_idx >= SYN_TASK_PROFILE_MAX_TASKS) return;

    SYN_TaskProfile *p = &mgr->profiles[task_idx];
    uint32_t duration = now_us - p->last_start_us;

    p->step_count++;
    p->total_time_us += duration;
    if (duration > p->max_yield_us) {
        p->max_yield_us = duration;
    }
}

void syn_task_profile_update(SYN_TaskProfileManager *mgr, uint32_t now_us)
{
    if (!mgr) return;

    uint32_t window_duration = now_us - mgr->window_start_us;
    if (window_duration == 0) return;

    for (size_t i = 0; i < mgr->count; i++) {
        SYN_TaskProfile *p = &mgr->profiles[i];
        p->cpu_pct = ((float)p->total_time_us / (float)window_duration) * 100.0f;
    }
}

SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx, SYN_TaskProfile *profile)
{
    if (!mgr || !profile || task_idx >= mgr->count) return SYN_INVALID_PARAM;

    *profile = mgr->profiles[task_idx];

    return SYN_OK;
}
```


