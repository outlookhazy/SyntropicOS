/**
 * @file test_filter.c
 * @brief Unity tests for syn_filter.
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

void run_filter_tests(void)
{
    RUN_TEST(test_filters);
}
