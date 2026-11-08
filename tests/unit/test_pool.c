/**
 * @file test_pool.c
 * @brief Unity tests for syn_pool — fixed-size block memory pool.
 */

#include "mocks/mock_port.h"
#include "syntropic/util/syn_pool.h"
#include "unity/unity.h"

#include <stdint.h>
#include <string.h>

/* ── Tests ──────────────────────────────────────────────────────────────── */

/** Init and verify block count. */
static void test_pool_init(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(32, 4) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 32);

    TEST_ASSERT_EQUAL(4, syn_pool_block_count(&pool));
    TEST_ASSERT_EQUAL(4, syn_pool_available(&pool));
    TEST_ASSERT_EQUAL(0, syn_pool_used(&pool));
    TEST_ASSERT_EQUAL(0, syn_pool_high_watermark(&pool));
    TEST_ASSERT_EQUAL(32, syn_pool_block_size(&pool));
}

/** Block size smaller than sizeof(void*) is rounded up. */
static void test_pool_min_block_size(void)
{
    static uintptr_t buf[256 / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 1);

    /* Minimum block size should be at least sizeof(void*), pointer-aligned */
    TEST_ASSERT_TRUE(syn_pool_block_size(&pool) >= sizeof(void *));
    TEST_ASSERT_EQUAL(0, syn_pool_block_size(&pool) % sizeof(void *));
}

/** Block size is rounded up to pointer alignment. */
static void test_pool_alignment(void)
{
    static uintptr_t buf[256 / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 13);

    /* 13 → rounded to 16 */
    TEST_ASSERT_EQUAL(16, syn_pool_block_size(&pool));
}

/** Alloc all blocks, then verify NULL on exhaustion. */
static void test_pool_alloc_exhaust(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(16, 3) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    void *b1 = syn_pool_alloc(&pool);
    void *b2 = syn_pool_alloc(&pool);
    void *b3 = syn_pool_alloc(&pool);

    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_NOT_NULL(b2);
    TEST_ASSERT_NOT_NULL(b3);
    TEST_ASSERT_EQUAL(3, syn_pool_used(&pool));
    TEST_ASSERT_EQUAL(0, syn_pool_available(&pool));

    /* Pool exhausted */
    void *b4 = syn_pool_alloc(&pool);
    TEST_ASSERT_NULL(b4);
}

/** Free and re-alloc verifies blocks are reused. */
static void test_pool_free_reuse(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(16, 2) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    void *b1 = syn_pool_alloc(&pool);
    void *b2 = syn_pool_alloc(&pool);
    TEST_ASSERT_EQUAL(0, syn_pool_available(&pool));

    syn_pool_free(&pool, b1);
    TEST_ASSERT_EQUAL(1, syn_pool_available(&pool));
    TEST_ASSERT_EQUAL(1, syn_pool_used(&pool));

    /* Re-alloc should succeed and return b1's address */
    void *b3 = syn_pool_alloc(&pool);
    TEST_ASSERT_NOT_NULL(b3);
    TEST_ASSERT_EQUAL_PTR(b1, b3);

    syn_pool_free(&pool, b2);
    syn_pool_free(&pool, b3);
    TEST_ASSERT_EQUAL(2, syn_pool_available(&pool));
    TEST_ASSERT_EQUAL(0, syn_pool_used(&pool));
}

/** High watermark tracks peak allocation. */
static void test_pool_high_watermark(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(16, 4) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    void *b1 = syn_pool_alloc(&pool);
    void *b2 = syn_pool_alloc(&pool);
    void *b3 = syn_pool_alloc(&pool);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool));

    syn_pool_free(&pool, b3);
    syn_pool_free(&pool, b2);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool)); /* doesn't decrease */

    syn_pool_free(&pool, b1);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool)); /* still 3 */
}

/** Free(NULL) is a safe no-op. */
static void test_pool_free_null(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(16, 2) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    syn_pool_free(&pool, NULL); /* should not crash */
    TEST_ASSERT_EQUAL(0, syn_pool_used(&pool));
}

