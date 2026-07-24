

# File syn\_task.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_task.h**](syn__task_8h.md)

[Go to the documentation of this file](syn__task_8h.md)


```C++

#ifndef SYN_TASK_H
#define SYN_TASK_H

#include "../pt/syn_pt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task states ────────────────────────────────────────────────────────── */

/* Forward-declare SYN_EventGroup so tasks can hold a pointer to one
 * without pulling in the full event header.                          */
struct SYN_EventGroup;

typedef enum {
    SYN_TASK_READY     = 0,  
    SYN_TASK_SUSPENDED = 1,  
    SYN_TASK_DEAD      = 2,  
    SYN_TASK_DEFERRED  = 3,  
    SYN_TASK_BLOCKED   = 4,  
    SYN_TASK_WAITING   = 5,  
} SYN_TaskState;

/* ── Forward declaration ────────────────────────────────────────────────── */

struct SYN_Task;

/* ── Task function signature ────────────────────────────────────────────── */

typedef SYN_PT_Status (*SYN_TaskFunc)(SYN_PT *pt, struct SYN_Task *task);

/* ── Task control block ─────────────────────────────────────────────────── */

#include "syn_event_flags.h"

typedef struct SYN_Task {
    SYN_PT          pt;           
    SYN_TaskFunc    func;         
    const char      *name;         
    uint8_t          priority;     
    uint8_t          state;        
    uint32_t         delay_until;  
    void            *user_data;    
    SYN_EventFlags  *wait_event;   
    uint32_t         wait_mask;    
} SYN_Task;

#ifdef __cplusplus
}
#endif

#endif /* SYN_TASK_H */
```


