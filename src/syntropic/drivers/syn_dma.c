/**
 * @file syn_dma.c
 * @brief Bare-Metal Safe DMA Transaction Engine implementation.
 */

#include "syn_dma.h"

#include "../common/syn_compiler.h"
#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_dma_init(SYN_DMA *dma, const SYN_DMA_Config *cfg)
{
    if ((dma == NULL) || (cfg == NULL)) {
        return SYN_INVALID_PARAM;
    }

    if ((cfg->data_size != SYN_DMA_SIZE_8BIT) && (cfg->data_size != SYN_DMA_SIZE_16BIT) &&
        (cfg->data_size != SYN_DMA_SIZE_32BIT)) {
        return SYN_INVALID_PARAM;
    }

    memset(dma, 0, sizeof(*dma));
    dma->cfg = *cfg;
    dma->state = SYN_DMA_STATE_IDLE;

    return SYN_OK;
}

SYN_Status syn_dma_start(SYN_DMA *dma, const void *src, void *dst, size_t count)
{
    if ((dma == NULL) || (src == NULL) || (dst == NULL) || (count == 0U)) {
        return SYN_INVALID_PARAM;
    }

    if ((dma->state == SYN_DMA_STATE_BUSY) || syn_port_dma_is_busy(dma->cfg.channel_id)) {
        return SYN_BUSY;
    }

    /* Safety Safeguard 1: Verify Memory Address Alignment */
    uintptr_t src_addr = (uintptr_t)src;
    uintptr_t dst_addr = (uintptr_t)dst;
    size_t align_mask = (size_t)dma->cfg.data_size - 1U;

    if (((src_addr & align_mask) != 0U) || ((dst_addr & align_mask) != 0U)) {
        return SYN_INVALID_PARAM;
    }

    size_t elem_bytes = (size_t)dma->cfg.data_size;
    size_t total_bytes = count * elem_bytes;
    size_t src_bytes = dma->cfg.src_inc ? total_bytes : elem_bytes;
    size_t dst_bytes = dma->cfg.dst_inc ? total_bytes : elem_bytes;

    /* Safety Safeguard 2: Clean source RAM D-Cache and Invalidate destination RAM D-Cache */
    if (dma->cfg.dir != SYN_DMA_DIR_PERIPH_TO_MEM) {
        syn_port_cache_clean(src, src_bytes);
    }
    if (dma->cfg.dir != SYN_DMA_DIR_MEM_TO_PERIPH) {
        syn_port_cache_invalidate(dst, dst_bytes);
    }

    /* Prepare port transfer request */
    SYN_PortDmaTransfer xfer = {.channel_id = dma->cfg.channel_id,
                                .dir = dma->cfg.dir,
                                .data_size = dma->cfg.data_size,
                                .src_inc = dma->cfg.src_inc,
                                .dst_inc = dma->cfg.dst_inc,
                                .src = src,
                                .dst = dst,
                                .count = count};

    dma->current_src = src;
    dma->current_dst = dst;
    dma->current_len = dst_bytes;
    dma->state = SYN_DMA_STATE_BUSY;

    /* Compiler memory barrier before hardware trigger */
    SYN_COMPILER_BARRIER();

    SYN_Status st = syn_port_dma_start(&xfer);
    if (st != SYN_OK) {
        dma->state = SYN_DMA_STATE_ERROR;
        dma->errors_cnt++;
        return st;
    }

    return SYN_OK;
}

