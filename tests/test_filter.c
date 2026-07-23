/**
 * @file test_filter.c
 * @brief Unity tests for syn_filter — full coverage (adds reset paths).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/dsp/syn_filter.h"

static void test_filters(void)
{
    /* Moving average */
    SYN_FilterMA ma;
    syn_filter_ma_init(&ma, 4);

    int16_t v;
    v = syn_filter_ma_update(&ma, 100);
    TEST_ASSERT_EQUAL_INT(100, v);
    v = syn_filter_ma_update(&ma, 200);
    TEST_ASSERT_EQUAL_INT(150, v);
    v = syn_filter_ma_update(&ma, 300);
    TEST_ASSERT_EQUAL_INT(200, v);
    v = syn_filter_ma_update(&ma, 400);
    TEST_ASSERT_EQUAL_INT(250, v);
    /* Window full — oldest drops */
    v = syn_filter_ma_update(&ma, 400);
    TEST_ASSERT_EQUAL_INT(325, v);

    syn_filter_ma_reset(&ma);
    v = syn_filter_ma_update(&ma, 50);
    TEST_ASSERT_EQUAL_INT(50, v);

    /* EMA */
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 128); /* alpha = 0.5 */
    v = syn_filter_ema_update(&ema, 100);
    TEST_ASSERT_EQUAL_INT(100, v);
    v = syn_filter_ema_update(&ema, 200);
    TEST_ASSERT_EQUAL_INT(150, v);

    /* Median */
    SYN_FilterMedian med;
    syn_filter_median_init(&med, 5);
    syn_filter_median_update(&med, 10);
    syn_filter_median_update(&med, 50);
    syn_filter_median_update(&med, 20);
    syn_filter_median_update(&med, 90);
    v = syn_filter_median_update(&med, 30);
    /* sorted: 10,20,30,50,90 -> median = 30 */
    TEST_ASSERT_EQUAL_INT(30, v);

    /* Spike rejection */
    syn_filter_median_init(&med, 3);
    syn_filter_median_update(&med, 100);
    syn_filter_median_update(&med, 100);
    v = syn_filter_median_update(&med, 9999);
    /* sorted: 100,100,9999 -> median = 100 */
    TEST_ASSERT_EQUAL_INT(100, v);
}

/** EMA reset — preserves alpha, clears value and primed state */
static void test_filter_ema_reset(void)
{
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 200); /* alpha = 200 */
    syn_filter_ema_update(&ema, 500);
    TEST_ASSERT_TRUE(ema.primed);

    /* Reset — should zero value and primed, keep alpha */
    syn_filter_ema_reset(&ema);
    TEST_ASSERT_FALSE(ema.primed);
    TEST_ASSERT_EQUAL_INT(0, ema.value);
    TEST_ASSERT_EQUAL_INT(200, ema.alpha); /* preserved */

    /* After reset, next update seeds the filter fresh */
    int16_t v = syn_filter_ema_update(&ema, 300);
    TEST_ASSERT_EQUAL_INT(300, v); /* first value = seed */
}

/** Median reset — preserves window size, clears buffer */
static void test_filter_median_reset(void)
{
    SYN_FilterMedian med;
    syn_filter_median_init(&med, 5);
    syn_filter_median_update(&med, 100);
    syn_filter_median_update(&med, 200);
    TEST_ASSERT_EQUAL_INT(2, med.count);

    syn_filter_median_reset(&med);
    TEST_ASSERT_EQUAL_INT(0, med.count);
    TEST_ASSERT_EQUAL_INT(5, med.window); /* preserved */

    /* After reset, behaves like freshly initialized */
    int16_t v = syn_filter_median_update(&med, 42);
    TEST_ASSERT_EQUAL_INT(42, v);
}

/** FIR filter test — 3-tap moving average */
static void test_filter_fir(void)
{
    q16_t taps[3] = { Q16_FROM_FRAC(1, 3), Q16_FROM_FRAC(1, 3), Q16_FROM_FRAC(1, 3) };
    q16_t history[3];

    SYN_FilterFIR fir;
    syn_filter_fir_init(&fir, taps, history, 3);

    /* Impulse response: feed 1.0 */
    q16_t out1 = syn_filter_fir_update(&fir, Q16_ONE);
    q16_t out2 = syn_filter_fir_update(&fir, 0);
    q16_t out3 = syn_filter_fir_update(&fir, 0);
    q16_t out4 = syn_filter_fir_update(&fir, 0);

    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_FROM_FRAC(1, 3), out1);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_FROM_FRAC(1, 3), out2);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_FROM_FRAC(1, 3), out3);
    TEST_ASSERT_EQUAL(0, out4);

    /* Reset test */
    syn_filter_fir_update(&fir, Q16_ONE);
    syn_filter_fir_reset(&fir);
    TEST_ASSERT_EQUAL(0, syn_filter_fir_update(&fir, 0));
}

void run_filter_tests(void)
{
    RUN_TEST(test_filters);
    RUN_TEST(test_filter_ema_reset);
    RUN_TEST(test_filter_median_reset);
    RUN_TEST(test_filter_fir);
}
