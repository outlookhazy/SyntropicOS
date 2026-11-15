

# File syn\_task.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_task.h**](syn__task_8h.md)

[Go to the documentation of this file](syn__task_8h.md)


```C++


#ifndef SYN_TASK_H
#define SYN_TASK_H

#include "../pt/syn_pt.h"
#include "syn_event_flags.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task priorities ────────────────────────────────────────────────────── */

typedef enum {
    SYN_PRIO_CRITICAL = 0,   
    SYN_PRIO_HIGH = 1,       
    SYN_PRIO_NORMAL = 2,     
    SYN_PRIO_LOW = 3,        
    SYN_PRIO_LEVEL_MAX = 255 
} SYN_Prio;

/* ── Task states ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_TASK_READY = 0,     
    SYN_TASK_SUSPENDED = 1, 
    SYN_TASK_DEAD = 2,      
    SYN_TASK_BLOCKED = 3,   
} SYN_TaskState;

/* ── Scheduler internal flags (stored in SYN_Task.sched_flags) ──────────── */

#define SYN_SCHED_FLAG_MORE 0x01U

#define SYN_SCHED_FLAG_PASSED 0x02U

#define SYN_SCHED_FLAG_SLEEPING 0x04U

#define SYN_SCHED_FLAG_DUE_VALID 0x08U

#define SYN_SCHED_FLAG_ISR_WAKE 0x10U

#define SYN_SCHED_FLAG_MUTEX_WAIT 0x20U

#define SYN_SCHED_FLAG_RESTARTED 0x40U

#define SYN_TASK_NO_MANUAL_PRIO 0xFFU

/* ── Forward declarations ───────────────────────────────────────────────── */

struct SYN_Task;
struct SYN_Sched;
struct SYN_Mutex;

/* ── Task function signature ────────────────────────────────────────────── */

typedef SYN_PT_Status (*SYN_TaskFunc)(SYN_PT *pt, struct SYN_Task *task);

/* ── Task control block ─────────────────────────────────────────────────── */

typedef struct SYN_Task {
    SYN_PT pt;                      
    uint8_t manual_priority;        
    SYN_TaskFunc func;              
    const char *name;               
    uint8_t priority;               
    uint8_t base_priority;          
    volatile uint8_t state;         
    volatile uint8_t sched_flags;   
    uint32_t delay_until;           
    uint32_t due_ms;                
    struct SYN_Sched *sched;        
    struct SYN_Mutex *held_mutexes; 
    void *user_data;                
    SYN_EventFlags *wait_event;     
    uint32_t wait_mask;             
} SYN_Task;

void syn_sched_refresh_share(struct SYN_Sched *sched);

static inline bool syn_task_arm_event_wait(SYN_Task *task, SYN_EventFlags *grp, uint32_t mask,
                                           bool use_timeout, uint32_t timeout_ms,
                                           uint32_t *out_matched)
{
    if (out_matched != NULL) {
        *out_matched = 0U;
    }
    if ((task == NULL) || (grp == NULL) || (mask == 0U)) {
        return false;
    }

    syn_port_enter_critical();
    if (syn_event_flags_wait(
            grp, mask, (uint32_t)SYN_EVENT_FLAGS_WAIT_ANY | (uint32_t)SYN_EVENT_FLAGS_AUTO_CLEAR,
            out_matched) == SYN_OK) {
        syn_sched_refresh_share(task->sched);
        syn_port_exit_critical();
        return false;
    }

    task->delay_until = 0U;
    if (use_timeout &&
        !syn_pt_arm_ms_deadline(&task->delay_until, syn_port_get_tick_ms(), timeout_ms)) {
        syn_port_exit_critical();
        return false;
    }

    task->wait_event = grp;
    task->wait_mask = mask;
    SYN_PT_STATE_BARRIER();
    syn_port_exit_critical();
    return true;
}

static inline void syn_task_park_blocked(SYN_Task *task)
{
    syn_port_enter_critical();
    if (task->state == (uint8_t)SYN_TASK_READY) {
        task->state = (uint8_t)SYN_TASK_BLOCKED;
        SYN_PT_STATE_BARRIER();
    }
    syn_port_exit_critical();
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_H */
```


