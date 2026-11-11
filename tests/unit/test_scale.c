/**
 * @file test_scale.c
 * @brief Unity tests for syn_scale module.
 */

#include "mocks/mock_port.h"
#include "syntropic/sensor/syn_scale.h"
#include "unity/unity.h"

static void test_scale_operations(void)
{
    mock_port_reset();
    SYN_Scale scale;

    SYN_Status st = syn_scale_init(&scale, 0, 1, SYN_SCALE_HX711);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    syn_scale_tare(&scale, 1000);
    syn_scale_set_calibration_factor(&scale, 400.0f);

    /* Feed raw ADC = 21000 (delta = 20000 -> 50g) */
    syn_scale_feed_adc(&scale, 21000);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, syn_scale_get_grams(&scale));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.05f, syn_scale_get_kg(&scale));

    /* NULL guards */
    syn_scale_tare(NULL, 0);
    syn_scale_set_calibration_factor(NULL, 1.0f);
    syn_scale_set_calibration_factor(&scale, 0.0f); /* Rejected by guard */
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, syn_scale_get_grams(&scale));
    scale.scale_factor = 0.0f; /* Force zero scale_factor */
    syn_scale_feed_adc(&scale, 21000);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, syn_scale_get_grams(&scale));
    syn_scale_feed_adc(NULL, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, syn_scale_get_grams(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, syn_scale_get_kg(NULL));
}

static void test_scale_signed_overflow_and_nonfinite_cal_guard(void)
{
    SYN_Scale scale;
    TEST_ASSERT_EQUAL(SYN_OK, syn_scale_init(&scale, 0, 1, SYN_SCALE_HX711));
    syn_scale_set_calibration_factor(&scale, 1000.0f);

    /* Reject NaN / Inf calibration factors */
    syn_scale_set_calibration_factor(&scale, 0.0f / 0.0f);
    syn_scale_set_calibration_factor(&scale, 1.0f / 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1000.0f, scale.scale_factor);

    /* Large opposite-sign ADC and tare must not wrap int32_t */
    syn_scale_tare(&scale, -8000000);
    syn_scale_feed_adc(&scale, 2147000000);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 2155000.0f, syn_scale_get_grams(&scale));
}

void run_scale_tests(void)
{
    RUN_TEST(test_scale_operations);
    RUN_TEST(test_scale_signed_overflow_and_nonfinite_cal_guard);
}
