/**
 * @file test_dc_motor.c
 * @brief Unity tests for syn_dc_motor — full coverage.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_dc_motor.h"

/* ── Duty callback capture ──────────────────────────────────────────────── */

static uint16_t last_duty_pin;
static uint16_t last_duty_val;
static int      duty_call_count;

static void duty_cb(SYN_GPIO_Pin pin, uint16_t duty, void *ctx)
{
    (void)ctx;
    last_duty_pin = pin;
    last_duty_val = duty;
    duty_call_count++;
}

static void duty_reset(void)
{
    last_duty_pin  = 0xFF;
    last_duty_val  = 0xFF;
    duty_call_count = 0;
}

/* ── Test: PWM_DIR mode — original tests preserved ──────────────────────── */

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

    /* Clamping (duty_max defaults to 1000) */
    syn_dc_motor_set_speed(&motor, 2000);
    TEST_ASSERT_EQUAL_INT(1000, syn_dc_motor_get_speed(&motor));

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
    syn_dc_motor_ramp_to(&motor, 1000, 100);
    TEST_ASSERT_FALSE(syn_dc_motor_at_target(&motor));

    mock_tick_advance(50);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) > 0);

    mock_tick_advance(60);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_at_target(&motor));
    TEST_ASSERT_EQUAL_INT(1000, syn_dc_motor_get_speed(&motor));
}

/* ── Test: PWM_DIR mode with duty callback ──────────────────────────────── */

static void test_dc_motor_pwm_dir_with_callback(void)
{
    duty_reset();

    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_duty_callback(&motor, duty_cb, NULL);

    /* Forward speed — callback should be called with duty=75 on pin_a */
    syn_dc_motor_set_speed(&motor, 75);
    TEST_ASSERT_TRUE(duty_call_count > 0);
    TEST_ASSERT_EQUAL_INT(1, last_duty_pin);
    TEST_ASSERT_EQUAL_INT(75, last_duty_val);

    /* Verify direction pin */
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[2]); /* pin_b = dir HIGH for forward */

    /* Reverse speed */
    duty_reset();
    syn_dc_motor_set_speed(&motor, -60);
    TEST_ASSERT_TRUE(duty_call_count > 0);
    TEST_ASSERT_EQUAL_INT(1, last_duty_pin);
    TEST_ASSERT_EQUAL_INT(60, last_duty_val);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[2]); /* dir LOW for reverse */
}

/* ── Test: DUAL_PWM mode — forward, no callback ─────────────────────────── */

static void test_dc_motor_dual_pwm_no_callback(void)
{
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 5, 6, SYN_DC_MODE_DUAL_PWM);

    /* Forward — pin_a HIGH, pin_b LOW */
    syn_dc_motor_set_speed(&motor, 80);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[5]); /* pin_a HIGH */
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW,  mock_gpio_states[6]); /* pin_b LOW */

    /* Reverse — pin_a LOW, pin_b HIGH */
    syn_dc_motor_set_speed(&motor, -80);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW,  mock_gpio_states[5]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[6]);

    /* Zero speed forward — pin_a LOW (duty=0), pin_b LOW */
    syn_dc_motor_set_speed(&motor, 0);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[5]);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[6]);
}

/* ── Test: DUAL_PWM mode — with duty callback ───────────────────────────── */

static void test_dc_motor_dual_pwm_with_callback(void)
{
    duty_reset();

    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 7, 8, SYN_DC_MODE_DUAL_PWM);
    syn_dc_motor_set_duty_callback(&motor, duty_cb, NULL);

    /* Forward: pin_a gets duty, pin_b gets 0 */
    duty_reset();
    syn_dc_motor_set_speed(&motor, 50);
    /* duty_cb should be called twice: once for pin_a=50, once for pin_b=0 */
    TEST_ASSERT_EQUAL_INT(2, duty_call_count);

    /* Reverse: pin_a gets 0, pin_b gets duty */
    duty_reset();
    syn_dc_motor_set_speed(&motor, -50);
    TEST_ASSERT_EQUAL_INT(2, duty_call_count);
    /* Last call was for pin_b with duty=50 */
    TEST_ASSERT_EQUAL_INT(8, last_duty_pin);
    TEST_ASSERT_EQUAL_INT(50, last_duty_val);
}

/* ── Test: invert flag ───────────────────────────────────────────────────── */

static void test_dc_motor_invert(void)
{
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 9, 10, SYN_DC_MODE_PWM_DIR);
    motor.invert = true;

    /* Positive speed + invert → reverse direction */
    syn_dc_motor_set_speed(&motor, 50);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[10]); /* dir LOW = reverse */

    motor.invert = false;
    syn_dc_motor_set_speed(&motor, 50);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[10]); /* dir HIGH = forward */
}

/* ── Test: ramp_to with duration==0 (instant) ───────────────────────────── */

