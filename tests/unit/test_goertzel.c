#include "syntropic/dsp/syn_goertzel.h"
#include "unity/unity.h"

#include <math.h>
#include <string.h>

static SYN_Goertzel g_goertzel;

void test_goertzel_init_and_null_checks(void)
{
    int16_t sample = 100;
    int16_t block[4] = {10, 20, 30, 40};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_init(NULL, 8000, 697, 205));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_init(&g_goertzel, 0, 697, 205));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_init(&g_goertzel, 8000, 0, 205));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_init(&g_goertzel, 8000, 697, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_goertzel_init(&g_goertzel, 8000, 4500, 205)); /* Nyquist overflow */

    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_init(&g_goertzel, 8000, 697, 205));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_reset(NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_goertzel_process_sample(NULL, sample));
    TEST_ASSERT_EQUAL(0, syn_goertzel_process_block(NULL, block, 4));
    TEST_ASSERT_EQUAL(0, syn_goertzel_process_block(&g_goertzel, NULL, 4));
    TEST_ASSERT_EQUAL(0, syn_goertzel_process_block(&g_goertzel, block, 0));

    TEST_ASSERT_EQUAL_UINT64(0ULL, syn_goertzel_get_magnitude_sq(NULL));
}

void test_goertzel_dtmf_tone_detection(void)
{
    /* Sampling frequency 8000 Hz, DTMF Row 1 tone = 697 Hz, Block size N = 205 */
    uint32_t fs = 8000;
    uint32_t target_freq = 697;
    size_t n = 205;

    SYN_Goertzel g_matched;
    SYN_Goertzel g_unmatched;

    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_init(&g_matched, fs, target_freq, n)); /* 697 Hz */
    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_init(&g_unmatched, fs, 1477, n));      /* 1477 Hz */

    /* Generate 205 samples of 697 Hz sine wave: x[i] = 10000 * sin(2 * pi * 697 * i / 8000) */
    int16_t pcm_697[205];
    for (size_t i = 0; i < n; i++) {
        double angle = 2.0 * 3.141592653589793 * 697.0 * (double)i / 8000.0;
        pcm_697[i] = (int16_t)(10000.0 * sin(angle));
    }

    TEST_ASSERT_EQUAL(n, syn_goertzel_process_block(&g_matched, pcm_697, n));
    TEST_ASSERT_EQUAL(n, syn_goertzel_process_block(&g_unmatched, pcm_697, n));

    uint64_t mag_matched = syn_goertzel_get_magnitude_sq(&g_matched);
    uint64_t mag_unmatched = syn_goertzel_get_magnitude_sq(&g_unmatched);

    /* Matched target frequency power should be significantly higher than out-of-band tone power */
    TEST_ASSERT_TRUE(mag_matched > mag_unmatched * 10ULL);

    /* Reset test */
    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_reset(&g_matched));
    TEST_ASSERT_EQUAL_INT32(0, g_matched.s1);
    TEST_ASSERT_EQUAL_INT32(0, g_matched.s2);
    TEST_ASSERT_EQUAL(0, g_matched.n_count);
}

void test_r5_goertzel_large_block_no_overflow(void)
{
    SYN_Goertzel g;
    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_init(&g, 8000U, 1000U, 8000U));

    /* 8000 samples at full-scale 32767 drives s1, s2 > 1e8 without int64_t multiply UB */
    static const int16_t wave8[8] = {0, 23170, 32767, 23170, 0, -23170, -32767, -23170};
    for (size_t i = 0U; i < 8000U; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_process_sample(&g, wave8[i & 7U]));
    }
    uint64_t mag_sq = syn_goertzel_get_magnitude_sq(&g);
    TEST_ASSERT_TRUE(mag_sq > 1000000000000ULL);

    /* Exercise positive and negative resonator state clamping + INT32_MIN magnitude guard */
    g.coeff_q15 = 32768;
    g.s1 = 2147480000;
    g.s2 = -2147480000;
    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_process_sample(&g, 32767));
    TEST_ASSERT_EQUAL_INT32(2147483647, g.s1);

    g.s1 = -2147480000;
    g.s2 = 2147480000;
    TEST_ASSERT_EQUAL(SYN_OK, syn_goertzel_process_sample(&g, -32768));
    TEST_ASSERT_EQUAL_INT32(-2147483647, g.s1);
    TEST_ASSERT_TRUE(syn_goertzel_get_magnitude_sq(&g) > 0ULL);

    g.s1 = INT32_MIN;
    g.s2 = INT32_MIN;
    g.coeff_q15 = -16384;
    TEST_ASSERT_TRUE(syn_goertzel_get_magnitude_sq(&g) > 0ULL);

    /* Rounding guard: prod >= sq_sum returns 0 */
    g.s1 = 1;
    g.s2 = 1;
    g.coeff_q15 = 32768;
    TEST_ASSERT_EQUAL_UINT64(0ULL, syn_goertzel_get_magnitude_sq(&g));
}

void run_goertzel_tests(void)
{
    RUN_TEST(test_goertzel_init_and_null_checks);
    RUN_TEST(test_goertzel_dtmf_tone_detection);
    RUN_TEST(test_r5_goertzel_large_block_no_overflow);
}
