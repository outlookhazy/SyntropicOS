/**
 * @file test_motor_ctrl.c
 * @brief Unity tests for syn_motor_ctrl.
 */

#include "mocks/mock_port.h"
#include "syntropic/motor/syn_dc_motor.h"
#include "syntropic/motor/syn_motor_ctrl.h"
#include "syntropic/motor/syn_stepper.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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
    (void)c;
    (void)ctx;
    mock_stall_count++;
}

static int mock_target_count = 0;
static void mock_target_cb(SYN_MotorCtrl *c, void *ctx)
{
    (void)c;
    (void)ctx;
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
    cfg.read_pos = mock_encoder_feedback;
    cfg.read_pos_ctx = NULL;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.pid_kp = 100;
    cfg.pid_ki = 10;
    cfg.pid_kd = 5;
    cfg.pid_scale = 6;
    cfg.update_hz = 1000;
    cfg.output_min = -255;
    cfg.output_max = 255;

    syn_motor_ctrl_init(&ctrl, &cfg);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_IDLE, syn_motor_ctrl_mode(&ctrl));

    /* Set velocity target */
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_VELOCITY, syn_motor_ctrl_mode(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_state(&ctrl));

    /* Simulate: feedback = 10 counts per update at 1000Hz = 10000 cnt/s */
    mock_ctrl_position = 10;
    mock_tick_advance(1);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);
    TEST_ASSERT_EQUAL_INT(10000, syn_motor_ctrl_velocity(&ctrl));

    /* Stop */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL_INT(0, syn_motor_ctrl_output(&ctrl));

    /* ── Test 2: Pot-based position control (linear actuator) ── */

    mock_pot_position = 1000;

    SYN_MotorCtrl act;
    SYN_MotorCtrl_Config acfg;
    memset(&acfg, 0, sizeof(acfg));
    acfg.read_pos = mock_pot_feedback;
    acfg.read_pos_ctx = NULL;
    acfg.motor = syn_dc_motor_output(&dc);
    acfg.pid_kp = 50;
    acfg.pid_ki = 5;
    acfg.pid_kd = 10;
    acfg.pid_scale = 6;
    acfg.update_hz = 50;
    acfg.output_min = -255;
    acfg.output_max = 255;
    acfg.position_deadband = 10;
    acfg.position_min = 100;
    acfg.position_max = 3900;

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
    mock_pot_position = 2050; /* within deadband of 10 */
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    TEST_ASSERT_EQUAL(SYN_MCTRL_ON_TARGET, st);
    TEST_ASSERT_EQUAL_INT(1, mock_target_count);

    /* ── Test 3: Soft position limits ───────────────────────── */

    mock_pot_position = 50; /* below position_min (100) */
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
    scfg.read_pos = mock_encoder_feedback;
    scfg.read_pos_ctx = NULL;
    scfg.motor = syn_dc_motor_output(&dc);
    scfg.pid_kp = 200;
    scfg.pid_ki = 0;
    scfg.pid_kd = 0;
    scfg.pid_scale = 6;
    scfg.update_hz = 1000;
    scfg.output_min = -255;
    scfg.output_max = 255;
    scfg.stall_timeout_ms = 100;
    scfg.stall_threshold = 0;

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
    cfg.motor = syn_stepper_output(&stepper);
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;

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

    /* Test NULL motor output (all callbacks NULL) returns safely */
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.ff_kv = 10;
    cfg.ff_ka = 5;
    cfg.ff_scale = 3; // division by 8

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. Test set_trajectory transitions when idle */
    SYN_MotorCtrl_Trajectory traj = {.position = 1000, .velocity = 40, .acceleration = 16};
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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    /* kp = 0, ki = 300 so that integral builds up and clamps.
     * integral_max = (output_max * scale * 1000) / ki = (100 * 1 * 1000) / 300 = 333 */
    cfg.pid_kp = 0;
    cfg.pid_ki = 300;
    cfg.pid_scale = 0; // scale = 1

    /* 1. Test positive saturation via integral_max clamping */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, 10); // target = 10, error = 10, step = 100
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = 100
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = 200
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = 300
    TEST_ASSERT_EQUAL_INT(300, ctrl.pid.integral);

    /* Update again: integral wants 400, clamped to integral_max (333) */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(333, ctrl.pid.integral);

    /* 2. Test negative saturation via integral_max clamping */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, -10);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = -100
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = -200
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = -300
    TEST_ASSERT_EQUAL_INT(-300, ctrl.pid.integral);

    /* Update again: integral wants -400, clamped to -integral_max (-333) */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT(-333, ctrl.pid.integral);

    /* 3. Test combined output saturation and anti-windup clamping with non-zero feedforward */
    /* 3a. Positive feedforward saturation (ff > 0), triggers line 356 */
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.ff_kv = 10;
    cfg.ff_scale = 0;
    cfg.pid_kp = 0;
    cfg.pid_ki = 300;
    cfg.pid_scale = 0;

    syn_motor_ctrl_init(&ctrl, &cfg);
    SYN_MotorCtrl_Trajectory traj = {.position = 10,
                                     .velocity = 2, // ff = 20
                                     .acceleration = 0};
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = 100
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = 200
    mock_tick_advance(10);
    syn_motor_ctrl_update(
        &ctrl); // integral = 300, pid_out = 90 > max_pid (80), takes line 356 subtraction
    TEST_ASSERT_TRUE(ctrl.pid.integral < 300);

    /* 3b. Negative feedforward saturation (ff < 0), triggers line 359 */
    syn_motor_ctrl_init(&ctrl, &cfg);
    traj.position = -10;
    traj.velocity = -2; // ff = -20
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    mock_ctrl_position = 0;
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = -100
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl); // integral = -200
    mock_tick_advance(10);
    syn_motor_ctrl_update(
        &ctrl); // integral = -300, pid_out = -90 < min_pid (-80), takes line 359 subtraction
    TEST_ASSERT_TRUE(ctrl.pid.integral > -300);

    /* 3c. Test integer underflow in ff to swap max_pid and min_pid, and huge negative ff to trigger
     * line 345-346 */
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -1000;
    cfg.output_max = 1000;
    cfg.ff_kv = 214748360; // will multiply by 10/(-10) below to make +/-2147483600
    cfg.ff_scale = 0;
    cfg.pid_kp = 10;
    cfg.pid_ki = 1000;
    cfg.pid_scale = 0;

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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;

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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.stall_timeout_ms = 50;
    cfg.stall_threshold = 1;
    cfg.pid_kp = 100;
    cfg.pid_scale = 0;

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
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.position_min = 100;
    cfg.position_max = 1000;
    cfg.stall_timeout_ms = 50;
    cfg.stall_threshold = 0;
    cfg.pid_kp = 100;
    cfg.pid_scale = 0;

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 1);
    cfg.errlog = &elog;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* 1. Test limits error logging (min limit) */
    syn_motor_ctrl_set_position(&ctrl, 500);
    ctrl.target_position = 50; // bypass clamp to force negative output
    mock_ctrl_position = 100;  // at min limit
    mock_tick_advance(10);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl); // should hit limit
    TEST_ASSERT_EQUAL(SYN_MCTRL_LIMIT, st);
    TEST_ASSERT_EQUAL_UINT(1, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL_UINT(0x0101, entries[0].code); // SYN_MCTRL_ERR_LIMIT

    /* 1b. Test upper position limit check (max limit) */
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_position(&ctrl, 500);
    ctrl.target_position = 2000; // bypass clamp to force positive output
    mock_ctrl_position = 1000;   // at max limit
    mock_tick_advance(10);
    st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_LIMIT, st);

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

