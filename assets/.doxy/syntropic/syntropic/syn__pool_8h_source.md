

# File syn\_pool.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pool.h**](syn__pool_8h.md)

[Go to the documentation of this file](syn__pool_8h.md)


```C++


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

#define SYN_POOL_ALIGN4(x) (((size_t)(x) + 3U) & ~(size_t)3U)

#define SYN_POOL_ALIGN_PTR(x) (((size_t)(x) + (sizeof(void *) - 1U)) & ~(sizeof(void *) - 1U))

#define SYN_POOL_BUF_SIZE(block_size, count)                                       \
    (((((size_t)(block_size) < sizeof(void *)) ? sizeof(void *)                    \
                                               : SYN_POOL_ALIGN_PTR(block_size)) * \
      (size_t)(count)) +                                                           \
     (sizeof(void *) - 1U))

/* ── Pool struct ────────────────────────────────────────────────────────── */

typedef struct {
    void *freelist;     
    uint8_t *buf;       
    size_t block_size;  
    size_t block_count; 
    size_t used;        
    size_t high_water;  
    uint64_t free_bits; 
} SYN_Pool;

/* ── API ────────────────────────────────────────────────────────────────── */

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

static inline size_t syn_pool_available(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->block_count - pool->used;
}

static inline size_t syn_pool_used(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->used;
}

static inline size_t syn_pool_high_watermark(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->high_water;
}

static inline size_t syn_pool_block_size(const SYN_Pool *pool)
{
    SYN_ASSERT(pool != NULL);
    return pool->block_size;
}

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
```


