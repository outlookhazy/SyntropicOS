/**
 * @file test_stepper.c
 * @brief Unity tests for syn_stepper.
 */

#include "mocks/mock_port.h"
#include "syntropic/motor/syn_stepper.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static void test_stepper(void)
{
    mock_tick_ms = 0;

    SYN_Stepper stepper;
    syn_stepper_init(&stepper, 5, 6);
    syn_stepper_set_speed(&stepper, 100, 1000);

    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));
    TEST_ASSERT_EQUAL_INT(0, syn_stepper_position(&stepper));

    /* Move 10 steps forward */
    syn_stepper_move(&stepper, 10);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&stepper));

    /* Tick until complete */
    int ticks = 0;
    while (syn_stepper_is_moving(&stepper) && ticks < 10000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));
    TEST_ASSERT_EQUAL_INT(10, syn_stepper_position(&stepper));

    /* Move to absolute position */
    syn_stepper_move_to(&stepper, 0);
    while (syn_stepper_is_moving(&stepper) && ticks < 20000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_EQUAL_INT(0, syn_stepper_position(&stepper));

    /* Emergency stop */
    syn_stepper_move(&stepper, 1000);
    mock_tick_advance(5);
    syn_stepper_tick(&stepper);
    syn_stepper_stop(&stepper);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));

    /* Set position */
    syn_stepper_set_position(&stepper, 500);
    TEST_ASSERT_EQUAL_INT(500, syn_stepper_position(&stepper));
}

/** syn_stepper_set_enable_pin + syn_stepper_enable — lines 51-57, 194-203 */
static void test_stepper_enable_pin(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);

    /* Configure enable pin (active_low=false) and toggle */
    syn_stepper_set_enable_pin(&s, 7, false);
    syn_stepper_enable(&s, true);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[7]);
    syn_stepper_enable(&s, false);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[7]);

    /* Configure enable pin (active_low=true) — exercises line 28, 200-201 */
    syn_stepper_set_enable_pin(&s, 8, true);
    syn_stepper_enable(&s, true);
    /* active_low: enable=true → write LOW */
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[8]);
    syn_stepper_enable(&s, false);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[8]);
}

/** syn_stepper_tick while IDLE — exercises case SYN_STEPPER_IDLE (line 145) */
static void test_stepper_tick_idle(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);

    /* Tick before any move — should be a no-op */
    syn_stepper_tick(&s);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    TEST_ASSERT_EQUAL_INT(0, syn_stepper_position(&s));
}

/** Short move with high accel — accel_steps clamped to abs_steps/2 (line 93) */
static void test_stepper_accel_clamp(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    /* High accel relative to speed: v²/(2a) would exceed half of 3 steps */
    syn_stepper_set_speed(&s, 1000, 10000); /* max_speed=1000 steps/s, accel=10000 steps/s² */
    syn_stepper_move(&s, 3); /* only 3 steps — accel_steps would be 50, clamped to 1 */
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&s));

    int ticks = 0;
    while (syn_stepper_is_moving(&s) && ticks < 50000) {
        mock_tick_advance(1);
        syn_stepper_tick(&s);
        ticks++;
    }
    TEST_ASSERT_EQUAL_INT(3, syn_stepper_position(&s));
}

/** dir_invert — exercises line 28 (direction flip) */
static void test_stepper_dir_invert(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);
    s.dir_invert = true; /* force direction inversion */

    /* Move forward — set_direction called with forward=true but inverted */
    syn_stepper_move(&s, 5);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&s));

    int ticks = 0;
    while (syn_stepper_is_moving(&s) && ticks < 50000) {
        mock_tick_advance(1);
        syn_stepper_tick(&s);
        ticks++;
    }
    /* position should still be +5 regardless of pin direction */
    TEST_ASSERT_EQUAL_INT(5, syn_stepper_position(&s));
}

static void test_stepper_zero_move(void)
{
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);
    syn_stepper_move(&s, 0);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));

    /* Enable with pin == -1 (no-op) */
    syn_stepper_enable(&s, true);
}

static void test_stepper_reverse_move(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);

    syn_stepper_move(&s, -10);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&s));

    int ticks = 0;
    while (syn_stepper_is_moving(&s) && ticks < 10000) {
        mock_tick_advance(1);
        syn_stepper_tick(&s);
        ticks++;
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    TEST_ASSERT_EQUAL_INT(-10, syn_stepper_position(&s));
}

static void test_stepper_motor_output_adapter(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 100, 1000);

    SYN_MotorOutput out = syn_stepper_output(&s);
    TEST_ASSERT_NOT_NULL(out.set_output);
    TEST_ASSERT_NOT_NULL(out.coast);
    TEST_ASSERT_NOT_NULL(out.brake);

    /* Start move */
    syn_stepper_move(&s, 5);
    out.set_output(out.ctx, 100);

    out.coast(out.ctx);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));

    syn_stepper_move(&s, 5);
    out.brake(out.ctx);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
}

