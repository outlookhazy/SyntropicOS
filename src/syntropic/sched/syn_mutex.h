/**
 * @file syn_mutex.h
 * @brief Priority-inheriting mutual exclusion primitive for tasks and protothreads.
 * @ingroup syn_sched
 *
 * Provides recursive mutual exclusion with priority inheritance to prevent
 * unbounded priority inversion. If a higher-priority task attempts to acquire
 * a mutex held by a lower-priority task, the owner's priority is temporarily
 * boosted to match the waiting task until the mutex is released.
 */

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

/** @brief Maximum concurrent priority-tracked waiters per mutex. */
#ifndef SYN_MUTEX_MAX_WAITERS
#define SYN_MUTEX_MAX_WAITERS 4U
#endif

/**
 * @brief Priority-inheriting recursive mutex structure.
 */
typedef struct SYN_Mutex {
    SYN_Task *owner;                          /**< Current owner task (NULL if unlocked) */
    SYN_Task *waiters[SYN_MUTEX_MAX_WAITERS]; /**< Active contended waiter tasks         */
    struct SYN_Mutex *next_held;              /**< Next mutex in owner lock list         */
    struct SYN_Mutex *next_contended;         /**< Next mutex in wait_sched list         */
    struct SYN_Sched *wait_sched;             /**< Scheduler tracking external waiters   */
    uint16_t lock_count;                      /**< Recursive lock acquisition depth      */
    bool has_evicted;                         /**< True if waiters[] overflowed          */
} SYN_Mutex;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a mutex in unlocked state.
 * @param mutex Pointer to mutex structure.
 * @return SYN_OK on success, or SYN_INVALID_PARAM if mutex is NULL.
 */
SYN_Status syn_mutex_init(SYN_Mutex *mutex);

/**
 * @brief Attempt to acquire the mutex without blocking.
 *
 * If the mutex is already locked by another task and @p task has higher priority
 * (numerically lower value) than the owner, the owner's priority is automatically
 * boosted.
 *
 * @note Single-scheduler (AMP) scope: all scheduled tasks (`task->sched != NULL`)
 *       acquiring or contending on a mutex must belong to the same `SYN_Sched`
 *       instance (`SYN_INVALID_PARAM` otherwise). Use `syn_port_spinlock.h` or
 *       SPSC `syn_mailbox` across AMP cores.
 *
 * @param mutex Pointer to mutex structure.
 * @param task  Pointer to acquiring task (optional, NULL for standalone lock).
 * @return SYN_OK if acquired, SYN_BUSY if held by another task, or SYN_INVALID_PARAM.
 */
SYN_Status syn_mutex_try_lock(SYN_Mutex *mutex, SYN_Task *task);

/**
 * @brief Probe and acquire the mutex without registering as a contended waiter.
 *
 * Acquires an unlocked or recursively-held mutex like syn_mutex_try_lock(),
 * but on contention returns SYN_BUSY without adding @p task to the waiter list
 * or boosting the current owner.
 *
 * @note Single-scheduler (AMP) scope: returns `SYN_INVALID_PARAM` if contended
 *       against a different `SYN_Sched` instance.
 *
 * @param mutex Pointer to mutex structure.
 * @param task  Pointer to acquiring task (optional, NULL for standalone lock).
 * @return SYN_OK if acquired, SYN_BUSY if held by another task, or SYN_INVALID_PARAM.
 */
SYN_Status syn_mutex_probe(SYN_Mutex *mutex, SYN_Task *task);

/**
 * @brief Release a previously acquired mutex.
 *
 * Decrements the recursion count. When the count reaches zero, the owner's
 * priority is restored to its base priority and ownership is cleared.
 *
 * @param mutex Pointer to mutex structure.
 * @param task  Pointer to owner task releasing the lock.
 * @return SYN_OK on success, SYN_ERROR if not locked or wrong owner, or SYN_INVALID_PARAM.
 */
SYN_Status syn_mutex_unlock(SYN_Mutex *mutex, SYN_Task *task);

/**
 * @brief Check if the mutex is currently locked.
 * @param mutex Pointer to mutex structure.
 * @return true if locked, false otherwise.
 */
bool syn_mutex_is_locked(const SYN_Mutex *mutex);

/**
 * @brief Get the current owner of the mutex.
 * @param mutex Pointer to mutex structure.
 * @return Pointer to owner task, or NULL if unlocked.
 */
SYN_Task *syn_mutex_get_owner(const SYN_Mutex *mutex);

/* ── Protothread Macros ─────────────────────────────────────────────────── */

/**
 * @brief Initialize a mutex.
 */
#define PT_MUTEX_INIT(mutex) syn_mutex_init(mutex)

/**
 * @brief Block the protothread until the mutex is acquired.
 *
 * Cooperatively waits via PT_WAIT_UNTIL (polled; keeps task READY and prevents
 * tickless sleep while contended). On contention, automatically boosts the
 * holding task's priority to prevent unbounded priority inversion.
 *
 * @param pt    Protothread control block.
 * @param task  Pointer to calling SYN_Task.
 * @param mutex Pointer to SYN_Mutex.
 */
#define PT_MUTEX_LOCK(pt, task, mutex) \
    PT_WAIT_UNTIL(pt, syn_mutex_try_lock((mutex), (task)) == SYN_OK)

/**
 * @brief Unlock a mutex held by the task.
 *
 * @param mutex Pointer to SYN_Mutex.
 * @param task  Pointer to calling SYN_Task.
 */
#define PT_MUTEX_UNLOCK(mutex, task) syn_mutex_unlock((mutex), (task))

/**
 * @brief Non-blocking try-lock macro for protothreads.
 *
 * @param mutex Pointer to SYN_Mutex.
 * @param task  Pointer to calling SYN_Task.
 * @return true if acquired, false if busy.
 */
#define PT_MUTEX_TRYLOCK(mutex, task) (syn_mutex_probe((mutex), (task)) == SYN_OK)

#ifdef __cplusplus
}
#endif

#endif /* SYN_MUTEX_H */
