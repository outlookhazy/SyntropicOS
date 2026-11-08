/**
 * @file test_netbuf.c
 * @brief Unit test suite for Zero-Copy Packet Buffer Pool (syn_netbuf).
 */

#include "syntropic/net/syn_netbuf.h"
#include "unity/unity.h"

#include <string.h>

void test_netbuf_alloc_push_pull_free(void)
{
    SYN_NetBufPool pool;
    SYN_NetBuf bufs[4];
    uint8_t raw_mem[4 * 128];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netbuf_pool_init(&pool, bufs, raw_mem, 4, 128));
    TEST_ASSERT_EQUAL_UINT32(4, pool.free_count);

    SYN_NetBuf *nb = syn_netbuf_alloc(&pool, 32);
    TEST_ASSERT_NOT_NULL(nb);
    TEST_ASSERT_EQUAL_UINT32(3, pool.free_count);
    TEST_ASSERT_EQUAL_UINT32(0, syn_netbuf_len(nb));

    /* Put payload data */
    uint8_t *p_data = syn_netbuf_put(nb, 16);
    TEST_ASSERT_NOT_NULL(p_data);
    memcpy(p_data, "1234567890ABCDEF", 16);
    TEST_ASSERT_EQUAL_UINT32(16, syn_netbuf_len(nb));

    /* Push header */
    uint8_t *p_hdr = syn_netbuf_push(nb, 8);
    TEST_ASSERT_NOT_NULL(p_hdr);
    memcpy(p_hdr, "HEADER!!", 8);
    TEST_ASSERT_EQUAL_UINT32(24, syn_netbuf_len(nb));

    /* Pull header */
    uint8_t *p_strip = syn_netbuf_pull(nb, 8);
    TEST_ASSERT_NOT_NULL(p_strip);
    TEST_ASSERT_EQUAL_UINT32(16, syn_netbuf_len(nb));

    /* Ref count */
    syn_netbuf_ref(nb);
    syn_netbuf_free(&pool, nb);
    TEST_ASSERT_EQUAL_UINT32(3, pool.free_count); /* Still retained by second ref */

    syn_netbuf_free(&pool, nb);
    TEST_ASSERT_EQUAL_UINT32(4, pool.free_count); /* Now returned to pool */
}

void test_netbuf_pool_exhaustion_and_invalid(void)
{
    SYN_NetBufPool pool;
    SYN_NetBuf bufs[2];
    uint8_t raw_mem[2 * 64];

    syn_netbuf_pool_init(&pool, bufs, raw_mem, 2, 64);
    SYN_NetBuf *b1 = syn_netbuf_alloc(&pool, 16);
    SYN_NetBuf *b2 = syn_netbuf_alloc(&pool, 16);
    SYN_NetBuf *b3 = syn_netbuf_alloc(&pool, 16);

    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_NOT_NULL(b2);
    TEST_ASSERT_NULL(b3); /* Pool exhausted */

    TEST_ASSERT_NULL(syn_netbuf_push(b1, 999)); /* Insufficient headroom */
    TEST_ASSERT_NULL(syn_netbuf_pull(b1, 999)); /* Insufficient payload */
    TEST_ASSERT_NULL(syn_netbuf_put(b1, 999));  /* Insufficient tailroom */

    syn_netbuf_free(&pool, b1);
    syn_netbuf_free(&pool, b2);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_netbuf_pool_init(NULL, bufs, raw_mem, 2, 64));
    TEST_ASSERT_NULL(syn_netbuf_alloc(NULL, 16));
    TEST_ASSERT_EQUAL_UINT32(0, syn_netbuf_len(NULL));
}

void test_netbuf_auto_pool_free_and_headroom_overflow(void)
{
    SYN_NetBufPool pool;
    SYN_NetBuf bufs[2];
    uint8_t raw_mem[2 * 64];

    syn_netbuf_pool_init(&pool, bufs, raw_mem, 2, 64);

    /* Headroom larger than buffer size defaults headroom to 0 */
    SYN_NetBuf *b1 = syn_netbuf_alloc(&pool, 200);
    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_EQUAL_PTR(b1->head, b1->data);

    /* Free buffer using NULL pool (retrieves pool pointer from buf->pool) */
    syn_netbuf_free(NULL, b1);
    TEST_ASSERT_EQUAL_UINT32(2, pool.free_count);
}

void test_netbuf_null_edge_cases(void)
{
    syn_netbuf_ref(NULL);
    syn_netbuf_free(NULL, NULL);

    SYN_NetBuf buf_no_pool;
    memset(&buf_no_pool, 0, sizeof(buf_no_pool));
    buf_no_pool.ref_count = 1;
    buf_no_pool.pool = NULL;
    syn_netbuf_free(NULL, &buf_no_pool);

    TEST_ASSERT_NULL(syn_netbuf_push(NULL, 10));
    TEST_ASSERT_NULL(syn_netbuf_pull(NULL, 10));
    TEST_ASSERT_NULL(syn_netbuf_put(NULL, 10));
}

static void test_tier_a_netbuf_regressions(void)
{
    SYN_NetBufPool pool_a;
    SYN_NetBufPool pool_b;
    SYN_NetBuf bufs_a[1];
    SYN_NetBuf bufs_b[1];
    uint8_t mem_a[64];
    uint8_t mem_b[64];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netbuf_pool_init(&pool_a, bufs_a, mem_a, 1U, 64U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_netbuf_pool_init(&pool_b, bufs_b, mem_b, 1U, 64U));

    /* 1. Alloc clears stale flags across free/re-alloc cycles */
    bufs_a[0].flags = 0xBEEFU;
    SYN_NetBuf *b = syn_netbuf_alloc(&pool_a, 16U);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_UINT16(0U, b->flags);
    b->flags = 0xCAFEU;

    /* 2. Foreign pool rejected in syn_netbuf_free */
    syn_netbuf_free(&pool_b, b);
    TEST_ASSERT_EQUAL_UINT16(1U, b->ref_count);
    TEST_ASSERT_EQUAL_size_t(0U, pool_a.free_count);
    TEST_ASSERT_EQUAL_size_t(1U, pool_b.free_count);

    /* 3. ref_count saturates at UINT16_MAX instead of wrapping to 0 */
    b->ref_count = UINT16_MAX;
    syn_netbuf_ref(b);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, b->ref_count);

    /* Restore ref_count = 1 and free back to owning pool_a */
    b->ref_count = 1U;
    syn_netbuf_free(&pool_a, b);
    TEST_ASSERT_EQUAL_size_t(1U, pool_a.free_count);
    TEST_ASSERT_EQUAL_UINT16(0U, b->flags);

    /* Freeing already-free buffer (ref_count == 0) is a safe no-op */
    syn_netbuf_free(&pool_a, b);
    TEST_ASSERT_EQUAL_size_t(1U, pool_a.free_count);
}

void run_netbuf_tests(void)
{
    RUN_TEST(test_netbuf_alloc_push_pull_free);
    RUN_TEST(test_netbuf_pool_exhaustion_and_invalid);
    RUN_TEST(test_netbuf_auto_pool_free_and_headroom_overflow);
    RUN_TEST(test_netbuf_null_edge_cases);
    RUN_TEST(test_tier_a_netbuf_regressions);
}
