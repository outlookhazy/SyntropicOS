/**
 * @file test_dsp_ext.c
 * @brief Unity tests for Biquad filter and FFT extensions.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include <math.h>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

static void test_trig_math(void)
{
    /* Test 0 */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), 0, q16_sin(0));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), Q16_ONE, q16_cos(0));

    /* Test PI/2 */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), Q16_ONE, q16_sin(Q16_PI_2));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), 0, q16_cos(Q16_PI_2));

    /* Test PI */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), 0, q16_sin(Q16_PI));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), -Q16_ONE, q16_cos(Q16_PI));

    /* Test -PI/2 */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), -Q16_ONE, q16_sin(-Q16_PI_2));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.005), 0, q16_cos(-Q16_PI_2));

    /* Test wrap around (2*PI + PI/6) = 13*PI/6 */
    q16_t x = Q16_2_PI + Q16_PI / 6;
    q16_t expected_sin = Q16_FROM_FLOAT(0.5); /* sin(pi/6) = 0.5 */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.002), expected_sin, q16_sin(x));
}

static void test_biquad_filter(void)
{
    SYN_FilterBiquad f;

    /* Setup lowpass filter: Cutoff = 10Hz, Sample Rate = 100Hz */
    syn_filter_biquad_lowpass(&f, Q16_FROM_INT(10), Q16_FROM_INT(100));

    /* Feed step input of 1.0 (Q16_ONE) for 40 steps */
    q16_t out = 0;
    for (int i = 0; i < 40; i++) {
        out = syn_filter_biquad_update(&f, Q16_ONE);
    }

    /* Verify step input converges to 1.0 (within 5% error margin) */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.05), Q16_ONE, out);
}

static void test_biquad_attenuation(void)
{
    SYN_FilterBiquad f;
    /* Lowpass filter: Cutoff = 5Hz, fs = 100Hz */
    syn_filter_biquad_lowpass(&f, Q16_FROM_INT(5), Q16_FROM_INT(100));

    /* 1. Low frequency input (1Hz): should pass with minimal attenuation */
    q16_t max_out_low = 0;
    syn_filter_biquad_reset(&f);
    for (int i = 0; i < 200; i++) {
        /* sin(2 * pi * 1 * i / 100) */
        double angle = 2.0 * 3.1415926535 * 1.0 * i / 100.0;
        q16_t in = Q16_FROM_FLOAT(sin(angle));
        q16_t out = syn_filter_biquad_update(&f, in);
        if (i > 100) { /* wait for transient response to settle */
            if (q16_abs(out) > max_out_low) max_out_low = q16_abs(out);
        }
    }
    /* Output amplitude should be > 0.85 (gain at 1Hz is close to 1) */
    TEST_ASSERT_TRUE(max_out_low > Q16_FROM_FLOAT(0.85));

    /* 2. High frequency input (40Hz): should be heavily attenuated */
    q16_t max_out_high = 0;
    syn_filter_biquad_reset(&f);
    for (int i = 0; i < 200; i++) {
        /* sin(2 * pi * 40 * i / 100) */
        double angle = 2.0 * 3.1415926535 * 40.0 * i / 100.0;
        q16_t in = Q16_FROM_FLOAT(sin(angle));
        q16_t out = syn_filter_biquad_update(&f, in);
        if (i > 100) {
            if (q16_abs(out) > max_out_high) max_out_high = q16_abs(out);
        }
    }
    /* Output amplitude should be heavily attenuated (< 0.15) */
    TEST_ASSERT_TRUE(max_out_high < Q16_FROM_FLOAT(0.15));
}

static void test_fft(void)
{
    q16_t real[32];
    q16_t imag[32];

    /* Generate a 4Hz cosine wave in N=32 samples: cos(2 * pi * 4 * i / 32) */
    for (int i = 0; i < 32; i++) {
        double angle = 2.0 * 3.1415926535 * 4.0 * i / 32.0;
        real[i] = Q16_FROM_FLOAT(cos(angle));
        imag[i] = 0;
    }

    SYN_Status st = syn_dsp_fft(real, imag, 32);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Verify FFT peak matches the input frequency of 4Hz */
    q16_t magnitude[32];
    for (int i = 0; i < 32; i++) {
        double r = (double)real[i] / 65536.0;
        double im = (double)imag[i] / 65536.0;
        magnitude[i] = Q16_FROM_FLOAT(sqrt(r*r + im*im));
    }

    q16_t max_mag = 0;
    int max_idx = -1;
    for (int i = 0; i < 16; i++) { /* First half of the spectrum */
        if (magnitude[i] > max_mag) {
            max_mag = magnitude[i];
            max_idx = i;
        }
    }

    TEST_ASSERT_EQUAL_INT(4, max_idx);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.1), Q16_FROM_INT(16), max_mag);
}

