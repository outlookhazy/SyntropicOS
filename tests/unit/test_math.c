/**
 * @file test_math.c
 * @brief Unity tests for syn_math.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_qmath.h"
#include "unity/unity.h"

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
    syn_rate_limit_init(&rl, 3, 1000); /* 3 per second */

    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_FALSE(syn_rate_limit_allow(&rl));

    TEST_ASSERT_EQUAL_INT(0, syn_rate_limit_remaining(&rl));

    /* After full interval, tokens refill */
    mock_tick_advance(1000);
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));

    /* Partial refill test: advance by 500ms (half interval) */
    syn_rate_limit_allow(&rl);
    syn_rate_limit_allow(&rl);
    syn_rate_limit_allow(&rl);
    TEST_ASSERT_FALSE(syn_rate_limit_allow(&rl));
    mock_tick_advance(500);
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));

    /* Partial refill token cap test (line 76 of syn_rate_limit.h) */
    SYN_RateLimit rl_cap;
    syn_rate_limit_init(&rl_cap, 10, 1000);
    syn_rate_limit_allow(&rl_cap); /* tokens = 9 */
    mock_tick_advance(900);
    TEST_ASSERT_TRUE(
        syn_rate_limit_allow(&rl_cap)); /* Partial refill 9 tokens -> 9+9=18 > 10, clamps to 10 */

    /* Reset */
    syn_rate_limit_reset(&rl);
    TEST_ASSERT_EQUAL_INT(3, syn_rate_limit_remaining(&rl));

    /* interval_ms == 0 must not divide by zero; acts as non-refilling token budget */
    SYN_RateLimit rl_zero;
    syn_rate_limit_init(&rl_zero, 2U, 0U);
    mock_tick_advance(100);
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl_zero));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl_zero));
    TEST_ASSERT_FALSE(syn_rate_limit_allow(&rl_zero));

    /* Large max_tokens * elapsed must not overflow uint32_t during partial refill */
    SYN_RateLimit rl_big;
    syn_rate_limit_init(&rl_big, 10000000U, 1000U);
    (void)syn_rate_limit_allow(&rl_big);
    (void)syn_rate_limit_allow(&rl_big);
    mock_tick_advance(
        500U); /* 10,000,000 * 500 = 5e9 > UINT32_MAX; exact half refill = 5,000,000 */
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl_big));
    TEST_ASSERT_EQUAL_UINT32(9999999U, syn_rate_limit_remaining(&rl_big));
}

static void test_q16_rounding(void)
{
    q16_t x_pos = Q16_FROM_INT(2) + Q16_FROM_FRAC(3, 4);  /* 2.75 */
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
    q16_t coeffs[3] = {Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3)};

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

static void test_q16_trig_fast(void)
{
    q16_t s_fast = q16_sin_fast(Q16_PI_2);
    q16_t c_fast = q16_cos_fast(0);
    q16_t s_out, c_out;

    q16_sincos_fast(Q16_PI / 4, &s_out, &c_out);

    /* sin(pi/2) ≈ 1.0, cos(0) ≈ 1.0 */
    TEST_ASSERT_INT32_WITHIN(100, Q16_ONE, s_fast);
    TEST_ASSERT_INT32_WITHIN(100, Q16_ONE, c_fast);

    /* sin(pi/4) == cos(pi/4) ≈ 0.7071 */
    TEST_ASSERT_INT32_WITHIN(100, s_out, c_out);

    /* Test negative angle wrap: x < -Q16_PI */
    TEST_ASSERT_INT32_WITHIN(200, 0, q16_sin(-4 * Q16_PI));
    TEST_ASSERT_INT32_WITHIN(200, 0, q16_sin_fast(-4 * Q16_PI));
    TEST_ASSERT_INT32_WITHIN(200, Q16_ONE, q16_cos_fast(-4 * Q16_PI));
}

