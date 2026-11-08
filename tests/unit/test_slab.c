/**
 * @file test_slab.c
 * @brief Unit test suite for Multi-Class Slab Allocator (syn_slab).
 */

#include "syntropic/util/syn_slab.h"
#include "unity/unity.h"

void test_slab_alloc_free_stats(void)
{
    uint8_t backing[1024];
    SYN_SlabAllocator slab;

    size_t sizes[] = {16, 64, 256};
    size_t counts[] = {8, 4, 2};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_init(&slab, backing, sizeof(backing), sizes, counts, 3));

    void *p16_a = syn_slab_alloc(&slab, 10);
    void *p16_b = syn_slab_alloc(&slab, 16);
    void *p64 = syn_slab_alloc(&slab, 50);
    void *p256 = syn_slab_alloc(&slab, 200);

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
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(NULL, backing, sizeof(backing), sizes, counts, 3));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_get_stats(NULL, &stats));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_get_stats(&slab, NULL));
}

void test_slab_edge_cases_and_exhaustion(void)
{
    uint8_t backing[256];
    SYN_SlabAllocator slab;

    size_t sizes[] = {16};
    size_t counts[] = {2};

    /* Insufficient backing memory */
    size_t big_counts[] = {100};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(&slab, backing, sizeof(backing), sizes, big_counts, 1));

    /* Exceed max classes */
    size_t many_sizes[10] = {16, 32, 48, 64, 80, 96, 112, 128, 144, 160};
    size_t many_counts[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_init(&slab, backing, sizeof(backing),
                                                           many_sizes, many_counts, 10));

    /* Valid init with 2 blocks of 16B */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_init(&slab, backing, sizeof(backing), sizes, counts, 1));

    /* Alloc zero size */
    TEST_ASSERT_NULL(syn_slab_alloc(&slab, 0));
    TEST_ASSERT_NULL(syn_slab_alloc(NULL, 10));

    /* Alloc larger than max class */
    TEST_ASSERT_NULL(syn_slab_alloc(&slab, 500));

    /* Exhaust class capacity */
    void *b1 = syn_slab_alloc(&slab, 16);
    void *b2 = syn_slab_alloc(&slab, 16);
    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_NOT_NULL(b2);

    /* 3rd allocation fails */
    TEST_ASSERT_NULL(syn_slab_alloc(&slab, 16));

    /* Misaligned pointer free */
    uint8_t *misaligned = ((uint8_t *)b1) + 1;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, misaligned));

    /* Free b1 and re-allocate */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, b1));
    void *b3 = syn_slab_alloc(&slab, 16);
    TEST_ASSERT_NOT_NULL(b3);

    syn_slab_free(&slab, b2);
    syn_slab_free(&slab, b3);

    /* Double free when fully free (lines 104-105 of syn_slab.c) */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, b3));

    /* Block size < 4 (line 32 of syn_slab.c) */
    size_t tiny_sizes[] = {1};
    size_t tiny_counts[] = {1};
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_slab_init(&slab, backing, sizeof(backing), tiny_sizes, tiny_counts, 1));
}

