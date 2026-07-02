

# File syn\_pt\_sem.h

[**File List**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt\_sem.h**](syn__pt__sem_8h.md)

[Go to the documentation of this file](syn__pt__sem_8h.md)


```C++

#ifndef SYN_PT_SEM_H
#define SYN_PT_SEM_H

#include "syn_pt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Semaphore structure ────────────────────────────────────────────────── */

typedef struct {
    volatile int16_t count;  
} SYN_PT_Sem;

/* ── Initialization ─────────────────────────────────────────────────────── */

#define PT_SEM_INIT(sem, initial)   ((sem)->count = (int16_t)(initial))

/* ── Blocking wait (protothread context) ────────────────────────────────── */

#define PT_SEM_WAIT(pt, sem)                                  \
    do {                                                       \
        PT_WAIT_UNTIL(pt, (sem)->count > 0);                   \
        (sem)->count--;                                        \
    } while (0)

/* ── Signal (any context) ───────────────────────────────────────────────── */

#define PT_SEM_SIGNAL(sem)   ((sem)->count++)

/* ── Non-blocking try ───────────────────────────────────────────────────── */

static inline int pt_sem_trywait(SYN_PT_Sem *sem)
{
    if (sem->count > 0) {
        sem->count--;
        return 1;
    }
    return 0;
}

#define PT_SEM_TRYWAIT(sem)   pt_sem_trywait(sem)

/* ── Query ──────────────────────────────────────────────────────────────── */

#define PT_SEM_COUNT(sem)     ((sem)->count)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_SEM_H */
```