static void test_fft_dc(void)
{
    q16_t real[32];
    q16_t imag[32];

    /* DC Signal: Constant 1.0 */
    for (int i = 0; i < 32; i++) {
        real[i] = Q16_ONE;
        imag[i] = 0;
    }

    SYN_Status st = syn_dsp_fft(real, imag, 32);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* DC bin (index 0) should be N * Amplitude = 32.0 */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_FROM_INT(32), real[0]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), 0, imag[0]);

    /* All other bins should be zero */
    for (int i = 1; i < 32; i++) {
        TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), 0, real[i]);
        TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), 0, imag[i]);
    }
}

static void test_fft_impulse(void)
{
    q16_t real[32];
    q16_t imag[32];

    /* Impulse at index 0: 1.0, all others 0 */
    real[0] = Q16_ONE;
    imag[0] = 0;
    for (int i = 1; i < 32; i++) {
        real[i] = 0;
        imag[i] = 0;
    }

    SYN_Status st = syn_dsp_fft(real, imag, 32);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Magnitude should be flat 1.0 across all bins */
    for (int i = 0; i < 32; i++) {
        double r = (double)real[i] / 65536.0;
        double im = (double)imag[i] / 65536.0;
        q16_t magnitude = Q16_FROM_FLOAT(sqrt(r*r + im*im));
        TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_ONE, magnitude);
    }
}

static void test_fft_invalid(void)
{
    q16_t real[32];
    q16_t imag[32];

    /* Not power of 2 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_dsp_fft(real, imag, 30));

    /* Too small (< 2) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_dsp_fft(real, imag, 1));

    /* Too large (> 256) */
    q16_t real_large[512];
    q16_t imag_large[512];
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_dsp_fft(real_large, imag_large, 512));
}

static void test_fft_against_reference(void)
{
    q16_t real[32];
    q16_t imag[32];
    double ref_real[32];
    double ref_imag[32];

    /* Generate an arbitrary composite signal: x[t] = 0.5*cos(2*pi*2*t) + 0.2*sin(2*pi*7*t) + 0.1 */
    for (int i = 0; i < 32; i++) {
        double t = (double)i / 32.0;
        double val = 0.5 * cos(2.0 * M_PI * 2.0 * t) + 0.2 * sin(2.0 * M_PI * 7.0 * t) + 0.1;
        real[i] = Q16_FROM_FLOAT(val);
        imag[i] = 0;
    }

    /* Compute reference DFT using double-precision float direct summation */
    for (int k = 0; k < 32; k++) {
        double sum_r = 0;
        double sum_i = 0;
        for (int n = 0; n < 32; n++) {
            double angle = 2.0 * M_PI * k * n / 32.0;
            double x_r = (double)real[n] / 65536.0;
            sum_r += x_r * cos(angle);
            sum_i += -x_r * sin(angle);
        }
        ref_real[k] = sum_r;
        ref_imag[k] = sum_i;
    }

    /* Run our fixed-point FFT */
    SYN_Status st = syn_dsp_fft(real, imag, 32);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Assert that every bin matches the reference DFT within 0.02 (2% full-scale of amplitude 1) */
    for (int k = 0; k < 32; k++) {
        double fft_r = (double)real[k] / 65536.0;
        double fft_i = (double)imag[k] / 65536.0;

        double diff_r = ref_real[k] - fft_r;
        double diff_i = ref_imag[k] - fft_i;

        if (diff_r < 0) diff_r = -diff_r;
        if (diff_i < 0) diff_i = -diff_i;

        TEST_ASSERT_TRUE(diff_r < 0.02);
        TEST_ASSERT_TRUE(diff_i < 0.02);
    }
}

void run_biquad_tests(void)
{
    RUN_TEST(test_biquad_filter);
    RUN_TEST(test_biquad_attenuation);
    RUN_TEST(test_trig_math);
}

void run_fft_tests(void)
{
    RUN_TEST(test_fft);
    RUN_TEST(test_fft_dc);
    RUN_TEST(test_fft_impulse);
    RUN_TEST(test_fft_invalid);
    RUN_TEST(test_fft_against_reference);
}
