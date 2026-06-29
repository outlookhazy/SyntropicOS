/**
 * @file syn_workqueue.h
 * @brief Deferred work queue — ISR-safe function dispatch.
 *
 * ISRs push work items (function + context) into a lock-free ring.
 * The main loop drains the queue by calling syn_workqueue_process().
 *
 * @par Usage
 * @code
 *   SYN_WorkItem items[8];
 *   SYN_WorkQueue wq;
 *   syn_workqueue_init(&wq, items, 8);
 *
 *   // In ISR:
 *   syn_workqueue_post(&wq, my_handler, my_data);
 *
 *   // In main loop:
 *   syn_workqueue_process(&wq);
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_WORKQUEUE_H
#define SYN_WORKQUEUE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Work function signature. */
typedef void (*SYN_WorkFunc)(void *ctx);

/** Single work item. */
typedef struct {
    SYN_WorkFunc  func;         /**< Deferred callback function pointer */
    void          *ctx;          /**< User context pointer for the callback */
} SYN_WorkItem;

/** Work queue (lock-free SPSC ring). */
typedef struct {
    SYN_WorkItem  *items;        /**< Array of work item slots (caller-owned) */
    size_t          capacity;     /**< Maximum items the queue can hold */
    volatile size_t head;     /**< Written by producer (ISR)              */
    volatile size_t tail;     /**< Written by consumer (main loop)        */
    uint32_t        overflow; /**< Counter: posts dropped due to full     */
} SYN_WorkQueue;

/**
 * @brief Initialize the work queue.
 *
 * @param wq       Pointer to work queue context to initialize.
 * @param buf      Backing storage array for work items.
 * @param capacity Number of elements in the buffer.
 */
void syn_workqueue_init(SYN_WorkQueue *wq,
                         SYN_WorkItem *buf, size_t capacity);

/**
 * @brief Post a work item. ISR-safe (single producer).
 *
 * @param wq   Pointer to work queue context.
 * @param func Callback function to defer.
 * @param ctx  User data passed to the callback.
 * @return true if posted, false if queue is full.
 */
bool syn_workqueue_post(SYN_WorkQueue *wq,
                         SYN_WorkFunc func, void *ctx);

/**
 * @brief Process all pending work items.
 *
 * Call from main loop. Executes each queued function in FIFO order.
 *
 * @param wq Pointer to work queue context.
 * @return Number of items processed.
 */
size_t syn_workqueue_process(SYN_WorkQueue *wq);

/**
 * @brief Check if the queue is empty.
 *
 * @param wq Pointer to work queue context.
 * @return true if empty.
 */
static inline bool syn_workqueue_empty(const SYN_WorkQueue *wq)
{
    return wq->head == wq->tail;
}

/**
 * @brief Get the number of pending items.
 *
 * @param wq Pointer to work queue context.
 * @return Number of pending items.
 */
static inline size_t syn_workqueue_pending(const SYN_WorkQueue *wq)
{
    return (wq->head >= wq->tail)
         ? wq->head - wq->tail
         : wq->capacity - wq->tail + wq->head;
}

/**
 * @brief Get overflow count (posts dropped).
 *
 * @param wq Pointer to work queue context.
 * @return Count of dropped posts.
 */
static inline uint32_t syn_workqueue_overflows(const SYN_WorkQueue *wq)
{
    return wq->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_WORKQUEUE_H */
