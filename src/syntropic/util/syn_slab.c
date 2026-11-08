/**
 * @file syn_slab.c
 * @brief Multi-Class Slab Memory Allocator implementation.
 */

#include "syn_slab.h"

#include <string.h>

#define SLAB_ALIGN_PTR(x) \
    (((size_t)(x) + (sizeof(void *) - 1U)) & ~(sizeof(void *) - 1U)) /**< Align to pointer size */

/**
 * @brief Check whether init parameters for syn_slab_init are valid.
 * @param slab             Slab allocator instance.
 * @param backing_memory   Backing memory buffer pointer.
 * @param block_sizes      Array of block sizes per class.
 * @param blocks_per_class Array of block counts per class.
 * @param num_classes      Number of slab size classes.
 * @return true if parameters are non-null and num_classes is within bounds.
 */
static bool slab_init_params_valid(const SYN_SlabAllocator *slab, const void *backing_memory,
                                   const size_t *block_sizes, const size_t *blocks_per_class,
                                   size_t num_classes)
{
    if (slab == NULL || backing_memory == NULL || block_sizes == NULL || blocks_per_class == NULL) {
        return false;
    }
    return (num_classes > 0U) && (num_classes <= SYN_SLAB_MAX_CLASSES);
}

/**
 * @brief Check whether @p ptr is already linked in @p sc->freelist.
 * @param sc  Slab class descriptor.
 * @param ptr Block pointer to search for in the freelist.
 * @return true if @p ptr is already present in the freelist.
 */
static bool slab_freelist_contains(const SYN_SlabClass *sc, const void *ptr)
{
    const void *curr = sc->freelist;
    while (curr != NULL) {
        if (curr == ptr) {
            return true;
        }
        void *next = NULL;
        memcpy(&next, curr, sizeof(void *));
        curr = next;
    }
    return false;
}

SYN_Status syn_slab_init(SYN_SlabAllocator *slab, void *backing_memory, size_t total_size,
                         const size_t *block_sizes, const size_t *blocks_per_class,
                         size_t num_classes)
{
    if (!slab_init_params_valid(slab, backing_memory, block_sizes, blocks_per_class, num_classes)) {
        return SYN_INVALID_PARAM;
    }

    uintptr_t raw_addr = (uintptr_t)backing_memory;
    uintptr_t align_mask = (uintptr_t)(sizeof(void *) - 1U);
    uintptr_t aligned_addr = (raw_addr + align_mask) & ~align_mask;
    size_t adjust = (size_t)(aligned_addr - raw_addr);
    if (total_size <= adjust) {
        return SYN_INVALID_PARAM;
    }

    memset(slab, 0, sizeof(*slab));
    slab->backing_memory = (uint8_t *)aligned_addr;
    slab->total_memory_size = total_size - adjust;

    uint8_t *curr_ptr = slab->backing_memory;
    size_t remaining_size = slab->total_memory_size;
    const size_t max_raw_block = SIZE_MAX - (sizeof(void *) - 1U);

    for (size_t i = 0U; i < num_classes; i++) {
        if ((block_sizes[i] == 0U) || (block_sizes[i] > max_raw_block)) {
            return SYN_INVALID_PARAM;
        }
        size_t b_size =
            (block_sizes[i] < sizeof(void *)) ? sizeof(void *) : SLAB_ALIGN_PTR(block_sizes[i]);
        size_t count = blocks_per_class[i];
        if ((count == 0U) || (b_size > (remaining_size / count))) {
            return SYN_INVALID_PARAM;
        }
        size_t req_size = b_size * count;

        SYN_SlabClass *sc = &slab->classes[i];
        sc->block_size = b_size;
        sc->total_blocks = count;
        sc->free_count = count;
        sc->start_ptr = curr_ptr;
        sc->freelist = NULL;
        sc->free_bits = (count >= 64U) ? UINT64_MAX : ((1ULL << count) - 1ULL);

        /* Build embedded freelist using memcpy to avoid strict-aliasing/alignment UB */
        for (size_t j = 0U; j < count; j++) {
            uint8_t *block = curr_ptr + (j * b_size);
            void *next = sc->freelist;
            memcpy(block, &next, sizeof(void *));
            sc->freelist = (void *)block;
        }

        curr_ptr += req_size;
        remaining_size -= req_size;
    }

    slab->num_classes = num_classes;
    return SYN_OK;
}

