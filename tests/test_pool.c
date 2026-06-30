/**
 * @file test_pool.c
 * @brief Unity tests for syn_pool — fixed-size block memory pool.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/util/syn_pool.h"

#include <stdint.h>
#include <string.h>

/* ── Tests ──────────────────────────────────────────────────────────────── */

/** Init and verify block count. */
static void test_pool_init(void)
{
    static uint8_t buf[SYN_POOL_BUF_SIZE(32, 4)];
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
    static uint8_t buf[256];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 1);

    /* Minimum block size should be at least sizeof(void*), aligned to 4 */
    TEST_ASSERT_TRUE(syn_pool_block_size(&pool) >= sizeof(void *));
    TEST_ASSERT_EQUAL(0, syn_pool_block_size(&pool) % 4);
}

/** Block size is rounded up to 4-byte alignment. */
static void test_pool_alignment(void)
{
    static uint8_t buf[256];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 13);

    /* 13 → rounded to 16 */
    TEST_ASSERT_EQUAL(16, syn_pool_block_size(&pool));
}

/** Alloc all blocks, then verify NULL on exhaustion. */
static void test_pool_alloc_exhaust(void)
{
    static uint8_t buf[SYN_POOL_BUF_SIZE(16, 3)];
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
    static uint8_t buf[SYN_POOL_BUF_SIZE(16, 2)];
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
    static uint8_t buf[SYN_POOL_BUF_SIZE(16, 4)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    void *b1 = syn_pool_alloc(&pool);
    void *b2 = syn_pool_alloc(&pool);
    void *b3 = syn_pool_alloc(&pool);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool));

    syn_pool_free(&pool, b3);
    syn_pool_free(&pool, b2);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool));  /* doesn't decrease */

    syn_pool_free(&pool, b1);
    TEST_ASSERT_EQUAL(3, syn_pool_high_watermark(&pool));  /* still 3 */
}

/** Free(NULL) is a safe no-op. */
static void test_pool_free_null(void)
{
    static uint8_t buf[SYN_POOL_BUF_SIZE(16, 2)];
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 16);

    syn_pool_free(&pool, NULL);  /* should not crash */
    TEST_ASSERT_EQUAL(0, syn_pool_used(&pool));
}

/** Verify allocated blocks return 4-byte aligned pointers. */
static void test_pool_pointer_alignment(void)
{
    /* Use a large-enough buffer with intentional padding */
    static uint8_t buf[SYN_POOL_BUF_SIZE(20, 4)] __attribute__((aligned(4)));
    SYN_Pool pool;
    syn_pool_init(&pool, buf, sizeof(buf), 20);

    size_t i;
    for (i = 0; i < 4; i++) {
        void *blk = syn_pool_alloc(&pool);
        TEST_ASSERT_NOT_NULL(blk);
        TEST_ASSERT_EQUAL(0, (uintptr_t)blk % 4);
    }
}

/** Write to allocated blocks to verify they don't overlap. */
static void test_pool_no_overlap(void)
{
    static uint8_t buf[SYN_POOL_BUF_SIZE(16, 3)];
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

/** SYN_POOL_BUF_SIZE macro produces correct values. */
static void test_pool_buf_size_macro(void)
{
    /* 32-byte blocks × 8 = 256 */
    TEST_ASSERT_EQUAL(256, SYN_POOL_BUF_SIZE(32, 8));

    /* 13 rounds to 16 → 16 × 4 = 64 */
    TEST_ASSERT_EQUAL(64, SYN_POOL_BUF_SIZE(13, 4));

    /* 4 stays 4 → 4 × 10 = 40 */
    TEST_ASSERT_EQUAL(40, SYN_POOL_BUF_SIZE(4, 10));
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
}
