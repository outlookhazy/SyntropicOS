/**
 * @file syn_slab.h
 * @brief Multi-Class Slab Memory Allocator.
 * @ingroup syn_core
 *
 * Partitions a single caller-provided static memory pool into multiple
 * fixed-size slab block classes (e.g. 16B, 64B, 256B). Allocations and
 * frees execute in deterministic O(1) time without heap fragmentation.
 */

#ifndef SYN_SLAB_H
#define SYN_SLAB_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum slab size classes per allocator instance. */
#define SYN_SLAB_MAX_CLASSES 8U

/**
 * @brief Single Slab Size Class Descriptor.
 */
typedef struct {
    size_t   block_size;   /**< Aligned block size for this class */
    size_t   total_blocks; /**< Total blocks in this slab class */
    size_t   free_count;   /**< Current free blocks */
    uint8_t *start_ptr;    /**< Start pointer of slab region */
    void    *freelist;     /**< Embedded freelist head */
} SYN_SlabClass;

/**
 * @brief Multi-Class Slab Allocator.
 */
typedef struct {
    uint8_t      *backing_memory;                /**< Backing memory pointer */
    size_t        total_memory_size;             /**< Total backing size */
    SYN_SlabClass classes[SYN_SLAB_MAX_CLASSES]; /**< Slab classes */
    size_t        num_classes;                   /**< Number of configured classes */
} SYN_SlabAllocator;

/**
 * @brief Slab Allocator Statistics.
 */
typedef struct {
    size_t total_bytes;     /**< Total managed memory in bytes */
    size_t allocated_bytes; /**< Currently allocated bytes */
    size_t total_allocs;    /**< Total active allocations count */
} SYN_SlabStats;

/**
 * @brief Initialize a Multi-Class Slab Allocator.
 * @param slab Pointer to allocator context.
 * @param backing_memory Pointer to static backing array.
 * @param total_size Size of backing memory in bytes.
 * @param block_sizes Array of desired block sizes (e.g. {16, 64, 256}).
 * @param blocks_per_class Array of block counts for each class.
 * @param num_classes Number of classes (1 to SYN_SLAB_MAX_CLASSES).
 * @return SYN_OK on success.
 */
SYN_Status syn_slab_init(SYN_SlabAllocator *slab,
                         void *backing_memory,
                         size_t total_size,
                         const size_t *block_sizes,
                         const size_t *blocks_per_class,
                         size_t num_classes);

/**
 * @brief Allocate a block from the optimal slab class.
 * @param slab Allocator context.
 * @param size Requested allocation size in bytes.
 * @return Pointer to allocated block, or NULL if unavailable.
 */
void *syn_slab_alloc(SYN_SlabAllocator *slab, size_t size);

/**
 * @brief Free a previously allocated slab block.
 * @param slab Allocator context.
 * @param ptr Pointer to block to free.
 * @return SYN_OK on success, or SYN_INVALID_PARAM.
 */
SYN_Status syn_slab_free(SYN_SlabAllocator *slab, void *ptr);

/**
 * @brief Retrieve current allocator usage statistics.
 * @param slab Allocator context.
 * @param stats Output statistics structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_slab_get_stats(const SYN_SlabAllocator *slab, SYN_SlabStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SLAB_H */
