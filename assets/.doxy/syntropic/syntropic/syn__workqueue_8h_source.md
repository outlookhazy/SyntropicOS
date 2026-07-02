

# File syn\_workqueue.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_workqueue.h**](syn__workqueue_8h.md)

[Go to the documentation of this file](syn__workqueue_8h.md)


```C++

#ifndef SYN_WORKQUEUE_H
#define SYN_WORKQUEUE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SYN_WorkFunc)(void *ctx);

typedef struct {
    SYN_WorkFunc  func;         
    void          *ctx;          
} SYN_WorkItem;

typedef struct {
    SYN_WorkItem  *items;        
    size_t          capacity;     
    volatile size_t head;     
    volatile size_t tail;     
    uint32_t        overflow; 
} SYN_WorkQueue;

void syn_workqueue_init(SYN_WorkQueue *wq,
                         SYN_WorkItem *buf, size_t capacity);

bool syn_workqueue_post(SYN_WorkQueue *wq,
                         SYN_WorkFunc func, void *ctx);

size_t syn_workqueue_process(SYN_WorkQueue *wq);

static inline bool syn_workqueue_empty(const SYN_WorkQueue *wq)
{
    return wq->head == wq->tail;
}

static inline size_t syn_workqueue_pending(const SYN_WorkQueue *wq)
{
    return (wq->head >= wq->tail)
         ? wq->head - wq->tail
         : wq->capacity - wq->tail + wq->head;
}

static inline uint32_t syn_workqueue_overflows(const SYN_WorkQueue *wq)
{
    return wq->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_WORKQUEUE_H */
```


