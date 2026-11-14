

# File syn\_mailbox.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mailbox.h**](syn__mailbox_8h.md)

[Go to the documentation of this file](syn__mailbox_8h.md)


```C++


#ifndef SYN_MAILBOX_H
#define SYN_MAILBOX_H

#include "../common/syn_barrier.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Mailbox instance ───────────────────────────────────────────────────── */

typedef struct {
    uint8_t *buf;               
    size_t msg_size;            
    size_t capacity;            
    volatile size_t head;       
    volatile size_t tail;       
    volatile uint32_t overflow; 
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    volatile uint8_t notify; 
#endif
} SYN_Mailbox;

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#define SYN_MAILBOX_DEFINE(name, type, count) \
    static type name##_buf[(count) + 1U];     \
    static SYN_Mailbox name = {               \
        .buf = (uint8_t *)name##_buf,         \
        .msg_size = sizeof(type),             \
        .capacity = ((count) + 1U),           \
        .head = 0,                            \
        .tail = 0,                            \
        .overflow = 0,                        \
        .notify = 0U,                         \
    }
#else
#define SYN_MAILBOX_DEFINE(name, type, count) \
    static type name##_buf[(count) + 1U];     \
    static SYN_Mailbox name = {               \
        .buf = (uint8_t *)name##_buf,         \
        .msg_size = sizeof(type),             \
        .capacity = ((count) + 1U),           \
        .head = 0,                            \
        .tail = 0,                            \
        .overflow = 0,                        \
    }
#endif

/* ── Forward declaration for IPC notify ────────────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
void syn_port_ipc_notify(void);
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_mailbox_set_overflow(SYN_Mailbox *mb, uint32_t val)
{
#if defined(SYN_BARRIER_TSAN)
    __atomic_store_n(&mb->overflow, val, __ATOMIC_RELAXED);
#else
    mb->overflow = val;
#endif
}

static inline void syn_mailbox_inc_overflow(SYN_Mailbox *mb)
{
#if defined(SYN_BARRIER_TSAN)
    uint32_t cur = __atomic_load_n(&mb->overflow, __ATOMIC_RELAXED);
    __atomic_store_n(&mb->overflow, cur + 1U, __ATOMIC_RELAXED);
#else
    mb->overflow = mb->overflow + 1U;
#endif
}

static inline void syn_mailbox_init(SYN_Mailbox *mb, void *buf, size_t msg_size, size_t capacity)
{
    bool bad_cap = (msg_size > 0U) && (capacity > (SIZE_MAX / msg_size));
    mb->buf = bad_cap ? NULL : (uint8_t *)buf;
    mb->msg_size = msg_size;
    mb->capacity = bad_cap ? 0U : capacity;
    mb->head = 0;
    mb->tail = 0;
    syn_mailbox_set_overflow(mb, 0U);
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#if defined(SYN_BARRIER_TSAN)
    __atomic_store_n(&mb->notify, 0U, __ATOMIC_RELAXED);
#else
    mb->notify = 0U;
#endif
#endif
}

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
static inline void syn_mailbox_set_notify(SYN_Mailbox *mb, bool enable)
{
    uint8_t val = enable ? 1U : 0U;
#if defined(SYN_BARRIER_TSAN)
    __atomic_store_n(&mb->notify, val, __ATOMIC_SEQ_CST);
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    syn_port_memory_barrier();
#else
    mb->notify = val;
    SYN_COMPILER_BARRIER();
    syn_port_memory_barrier();
#endif
}
#endif

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 10)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-use-of-uninitialized-value"
#endif
static inline bool syn_mailbox_post(SYN_Mailbox *mb, const void *msg)
{
    if (mb->msg_size == 0U || mb->buf == NULL || msg == NULL ||
        mb->capacity > (SIZE_MAX / mb->msg_size)) {
        return false;
    }
    if (mb->capacity <= 1U) {
        syn_mailbox_inc_overflow(mb);
        return false;
    }

    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    size_t next = head + 1U;
    if (next >= mb->capacity)
        next = 0U;

    if (next == SYN_LOAD_ACQUIRE(&mb->tail)) {
        syn_mailbox_inc_overflow(mb);
        return false;
    }

    memcpy(&mb->buf[head * mb->msg_size], msg, mb->msg_size);
    SYN_STORE_RELEASE(&mb->head, next);

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#if defined(SYN_BARRIER_TSAN)
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    syn_port_memory_barrier();
    if (__atomic_load_n(&mb->notify, __ATOMIC_SEQ_CST) != 0U) {
        syn_port_ipc_notify();
    }
#else
    SYN_COMPILER_BARRIER();
    syn_port_memory_barrier();
    if (mb->notify != 0U) {
        syn_port_ipc_notify();
    }
#endif
#endif

    return true;
}
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 10)
#pragma GCC diagnostic pop
#endif

static inline bool syn_mailbox_receive(SYN_Mailbox *mb, void *msg)
{
    if (mb->msg_size == 0U || mb->buf == NULL || msg == NULL ||
        mb->capacity > (SIZE_MAX / mb->msg_size))
        return false;

    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&mb->tail);

    if (tail == head)
        return false;

    memcpy(msg, &mb->buf[tail * mb->msg_size], mb->msg_size);

    size_t next = tail + 1U;
    if (next >= mb->capacity)
        next = 0U;
    SYN_STORE_RELEASE(&mb->tail, next);

    return true;
}

static inline const void *syn_mailbox_peek(const SYN_Mailbox *mb)
{
    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    size_t tail = SYN_LOAD_ACQUIRE(&mb->tail);
    if (head == tail || mb->buf == NULL || mb->msg_size == 0U ||
        mb->capacity > (SIZE_MAX / mb->msg_size))
        return NULL;
    return &mb->buf[tail * mb->msg_size];
}

static inline bool syn_mailbox_empty(const SYN_Mailbox *mb)
{
    return SYN_LOAD_ACQUIRE(&mb->head) == SYN_LOAD_ACQUIRE(&mb->tail);
}

static inline bool syn_mailbox_full(const SYN_Mailbox *mb)
{
    if (mb->capacity <= 1U)
        return true;
    size_t next = SYN_LOAD_ACQUIRE(&mb->head) + 1U;
    if (next >= mb->capacity)
        next = 0U;
    return next == SYN_LOAD_ACQUIRE(&mb->tail);
}

static inline size_t syn_mailbox_pending(const SYN_Mailbox *mb)
{
    if (mb->capacity <= 1U)
        return 0U;
    size_t h = SYN_LOAD_ACQUIRE(&mb->head);
    size_t t = SYN_LOAD_ACQUIRE(&mb->tail);
    return (h >= t) ? (h - t) : (mb->capacity - t + h);
}

static inline size_t syn_mailbox_free(const SYN_Mailbox *mb)
{
    /* One slot is always reserved as sentinel */
    if (mb->capacity <= 1U)
        return 0U;
    size_t pending = syn_mailbox_pending(mb);
    size_t usable = mb->capacity - 1U;
    return (usable > pending) ? (usable - pending) : 0U;
}

static inline uint32_t syn_mailbox_overflows(const SYN_Mailbox *mb)
{
#if defined(SYN_BARRIER_TSAN)
    return __atomic_load_n(&mb->overflow, __ATOMIC_RELAXED);
#else
    return mb->overflow;
#endif
}

static inline void syn_mailbox_flush(SYN_Mailbox *mb)
{
    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    SYN_STORE_RELEASE(&mb->tail, head);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_MAILBOX_H */
```


