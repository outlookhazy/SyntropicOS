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

static void test_motor_ctrl_stepper(void)
{
    SYN_Stepper stepper;
    syn_stepper_init(&stepper, 5, 6);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type     = SYN_MCTRL_STEPPER;
    cfg.stepper  = &stepper;
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 100;

    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_velocity(&ctrl, 500);

    /* Test stepper output update */
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);

    /* Test stepper stop */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));

    /* Test stepper estop */
    syn_motor_ctrl_set_velocity(&ctrl, 500);
    syn_motor_ctrl_estop(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));

    /* Test NULL stepper pointer returns safely */
    cfg.stepper = NULL;
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_velocity(&ctrl, 500);
    st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);
    syn_motor_ctrl_stop(&ctrl);
    syn_motor_ctrl_estop(&ctrl);

    /* Test DC type with NULL motor pointer returns safely */
    cfg.type = SYN_MCTRL_DC;
    cfg.dc_motor = NULL;
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_velocity(&ctrl, 500);
    st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);
    syn_motor_ctrl_stop(&ctrl);
    syn_motor_ctrl_estop(&ctrl);
}

static void test_motor_ctrl_trajectory(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.dc_motor      = &dc;
    cfg.update_hz     = 100;
    cfg.output_min    = -100;
    cfg.output_max    = 100;
    cfg.ff_kv         = 10;
    cfg.ff_ka         = 5;
    cfg.ff_scale      = 3; // division by 8

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. Test set_trajectory transitions when idle */
    SYN_MotorCtrl_Trajectory traj = {
        .position = 1000,
        .velocity = 40,
        .acceleration = 16
    };
    syn_motor_ctrl_set_gains(&ctrl, 0, 0, 0); // No PID output, focus on FF
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_POSITION, ctrl.mode);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, ctrl.state);
    TEST_ASSERT_TRUE(ctrl.trajectory_active);

    /* Update: ff should be (10 * 40 + 5 * 16) / 8 = (400 + 80) / 8 = 60 */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(60, ctrl.ff_output);

    /* 2. Call set_trajectory when already in POSITION mode & RUNNING */
    traj.velocity = 80;
    traj.acceleration = 32;
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    /* ff should be (10 * 80 + 5 * 32) / 8 = (800 + 160) / 8 = 120. But clamped to 90! */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(90, ctrl.ff_output);

    /* 3. Test negative clamp of FF */
    traj.velocity = -80;
    traj.acceleration = -32;
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    /* ff should be (10 * -80 + 5 * -32) / 8 = -120. Clamped to -90! */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(-90, ctrl.ff_output);

    /* 4. Test ff_scale <= 0 */
    cfg.ff_scale = 0;
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_gains(&ctrl, 0, 0, 0);
    traj.velocity = 5;
    traj.acceleration = 2;
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    /* ff should be (10 * 5 + 5 * 2) / 1 = 60 */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(60, ctrl.ff_output);
}

static void test_motor_ctrl_saturation(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.dc_motor      = &dc;
    cfg.update_hz     = 100;
    cfg.output_min    = -100;
    cfg.output_max    = 100;
    /* kp = 0, ki = 300 so that integral builds up and clamps.
     * integral_max = (output_max * scale * 1000) / ki = (100 * 1 * 1000) / 300 = 333 */
    cfg.pid_kp        = 0;
    cfg.pid_ki        = 300; 
    cfg.pid_scale     = 0; // scale = 1

    /* 1. Test positive saturation via integral_max clamping */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, 10); // target = 10, error = 10, step = 100
    mock_ctrl_position = 0;
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 100
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 200
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 300
    TEST_ASSERT_EQUAL_INT(300, ctrl.pid.integral);

    /* Update again: integral wants 400, clamped to integral_max (333) */
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(333, ctrl.pid.integral);

    /* 2. Test negative saturation via integral_max clamping */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, -10);
    mock_ctrl_position = 0;
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -100
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -200
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -300
    TEST_ASSERT_EQUAL_INT(-300, ctrl.pid.integral);

    /* Update again: integral wants -400, clamped to -integral_max (-333) */
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(-333, ctrl.pid.integral);

    /* 3. Test combined output saturation and anti-windup clamping with non-zero feedforward */
    /* 3a. Positive feedforward saturation (ff > 0), triggers line 356 */
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.dc_motor      = &dc;
    cfg.update_hz     = 100;
    cfg.output_min    = -100;
    cfg.output_max    = 100;
    cfg.ff_kv         = 10;
    cfg.ff_scale      = 0;
    cfg.pid_kp        = 0;
    cfg.pid_ki        = 300;
    cfg.pid_scale     = 0;

    syn_motor_ctrl_init(&ctrl, &cfg);
    SYN_MotorCtrl_Trajectory traj = {
        .position = 10,
        .velocity = 2, // ff = 20
        .acceleration = 0
    };
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 100
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 200
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = 300, pid_out = 90 > max_pid (80), takes line 356 subtraction
    TEST_ASSERT_TRUE(ctrl.pid.integral < 300);

    /* 3b. Negative feedforward saturation (ff < 0), triggers line 359 */
    syn_motor_ctrl_init(&ctrl, &cfg);
    traj.position = -10;
    traj.velocity = -2; // ff = -20
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -100
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -200
    mock_tick_advance(10); syn_motor_ctrl_update(&ctrl); // integral = -300, pid_out = -90 < min_pid (-80), takes line 359 subtraction
    TEST_ASSERT_TRUE(ctrl.pid.integral > -300);

    /* 3c. Test integer underflow in ff to swap max_pid and min_pid, and huge negative ff to trigger line 345-346 */
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.dc_motor      = &dc;
    cfg.update_hz     = 100;
    cfg.output_min    = -1000;
    cfg.output_max    = 1000;
    cfg.ff_kv         = 214748360; // will multiply by 10/(-10) below to make +/-2147483600
    cfg.ff_scale      = 0;
    cfg.pid_kp        = 10;
    cfg.pid_ki        = 1000;
    cfg.pid_scale     = 0;

    syn_motor_ctrl_init(&ctrl, &cfg);
    traj.position = 100;
    traj.velocity = 10; // ff = 2147483600
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // positive underflow swap

    syn_motor_ctrl_init(&ctrl, &cfg);
    traj.position = -100;
    traj.velocity = -10; // ff = -2147483600
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // negative underflow swap + triggers lines 345-346!
}

