

# File syn\_ringbuf.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ringbuf.c**](syn__ringbuf_8c.md)

[Go to the documentation of this file](syn__ringbuf_8c.md)


```C++

#include "syn_ringbuf.h"
#include "../util/syn_assert.h"
#include "../common/syn_barrier.h"
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static inline size_t advance(size_t idx, size_t size)
{
    size_t next = idx + 1;
    return (next >= size) ? 0 : next;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_ringbuf_init(SYN_RingBuf *rb, uint8_t *buf, size_t size)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(size > 1); /* need at least 2 bytes (1 usable + 1 sentinel) */

    rb->buf  = buf;
    rb->size = size;
    SYN_STORE_RELEASE(&rb->head, 0);
    SYN_STORE_RELEASE(&rb->tail, 0);
}

void syn_ringbuf_reset(SYN_RingBuf *rb)
{
    SYN_ASSERT(rb != NULL);
    SYN_STORE_RELEASE(&rb->head, 0);
    SYN_STORE_RELEASE(&rb->tail, 0);
}

bool syn_ringbuf_put(SYN_RingBuf *rb, uint8_t byte)
{
    SYN_ASSERT(rb != NULL);

    size_t head = SYN_LOAD_ACQUIRE(&rb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);
    size_t next_head = advance(head, rb->size);
    if (next_head == tail) {
        return false; /* full */
    }

    rb->buf[head] = byte;
    SYN_STORE_RELEASE(&rb->head, next_head);
    return true;
}

bool syn_ringbuf_get(SYN_RingBuf *rb, uint8_t *byte)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(byte != NULL);

    size_t head = SYN_LOAD_ACQUIRE(&rb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);
    if (head == tail) {
        return false; /* empty */
    }

    *byte = rb->buf[tail];
    SYN_STORE_RELEASE(&rb->tail, advance(tail, rb->size));
    return true;
}

bool syn_ringbuf_peek(const SYN_RingBuf *rb, uint8_t *byte)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(byte != NULL);

    size_t head = SYN_LOAD_ACQUIRE(&rb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);
    if (head == tail) {
        return false;
    }

    *byte = rb->buf[tail];
    return true;
}

bool syn_ringbuf_full(const SYN_RingBuf *rb)
{
    SYN_ASSERT(rb != NULL);
    size_t head = SYN_LOAD_ACQUIRE(&rb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);
    return advance(head, rb->size) == tail;
}

bool syn_ringbuf_empty(const SYN_RingBuf *rb)
{
    SYN_ASSERT(rb != NULL);
    size_t head = SYN_LOAD_ACQUIRE(&rb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);
    return head == tail;
}

size_t syn_ringbuf_count(const SYN_RingBuf *rb)
{
    SYN_ASSERT(rb != NULL);

    size_t h = SYN_LOAD_ACQUIRE(&rb->head);
    size_t t = SYN_LOAD_ACQUIRE(&rb->tail);

    if (h >= t) {
        return h - t;
    }
    return rb->size - t + h;
}

size_t syn_ringbuf_free(const SYN_RingBuf *rb)
{
    SYN_ASSERT(rb != NULL);
    /* Capacity is size - 1 (one slot reserved as sentinel). */
    return (rb->size - 1) - syn_ringbuf_count(rb);
}

size_t syn_ringbuf_write(SYN_RingBuf *rb, const uint8_t *data, size_t len)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(data != NULL || len == 0);

    size_t avail = syn_ringbuf_free(rb);
    if (len > avail) {
        len = avail;
    }
    if (len == 0) {
        return 0;
    }

    size_t head = SYN_LOAD_ACQUIRE(&rb->head);

    /* Bytes from head to end of backing array */
    size_t to_end = rb->size - head;

    if (len <= to_end) {
        /* No wrap needed */
        memcpy(&rb->buf[head], data, len);
    } else {
        /* Two-part copy across the wrap boundary */
        memcpy(&rb->buf[head], data, to_end);
        memcpy(&rb->buf[0], data + to_end, len - to_end);
    }

    size_t new_head = head + len;
    SYN_STORE_RELEASE(&rb->head, (new_head >= rb->size) ? new_head - rb->size : new_head);
    return len;
}

size_t syn_ringbuf_read(SYN_RingBuf *rb, uint8_t *data, size_t len)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(data != NULL || len == 0);

    size_t avail = syn_ringbuf_count(rb);
    if (len > avail) {
        len = avail;
    }
    if (len == 0) {
        return 0;
    }

    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);

    /* Bytes from tail to end of backing array */
    size_t to_end = rb->size - tail;

    if (len <= to_end) {
        /* No wrap needed */
        memcpy(data, &rb->buf[tail], len);
    } else {
        /* Two-part copy across the wrap boundary */
        memcpy(data, &rb->buf[tail], to_end);
        memcpy(data + to_end, &rb->buf[0], len - to_end);
    }

    size_t new_tail = tail + len;
    SYN_STORE_RELEASE(&rb->tail, (new_tail >= rb->size) ? new_tail - rb->size : new_tail);
    return len;
}

size_t syn_ringbuf_peek_n(const SYN_RingBuf *rb, uint8_t *data, size_t len)
{
    SYN_ASSERT(rb != NULL);
    SYN_ASSERT(data != NULL || len == 0);

    size_t avail = syn_ringbuf_count(rb);
    if (len > avail) {
        len = avail;
    }
    if (len == 0) {
        return 0;
    }

    size_t tail = SYN_LOAD_ACQUIRE(&rb->tail);

    /* Bytes from tail to end of backing array */
    size_t to_end = rb->size - tail;

    if (len <= to_end) {
        /* No wrap needed */
        memcpy(data, &rb->buf[tail], len);
    } else {
        /* Two-part copy across the wrap boundary */
        memcpy(data, &rb->buf[tail], to_end);
        memcpy(data + to_end, &rb->buf[0], len - to_end);
    }

    /* Tail is NOT advanced — this is a peek, not a consume */
    return len;
}
```


