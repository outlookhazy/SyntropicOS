/**
 * @file test_filter_design.c
 * @brief Unity tests for Dynamic IIR Biquad Filter Design Generator.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/dsp/syn_filter_design.h"
#include "syntropic/dsp/syn_biquad.h"

static void test_filter_design_lowpass(void)
{
    SYN_BiquadCoeffs coeffs;
    q16_t fc = Q16_FROM_INT(100);
    q16_t fs = Q16_FROM_INT(1000);
    q16_t Q  = Q16_FROM_FLOAT(0.7071); /* Butterworth */

    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_design_lowpass(fc, fs, Q, &coeffs));

    /* Check coefficient properties for Butterworth LPF */
    TEST_ASSERT_TRUE(coeffs.b0 > 0);
    TEST_ASSERT_TRUE(coeffs.b1 > 0);
    TEST_ASSERT_TRUE(coeffs.b2 > 0);
    TEST_ASSERT_TRUE(coeffs.a1 < 0);

    SYN_FilterBiquad biquad;
    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_design_apply_to_biquad(&coeffs, &biquad));

    /* Test processing a DC signal (1.0) -> steady-state output should approach 1.0 */
    q16_t out = 0;
    for (int i = 0; i < 50; i++) {
        out = syn_filter_biquad_update(&biquad, Q16_ONE);
    }
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.05), Q16_ONE, out);
}

static void test_filter_design_highpass(void)
{
    SYN_BiquadCoeffs coeffs;
    q16_t fc = Q16_FROM_INT(200);
    q16_t fs = Q16_FROM_INT(1000);
    q16_t Q  = Q16_FROM_FLOAT(0.7071);

    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_design_highpass(fc, fs, Q, &coeffs));

    /* Highpass b1 coefficient should be negative */
    TEST_ASSERT_TRUE(coeffs.b0 > 0);
    TEST_ASSERT_TRUE(coeffs.b1 < 0);

    SYN_FilterBiquad biquad;
    syn_filter_design_apply_to_biquad(&coeffs, &biquad);

    /* Test processing a DC signal (1.0) -> highpass output should attenuate to 0.0 */
    q16_t out = 0;
    for (int i = 0; i < 50; i++) {
        out = syn_filter_biquad_update(&biquad, Q16_ONE);
    }
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.05), 0, out);
}

static void test_filter_design_bandpass_and_notch(void)
{
    SYN_BiquadCoeffs bp_coeffs, notch_coeffs;
    q16_t fc = Q16_FROM_INT(60);
    q16_t fs = Q16_FROM_INT(1000);
    q16_t Q  = Q16_FROM_INT(5);

    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_design_bandpass(fc, fs, Q, &bp_coeffs));
    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_design_notch(fc, fs, Q, &notch_coeffs));

    TEST_ASSERT_EQUAL(0, bp_coeffs.b1);
    TEST_ASSERT_EQUAL(bp_coeffs.b1, bp_coeffs.b1);

    SYN_FilterBiquad notch_biquad;
    syn_filter_design_apply_to_biquad(&notch_coeffs, &notch_biquad);

    /* Feed 60 Hz hum: x[n] = cos(2*pi*60*n/1000) */
    q16_t hum_out_peak = 0;
    for (int i = 0; i < 300; i++) {
        q16_t t = q16_mul(Q16_2_PI, q16_div(Q16_FROM_INT(60 * i), fs));
        q16_t sample = q16_cos(t);
        q16_t filtered = syn_filter_biquad_update(&notch_biquad, sample);
        if (i > 150 && q16_abs(filtered) > hum_out_peak) {
            hum_out_peak = q16_abs(filtered);
        }
    }

    /* 60 Hz hum should be heavily attenuated in steady-state (peak < 0.25) */
    TEST_ASSERT_TRUE(hum_out_peak < Q16_FROM_FLOAT(0.25));
}

static void test_filter_design_invalid_params(void)
{
    SYN_BiquadCoeffs coeffs;
    q16_t fc = Q16_FROM_INT(100);
    q16_t fs = Q16_FROM_INT(1000);
    q16_t Q  = Q16_FROM_FLOAT(0.7071);

    /* NULL outputs */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_lowpass(fc, fs, Q, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_highpass(fc, fs, Q, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_bandpass(fc, fs, Q, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_notch(fc, fs, Q, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_apply_to_biquad(NULL, NULL));

    /* Frequency limits (fc <= 0 or fc >= fs/2) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_lowpass(0, fs, Q, &coeffs));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_lowpass(Q16_FROM_INT(500), fs, Q, &coeffs));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_lowpass(fc, 0, Q, &coeffs));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_design_lowpass(fc, fs, 0, &coeffs));
}

void run_filter_design_tests(void)
{
    RUN_TEST(test_filter_design_lowpass);
    RUN_TEST(test_filter_design_highpass);
    RUN_TEST(test_filter_design_bandpass_and_notch);
    RUN_TEST(test_filter_design_invalid_params);
}
