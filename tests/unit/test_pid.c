/**
 * @file test_pid.c
 * @brief Unity tests for syn_pid.
 */

#include "mocks/mock_port.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static void test_pid(void)
{
    SYN_PID pid;
    SYN_PID_Config cfg = {
        .kp = 100,
        .ki = 10,
        .kd = 50,
        .scale = 100,
        .out_min = -1000,
        .out_max = 1000,
        .integral_max = 0,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid, &cfg);

    /* Proportional only (first call, I=0, D=0) */
    int32_t out = syn_pid_update(&pid, 100, 0, 10);
    TEST_ASSERT_EQUAL_INT(100, out);

    /* At setpoint → output drops to near zero */
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 50, 50, 10);
    TEST_ASSERT_EQUAL_INT(0, out);

    /* Output clamping */
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 5000, 0, 10);
    TEST_ASSERT_EQUAL_INT(1000, out);

    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, -5000, 0, 10);
    TEST_ASSERT_EQUAL_INT(-1000, out);

    /* Set gains at runtime */
    syn_pid_set_gains(&pid, 200, 0, 0);
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 100, 0, 10);
    TEST_ASSERT_EQUAL_INT(200, out);

    /* PID output getter */
    TEST_ASSERT_EQUAL(out, syn_pid_output(&pid));
}

static void test_pid_edge_cases(void)
{
    /* 1. Limits setter (syn_pid_set_limits) */
    SYN_PID pid;
    SYN_PID_Config cfg = {
        .kp = 100,
        .ki = 10,
        .kd = 50,
        .scale = 100,
        .out_min = -1000,
        .out_max = 1000,
        .integral_max = 0,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid, &cfg);

    syn_pid_set_limits(&pid, -500, 500);
    TEST_ASSERT_EQUAL_INT(-500, pid.cfg.out_min);
    TEST_ASSERT_EQUAL_INT(500, pid.cfg.out_max);

    /* Test output is clamped to new limits */
    int32_t out = syn_pid_update(&pid, 5000, 0, 10);
    TEST_ASSERT_EQUAL_INT(500, out);

    out = syn_pid_update(&pid, -5000, 0, 10);
    TEST_ASSERT_EQUAL_INT(-500, out);

    /* 2. EMA filter bypass branch (d_filter_alpha >= 255) */
    syn_pid_reset(&pid);
    pid.cfg.d_filter_alpha = 255;
    out = syn_pid_update(&pid, 100, 0, 10); // first call
    (void)out;
    out = syn_pid_update(&pid, 200, 0, 10); // second call (derivative is active)
    TEST_ASSERT_EQUAL_INT(500, out);
    TEST_ASSERT_EQUAL_INT(0, pid.prev_d_filtered); // should not be updated since bypass

    /* 3. Anti-windup saturation branch coverage */
    SYN_PID pid_aw;
    SYN_PID_Config cfg_aw = {
        .kp = 100,
        .ki = 100,
        .kd = 0,
        .scale = 100,
        .out_min = -10,
        .out_max = 10,
        .integral_max = 50000,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid_aw, &cfg_aw);

    /* Force positive saturation using huge integral */
    pid_aw.integral = 5000; // i_term = (100 * 5000) / 100000 = 5.
    out = syn_pid_update(&pid_aw, 10, 0, 10);
    TEST_ASSERT_EQUAL_INT(10, out);
    TEST_ASSERT_EQUAL_INT(5000, pid_aw.integral); // frozen!

    /* Saturated at out_max (10), but error is negative (-10) */
    pid_aw.integral = 30000;
    out = syn_pid_update(&pid_aw, -10, 0, 10);
    TEST_ASSERT_EQUAL_INT(10, out);
    TEST_ASSERT_EQUAL_INT(29900, pid_aw.integral); // NOT frozen! Decreased from 30000 to 29900.

    /* Saturated at out_min (-10) but error is positive (10) */
    pid_aw.integral = -30000;
    out = syn_pid_update(&pid_aw, 10, 0, 10);
    TEST_ASSERT_EQUAL_INT(-10, out);
    TEST_ASSERT_EQUAL_INT(-29900, pid_aw.integral); // NOT frozen! Increased from -30000 to -29900.

    /* Verify active anti-windup freezing under negative saturation and negative error */
    pid_aw.integral = -5000;
    out = syn_pid_update(&pid_aw, -10, 0, 10);
    TEST_ASSERT_EQUAL_INT(-10, out);
    TEST_ASSERT_EQUAL_INT(-5000, pid_aw.integral); // frozen!

    /* 4. Zero Timing Delta (dt_ms = 0) */
    syn_pid_reset(&pid_aw);
    pid_aw.cfg.out_max = 1000;
    out = syn_pid_update(&pid_aw, 10, 0, 0);
    TEST_ASSERT_EQUAL_INT(10, out);
    TEST_ASSERT_EQUAL_INT(10, pid_aw.integral); // accumulated with dt = 1

    /* 5. Extreme negative 64-bit error clamping to INT32_MIN in pid_clamp_i64 */
    SYN_PID pid_ext;
    SYN_PID_Config cfg_ext = {
        .kp = 1000,
        .ki = 0,
        .kd = 0,
        .scale = 1,
        .out_min = INT32_MIN,
        .out_max = INT32_MAX,
        .integral_max = 1000,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid_ext, &cfg_ext);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, syn_pid_update(&pid_ext, INT32_MIN, INT32_MAX, 10));
}

