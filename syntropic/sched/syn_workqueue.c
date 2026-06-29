#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WORKQUEUE) || SYN_USE_WORKQUEUE

/**
 * @file syn_workqueue.c
 * @brief Deferred work queue implementation.
 */

#include "syn_workqueue.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_workqueue_init(SYN_WorkQueue *wq,
                         SYN_WorkItem *buf, size_t capacity)
{
    SYN_ASSERT(wq != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(capacity > 1);

    wq->items    = buf;
    wq->capacity = capacity;
    wq->head     = 0;
    wq->tail     = 0;
    wq->overflow = 0;

    memset(buf, 0, sizeof(SYN_WorkItem) * capacity);
}

bool syn_workqueue_post(SYN_WorkQueue *wq,
                         SYN_WorkFunc func, void *ctx)
{
    size_t next = wq->head + 1;
    if (next >= wq->capacity) next = 0;

    if (next == wq->tail) {
        /* Queue full */
        wq->overflow++;
        return false;
    }

    wq->items[wq->head].func = func;
    wq->items[wq->head].ctx  = ctx;

    /* Memory barrier hint for ISR→main ordering.
     * On Cortex-M this is sufficient as stores are ordered. */
    wq->head = next;

    return true;
}

size_t syn_workqueue_process(SYN_WorkQueue *wq)
{
    size_t count = 0;

    while (wq->tail != wq->head) {
        SYN_WorkFunc func = wq->items[wq->tail].func;
        void         *ctx  = wq->items[wq->tail].ctx;

        size_t next = wq->tail + 1;
        if (next >= wq->capacity) next = 0;
        wq->tail = next;

        if (func != NULL) {
            func(ctx);
            count++;
        }
    }

    return count;
}

#endif /* SYN_USE_WORKQUEUE */
