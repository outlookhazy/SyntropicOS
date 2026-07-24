/**
 * @file syn_slab.c
 * @brief Multi-Class Slab Memory Allocator implementation.
 */

#include "syn_slab.h"
#include <string.h>

#define ALIGN4(x) (((x) + 3u) & ~3u)

SYN_Status syn_slab_init(SYN_SlabAllocator *slab,
                         void *backing_memory,
                         size_t total_size,
                         const size_t *block_sizes,
                         const size_t *blocks_per_class,
                         size_t num_classes)
{
    if (!slab || !backing_memory || !block_sizes || !blocks_per_class ||
        num_classes == 0 || num_classes > SYN_SLAB_MAX_CLASSES) {
        return SYN_INVALID_PARAM;
    }

    memset(slab, 0, sizeof(*slab));
    slab->backing_memory    = (uint8_t *)backing_memory;
    slab->total_memory_size = total_size;
    slab->num_classes       = num_classes;

    uint8_t *curr_ptr = slab->backing_memory;
    size_t remaining_size = total_size;

    for (size_t i = 0; i < num_classes; i++) {
        size_t b_size = ALIGN4(block_sizes[i]);
        if (b_size < sizeof(void *)) {
            b_size = sizeof(void *);
        }

        size_t count = blocks_per_class[i];
        size_t req_size = b_size * count;

        if (req_size > remaining_size) {
            return SYN_INVALID_PARAM; /* Insufficient backing memory */
        }

        SYN_SlabClass *sc = &slab->classes[i];
        sc->block_size   = b_size;
        sc->total_blocks = count;
        sc->free_count   = count;
        sc->start_ptr    = curr_ptr;
        sc->freelist     = NULL;

        /* Build embedded freelist */
        for (size_t j = 0; j < count; j++) {
            uint8_t *block = curr_ptr + (j * b_size);
            void *next = sc->freelist;
            sc->freelist = (void *)block;
            *(void **)block = next;
        }

        curr_ptr += req_size;
        remaining_size -= req_size;
    }

    return SYN_OK;
}

void *syn_slab_alloc(SYN_SlabAllocator *slab, size_t size)
{
    if (!slab || size == 0) return NULL;

    /* Find smallest fitting slab class */
    for (size_t i = 0; i < slab->num_classes; i++) {
        SYN_SlabClass *sc = &slab->classes[i];
        if (sc->block_size >= size) {
            if (sc->free_count > 0 && sc->freelist) {
                void *ptr = sc->freelist;
                sc->freelist = *(void **)ptr;
                sc->free_count--;
                return ptr;
            }
        }
    }

    return NULL; /* Out of memory / no fitting block free */
}

SYN_Status syn_slab_free(SYN_SlabAllocator *slab, void *ptr)
{
    if (!slab || !ptr) return SYN_INVALID_PARAM;

    const uint8_t *p = (const uint8_t *)ptr;

    /* Identify owning slab class */
    for (size_t i = 0; i < slab->num_classes; i++) {
        SYN_SlabClass *sc = &slab->classes[i];
        size_t slab_bytes = sc->block_size * sc->total_blocks;
        if (p >= sc->start_ptr && p < (sc->start_ptr + slab_bytes)) {
            /* Verify pointer alignment within slab */
            size_t offset = (size_t)(p - sc->start_ptr);
            if ((offset % sc->block_size) != 0) {
                return SYN_INVALID_PARAM; /* Misaligned pointer */
            }

            *(void **)ptr = sc->freelist;
            sc->freelist = ptr;
            sc->free_count++;
            return SYN_OK;
        }
    }

    return SYN_INVALID_PARAM; /* Pointer out of slab bounds */
}

SYN_Status syn_slab_get_stats(const SYN_SlabAllocator *slab, SYN_SlabStats *stats)
{
    if (!slab || !stats) return SYN_INVALID_PARAM;

    stats->total_bytes     = slab->total_memory_size;
    stats->allocated_bytes = 0;
    stats->total_allocs    = 0;

    for (size_t i = 0; i < slab->num_classes; i++) {
        const SYN_SlabClass *sc = &slab->classes[i];
        size_t used = sc->total_blocks - sc->free_count;
        stats->allocated_bytes += used * sc->block_size;
        stats->total_allocs    += used;
    }

    return SYN_OK;
}
