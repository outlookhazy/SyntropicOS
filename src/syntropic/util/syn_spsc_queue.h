/**
 * @file syn_spsc_queue.h
 * @brief Lock-free Single-Producer Single-Consumer (SPSC) queue.
 * @ingroup syn_core
 *
 * Provides a lock-free, zero-allocation SPSC item queue designed for
 * high-performance ISR-to-Task, Task-to-Task, or Inter-Core (multicore)
 * message passing without spinlocks or disabling interrupts.
 */

#ifndef SYN_SPSC_QUEUE_H
#define SYN_SPSC_QUEUE_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPSC Queue Control Structure.
 */
typedef struct {
    uint8_t *buffer;       /**< Pointer to caller-provided element buffer */
    size_t   elem_size;    /**< Size of each element in bytes */
    size_t   capacity;     /**< Maximum element capacity */
    volatile size_t head;  /**< Write index (Producer modifies) */
    volatile size_t tail;  /**< Read index (Consumer modifies) */
} SYN_SPSC_Queue;

/**
 * @brief Initialize a Lock-Free SPSC Queue.
 * @param q Pointer to queue control structure.
 * @param elem_buf Pointer to caller-provided backing memory buffer.
 * @param elem_size Size of each element in bytes (must be > 0).
 * @param capacity Number of elements buffer can hold (must be > 0).
 * @return SYN_OK on success.
 */
SYN_Status syn_spsc_queue_init(SYN_SPSC_Queue *q, void *elem_buf, size_t elem_size, size_t capacity);

/**
 * @brief Push an item into the queue (Producer only).
 * @param q Queue context.
 * @param item Pointer to item to copy into queue.
 * @return SYN_OK on success, or SYN_BUSY if full.
 */
SYN_Status syn_spsc_queue_push(SYN_SPSC_Queue *q, const void *item);

/**
 * @brief Pop an item from the queue (Consumer only).
 * @param q Queue context.
 * @param out_item Pointer to memory receiving popped item.
 * @return SYN_OK on success, or SYN_ERROR if empty.
 */
SYN_Status syn_spsc_queue_pop(SYN_SPSC_Queue *q, void *out_item);

/**
 * @brief Check if the queue is empty.
 * @param q Queue context.
 * @return true if empty.
 */
bool syn_spsc_queue_is_empty(const SYN_SPSC_Queue *q);

/**
 * @brief Check if the queue is full.
 * @param q Queue context.
 * @return true if full.
 */
bool syn_spsc_queue_is_full(const SYN_SPSC_Queue *q);

/**
 * @brief Get current number of items stored in queue.
 * @param q Queue context.
 * @return Count of items in queue.
 */
size_t syn_spsc_queue_count(const SYN_SPSC_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPSC_QUEUE_H */
