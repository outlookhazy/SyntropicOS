

# File syn\_dma.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dma.c**](syn__dma_8c.md)

[Go to the documentation of this file](syn__dma_8c.md)


```C++

#include "syn_dma.h"

#include "../common/syn_compiler.h"
#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_dma_init(SYN_DMA *dma, const SYN_DMA_Config *cfg)
{
    if (!dma || !cfg) {
        return SYN_INVALID_PARAM;
    }

    if (cfg->data_size != SYN_DMA_SIZE_8BIT && cfg->data_size != SYN_DMA_SIZE_16BIT &&
        cfg->data_size != SYN_DMA_SIZE_32BIT) {
        return SYN_INVALID_PARAM;
    }

    memset(dma, 0, sizeof(*dma));
    dma->cfg = *cfg;
    dma->state = SYN_DMA_STATE_IDLE;

    return SYN_OK;
}

SYN_Status syn_dma_start(SYN_DMA *dma, const void *src, void *dst, size_t count)
{
    if (!dma || !src || !dst || count == 0) {
        return SYN_INVALID_PARAM;
    }

    if (dma->state == SYN_DMA_STATE_BUSY || syn_port_dma_is_busy(dma->cfg.channel_id)) {
        return SYN_BUSY;
    }

    /* Safety Safeguard 1: Verify Memory Address Alignment */
    uintptr_t src_addr = (uintptr_t)src;
    uintptr_t dst_addr = (uintptr_t)dst;
    size_t align_mask = (size_t)dma->cfg.data_size - 1U;

    if ((src_addr & align_mask) != 0 || (dst_addr & align_mask) != 0) {
        return SYN_INVALID_PARAM;
    }

    size_t total_bytes = count * (size_t)dma->cfg.data_size;

    /* Safety Safeguard 2: Clean source D-Cache and Invalidate destination D-Cache */
    syn_port_cache_clean(src, total_bytes);
    syn_port_cache_invalidate(dst, total_bytes);

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
    dma->current_len = total_bytes;
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
    if (!dma)
        return SYN_INVALID_PARAM;

    syn_port_dma_stop(dma->cfg.channel_id);
    dma->state = SYN_DMA_STATE_IDLE;
    return SYN_OK;
}

SYN_DMA_State syn_dma_get_state(const SYN_DMA *dma)
{
    if (!dma)
        return SYN_DMA_STATE_ERROR;
    return dma->state;
}

void syn_dma_isr_handler(SYN_DMA *dma, SYN_DMA_Event event)
{
    if (!dma)
        return;

    if (event & SYN_DMA_EVENT_ERROR) {
        syn_port_dma_stop(dma->cfg.channel_id);
        dma->state = SYN_DMA_STATE_ERROR;
        dma->errors_cnt++;
    } else if (event & SYN_DMA_EVENT_COMPLETE) {
        syn_port_dma_stop(dma->cfg.channel_id);
        /* Clean/Invalidate cache after hardware write completion */
        syn_port_cache_invalidate(dma->current_dst, dma->current_len);
        dma->state = SYN_DMA_STATE_COMPLETE;
        dma->transfers_cnt++;
    }

    /* Fire user completion/event callback */
    if (dma->cfg.callback) {
        dma->cfg.callback(dma, event, dma->cfg.user_ctx);
    }
}

/* ── Circular DMA Ring Buffer Implementation ─────────────────────────────── */

SYN_Status syn_dma_ringbuf_init(SYN_DMA_RingBuf *r, SYN_DMA *dma, uint8_t *buf, size_t capacity)
{
    if (!r || !dma || !buf || capacity == 0) {
        return SYN_INVALID_PARAM;
    }

    memset(r, 0, sizeof(*r));
    r->dma = dma;
    r->buf = buf;
    r->capacity = capacity;
    r->tail = 0;

    return SYN_OK;
}

SYN_Status syn_dma_ringbuf_start(SYN_DMA_RingBuf *r, const void *periph_src)
{
    if (!r || !periph_src) {
        return SYN_INVALID_PARAM;
    }

    r->tail = 0;
    return syn_dma_start(r->dma, periph_src, r->buf, r->capacity);
}

size_t syn_dma_ringbuf_bytes_available(const SYN_DMA_RingBuf *r)
{
    if (!r || !r->dma || r->capacity == 0) {
        return 0;
    }

    uint32_t remaining = syn_port_dma_get_counter(r->dma->cfg.channel_id);
    size_t head = (remaining <= r->capacity) ? (r->capacity - remaining) : 0;
    size_t tail = r->tail;

    if (head >= tail) {
        return head - tail;
    } else {
        return r->capacity - (tail - head);
    }
}

size_t syn_dma_ringbuf_read(SYN_DMA_RingBuf *r, uint8_t *dest, size_t len)
{
    if (!r || !dest || len == 0 || r->capacity == 0) {
        return 0;
    }

    size_t avail = syn_dma_ringbuf_bytes_available(r);
    if (avail == 0) {
        return 0;
    }

    size_t to_read = (len < avail) ? len : avail;
    size_t tail = r->tail;
    size_t read_bytes = 0;

    while (read_bytes < to_read) {
        size_t chunk = r->capacity - tail;
        if (chunk > to_read - read_bytes) {
            chunk = to_read - read_bytes;
        }

        memcpy(dest + read_bytes, r->buf + tail, chunk);
        tail = (tail + chunk) % r->capacity;
        read_bytes += chunk;
    }

    r->tail = tail;
    return read_bytes;
}
```


