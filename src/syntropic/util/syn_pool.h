/**
 * @file syn_pool.h
 * @brief Fixed-size block memory pool — zero-heap, header-only.
 *
 * A pool allocator backed by a caller-provided byte array. All blocks
 * are the same size. Allocation and free are O(1) via a freelist
 * embedded in the free blocks themselves.
 *
 * Block size is automatically rounded up to 4-byte alignment.
 * Minimum block size is sizeof(void*) (4 bytes on 32-bit targets).
 *
 * @par Usage
 * @code
 *   // Allocate a pool of 8 blocks, each 64 bytes
 *   static uint8_t backing[SYN_POOL_BUF_SIZE(64, 8)];
 *   static SYN_Pool pool;
 *
 *   syn_pool_init(&pool, backing, sizeof(backing), 64);
 *
 *   void *blk = syn_pool_alloc(&pool);
 *   // ... use blk ...
 *   syn_pool_free(&pool, blk);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_POOL_H
#define SYN_POOL_H

#include "../common/syn_defs.h"

#if !defined(SYN_USE_POOL) || SYN_USE_POOL

#include "../util/syn_assert.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Helpers ────────────────────────────────────────────────────────────── */

/** @brief Round @p x up to the next multiple of 4. */
#define SYN_POOL_ALIGN4(x) (((size_t)(x) + 3U) & ~(size_t)3U)

/** @brief Round @p x up to pointer alignment (`sizeof(void *)`). */
#define SYN_POOL_ALIGN_PTR(x) (((size_t)(x) + (sizeof(void *) - 1U)) & ~(sizeof(void *) - 1U))

/**
 * @brief Compute the buffer size required for a pool (includes pointer-alignment slack).
 * @param block_size  Desired block size (clamped to `sizeof(void *)` and pointer-aligned).
 * @param count       Number of blocks.
 * @return Required buffer size in bytes.
 */
#define SYN_POOL_BUF_SIZE(block_size, count)                                       \
    (((((size_t)(block_size) < sizeof(void *)) ? sizeof(void *)                    \
                                               : SYN_POOL_ALIGN_PTR(block_size)) * \
      (size_t)(count)) +                                                           \
     (sizeof(void *) - 1U))

/* ── Pool struct ────────────────────────────────────────────────────────── */

/**
 * @brief Fixed-size block memory pool.
 */
typedef struct {
    void *freelist;     /**< Head of the freelist (NULL when empty)    */
    uint8_t *buf;       /**< Backing storage                          */
    size_t block_size;  /**< Aligned block size in bytes              */
    size_t block_count; /**< Total number of blocks                   */
    size_t used;        /**< Currently allocated blocks               */
    size_t high_water;  /**< Peak allocation count                    */
    uint64_t free_bits; /**< O(1) free-block bitmap for indices 0..63 */
} SYN_Pool;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a memory pool.
 *
 * Partitions the backing buffer into fixed-size blocks and threads
 * a freelist through all of them.
 *
 * @param pool        Pool instance (caller-owned).
 * @param buf         Backing storage (caller-owned, must be 4-byte aligned).
 * @param buf_size    Size of backing storage in bytes.
 * @param block_size  Desired block size. Rounded up to 4-byte alignment.
 *                    Must be >= sizeof(void*).
 */
static inline void syn_pool_init(SYN_Pool *pool, void *buf, size_t buf_size, size_t block_size)
{
    SYN_ASSERT(pool != NULL);
    SYN_ASSERT(buf != NULL);

    /* Enforce minimum block size and pointer-size alignment */
    if (block_size < sizeof(void *)) {
        block_size = sizeof(void *);
    }
    block_size = SYN_POOL_ALIGN_PTR(block_size);

    /* Align starting buffer address up to pointer boundary if unaligned */
    uintptr_t raw_addr = (uintptr_t)buf;
    uintptr_t align_mask = (uintptr_t)(sizeof(void *) - 1U);
    uintptr_t aligned_addr = (raw_addr + align_mask) & ~align_mask;
    size_t adjust = (size_t)(aligned_addr - raw_addr);
    SYN_ASSERT(buf_size > adjust);
    buf_size -= adjust;

    size_t count = buf_size / block_size;
    SYN_ASSERT(count > 0U);

    pool->buf = (uint8_t *)aligned_addr;
    pool->block_size = block_size;
    pool->block_count = count;
    pool->used = 0U;
    pool->high_water = 0U;
    pool->free_bits = (count >= 64U) ? UINT64_MAX : ((1ULL << count) - 1ULL);

    /* Thread freelist through all blocks */
    pool->freelist = NULL;
    size_t i = count;
    while (i > 0U) {
        i--;
        uint8_t *blk = pool->buf + (i * block_size);
        void *next = pool->freelist;
        memcpy(blk, &next, sizeof(void *));
        pool->freelist = (void *)blk;
    }
}

