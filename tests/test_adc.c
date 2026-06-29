/**
 * @file test_adc.c
 * @brief Unity tests for syn_adc.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/drivers/syn_adc.h"

static void test_adc(void)
{

    SYN_ADC adc;
    SYN_ADC_Config cfg = {
        .channel = 0,
        .oversample = 4,
        .filter = NULL,
        .filter_type = SYN_ADC_FILTER_NONE,
        .cal_offset = 0,
        .cal_scale = 1,
        .cal_scale_shift = 0,
    };

    mock_adc_value = 2048;
    syn_adc_init(&adc, &cfg);

    int32_t val = syn_adc_read(&adc);
    TEST_ASSERT_EQUAL_INT(2048, val);
    TEST_ASSERT_EQUAL_INT(2048, syn_adc_raw(&adc));

    /* mV conversion: 2048/4095 * 3300 ≈ 1649 */
    int32_t mv = syn_adc_read_mv(&adc);
    TEST_ASSERT_TRUE(mv >= 1640 && mv <= 1660);

    /* With calibration offset */
    syn_adc_set_calibration(&adc, 100, 1, 0);
    val = syn_adc_read(&adc);
    TEST_ASSERT_EQUAL_INT(2148, val);

    /* With scale */
    syn_adc_set_calibration(&adc, 0, 2048, 10);  /* × 2.0 */
    val = syn_adc_read(&adc);
    TEST_ASSERT_EQUAL_INT(4096, val);
}

void run_adc_tests(void)
{
    RUN_TEST(test_adc);
}
