/**
 * @file syn_dma.h
 * @brief Bare-Metal Safe DMA Transaction Engine.
 * @ingroup syn_drivers
 *
 * Provides safe DMA transaction submission with memory alignment verification,
 * data cache maintenance, re-entrancy locking, and ISR event callbacks.
 */

#ifndef SYN_DMA_H
#define SYN_DMA_H

#include "../common/syn_defs.h"
#include "../port/syn_port_dma.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DMA channel execution state */
typedef enum {
    SYN_DMA_STATE_IDLE = 0,     /**< Channel ready for transfer */
    SYN_DMA_STATE_BUSY = 1,     /**< Transfer actively running */
    SYN_DMA_STATE_COMPLETE = 2, /**< Last transfer finished */
    SYN_DMA_STATE_ERROR = 3     /**< Transfer error occurred */
} SYN_DMA_State;

/** @brief DMA ISR event flags */
typedef enum {
    SYN_DMA_EVENT_HALF_COMPLETE = 0x01, /**< Half transfer completed */
    SYN_DMA_EVENT_COMPLETE = 0x02,      /**< Full transfer completed */
    SYN_DMA_EVENT_ERROR = 0x04          /**< Hardware bus or transfer error */
} SYN_DMA_Event;

struct SYN_DMA;

/** @brief Callback for DMA transfer events */
typedef void (*SYN_DMA_Callback)(struct SYN_DMA *dma, SYN_DMA_Event event, void *user_ctx);

/**
 * @brief Configuration parameters for a DMA channel.
 */
typedef struct {
    uint8_t channel_id;        /**< Hardware DMA channel index */
    SYN_DMA_Direction dir;     /**< Transfer direction */
    SYN_DMA_Size data_size;    /**< Data width (1, 2, or 4 bytes) */
    bool src_inc;              /**< Auto-increment source address */
    bool dst_inc;              /**< Auto-increment destination address */
    SYN_DMA_Callback callback; /**< Optional ISR event callback */
    void *user_ctx;            /**< User context passed to callback */
} SYN_DMA_Config;

/**
 * @brief DMA Channel instance object.
 */
typedef struct SYN_DMA {
    SYN_DMA_Config cfg;      /**< Channel static configuration */
    SYN_DMA_State state;     /**< Current channel execution state */
    const void *current_src; /**< Active transfer source address */
    void *current_dst;       /**< Active transfer destination address */
    size_t current_len;      /**< Active transfer length in bytes */
    uint32_t transfers_cnt;  /**< Total successful transfers count */
    uint32_t errors_cnt;     /**< Total transfer errors count */
    bool ring_saw_nonzero;   /**< Circular ringbuf observed hardware progress */
} SYN_DMA;

/**
 * @brief Initialize a DMA channel object.
 * @param dma Pointer to DMA instance.
 * @param cfg Configuration structure.
 * @return SYN_OK on success, or SYN_INVALID_PARAM if dma/cfg is NULL.
 */
SYN_Status syn_dma_init(SYN_DMA *dma, const SYN_DMA_Config *cfg);

/**
 * @brief Submit and trigger a DMA transfer.
 *
 * Verifies address alignment against data element size (8/16/32-bit), cleans
 * D-cache for source region, invalidates D-cache for destination region, and
 * launches hardware transfer.
 *
 * @param dma Pointer to initialized DMA channel instance.
 * @param src Source buffer address.
 * @param dst Destination buffer address.
 * @param count Number of elements to transfer (must be > 0).
 * @return SYN_OK on success.
 * @return SYN_INVALID_PARAM if buffer address is unaligned to data_size or parameters invalid.
 * @return SYN_BUSY if channel is actively transferring.
 */
SYN_Status syn_dma_start(SYN_DMA *dma, const void *src, void *dst, size_t count);

/**
 * @brief Stop/abort an active DMA transfer.
 * @param dma Pointer to DMA channel.
 * @return SYN_OK on success.
 */
SYN_Status syn_dma_stop(SYN_DMA *dma);

/**
 * @brief Get current state of DMA channel.
 * @param dma Pointer to DMA channel.
 * @return Current SYN_DMA_State.
 */
SYN_DMA_State syn_dma_get_state(const SYN_DMA *dma);

/**
 * @brief Hardware ISR event handler entry point.
 *
 * Call this function from the MCU hardware DMA interrupt handler when a channel
 * fires a half-transfer, complete, or error interrupt.
 *
 * @param dma Pointer to DMA channel.
 * @param event Event bitmask (SYN_DMA_EVENT_*).
 */
void syn_dma_isr_handler(SYN_DMA *dma, SYN_DMA_Event event);

/* ── Circular DMA Ring Buffer Stream Receiver ────────────────────────────── */

/**
 * @brief Continuous circular DMA reception stream controller.
 */
typedef struct {
    SYN_DMA *dma;         /**< Underlying DMA channel                         */
    uint8_t *buf;         /**< Caller-allocated circular reception buffer     */
    size_t capacity;      /**< Total capacity of buffer in bytes              */
    volatile size_t tail; /**< Application read pointer index                 */
    bool wrap_drained;    /**< True when full capacity drained at remaining=0 */
} SYN_DMA_RingBuf;

/**
 * @brief Initialize a circular DMA ring buffer receiver.
 * @param r        Ring buffer instance.
 * @param dma      Initialized DMA channel instance.
 * @param buf      Reception buffer (aligned to dma data size).
 * @param capacity Buffer capacity in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_dma_ringbuf_init(SYN_DMA_RingBuf *r, SYN_DMA *dma, uint8_t *buf, size_t capacity);

/**
 * @brief Start continuous circular DMA stream reception.
 * @param r          Ring buffer instance.
 * @param periph_src Peripheral data register source address.
 * @return SYN_OK on success.
 */
SYN_Status syn_dma_ringbuf_start(SYN_DMA_RingBuf *r, const void *periph_src);

/**
 * @brief Get total bytes available to read from the circular DMA stream.
 * @param r Ring buffer instance.
 * @return Number of unread bytes.
 */
size_t syn_dma_ringbuf_bytes_available(const SYN_DMA_RingBuf *r);

/**
 * @brief Read bytes from the circular DMA reception buffer into destination.
 * @param r     Ring buffer instance.
 * @param dest  Output data buffer.
 * @param len   Maximum bytes to read.
 * @return Actual bytes read.
 */
size_t syn_dma_ringbuf_read(SYN_DMA_RingBuf *r, uint8_t *dest, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DMA_H */
