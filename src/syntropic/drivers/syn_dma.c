/**
 * @file syn_dma.c
 * @brief Bare-Metal Safe DMA Transaction Engine implementation.
 */

#include "syn_dma.h"
#include "../util/syn_assert.h"
#include "../common/syn_compiler.h"
#include <string.h>

SYN_Status syn_dma_init(SYN_DMA *dma, const SYN_DMA_Config *cfg)
{
    if (!dma || !cfg) {
        return SYN_INVALID_PARAM;
    }

    if (cfg->data_size != SYN_DMA_SIZE_8BIT &&
        cfg->data_size != SYN_DMA_SIZE_16BIT &&
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
    SYN_PortDmaTransfer xfer = {
        .channel_id = dma->cfg.channel_id,
        .dir        = dma->cfg.dir,
        .data_size  = dma->cfg.data_size,
        .src_inc    = dma->cfg.src_inc,
        .dst_inc    = dma->cfg.dst_inc,
        .src        = src,
        .dst        = dst,
        .count      = count
    };

    dma->current_src = src;
    dma->current_dst = dst;
    dma->current_len = total_bytes;
    dma->state       = SYN_DMA_STATE_BUSY;

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
    if (!dma) return SYN_INVALID_PARAM;

    syn_port_dma_stop(dma->cfg.channel_id);
    dma->state = SYN_DMA_STATE_IDLE;
    return SYN_OK;
}

SYN_DMA_State syn_dma_get_state(const SYN_DMA *dma)
{
    if (!dma) return SYN_DMA_STATE_ERROR;
    return dma->state;
}

void syn_dma_isr_handler(SYN_DMA *dma, SYN_DMA_Event event)
{
    if (!dma) return;

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