static void test_r5_stepper_zero_accel_and_incremental_dt(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);

    /* Move before set_speed (accel == 0) must safely no-op without divide-by-zero */
    syn_stepper_move(&s, 10);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    syn_stepper_move_to(&s, 25);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));

    /* Corrupt accel = 0 while moving transitions cleanly to IDLE */
    syn_stepper_set_speed(&s, 2000, 500);
    syn_stepper_move(&s, 10);
    s.accel = 0U;
    syn_stepper_tick(&s);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));

    /* Incremental dt integration: 100 sps^2 over ten 1ms ticks yields 1 sps (not quadratic sum) */
    syn_stepper_set_speed(&s, 2000, 100);
    syn_stepper_move(&s, 4);
    for (int i = 0; i < 10; i++) {
        mock_tick_advance(1);
        syn_stepper_tick(&s);
    }
    TEST_ASSERT_EQUAL_UINT32(1U, s.speed);

    /* High max_speed (> 1000 sps -> interval_ms == 0 clamped to 1ms) + CRUISE state transition */
    syn_stepper_set_speed(&s, 2000, 50000);
    syn_stepper_move(&s, 500);
    mock_tick_advance(50);
    syn_stepper_tick(&s);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_STEPPER_CRUISE, s.state);
    TEST_ASSERT_EQUAL_UINT32(2000U, s.speed);
    mock_tick_advance(1);
    syn_stepper_tick(&s);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_STEPPER_CRUISE, s.state);
    syn_stepper_stop(&s);
    syn_stepper_set_position(&s, 0);

    syn_stepper_move_to(&s, 6);
    int ticks = 0;
    while (syn_stepper_is_moving(&s) && ticks < 500) {
        mock_tick_advance(1);
        syn_stepper_tick(&s);
        ticks++;
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    TEST_ASSERT_EQUAL_INT32(6, syn_stepper_position(&s));

    /* Large move_to clamping without signed overflow */
    syn_stepper_set_position(&s, 1500000000);
    syn_stepper_move_to(&s, -1500000000);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&s));
    syn_stepper_stop(&s);

    syn_stepper_set_position(&s, -1500000000);
    syn_stepper_move_to(&s, 1500000000);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&s));
    syn_stepper_stop(&s);

    /* Target saturation when relative move exceeds INT32_MAX / INT32_MIN */
    syn_stepper_set_position(&s, 1500000000);
    syn_stepper_move(&s, 1500000000);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, s.target);
    syn_stepper_stop(&s);

    syn_stepper_set_position(&s, -1500000000);
    syn_stepper_move(&s, -1500000000);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, s.target);
    syn_stepper_stop(&s);
}

static void test_stepper_rail_saturation_no_underflow_or_oscillation(void)
{
    mock_tick_ms = 0;
    SYN_Stepper s;
    syn_stepper_init(&s, 5, 6);
    syn_stepper_set_speed(&s, 1000, 10000);

    /* Move +10 from INT32_MAX - 2 must stop cleanly after 2 steps at INT32_MAX */
    syn_stepper_set_position(&s, INT32_MAX - 2);
    syn_stepper_move(&s, 10);
    TEST_ASSERT_EQUAL_INT32(2, s.steps_to_go);
    for (int i = 0; i < 50 && syn_stepper_is_moving(&s); i++) {
        mock_tick_advance(2);
        syn_stepper_tick(&s);
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_stepper_position(&s));

    /* Move -10 from INT32_MIN + 2 must stop cleanly after 2 steps at INT32_MIN without UB */
    syn_stepper_set_position(&s, INT32_MIN + 2);
    syn_stepper_move(&s, -10);
    TEST_ASSERT_EQUAL_INT32(2, s.steps_to_go);
    for (int i = 0; i < 50 && syn_stepper_is_moving(&s); i++) {
        mock_tick_advance(2);
        syn_stepper_tick(&s);
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, syn_stepper_position(&s));

    /* Already at INT32_MIN: further negative move is a safe no-op; INT32_MIN step from 0 clamps */
    syn_stepper_move(&s, -5);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&s));
    syn_stepper_set_position(&s, 0);
    syn_stepper_move(&s, INT32_MIN);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, s.steps_to_go);
    syn_stepper_stop(&s);
}

void run_stepper_tests(void)
{
    RUN_TEST(test_stepper);
    RUN_TEST(test_stepper_enable_pin);
    RUN_TEST(test_stepper_tick_idle);
    RUN_TEST(test_stepper_accel_clamp);
    RUN_TEST(test_stepper_dir_invert);
    RUN_TEST(test_stepper_zero_move);
    RUN_TEST(test_stepper_reverse_move);
    RUN_TEST(test_stepper_motor_output_adapter);
    RUN_TEST(test_r5_stepper_zero_accel_and_incremental_dt);
    RUN_TEST(test_stepper_rail_saturation_no_underflow_or_oscillation);
}
