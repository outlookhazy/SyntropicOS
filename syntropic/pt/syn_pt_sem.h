/**
 * @file syn_pt_sem.h
 * @brief Lightweight counting semaphores for protothreads.
 *
 * Provides inter-protothread synchronization with zero RAM overhead
 * beyond the 2-byte counter itself. All operations are macro/inline —
 * no .c file is needed.
 *
 * @par Usage
 * @code
 *   static SYN_PT_Sem data_ready;
 *   PT_SEM_INIT(&data_ready, 0);
 *
 *   // Producer (can be called from ISR):
 *   PT_SEM_SIGNAL(&data_ready);
 *
 *   // Consumer (inside a protothread):
 *   PT_SEM_WAIT(pt, &data_ready);
 *   // ... data is now available ...
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_PT_SEM_H
#define SYN_PT_SEM_H

#include "syn_pt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Semaphore structure ────────────────────────────────────────────────── */

/**
 * @brief Counting semaphore for protothreads.
 *
 * Costs 2 bytes of RAM. The count is signed to allow detecting underflow
 * bugs, but normal operation keeps it >= 0.
 */
typedef struct {
    volatile int16_t count;  /**< Current semaphore count (>= 0 means available) */
} SYN_PT_Sem;

/* ── Initialization ─────────────────────────────────────────────────────── */

/**
 * @brief Initialize a semaphore with the given count.
 *
 * @param sem      Pointer to the semaphore.
 * @param initial  Initial count (typically 0 or 1).
 */
#define PT_SEM_INIT(sem, initial)   ((sem)->count = (int16_t)(initial))

/* ── Blocking wait (protothread context) ────────────────────────────────── */

/**
 * @brief Block the protothread until the semaphore count is > 0,
 *        then decrement it.
 *
 * Must be called from within a protothread (between PT_BEGIN/PT_END).
 *
 * @param pt   Protothread.
 * @param sem  Semaphore to wait on.
 */
#define PT_SEM_WAIT(pt, sem)                                  \
    do {                                                       \
        PT_WAIT_UNTIL(pt, (sem)->count > 0);                   \
        (sem)->count--;                                        \
    } while (0)

/* ── Signal (any context) ───────────────────────────────────────────────── */

/**
 * @brief Increment the semaphore count, unblocking a waiting protothread.
 *
 * Safe to call from ISR context.
 *
 * @param sem  Semaphore to signal.
 */
#define PT_SEM_SIGNAL(sem)   ((sem)->count++)

/* ── Non-blocking try ───────────────────────────────────────────────────── */

/**
 * @brief Try to acquire the semaphore without blocking.
 *
 * @param sem  Semaphore to try.
 * @return 1 if acquired (count was > 0 and was decremented), 0 otherwise.
 */
static inline int pt_sem_trywait(SYN_PT_Sem *sem)
{
    if (sem->count > 0) {
        sem->count--;
        return 1;
    }
    return 0;
}

/**
 * @brief Macro wrapper for pt_sem_trywait.
 */
#define PT_SEM_TRYWAIT(sem)   pt_sem_trywait(sem)

/* ── Query ──────────────────────────────────────────────────────────────── */

/** Return the current count. */
#define PT_SEM_COUNT(sem)     ((sem)->count)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_SEM_H */
