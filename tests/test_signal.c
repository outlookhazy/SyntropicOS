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

/** Variance/min/max on a full (wrapped) buffer — exercises lines 127 and 164 */
static void test_signal_wrapped_stats(void)
{
    static int32_t samples2[4];
    SYN_Signal sig;
    syn_signal_init(&sig, samples2, 4); /* capacity = 4 */

    /* Push 6 values — wraps the ring buffer (count stays at capacity) */
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 20);
    syn_signal_push(&sig, 30);
    syn_signal_push(&sig, 40);
    syn_signal_push(&sig, 50); /* wraps: oldest (10) evicted */
    syn_signal_push(&sig, 60); /* wraps: oldest (20) evicted */
    /* Buffer now contains [30, 40, 50, 60] (in circular order) */
    TEST_ASSERT_EQUAL_INT(60, syn_signal_max(&sig));
    TEST_ASSERT_EQUAL_INT(30, syn_signal_min(&sig));
    /* variance: mean=45, diffs: -15,-5,5,15 → var = (225+25+25+225)/4 = 125 */
    int32_t var = syn_signal_variance_q16(&sig);
    TEST_ASSERT_TRUE(var > 0);
}

static void test_signal_rms_std_dev(void)
{
    int32_t samples[4];
    SYN_Signal sig;
    syn_signal_init(&sig, samples, 4);

    TEST_ASSERT_EQUAL_INT(0, syn_signal_rms_q16(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_std_dev_q16(&sig));

    /* Push 10, 10, 10, 10 -> mean=10, RMS=10 (in Q16: 10<<16 = 655360), std_dev=0 */
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 10);

    TEST_ASSERT_INT_WITHIN(100, 655360, syn_signal_rms_q16(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_std_dev_q16(&sig));

    /* Push 0, 10, 0, 10 -> variance > 0 -> std_dev > 0 */
    syn_signal_clear(&sig);
    syn_signal_push(&sig, 0);
    syn_signal_push(&sig, 10);
    TEST_ASSERT_TRUE(syn_signal_std_dev_q16(&sig) > 0);
}

static void test_signal_edge_cases(void)
{
    int32_t samples[2];
    SYN_Signal sig;
    syn_signal_init(&sig, samples, 2);

    TEST_ASSERT_EQUAL_INT(0, syn_signal_min(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_max(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_latest(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_delta(&sig));
    TEST_ASSERT_EQUAL_INT(0, syn_signal_at(&sig, 5));

    syn_signal_push(&sig, 42);
    TEST_ASSERT_EQUAL_INT(0, syn_signal_delta(&sig));
    TEST_ASSERT_EQUAL_INT(42, syn_signal_latest(&sig));

    syn_signal_push(&sig, 50);
    /* Delta = 50 - 42 = 8 */
    TEST_ASSERT_EQUAL_INT(8, syn_signal_delta(&sig));
    /* Wrap ring buffer with 60 */
    syn_signal_push(&sig, 60);
    TEST_ASSERT_EQUAL_INT(10, syn_signal_delta(&sig));
}

void run_signal_tests(void)
{
    RUN_TEST(test_signal);
    RUN_TEST(test_signal_wrapped_stats);
    RUN_TEST(test_signal_rms_std_dev);
    RUN_TEST(test_signal_edge_cases);
}
