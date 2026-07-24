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

    syn_spsc_queue_init(&wq->queue, buf, sizeof(SYN_WorkItem), capacity);
    wq->overflow = 0;

    memset(buf, 0, sizeof(SYN_WorkItem) * capacity);
}

bool syn_workqueue_post(SYN_WorkQueue *wq,
                         SYN_WorkFunc func, void *ctx)
{
    if (!wq || !func) return false;

    SYN_WorkItem item = {.func = func, .ctx = ctx};
    SYN_Status status = syn_spsc_queue_push(&wq->queue, &item);

    if (status != SYN_OK) {
        wq->overflow++;
        return false;
    }

    return true;
}

size_t syn_workqueue_process(SYN_WorkQueue *wq)
{
    if (!wq) return 0;
    size_t count = 0;
    SYN_WorkItem item;

    while (syn_spsc_queue_pop(&wq->queue, &item) == SYN_OK) {
        if (item.func != NULL) {
            item.func(item.ctx);
            count++;
        }
    }

    return count;
}

#endif /* SYN_USE_WORKQUEUE */
