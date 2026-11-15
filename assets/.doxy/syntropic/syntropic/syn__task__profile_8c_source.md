

# File syn\_task\_profile.c

[**File List**](files.md) **>** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md) **>** [**syn\_task\_profile.c**](syn__task__profile_8c.md)

[Go to the documentation of this file](syn__task__profile_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER


#include "syn_task_profile.h"

#include <string.h>

SYN_Status syn_task_profile_init(SYN_TaskProfileManager *mgr)
{
    if (!mgr) {
        return SYN_INVALID_PARAM;
    }

    memset(mgr, 0, sizeof(*mgr));
    mgr->enabled = true;
    mgr->window_start_us = 0U;

    return SYN_OK;
}

void syn_task_profile_enable(SYN_TaskProfileManager *mgr, bool enable)
{
    if (!mgr) {
        return;
    }
    if (!enable) {
        for (size_t i = 0U; i < mgr->count; i++) {
            mgr->profiles[i].step_active = false;
        }
    }
    mgr->enabled = enable;
}

void syn_task_profile_step_start(SYN_TaskProfileManager *mgr, size_t task_idx,
                                 const char *task_name, uint32_t now_us)
{
    if (!mgr || !mgr->enabled || task_idx >= SYN_TASK_PROFILE_MAX_TASKS) {
        return;
    }

    SYN_TaskProfile *p = &mgr->profiles[task_idx];
    if (!p->task_name) {
        p->task_name = task_name ? task_name : "unnamed";
    } else if (task_name != NULL) {
        p->task_name = task_name;
    }
    if (task_idx >= mgr->count) {
        mgr->count = task_idx + 1U;
    }

    p->last_start_us = now_us;
    p->step_active = true;
}

void syn_task_profile_step_end(SYN_TaskProfileManager *mgr, size_t task_idx, uint32_t now_us)
{
    if (!mgr || !mgr->enabled || task_idx >= SYN_TASK_PROFILE_MAX_TASKS) {
        return;
    }

    SYN_TaskProfile *p = &mgr->profiles[task_idx];
    if (!p->step_active) {
        return;
    }
    p->step_active = false;

    uint32_t duration = now_us - p->last_start_us;

    p->step_count++;
    p->total_time_us += duration;
    p->window_time_us += duration;
    if (duration > p->max_yield_us) {
        p->max_yield_us = duration;
    }
}

void syn_task_profile_update(SYN_TaskProfileManager *mgr, uint32_t now_us)
{
    if (!mgr) {
        return;
    }

    uint32_t window_duration = now_us - mgr->window_start_us;
    if (window_duration == 0U) {
        return;
    }

    for (size_t i = 0U; i < mgr->count; i++) {
        SYN_TaskProfile *p = &mgr->profiles[i];
        float pct = ((float)p->window_time_us / (float)window_duration) * 100.0f;
        p->cpu_pct = (pct > 100.0f) ? 100.0f : pct;
        p->window_time_us = 0U;
    }
    mgr->window_start_us = now_us;
}

SYN_Status syn_task_profile_get(const SYN_TaskProfileManager *mgr, size_t task_idx,
                                SYN_TaskProfile *profile)
{
    if (!mgr || !profile || task_idx >= mgr->count) {
        return SYN_INVALID_PARAM;
    }

    *profile = mgr->profiles[task_idx];

    return SYN_OK;
}

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED

static bool profile_peer_participates(const SYN_TaskProfileManager *mgr, const SYN_Task *peer,
                                      size_t idx)
{
    if (peer->func == NULL || peer->state == (uint8_t)SYN_TASK_SUSPENDED) {
        return false;
    }
    if (peer->state != (uint8_t)SYN_TASK_DEAD) {
        return true;
    }
    return (mgr != NULL) && (idx < mgr->count) && (mgr->profiles[idx].step_count > 0U);
}

static uint32_t profile_peer_wait_us(const SYN_Sched *sched, uint8_t raw_prio, uint8_t *out_prio)
{
    uint8_t p = (raw_prio >= (uint8_t)SYN_SCHED_PRIO_LEVELS) ? (uint8_t)(SYN_SCHED_PRIO_LEVELS - 1U)
                                                             : raw_prio;
    *out_prio = p;
    return (uint32_t)sched->max_wait_ms[p] * 1000U;
}

static void profile_record_worst(SYN_TaskBudgetViolation *worst_out, uint32_t *worst_overrun,
                                 bool is_first, size_t idx, const SYN_Task *task,
                                 const SYN_TaskProfile *prof, uint8_t peer_prio, uint32_t budget_us)
{
    uint32_t overrun = prof->max_yield_us - budget_us;
    if (worst_out != NULL && (is_first || overrun > *worst_overrun)) {
        worst_out->task_idx = idx;
        worst_out->task_name = prof->task_name;
        worst_out->task_priority = task->base_priority;
        worst_out->peer_priority = peer_prio;
        worst_out->max_yield_us = prof->max_yield_us;
        worst_out->budget_us = budget_us;
        *worst_overrun = overrun;
    }
}

uint32_t syn_task_profile_task_budget_us(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                         size_t task_idx, uint8_t *out_peer_prio)
{
    if (out_peer_prio != NULL) {
        *out_peer_prio = 0xFFU;
    }
    if (sched == NULL || sched->tasks == NULL || task_idx >= sched->task_count) {
        return UINT32_MAX;
    }

    uint32_t best_us = UINT32_MAX;
    uint8_t best_prio = 0xFFU;

    for (size_t j = 0U; j < sched->task_count; j++) {
        if (j == task_idx || !profile_peer_participates(mgr, &sched->tasks[j], j)) {
            continue;
        }
        uint8_t p = 0U;
        uint32_t wait_us = profile_peer_wait_us(sched, sched->tasks[j].priority, &p);
        if (wait_us < best_us || (wait_us == best_us && p < best_prio)) {
            best_us = wait_us;
            best_prio = p;
        }
    }

    if (out_peer_prio != NULL) {
        *out_peer_prio = best_prio;
    }
    return best_us;
}

SYN_Status syn_task_profile_check_budgets(const SYN_TaskProfileManager *mgr, const SYN_Sched *sched,
                                          size_t *out_violations,
                                          SYN_TaskBudgetViolation *worst_out)
{
    if (mgr == NULL || sched == NULL || sched->tasks == NULL || out_violations == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (worst_out != NULL) {
        memset(worst_out, 0, sizeof(*worst_out));
    }

    size_t limit = (mgr->count < sched->task_count) ? mgr->count : sched->task_count;
    size_t violations = 0U;
    uint32_t worst_overrun = 0U;

    for (size_t i = 0U; i < limit; i++) {
        const SYN_TaskProfile *prof = &mgr->profiles[i];
        if (prof->step_count == 0U) {
            continue;
        }
        uint8_t peer_prio = 0xFFU;
        uint32_t budget_us = syn_task_profile_task_budget_us(mgr, sched, i, &peer_prio);
        if (budget_us != UINT32_MAX && prof->max_yield_us > budget_us) {
            violations++;
            profile_record_worst(worst_out, &worst_overrun, violations == 1U, i, &sched->tasks[i],
                                 prof, peer_prio, budget_us);
        }
    }

    *out_violations = violations;
    return SYN_OK;
}

#endif /* SYN_USE_SCHED */

#else
typedef int syn_task_profile_unused_tu;
#endif /* SYN_USE_PROFILER */
```


