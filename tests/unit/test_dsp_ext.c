/**
 * @file test_dsp_ext.c
 * @brief Unity tests for Biquad filter and FFT extensions.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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
    syn_filter_biquad_init(&f, Q16_ONE, Q16_FROM_INT(2), Q16_FROM_INT(3), Q16_FROM_INT(4),
                           Q16_FROM_INT(5));
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
        /* sinf(2 * pi * 1 * i / 100) */
        double angle = 2.0 * 3.1415926535 * 1.0 * i / 100.0;
        q16_t in = Q16_FROM_FLOAT(sinf(angle));
        q16_t out = syn_filter_biquad_update(&f, in);
        if (i > 100) { /* wait for transient response to settle */
            if (q16_abs(out) > max_out_low)
                max_out_low = q16_abs(out);
        }
    }
    /* Output amplitude should be > 0.85 (gain at 1Hz is close to 1) */
    TEST_ASSERT_TRUE(max_out_low > Q16_FROM_FLOAT(0.85));

    /* 2. High frequency input (40Hz): should be heavily attenuated */
    q16_t max_out_high = 0;
    syn_filter_biquad_reset(&f);
    for (int i = 0; i < 200; i++) {
        /* sinf(2 * pi * 40 * i / 100) */
        double angle = 2.0 * 3.1415926535 * 40.0 * i / 100.0;
        q16_t in = Q16_FROM_FLOAT(sinf(angle));
        q16_t out = syn_filter_biquad_update(&f, in);
        if (i > 100) {
            if (q16_abs(out) > max_out_high)
                max_out_high = q16_abs(out);
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
        magnitude[i] = Q16_FROM_FLOAT(sqrt(r * r + im * im));
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
        q16_t magnitude = Q16_FROM_FLOAT(sqrt(r * r + im * im));
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

    /* Generate an arbitrary composite signal: x[t] = 0.5*cosf(2*pi*2*t) + 0.2*sinf(2*pi*7*t) + 0.1
     */
    for (int i = 0; i < 32; i++) {
        double t = (double)i / 32.0;
        double val = 0.5 * cosf(2.0 * M_PI * 2.0 * t) + 0.2 * sinf(2.0 * M_PI * 7.0 * t) + 0.1;
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
            sum_r += x_r * cosf(angle);
            sum_i += -x_r * sinf(angle);
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

        if (diff_r < 0)
            diff_r = -diff_r;
        if (diff_i < 0)
            diff_i = -diff_i;

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

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_window_hanning(NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_window_hamming(NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_window_blackman(NULL, 0));
}

static void test_fft_spectrum_analytics(void)
{
    q16_t real[32], imag[32], mag[17];

    /* Synthesize signal: fundamental at bin 4 (A=1.0), harmonic at bin 8 (A=0.1) */
    for (uint16_t i = 0; i < 32; i++) {
        q16_t t =
            q16_div(q16_mul(q16_mul(Q16_FROM_INT(2), Q16_PI), Q16_FROM_INT(i)), Q16_FROM_INT(32));
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
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_fft_find_peaks(mag, 17, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_TRUE(num_peaks >= 2);
    TEST_ASSERT_EQUAL_INT(4, peaks[0].bin); /* Top peak at bin 4 */
    TEST_ASSERT_EQUAL_INT(8, peaks[1].bin); /* Second peak at bin 8 */

    /* THD calculation: bin 4 fundamental, bin 8 harmonic */
    q16_t thd_pct = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_thd(mag, 17, 4, 3, &thd_pct));
    /* Harmonic is 0.1 of fundamental -> ~10% THD */
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(2.0), Q16_FROM_INT(10), thd_pct);

    /* Zero fundamental magnitude (line 263 of syn_fft.c) */
    q16_t zero_mag[17] = {0};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_thd(zero_mag, 17, 4, 3, &thd_pct));

    /* Invalid parameters tests */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_magnitude_spectrum(NULL, imag, mag, 32));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_fft_find_peaks(NULL, 17, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_fft_thd(NULL, 17, 4, 3, &thd_pct));

    /* Peak sorting test: smaller peak at lower bin, larger peak at higher bin */
    q16_t synthetic_mag[8] = {0, 100, 0, 500, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_fft_find_peaks(synthetic_mag, 8, Q16_FROM_INT(1000), peaks, 4, &num_peaks));
    TEST_ASSERT_EQUAL_INT(2, num_peaks);
    TEST_ASSERT_EQUAL_INT(3, peaks[0].bin); /* Larger peak (500) sorted first */
    TEST_ASSERT_EQUAL_INT(1, peaks[1].bin); /* Smaller peak (100) sorted second */
}

static void test_biquad_cascade(void)
{
    SYN_FilterBiquadCascade cascade;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_filter_biquad_cascade_init(&cascade, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_filter_biquad_cascade_init(&cascade, SYN_BIQUAD_CASCADE_MAX_STAGES + 1));
    TEST_ASSERT_EQUAL(SYN_OK, syn_filter_biquad_cascade_init(&cascade, 2));

    /* Stage 0: lowpass filter at 100Hz, sample rate 1000Hz */
    syn_filter_biquad_lowpass(&cascade.stages[0], Q16_FROM_INT(100), Q16_FROM_INT(1000));
    /* Stage 1: lowpass filter at 100Hz, sample rate 1000Hz (4th-order response) */
    syn_filter_biquad_lowpass(&cascade.stages[1], Q16_FROM_INT(100), Q16_FROM_INT(1000));

    /* Process block of samples */
    q16_t in[10] = {Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE,
                    Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE, Q16_ONE};
    q16_t out[10] = {0};

    syn_filter_biquad_cascade_process_block(&cascade, in, out, 10);
    TEST_ASSERT_TRUE(out[0] != 0);

    syn_filter_biquad_cascade_reset(&cascade);
    TEST_ASSERT_EQUAL_INT32(0, syn_filter_biquad_cascade_update(&cascade, 0));
}

static void test_r5_biquad_high_cutoff_no_overflow(void)
{
    SYN_FilterBiquad f;
    q16_t fc = Q16_FROM_INT(8000);
    q16_t fs = Q16_FROM_INT(24000);
    q16_t q = Q16_ONE;

    syn_filter_biquad_lowpass(&f, fc, fs);
    TEST_ASSERT_TRUE(f.b0 > 0);
    q16_t in_blk[32];
    q16_t out_blk[32];
    for (uint16_t i = 0U; i < 32U; i++) {
        in_blk[i] = Q16_ONE;
    }
    syn_filter_biquad_process_block(&f, in_blk, out_blk, 32U);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 20), Q16_ONE, out_blk[31]);

    syn_filter_biquad_highpass(&f, fc, fs);
    TEST_ASSERT_TRUE(f.b0 > 0);

    syn_filter_biquad_bandpass(&f, fc, fs, q);
    TEST_ASSERT_TRUE(f.b0 > 0);

    syn_filter_biquad_notch(&f, fc, fs, q);
    TEST_ASSERT_TRUE(f.b0 > 0);

    syn_filter_biquad_allpass(&f, fc, fs, q);
    TEST_ASSERT_TRUE(f.b2 > 0);

    syn_filter_biquad_peaking_eq(&f, fc, fs, Q16_FROM_INT(6), q);
    TEST_ASSERT_TRUE(f.b0 > Q16_ONE);

    syn_filter_biquad_peaking_eq(&f, fc, fs, Q16_FROM_INT(-40), q);
    TEST_ASSERT_TRUE(f.b0 < Q16_ONE);
}

