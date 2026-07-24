/**
 * @file syn_port_dma.h
 * @brief Hardware port abstraction interface for DMA controller.
 * @ingroup syn_port
 */

#ifndef SYN_PORT_DMA_H
#define SYN_PORT_DMA_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DMA transfer direction */
typedef enum {
    SYN_DMA_DIR_MEM_TO_MEM = 0, /**< Memory to Memory transfer */
    SYN_DMA_DIR_MEM_TO_PERIPH = 1, /**< Memory to Peripheral transfer */
    SYN_DMA_DIR_PERIPH_TO_MEM = 2  /**< Peripheral to Memory transfer */
} SYN_DMA_Direction;

/** @brief DMA transfer data element size */
typedef enum {
    SYN_DMA_SIZE_8BIT  = 1, /**< 8-bit byte transfers */
    SYN_DMA_SIZE_16BIT = 2, /**< 16-bit halfword transfers */
    SYN_DMA_SIZE_32BIT = 4  /**< 32-bit word transfers */
} SYN_DMA_Size;

/**
 * @brief Hardware DMA transfer hardware request parameters.
 */
typedef struct {
    uint8_t           channel_id;   /**< Hardware DMA channel index */
    SYN_DMA_Direction dir;          /**< Transfer direction */
    SYN_DMA_Size      data_size;    /**< Data element size (1, 2, 4) */
    bool              src_inc;      /**< Auto-increment source address */
    bool              dst_inc;      /**< Auto-increment destination address */
    const void       *src;          /**< Source address */
    void             *dst;          /**< Destination address */
    size_t            count;        /**< Transfer count (elements) */
} SYN_PortDmaTransfer;

/**
 * @brief Trigger hardware DMA channel start.
 * @param xfer Transfer configuration.
 * @return SYN_OK on success, error code otherwise.
 */
SYN_Status syn_port_dma_start(const SYN_PortDmaTransfer *xfer);

/**
 * @brief Abort an active hardware DMA channel.
 * @param channel_id Hardware DMA channel index.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_dma_stop(uint8_t channel_id);

/**
 * @brief Check if hardware DMA channel is active.
 * @param channel_id Hardware DMA channel index.
 * @return true if channel is actively transferring.
 */
bool syn_port_dma_is_busy(uint8_t channel_id);

/**
 * @brief Flush/clean data cache for source RAM region before DMA read.
 * @param addr Start address.
 * @param len Byte length.
 */
void syn_port_cache_clean(const void *addr, size_t len);

/**
 * @brief Invalidate data cache for destination RAM region after DMA write.
 * @param addr Start address.
 * @param len Byte length.
 */
void syn_port_cache_invalidate(void *addr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_DMA_H */
