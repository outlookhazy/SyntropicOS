/**
 * @file syn_pingpong.h
 * @brief Ping-pong (double) buffer — header-only, zero-copy DMA pattern.
 *
 * One buffer is "active" (being filled by DMA/ISR), the other is
 * "ready" (available for processing). Call swap() when the active
 * buffer is full to exchange them atomically.
 *
 * @par Usage
 * @code
 *   uint8_t pool_a[256], pool_b[256];
 *   SYN_PingPong pp;
 *   syn_pingpong_init(&pp, pool_a, pool_b, 256);
 *
 *   // DMA writes into active buffer:
 *   uint8_t *dma_buf = syn_pingpong_active(&pp);
 *   start_dma(dma_buf, 256);
 *
 *   // DMA complete ISR:
 *   syn_pingpong_swap(&pp);
 *
 *   // Main loop processes the ready buffer:
 *   if (syn_pingpong_ready(&pp)) {
 *       uint8_t *data = syn_pingpong_ready_buf(&pp);
 *       process(data, 256);
 *       syn_pingpong_consume(&pp);
 *   }
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_PINGPONG_H
#define SYN_PINGPONG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Ping-pong (double) buffer — zero-copy DMA pattern. */
typedef struct {
    uint8_t  *buf[2];       /**< Two buffer pointers                      */
    size_t    size;          /**< Size of each buffer                      */
    uint8_t   active_idx;   /**< Index (0 or 1) of the active buffer      */
    volatile bool data_ready; /**< True when ready buffer has data         */
} SYN_PingPong;

/**
 * @brief Initialize with two user-provided buffers of equal size.
 * @param pp     Ping-pong instance.
 * @param buf_a  First buffer.
 * @param buf_b  Second buffer.
 * @param size   Size of each buffer in bytes.
 */
static inline void syn_pingpong_init(SYN_PingPong *pp,
                                      uint8_t *buf_a,
                                      uint8_t *buf_b,
                                      size_t size)
{
    pp->buf[0]     = buf_a;
    pp->buf[1]     = buf_b;
    pp->size       = size;
    pp->active_idx = 0;
    pp->data_ready = false;
}

/**
 * @brief Get pointer to the active (being-filled) buffer.
 * @param pp  Ping-pong instance.
 * @return Active buffer pointer.
 */
static inline uint8_t *syn_pingpong_active(const SYN_PingPong *pp)
{
    return pp->buf[pp->active_idx];
}

/**
 * @brief Get pointer to the ready (available-for-reading) buffer.
 * @param pp  Ping-pong instance.
 * @return Ready buffer pointer.
 */
static inline uint8_t *syn_pingpong_ready_buf(const SYN_PingPong *pp)
{
    return pp->buf[1 - pp->active_idx];
}

/**
 * @brief Check if the ready buffer has unprocessed data.
 * @param pp  Ping-pong instance.
 * @return true if data is ready.
 */
static inline bool syn_pingpong_ready(const SYN_PingPong *pp)
{
    return pp->data_ready;
}

/**
 * @brief Swap buffers. Call from ISR/DMA complete.
 *
 * The previously-active buffer becomes ready for processing.
 * The previously-ready buffer becomes the new active buffer.
 *
 * @param pp  Ping-pong instance.
 */
static inline void syn_pingpong_swap(SYN_PingPong *pp)
{
    pp->active_idx = (uint8_t)(1 - pp->active_idx);
    pp->data_ready = true;
}

/**
 * @brief Mark the ready buffer as consumed. Call after processing.
 * @param pp  Ping-pong instance.
 */
static inline void syn_pingpong_consume(SYN_PingPong *pp)
{
    pp->data_ready = false;
}

/**
 * @brief Get the buffer size.
 * @param pp  Ping-pong instance.
 * @return Buffer size in bytes.
 */
static inline size_t syn_pingpong_size(const SYN_PingPong *pp)
{
    return pp->size;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PINGPONG_H */
