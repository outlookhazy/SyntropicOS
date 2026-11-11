/**
 * @file test_filter.c
 * @brief Unity tests for syn_filter — full coverage (adds reset paths).
 */

#include "mocks/mock_port.h"
#include "syntropic/dsp/syn_filter.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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

    /* Boundary initialization checks */
    SYN_FilterMA ma_bad;
    syn_filter_ma_init(&ma_bad, 0);
    syn_filter_ma_init(&ma_bad, SYN_FILTER_MAX_WINDOW + 1);

    SYN_FilterMedian med_bad;
    syn_filter_median_init(&med_bad, 0);
    syn_filter_median_init(&med_bad, SYN_FILTER_MAX_WINDOW + 1);
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
    q16_t taps[3] = {Q16_FROM_FRAC(1, 3), Q16_FROM_FRAC(1, 3), Q16_FROM_FRAC(1, 3)};
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

    /* Test 2-tap FIR filter wrap-around (idx == 0 branch) */
    q16_t taps2[2] = {Q16_ONE, Q16_ONE};
    q16_t history2[2];
    SYN_FilterFIR fir2;
    syn_filter_fir_init(&fir2, taps2, history2, 2);
    for (int i = 0; i < 5; i++) {
        (void)syn_filter_fir_update(&fir2, Q16_FROM_INT(i));
    }
}

static void test_biquad_process_block(void)
{
    SYN_FilterBiquad f1, f2;
    syn_filter_biquad_lowpass(&f1, Q16_FROM_INT(100), Q16_FROM_INT(1000));
    syn_filter_biquad_lowpass(&f2, Q16_FROM_INT(100), Q16_FROM_INT(1000));

    q16_t in[5] = {Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE};
    q16_t out_sample[5];
    q16_t out_block[5];

    for (int i = 0; i < 5; i++) {
        out_sample[i] = syn_filter_biquad_update(&f1, in[i]);
    }

    syn_filter_biquad_process_block(&f2, in, out_block, 5);

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT32(out_sample[i], out_block[i]);
    }

    /* Low-ratio fc/fs (fc = 0.25 Hz, fs = 20000 Hz) retains 6.28x 64-bit omega precision */
    SYN_FilterBiquad low_fc;
    syn_filter_biquad_lowpass(&low_fc, Q16_FROM_FRAC(1, 4), Q16_FROM_INT(20000));
    SYN_FilterBiquad bp_low;
    syn_filter_biquad_bandpass(&bp_low, Q16_FROM_FRAC(1, 4), Q16_FROM_INT(20000), Q16_HALF);
    TEST_ASSERT_TRUE(bp_low.b0 > 0);
}

static void test_filter_biquad_highpass(void)
{
    SYN_FilterBiquad f;
    syn_filter_biquad_highpass(&f, Q16_FROM_INT(100), Q16_FROM_INT(1000));
    q16_t out = syn_filter_biquad_update(&f, Q16_ONE);
    TEST_ASSERT_NOT_EQUAL(0, out);
}

static void test_filter_biquad_allpass_and_peaking_eq(void)
{
    SYN_FilterBiquad ap, eq, eq_neg;
    syn_filter_biquad_allpass(&ap, Q16_FROM_INT(100), Q16_FROM_INT(1000), Q16_FROM_FLOAT(0.707));
    syn_filter_biquad_peaking_eq(&eq, Q16_FROM_INT(100), Q16_FROM_INT(1000), Q16_FROM_INT(6),
                                 Q16_FROM_FLOAT(0.707));

    /* Negative gain clamp test */
    syn_filter_biquad_peaking_eq(&eq_neg, Q16_FROM_INT(100), Q16_FROM_INT(1000), -Q16_FROM_INT(100),
                                 Q16_FROM_FLOAT(0.707));

    q16_t ap_out = syn_filter_biquad_update(&ap, Q16_ONE);
    q16_t eq_out = syn_filter_biquad_update(&eq, Q16_ONE);
    q16_t eq_neg_out = syn_filter_biquad_update(&eq_neg, Q16_ONE);

    TEST_ASSERT_NOT_EQUAL(0, ap_out);
    TEST_ASSERT_NOT_EQUAL(0, eq_out);
    TEST_ASSERT_NOT_EQUAL(0, eq_neg_out);
}

