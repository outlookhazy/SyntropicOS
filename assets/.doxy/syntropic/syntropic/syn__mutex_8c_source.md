

# File syn\_mutex.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mutex.c**](syn__mutex_8c.md)

[Go to the documentation of this file](syn__mutex_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_mutex.h"
#include "syn_sched.h"

SYN_Status syn_mutex_init(SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    mutex->owner = NULL;
    mutex->lock_count = 0;
    mutex->original_prio = 0;

    return SYN_OK;
}

SYN_Status syn_mutex_try_lock(SYN_Mutex *mutex, SYN_Task *task)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();

    /* Case 1: Mutex is currently unlocked */
    if (mutex->owner == NULL && mutex->lock_count == 0) {
        mutex->owner = task;
        mutex->lock_count = 1;
        mutex->original_prio = (task != NULL) ? task->base_priority : 0;
        syn_port_exit_critical();
        return SYN_OK;
    }

    /* Case 2: Recursive acquisition by the current owner */
    if (task != NULL && mutex->owner == task) {
        mutex->lock_count++;
        syn_port_exit_critical();
        return SYN_OK;
    }

    /* Case 3: Contended by another task — apply priority inheritance */
    if (task != NULL && mutex->owner != NULL) {
        if (task->priority < mutex->owner->priority) {
            syn_task_boost_priority(mutex->owner, task->priority);
        }
    }

    syn_port_exit_critical();
    return SYN_BUSY;
}

SYN_Status syn_mutex_unlock(SYN_Mutex *mutex, SYN_Task *task)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();

    if (mutex->lock_count == 0 || (task != NULL && mutex->owner != task)) {
        syn_port_exit_critical();
        return SYN_ERROR;
    }

    mutex->lock_count--;
    if (mutex->lock_count == 0) {
        if (mutex->owner != NULL) {
            syn_task_restore_priority(mutex->owner);
        }
        mutex->owner = NULL;
        mutex->original_prio = 0;
    }

    syn_port_exit_critical();
    return SYN_OK;
}

bool syn_mutex_is_locked(const SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return false;
    }
    return (mutex->lock_count > 0);
}

SYN_Task *syn_mutex_get_owner(const SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return NULL;
    }
    return mutex->owner;
}

#endif /* SYN_USE_MUTEX */
```