static void test_slab_null_params(void)
{
    uint8_t backing[64];
    size_t sizes[] = {16};
    size_t counts[] = {1};
    SYN_SlabAllocator slab;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_init(NULL, NULL, 0, NULL, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_init(&slab, NULL, 64, sizes, counts, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_init(&slab, backing, 64, NULL, counts, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_init(&slab, backing, 64, sizes, NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_init(&slab, backing, 64, sizes, counts, 0));
    TEST_ASSERT_NULL(syn_slab_alloc(NULL, 16));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_slab_free(NULL, NULL));
}

static void test_r5_slab_alignment_and_double_free(void)
{
    uintptr_t raw_words[128];
    uint8_t *unaligned = ((uint8_t *)raw_words) + 1U;
    SYN_SlabAllocator slab;

    /* Too-small unaligned buffer (total_size <= alignment adjust) rejected */
    size_t sizes_odd[] = {12U, 20U};
    size_t counts_odd[] = {3U, 3U};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(&slab, unaligned, 2U, sizes_odd, counts_odd, 2U));

    /* Zero block count in class rejected */
    size_t zero_counts[] = {0U};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_init(&slab, raw_words, sizeof(raw_words),
                                                           sizes_odd, zero_counts, 1U));

    /* Unaligned backing + odd sizes (12, 20) aligned to sizeof(void *) */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_slab_init(&slab, unaligned, sizeof(raw_words) - 1U, sizes_odd, counts_odd, 2U));

    void *a1 = syn_slab_alloc(&slab, 12U);
    void *a2 = syn_slab_alloc(&slab, 12U);
    void *a3 = syn_slab_alloc(&slab, 12U);
    void *b1 = syn_slab_alloc(&slab, 20U);
    TEST_ASSERT_NOT_NULL(a1);
    TEST_ASSERT_NOT_NULL(a2);
    TEST_ASSERT_NOT_NULL(a3);
    TEST_ASSERT_NOT_NULL(b1);
    TEST_ASSERT_EQUAL_UINT(0U, (uintptr_t)a1 % sizeof(void *));
    TEST_ASSERT_EQUAL_UINT(0U, (uintptr_t)a2 % sizeof(void *));
    TEST_ASSERT_EQUAL_UINT(0U, (uintptr_t)a3 % sizeof(void *));
    TEST_ASSERT_EQUAL_UINT(0U, (uintptr_t)b1 % sizeof(void *));

    /* Out-of-bounds before start_ptr and after slab end */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, (void *)raw_words));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_free(&slab, ((uint8_t *)raw_words) + sizeof(raw_words)));

    /* Double-free while 1 block (a3) remains allocated in class 0 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, a1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, a2));
    /* Free head (a2) and second node (a1) again — both must fail without cycle */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, a2));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, a1));

    SYN_SlabStats stats;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_get_stats(&slab, &stats));
    TEST_ASSERT_EQUAL_size_t(2U, stats.total_allocs);

    /* Exhaust remaining class 1 blocks so best-fit fallback cannot satisfy class 0 overflow */
    void *b2 = syn_slab_alloc(&slab, 20U);
    void *b3 = syn_slab_alloc(&slab, 20U);
    TEST_ASSERT_NOT_NULL(b2);
    TEST_ASSERT_NOT_NULL(b3);

    /* Re-allocating twice yields two distinct blocks, third fails */
    void *r1 = syn_slab_alloc(&slab, 12U);
    void *r2 = syn_slab_alloc(&slab, 12U);
    TEST_ASSERT_NOT_NULL(r1);
    TEST_ASSERT_NOT_NULL(r2);
    TEST_ASSERT_NOT_EQUAL(r1, r2);
    TEST_ASSERT_NULL(syn_slab_alloc(&slab, 12U));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, r1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, r2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, a3));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, b1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, b2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, b3));

    /* Class with > 64 blocks exercises idx >= 64 fallback in slab_mark_block_free */
    uintptr_t big_words[70];
    size_t big_sz[] = {sizeof(void *)};
    size_t big_cnt[] = {66U};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_slab_init(&slab, big_words, sizeof(big_words), big_sz, big_cnt, 1U));
    /* Freelist starts at idx 65, then 64, then 63 */
    void *hi65 = syn_slab_alloc(&slab, sizeof(void *));
    void *hi64 = syn_slab_alloc(&slab, sizeof(void *));
    void *lo63 = syn_slab_alloc(&slab, sizeof(void *));
    TEST_ASSERT_NOT_NULL(hi65);
    TEST_ASSERT_NOT_NULL(hi64);
    TEST_ASSERT_NOT_NULL(lo63);
    /* Free hi64 then lo63 so hi64 is second in freelist; double-free hi64 & valid free hi65 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, hi64));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, lo63));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_slab_free(&slab, hi64));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_slab_free(&slab, hi65));
}

static void test_tier_a_slab_zero_and_overflow_block_size(void)
{
    uintptr_t backing[32];
    SYN_SlabAllocator slab;
    size_t counts[] = {1U};

    /* UTL-05: Zero block size rejected */
    size_t zero_sizes[] = {0U};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(&slab, backing, sizeof(backing), zero_sizes, counts, 1U));

    /* UTL-05: Near-SIZE_MAX block size cannot wrap SLAB_ALIGN_PTR to 0 */
    size_t wrap_sizes1[] = {SIZE_MAX};
    size_t wrap_sizes2[] = {SIZE_MAX - (sizeof(void *) - 2U)};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(&slab, backing, sizeof(backing), wrap_sizes1, counts, 1U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_slab_init(&slab, backing, sizeof(backing), wrap_sizes2, counts, 1U));
    TEST_ASSERT_NULL(syn_slab_alloc(&slab, 4U));
}

void run_slab_tests(void)
{
    RUN_TEST(test_slab_alloc_free_stats);
    RUN_TEST(test_slab_edge_cases_and_exhaustion);
    RUN_TEST(test_slab_null_params);
    RUN_TEST(test_r5_slab_alignment_and_double_free);
    RUN_TEST(test_tier_a_slab_zero_and_overflow_block_size);
}