/** Verify allocated blocks return pointer-aligned pointers. */
static void test_pool_pointer_alignment(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(20, 4) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 20);

    size_t i;
    for (i = 0; i < 4; i++) {
        void *blk = syn_pool_alloc(&pool);
        TEST_ASSERT_NOT_NULL(blk);
        TEST_ASSERT_EQUAL(0, (uintptr_t)blk % sizeof(void *));
    }
}

/** Write to allocated blocks to verify they don't overlap. */
static void test_pool_no_overlap(void)
{
    static uintptr_t buf[SYN_POOL_BUF_SIZE(16, 3) / sizeof(uintptr_t)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    uint8_t *b1 = (uint8_t *)syn_pool_alloc(&pool);
    uint8_t *b2 = (uint8_t *)syn_pool_alloc(&pool);
    uint8_t *b3 = (uint8_t *)syn_pool_alloc(&pool);

    /* Fill each block with a distinct pattern */
    memset(b1, 0xAA, 16);
    memset(b2, 0xBB, 16);
    memset(b3, 0xCC, 16);

    /* Verify patterns didn't bleed */
    size_t i;
    for (i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xAA, b1[i]);
        TEST_ASSERT_EQUAL_HEX8(0xBB, b2[i]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, b3[i]);
    }
}

/** SYN_POOL_BUF_SIZE macro produces correct values including alignment slack. */
static void test_pool_buf_size_macro(void)
{
    const size_t slack = sizeof(void *) - 1U;

    /* 32-byte blocks × 8 + alignment slack */
    TEST_ASSERT_EQUAL(256U + slack, SYN_POOL_BUF_SIZE(32, 8));

    /* 13 rounds to 16 → 16 × 4 + alignment slack */
    TEST_ASSERT_EQUAL(64U + slack, SYN_POOL_BUF_SIZE(13, 4));

    /* 4 clamps up to sizeof(void *) → sizeof(void *) × 10 + alignment slack */
    TEST_ASSERT_EQUAL((sizeof(void *) * 10U) + slack, SYN_POOL_BUF_SIZE(4, 10));
}

