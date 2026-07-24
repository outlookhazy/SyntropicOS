/**
 * @file syn_netbuf.h
 * @brief Zero-Copy Packet Buffer Pool.
 * @ingroup syn_util
 *
 * Provides a fixed-block memory pool manager for network/CAN packet buffers
 * supporting reference counting, zero-copy header push/pull, and slicing.
 */

#ifndef SYN_NETBUF_H
#define SYN_NETBUF_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_NETBUF_HEADROOM_DEFAULT 32U

/**
 * @brief Zero-Copy Packet Buffer Descriptor.
 */
typedef struct SYN_NetBuf {
    uint8_t  *head;          /**< Pointer to start of allocated raw memory */
    uint8_t  *data;          /**< Pointer to current payload start         */
    uint8_t  *tail;          /**< Pointer to end of valid payload          */
    uint8_t  *end;           /**< Pointer to end of raw memory buffer      */
    volatile uint16_t ref_count; /**< Reference counter                    */
    uint16_t  flags;         /**< Internal buffer flags                    */
} SYN_NetBuf;

/**
 * @brief Packet Buffer Pool Descriptor.
 */
typedef struct {
    SYN_NetBuf *bufs;        /**< Array of buffer descriptors              */
    uint8_t    *raw_mem;     /**< Array of raw data storage blocks         */
    size_t      buf_count;   /**< Total buffer count in pool               */
    size_t      buf_size;    /**< Raw storage block size in bytes          */
    size_t      free_count;  /**< Number of currently available buffers    */
} SYN_NetBufPool;

/**
 * @brief Initialize a packet buffer pool.
 * @param pool Pointer to pool descriptor.
 * @param bufs Array of buffer descriptors.
 * @param raw_mem Raw memory block storage.
 * @param buf_count Number of buffers in pool.
 * @param buf_size Size of each buffer block in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_netbuf_pool_init(SYN_NetBufPool *pool, SYN_NetBuf *bufs, uint8_t *raw_mem, size_t buf_count, size_t buf_size);

/**
 * @brief Allocate a packet buffer from the pool.
 * @param pool Pointer to pool descriptor.
 * @param headroom Initial headroom bytes reserved for network headers.
 * @return Allocated SYN_NetBuf pointer, or NULL if pool is exhausted.
 */
SYN_NetBuf *syn_netbuf_alloc(SYN_NetBufPool *pool, size_t headroom);

/**
 * @brief Increment the reference counter of a packet buffer.
 * @param buf Pointer to packet buffer.
 */
void syn_netbuf_ref(SYN_NetBuf *buf);

/**
 * @brief Decrement reference counter and free buffer if count reaches 0.
 * @param pool Pointer to pool descriptor.
 * @param buf Pointer to packet buffer.
 */
void syn_netbuf_free(SYN_NetBufPool *pool, SYN_NetBuf *buf);

/**
 * @brief Prepend header space before payload data.
 * @param buf Pointer to packet buffer.
 * @param len Bytes to prepend.
 * @return Pointer to new payload start, or NULL if headroom insufficient.
 */
uint8_t *syn_netbuf_push(SYN_NetBuf *buf, size_t len);

/**
 * @brief Strip header bytes from start of payload.
 * @param buf Pointer to packet buffer.
 * @param len Bytes to strip.
 * @return Pointer to new payload start, or NULL if payload shorter than len.
 */
uint8_t *syn_netbuf_pull(SYN_NetBuf *buf, size_t len);

/**
 * @brief Append payload bytes to end of buffer.
 * @param buf Pointer to packet buffer.
 * @param len Bytes to append.
 * @return Pointer to start of appended bytes, or NULL if tailroom insufficient.
 */
uint8_t *syn_netbuf_put(SYN_NetBuf *buf, size_t len);

/**
 * @brief Get payload length in bytes.
 * @param buf Pointer to packet buffer.
 * @return Payload length in bytes.
 */
size_t syn_netbuf_len(const SYN_NetBuf *buf);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NETBUF_H */