/* ── Test: built-in move_to profile ─────────────────────────────────────── */

static void test_motor_ctrl_move_to(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 3, 4, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.pid_kp = 200;
    cfg.pid_ki = 50;
    cfg.pid_kd = 10;
    cfg.pid_scale = 8;
    cfg.output_min = -dc.duty_max;
    cfg.output_max = dc.duty_max;
    cfg.position_deadband = 5;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* Start a move_to (per-second units now) */
    mock_ctrl_position = 0;
    syn_motor_ctrl_move_to(&ctrl, 1000, 500, 5000);
    /*                      target  vel/s  accel/s² */

    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_POSITION, syn_motor_ctrl_mode(&ctrl));
    TEST_ASSERT_TRUE(ctrl.profile_active);
    TEST_ASSERT_TRUE(ctrl.trajectory_active);

    /* After some updates the profile should advance */
    mock_tick_advance(10);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, st);

    /* Stop should clear profile_active */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_FALSE(ctrl.profile_active);
    TEST_ASSERT_FALSE(ctrl.trajectory_active);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
}

/* ── Test: Q8 ramp fixed-point reaches target ──────────────────────────── */

static void test_ramp_trapezoid_fp(void)
{
    SYN_Ramp ramp;
    syn_ramp_init(&ramp, 0);

    /* Use Q8 with small fractional velocity:
     * vel_q8=128 → 0.5 units/tick, accel_q8=25 → ~0.1 units/tick² */
    syn_ramp_set_target_trapezoid_fp(&ramp, 100, 128, 25, 8);

    /* Run enough ticks to complete */
    for (int i = 0; i < 2000; i++) {
        syn_ramp_update(&ramp);
        if (syn_ramp_done(&ramp))
            break;
    }

    TEST_ASSERT_TRUE(syn_ramp_done(&ramp));
    TEST_ASSERT_EQUAL(100, syn_ramp_value(&ramp));
}

