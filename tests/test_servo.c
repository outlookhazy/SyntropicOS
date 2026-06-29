/**
 * @file test_servo.c
 * @brief Unity tests for syn_servo.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_servo.h"

static void test_servo(void)
{

    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);

    /* Default is center */
    TEST_ASSERT_EQUAL_INT(1500, syn_servo_get_pulse_us(&servo));

    /* Set angle */
    syn_servo_set_angle(&servo, 0);
    TEST_ASSERT_EQUAL_INT(1000, syn_servo_get_pulse_us(&servo));

    syn_servo_set_angle(&servo, 180);
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));

    syn_servo_set_angle(&servo, 90);
    TEST_ASSERT_EQUAL_INT(1500, syn_servo_get_pulse_us(&servo));

    /* Get angle */
    TEST_ASSERT_EQUAL_INT(90, syn_servo_get_angle(&servo));

    /* Set raw pulse */
    syn_servo_set_pulse(&servo, 1250);
    TEST_ASSERT_EQUAL_INT(1250, syn_servo_get_pulse_us(&servo));

    /* At target (immediate moves) */
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));

    /* Smooth move */
    mock_tick_ms = 0;
    syn_servo_set_angle(&servo, 0);   /* start at 0° (1000µs) */
    syn_servo_move_to(&servo, 180, 1000);  /* move to 180° over 1s */
    TEST_ASSERT_FALSE(syn_servo_at_target(&servo));

    mock_tick_advance(500);
    syn_servo_update(&servo);
    uint16_t mid = syn_servo_get_pulse_us(&servo);
    /* Should be roughly halfway: ~1500µs */
    TEST_ASSERT_TRUE(mid >= 1400 && mid <= 1600);

    mock_tick_advance(600);
    syn_servo_update(&servo);
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));
}

void run_servo_tests(void)
{
    RUN_TEST(test_servo);
}