static void test_motor_ctrl_datalog(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.dc_motor      = &dc;
    cfg.update_hz     = 100;
    cfg.output_min    = -100;
    cfg.output_max    = 100;

    syn_motor_ctrl_init(&ctrl, &cfg);

    uint8_t backing[256];
    SYN_DataLog datalog;
    syn_datalog_init(&datalog, backing, sizeof(backing));
    syn_motor_ctrl_set_datalog(&ctrl, &datalog);
    TEST_ASSERT_EQUAL_PTR(&datalog, ctrl.datalog);

    /* Update the controller; it should write a sample to the datalog */
    syn_motor_ctrl_set_velocity(&ctrl, 500);
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);

    /* Check that bytes were written to the datalog ringbuffer */
    TEST_ASSERT_TRUE(syn_ringbuf_count(&datalog.rb) > 0);
}

static void test_motor_ctrl_metrics(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type              = SYN_MCTRL_DC;
    cfg.read_pos          = mock_encoder_feedback;
    cfg.dc_motor          = &dc;
    cfg.update_hz         = 100;
    cfg.output_min        = -100;
    cfg.output_max        = 100;
    cfg.position_deadband = 10;

    mock_tick_ms = 1000;
    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. RMS error when sample count is 0 */
    TEST_ASSERT_EQUAL_INT(0, syn_motor_ctrl_rms_error(&ctrl));

    /* 2. Reset metrics */
    syn_motor_ctrl_reset_metrics(&ctrl);
    TEST_ASSERT_TRUE(ctrl.metrics.move_start_tick > 0);

    /* 3. Run updates to accumulate metrics, overshoot, and settle tick */
    syn_motor_ctrl_set_position(&ctrl, 100);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // max_error = 100, error_sq_sum = 10000

    /* overshoot requires abs_err < max_error, and crossing target */
    mock_ctrl_position = 120; // error is -20, beyond is 20
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // overshoot should become 20
    TEST_ASSERT_EQUAL_INT(20, ctrl.metrics.overshoot);

    /* settle_tick requires abs_err <= deadband. bypass early return via trajectory_active */
    mock_ctrl_position = 95; // error is 5 <= deadband(10)
    mock_tick_advance(10);
    ctrl.trajectory_active = true;
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_TRUE(ctrl.metrics.settle_tick > 0);

    /* 4. Test RMS error calculation (n > 0) */
    /* Let's reset and do 1 update with error = 100 to test isqrt(10000) = 100 */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, 100);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(100, syn_motor_ctrl_rms_error(&ctrl));

    /* 5. Test isqrt for n = 0 */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, 0);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    ctrl.trajectory_active = true;
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(0, syn_motor_ctrl_rms_error(&ctrl));
}