/* ── Test: move_to per-second produces nonzero acceleration FF ─────────── */

static void test_motor_ctrl_move_to_accel_ff(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 3, 4, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.pid_kp = 100;
    cfg.pid_scale = 8;
    cfg.ff_kv = 10;
    cfg.ff_ka = 5;
    cfg.ff_scale = 4;
    cfg.output_min = -1000;
    cfg.output_max = 1000;
    cfg.position_deadband = 5;

    syn_motor_ctrl_init(&ctrl, &cfg);

    mock_ctrl_position = 0;
    syn_motor_ctrl_move_to(&ctrl, 10000, 2000, 20000);

    /* Run a few ticks during acceleration phase */
    bool saw_nonzero_accel = false;
    for (int i = 0; i < 50; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&ctrl);
        if (ctrl.trajectory.acceleration != 0) {
            saw_nonzero_accel = true;
            break;
        }
    }

    TEST_ASSERT_TRUE_MESSAGE(saw_nonzero_accel,
                             "Expected nonzero acceleration during move_to accel phase");
}

/* ── Test: defaults macro ──────────────────────────────────────────────── */

static void test_motor_ctrl_defaults(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 3, 4, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl_Config cfg =
        SYN_MOTOR_CTRL_DEFAULTS(syn_dc_motor_output(&dc), mock_encoder_feedback, NULL, 1000, 1000);

    /* Verify defaults are sane */
    TEST_ASSERT_EQUAL(1000, cfg.update_hz);
    TEST_ASSERT_EQUAL(-1000, cfg.output_min);
    TEST_ASSERT_EQUAL(1000, cfg.output_max);
    TEST_ASSERT_TRUE(cfg.pid_kp > 0);
    TEST_ASSERT_EQUAL(8, cfg.pid_scale);
    TEST_ASSERT_EQUAL(2, cfg.position_deadband);
    TEST_ASSERT_EQUAL(1000, cfg.stall_timeout_ms);

    /* Should initialize successfully */
    SYN_MotorCtrl ctrl;
    mock_ctrl_position = 0;
    syn_motor_ctrl_init(&ctrl, &cfg);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
}

/* ── Test: S-curve move to ─────────────────────────────────────────────── */

static void test_motor_ctrl_move_to_scurve(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 3, 4, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.pid_kp = 200;
    cfg.pid_scale = 8;
    cfg.output_min = -1000;
    cfg.output_max = 1000;
    cfg.position_deadband = 5;

    syn_motor_ctrl_init(&ctrl, &cfg);
    mock_ctrl_position = 0;

    /* S-curve move: per-second units */
    syn_motor_ctrl_move_to_scurve(&ctrl, 500, 100, 5000, 500000);

    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_TRUE(ctrl.scurve_active);
    TEST_ASSERT_TRUE(ctrl.trajectory_active);
    TEST_ASSERT_FALSE(ctrl.profile_active);

    /* Run some ticks */
    for (int i = 0; i < 20; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&ctrl);
    }

    /* Stop should clear scurve_active */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_FALSE(ctrl.scurve_active);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
}

/* ── Test: open-loop mode ──────────────────────────────────────────────── */