void *syn_slab_alloc(SYN_SlabAllocator *slab, size_t size)
{
    if (!slab || size == 0U) {
        return NULL;
    }

    /* Find smallest fitting slab class */
    for (size_t i = 0U; i < slab->num_classes; i++) {
        SYN_SlabClass *sc = &slab->classes[i];
        if (sc->block_size >= size && sc->free_count > 0U && sc->freelist != NULL) {
            void *ptr = sc->freelist;
            void *next = NULL;
            memcpy(&next, ptr, sizeof(void *));
            sc->freelist = next;
            sc->free_count--;
            size_t idx = (size_t)((uintptr_t)ptr - (uintptr_t)sc->start_ptr) / sc->block_size;
            if (idx < 64U) {
                sc->free_bits &= ~(1ULL << idx);
            }
            return ptr;
        }
    }

    return NULL; /* Out of memory / no fitting block free */
}

/**
 * @brief Validate and mark a slab block free, rejecting double-frees in O(1) for idx < 64.
 * @param sc  Slab class descriptor.
 * @param ptr Block pointer aligned within @p sc.
 * @param idx Block index within @p sc.
 * @return true if block was allocated and is now marked free, false on double-free.
 */
static bool slab_mark_block_free(SYN_SlabClass *sc, const void *ptr, size_t idx)
{
    if (sc->free_count >= sc->total_blocks) {
        return false;
    }
    if (idx < 64U) {
        if ((sc->free_bits & (1ULL << idx)) != 0ULL) {
            return false;
        }
        sc->free_bits |= (1ULL << idx);
        return true;
    }
    return !slab_freelist_contains(sc, ptr);
}

SYN_Status syn_slab_free(SYN_SlabAllocator *slab, void *ptr)
{
    if (!slab || !ptr) {
        return SYN_INVALID_PARAM;
    }

    uintptr_t p_addr = (uintptr_t)ptr;

    /* Identify owning slab class */
    for (size_t i = 0U; i < slab->num_classes; i++) {
        SYN_SlabClass *sc = &slab->classes[i];
        uintptr_t start_addr = (uintptr_t)sc->start_ptr;
        size_t slab_bytes = sc->block_size * sc->total_blocks;
        if (p_addr >= start_addr && (p_addr - start_addr) < slab_bytes) {
            size_t offset = (size_t)(p_addr - start_addr);
            if ((offset % sc->block_size) != 0U) {
                return SYN_INVALID_PARAM; /* Misaligned interior pointer */
            }
            size_t idx = offset / sc->block_size;
            if (!slab_mark_block_free(sc, ptr, idx)) {
                return SYN_INVALID_PARAM; /* Double-free detected */
            }

            void *next = sc->freelist;
            memcpy(ptr, &next, sizeof(void *));
            sc->freelist = ptr;
            sc->free_count++;
            return SYN_OK;
        }
    }

    return SYN_INVALID_PARAM; /* Pointer out of slab bounds */
}

SYN_Status syn_slab_get_stats(const SYN_SlabAllocator *slab, SYN_SlabStats *stats)
{
    if (!slab || !stats)
        return SYN_INVALID_PARAM;

    stats->total_bytes = slab->total_memory_size;
    stats->allocated_bytes = 0;
    stats->total_allocs = 0;

    for (size_t i = 0; i < slab->num_classes; i++) {
        const SYN_SlabClass *sc = &slab->classes[i];
        size_t used = sc->total_blocks - sc->free_count;
        stats->allocated_bytes += used * sc->block_size;
        stats->total_allocs += used;
    }

    return SYN_OK;
}
