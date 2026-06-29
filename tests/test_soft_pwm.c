/**
 * @file test_soft_pwm.c
 * @brief Unity tests for syn_soft_pwm.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/output/syn_soft_pwm.h"

static void test_soft_pwm(void)
{

    mock_gpio_states[2] = 0;

    SYN_SoftPWM pwm;
    syn_soft_pwm_init(&pwm, 2, 10); /* resolution = 10 steps */
    syn_soft_pwm_set_duty(&pwm, 3);  /* 30% duty */

    int on_count = 0;
    int i;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (mock_gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_EQUAL_INT(3, on_count);

    /* 0% duty */
    syn_soft_pwm_set_duty(&pwm, 0);
    on_count = 0;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (mock_gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_EQUAL_INT(0, on_count);

    /* 100% duty */
    syn_soft_pwm_set_duty(&pwm, 10);
    on_count = 0;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (mock_gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_EQUAL_INT(10, on_count);

    /* Percent API */
    syn_soft_pwm_set_percent(&pwm, 50);
    TEST_ASSERT_EQUAL_INT(5, syn_soft_pwm_get_duty(&pwm));
}

void run_soft_pwm_tests(void)
{
    RUN_TEST(test_soft_pwm);
}
