

# File syn\_mailbox.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_mailbox.h**](syn__mailbox_8h.md)

[Go to the documentation of this file](syn__mailbox_8h.md)


```C++

#ifndef SYN_MAILBOX_H
#define SYN_MAILBOX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "../common/syn_barrier.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Mailbox instance ───────────────────────────────────────────────────── */

typedef struct {
    uint8_t        *buf;          
    size_t          msg_size;     
    size_t          capacity;     
    volatile size_t head;         
    volatile size_t tail;         
    uint32_t        overflow;     
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    bool            notify;       
#endif
} SYN_Mailbox;

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#define SYN_MAILBOX_DEFINE(name, type, count)                      \
    static uint8_t name##_buf[(count) * sizeof(type)];              \
    static SYN_Mailbox name = {                                    \
        .buf      = name##_buf,                                     \
        .msg_size = sizeof(type),                                   \
        .capacity = (count),                                        \
        .head     = 0,                                              \
        .tail     = 0,                                              \
        .overflow = 0,                                              \
        .notify   = false,                                          \
    }
#else
#define SYN_MAILBOX_DEFINE(name, type, count)                      \
    static uint8_t name##_buf[(count) * sizeof(type)];              \
    static SYN_Mailbox name = {                                    \
        .buf      = name##_buf,                                     \
        .msg_size = sizeof(type),                                   \
        .capacity = (count),                                        \
        .head     = 0,                                              \
        .tail     = 0,                                              \
        .overflow = 0,                                              \
    }
#endif

/* ── Forward declaration for IPC notify ────────────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
void syn_port_ipc_notify(void);
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_mailbox_init(SYN_Mailbox *mb,
                                     void *buf,
                                     size_t msg_size,
                                     size_t capacity)
{
    mb->buf      = (uint8_t *)buf;
    mb->msg_size = msg_size;
    mb->capacity = capacity;
    mb->head     = 0;
    mb->tail     = 0;
    mb->overflow = 0;
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    mb->notify   = false;
#endif
}

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
static inline void syn_mailbox_set_notify(SYN_Mailbox *mb, bool enable)
{
    mb->notify = enable;
}
#endif

static inline bool syn_mailbox_post(SYN_Mailbox *mb, const void *msg)
{
    size_t head = mb->head;
    size_t next = head + 1;
    if (next >= mb->capacity) next = 0;

    if (next == SYN_LOAD_ACQUIRE(&mb->tail)) {
        mb->overflow++;
        return false;
    }

    memcpy(&mb->buf[head * mb->msg_size], msg, mb->msg_size);
    SYN_STORE_RELEASE(&mb->head, next);

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    if (mb->notify) {
        syn_port_ipc_notify();
    }
#endif

    return true;
}

static inline bool syn_mailbox_receive(SYN_Mailbox *mb, void *msg)
{
    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    size_t tail = mb->tail;

    if (tail == head) return false;

    memcpy(msg, &mb->buf[tail * mb->msg_size], mb->msg_size);

    size_t next = tail + 1;
    if (next >= mb->capacity) next = 0;
    SYN_STORE_RELEASE(&mb->tail, next);

    return true;
}

static inline const void *syn_mailbox_peek(const SYN_Mailbox *mb)
{
    if (SYN_LOAD_ACQUIRE(&mb->head) == mb->tail) return NULL;
    return &mb->buf[mb->tail * mb->msg_size];
}

static inline bool syn_mailbox_empty(const SYN_Mailbox *mb)
{
    return SYN_LOAD_ACQUIRE(&mb->head) == mb->tail;
}

static inline bool syn_mailbox_full(const SYN_Mailbox *mb)
{
    size_t next = mb->head + 1;
    if (next >= mb->capacity) next = 0;
    return next == SYN_LOAD_ACQUIRE(&mb->tail);
}

static inline size_t syn_mailbox_pending(const SYN_Mailbox *mb)
{
    size_t h = SYN_LOAD_ACQUIRE(&mb->head);
    size_t t = mb->tail;
    return (h >= t) ? h - t : mb->capacity - t + h;
}

static inline size_t syn_mailbox_free(const SYN_Mailbox *mb)
{
    /* One slot is always reserved as sentinel */
    return mb->capacity - 1 - syn_mailbox_pending(mb);
}

static inline uint32_t syn_mailbox_overflows(const SYN_Mailbox *mb)
{
    return mb->overflow;
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


