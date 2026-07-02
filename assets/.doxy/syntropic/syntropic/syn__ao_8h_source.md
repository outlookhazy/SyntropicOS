

# File syn\_ao.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_ao.h**](syn__ao_8h.md)

[Go to the documentation of this file](syn__ao_8h.md)


```C++

#ifndef SYN_AO_H
#define SYN_AO_H

#include "../common/syn_defs.h"
#include "../sched/syn_task.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_mailbox.h"
#include "../util/syn_fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t sig;   
    void    *data;  
} SYN_AO_Event;

typedef struct {
    SYN_Task         task;     
    SYN_PT           pt;       
    SYN_Mailbox      mailbox;  
    SYN_FSM          fsm;      
    SYN_AO_Event     last_event; 
} SYN_ActiveObject;

void syn_ao_init(SYN_ActiveObject *ao,
                 const char *name,
                 const SYN_FSM_Transition *transitions,
                 SYN_FSM_State initial_state,
                 void *mailbox_buf,
                 size_t mailbox_cap);

bool syn_ao_post(SYN_ActiveObject *ao, uint16_t sig, void *data);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AO_H */
```


