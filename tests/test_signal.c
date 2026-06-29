/**
 * @file test_signal.c
 * @brief Unity tests for syn_signal.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/dsp/syn_signal.h"

static void test_signal(void)
{

    int32_t samples[8];
    SYN_Signal sig;
    syn_signal_init(&sig, samples, 8);

    TEST_ASSERT_EQUAL_INT(0, syn_signal_count(&sig));
    TEST_ASSERT_FALSE(syn_signal_full(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_mean(&sig));

    /* Push some values: 10, 20, 30, 40, 50 */
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 20);
    syn_signal_push(&sig, 30);
    syn_signal_push(&sig, 40);
    syn_signal_push(&sig, 50);

    TEST_ASSERT_EQUAL_INT(5, syn_signal_count(&sig));
    TEST_ASSERT_EQUAL_INT(10, syn_signal_min(&sig));
    TEST_ASSERT_EQUAL_INT(50, syn_signal_max(&sig));
    TEST_ASSERT_EQUAL_INT(30, syn_signal_mean(&sig));
    TEST_ASSERT_EQUAL_INT(40, syn_signal_peak_to_peak(&sig));
    TEST_ASSERT_EQUAL_INT(50, syn_signal_latest(&sig));

    /* Delta (50 - 40 = 10) */
    TEST_ASSERT_EQUAL_INT(10, syn_signal_delta(&sig));

    /* Access by index */
    TEST_ASSERT_EQUAL_INT(10, syn_signal_at(&sig, 0));
    TEST_ASSERT_EQUAL_INT(50, syn_signal_at(&sig, 4));

    /* Sum */
    TEST_ASSERT_EQUAL_INT(150, syn_signal_sum(&sig));

    /* Variance: Var(10,20,30,40,50) = 200.0 → Q16.16 = 200 << 16 */
    int32_t var = syn_signal_variance_q16(&sig);
    /* 200 in Q16.16 = 200 * 65536 = 13107200 */
    TEST_ASSERT_EQUAL_INT(13107200, var);

    /* Fill to capacity (window = 8) then overflow */
    syn_signal_push(&sig, 60);
    syn_signal_push(&sig, 70);
    syn_signal_push(&sig, 80);
    TEST_ASSERT_TRUE(syn_signal_full(&sig));
    TEST_ASSERT_EQUAL_INT(8, syn_signal_count(&sig));

    /* Push one more → evicts oldest (10) */
    syn_signal_push(&sig, 100);
    TEST_ASSERT_EQUAL_INT(8, syn_signal_count(&sig));
    TEST_ASSERT_EQUAL_INT(20, syn_signal_min(&sig));
    TEST_ASSERT_EQUAL_INT(100, syn_signal_max(&sig));

    /* Clear */
    syn_signal_clear(&sig);
    TEST_ASSERT_EQUAL_INT(0, syn_signal_count(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_mean(&sig));

    /* Negative values */
    syn_signal_push(&sig, -10);
    syn_signal_push(&sig, -20);
    syn_signal_push(&sig, -30);
    TEST_ASSERT_EQUAL_INT(-30, syn_signal_min(&sig));
    TEST_ASSERT_EQUAL_INT(-10, syn_signal_max(&sig));
    TEST_ASSERT_EQUAL_INT(-20, syn_signal_mean(&sig));
}

void run_signal_tests(void)
{
    RUN_TEST(test_signal);
}
