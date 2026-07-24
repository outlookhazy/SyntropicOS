

# File syn\_spsc\_queue.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_spsc\_queue.c**](syn__spsc__queue_8c.md)

[Go to the documentation of this file](syn__spsc__queue_8c.md)


```C++

#include "syn_spsc_queue.h"
#include "../common/syn_barrier.h"
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static inline size_t spsc_advance(size_t idx, size_t capacity)
{
    size_t next = idx + 1;
    return (next >= capacity) ? 0 : next;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_spsc_queue_init(SYN_SPSC_Queue *q, void *elem_buf, size_t elem_size, size_t capacity)
{
    if (!q || !elem_buf || elem_size == 0 || capacity == 0) return SYN_INVALID_PARAM;

    q->buffer    = (uint8_t *)elem_buf;
    q->elem_size = elem_size;
    q->capacity  = capacity;
    q->head      = 0;
    q->tail      = 0;

    return SYN_OK;
}

bool syn_spsc_queue_is_empty(const SYN_SPSC_Queue *q)
{
    if (!q) return true;
    return (SYN_LOAD_ACQUIRE(&q->head) == q->tail);
}

bool syn_spsc_queue_is_full(const SYN_SPSC_Queue *q)
{
    if (!q) return false;
    size_t next_head = spsc_advance(q->head, q->capacity);
    return (next_head == SYN_LOAD_ACQUIRE(&q->tail));
}

size_t syn_spsc_queue_count(const SYN_SPSC_Queue *q)
{
    if (!q) return 0;
    size_t head = SYN_LOAD_ACQUIRE(&q->head);
    size_t tail = q->tail;
    if (head >= tail) {
        return head - tail;
    }
    return q->capacity - (tail - head);
}

SYN_Status syn_spsc_queue_push(SYN_SPSC_Queue *q, const void *item)
{
    if (!q || !item) return SYN_INVALID_PARAM;

    size_t current_head = q->head;
    size_t next_head = spsc_advance(current_head, q->capacity);

    if (next_head == SYN_LOAD_ACQUIRE(&q->tail)) {
        return SYN_BUSY; /* Queue full */
    }

    memcpy(q->buffer + (current_head * q->elem_size), item, q->elem_size);
    SYN_STORE_RELEASE(&q->head, next_head);

    return SYN_OK;
}

SYN_Status syn_spsc_queue_pop(SYN_SPSC_Queue *q, void *out_item)
{
    if (!q || !out_item) return SYN_INVALID_PARAM;

    size_t current_head = SYN_LOAD_ACQUIRE(&q->head);
    size_t current_tail = q->tail;

    if (current_head == current_tail) {
        return SYN_ERROR; /* Queue empty */
    }

    memcpy(out_item, q->buffer + (current_tail * q->elem_size), q->elem_size);
    size_t next_tail = spsc_advance(current_tail, q->capacity);
    SYN_STORE_RELEASE(&q->tail, next_tail);

    return SYN_OK;
}
```