/**
 * @brief Allocate a block from the pool.
 * @param pool  Pool instance.
 * @return Pointer to allocated block, or NULL if pool is exhausted.
 */
static inline void *syn_pool_alloc(SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);

    if (pool->freelist == NULL) {
        return NULL; /* exhausted */
    }

    /* Pop head of freelist */
    void *blk = pool->freelist;
    void *next = NULL;
    memcpy(&next, blk, sizeof(void *));
    pool->freelist = next;

    size_t idx = (size_t)((uintptr_t)blk - (uintptr_t)pool->buf) / pool->block_size;
    if (idx < 64U) {
        pool->free_bits &= ~(1ULL << idx);
    }

    pool->used++;
    if (pool->used > pool->high_water) {
        pool->high_water = pool->used;
    }

    return blk;
}

/**
 * @brief Fallback freelist membership check for block indices >= 64.
 * @param pool  Pool instance.
 * @param block Candidate block pointer.
 * @return true if @p block is already present in @p pool->freelist.
 */
static inline bool syn_pool_freelist_contains(const SYN_Pool *pool, const void *block)
{
    const void *curr = pool->freelist;
    while (curr != NULL) {
        if (curr == block) {
            return true;
        }
        void *next_free = NULL;
        memcpy(&next_free, curr, sizeof(void *));
        curr = next_free;
    }
    return false;
}

/**
 * @brief Return a block to the pool.
 *
 * The block must have been previously allocated from this pool.
 *
 * @param pool  Pool instance.
 * @param block Pointer to block to free (NULL is safely ignored).
 */
static inline void syn_pool_free(SYN_Pool *pool, void *block)
{
    SYN_ASSERT(pool != NULL);
    if (block == NULL || pool->used == 0U) {
        return;
    }

    /* Validate pointer bounds and stride alignment */
    uintptr_t blk_addr = (uintptr_t)block;
    uintptr_t base_addr = (uintptr_t)pool->buf;
    size_t total_bytes = pool->block_count * pool->block_size;
    if (blk_addr < base_addr || (blk_addr - base_addr) >= total_bytes) {
        return;
    }
    if (((blk_addr - base_addr) % pool->block_size) != 0U) {
        return;
    }

    /* O(1) bitmap double-free check for indices < 64; freelist fallback for larger pools */
    size_t idx = (size_t)(blk_addr - base_addr) / pool->block_size;
    if (idx < 64U) {
        if ((pool->free_bits & (1ULL << idx)) != 0ULL) {
            return;
        }
        pool->free_bits |= (1ULL << idx);
    } else if (syn_pool_freelist_contains(pool, block)) {
        return;
    }

    /* Push onto freelist head */
    void *next = pool->freelist;
    memcpy(block, &next, sizeof(void *));
    pool->freelist = block;
    pool->used--;
}

/**
 * @brief Get the number of available (free) blocks.
 * @param pool  Pool instance.
 * @return Free block count.
 */
static inline size_t syn_pool_available(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->block_count - pool->used;
}

/**
 * @brief Get the number of currently allocated blocks.
 * @param pool  Pool instance.
 * @return Used block count.
 */
static inline size_t syn_pool_used(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->used;
}

/**
 * @brief Get the peak allocation count (high watermark).
 * @param pool  Pool instance.
 * @return Maximum simultaneous allocations observed.
 */
static inline size_t syn_pool_high_watermark(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->high_water;
}

/**
 * @brief Get the aligned block size.
 * @param pool  Pool instance.
 * @return Block size in bytes (4-byte aligned).
 */
static inline size_t syn_pool_block_size(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->block_size;
}

/**
 * @brief Get the total block count.
 * @param pool  Pool instance.
 * @return Total number of blocks.
 */
static inline size_t syn_pool_block_count(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->block_count;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_POOL */

#endif /* SYN_POOL_H */