static void test_motor_ctrl_open_loop(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 3, 4, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.pid_kp = 200;
    cfg.pid_scale = 8;
    cfg.output_min = -dc.duty_max;
    cfg.output_max = dc.duty_max;
    cfg.position_deadband = 5;

    syn_motor_ctrl_init(&ctrl, &cfg);

    mock_ctrl_position = 0;

    /* Set direct output */
    syn_motor_ctrl_set_output(&ctrl, 500);

    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_OPEN_LOOP, syn_motor_ctrl_mode(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_RUNNING, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL(500, ctrl.total_output);

    /* Update should maintain the output */
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL(500, ctrl.total_output);

    /* Stop */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_EQUAL(SYN_MCTRL_STOPPED, syn_motor_ctrl_state(&ctrl));
    TEST_ASSERT_EQUAL(SYN_MCTRL_MODE_IDLE, syn_motor_ctrl_mode(&ctrl));
}

static void test_motor_ctrl_soft_limits_and_mid_move_retarget(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.position_min = 100;
    cfg.position_max = 500;
    cfg.output_min = -255;
    cfg.output_max = 255;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* Move to position below min -> clamped to min (100) */
    mock_ctrl_position = 200;
    syn_motor_ctrl_move_to(&ctrl, 50, 1000, 100);
    TEST_ASSERT_EQUAL_INT32(100, ctrl.target_position);

    /* Move to position above max -> clamped to max (500) */
    syn_motor_ctrl_move_to(&ctrl, 800, 1000, 100);
    TEST_ASSERT_EQUAL_INT32(500, ctrl.target_position);

    /* Mid-move retarget (while profile_active is true) */
    syn_motor_ctrl_move_to(&ctrl, 300, 1000, 100);
    TEST_ASSERT_EQUAL_INT32(300, ctrl.target_position);
    TEST_ASSERT_TRUE(ctrl.profile_active);

    /* Move to S-curve soft limit clamping (< min and > max) */
    syn_motor_ctrl_move_to_scurve(&ctrl, 10, 100, 10, 2);
    TEST_ASSERT_EQUAL_INT32(100, ctrl.target_position);

    syn_motor_ctrl_move_to_scurve(&ctrl, 900, 100, 10, 2);
    TEST_ASSERT_EQUAL_INT32(500, ctrl.target_position);
}

static void test_motor_ctrl_profile_completion_deactivation(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 1, 2, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg = {
        .read_pos = mock_encoder_feedback,
        .motor = syn_dc_motor_output(&dc),
        .update_hz = 1000,
    };
    syn_motor_ctrl_init(&ctrl, &cfg);

    /* Move to position with high velocity/accel so ramp finishes quickly */
    mock_ctrl_position = 0;
    syn_motor_ctrl_move_to(&ctrl, 10, 10000, 10000);
    TEST_ASSERT_TRUE(ctrl.profile_active);

    /* Run update steps until ramp finishes */
    for (int i = 0; i < 200; i++) {
        mock_tick_advance(1);
        syn_motor_ctrl_update(&ctrl);
    }
    TEST_ASSERT_FALSE(ctrl.profile_active);

    /* Move with S-curve and step until scurve finishes */
    syn_motor_ctrl_move_to_scurve(&ctrl, 50, 1000, 500, 10);
    TEST_ASSERT_TRUE(ctrl.scurve_active);

    for (int i = 0; i < 2000; i++) {
        mock_tick_advance(1);
        syn_motor_ctrl_update(&ctrl);
    }
    TEST_ASSERT_FALSE(ctrl.scurve_active);
}

static void test_motor_ctrl_feedforward_negative_saturation_swap(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.ff_kv = -1000; /* Large negative feedforward -> min_pid > max_pid */
    cfg.pid_kp = 10;
    syn_motor_ctrl_init(&ctrl, &cfg);
    syn_motor_ctrl_set_velocity(&ctrl, 500);

    /* Update to trigger max_pid < min_pid swap branch in saturation handling */
    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);
}

static void test_motor_ctrl_integrator_anti_windup_clamping(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.output_min = -10;
    cfg.output_max = 10;
    cfg.pid_kp = 100;
    cfg.pid_ki = 50;
    cfg.ff_ka = 500; /* Force ff > output_max to trigger max_pid < min_pid swap */

    syn_motor_ctrl_init(&ctrl, &cfg);
    ctrl.pid.integral = 500;
    syn_motor_ctrl_set_velocity(&ctrl, 500);

    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);
}

static void test_motor_ctrl_output_and_soft_limit_clamps(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.update_hz = 1000;
    cfg.output_min = -100;
    cfg.output_max = 100;
    cfg.position_min = -50;
    cfg.position_max = 50;

    syn_motor_ctrl_init(&ctrl, &cfg);

    /* Output clamps */
    syn_motor_ctrl_set_output(&ctrl, 500);
    TEST_ASSERT_EQUAL_INT(100, ctrl.total_output);

    syn_motor_ctrl_set_output(&ctrl, -500);
    TEST_ASSERT_EQUAL_INT(-100, ctrl.total_output);

    /* Soft limit clamps */
    syn_motor_ctrl_set_position(&ctrl, -200);
    TEST_ASSERT_EQUAL_INT(-50, ctrl.target_position);

    syn_motor_ctrl_set_position(&ctrl, 200);
    TEST_ASSERT_EQUAL_INT(50, ctrl.target_position);

    /* Move to with low velocity */
    syn_motor_ctrl_move_to(&ctrl, 10, 1, 1);

    /* pid_scale >= 31 parameter check */
    cfg.pid_scale = 32;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_motor_ctrl_init(&ctrl, &cfg));
    cfg.pid_scale = 0;
}