static void test_pid_unsaturated_derivative_and_ema_filter(void)
{
    /* 1. Exact unsaturated P + I + D verification on measurement ramp vs setpoint step */
    SYN_PID pid;
    SYN_PID_Config cfg = {
        .kp = 100,
        .ki = 10,
        .kd = 50,
        .scale = 100,
        .out_min = -10000,
        .out_max = 10000,
        .integral_max = 0,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid, &cfg);

    /* Step 1 (first=true): err=100 -> P=100, I=1, D=0 => 101 */
    TEST_ASSERT_EQUAL_INT32(101, syn_pid_update(&pid, 100, 0, 100U));
    /* Step 2 (measurement rises to 20): err=80, d_raw=-200 -> P=80, I=1, D=-100 => -19 */
    TEST_ASSERT_EQUAL_INT32(-19, syn_pid_update(&pid, 100, 20, 100U));
    /* Step 3 (setpoint step to 200): err=180, d_raw=+1000 -> P=180, I=3, D=+500 => 683 */
    TEST_ASSERT_EQUAL_INT32(683, syn_pid_update(&pid, 200, 20, 100U));

    /* 2. Active EMA derivative filter (alpha = 128 => 50% smoothing per sample) */
    SYN_PID pid_ema;
    SYN_PID_Config cfg_ema = {
        .kp = 0,
        .ki = 0,
        .kd = 100,
        .scale = 100,
        .out_min = -10000,
        .out_max = 10000,
        .integral_max = 1000,
        .d_filter_alpha = 128U,
    };
    syn_pid_init(&pid_ema, &cfg_ema);

    TEST_ASSERT_EQUAL_INT32(0, syn_pid_update(&pid_ema, 0, 0, 100U));
    /* Step change err 0 -> 100 over 100 ms: d_raw = 1000, EMA = 500 */
    TEST_ASSERT_EQUAL_INT32(500, syn_pid_update(&pid_ema, 100, 0, 100U));
    /* Held error 100 -> 100: d_raw = 0, EMA decays 500 -> 250 -> 125 */
    TEST_ASSERT_EQUAL_INT32(250, syn_pid_update(&pid_ema, 100, 0, 100U));
    TEST_ASSERT_EQUAL_INT32(125, syn_pid_update(&pid_ema, 100, 0, 100U));
}

void run_pid_tests(void)
{
    RUN_TEST(test_pid);
    RUN_TEST(test_pid_edge_cases);
    RUN_TEST(test_pid_unsaturated_derivative_and_ema_filter);
}
