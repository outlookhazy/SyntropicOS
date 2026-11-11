/**
 * @file test_mfcc.c
 * @brief Unity tests for Zero-Heap MFCC Feature Extractor.
 */

#include "mocks/mock_port.h"
#include "syntropic/dsp/syn_mfcc.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static SYN_MFCC g_mfcc;

void test_mfcc_init_and_null_checks(void)
{
    int16_t pcm[128] = {0};
    q7_t out[13] = {0};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_init(NULL, 16000, 128));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_init(&g_mfcc, 16000, 32));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_init(&g_mfcc, 0, 128));

    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_init(&g_mfcc, 16000, 128));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_process_frame(NULL, pcm, out));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_process_frame(&g_mfcc, NULL, out));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mfcc_process_frame(&g_mfcc, pcm, NULL));
}

void test_mfcc_process_sine_wave(void)
{
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_init(&g_mfcc, 16000, 128));

    int16_t pcm[128];
    for (int i = 0; i < 128; i++) {
        pcm[i] = (int16_t)(1000.0f * sinf(2.0f * 3.14159f * 1000.0f * i / 16000.0f));
    }

    q7_t coeffs[13];
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_process_frame(&g_mfcc, pcm, coeffs));

    /* Verify non-zero output features */
    bool non_zero = false;
    for (int i = 0; i < 13; i++) {
        if (coeffs[i] != 0) {
            non_zero = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(non_zero);

    /* Test 64-point FFT configuration */
    SYN_MFCC mfcc64;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_init(&mfcc64, 8000, 64));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_process_frame(&mfcc64, pcm, coeffs));

    /* Test FFT error branch */
    mfcc64.fft_size = 15; /* Not a power of 2 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mfcc_process_frame(&mfcc64, pcm, coeffs));

    /* High amplitude sine wave to trigger log_val > 127 saturation clamp (Line 70) */
    int16_t max_pcm[128];
    for (int i = 0; i < 128; i++) {
        max_pcm[i] = 32767;
    }
    SYN_MFCC mfcc_sat;
    syn_mfcc_init(&mfcc_sat, 16000, 128);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_process_frame(&mfcc_sat, max_pcm, coeffs));
}

static void test_mfcc_log_compression_and_mel_energies(void)
{
    int16_t pcm_low[128];
    int16_t pcm_high[128];
    q7_t coeffs_low[13] = {0};
    q7_t coeffs_high[13] = {0};

    for (int i = 0; i < 128; i++) {
        float s = sinf(2.0f * 3.14159265f * 1000.0f * (float)i / 16000.0f);
        pcm_low[i] = (int16_t)(1000.0f * s);
        pcm_high[i] = (int16_t)(4000.0f * s);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_init(&g_mfcc, 16000U, 128U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_process_frame(&g_mfcc, pcm_low, coeffs_low));
    q16_t peak_mel_low = 0;
    for (uint16_t m = 0U; m < SYN_MFCC_NUM_MEL_FILTERS; m++) {
        if (g_mfcc.mel_energies[m] > peak_mel_low) {
            peak_mel_low = g_mfcc.mel_energies[m];
        }
    }
    TEST_ASSERT_TRUE(peak_mel_low > 0);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mfcc_process_frame(&g_mfcc, pcm_high, coeffs_high));
    q16_t peak_mel_high = 0;
    for (uint16_t m = 0U; m < SYN_MFCC_NUM_MEL_FILTERS; m++) {
        if (g_mfcc.mel_energies[m] > peak_mel_high) {
            peak_mel_high = g_mfcc.mel_energies[m];
        }
    }
    /* 4x amplitude = 16x power = +4.0 log2 units (262144 in Q16.16) */
    TEST_ASSERT_INT_WITHIN(Q16_HALF, Q16_FROM_INT(4), peak_mel_high - peak_mel_low);
    TEST_ASSERT_NOT_EQUAL(coeffs_low[0], coeffs_high[0]);
}

void run_mfcc_tests(void)
{
    RUN_TEST(test_mfcc_init_and_null_checks);
    RUN_TEST(test_mfcc_process_sine_wave);
    RUN_TEST(test_mfcc_log_compression_and_mel_energies);
}