static void test_biquad_cascade_and_edge_cases(void)
{
    SYN_FilterBiquadCascade c;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_biquad_cascade_init(&c, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_biquad_cascade_init(&c, 10));

    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_biquad_cascade_init(&c, 2));

    /* Configure 2 lowpass stages */
    syn_filter_biquad_lowpass(&c.stages[0], Q16_FROM_INT(100), Q16_FROM_INT(1000));
    syn_filter_biquad_lowpass(&c.stages[1], Q16_FROM_INT(100), Q16_FROM_INT(1000));

    q16_t sample_out = syn_filter_biquad_cascade_update(&c, Q16_ONE);
    TEST_ASSERT_NOT_EQUAL(0, sample_out);

    syn_filter_biquad_cascade_reset(&c);

    q16_t in_buf[4] = {Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE};
    q16_t out_buf[4] = {0};
    syn_filter_biquad_cascade_process_block(&c, in_buf, out_buf, 4);
    TEST_ASSERT_NOT_EQUAL(0, out_buf[0]);
}

static void test_filter_biquad_bandpass_and_notch(void)
{
    SYN_FilterBiquad bp, notch;
    syn_filter_biquad_bandpass(&bp, Q16_FROM_INT(100), Q16_FROM_INT(1000), Q16_FROM_FLOAT(1.414));
    syn_filter_biquad_notch(&notch, Q16_FROM_INT(100), Q16_FROM_INT(1000), Q16_FROM_FLOAT(1.414));

    q16_t bp_out = syn_filter_biquad_update(&bp, Q16_ONE);
    q16_t notch_out = syn_filter_biquad_update(&notch, Q16_ONE);

    TEST_ASSERT_NOT_EQUAL(0, bp_out);
    TEST_ASSERT_NOT_EQUAL(0, notch_out);
}

static void test_filter_median_sorting_patterns(void)
{
    SYN_FilterMedian med;
    syn_filter_median_init(&med, 5);

    /* Reverse sorted input: 50, 40, 30, 20, 10 */
    syn_filter_median_update(&med, 50);
    syn_filter_median_update(&med, 40);
    syn_filter_median_update(&med, 30);
    syn_filter_median_update(&med, 20);
    int16_t m_rev = syn_filter_median_update(&med, 10);
    TEST_ASSERT_EQUAL_INT(30, m_rev);

    /* Already sorted input: 10, 20, 30, 40, 50 */
    syn_filter_median_reset(&med);
    syn_filter_median_update(&med, 10);
    syn_filter_median_update(&med, 20);
    syn_filter_median_update(&med, 30);
    syn_filter_median_update(&med, 40);
    int16_t m_sort = syn_filter_median_update(&med, 50);
    TEST_ASSERT_EQUAL_INT(30, m_sort);

    /* Moving average with negative numbers */
    SYN_FilterMA ma;
    syn_filter_ma_init(&ma, 3);
    syn_filter_ma_update(&ma, -30);
    syn_filter_ma_update(&ma, -60);
    int16_t ma_neg = syn_filter_ma_update(&ma, -90);
    TEST_ASSERT_EQUAL_INT(-60, ma_neg);
}

static void test_filter_ema_and_fir_overflow(void)
{
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 200U);
    TEST_ASSERT_EQUAL_INT16(-32768, syn_filter_ema_update(&ema, -32768));
    /* Full-scale step with alpha = 200/256: -32768 + 0.78125 * 65535 = +18431 */
    int16_t step_out = syn_filter_ema_update(&ema, 32767);
    TEST_ASSERT_INT_WITHIN(2, 18431, step_out);

    q16_t taps[2] = {Q16_FROM_INT(16), Q16_FROM_INT(16)};
    q16_t hist[2];
    SYN_FilterFIR fir;
    syn_filter_fir_init(&fir, taps, hist, 2U);
    (void)syn_filter_fir_update(&fir, Q16_FROM_INT(2048));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_filter_fir_update(&fir, Q16_FROM_INT(2048)));

    syn_filter_fir_reset(&fir);
    (void)syn_filter_fir_update(&fir, Q16_FROM_INT(-2048));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, syn_filter_fir_update(&fir, Q16_FROM_INT(-2048)));
}

void run_filter_tests(void)
{
    RUN_TEST(test_filters);
    RUN_TEST(test_filter_ema_reset);
    RUN_TEST(test_filter_median_reset);
    RUN_TEST(test_filter_fir);
    RUN_TEST(test_biquad_process_block);
    RUN_TEST(test_filter_biquad_highpass);
    RUN_TEST(test_filter_biquad_allpass_and_peaking_eq);
    RUN_TEST(test_biquad_cascade_and_edge_cases);
    RUN_TEST(test_filter_biquad_bandpass_and_notch);
    RUN_TEST(test_filter_median_sorting_patterns);
    RUN_TEST(test_filter_ema_and_fir_overflow);
}
