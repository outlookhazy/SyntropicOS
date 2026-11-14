

# File syn\_sched.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sched.h**](syn__sched_8h.md)

[Go to the documentation of this file](syn__sched_8h.md)


```C++


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

#ifndef SYN_SCHED_PRIO_LEVELS
#define SYN_SCHED_PRIO_LEVELS 4
#endif

#if (SYN_SCHED_PRIO_LEVELS < 1) || (SYN_SCHED_PRIO_LEVELS > 255)
#error "SYN_SCHED_PRIO_LEVELS must be between 1 and 255"
#endif

#ifndef SYN_SCHED_MAX_WAIT_CRITICAL_MS
#define SYN_SCHED_MAX_WAIT_CRITICAL_MS 1U
#endif

#ifndef SYN_SCHED_MAX_WAIT_HIGH_MS
#define SYN_SCHED_MAX_WAIT_HIGH_MS 5U
#endif

#ifndef SYN_SCHED_MAX_WAIT_NORMAL_MS
#define SYN_SCHED_MAX_WAIT_NORMAL_MS 15U
#endif

#ifndef SYN_SCHED_MAX_WAIT_LOW_MS
#define SYN_SCHED_MAX_WAIT_LOW_MS 50U
#endif

/* ── Scheduler struct ───────────────────────────────────────────────────── */

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER
struct SYN_TaskProfileManager;
#endif

typedef struct SYN_Sched {
    SYN_Task *tasks;                             
    size_t task_count;                           
    uint16_t rr_per_prio[SYN_SCHED_PRIO_LEVELS]; 
    uint16_t max_wait_ms[SYN_SCHED_PRIO_LEVELS]; 
    uint32_t share_deadline_ms;                  
    SYN_Task *running;                           
    struct SYN_Mutex *standalone_mutexes;        
#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER
    struct SYN_TaskProfileManager *profiler; 
#endif
    bool share_contended;     
    bool share_has_deadline;  
    bool share_watch_wakeups; 
} SYN_Sched;

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count);

void syn_sched_set_max_wait(SYN_Sched *sched, SYN_Prio prio, uint16_t ms);

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER
void syn_sched_set_profiler(SYN_Sched *sched, struct SYN_TaskProfileManager *profiler);
#endif

void syn_task_create(SYN_Task *task, const char *name, SYN_TaskFunc func, SYN_Prio priority,
                     void *user_data);

/* ── Cooperative CPU sharing ────────────────────────────────────────────── */

bool syn_task_should_share(const SYN_Task *task);

#define PT_CONTINUE_IF_NEEDED(pt, task)    \
    do {                                   \
        if (syn_task_should_share(task)) { \
            PT_CONTINUE(pt);               \
        }                                  \
    } while (0)

#define PT_SPAWN_IF_NEEDED(pt, task, child, func)                         \
    do {                                                                  \
        PT_INIT(child);                                                   \
        (pt)->lc = __LINE__;                                              \
        SYN_FALLTHROUGH;                                                  \
    case __LINE__:                                                        \
        for (;;) {                                                        \
            SYN_PT_Status _pt_child_st = (func);                          \
            if (_pt_child_st >= PT_EXITED) {                              \
                break;                                                    \
            }                                                             \
            if (_pt_child_st == PT_IDLE || syn_task_should_share(task)) { \
                return _pt_child_st;                                      \
            }                                                             \
        }                                                                 \
    } while (0)

/* ── Scheduler execution ────────────────────────────────────────────────── */

bool syn_sched_run(SYN_Sched *sched);

SYN_NORETURN void syn_sched_run_forever(SYN_Sched *sched);

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS

uint32_t syn_sched_next_wakeup(const SYN_Sched *sched);

SYN_NORETURN void syn_sched_run_tickless(SYN_Sched *sched, SYN_Sleep *sleep);

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER
SYN_NORETURN void syn_sched_run_tickless_ex(SYN_Sched *sched, SYN_Sleep *sleep, SYN_Timer *timers,
                                            size_t timer_count);
#endif /* SYN_USE_TIMER */

#endif /* SYN_USE_TICKLESS */

/* ── Task control ───────────────────────────────────────────────────────── */

void syn_task_suspend(SYN_Task *task);

void syn_task_resume(SYN_Task *task);

void syn_task_restart(SYN_Task *task);

static inline bool syn_task_is_alive(const SYN_Task *task)
{
    return task->state != (uint8_t)SYN_TASK_DEAD;
}

size_t syn_sched_alive_count(const SYN_Sched *sched);

void syn_task_boost_priority(SYN_Task *task, SYN_Prio temp_prio);

void syn_task_restore_priority(SYN_Task *task);

void syn_task_set_base_priority(SYN_Task *task, SYN_Prio new_prio);

void syn_sched_boost_task_prio(SYN_Task *task);
void syn_sched_recompute_task_prio(SYN_Task *task);
void syn_sched_clear_mutex_wait(SYN_Task *task);
void syn_sched_refresh_share(SYN_Sched *sched);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SCHED_H */
```


