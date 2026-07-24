/**
 * @file test_random.c
 * @brief Unity tests for syn_random utility.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/util/syn_random.h"
#include <string.h>

void test_random_fill_edge_cases(void)
{
    uint8_t buf[8];
    /* NULL buffer or 0 length should return SYN_OK immediately */
    TEST_ASSERT_EQUAL(SYN_OK, syn_random_fill(NULL, 10));
    TEST_ASSERT_EQUAL(SYN_OK, syn_random_fill(buf, 0));
}

void test_random_range_edge_cases(void)
{
    /* min >= max should return min */
    TEST_ASSERT_EQUAL(10, syn_random_range(10, 10));
    TEST_ASSERT_EQUAL(20, syn_random_range(20, 10));
}

void test_random_fill(void)
{
    uint8_t buf1[16];
    uint8_t buf2[16];
    
    /* Fill two buffers and ensure they are different (statistical probability
     * of being identical is near zero with 128 bits of entropy) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_random_fill(buf1, sizeof(buf1)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_random_fill(buf2, sizeof(buf2)));
    
    TEST_ASSERT_NOT_EQUAL(0, memcmp(buf1, buf2, 16));
}

void test_random_u32(void)
{
    uint32_t v1 = syn_random_u32();
    uint32_t v2 = syn_random_u32();
    
    TEST_ASSERT_NOT_EQUAL(v1, v2);
}

void test_random_range(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_random_fill(NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(10, syn_random_range(10, 5));
    TEST_ASSERT_EQUAL_UINT32(10, syn_random_range(10, 10));

    for (int i = 0; i < 100; i++) {
        uint32_t r = syn_random_range(10, 20);
        TEST_ASSERT_TRUE(r >= 10 && r <= 20);
    }
}

void test_random_fallback(void)
{
    uint8_t buf[16];
    
    /* Test the fallback logic directly */
    TEST_ASSERT_EQUAL(SYN_OK, syn_random_fallback_fill(buf, sizeof(buf)));
    
    /* Verify it actually did something */
    uint8_t zero[16] = {0};
    TEST_ASSERT_NOT_EQUAL(0, memcmp(buf, zero, 16));
}

void test_random_u32_fail(void)
{
    mock_random_skip = true;
    TEST_ASSERT_EQUAL(0, syn_random_u32());
    mock_random_skip = false;
}

void run_random_tests(void)
{
    RUN_TEST(test_random_fill);
    RUN_TEST(test_random_fill_edge_cases);
    RUN_TEST(test_random_u32);
    RUN_TEST(test_random_range);
    RUN_TEST(test_random_range_edge_cases);
    RUN_TEST(test_random_fallback);
    RUN_TEST(test_random_u32_fail);
}
