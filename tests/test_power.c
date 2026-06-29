/**
 * @file test_power.c
 * @brief Unity tests for syn_power.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

#include "syntropic/system/syn_power.h"

static int power_bo_count = 0;
static int power_re_count = 0;
static void pwr_bo_cb(SYN_Power *p, void *c) { (void)p; (void)c; power_bo_count++; }
static void pwr_re_cb(SYN_Power *p, void *c) { (void)p; (void)c; power_re_count++; }

static void test_power(void)
{
    static SYN_ADC pwr_adc;
    SYN_ADC_Config pwr_adc_cfg = { .channel = 0, .oversample = 1, .cal_scale = 1 };
    mock_adc_value = 4095; /* ~3300mV */
    syn_adc_init(&pwr_adc, &pwr_adc_cfg);
    power_bo_count = 0; power_re_count = 0;
    SYN_Power pwr;
    SYN_Power_Config pcfg = {
        .adc = &pwr_adc, .brownout_mv = 3000, .restore_mv = 3200,
        .on_brownout = pwr_bo_cb, .on_restore = pwr_re_cb,
    };
    syn_power_init(&pwr, &pcfg);
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(!syn_power_is_brownout(&pwr));
    mock_adc_value = 3600; /* ~2900mV */
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(syn_power_is_brownout(&pwr));
    TEST_ASSERT_EQUAL_INT(1, power_bo_count);
    syn_power_update(&pwr);
    TEST_ASSERT_EQUAL_INT(1, power_bo_count);
    mock_adc_value = 4095; /* ~3300mV */
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(!syn_power_is_brownout(&pwr));
    TEST_ASSERT_EQUAL_INT(1, power_re_count);
    SYN_Signal pwr_stats;
    int32_t pwr_samp[8];
    syn_signal_init(&pwr_stats, pwr_samp, 8);
    syn_power_set_stats(&pwr, &pwr_stats);
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(syn_signal_count(&pwr_stats) == 1);
}

void run_power_tests(void)
{
    RUN_TEST(test_power);
}
