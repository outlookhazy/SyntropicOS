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

static void test_q16_rounding(void)
{
    q16_t x_pos = Q16_FROM_INT(2) + Q16_FROM_FRAC(3, 4); /* 2.75 */
    q16_t x_neg = Q16_FROM_INT(-2) - Q16_FROM_FRAC(3, 4); /* -2.75 */

    TEST_ASSERT_EQUAL(Q16_FROM_INT(2), q16_floor(x_pos));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(-3), q16_floor(x_neg));

    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), q16_ceil(x_pos));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(-2), q16_ceil(x_neg));

    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), q16_round(x_pos));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(-3), q16_round(x_neg));
}

static void test_q16_saturating(void)
{
    q16_t max_val = INT32_MAX;
    q16_t min_val = INT32_MIN;

    TEST_ASSERT_EQUAL(max_val, q16_add_sat(max_val - 10, Q16_FROM_INT(1)));
    TEST_ASSERT_EQUAL(min_val, q16_sub_sat(min_val + 10, Q16_FROM_INT(1)));
    TEST_ASSERT_EQUAL(max_val, q16_mul_sat(Q16_FROM_INT(10000), Q16_FROM_INT(10000)));
}

static void test_q16_poly_eval(void)
{
    /* P(x) = 1 + 2*x + 3*x^2 */
    q16_t coeffs[3] = { Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3) };

    /* P(0) = 1 */
    TEST_ASSERT_EQUAL(Q16_FROM_INT(1), q16_poly_eval(coeffs, 3, 0));

    /* P(1) = 1 + 2 + 3 = 6 */
    TEST_ASSERT_EQUAL(Q16_FROM_INT(6), q16_poly_eval(coeffs, 3, Q16_ONE));

    /* P(2) = 1 + 4 + 12 = 17 */
    TEST_ASSERT_EQUAL(Q16_FROM_INT(17), q16_poly_eval(coeffs, 3, Q16_FROM_INT(2)));

    /* Null / 0 size */
    TEST_ASSERT_EQUAL(0, q16_poly_eval(NULL, 3, Q16_ONE));
    TEST_ASSERT_EQUAL(0, q16_poly_eval(coeffs, 0, Q16_ONE));
}

static void test_q16_math_edge_cases(void)
{
    /* q16_exp edge cases */
    TEST_ASSERT_EQUAL(Q16_ONE, q16_exp(0));
    TEST_ASSERT_EQUAL(INT32_MAX, q16_exp(Q16_FROM_INT(11)));
    /* exp of negative value with negative k */
    q16_t exp_neg = q16_exp(-Q16_FROM_INT(5));
    TEST_ASSERT_TRUE(exp_neg > 0 && exp_neg < Q16_ONE);

    /* q16_log edge cases */
    TEST_ASSERT_EQUAL(INT32_MIN, q16_log(0));
    TEST_ASSERT_EQUAL(INT32_MIN, q16_log(-100));
    TEST_ASSERT_EQUAL(0, q16_log(Q16_ONE));
    /* log of fractional value (x < 1.0) */
    q16_t log_frac = q16_log(Q16_HALF);
    TEST_ASSERT_TRUE(log_frac < 0);

    /* q16_pow edge cases */
    TEST_ASSERT_EQUAL(0, q16_pow(0, Q16_ONE));
    TEST_ASSERT_EQUAL(0, q16_pow(-Q16_ONE, Q16_ONE));
    TEST_ASSERT_EQUAL(Q16_ONE, q16_pow(Q16_FROM_INT(5), 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), q16_pow(Q16_FROM_INT(5), Q16_ONE));
}

void run_math_tests(void)
{
    RUN_TEST(test_qmath);
    RUN_TEST(test_q16_rounding);
    RUN_TEST(test_q16_saturating);
    RUN_TEST(test_q16_poly_eval);
    RUN_TEST(test_q16_math_edge_cases);
    RUN_TEST(test_rate_limit);
}
