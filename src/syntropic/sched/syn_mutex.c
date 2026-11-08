/**
 * @file syn_mutex.c
 * @brief Priority-inheriting recursive mutex implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_mutex.h"
#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
#include "syn_sched.h"
#endif

/** @cond INTERNAL */
static bool mutex_waiter_is_active(const SYN_Task *w)
{
    return (w != NULL) && ((w->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U) &&
           (w->state == (uint8_t)SYN_TASK_READY) && (w->delay_until == 0U);
}

static void mutex_recompute_owner_prio(SYN_Task *owner)
{
#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
    syn_sched_recompute_task_prio(owner);
#else
    uint8_t target = owner->base_priority;
    if (owner->manual_priority < target) {
        target = owner->manual_priority;
    }
    for (SYN_Mutex *m = owner->held_mutexes; m != NULL; m = m->next_held) {
        for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
            const SYN_Task *w = m->waiters[i];
            if (mutex_waiter_is_active(w) && w->priority < target) {
                target = w->priority;
            }
        }
    }
    owner->priority = target;
#endif
}

static void mutex_clear_task_wait(SYN_Task *task)
{
#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
    syn_sched_clear_mutex_wait(task);
#else
    task->sched_flags &= (uint8_t)~SYN_SCHED_FLAG_MUTEX_WAIT;
#endif
}

static void mutex_record_waiter(SYN_Mutex *mutex, SYN_Task *task)
{
    size_t free_idx = SYN_MUTEX_MAX_WAITERS;
    size_t worst_idx = SYN_MUTEX_MAX_WAITERS;
    bool present = false;

    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        SYN_Task *w = mutex->waiters[i];
        if (!mutex_waiter_is_active(w)) {
            mutex->waiters[i] = NULL;
            if (free_idx == SYN_MUTEX_MAX_WAITERS) {
                free_idx = i;
            }
            continue;
        }
        if (w == task) {
            present = true;
        }
        if (worst_idx == SYN_MUTEX_MAX_WAITERS ||
            w->priority > mutex->waiters[worst_idx]->priority) {
            worst_idx = i;
        }
    }

    task->sched_flags |= SYN_SCHED_FLAG_MUTEX_WAIT;
    if (present || !mutex_waiter_is_active(task)) {
        return;
    }
    if (free_idx < SYN_MUTEX_MAX_WAITERS) {
        mutex->waiters[free_idx] = task;
    } else if (task->priority < mutex->waiters[worst_idx]->priority) {
        mutex->waiters[worst_idx] = task;
    }
}

static void mutex_unlink_held(SYN_Task *owner, const SYN_Mutex *mutex)
{
    SYN_Mutex **pp = &owner->held_mutexes;
    while (*pp != NULL) {
        if (*pp == mutex) {
            *pp = (*pp)->next_held;
            return;
        }
        pp = &(*pp)->next_held;
    }
}

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
static bool mutex_is_tracked_in_held(const SYN_Sched *sched, const SYN_Task *task)
{
    for (size_t i = 0U; i < sched->task_count; i++) {
        for (const SYN_Mutex *m = sched->tasks[i].held_mutexes; m != NULL; m = m->next_held) {
            for (size_t j = 0U; j < SYN_MUTEX_MAX_WAITERS; j++) {
                if (m->waiters[j] == task) {
                    return true;
                }
            }
        }
    }
    return false;
}

static void mutex_wake_evicted_waiters(const SYN_Sched *sched)
{
    if (sched == NULL) {
        return;
    }
    uint8_t clr = (uint8_t)~(SYN_SCHED_FLAG_PASSED | SYN_SCHED_FLAG_MUTEX_WAIT);
    for (size_t i = 0U; i < sched->task_count; i++) {
        SYN_Task *t = &sched->tasks[i];
        if (mutex_waiter_is_active(t) && !mutex_is_tracked_in_held(sched, t)) {
            t->sched_flags = (uint8_t)((t->sched_flags & clr) | SYN_SCHED_FLAG_MORE);
        }
    }
}
#endif

static void mutex_contend_waiter(SYN_Mutex *mutex, SYN_Task *task)
{
    if (task != NULL && mutex->owner != NULL) {
        mutex_record_waiter(mutex, task);
        mutex_recompute_owner_prio(mutex->owner);
    }
}

static SYN_Status mutex_acquire_impl(SYN_Mutex *mutex, SYN_Task *task, bool wait_on_busy)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();

    /* Case 1: Mutex is currently unlocked */
    if (mutex->owner == NULL && mutex->lock_count == 0U) {
        mutex->owner = task;
        mutex->lock_count = 1U;
        if (task != NULL) {
            mutex_clear_task_wait(task);
            mutex->next_held = task->held_mutexes;
            task->held_mutexes = mutex;
        } else {
            mutex->next_held = NULL;
        }
        syn_port_exit_critical();
        return SYN_OK;
    }

    /* Case 2: Recursive acquisition by the current owner */
    if (task != NULL && mutex->owner == task) {
        if (mutex->lock_count == UINT16_MAX) {
            syn_port_exit_critical();
            return SYN_ERROR;
        }
        mutex_clear_task_wait(task);
        mutex->lock_count++;
        syn_port_exit_critical();
        return SYN_OK;
    }

    /* Case 3: Contended by another task */
    if (wait_on_busy) {
        mutex_contend_waiter(mutex, task);
    }

    syn_port_exit_critical();
    return SYN_BUSY;
}
/** @endcond */

SYN_Status syn_mutex_init(SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    mutex->owner = NULL;
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        mutex->waiters[i] = NULL;
    }
    mutex->next_held = NULL;
    mutex->lock_count = 0U;

    return SYN_OK;
}

SYN_Status syn_mutex_try_lock(SYN_Mutex *mutex, SYN_Task *task)
{
    return mutex_acquire_impl(mutex, task, true);
}

SYN_Status syn_mutex_probe(SYN_Mutex *mutex, SYN_Task *task)
{
    return mutex_acquire_impl(mutex, task, false);
}

SYN_Status syn_mutex_unlock(SYN_Mutex *mutex, SYN_Task *task)
{
    if (mutex == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();

    if (mutex->lock_count == 0U || mutex->owner != task) {
        syn_port_exit_critical();
        return SYN_ERROR;
    }

    mutex->lock_count--;
    if (mutex->lock_count == 0U) {
        SYN_Task *prev_owner = mutex->owner;
        mutex->owner = NULL;

        for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
            SYN_Task *w = mutex->waiters[i];
            mutex->waiters[i] = NULL;
            if (mutex_waiter_is_active(w)) {
                uint8_t clr = (uint8_t)~(SYN_SCHED_FLAG_PASSED | SYN_SCHED_FLAG_MUTEX_WAIT);
                w->sched_flags = (uint8_t)((w->sched_flags & clr) | SYN_SCHED_FLAG_MORE);
            }
        }

        if (prev_owner != NULL) {
            mutex_unlink_held(prev_owner, mutex);
            mutex->next_held = NULL;
#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
            mutex_wake_evicted_waiters(prev_owner->sched);
#endif
            mutex_recompute_owner_prio(prev_owner);
        }
    }

    syn_port_exit_critical();
    return SYN_OK;
}

bool syn_mutex_is_locked(const SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return false;
    }
    return (mutex->lock_count > 0U);
}

SYN_Task *syn_mutex_get_owner(const SYN_Mutex *mutex)
{
    if (mutex == NULL) {
        return NULL;
    }
    return mutex->owner;
}

#endif /* SYN_USE_MUTEX */
