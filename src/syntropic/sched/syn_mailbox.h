/**
 * @file syn_mailbox.h
 * @brief Typed message queue — fixed-size SPSC ring for inter-task IPC.
 *
 * Passes fixed-size messages between producer (ISR or task) and consumer
 * (task). Lock-free single-producer single-consumer design.
 *
 * When SYN_USE_MULTICORE is enabled, acquire/release memory barriers
 * are inserted at every index access, making the mailbox safe for
 * cross-core communication (core A posts, core B receives). When
 * multicore is disabled (the default), these compile to plain volatile
 * reads and writes — zero overhead.
 *
 * @par Correctness model (multicore)
 *
 * The mailbox has exactly two indices:
 * - `head` — written ONLY by the producer
 * - `tail` — written ONLY by the consumer
 *
 * **Producer `syn_mailbox_post()`:**
 * 1. Read `tail` (LOAD_ACQUIRE) — observe consumer's latest progress
 * 2. If full, return false
 * 3. memcpy message data into `buf[head * msg_size]`
 * 4. Write `head = next` (STORE_RELEASE) — data committed before index
 *
 * **Consumer `syn_mailbox_receive()`:**
 * 1. Read `head` (LOAD_ACQUIRE) — observe producer's latest index
 * 2. If empty, return false
 * 3. memcpy message from `buf[tail * msg_size]` — safe: data visible
 * 4. Write `tail = next` (STORE_RELEASE) — slot freed for producer
 *
 * @par Usage
 * @code
 *   typedef struct { uint16_t id; int32_t value; } SensorMsg;
 *
 *   // Static allocation
 *   SYN_MAILBOX_DEFINE(sensor_mbox, SensorMsg, 8);
 *
 *   // Producer (ISR or other core):
 *   SensorMsg msg = { .id = 1, .value = 2345 };
 *   syn_mailbox_post(&sensor_mbox, &msg);
 *
 *   // Consumer (main loop or other core):
 *   SensorMsg rx;
 *   while (syn_mailbox_receive(&sensor_mbox, &rx)) {
 *       process(rx.id, rx.value);
 *   }
 * @endcode
 * @ingroup syn_sched
 */

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

/** @brief Typed message queue — fixed-size SPSC ring buffer. */
typedef struct {
    uint8_t        *buf;          /**< Message storage (flat byte array)   */
    size_t          msg_size;     /**< Size of each message in bytes       */
    size_t          capacity;     /**< Max messages (slots)                */
    volatile size_t head;         /**< Write index (producer)              */
    volatile size_t tail;         /**< Read index (consumer)               */
    uint32_t        overflow;     /**< Messages dropped (queue full)       */
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    bool            notify;       /**< Auto-notify consumer core on post?  */
#endif
} SYN_Mailbox;

/**
 * @brief Static mailbox definition macro.
 *
 * Creates a mailbox instance with embedded storage.
 * @param name      Variable name.
 * @param type      Message struct type.
 * @param count     Max number of messages.
 */
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

/**
 * @brief Initialize a mailbox at runtime.
 * @param mb        Mailbox instance.
 * @param buf       Message storage buffer.
 * @param msg_size  Size of one message in bytes.
 * @param capacity  Maximum number of messages.
 */
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
/**
 * @brief Enable cross-core notification on post.
 *
 * When enabled, each successful post() calls syn_port_ipc_notify()
 * to wake the consumer core from WFE / low-power idle.
 *
 * @param mb      Mailbox.
 * @param enable  true to enable IPC notifications.
 */
static inline void syn_mailbox_set_notify(SYN_Mailbox *mb, bool enable)
{
    mb->notify = enable;
}
#endif

/**
 * @brief Post a message. ISR-safe (single producer).
 *
 * Uses STORE_RELEASE on the head update to ensure message data
 * is committed before the consumer can see the new index.
 *
 * @param mb   Mailbox.
 * @param msg  Pointer to message data (msg_size bytes copied).
 * @return true if posted, false if full.
 */
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

/**
 * @brief Receive a message. Single consumer.
 *
 * Uses LOAD_ACQUIRE on the head read to ensure all message data
 * written by the producer is visible. Uses STORE_RELEASE on the
 * tail update to ensure the producer sees the freed slot.
 *
 * @param mb   Mailbox.
 * @param msg  Pointer to receive buffer (msg_size bytes copied out).
 * @return true if a message was received, false if empty.
 */
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

/**
 * @brief Peek at the next message without consuming it.
 *
 * @param mb  Mailbox.
 * @return Pointer to the message, or NULL if empty.
 *         Valid until the next receive().
 */
static inline const void *syn_mailbox_peek(const SYN_Mailbox *mb)
{
    if (SYN_LOAD_ACQUIRE(&mb->head) == mb->tail) return NULL;
    return &mb->buf[mb->tail * mb->msg_size];
}

/**
 * @brief Check if the mailbox is empty.
 * @param mb  Mailbox.
 * @return true if empty.
 */
static inline bool syn_mailbox_empty(const SYN_Mailbox *mb)
{
    return SYN_LOAD_ACQUIRE(&mb->head) == mb->tail;
}

/**
 * @brief Check if the mailbox is full.
 * @param mb  Mailbox.
 * @return true if full.
 */
static inline bool syn_mailbox_full(const SYN_Mailbox *mb)
{
    size_t next = mb->head + 1;
    if (next >= mb->capacity) next = 0;
    return next == SYN_LOAD_ACQUIRE(&mb->tail);
}

/**
 * @brief Number of messages currently in the mailbox.
 * @param mb  Mailbox.
 * @return Message count.
 */
static inline size_t syn_mailbox_pending(const SYN_Mailbox *mb)
{
    size_t h = SYN_LOAD_ACQUIRE(&mb->head);
    size_t t = mb->tail;
    return (h >= t) ? h - t : mb->capacity - t + h;
}

/**
 * @brief Number of free slots.
 * @param mb  Mailbox.
 * @return Free slot count.
 */
static inline size_t syn_mailbox_free(const SYN_Mailbox *mb)
{
    /* One slot is always reserved as sentinel */
    return mb->capacity - 1 - syn_mailbox_pending(mb);
}

/**
 * @brief Get overflow count.
 * @param mb  Mailbox.
 * @return Number of dropped messages.
 */
static inline uint32_t syn_mailbox_overflows(const SYN_Mailbox *mb)
{
    return mb->overflow;
}

/**
 * @brief Flush all messages.
 * @param mb  Mailbox.
 */
static inline void syn_mailbox_flush(SYN_Mailbox *mb)
{
    size_t head = SYN_LOAD_ACQUIRE(&mb->head);
    SYN_STORE_RELEASE(&mb->tail, head);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_MAILBOX_H */
