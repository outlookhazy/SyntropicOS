/**
 * @file test_netbuf.c
 * @brief Unit test suite for Zero-Copy Packet Buffer Pool (syn_netbuf).
 */

#include "unity/unity.h"
#include "syntropic/util/syn_netbuf.h"
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

void run_netbuf_tests(void)
{
    RUN_TEST(test_netbuf_alloc_push_pull_free);
    RUN_TEST(test_netbuf_pool_exhaustion_and_invalid);
}