SYN_Status syn_dma_stop(SYN_DMA *dma)
{
    if (dma == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_dma_stop(dma->cfg.channel_id);
    dma->state = SYN_DMA_STATE_IDLE;
    return SYN_OK;
}

SYN_DMA_State syn_dma_get_state(const SYN_DMA *dma)
{
    if (dma == NULL) {
        return SYN_DMA_STATE_ERROR;
    }
    return dma->state;
}

void syn_dma_isr_handler(SYN_DMA *dma, SYN_DMA_Event event)
{
    if (dma == NULL) {
        return;
    }

    if ((event & SYN_DMA_EVENT_HALF_COMPLETE) != 0) {
        dma->ring_saw_nonzero = true;
    }

    if ((event & SYN_DMA_EVENT_ERROR) != 0) {
        syn_port_dma_stop(dma->cfg.channel_id);
        dma->state = SYN_DMA_STATE_ERROR;
        dma->errors_cnt++;
    } else if ((event & SYN_DMA_EVENT_COMPLETE) != 0) {
        syn_port_dma_stop(dma->cfg.channel_id);
        /* Invalidate destination RAM cache after hardware write completion */
        if (dma->cfg.dir != SYN_DMA_DIR_MEM_TO_PERIPH) {
            syn_port_cache_invalidate(dma->current_dst, dma->current_len);
        }
        dma->state = SYN_DMA_STATE_COMPLETE;
        dma->transfers_cnt++;
        dma->ring_saw_nonzero = true;
    }

    /* Fire user completion/event callback */
    if (dma->cfg.callback != NULL) {
        dma->cfg.callback(dma, event, dma->cfg.user_ctx);
    }
}

/* ── Circular DMA Ring Buffer Implementation ─────────────────────────────── */

SYN_Status syn_dma_ringbuf_init(SYN_DMA_RingBuf *r, SYN_DMA *dma, uint8_t *buf, size_t capacity)
{
    if ((r == NULL) || (dma == NULL) || (buf == NULL) || (capacity == 0U)) {
        return SYN_INVALID_PARAM;
    }

    memset(r, 0, sizeof(*r));
    r->dma = dma;
    r->buf = buf;
    r->capacity = capacity;
    r->tail = 0U;
    r->wrap_drained = false;
    dma->ring_saw_nonzero = false;

    return SYN_OK;
}

SYN_Status syn_dma_ringbuf_start(SYN_DMA_RingBuf *r, const void *periph_src)
{
    if ((r == NULL) || (periph_src == NULL)) {
        return SYN_INVALID_PARAM;
    }

    r->tail = 0U;
    r->wrap_drained = false;
    r->dma->ring_saw_nonzero = false;
    return syn_dma_start(r->dma, periph_src, r->buf, r->capacity);
}

/**
 * @brief Compute unread bytes from a single hardware counter snapshot.
 * @param r         Ring buffer instance.
 * @param rem_out   Output hardware remaining counter snapshot.
 * @return Number of unread bytes in the ring buffer.
 */
static size_t dma_ringbuf_avail_snapshot(const SYN_DMA_RingBuf *r, uint32_t *rem_out)
{
    uint32_t remaining = syn_port_dma_get_counter(r->dma->cfg.channel_id);
    *rem_out = remaining;
    if (remaining > r->capacity) {
        return 0U;
    }

    if ((remaining > 0U) && (remaining < r->capacity)) {
        r->dma->ring_saw_nonzero = true;
    }

    size_t tail = r->tail;
    if (remaining == 0U) {
        if (r->wrap_drained && !r->dma->ring_saw_nonzero) {
            return 0U;
        }
        return r->capacity - tail;
    }

    size_t head = r->capacity - (size_t)remaining;
    if (head >= tail) {
        return head - tail;
    }
    return r->capacity - (tail - head);
}

size_t syn_dma_ringbuf_bytes_available(const SYN_DMA_RingBuf *r)
{
    if ((r == NULL) || (r->dma == NULL) || (r->capacity == 0U)) {
        return 0U;
    }

    uint32_t remaining = 0U;
    return dma_ringbuf_avail_snapshot(r, &remaining);
}

size_t syn_dma_ringbuf_read(SYN_DMA_RingBuf *r, uint8_t *dest, size_t len)
{
    if ((r == NULL) || (r->dma == NULL) || (dest == NULL) || (len == 0U) || (r->capacity == 0U)) {
        return 0U;
    }

    uint32_t remaining = 0U;
    size_t avail = dma_ringbuf_avail_snapshot(r, &remaining);
    if (avail == 0U) {
        return 0U;
    }

    size_t to_read = (len < avail) ? len : avail;
    size_t tail = r->tail;
    size_t read_bytes = 0U;

    while (read_bytes < to_read) {
        size_t chunk = r->capacity - tail;
        if (chunk > (to_read - read_bytes)) {
            chunk = to_read - read_bytes;
        }

        if (r->dma->cfg.dir != SYN_DMA_DIR_MEM_TO_PERIPH) {
            syn_port_cache_invalidate(r->buf + tail, chunk);
        }
        memcpy(dest + read_bytes, r->buf + tail, chunk);
        tail = (tail + chunk) % r->capacity;
        read_bytes += chunk;
    }

    r->tail = tail;
    if ((remaining == 0U) && (tail == 0U)) {
        r->wrap_drained = true;
        r->dma->ring_saw_nonzero = false;
    } else {
        r->wrap_drained = false;
    }
    return read_bytes;
}
