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

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_POOL) || SYN_USE_POOL

#include "../util/syn_assert.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Helpers ────────────────────────────────────────────────────────────── */

/** @brief Round @p x up to the next multiple of 4. */
#define SYN_POOL_ALIGN4(x)  (((x) + 3u) & ~3u)

/**
 * @brief Compute the buffer size required for a pool.
 * @param block_size  Desired block size (will be aligned to 4).
 * @param count       Number of blocks.
 * @return Required buffer size in bytes.
 */
#define SYN_POOL_BUF_SIZE(block_size, count) \
    (SYN_POOL_ALIGN4(block_size) * (count))

/* ── Pool struct ────────────────────────────────────────────────────────── */

/**
 * @brief Fixed-size block memory pool.
 */
typedef struct {
    void    *freelist;      /**< Head of the freelist (NULL when empty)    */
    uint8_t *buf;           /**< Backing storage                          */
    size_t   block_size;    /**< Aligned block size in bytes              */
    size_t   block_count;   /**< Total number of blocks                   */
    size_t   used;          /**< Currently allocated blocks               */
    size_t   high_water;    /**< Peak allocation count                    */
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
static inline void syn_pool_init(SYN_Pool *pool, void *buf,
                                  size_t buf_size, size_t block_size)
{
    SYN_ASSERT(pool != NULL);
    SYN_ASSERT(buf != NULL);

    /* Enforce minimum block size (must hold a pointer for the freelist) */
    if (block_size < sizeof(void *)) {
        block_size = sizeof(void *);
    }

    /* Round up to 4-byte alignment */
    block_size = SYN_POOL_ALIGN4(block_size);

    size_t count = buf_size / block_size;
    SYN_ASSERT(count > 0);

    pool->buf         = (uint8_t *)buf;
    pool->block_size  = block_size;
    pool->block_count = count;
    pool->used        = 0;
    pool->high_water  = 0;

    /* Thread freelist through all blocks */
    pool->freelist = NULL;
    size_t i = count;
    while (i > 0) {
        i--;
        void **blk = (void **)(pool->buf + i * block_size);
        *blk = pool->freelist;
        pool->freelist = blk;
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
        return NULL;  /* exhausted */
    }

    /* Pop head of freelist */
    void *blk = pool->freelist;
    pool->freelist = *(void **)blk;

    pool->used++;
    if (pool->used > pool->high_water) {
        pool->high_water = pool->used;
    }

    return blk;
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
    if (block == NULL) return;

    /* Push onto freelist head */
    *(void **)block = pool->freelist;
    pool->freelist = block;

    SYN_ASSERT(pool->used > 0);
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
