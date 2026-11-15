

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
    if ((q == NULL) || (elem_buf == NULL) || (elem_size == 0U) || (capacity < 2U) ||
        (capacity > (SIZE_MAX / elem_size))) {
        return SYN_INVALID_PARAM;
    }

    q->buffer = (uint8_t *)elem_buf;
    q->elem_size = elem_size;
    q->capacity = capacity;
    SYN_STORE_RELEASE(&q->head, 0U);
    SYN_STORE_RELEASE(&q->tail, 0U);

    return SYN_OK;
}

bool syn_spsc_queue_is_empty(const SYN_SPSC_Queue *q)
{
    if (q == NULL || q->buffer == NULL || q->capacity < 2U) {
        return true;
    }
    size_t head = SYN_LOAD_ACQUIRE(&q->head);
    size_t tail = SYN_LOAD_ACQUIRE(&q->tail);
    return head == tail;
}

bool syn_spsc_queue_is_full(const SYN_SPSC_Queue *q)
{
    if (q == NULL || q->buffer == NULL || q->capacity < 2U) {
        return false;
    }
    size_t head = SYN_LOAD_ACQUIRE(&q->head);
    size_t tail = SYN_LOAD_ACQUIRE(&q->tail);
    return spsc_advance(head, q->capacity) == tail;
}

size_t syn_spsc_queue_count(const SYN_SPSC_Queue *q)
{
    if (q == NULL || q->buffer == NULL || q->capacity < 2U) {
        return 0U;
    }
    size_t head = SYN_LOAD_ACQUIRE(&q->head);
    size_t tail = SYN_LOAD_ACQUIRE(&q->tail);
    if (head >= tail) {
        return head - tail;
    }
    return q->capacity - (tail - head);
}

SYN_Status syn_spsc_queue_push(SYN_SPSC_Queue *q, const void *item)
{
    if (q == NULL || q->buffer == NULL || item == NULL || q->capacity < 2U || q->elem_size == 0U) {
        return SYN_INVALID_PARAM;
    }

    size_t current_head = SYN_LOAD_ACQUIRE(&q->head);
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
    if (q == NULL || q->buffer == NULL || out_item == NULL || q->capacity < 2U ||
        q->elem_size == 0U) {
        return SYN_INVALID_PARAM;
    }

    size_t current_head = SYN_LOAD_ACQUIRE(&q->head);
    size_t current_tail = SYN_LOAD_ACQUIRE(&q->tail);

    if (current_head == current_tail) {
        return SYN_ERROR; /* Queue empty */
    }

    memcpy(out_item, q->buffer + (current_tail * q->elem_size), q->elem_size);
    size_t next_tail = spsc_advance(current_tail, q->capacity);
    SYN_STORE_RELEASE(&q->tail, next_tail);

    return SYN_OK;
}
```


