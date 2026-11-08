/**
 * @file test_dds.c
 * @brief Unit test suite for Direct Digital Synthesis (DDS) Waveform Synthesizer (syn_dds).
 */

#include "syntropic/dsp/syn_dds.h"
#include "unity/unity.h"

#include <string.h>

void test_dds_sine_wave(void)
{
    SYN_DDS dds;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_init(&dds, SYN_DDS_SINE, 1000, 10000));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_set_gain(&dds, Q16_ONE, 0));

    q16_t sample0 = syn_dds_step(&dds);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FRAC(1, 10), 0, sample0);

    /* Generate quarter period (~90 deg peak -> ~1.0) */
    for (int i = 0; i < 10; i++)
        syn_dds_step(&dds);
    q16_t sample_peak = syn_dds_step(&dds);
    TEST_ASSERT_TRUE(sample_peak != 0);
}

void test_dds_triangle_and_sawtooth(void)
{
    SYN_DDS dds;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_init(&dds, SYN_DDS_TRIANGLE, 100, 1000));

    q16_t s1 = syn_dds_step(&dds);
    q16_t s2 = syn_dds_step(&dds);
    TEST_ASSERT_TRUE(s2 > s1); /* Ramping up */

    /* Step past 180 deg to hit ramp down */
    for (int i = 0; i < 10; i++)
        syn_dds_step(&dds);

    /* Sawtooth */
    syn_dds_init(&dds, SYN_DDS_SAWTOOTH, 100, 1000);
    q16_t saw1 = syn_dds_step(&dds);
    q16_t saw2 = syn_dds_step(&dds);
    TEST_ASSERT_TRUE(saw2 > saw1);
}

void test_dds_square_pwm_and_noise(void)
{
    SYN_DDS dds;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_init(&dds, SYN_DDS_SQUARE, 100, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_set_duty(&dds, 25.0f));

    q16_t sq1 = syn_dds_step(&dds);
    TEST_ASSERT_EQUAL(Q16_ONE, sq1);

    /* Step past duty cycle threshold to hit low phase */
    for (int i = 0; i < 10; i++)
        syn_dds_step(&dds);

    /* Noise */
    syn_dds_init(&dds, SYN_DDS_NOISE, 100, 1000);
    q16_t n1 = syn_dds_step(&dds);
    q16_t n2 = syn_dds_step(&dds);
    TEST_ASSERT_NOT_EQUAL(n1, n2);
}

void test_dds_buffer_fill(void)
{
    SYN_DDS dds;
    syn_dds_init(&dds, SYN_DDS_SINE, 440, 44100);

    q16_t qbuf[16];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_fill_q16(&dds, qbuf, 16));

    uint16_t dac_buf[16];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_fill_u16(&dds, dac_buf, 16, 2048, 2047));
    TEST_ASSERT_TRUE(dac_buf[0] > 0 && dac_buf[0] < 4096);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_fill_q16(NULL, qbuf, 16));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_fill_u16(NULL, dac_buf, 16, 2048, 2047));

    /* Duty cycle boundary clamping */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_set_duty(&dds, -10.0f));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_set_duty(&dds, 150.0f));

    /* Gain & null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_init(NULL, SYN_DDS_SINE, 100, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_init(&dds, SYN_DDS_SINE, 100, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_set_freq(NULL, 100, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_set_freq(&dds, 100, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_set_gain(NULL, 1, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dds_set_duty(NULL, 50.0f));
    TEST_ASSERT_EQUAL_INT(0, syn_dds_step(NULL));

    /* DAC buffer clamping under high gain */
    syn_dds_init(&dds, SYN_DDS_SAWTOOTH, 100, 1000);
    syn_dds_set_gain(&dds, Q16_FROM_INT(100), Q16_FROM_INT(50));
    syn_dds_fill_u16(&dds, dac_buf, 16, 2048, 2047);

    /* Invalid waveform default branch returns offset Q16 (50 << 16 = 3276800) */
    dds.type = (SYN_DDS_Waveform)999;
    TEST_ASSERT_EQUAL(3276800, syn_dds_step(&dds));
}

static void test_r5_dds_deterministic_xorshift_noise(void)
{
    SYN_DDS d1;
    SYN_DDS d2;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_init(&d1, SYN_DDS_NOISE, 1000U, 10000U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dds_init(&d2, SYN_DDS_NOISE, 1000U, 10000U));

    /* Per-instance rand_state produces reproducible independent sequences */
    for (int i = 0; i < 16; i++) {
        q16_t s1 = syn_dds_step(&d1);
        q16_t s2 = syn_dds_step(&d2);
        TEST_ASSERT_EQUAL_INT32(s1, s2);
    }
    TEST_ASSERT_NOT_EQUAL(0x12345678U, d1.rand_state);

    /* Zero seed fallback avoids xorshift32 lockup */
    d1.rand_state = 0U;
    q16_t z1 = syn_dds_step(&d1);
    q16_t z2 = syn_dds_step(&d1);
    TEST_ASSERT_NOT_EQUAL(0U, d1.rand_state);
    TEST_ASSERT_NOT_EQUAL(z1, z2);
}

void run_dds_tests(void)
{
    RUN_TEST(test_dds_sine_wave);
    RUN_TEST(test_dds_triangle_and_sawtooth);
    RUN_TEST(test_dds_square_pwm_and_noise);
    RUN_TEST(test_dds_buffer_fill);
    RUN_TEST(test_r5_dds_deterministic_xorshift_noise);
}