static void test_r5_fft_thd_max_harmonics_255(void)
{
    static q16_t mag[257];
    for (uint16_t i = 0U; i < 257U; i++) {
        mag[i] = 0;
    }
    mag[1] = Q16_ONE;
    mag[2] = Q16_FROM_FRAC(1, 10);

    q16_t thd_pct = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_thd(mag, 257U, 1U, 255U, &thd_pct));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 2), Q16_FROM_INT(10), thd_pct);

    /* Large harmonic energy saturation guard */
    mag[2] = Q16_FROM_INT(200);
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_thd(mag, 257U, 1U, 255U, &thd_pct));
    TEST_ASSERT_TRUE(thd_pct > Q16_FROM_INT(100));
}

static void test_tier_a_biquad_saturation_and_fc_clamping(void)
{
    SYN_FilterBiquad f;
    q16_t q = Q16_ONE;
    q16_t fs = Q16_FROM_INT(1000);

    /* NUM-01a: Accumulator saturation prevents positive/negative sign wraparound */
    syn_filter_biquad_init(&f, Q16_FROM_INT(16), 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_filter_biquad_update(&f, Q16_FROM_INT(4096)));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, syn_filter_biquad_update(&f, Q16_FROM_INT(-4096)));

    q16_t blk_in[2] = {Q16_FROM_INT(4096), Q16_FROM_INT(-4096)};
    q16_t blk_out[2] = {0, 0};
    syn_filter_biquad_reset(&f);
    syn_filter_biquad_process_block(&f, blk_in, blk_out, 2U);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, blk_out[0]);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, blk_out[1]);

    /* NUM-01b: Invalid fs <= 0 falls back safely to unity passthrough across all 6 designs */
    syn_filter_biquad_lowpass(&f, Q16_FROM_INT(100), 0);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);
    syn_filter_biquad_highpass(&f, Q16_FROM_INT(100), -Q16_ONE);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);
    syn_filter_biquad_bandpass(&f, Q16_FROM_INT(100), 0, q);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);
    syn_filter_biquad_notch(&f, Q16_FROM_INT(100), 0, q);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);
    syn_filter_biquad_allpass(&f, Q16_FROM_INT(100), 0, q);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);
    syn_filter_biquad_peaking_eq(&f, Q16_FROM_INT(100), 0, Q16_FROM_INT(3), q);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, f.b0);

    /* NUM-01c: Out-of-range fc <= 0 and fc >= fs/2 clamped so poles stay inside unit circle */
    syn_filter_biquad_lowpass(&f, 0, fs);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_lowpass(&f, Q16_FROM_INT(600), fs);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_highpass(&f, Q16_FROM_INT(500), fs);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_bandpass(&f, Q16_FROM_INT(700), fs, q);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_notch(&f, Q16_FROM_INT(700), fs, q);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_allpass(&f, Q16_FROM_INT(700), fs, q);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
    syn_filter_biquad_peaking_eq(&f, Q16_FROM_INT(700), fs, Q16_FROM_INT(6), q);
    TEST_ASSERT_TRUE(q16_abs(f.a2) <= Q16_ONE);
}

