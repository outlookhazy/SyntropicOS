

# File syn\_workqueue.c

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_workqueue.c**](syn__workqueue_8c.md)

[Go to the documentation of this file](syn__workqueue_8c.md)


```C++
#include "../common/syn_defs.h"

#if !defined(SYN_USE_WORKQUEUE) || SYN_USE_WORKQUEUE


#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_workqueue.h"

#include <string.h>

void syn_workqueue_init(SYN_WorkQueue *wq, SYN_WorkItem *buf, size_t capacity)
{
    SYN_ASSERT(wq != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(capacity > 1);

    if (wq == NULL || buf == NULL || capacity <= 1) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    syn_port_enter_critical();
    wq->overflow = 0U;
    if (capacity > (SIZE_MAX / sizeof(SYN_WorkItem))) {
        memset(&wq->queue, 0, sizeof(wq->queue));
        syn_port_exit_critical();
        return;
    }

    memset(buf, 0, sizeof(SYN_WorkItem) * capacity);
    (void)syn_spsc_queue_init(&wq->queue, buf, sizeof(SYN_WorkItem), capacity);
    syn_port_exit_critical();
}

bool syn_workqueue_post(SYN_WorkQueue *wq, SYN_WorkFunc func, void *ctx)
{
    if (!wq || !func)
        return false;

    syn_port_enter_critical();
    SYN_WorkItem item = {.func = func, .ctx = ctx};
    SYN_Status status = syn_spsc_queue_push(&wq->queue, &item);

    if (status == SYN_BUSY) {
        wq->overflow++;
    }
    if (status != SYN_OK) {
        syn_port_exit_critical();
        return false;
    }

    syn_port_exit_critical();
    return true;
}

size_t syn_workqueue_process(SYN_WorkQueue *wq)
{
    if (!wq)
        return 0;
    size_t budget = syn_spsc_queue_count(&wq->queue);
    size_t count = 0;
    SYN_WorkItem item;

    while (budget > 0U && syn_spsc_queue_pop(&wq->queue, &item) == SYN_OK) {
        budget--;
        if (item.func != NULL) {
            item.func(item.ctx);
            count++;
        }
    }

    return count;
}

#endif /* SYN_USE_WORKQUEUE */
```


