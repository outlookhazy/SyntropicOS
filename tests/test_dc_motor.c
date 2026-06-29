/**
 * @file test_dc_motor.c
 * @brief Unity tests for syn_dc_motor.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_dc_motor.h"

static void test_dc_motor(void)
{

    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 3, 4, SYN_DC_MODE_PWM_DIR);

    /* Set speed */
    syn_dc_motor_set_speed(&motor, 75);
    TEST_ASSERT_EQUAL_INT(75, syn_dc_motor_get_speed(&motor));
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[4]);

    syn_dc_motor_set_speed(&motor, -50);
    TEST_ASSERT_EQUAL_INT(-50, syn_dc_motor_get_speed(&motor));
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[4]);

    /* Clamping */
    syn_dc_motor_set_speed(&motor, 200);
    TEST_ASSERT_EQUAL_INT(100, syn_dc_motor_get_speed(&motor));

    /* Coast */
    syn_dc_motor_coast(&motor);
    TEST_ASSERT_EQUAL_INT(0, syn_dc_motor_get_speed(&motor));
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[3]);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[4]);

    /* Brake */
    syn_dc_motor_brake(&motor);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[3]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[4]);

    /* Ramp */
    mock_tick_ms = 0;
    syn_dc_motor_set_speed(&motor, 0);
    syn_dc_motor_ramp_to(&motor, 100, 100);
    TEST_ASSERT_FALSE(syn_dc_motor_at_target(&motor));

    mock_tick_advance(50);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) > 0);

    mock_tick_advance(60);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_at_target(&motor));
    TEST_ASSERT_EQUAL_INT(100, syn_dc_motor_get_speed(&motor));
}

void run_dc_motor_tests(void)
{
    RUN_TEST(test_dc_motor);
}
