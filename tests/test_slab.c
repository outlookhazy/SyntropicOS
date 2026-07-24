/**
 * @file test_slab.c
 * @brief Unit test suite for Multi-Class Slab Allocator (syn_slab).
 */

#include "unity/unity.h"
#include "syntropic/util/syn_slab.h"

void test_slab_alloc_free_stats(void)
{
    uint8_t backing[1024];
    SYN_SlabAllocator slab;

    size_t sizes[]  = {16, 64, 256};
    size_t counts[] = {8,  4,  2};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_init(&slab, backing, sizeof(backing), sizes, counts, 3));

    void *p16_a = syn_slab_alloc(&slab, 10);
    void *p16_b = syn_slab_alloc(&slab, 16);
    void *p64   = syn_slab_alloc(&slab, 50);
    void *p256  = syn_slab_alloc(&slab, 200);

    TEST_ASSERT_NOT_NULL(p16_a);
    TEST_ASSERT_NOT_NULL(p16_b);
    TEST_ASSERT_NOT_NULL(p64);
    TEST_ASSERT_NOT_NULL(p256);

    SYN_SlabStats stats;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_get_stats(&slab, &stats));
    TEST_ASSERT_EQUAL_UINT32(4, stats.total_allocs);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, p16_a));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, p16_b));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, p64));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, p256));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_get_stats(&slab, &stats));
    TEST_ASSERT_EQUAL_UINT32(0, stats.total_allocs);

    /* Test invalid free pointer */
    uint8_t dummy = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, &dummy));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_init(NULL, backing, sizeof(backing), sizes, counts, 3));
}

void run_slab_tests(void)
{
    RUN_TEST(test_slab_alloc_free_stats);
}