/** Regression R5-K03: SYN_POOL_BUF_SIZE(4, 1), odd sizes, unaligned buf, double-free. */
static void test_r5_pool_alignment_and_double_free(void)
{
    /* 1. SYN_POOL_BUF_SIZE(4, 1) must hold 1 block on both 32-bit and 64-bit */
    uintptr_t tiny_buf[SYN_POOL_BUF_SIZE(4, 1) / sizeof(uintptr_t)];
    SYN_Pool tiny_pool;
    syn_pool_init(&tiny_pool, tiny_buf, sizeof(tiny_buf), 4U);
    TEST_ASSERT_EQUAL(1U, syn_pool_block_count(&tiny_pool));
    void *tb = syn_pool_alloc(&tiny_pool);
    TEST_ASSERT_NOT_NULL(tb);
    TEST_ASSERT_NULL(syn_pool_alloc(&tiny_pool));
    syn_pool_free(&tiny_pool, tb);
    /* Free when used == 0 is ignored */
    syn_pool_free(&tiny_pool, tb);
    TEST_ASSERT_EQUAL(0U, syn_pool_used(&tiny_pool));

    /* 2. Odd block sizes (12, 20) and unaligned backing buffer (raw + 1) */
    uintptr_t raw_words[(SYN_POOL_BUF_SIZE(12, 4) / sizeof(uintptr_t)) + 2U];
    uint8_t *unaligned = ((uint8_t *)raw_words) + 1U;
    SYN_Pool pool12;
    syn_pool_init(&pool12, unaligned, sizeof(raw_words) - 1U, 12U);
    TEST_ASSERT_EQUAL(0U, syn_pool_block_size(&pool12) % sizeof(void *));
    TEST_ASSERT_TRUE(syn_pool_block_count(&pool12) >= 4U);

    void *b1 = syn_pool_alloc(&pool12);
    void *b2 = syn_pool_alloc(&pool12);
    void *b3 = syn_pool_alloc(&pool12);
    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_NOT_NULL(b2);
    TEST_ASSERT_NOT_NULL(b3);
    TEST_ASSERT_EQUAL(0U, (uintptr_t)b1 % sizeof(void *));
    TEST_ASSERT_EQUAL(0U, (uintptr_t)b2 % sizeof(void *));
    TEST_ASSERT_EQUAL(0U, (uintptr_t)b3 % sizeof(void *));

    /* 3. Out-of-bounds and misaligned interior pointers ignored while blocks allocated */
    syn_pool_free(&pool12, (uint8_t *)raw_words);
    syn_pool_free(&pool12, ((uint8_t *)raw_words) + sizeof(raw_words));
    syn_pool_free(&pool12, ((uint8_t *)b1) + 1U);
    TEST_ASSERT_EQUAL(3U, syn_pool_used(&pool12));

    /* 4. Double-free when 1+ blocks remain allocated (head and non-head of freelist) */
    syn_pool_free(&pool12, b1);
    syn_pool_free(&pool12, b2);
    size_t used_after = syn_pool_used(&pool12);
    TEST_ASSERT_EQUAL(1U, used_after);

    /* Free b2 again (freelist head) and b1 again (second node in freelist) */
    syn_pool_free(&pool12, b2);
    syn_pool_free(&pool12, b1);
    TEST_ASSERT_EQUAL(1U, syn_pool_used(&pool12));

    /* Verify freelist has no self-cycle: can still exhaust cleanly */
    size_t avail = syn_pool_available(&pool12);
    for (size_t i = 0U; i < avail; i++) {
        TEST_ASSERT_NOT_NULL(syn_pool_alloc(&pool12));
    }
    TEST_ASSERT_NULL(syn_pool_alloc(&pool12));

    /* 5. Unaligned uint8_t[SYN_POOL_BUF_SIZE(64, 8)] at every byte offset yields exact 8 blocks */
    uint8_t raw_u8[SYN_POOL_BUF_SIZE(64, 8) + sizeof(void *)];
    for (size_t off = 0U; off < sizeof(void *); off++) {
        SYN_Pool upool;
        syn_pool_init(&upool, &raw_u8[off], SYN_POOL_BUF_SIZE(64, 8), 64U);
        TEST_ASSERT_EQUAL(8U, syn_pool_block_count(&upool));
    }

    /* 6. Large > 64-block pool exercises idx >= 64 double-free fallback path */
    uintptr_t big_buf[SYN_POOL_BUF_SIZE(8, 66) / sizeof(uintptr_t)];
    SYN_Pool big_pool;
    syn_pool_init(&big_pool, big_buf, sizeof(big_buf), 8U);
    TEST_ASSERT_EQUAL(66U, syn_pool_block_count(&big_pool));
    void *blks[66];
    for (size_t i = 0U; i < 66U; i++) {
        blks[i] = syn_pool_alloc(&big_pool);
        TEST_ASSERT_NOT_NULL(blks[i]);
    }
    /* Free idx 64 and idx 65, keep idx 0..63 allocated, then double-free idx 64 (non-head) */
    syn_pool_free(&big_pool, blks[64]);
    syn_pool_free(&big_pool, blks[65]);
    TEST_ASSERT_EQUAL(64U, syn_pool_used(&big_pool));
    syn_pool_free(&big_pool, blks[64]);
    TEST_ASSERT_EQUAL(64U, syn_pool_used(&big_pool));
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_pool_tests(void)
{
    RUN_TEST(test_pool_init);
    RUN_TEST(test_pool_min_block_size);
    RUN_TEST(test_pool_alignment);
    RUN_TEST(test_pool_alloc_exhaust);
    RUN_TEST(test_pool_free_reuse);
    RUN_TEST(test_pool_high_watermark);
    RUN_TEST(test_pool_free_null);
    RUN_TEST(test_pool_pointer_alignment);
    RUN_TEST(test_pool_no_overlap);
    RUN_TEST(test_pool_buf_size_macro);
    RUN_TEST(test_r5_pool_alignment_and_double_free);
}
