/**
 * @file test_math.c
 * @brief Unity tests for syn_math.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_qmath.h"

static void test_qmath(void)
{

    q16_t a = Q16_FROM_INT(3);
    q16_t b = Q16_FROM_INT(2);
    TEST_ASSERT_EQUAL_INT(3, Q16_TO_INT(a));

    /* Add/sub */
    TEST_ASSERT_EQUAL_INT(5, Q16_TO_INT(q16_add(a, b)));
    TEST_ASSERT_EQUAL_INT(1, Q16_TO_INT(q16_sub(a, b)));

    /* Multiply */
    q16_t c = q16_mul(a, Q16_FROM_FRAC(1, 2));
    TEST_ASSERT_EQUAL_INT(1, Q16_TO_INT(c));
    TEST_ASSERT_EQUAL_INT(2, Q16_TO_INT_ROUND(c));

    /* Divide */
    q16_t d = q16_div(Q16_FROM_INT(10), Q16_FROM_INT(4));
    TEST_ASSERT_EQUAL_INT(2, Q16_TO_INT(d));
    TEST_ASSERT_EQUAL_INT(500, Q16_FRAC_1000(d));

    /* Abs */
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), q16_abs(Q16_FROM_INT(-5)));

    /* Clamp */
    q16_t lo = Q16_FROM_INT(0);
    q16_t hi = Q16_FROM_INT(100);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(50), q16_clamp(Q16_FROM_INT(50), lo, hi));
    TEST_ASSERT_EQUAL(lo, q16_clamp(Q16_FROM_INT(-10), lo, hi));
    TEST_ASSERT_EQUAL(hi, q16_clamp(Q16_FROM_INT(200), lo, hi));

    /* Lerp */
    q16_t r = q16_lerp(Q16_FROM_INT(0), Q16_FROM_INT(100), Q16_FROM_FRAC(1, 2));
    TEST_ASSERT_EQUAL_INT(50, Q16_TO_INT(r));

    /* Saturating add */
    q16_t big = INT32_MAX - 1000;
    TEST_ASSERT_EQUAL(INT32_MAX, q16_add_sat(big, Q16_FROM_INT(1)));
}

static void test_rate_limit(void)
{

    mock_tick_ms = 0;

    SYN_RateLimit rl;
    syn_rate_limit_init(&rl, 3, 1000);  /* 3 per second */

    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_FALSE(syn_rate_limit_allow(&rl));

    TEST_ASSERT_EQUAL_INT(0, syn_rate_limit_remaining(&rl));

    /* After full interval, tokens refill */
    mock_tick_advance(1000);
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));

    /* Reset */
    syn_rate_limit_reset(&rl);
    TEST_ASSERT_EQUAL_INT(3, syn_rate_limit_remaining(&rl));
}

void run_math_tests(void)
{
    RUN_TEST(test_qmath);
    RUN_TEST(test_rate_limit);
}