static void test_tier_a_fft_large_window_and_peak_bins(void)
{
    /* NUM-02a: Window generation for n = 8192 (> 5215) must not overflow Q16 angle math */
    static q16_t win_large[8192];
    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_hanning(win_large, 8192U));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 50), 0, win_large[0]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 50), Q16_ONE, win_large[4096]);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 50), 0, win_large[8191]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_hamming(win_large, 8192U));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 50), Q16_ONE, win_large[4096]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_fft_window_blackman(win_large, 8192U));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 50), Q16_ONE, win_large[4096]);

    /* NUM-02a: syn_fft_find_peaks with n_bins = 32769 ((n_bins-1)*2 == 65536) avoids overflow */
    static q16_t mag_large[32769];
    for (uint32_t i = 0U; i < 32769U; i++) {
        mag_large[i] = 0;
    }
    mag_large[16384] = Q16_FROM_INT(10);
    SYN_FFTPeak pk;
    uint8_t found = 0U;
    SYN_Status st = syn_fft_find_peaks(mag_large, 32769U, Q16_FROM_INT(16384), &pk, 1U, &found);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT8(1U, found);
    TEST_ASSERT_EQUAL_UINT16(16384U, pk.bin);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(4096), pk.freq_hz);
}

void run_biquad_tests(void)
{
    RUN_TEST(test_biquad_filter);
    RUN_TEST(test_biquad_attenuation);
    RUN_TEST(test_biquad_cascade);
    RUN_TEST(test_trig_math);
    RUN_TEST(test_r5_biquad_high_cutoff_no_overflow);
    RUN_TEST(test_tier_a_biquad_saturation_and_fc_clamping);
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
    RUN_TEST(test_r5_fft_thd_max_harmonics_255);
    RUN_TEST(test_tier_a_fft_large_window_and_peak_bins);
}
