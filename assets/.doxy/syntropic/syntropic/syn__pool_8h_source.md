

# File syn\_pool.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pool.h**](syn__pool_8h.md)

[Go to the documentation of this file](syn__pool_8h.md)


```C++

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

#define SYN_POOL_ALIGN4(x)  (((x) + 3u) & ~3u)

#define SYN_POOL_BUF_SIZE(block_size, count) \
    (SYN_POOL_ALIGN4(block_size) * (count))

/* ── Pool struct ────────────────────────────────────────────────────────── */

typedef struct {
    void    *freelist;      
    uint8_t *buf;           
    size_t   block_size;    
    size_t   block_count;   
    size_t   used;          
    size_t   high_water;    
} SYN_Pool;

/* ── API ────────────────────────────────────────────────────────────────── */

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