static void test_q16_inv_and_rsqrt(void)
{
    /* 1 / 2.0 = 0.5 */
    TEST_ASSERT_EQUAL_INT32(Q16_HALF, q16_inv(Q16_FROM_INT(2)));
    /* 1 / 0.5 = 2.0 */
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(2), q16_inv(Q16_HALF));
    /* 1 / 1.0 = 1.0 */
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q16_inv(Q16_ONE));

    /* 1 / 0.00001 (1 in Q16) -> INT32_MAX overflow */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_inv(1));

    /* 1 / sqrt(4.0) = 0.5 */
    TEST_ASSERT_EQUAL_INT32(Q16_HALF, q16_rsqrt(Q16_FROM_INT(4)));
    /* 1 / sqrt(1.0) = 1.0 */
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q16_rsqrt(Q16_ONE));

    /* q16_hypot exact non-overflow at INT32_MAX / 2 (~23170.47) and clamping at INT32_MAX */
    TEST_ASSERT_INT32_WITHIN(4, 1518500249, q16_hypot(INT32_MAX / 2, INT32_MAX / 2));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_hypot(INT32_MAX, INT32_MAX));
}

static void test_q16_exp_log_fast(void)
{
    /* e^0 = 1.0 */
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q16_exp_fast(0));
    /* ln(1.0) = 0 */
    TEST_ASSERT_EQUAL_INT32(0, q16_log_fast(Q16_ONE));

    /* e^1.0 ≈ 2.718 */
    q16_t ef = q16_exp_fast(Q16_ONE);
    TEST_ASSERT_INT32_WITHIN(1000, Q16_FROM_FRAC(2718, 1000), ef);

    /* ln(2.0) ≈ 0.693 */
    q16_t lf = q16_log_fast(Q16_FROM_INT(2));
    TEST_ASSERT_INT32_WITHIN(1000, Q16_LN2, lf);
}

static void test_q16_q7_q15_edge_cases(void)
{
    /* q16_div by zero */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_div(Q16_FROM_INT(5), 0));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_div(-Q16_FROM_INT(5), 0));

    /* q16_sqrt negative input */
    TEST_ASSERT_EQUAL_INT32(0, q16_sqrt(-Q16_FROM_INT(4)));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(2), q16_sqrt(Q16_FROM_INT(4)));

    /* q16_atan2 all 4 quadrants and origin */
    TEST_ASSERT_EQUAL_INT32(0, q16_atan2(0, 0));
    TEST_ASSERT_INT32_WITHIN(500, Q16_PI / 4, q16_atan2(Q16_ONE, Q16_ONE));
    TEST_ASSERT_INT32_WITHIN(500, (3 * Q16_PI) / 4, q16_atan2(Q16_ONE, -Q16_ONE));
    TEST_ASSERT_INT32_WITHIN(500, -(3 * Q16_PI) / 4, q16_atan2(-Q16_ONE, -Q16_ONE));
    TEST_ASSERT_INT32_WITHIN(500, -Q16_PI / 4, q16_atan2(-Q16_ONE, Q16_ONE));

    /* Q7 & Q15 conversions */
    q7_t q7_val = 64; /* 0.5 in Q7 */
    q16_t q16_conv = q7_to_q16(q7_val);
    TEST_ASSERT_EQUAL_INT32(Q16_HALF, q16_conv);
    TEST_ASSERT_EQUAL_INT8(q7_val, q16_to_q7(q16_conv));
    TEST_ASSERT_EQUAL_INT8(127, q16_to_q7(Q16_FROM_INT(500)));
    TEST_ASSERT_EQUAL_INT8(-128, q16_to_q7(-Q16_FROM_INT(500)));

    q15_t q15_val = 16384; /* 0.5 in Q15 */
    q16_t q16_q15 = q15_to_q16(q15_val);
    TEST_ASSERT_EQUAL_INT32(Q16_HALF, q16_q15);
    TEST_ASSERT_EQUAL_INT16(q15_val, q16_to_q15(q16_q15));

    TEST_ASSERT_EQUAL_INT8(32, q7_mul(64, 64)); /* 0.5 * 0.5 = 0.25 in Q7 */

    /* q16_inv edge cases */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_inv(0));
    TEST_ASSERT_EQUAL_INT32(-32768, q16_inv(-Q16_FROM_INT(2)));
    TEST_ASSERT_EQUAL_INT32(0, q16_inv(INT32_MIN));

    /* q16_rsqrt edge cases */
    TEST_ASSERT_EQUAL_INT32(0, q16_rsqrt(0));
    TEST_ASSERT_EQUAL_INT32(0, q16_rsqrt(-100));

    /* q16_hypot large values triggering shift branch: sqrt(200^2 + 200^2) ≈ 282.84 */
    q16_t large_hypot = q16_hypot(Q16_FROM_INT(200), Q16_FROM_INT(200));
    TEST_ASSERT_INT32_WITHIN(2000, 18536380, large_hypot);

    /* q16_exp overflow and underflow shift branches */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_exp(Q16_FROM_INT(12)));
    TEST_ASSERT_INT32_WITHIN(5, 0, q16_exp(-Q16_FROM_INT(20)));

    /* q16_exp_fast negative, overflow, underflow */
    TEST_ASSERT_INT32_WITHIN(1000, Q16_ONE, q16_exp_fast(0));
    TEST_ASSERT_INT32_WITHIN(1000, 24109, q16_exp_fast(-Q16_ONE)); /* exp(-1) ~ 0.3678 in Q16.16 */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_exp_fast(Q16_FROM_INT(12)));
    TEST_ASSERT_INT32_WITHIN(5, 0, q16_exp_fast(-Q16_FROM_INT(20)));

    /* q16_tan near pi/2 (line 100 of syn_qmath.c) */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_tan(Q16_PI_2));

    /* Q16 conversions and saturating arithmetic non-clamping paths (lines 132, 155, 157, 263, 279,
     * 295 of syn_qmath.h) */
    TEST_ASSERT_EQUAL_INT8(-128, q16_to_q7(Q16_FROM_INT(-200)));
    TEST_ASSERT_EQUAL_INT16(32767, q16_to_q15(Q16_FROM_INT(100)));
    TEST_ASSERT_EQUAL_INT16(-32768, q16_to_q15(Q16_FROM_INT(-100)));
    TEST_ASSERT_EQUAL_INT32(2 * Q16_ONE, q16_add_sat(Q16_ONE, Q16_ONE));
    TEST_ASSERT_EQUAL_INT32(0, q16_sub_sat(Q16_ONE, Q16_ONE));
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q16_mul_sat(Q16_ONE, Q16_ONE));
}

