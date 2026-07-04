/**
 * @file test_backoff.c
 * @brief Unit tests for syn_backoff (Exponential Backoff with Jitter).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/util/syn_backoff.h"

void test_backoff_initial_state(void)
{
    SYN_Backoff b;
    syn_backoff_init(&b, 100, 1000, 2, 3);
    
    TEST_ASSERT_EQUAL_UINT32(100, b.min_ms);
    TEST_ASSERT_EQUAL_UINT32(1000, b.max_ms);
    TEST_ASSERT_EQUAL_UINT8(0, b.attempts);
    TEST_ASSERT_FALSE(syn_backoff_exhausted(&b));
}

void test_backoff_exponential_growth(void)
{
    SYN_Backoff b;
    /* factor = 2, jitter is base + [0, base/2]. 
     * We'll check that the result is within the expected range. */
    syn_backoff_init(&b, 100, 1000, 2, 5);
    
    /* Attempt 1: base = 100. Range = [100, 150] */
    uint32_t d1 = syn_backoff_next_ms(&b);
    TEST_ASSERT_TRUE(d1 >= 100 && d1 <= 150);
    TEST_ASSERT_EQUAL_UINT8(1, b.attempts);
    
    /* Attempt 2: base = 200. Range = [200, 300] */
    uint32_t d2 = syn_backoff_next_ms(&b);
    TEST_ASSERT_TRUE(d2 >= 200 && d2 <= 300);
    
    /* Attempt 3: base = 400. Range = [400, 600] */
    uint32_t d3 = syn_backoff_next_ms(&b);
    TEST_ASSERT_TRUE(d3 >= 400 && d3 <= 600);
}

void test_backoff_capping(void)
{
    SYN_Backoff b;
    syn_backoff_init(&b, 100, 500, 2, 10);
    
    syn_backoff_next_ms(&b); /* 100 */
    syn_backoff_next_ms(&b); /* 200 */
    syn_backoff_next_ms(&b); /* 400 */
    
    /* Next should be capped at 500 */
    uint32_t d4 = syn_backoff_next_ms(&b);
    TEST_ASSERT_TRUE(d4 >= 500 && d4 <= 750);
    TEST_ASSERT_EQUAL_UINT32(500, b.current_ms);
}

void test_backoff_exhaustion(void)
{
    SYN_Backoff b;
    syn_backoff_init(&b, 100, 1000, 2, 2);
    
    TEST_ASSERT_FALSE(syn_backoff_exhausted(&b));
    syn_backoff_next_ms(&b);
    TEST_ASSERT_FALSE(syn_backoff_exhausted(&b));
    syn_backoff_next_ms(&b);
    TEST_ASSERT_TRUE(syn_backoff_exhausted(&b));
}

void test_backoff_reset(void)
{
    SYN_Backoff b;
    syn_backoff_init(&b, 100, 1000, 2, 5);
    
    syn_backoff_next_ms(&b);
    syn_backoff_next_ms(&b);
    TEST_ASSERT_EQUAL_UINT8(2, b.attempts);
    
    syn_backoff_reset(&b);
    TEST_ASSERT_EQUAL_UINT8(0, b.attempts);
    TEST_ASSERT_EQUAL_UINT32(100, b.current_ms);
}

void run_backoff_tests(void)
{
    RUN_TEST(test_backoff_initial_state);
    RUN_TEST(test_backoff_exponential_growth);
    RUN_TEST(test_backoff_capping);
    RUN_TEST(test_backoff_exhaustion);
    RUN_TEST(test_backoff_reset);
}
