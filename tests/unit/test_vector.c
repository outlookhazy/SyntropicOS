/**
 * @file test_vector.c
 * @brief Unit tests for fixed-point vector operations and signal statistics.
 */

#include "syntropic/util/syn_vector.h"
#include "unity/unity.h"

#define Q16_TOL 50

static void ASSERT_Q16_NEAR(q16_t expected, q16_t actual, q16_t tol)
{
    q16_t diff = q16_abs(expected - actual);
    if (diff > tol) {
        TEST_FAIL_MESSAGE("Q16 values not equal within tolerance");
    }
}

void test_vector_basic_ops(void)
{
    q16_t a[4] = {Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3), Q16_FROM_INT(4)};
    q16_t b[4] = {Q16_FROM_INT(10), Q16_FROM_INT(20), Q16_FROM_INT(30), Q16_FROM_INT(40)};
    q16_t out[4];

    syn_vec_add(a, b, out, 4);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(11), out[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(44), out[3]);

    syn_vec_sub(b, a, out, 4);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(9), out[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(36), out[3]);

    syn_vec_scale(a, Q16_FROM_INT(2), out, 4);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(2), out[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(8), out[3]);
}

void test_vector_stats(void)
{
    q16_t v[5] = {Q16_FROM_INT(6), Q16_FROM_INT(2), Q16_FROM_INT(10), Q16_FROM_INT(4),
                  Q16_FROM_INT(8)};

    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(2), syn_vec_min(v, 5));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(10), syn_vec_max(v, 5));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(6), syn_vec_mean(v, 5));

    /* RMS of [2, 4, 6, 8, 10]: sqrt((4+16+36+64+100)/5) = sqrt(220/5) = sqrt(44) ≈ 6.633 */
    q16_t rms = syn_vec_rms(v, 5);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(6633, 1000), rms, Q16_TOL * 10);

    /* Variance of [2, 4, 6, 8, 10]: mean=6, diffs=[-4,-2,0,2,4], diffs^2=[16,4,0,4,16], sum=40,
     * mean=8 */
    q16_t var = syn_vec_variance(v, 5);
    ASSERT_Q16_NEAR(Q16_FROM_INT(8), var, Q16_TOL * 10);

    /* Clamp test */
    q16_t v_unclamped[4] = {Q16_FROM_INT(-10), Q16_FROM_INT(5), Q16_FROM_INT(15), Q16_FROM_INT(25)};
    q16_t v_clamped[4];
    syn_vec_clamp(v_unclamped, Q16_FROM_INT(0), Q16_FROM_INT(20), v_clamped, 4);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(0), v_clamped[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(5), v_clamped[1]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(15), v_clamped[2]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(20), v_clamped[3]);
}

void test_vector_edge_cases(void)
{
    /* Single element vector */
    q16_t single[1] = {Q16_FROM_INT(-42)};
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-42), syn_vec_min(single, 1));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-42), syn_vec_max(single, 1));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-42), syn_vec_mean(single, 1));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_variance(single, 1));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(42), syn_vec_rms(single, 1));

    /* Negative vector scaling and operations */
    q16_t v_neg[3] = {Q16_FROM_INT(-5), Q16_FROM_INT(-15), Q16_FROM_INT(-10)};
    q16_t v_scaled[3];
    syn_vec_scale(v_neg, -Q16_FROM_INT(2), v_scaled, 3);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(10), v_scaled[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(30), v_scaled[1]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(20), v_scaled[2]);

    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-15), syn_vec_min(v_neg, 3));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-5), syn_vec_max(v_neg, 3));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(-10), syn_vec_mean(v_neg, 3));

    /* Clamping with both bounds exceeded */
    q16_t v_clamp_in[3] = {Q16_FROM_INT(-100), Q16_FROM_INT(50), Q16_FROM_INT(100)};
    q16_t v_clamp_out[3];
    syn_vec_clamp(v_clamp_in, -Q16_FROM_INT(20), Q16_FROM_INT(20), v_clamp_out, 3);
    TEST_ASSERT_EQUAL_INT32(-Q16_FROM_INT(20), v_clamp_out[0]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(20), v_clamp_out[1]);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(20), v_clamp_out[2]);
}

void test_r5_vector_empty_and_null_guards(void)
{
    q16_t v[2] = {Q16_FROM_INT(10), Q16_FROM_INT(-10)};
    q16_t out[2] = {Q16_FROM_INT(77), Q16_FROM_INT(88)};

    TEST_ASSERT_EQUAL_INT32(0, syn_vec_min(v, 0U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_max(v, 0U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_mean(v, 0U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_variance(v, 0U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_rms(v, 0U));

    TEST_ASSERT_EQUAL_INT32(0, syn_vec_min(NULL, 2U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_max(NULL, 2U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_mean(NULL, 2U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_variance(NULL, 2U));
    TEST_ASSERT_EQUAL_INT32(0, syn_vec_rms(NULL, 2U));

    syn_vec_add(v, v, out, 0U);
    syn_vec_sub(v, v, out, 0U);
    syn_vec_scale(v, Q16_ONE, out, 0U);
    syn_vec_clamp(v, -Q16_ONE, Q16_ONE, out, 0U);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(77), out[0]);
}

void run_vector_tests(void)
{
    RUN_TEST(test_vector_basic_ops);
    RUN_TEST(test_vector_stats);
    RUN_TEST(test_vector_edge_cases);
    RUN_TEST(test_r5_vector_empty_and_null_guards);
}