static void test_q16_str_conversion_branches(void)
{
    char buf[4];
    q16_t val;
    /* 1. q16_to_str with too small output buffer */
    TEST_ASSERT_EQUAL(0, q16_to_str(Q16_FROM_INT(12345), buf, sizeof(buf), 2));

    /* 2. q16_from_str null check */
    TEST_ASSERT_EQUAL(0, q16_from_str(NULL, &val));
    TEST_ASSERT_EQUAL(0, q16_from_str("1.5", NULL));

    /* 3. q16_from_str explicit '+' sign */
    TEST_ASSERT_GREATER_THAN(0, q16_from_str("+1.5", &val));
    TEST_ASSERT_INT32_WITHIN(100, Q16_FROM_INT(1) + 32768, val);

    /* 4. q16_from_str invalid non-digit start */
    TEST_ASSERT_EQUAL(0, q16_from_str("abc", &val));

    /* 5. q16_from_str precision limit (> 5 fractional digits) */
    TEST_ASSERT_GREATER_THAN(0, q16_from_str("1.12345678", &val));
    TEST_ASSERT_EQUAL_UINT(10U, q16_from_str("1.12345678", &val));
}

static void test_q16_exp_clamping_and_underflow_branches(void)
{
    /* Underflow k < -16 in q16_exp and q16_exp_fast */
    TEST_ASSERT_INT32_WITHIN(5, 2, q16_exp(-Q16_FROM_INT(25)));
    TEST_ASSERT_INT32_WITHIN(5, 2, q16_exp_fast(-Q16_FROM_INT(25)));

    /* Negative exp_fast returning inverse of INT32_MAX */
    TEST_ASSERT_INT32_WITHIN(5, 2, q16_exp_fast(-Q16_FROM_INT(15)));
}

static void test_q16_exp_remainder_boundary_branches(void)
{
    /* Test x close to integer multiples of Q16_LN2 */
    q16_t x1 = Q16_LN2 - 1;
    q16_t x2 = Q16_LN2 + 1;
    q16_exp(x1);
    q16_exp(x2);
    q16_exp_fast(x1);
    q16_exp_fast(x2);

    /* Test k >= 15 overflow in q16_exp_fast */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_exp_fast(Q16_FROM_INT(12)));
}

