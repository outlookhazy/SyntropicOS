/**
 * @file test_actuator.c
 * @brief Unity tests for syn_actuator.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

#include "syntropic/motor/syn_actuator.h"

static int32_t act_mock_pos = 500;
static int32_t act_read(void *c) { (void)c; return act_mock_pos; }

static void test_actuator(void)
{
    static SYN_DCMotor act_motor;
    syn_dc_motor_init(&act_motor, 10, 11, SYN_DC_MODE_PWM_DIR);
    act_mock_pos = 500;
    SYN_Actuator act;
    SYN_Actuator_Config acfg = {
        .dc_motor = &act_motor, .read_pos = act_read, .read_ctx = NULL,
        .stroke_min = 100, .stroke_max = 900, .update_hz = 50,
        .pid_kp = 100, .pid_scale = 4,
    };
    syn_actuator_init(&act, &acfg);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 500);
    syn_actuator_set_position(&act, 750);
    act_mock_pos = 700;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 750);
    syn_actuator_stop(&act);
    TEST_ASSERT_TRUE(act.ctrl.state == SYN_MCTRL_STOPPED);
    syn_actuator_set_position(&act, 0);
    act_mock_pos = 100;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 0);
    syn_actuator_set_position(&act, 1000);
    act_mock_pos = 900;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 1000);
    syn_actuator_set_position(&act, 2000);
    TEST_ASSERT_TRUE(act.target_pct == 1000);
    syn_actuator_set_position(&act, -500);
    TEST_ASSERT_TRUE(act.target_pct == 0);
}

void run_actuator_tests(void)
{
    RUN_TEST(test_actuator);
}
