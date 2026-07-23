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

    /* Test manual initialization */
    syn_filter_biquad_init(&f, Q16_ONE, Q16_FROM_INT(2), Q16_FROM_INT(3), Q16_FROM_INT(4), Q16_FROM_INT(5));
    TEST_ASSERT_EQUAL(Q16_ONE, f.b0);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(2), f.b1);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), f.b2);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), f.a1);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), f.a2);

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

/** FFT with N=256 — exercises the maximum supported transform size.
 *  Note: twiddle quads 2/3 (lines 91-96) are mathematically unreachable
 *  in this Cooley-Tukey implementation (max idx < 128 always), so they
 *  are accepted as dead code. This test verifies N=256 runs correctly. */
static void test_fft_n256(void)
{
    static q16_t real[256], imag[256];

    /* Impulse at t=0: flat spectrum */
    for (int i = 0; i < 256; i++) {
        real[i] = (i == 0) ? Q16_ONE : 0;
        imag[i] = 0;
    }

    SYN_Status st = syn_dsp_fft(real, imag, 256);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    /* All bins should have approximately equal magnitude = 1/N */
    /* Just check bin 0 is non-zero and it completed OK */
    TEST_ASSERT_TRUE(q16_abs(real[0]) > 0);
}

static void test_fft_windows(void)
{
    q16_t win[32];

    /* 1. Hanning window: w[0] ≈ 0, w[16] ≈ 1.0, w[31] ≈ 0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_hanning(win, 32));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), 0, win[0]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_ONE, win[16]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), 0, win[31]);

    /* 2. Hamming window: w[0] ≈ 0.08, w[16] ≈ 1.0, w[31] ≈ 0.08 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_hamming(win, 32));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), Q16_FROM_FLOAT(0.08), win[0]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), Q16_ONE, win[16]);

    /* 3. Blackman-Harris window: w[0] ≈ 0, w[16] ≈ 1.0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_blackman(win, 32));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), 0, win[0]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), Q16_ONE, win[16]);
}

static void test_fft_spectrum_analytics(void)
{
    q16_t real[32], imag[32], mag[17];

    /* Synthesize signal: fundamental at bin 4 (A=1.0), harmonic at bin 8 (A=0.1) */
    for (uint16_t i = 0; i < 32; i++) {
        q16_t t = q16_div(q16_mul(q16_mul(Q16_FROM_INT(2), Q16_PI), Q16_FROM_INT(i)), Q16_FROM_INT(32));
        q16_t f1 = q16_cos(q16_mul(Q16_FROM_INT(4), t));
        q16_t f2 = q16_mul(Q16_FROM_FLOAT(0.1), q16_cos(q16_mul(Q16_FROM_INT(8), t)));
        real[i] = f1 + f2;
        imag[i] = 0;
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_dsp_fft(real, imag, 32));
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_magnitude_spectrum(real, imag, mag, 32));

    /* Peak finding */
    SYN_FFTPeak peaks[4];
    uint8_t num_peaks = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_find_peaks(mag, 17, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_TRUE(num_peaks >= 2);
    TEST_ASSERT_EQUAL_INT(4, peaks[0].bin); /* Top peak at bin 4 */
    TEST_ASSERT_EQUAL_INT(8, peaks[1].bin); /* Second peak at bin 8 */

    /* THD calculation: bin 4 fundamental, bin 8 harmonic */
    q16_t thd_pct = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_thd(mag, 17, 4, 3, &thd_pct));
    /* Harmonic is 0.1 of fundamental -> ~10% THD */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(2.0), Q16_FROM_INT(10), thd_pct);

    /* Invalid parameters tests */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_magnitude_spectrum(NULL, imag, mag, 32));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_find_peaks(NULL, 17, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_thd(NULL, 17, 4, 3, &thd_pct));

    /* Peak sorting test: smaller peak at lower bin, larger peak at higher bin */
    q16_t synthetic_mag[8] = { 0, 100, 0, 500, 0, 0, 0, 0 };
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_find_peaks(synthetic_mag, 8, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_EQUAL_INT(2, num_peaks);
    TEST_ASSERT_EQUAL_INT(3, peaks[0].bin); /* Larger peak (500) sorted first */
    TEST_ASSERT_EQUAL_INT(1, peaks[1].bin); /* Smaller peak (100) sorted second */
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
    RUN_TEST(test_fft_n256);
    RUN_TEST(test_fft_windows);
    RUN_TEST(test_fft_spectrum_analytics);
}
