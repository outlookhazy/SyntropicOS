/**
 * @file test_servo.c
 * @brief Unity tests for syn_servo.
 */

#include "mocks/mock_port.h"
#include "syntropic/motor/syn_servo.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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
    syn_servo_set_angle(&servo, 0);       /* start at 0° (1000µs) */
    syn_servo_move_to(&servo, 180, 1000); /* move to 180° over 1s */
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

/** duration=0 → immediate snap (lines 92-94) */
static void test_servo_move_immediate(void)
{
    mock_tick_ms = 0;
    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);
    syn_servo_set_angle(&servo, 0);

    /* duration=0 → current_us = target immediately, rate=0 */
    syn_servo_move_to(&servo, 180, 0);
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));
}

/** Very tiny delta and large duration → rate rounds to 0, clamped to ±1 (line 102) */
static void test_servo_rate_clamp(void)
{
    mock_tick_ms = 0;
    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);
    syn_servo_set_angle(&servo, 89); /* 1494µs */

    /* delta = 1µs, duration = 1000ms → rate = (1*256)/1000 = 0 → clamped to +1 */
    syn_servo_move_to(&servo, 90, 1000); /* 1-degree step over 1 second */
    TEST_ASSERT_FALSE(syn_servo_at_target(&servo));

    mock_tick_advance(50);
    syn_servo_update(&servo);
    /* Should have moved by at least 1µs */
    TEST_ASSERT_FALSE(syn_servo_at_target(&servo));
}

static void test_servo_edge_cases_and_nulls(void)
{
    SYN_Servo servo;
    memset(&servo, 0, sizeof(servo));
    syn_servo_init(NULL, 1000, 2000, 180);
    syn_servo_init(&servo, 2000, 1000, 180); /* invalid range */
    syn_servo_init(&servo, 1000, 2000, 0);   /* invalid range */

    syn_servo_init(&servo, 1000, 2000, 180);

    /* Out of bounds clamping */
    syn_servo_set_angle(&servo, 250); /* > 180 */
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));

    syn_servo_set_pulse(&servo, 500); /* < 1000 */
    TEST_ASSERT_EQUAL_INT(1000, syn_servo_get_pulse_us(&servo));

    syn_servo_set_pulse(&servo, 2500); /* > 2000 */
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));

    /* Check NULL getters */
    TEST_ASSERT_EQUAL_INT(0, syn_servo_get_pulse_us(NULL));
    TEST_ASSERT_EQUAL_INT(0, syn_servo_get_angle(NULL));
    TEST_ASSERT_TRUE(syn_servo_at_target(NULL));
}

static void test_servo_reverse_move_and_zero_dt(void)
{
    mock_tick_ms = 100;
    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);
    syn_servo_set_angle(&servo, 180); /* 2000µs */

    /* Move reverse from 180° to 0° over 1000ms */
    syn_servo_move_to(&servo, 0, 1000);
    TEST_ASSERT_FALSE(syn_servo_at_target(&servo));

    /* dt = 0 -> no update */
    syn_servo_update(&servo);
    TEST_ASSERT_EQUAL_INT(2000, syn_servo_get_pulse_us(&servo));

    /* Advance time to complete move */
    mock_tick_advance(1000);
    syn_servo_update(&servo);
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));
    TEST_ASSERT_EQUAL_INT(1000, syn_servo_get_pulse_us(&servo));
}

static void test_servo_negative_rate_clamp(void)
{
    mock_tick_ms = 0;
    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);
    syn_servo_set_angle(&servo, 90); /* 1500µs */

    /* Move reverse tiny step: 1500µs to 1499µs over 10000ms -> rate = -1 */
    syn_servo_move_to(&servo, 89, 10000);
    TEST_ASSERT_FALSE(syn_servo_at_target(&servo));

    mock_tick_advance(10);
    syn_servo_update(&servo);

    /* Null API safety checks */
    syn_servo_set_angle(NULL, 90);
    syn_servo_set_pulse(NULL, 1500);
    syn_servo_move_to(NULL, 90, 100);
}

static void test_servo_fast_rate_large_dt_no_overflow(void)
{
    mock_tick_ms = 0;
    SYN_Servo servo;
    syn_servo_init(&servo, 500, 2500, 180);
    syn_servo_set_angle(&servo, 0); /* 500 us */

    /* delta = 2000 us over 1 ms -> rate = 512000; dt = 8389 ms wraps 32-bit rate * dt to +784 us */
    syn_servo_move_to(&servo, 180, 1);
    mock_tick_advance(8389);
    syn_servo_update(&servo);
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));
    TEST_ASSERT_EQUAL_UINT16(2500U, syn_servo_get_pulse_us(&servo));
}

void run_servo_tests(void)
{
    RUN_TEST(test_servo);
    RUN_TEST(test_servo_move_immediate);
    RUN_TEST(test_servo_rate_clamp);
    RUN_TEST(test_servo_edge_cases_and_nulls);
    RUN_TEST(test_servo_reverse_move_and_zero_dt);
    RUN_TEST(test_servo_negative_rate_clamp);
    RUN_TEST(test_servo_fast_rate_large_dt_no_overflow);
}
