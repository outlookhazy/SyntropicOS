/**
 * @file test_pid.c
 * @brief Unity tests for syn_pid.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/control/syn_pid.h"

static void test_pid(void)
{

    SYN_PID pid;
    SYN_PID_Config cfg = {
        .kp = 100, .ki = 10, .kd = 50,
        .scale = 100,
        .out_min = -1000, .out_max = 1000,
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

void run_pid_tests(void)
{
    RUN_TEST(test_pid);
}
