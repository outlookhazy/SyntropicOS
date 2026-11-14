

# File syn\_pt\_sem.h

[**File List**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt\_sem.h**](syn__pt__sem_8h.md)

[Go to the documentation of this file](syn__pt__sem_8h.md)


```C++


#ifndef SYN_PT_SEM_H
#define SYN_PT_SEM_H

#include "../port/syn_port_system.h"
#include "syn_pt.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Semaphore structure ────────────────────────────────────────────────── */

typedef struct {
    volatile int16_t count; 
} SYN_PT_Sem;

/* ── Initialization ─────────────────────────────────────────────────────── */

#define PT_SEM_INIT(sem, initial) ((sem)->count = (int16_t)(initial))

/* ── Blocking wait (protothread context) ────────────────────────────────── */

#define PT_SEM_WAIT(pt, sem) PT_WAIT_UNTIL((pt), pt_sem_trywait(sem) != 0)

#define PT_SEM_BLOCK(pt, task, sem) PT_BLOCK_CONDITION((pt), (task), pt_sem_trywait(sem) != 0)

/* ── Signal (any context) ───────────────────────────────────────────────── */

static inline void pt_sem_signal(SYN_PT_Sem *sem)
{
    if (sem == NULL) {
        return;
    }
    syn_port_enter_critical();
    int16_t cur = sem->count;
    if (cur < INT16_MAX) {
        sem->count = (int16_t)(cur + 1);
    }
    syn_port_exit_critical();
}

#define PT_SEM_SIGNAL(sem) pt_sem_signal(sem)

/* ── Non-blocking try ───────────────────────────────────────────────────── */

static inline int pt_sem_trywait(SYN_PT_Sem *sem)
{
    if (sem == NULL) {
        return 0;
    }
    int acquired = 0;

    syn_port_enter_critical();
    if (sem->count > 0) {
        sem->count = (int16_t)(sem->count - 1);
        acquired = 1;
    }
    syn_port_exit_critical();

    return acquired;
}

#define PT_SEM_TRYWAIT(sem) pt_sem_trywait(sem)

/* ── Query ──────────────────────────────────────────────────────────────── */

#define PT_SEM_COUNT(sem) ((sem)->count)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_SEM_H */
```


