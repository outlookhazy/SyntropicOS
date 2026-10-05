

# File syn\_mutex.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mutex.h**](syn__mutex_8h.md)

[Go to the documentation of this file](syn__mutex_8h.md)


```C++

#ifndef SYN_MUTEX_H
#define SYN_MUTEX_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "syn_task.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Mutex Control Structure ────────────────────────────────────────────── */

typedef struct SYN_Mutex {
    SYN_Task *owner;       
    uint16_t lock_count;   
    uint8_t original_prio; 
} SYN_Mutex;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_mutex_init(SYN_Mutex *mutex);

SYN_Status syn_mutex_try_lock(SYN_Mutex *mutex, SYN_Task *task);

SYN_Status syn_mutex_unlock(SYN_Mutex *mutex, SYN_Task *task);

bool syn_mutex_is_locked(const SYN_Mutex *mutex);

SYN_Task *syn_mutex_get_owner(const SYN_Mutex *mutex);

/* ── Protothread Macros ─────────────────────────────────────────────────── */

#define PT_MUTEX_INIT(mutex) syn_mutex_init(mutex)

#define PT_MUTEX_LOCK(pt, task, mutex) \
    PT_WAIT_UNTIL(pt, syn_mutex_try_lock((mutex), (task)) == SYN_OK)

#define PT_MUTEX_UNLOCK(mutex, task) syn_mutex_unlock((mutex), (task))

#define PT_MUTEX_TRYLOCK(mutex, task) (syn_mutex_try_lock((mutex), (task)) == SYN_OK)

#ifdef __cplusplus
}
#endif

#endif /* SYN_MUTEX_H */
```