static void test_q16_saturating_arithmetic_overflow_underflow(void)
{
    /* q16_add_sat underflow */
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_add_sat(INT32_MIN, -100));

    /* q16_sub_sat overflow */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_sub_sat(INT32_MAX, -100));

    /* q16_mul_sat underflow */
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_mul_sat(INT32_MAX, -Q16_FROM_INT(2)));
}

static void test_q16_exp_large_k_clamping_and_underflow(void)
{
    /* k >= 15 overflow */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_exp(Q16_FROM_INT(12)));

    /* Negative exp returning inverse of INT32_MAX */
    TEST_ASSERT_INT32_WITHIN(5, 2, q16_exp(-Q16_FROM_INT(20)));
}

static void test_q16_exp_fast_k_overflow(void)
{
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_exp_fast(Q16_FROM_INT(12)));
}

static void test_q16_exp_fast_negative_underflow(void)
{
    TEST_ASSERT_INT32_WITHIN(5, 2, q16_exp_fast(-Q16_FROM_INT(20)));
}

static void test_q16_trig_overflow_and_atan2_poles(void)
{
    /* q16_div with zero divisor */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_div(Q16_ONE, 0));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_div(-Q16_ONE, 0));

    /* q16_rsqrt with negative input returns 0 */
    TEST_ASSERT_EQUAL_INT32(0, q16_rsqrt(-1));

    /* q16_exp and q16_exp_fast underflow k < -16 */
    TEST_ASSERT_INT32_WITHIN(5, 0, q16_exp(-Q16_FROM_INT(30)));
    TEST_ASSERT_INT32_WITHIN(5, 0, q16_exp_fast(-Q16_FROM_INT(30)));

    /* q16_log_fast zero, negative, and fractional (ux < Q16_ONE) */
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_log_fast(0));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_log_fast(-10));
    TEST_ASSERT_INT32_WITHIN(1000, -90871, q16_log_fast(Q16_ONE / 4));

    /* q16_to_str with decimals > 4 and NULL/zero buffer (line 548) */
    char str_buf[32];
    TEST_ASSERT_GREATER_THAN(0, q16_to_str(Q16_FROM_INT(5), str_buf, sizeof(str_buf), 6));
    TEST_ASSERT_EQUAL_UINT(0, q16_to_str(Q16_FROM_INT(5), NULL, 10, 2));
    TEST_ASSERT_EQUAL_UINT(0, q16_to_str(Q16_FROM_INT(5), str_buf, 0, 2));
}

