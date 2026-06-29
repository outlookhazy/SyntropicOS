/**
 * @file test_motor_ctrl.c
 * @brief Unity tests for syn_motor_ctrl.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_motor_ctrl.h"

static int32_t mock_ctrl_position = 0;

static int32_t mock_encoder_feedback(void *ctx)
{
    (void)ctx;
    return mock_ctrl_position;
}

/* Simulate pot reading for linear actuator */
static int32_t mock_pot_position = 2048;

static int32_t mock_pot_feedback(void *ctx)
{
    (void)ctx;
    return mock_pot_position;
}

static int mock_stall_count = 0;
static void mock_stall_cb(SYN_MotorCtrl *c, void *ctx)
{
    (void)c; (void)ctx;
    mock_stall_count++;
}

static int mock_target_count = 0;
static void mock_target_cb(SYN_MotorCtrl *c, void *ctx)
{
    (void)c; (void)ctx;
    mock_target_count++;
}

static void test_motor_ctrl(void)
{

    mock_tick_ms = 0;
    mock_ctrl_position = 0;
    mock_stall_count = 0;
    mock_target_count = 0;

    /* ── Test 1: Encoder-based velocity control ─────────────── */

    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.read_pos_ctx  = NULL;
    cfg.dc_motor      = &dc;
    cfg.pid_kp        = 100;
    cfg.pid_ki        = 10;
    cfg.pid_kd        = 5;
    cfg.pid_scale     = 6;
    cfg.update_hz     = 100;
    cfg.output_min    = -255;
    cfg.output_max    = 255;

    syn_motor_ctrl_init(&ctrl, &cfg);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_IDLE, syn_motor_ctrl_mode(&ctrl));

    /* Set velocity target */
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_VELOCITY, syn_motor_ctrl_mode(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_state(&ctrl));

    /* Simulate: feedback = 10 counts per update at 100Hz = 1000 cnt/s */
    mock_ctrl_position = 10;
    mock_tick_advance(10);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);
    TEST_ASSERT_EQUAL_INT(1000, syn_motor_ctrl_velocity(&ctrl));

    /* Stop */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL_INT(0, syn_motor_ctrl_output(&ctrl));

    /* ── Test 2: Pot-based position control (linear actuator) ── */

    mock_pot_position = 1000;

    SYN_MotorCtrl act;
    SYN_MotorCtrl_Config acfg;
    memset(&acfg, 0, sizeof(acfg));
    acfg.type              = SYN_MCTRL_DC;
    acfg.read_pos          = mock_pot_feedback;
    acfg.read_pos_ctx      = NULL;
    acfg.dc_motor          = &dc;
    acfg.pid_kp            = 50;
    acfg.pid_ki            = 5;
    acfg.pid_kd            = 10;
    acfg.pid_scale         = 6;
    acfg.update_hz         = 50;
    acfg.output_min        = -255;
    acfg.output_max        = 255;
    acfg.position_deadband = 10;
    acfg.position_min      = 100;
    acfg.position_max      = 3900;

    syn_motor_ctrl_init(&act, &acfg);
    syn_motor_ctrl_on_target(&act, mock_target_cb, NULL);

    /* Set position target */
    syn_motor_ctrl_set_position(&act, 2048);
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_POSITION, syn_motor_ctrl_mode(&act));

    /* Simulate: still at 1000, far from 2048 */
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);
    TEST_ASSERT_TRUE(syn_motor_ctrl_output(&act) > 0);

    /* Simulate: arrived at target */
    mock_pot_position = 2050;  /* within deadband of 10 */
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    TEST_ASSERT_EQUAL(SYN_MCTRL_ON_TARGET, st);
    TEST_ASSERT_EQUAL_INT(1, mock_target_count);

    /* ── Test 3: Soft position limits ───────────────────────── */

    mock_pot_position = 50;  /* below position_min (100) */
    syn_motor_ctrl_set_position(&act, 500);
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    /* PID wants positive output (moving up), which is allowed from below min */
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);

    /* Target beyond max gets clamped */
    syn_motor_ctrl_set_position(&act, 5000);
    TEST_ASSERT_EQUAL_INT(3900, act.target_position);

    /* ── Test 4: Stall detection ────────────────────────────── */

    mock_ctrl_position = 0;
    mock_stall_count = 0;

    SYN_MotorCtrl stctrl;
    SYN_MotorCtrl_Config scfg;
    memset(&scfg, 0, sizeof(scfg));
    scfg.type              = SYN_MCTRL_DC;
    scfg.read_pos          = mock_encoder_feedback;
    scfg.read_pos_ctx      = NULL;
    scfg.dc_motor          = &dc;
    scfg.pid_kp            = 200;
    scfg.pid_ki            = 0;
    scfg.pid_kd            = 0;
    scfg.pid_scale         = 6;
    scfg.update_hz         = 100;
    scfg.output_min        = -255;
    scfg.output_max        = 255;
    scfg.stall_timeout_ms  = 100;
    scfg.stall_threshold   = 0;

    syn_motor_ctrl_init(&stctrl, &scfg);
    syn_motor_ctrl_on_stall(&stctrl, mock_stall_cb, NULL);

    /* Set velocity but don't move feedback → stall */
    syn_motor_ctrl_set_velocity(&stctrl, 5000);
    mock_ctrl_position = 0; /* not moving! */

    /* Pump updates without moving */
    int i;
    for (i = 0; i < 12; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&stctrl);
    }
    /* After 120ms > stall_timeout_ms(100), should be stalled */
    TEST_ASSERT_EQUAL(SYN_MCTRL_STALLED, syn_motor_ctrl_state(&stctrl));
    TEST_ASSERT_EQUAL_INT(1, mock_stall_count);

    /* ── Test 5: E-stop ─────────────────────────────────────── */
    syn_motor_ctrl_estop(&stctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&stctrl));
}

void run_motor_ctrl_tests(void)
{
    RUN_TEST(test_motor_ctrl);
}