static void test_r5_motor_ctrl_overflows_and_mode_switches(void)
{
    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.read_pos = mock_encoder_feedback;
    cfg.motor = syn_dc_motor_output(&dc);
    cfg.update_hz = 10000; /* hz^3 = 10^12 > INT32_MAX */
    cfg.output_min = -1000;
    cfg.output_max = 1000;
    cfg.position_min = -500;
    cfg.position_max = 500;
    cfg.ff_kv = 100000;
    cfg.ff_ka = 100000;
    cfg.ff_scale = 35; /* >= 31 guard */
    cfg.pid_kp = 1;
    cfg.pid_scale = 0;

    mock_tick_ms = 0;
    mock_ctrl_position = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_motor_ctrl_init(&ctrl, &cfg));

    /* 1. High update_hz move_to_scurve without hz^3 overflow + mode switches clear scurve_active */
    syn_motor_ctrl_move_to_scurve(&ctrl, 200, 100000, 1000000000, 1000000000);
    TEST_ASSERT_TRUE(ctrl.scurve_active);
    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);

    syn_motor_ctrl_move_to(&ctrl, 150, 50000, 500000);
    TEST_ASSERT_FALSE(ctrl.scurve_active);
    TEST_ASSERT_TRUE(ctrl.profile_active);

    syn_motor_ctrl_move_to_scurve(&ctrl, 200, 50000, 500000, 5000000);
    TEST_ASSERT_TRUE(ctrl.scurve_active);
    SYN_MotorCtrl_Trajectory traj = {.position = 100, .velocity = 50000, .acceleration = 50000};
    syn_motor_ctrl_set_trajectory(&ctrl, &traj);
    TEST_ASSERT_FALSE(ctrl.scurve_active);
    TEST_ASSERT_FALSE(ctrl.profile_active);
    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);

    /* 2. Soft-limit stop clears scurve_active, and large +/- delta clamps safely */
    mock_ctrl_position = 0;
    syn_motor_ctrl_move_to_scurve(&ctrl, 500, 50000, 500000, 5000000);
    TEST_ASSERT_TRUE(ctrl.scurve_active);
    mock_ctrl_position = 500; /* == position_max while ff (+900) + pid (-499) > 0 */
    mock_tick_advance(1);
    TEST_ASSERT_EQUAL(SYN_MCTRL_LIMIT, syn_motor_ctrl_update(&ctrl));
    TEST_ASSERT_FALSE(ctrl.scurve_active);
    TEST_ASSERT_FALSE(ctrl.trajectory_active);

    mock_ctrl_position = 0;
    syn_motor_ctrl_set_velocity(&ctrl, 0);
    mock_ctrl_position = 300000; /* delta = +300000, delta * 10000 = 3e9 > INT32_MAX */
    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ctrl.measured_velocity);

    mock_ctrl_position = -300000; /* delta = -600000, delta * 10000 = -6e9 < INT32_MIN */
    mock_tick_advance(1);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, ctrl.measured_velocity);
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
    RUN_TEST(test_motor_ctrl_move_to);
    RUN_TEST(test_ramp_trapezoid_fp);
    RUN_TEST(test_motor_ctrl_move_to_accel_ff);
    RUN_TEST(test_motor_ctrl_defaults);
    RUN_TEST(test_motor_ctrl_move_to_scurve);
    RUN_TEST(test_motor_ctrl_open_loop);
    RUN_TEST(test_motor_ctrl_soft_limits_and_mid_move_retarget);
    RUN_TEST(test_motor_ctrl_profile_completion_deactivation);
    RUN_TEST(test_motor_ctrl_feedforward_negative_saturation_swap);
    RUN_TEST(test_motor_ctrl_integrator_anti_windup_clamping);
    RUN_TEST(test_motor_ctrl_output_and_soft_limit_clamps);
    RUN_TEST(test_r5_motor_ctrl_overflows_and_mode_switches);
}
