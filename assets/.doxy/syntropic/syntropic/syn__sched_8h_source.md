

# File syn\_sched.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sched.h**](syn__sched_8h.md)

[Go to the documentation of this file](syn__sched_8h.md)


```C++

#ifndef SYN_SCHED_H
#define SYN_SCHED_H

#include "syn_task.h"
#include "../common/syn_compiler.h"
#include <stdbool.h>

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS && \
    defined(SYN_USE_TIMER) && SYN_USE_TIMER
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
#define SYN_SCHED_PRIO_LEVELS 8
#endif

/* ── Scheduler struct ───────────────────────────────────────────────────── */

typedef struct SYN_Sched {
    SYN_Task  *tasks;         
    size_t     task_count;    
    size_t     rr_per_prio[SYN_SCHED_PRIO_LEVELS];
} SYN_Sched;

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count);

void syn_task_create(SYN_Task *task,
                      const char *name,
                      SYN_TaskFunc func,
                      uint8_t priority,
                      void *user_data);

/* ── Scheduler execution ────────────────────────────────────────────────── */

bool syn_sched_run(SYN_Sched *sched);

SYN_NORETURN void syn_sched_run_forever(SYN_Sched *sched);

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS

uint32_t syn_sched_next_wakeup(const SYN_Sched *sched);

SYN_NORETURN void syn_sched_run_tickless(SYN_Sched *sched, SYN_Sleep *sleep);

#if defined(SYN_USE_TIMER) && SYN_USE_TIMER
SYN_NORETURN void syn_sched_run_tickless_ex(SYN_Sched *sched,
                                             SYN_Sleep *sleep,
                                             SYN_Timer *timers,
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

#ifdef __cplusplus
}
#endif

#endif /* SYN_SCHED_H */
```