static void test_motor_ctrl_stall_and_recovery(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type              = SYN_MCTRL_DC;
    cfg.read_pos          = mock_encoder_feedback;
    cfg.dc_motor          = &dc;
    cfg.update_hz         = 100;
    cfg.output_min        = -100;
    cfg.output_max        = 100;
    cfg.stall_timeout_ms  = 50;
    cfg.stall_threshold   = 1;
    cfg.pid_kp            = 100;
    cfg.pid_scale         = 0;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. Test update when mode is IDLE */
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_IDLE, ctrl.mode);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_update(&ctrl));

    /* 2. Test update when disabled */
    syn_motor_ctrl_set_velocity(&ctrl, 500);
    ctrl.enabled = false;
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_update(&ctrl));

    /* 3. Re-enable and test stall detection when output is low (<= output_max / 4 = 25) */
    ctrl.enabled = true;
    syn_motor_ctrl_set_velocity(&ctrl, 0); // target speed = 0, output is small
    /* Update multiple times; output is small, stall active should be false */
    int i;
    for (i = 0; i < 10; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&ctrl);
    }
    TEST_ASSERT_FALSE(ctrl.stall_active);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, ctrl.state);

    /* 4. Test stall active gets reset when delta > stall_threshold */
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // first update to set last_position
    
    mock_ctrl_position = 2; // delta = 2 > stall_threshold
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_FALSE(ctrl.stall_active);

    /* Now feedback stops moving (delta = 0 <= stall_threshold) */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_TRUE(ctrl.stall_active);
    
    /* Feedback moves again (delta = 2 > stall_threshold) -> stall_active goes false */
    mock_ctrl_position = 4;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_FALSE(ctrl.stall_active);

    /* 5. Trigger stall and then clear it */
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    mock_ctrl_position = 0;
    /* update once to initialize last_position */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);

    /* Let it stall: no movement for 60ms > stall_timeout_ms(50) */
    for (i = 0; i < 7; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&ctrl);
    }
    TEST_ASSERT_EQUAL(SYN_MCTRL_STALLED, ctrl.state);

    /* Clear stall */
    syn_motor_ctrl_clear_stall(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, ctrl.state);
    TEST_ASSERT_FALSE(ctrl.stall_active);

    /* 6. Clear stall when NOT stalled does nothing */
    syn_motor_ctrl_clear_stall(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, ctrl.state);
}

static void test_motor_ctrl_errors_and_setters(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type              = SYN_MCTRL_DC;
    cfg.read_pos          = mock_encoder_feedback;
    cfg.dc_motor          = &dc;
    cfg.update_hz         = 100;
    cfg.output_min        = -100;
    cfg.output_max        = 100;
    cfg.position_min      = 100;
    cfg.position_max      = 1000;
    cfg.stall_timeout_ms  = 50;
    cfg.stall_threshold   = 0;
    cfg.pid_kp            = 100;
    cfg.pid_scale         = 0;

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 1);
    cfg.errlog = &elog;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. Test limits error logging */
    syn_motor_ctrl_set_position(&ctrl, 500);
    ctrl.target_position = 50; // bypass clamp to force negative output
    mock_ctrl_position = 100; // at min limit
    mock_tick_advance(10);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl); // should hit limit
    TEST_ASSERT_EQUAL(SYN_MCTRL_LIMIT, st);
    TEST_ASSERT_EQUAL_UINT(1, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL_UINT(0x0101, entries[0].code); // SYN_MCTRL_ERR_LIMIT

    /* 2. Test stall error logging */
    syn_errlog_init(&elog, entries, 4, 1);
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    int i;
    for (i = 0; i < 7; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&ctrl);
    }
    TEST_ASSERT_EQUAL(SYN_MCTRL_STALLED, ctrl.state);
    TEST_ASSERT_EQUAL_UINT(1, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL_UINT(0x0100, entries[0].code); // SYN_MCTRL_ERR_STALL

    /* 3. Test setter wrappers */
    syn_motor_ctrl_set_ff_gains(&ctrl, 42, 24);
    TEST_ASSERT_EQUAL_INT(42, ctrl.cfg.ff_kv);
    TEST_ASSERT_EQUAL_INT(24, ctrl.cfg.ff_ka);

    syn_motor_ctrl_set_gains(&ctrl, 3, 2, 1);
    TEST_ASSERT_EQUAL_INT(3, ctrl.pid.cfg.kp);
    TEST_ASSERT_EQUAL_INT(2, ctrl.pid.cfg.ki);
    TEST_ASSERT_EQUAL_INT(1, ctrl.pid.cfg.kd);
}

void run_motor_ctrl_tests(void)
{
    RUN_TEST(test_motor_ctrl);
    RUN_TEST(test_motor_ctrl_stepper);
    RUN_TEST(test_motor_ctrl_trajectory);
    RUN_TEST(test_motor_ctrl_saturation);
    RUN_TEST(test_motor_ctrl_datalog);
    RUN_TEST(test_motor_ctrl_metrics);
    RUN_TEST(test_motor_ctrl_stall_and_recovery);
    RUN_TEST(test_motor_ctrl_errors_and_setters);
}
