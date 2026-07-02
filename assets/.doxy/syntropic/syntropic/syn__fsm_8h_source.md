

# File syn\_fsm.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fsm.h**](syn__fsm_8h.md)

[Go to the documentation of this file](syn__fsm_8h.md)


```C++

#ifndef SYN_FSM_H
#define SYN_FSM_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

typedef int16_t SYN_FSM_State;  
typedef int16_t SYN_FSM_Event;  
#define SYN_FSM_STATE_NONE   ((SYN_FSM_State)-1)

typedef bool (*SYN_FSM_Guard)(void *ctx);

typedef void (*SYN_FSM_Action)(void *ctx);

/* ── Transition descriptor ──────────────────────────────────────────────── */

typedef struct {
    SYN_FSM_State   from;     
    SYN_FSM_Event   event;    
    SYN_FSM_State   to;       
    SYN_FSM_Guard   guard;    
    SYN_FSM_Action  action;   
} SYN_FSM_Transition;

#define SYN_FSM_END  { SYN_FSM_STATE_NONE, 0, 0, NULL, NULL }

/* ── State descriptor (optional) ────────────────────────────────────────── */

typedef struct {
    SYN_FSM_State   state;    
    SYN_FSM_Action  on_enter; 
    SYN_FSM_Action  on_exit;  
} SYN_FSM_StateDesc;

#define SYN_FSM_STATE_END  { SYN_FSM_STATE_NONE, NULL, NULL }

/* ── FSM instance ───────────────────────────────────────────────────────── */

typedef struct {
    const SYN_FSM_Transition  *transitions;  
    const SYN_FSM_StateDesc   *state_descs;  
    const char * const         *state_names;  
    const char                 *tag;           
    SYN_FSM_State              current;       
    void                       *ctx;           
} SYN_FSM;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_fsm_init(SYN_FSM *fsm,
                   const SYN_FSM_Transition *transitions,
                   SYN_FSM_State initial,
                   const char *tag);

void syn_fsm_set_state_descs(SYN_FSM *fsm, const SYN_FSM_StateDesc *descs);

void syn_fsm_set_state_names(SYN_FSM *fsm, const char * const *names);

void syn_fsm_set_context(SYN_FSM *fsm, void *ctx);

bool syn_fsm_dispatch(SYN_FSM *fsm, SYN_FSM_Event event);

static inline SYN_FSM_State syn_fsm_state(const SYN_FSM *fsm)
{
    return fsm->current;
}

static inline bool syn_fsm_in_state(const SYN_FSM *fsm, SYN_FSM_State st)
{
    return fsm->current == st;
}

void syn_fsm_set_state(SYN_FSM *fsm, SYN_FSM_State state);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FSM_H */
```