static void test_dc_motor_ramp_instant(void)
{
    mock_tick_ms = 0;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 0);

    /* duration==0 → instant speed change */
    syn_dc_motor_ramp_to(&motor, 800, 0);
    TEST_ASSERT_EQUAL_INT(800, syn_dc_motor_get_speed(&motor));
    TEST_ASSERT_TRUE(syn_dc_motor_at_target(&motor));
    TEST_ASSERT_EQUAL_INT(0, motor.ramp_rate);
}

/* ── Test: ramp_to with tiny delta (rate rounds to 0) ───────────────────── */

static void test_dc_motor_ramp_tiny_delta(void)
{
    mock_tick_ms = 0;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 50);

    /* delta=1 over 1000ms → ramp_rate = (1*256)/1000 = 0, should become +1 */
    syn_dc_motor_ramp_to(&motor, 51, 1000);
    TEST_ASSERT_EQUAL_INT(1, motor.ramp_rate); /* minimum positive rate */

    /* Negative tiny delta */
    syn_dc_motor_set_speed(&motor, 50);
    syn_dc_motor_ramp_to(&motor, 49, 1000);
    TEST_ASSERT_EQUAL_INT(-1, motor.ramp_rate); /* minimum negative rate */
}

/* ── Test: update with dt==0 (no tick advance) ──────────────────────────── */

static void test_dc_motor_update_dt_zero(void)
{
    mock_tick_ms = 500;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 0);
    syn_dc_motor_ramp_to(&motor, 100, 100);

    /* Don't advance tick — dt==0, update should return early */
    int16_t speed_before = syn_dc_motor_get_speed(&motor);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_EQUAL_INT(speed_before, syn_dc_motor_get_speed(&motor));
}

/* ── Test: update with delta==0 (small dt, ramp_rate gives 0 increment) ── */

static void test_dc_motor_update_small_dt(void)
{
    mock_tick_ms = 0;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 0);

    /* ramp_rate = (100*256)/10000 = 2 (Q8 fixed); dt=1 → delta = (2*1)/256 = 0 */
    /* When delta==0, should use sign of ramp_rate (+1 or -1) */
    syn_dc_motor_ramp_to(&motor, 100, 10000);
    TEST_ASSERT_TRUE(motor.ramp_rate != 0);

    mock_tick_advance(1);
    syn_dc_motor_update(&motor);
    /* Speed should have moved by 1 at minimum */
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) > 0);
}

/* ── Test: update when already at target (early return) ────────────────── */

static void test_dc_motor_update_at_target(void)
{
    mock_tick_ms = 0;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 50);
    /* No ramp — speed==target */
    mock_tick_advance(10);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_EQUAL_INT(50, syn_dc_motor_get_speed(&motor)); /* no change */
}

/* ── Test: negative ramp (downward) ────────────────────────────────────── */

static void test_dc_motor_ramp_downward(void)
{
    mock_tick_ms = 0;
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, 1000);
    syn_dc_motor_ramp_to(&motor, 0, 100);
    TEST_ASSERT_TRUE(motor.ramp_rate < 0);

    mock_tick_advance(50);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) < 1000);

    mock_tick_advance(60);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_at_target(&motor));
    TEST_ASSERT_EQUAL_INT(0, syn_dc_motor_get_speed(&motor));
}

/* ── Test: clamping negative speeds ────────────────────────────────────── */

static void test_dc_motor_clamp_negative(void)
{
    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_PWM_DIR);
    syn_dc_motor_set_speed(&motor, -2000);
    TEST_ASSERT_EQUAL_INT(-1000, syn_dc_motor_get_speed(&motor));

    /* Dual PWM without callback reverse */
    syn_dc_motor_init(&motor, 1, 2, SYN_DC_MODE_DUAL_PWM);
    syn_dc_motor_set_speed(&motor, -500);
    TEST_ASSERT_EQUAL_INT(-500, syn_dc_motor_get_speed(&motor));
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[1]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[2]);
}

/* ── Test runner ─────────────────────────────────────────────────────── */

void run_dc_motor_tests(void)
{
    RUN_TEST(test_dc_motor);
    RUN_TEST(test_dc_motor_pwm_dir_with_callback);
    RUN_TEST(test_dc_motor_dual_pwm_no_callback);
    RUN_TEST(test_dc_motor_dual_pwm_with_callback);
    RUN_TEST(test_dc_motor_invert);
    RUN_TEST(test_dc_motor_ramp_instant);
    RUN_TEST(test_dc_motor_ramp_tiny_delta);
    RUN_TEST(test_dc_motor_update_dt_zero);
    RUN_TEST(test_dc_motor_update_small_dt);
    RUN_TEST(test_dc_motor_update_at_target);
    RUN_TEST(test_dc_motor_ramp_downward);
    RUN_TEST(test_dc_motor_clamp_negative);
}