static void test_r5_qmath_ubsan_and_asin_accuracy(void)
{
    /* 1. Negative signed shift UB fixes in q7_to_q16 / q15_to_q16 */
    TEST_ASSERT_EQUAL_INT32(-Q16_ONE, q7_to_q16(Q7_MIN));
    TEST_ASSERT_EQUAL_INT32(-Q16_HALF, q7_to_q16(-64));
    TEST_ASSERT_EQUAL_INT32(-Q16_ONE, q15_to_q16(Q15_MIN));
    TEST_ASSERT_EQUAL_INT32(-Q16_HALF, q15_to_q16(-16384));

    /* 2. q16_asin / q16_acos accuracy near +-1.0 (< 0.05 deg == 57 LSB in Q16 rad) */
    /* asin(65535/65536) = 1.5652707 rad = 102582 in Q16.16 */
    TEST_ASSERT_INT32_WITHIN(50, 102582, q16_asin(Q16_ONE - 1));
    TEST_ASSERT_INT32_WITHIN(50, -102582, q16_asin(-(Q16_ONE - 1)));
    /* asin(0.5) = pi/6 = 34315, asin(0.8660254) = pi/3 = 68629 */
    TEST_ASSERT_INT32_WITHIN(20, 34315, q16_asin(Q16_HALF));
    TEST_ASSERT_INT32_WITHIN(30, 68629, q16_asin(56756));
    TEST_ASSERT_INT32_WITHIN(30, -68629, q16_asin(-56756));
    /* acos(65535/65536) = 0.0055256 rad = 362 in Q16.16 */
    TEST_ASSERT_INT32_WITHIN(50, 362, q16_acos(Q16_ONE - 1));

    /* 3. q16_hypot accuracy at 150, 150 and UBSan safety on INT32_MIN */
    /* sqrt(150^2 + 150^2) * 65536 = 13902284 */
    TEST_ASSERT_INT32_WITHIN(4, 13902284, q16_hypot(Q16_FROM_INT(150), Q16_FROM_INT(150)));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_hypot(INT32_MIN, 0));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_hypot(0, INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_hypot(INT32_MIN, INT32_MIN));

    /* 4. q16_abs, q16_lerp, q16_div signed overflow guards */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_abs(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(0, q16_lerp(Q16_FROM_INT(-20000), Q16_FROM_INT(20000), Q16_HALF));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_lerp(0, Q16_FROM_INT(20000), Q16_FROM_INT(2)));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_lerp(0, Q16_FROM_INT(-20000), Q16_FROM_INT(2)));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_div(INT32_MIN, -1));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_div(INT32_MIN, -Q16_ONE));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_div(INT32_MIN, 1));

    /* 5. q16_cos / q16_cos_fast near INT32_MAX and INT32_MIN */
    q16_t c_max = q16_cos(INT32_MAX);
    q16_t c_min = q16_cos(INT32_MIN);
    q16_t cf_max = q16_cos_fast(INT32_MAX);
    q16_t cf_min = q16_cos_fast(INT32_MIN);
    TEST_ASSERT_TRUE(c_max >= -Q16_ONE - 10 && c_max <= Q16_ONE + 10);
    TEST_ASSERT_TRUE(c_min >= -Q16_ONE - 10 && c_min <= Q16_ONE + 10);
    TEST_ASSERT_TRUE(cf_max >= -Q16_ONE - 100 && cf_max <= Q16_ONE + 100);
    TEST_ASSERT_TRUE(cf_min >= -Q16_ONE - 100 && cf_min <= Q16_ONE + 100);

    /* 6. q16_ceil, q16_round, Q16_TO_INT_ROUND, Q16_FRAC_1000/10000 near extremes */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_ceil(INT32_MAX));
    TEST_ASSERT_EQUAL_INT32((q16_t)0x7FFF0000, q16_ceil((q16_t)0x7FFF0000));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_ceil(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, q16_round(INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, q16_round(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(32768, Q16_TO_INT_ROUND(INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(-32768, Q16_TO_INT_ROUND(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(0, Q16_FRAC_1000(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(0, Q16_FRAC_10000(INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(250, Q16_FRAC_1000(-Q16_FROM_FRAC(9, 4)));
    TEST_ASSERT_EQUAL_INT32(2500, Q16_FRAC_10000(-Q16_FROM_FRAC(9, 4)));

    /* 7. q16_from_str saturation on huge positive/negative integers */
    q16_t parsed = 0;
    TEST_ASSERT_EQUAL_size_t(12U, q16_from_str("999999999999", &parsed));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, parsed);
    TEST_ASSERT_EQUAL_size_t(13U, q16_from_str("-999999999999", &parsed));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, parsed);
    TEST_ASSERT_EQUAL_size_t(6U, q16_from_str("-32768", &parsed));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, parsed);
}

void run_math_tests(void)
{
    RUN_TEST(test_qmath);
    RUN_TEST(test_q16_rounding);
    RUN_TEST(test_q16_saturating);
    RUN_TEST(test_q16_poly_eval);
    RUN_TEST(test_q16_math_edge_cases);
    RUN_TEST(test_q16_trig_fast);
    RUN_TEST(test_q16_inv_and_rsqrt);
    RUN_TEST(test_q16_exp_log_fast);
    RUN_TEST(test_q16_q7_q15_edge_cases);
    RUN_TEST(test_rate_limit);
    RUN_TEST(test_q16_str_conversion_branches);
    RUN_TEST(test_q16_exp_clamping_and_underflow_branches);
    RUN_TEST(test_q16_exp_remainder_boundary_branches);
    RUN_TEST(test_q16_saturating_arithmetic_overflow_underflow);
    RUN_TEST(test_q16_exp_large_k_clamping_and_underflow);
    RUN_TEST(test_q16_exp_fast_k_overflow);
    RUN_TEST(test_q16_exp_fast_negative_underflow);
    RUN_TEST(test_q16_trig_overflow_and_atan2_poles);
    RUN_TEST(test_r5_qmath_ubsan_and_asin_accuracy);
}
