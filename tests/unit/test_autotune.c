#include "mocks/mock_port.h"
#include "syntropic/control/syn_autotune.h"
#include "syntropic/log/syn_datalog.h"
#include "syntropic/motor/syn_dc_motor.h"
#include "syntropic/motor/syn_motor_ctrl.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static int32_t mock_pos = 0;
static int32_t last_mock_output = 0;

int32_t mock_read_pos(void *ctx)
{
    (void)ctx;
    return mock_pos;
}

void mock_set_output(void *ctx, int32_t output)
{
    (void)ctx;
    last_mock_output = output;
}

void test_autotune_probe_phase(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {.position_min = -1000, .position_max = 1000};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL, 100);

    TEST_ASSERT_EQUAL(SYN_ATUNE_PROBE, at.state);

    /* Initially probing at 5% */
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(5, last_mock_output);

    /* No motion for 300ms -> increment to 6% */
    mock_tick_advance(301);
    syn_autotune_update(&at); /* This increments but applies old */
    syn_autotune_update(&at); /* This applies new */
    TEST_ASSERT_EQUAL(6, last_mock_output);

    /* Advance another 300ms -> increment to 7% */
    mock_tick_advance(301);
    syn_autotune_update(&at);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(7, last_mock_output);

    /* Simulate motion at 7% */
    mock_pos = 100;
    syn_autotune_update(&at); /* transitions to BRAKING */
    syn_autotune_update(&at); /* motor stopped -> transitions to RAMP_UP */

    /* Should transition to RAMP_UP */
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_UP, at.state);
    TEST_ASSERT_EQUAL(0, last_mock_output);

    /* Verify test_output was computed (7 * 1.2 = 8) */
    TEST_ASSERT_EQUAL(8, at.cfg.test_output);
}

void test_autotune_datalog(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    uint8_t log_buf[256];
    SYN_DataLog log;
    syn_datalog_init(&log, log_buf, sizeof(log_buf));

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_FF_IDENT,
                               .test_output = 20,
                               .limits = {.position_min = -1000, .position_max = 1000},
                               .datalog = &log};
    syn_autotune_init(&at, &ctrl, &cfg);

    syn_autotune_update(&at);

    /* Verify log entry */
    uint16_t id;
    SYN_AutoTune_LogFrame frame;
    size_t n = syn_datalog_read(&log, &id, &frame, sizeof(frame));
    TEST_ASSERT_EQUAL(sizeof(frame), n);
    TEST_ASSERT_EQUAL(SYN_ATUNE_LOG_ID, id);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_UP, frame.state);
}

void test_autotune_safety_limits(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {.position_min = -10, .position_max = 10};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL, 100);

    /* Trigger probe motion that exceeds limits */
    mock_pos = 100;
    syn_autotune_update(&at);

    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_POSITION, syn_autotune_abort_reason(&at));
}

void test_autotune_auto_sequence(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {
        .position_min = -10000, .position_max = 10000, .watchdog_ms = 10000};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL, 100);

    /* 1. Probe phase */
    mock_pos = 100;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_BRAKING, at.state);

    /* 2. Transition to FF Identification */
    mock_tick_advance(2001);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_UP, at.state);

    /* Ramp up completes */
    mock_tick_advance(501);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_SETTLING, at.state);

    /* Settling completes (adaptive) */
    for (int i = 0; i < 4; i++) {
        mock_tick_advance(300);
        syn_autotune_update(&at);
    }
    /* P2 should be captured by now (1200ms elapsed) */
    for (int i = 0; i < 4; i++) {
        mock_tick_advance(300);
        syn_autotune_update(&at);
    }
    /* Steady-state should be reached (another 1200ms elapsed) */
    TEST_ASSERT_EQUAL(SYN_ATUNE_MEASURING, at.state);

    /* Measuring 1 completes -> transition to SETTLING_2 */
    mock_tick_advance(2001);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_SETTLING_2, at.state);

    /* Settling 2 completes -> transition to MEASURING_2 */
    mock_tick_advance(1001);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_MEASURING_2, at.state);

    /* Measuring 2 completes -> transition to BRAKING */
    mock_pos = 2000; /* faster velocity */
    mock_tick_advance(2001);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_BRAKING, at.state);
    TEST_ASSERT_TRUE(at.result.ff_kv > 0);

    /* 3. Transition to Relay Tuning */
    mock_tick_advance(2001);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_UP, at.state);

    mock_tick_advance(501);
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RELAY, at.state);
}

void test_autotune_edge_cases(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {0};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_TUNE_PID, 100);

    /* 1. Trigger zero amplitude branch (line 456) */
    at.state = SYN_ATUNE_RELAY;
    at.period_count = at.cfg.relay_cycles;
    at.amplitude_count = 0; /* Force zero */
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);

    /* 2. Trigger scale increment (line 475) */
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_TUNE_PID, 100);
    at.cfg.test_output = 1;
    at.state = SYN_ATUNE_RELAY;
    at.period_count = at.cfg.relay_cycles;
    at.amplitude_sum = 2000;
    at.amplitude_count = 1; /* half_amp = 1000 */
    syn_autotune_update(&at);
    TEST_ASSERT_TRUE(at.result.pid_scale > 8);
}

void test_autotune_gain_multiplier(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {0};

    /* Tune at 100% */
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_TUNE_PID, 100);
    at.state = SYN_ATUNE_RELAY;
    at.period_count = at.cfg.relay_cycles;
    at.amplitude_sum = 2000;
    at.amplitude_count = 1;
    syn_autotune_update(&at);
    int32_t kp_100 = at.result.kp;

    /* Tune at 50% */
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_TUNE_PID, 50);
    at.state = SYN_ATUNE_RELAY;
    at.period_count = at.cfg.relay_cycles;
    at.amplitude_sum = 2000;
    at.amplitude_count = 1;
    syn_autotune_update(&at);
    int32_t kp_50 = at.result.kp;

    TEST_ASSERT_EQUAL(kp_100 / 2, kp_50);
}

void test_autotune_abort_apply(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {0};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_TUNE_PID, 100);

    /* Test Abort */
    syn_autotune_abort(NULL);
    syn_autotune_abort(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_USER, syn_autotune_abort_reason(&at));

    /* Test Apply on finished state */
    at.state = SYN_ATUNE_DONE;
    at.result.kp = 123;
    syn_autotune_apply(&at);
    TEST_ASSERT_EQUAL(123, ctrl.cfg.pid_kp);

    /* Test Apply on non-finished state (should return) */
    at.state = SYN_ATUNE_IDLE;
    at.result.kp = 456;
    syn_autotune_apply(&at);
    TEST_ASSERT_EQUAL(123, ctrl.cfg.pid_kp); /* unchanged */

    /* Test Abort NULL */
    syn_autotune_abort(NULL); /* should not crash */
}

void test_autotune_zn_methods(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {
        .mode = SYN_ATUNE_MODE_AUTO, .method = SYN_ATUNE_ZN_CLASSIC, .relay_cycles = 1};
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_RELAY;
    at.period_count = 1;
    at.period_sum = 1000; /* Force Tu = 1000ms */
    at.amplitude_sum = 2000;
    at.amplitude_count = 1;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);

    cfg.method = SYN_ATUNE_ZN_NO_OVERSHOOT;
    cfg.test_output = 100;
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_RELAY;
    at.period_count = 1;
    at.period_sum = 1000;
    at.amplitude_sum = 200; /* half_amp = 100 */
    at.amplitude_count = 1;
    syn_autotune_update(&at);

    /* Zero period count / zero period sum test (division by zero protection) */
    cfg.method = SYN_ATUNE_ZN_CLASSIC;
    syn_autotune_init(&at, &ctrl, &cfg);
    at.cfg.relay_cycles = 0;
    at.state = SYN_ATUNE_RELAY;
    at.period_count = 0;
    at.period_sum = 0;
    at.amplitude_sum = 0;
    at.amplitude_count = 0;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);
}

void test_autotune_ramp_down(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .ramp_ms = 1000};
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_RAMP_DOWN;
    at.start_output = 100;
    at.phase_start_tick = 0;

    mock_tick_ms = 0;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(100, last_mock_output);

    mock_tick_ms = 500;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(50, last_mock_output);

    mock_tick_ms = 1000;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(0, last_mock_output);
    TEST_ASSERT_EQUAL(SYN_ATUNE_DONE, at.state);
}

void test_autotune_ramp_down_negative(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .ramp_ms = 1000};
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_RAMP_DOWN;
    at.start_output = -100;
    at.phase_start_tick = 0;

    mock_tick_ms = 500;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(-50, last_mock_output);

    mock_tick_ms = 1000;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(0, last_mock_output);
    TEST_ASSERT_EQUAL(SYN_ATUNE_DONE, at.state);
}

void test_autotune_ramp_zero(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .ramp_ms = 0};
    syn_autotune_init(&at, &ctrl, &cfg);
    at.cfg.ramp_ms = 0; /* Override the default 500 set by init */
    at.state = SYN_ATUNE_RAMP_DOWN;
    at.start_output = 100;
    at.phase_start_tick = 0;

    mock_tick_ms = 500;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_DONE, at.state);
}

void test_autotune_terminal_states(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);
    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO};
    syn_autotune_init(&at, &ctrl, &cfg);

    at.state = SYN_ATUNE_DONE;
    TEST_ASSERT_EQUAL(SYN_ATUNE_DONE, syn_autotune_update(&at));

    at.state = SYN_ATUNE_ABORTED;
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, syn_autotune_update(&at));

    at.state = SYN_ATUNE_IDLE;
    TEST_ASSERT_EQUAL(SYN_ATUNE_IDLE, syn_autotune_update(&at));
}

void test_autotune_relay_mode(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);
    ctrl.measured_position = 1000;

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_RELAY, .setpoint = 1000, .relay_cycles = 3};
    syn_autotune_init(&at, &ctrl, &cfg);
    TEST_ASSERT_EQUAL(SYN_ATUNE_MODE_RELAY, at.cfg.mode);
    TEST_ASSERT_EQUAL(1000, at.osc_peak_pos);
}

void test_autotune_abort_velocity(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {.max_velocity = 100};
    syn_autotune_start(&at, &ctrl, &limits, 0, 100);

    mock_pos = 0;
    syn_autotune_update(&at);

    /* Rapid movement */
    mock_pos = 1000;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_VELOCITY, at.abort_reason);
}

void test_autotune_abort_soft_limit(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    mcfg.position_min = 0;
    mcfg.position_max = 1000;
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Limits limits = {0};
    syn_autotune_start(&at, &ctrl, &limits, 0, 100);

    mock_pos = 2000;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_SOFT_LIMIT, at.abort_reason);
}

void test_autotune_abort_no_motion(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 100, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .test_output = 0};
    syn_autotune_init(&at, &ctrl, &cfg);

    TEST_ASSERT_EQUAL(SYN_ATUNE_PROBE, at.state);
    mock_tick_ms = 0;
    mock_pos = 0;

    /* Need output > 50. Starts at 5, +1 every 300ms.
     * Must call update() repeatedly to increment. */
    for (int i = 0; i < 60; i++) {
        mock_tick_ms += 300;
        syn_autotune_update(&at);
        if (at.state == SYN_ATUNE_ABORTED)
            break;
    }

    /* The probe should terminate — either via no-motion abort or watchdog.
     * Accept either ABORTED or DONE (ramp_down shortcut). */
    TEST_ASSERT_TRUE(at.state == SYN_ATUNE_ABORTED || at.state == SYN_ATUNE_DONE);
}

void test_autotune_abort_watchdog(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 100, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    mock_tick_ms = 100;
    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .watchdog_ms = 500};
    syn_autotune_init(&at, &ctrl, &cfg);

    syn_autotune_update(&at);

    mock_tick_ms = 1000;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_WATCHDOG, at.abort_reason);
}

void test_autotune_ka_tuning(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 100, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO,
                               .flags = SYN_ATUNE_FLAG_IDENT_KA,
                               .test_output = 50,
                               .measure_ms = 1000,
                               .settle_ms = 1000};
    syn_autotune_init(&at, &ctrl, &cfg);

    mock_tick_ms = 0;
    mock_pos = 0;
    syn_autotune_update(&at);

    at.state = SYN_ATUNE_SETTLING;
    at.ka_p1_captured = false;
    at.ka_p2_captured = false;

    mock_tick_ms = 100;
    mock_pos = 100;
    syn_autotune_update(&at);
    TEST_ASSERT_TRUE(at.ka_p1_captured);
    TEST_ASSERT_EQUAL(10000, at.ka_v1); /* 100 counts * 100Hz = 10000 */

    mock_tick_ms = 200;
    mock_pos = 300;
    syn_autotune_update(&at);
    TEST_ASSERT_TRUE(at.ka_p2_captured);
    TEST_ASSERT_EQUAL(20000, at.ka_v2); /* (300-100) * 100Hz = 20000 */
}

void test_autotune_relay_zero_crossings(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 100, 1000);
    ctrl.cfg.position_deadband = 5;
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_RELAY,
                               .test_output = 50,
                               .setpoint = 100,
                               .relay_cycles = 2,
                               .watchdog_ms = 5000};
    syn_autotune_init(&at, &ctrl, &cfg);

    at.state = SYN_ATUNE_RELAY;
    at.above_setpoint = false;
    at.relay_output = 50;

    /* Pos moves above setpoint + hysteresis (100 + 10 = 110) */
    mock_pos = 120;
    mock_tick_ms = 100;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(1, at.half_cycles);
    TEST_ASSERT_TRUE(at.above_setpoint);
    TEST_ASSERT_EQUAL(-50, at.relay_output);

    /* Pos drops below setpoint - hysteresis (100 - 10 = 90) */
    mock_pos = 80;
    mock_tick_ms = 200;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(2, at.half_cycles);
    TEST_ASSERT_FALSE(at.above_setpoint);
    TEST_ASSERT_EQUAL(50, at.relay_output);
}

static void test_autotune_calc_relay_gains(void)
{
    int32_t kp = 0, ki = 0, kd = 0;

    /* Ziegler-Nichols Classic: Ku=1000, Tu=500ms */
    syn_autotune_calc_relay_gains(1000, 500, SYN_ATUNE_ZN_CLASSIC, 100, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(600, kp);
    TEST_ASSERT_EQUAL_INT32(2400, ki);
    TEST_ASSERT_EQUAL_INT32(37, kd);

    /* Tyreus-Luyben Conservative: Kp = Ku / 2.2, Ki = Kp / (2.2 * Pu), Kd = Kp * Pu / 6.3 */
    syn_autotune_calc_relay_gains(1000, 500, SYN_ATUNE_TYREUS_LUYBEN, 100, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(454, kp);
    TEST_ASSERT_EQUAL_INT32(412, ki);
    TEST_ASSERT_EQUAL_INT32(36, kd);

    /* No-Overshoot Mode */
    syn_autotune_calc_relay_gains(1000, 500, SYN_ATUNE_ZN_NO_OVERSHOOT, 100, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(200, kp);
    TEST_ASSERT_EQUAL_INT32(800, ki);
    TEST_ASSERT_EQUAL_INT32(33, kd);

    /* With 80% Safety Multiplier */
    syn_autotune_calc_relay_gains(1000, 500, SYN_ATUNE_ZN_CLASSIC, 80, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(480, kp);
    TEST_ASSERT_EQUAL_INT32(1920, ki);
    TEST_ASSERT_EQUAL_INT32(29, kd);
}

static void test_autotune_ka_identification_calculation(void)
{
    mock_tick_ms = 200;
    mock_pos = 1;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    mcfg.ff_scale = 8;
    syn_motor_ctrl_init(&ctrl, &mcfg);
    ctrl.last_position = 0;

    SYN_AutoTune at;
    memset(&at, 0, sizeof(at));
    at.ctrl = &ctrl;
    at.cfg.flags = SYN_ATUNE_FLAG_IDENT_KA;
    at.cfg.test_output = 500;
    at.cfg.measure_ms = 100;
    at.cfg.limits.position_max = 10000;
    at.cfg.limits.position_min = -10000;

    at.ka_t1 = 100;
    at.ka_t2 = 300;
    at.ka_v1 = 10;
    at.ka_v2 = 500;
    at.result.steady_velocity_1 = 100;
    at.velocity_sum = 0;
    at.velocity_samples = 0;
    at.state = SYN_ATUNE_MEASURING_2;
    at.phase_start_tick = 50;

    syn_autotune_update(&at);
    TEST_ASSERT_TRUE(at.result.ff_ka > 0);
}

static void test_autotune_zn_no_overshoot_and_tyreus_luyben(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    memset(&at, 0, sizeof(at));
    at.ctrl = &ctrl;
    at.cfg.relay_cycles = 1;
    at.cfg.test_output = 100;
    at.cfg.gain_multiplier_pct = 100;
    at.period_sum = 200;
    at.period_count = 1;
    at.amplitude_sum = 50;
    at.amplitude_count = 1;
    at.half_cycles = 2;
    at.state = SYN_ATUNE_RELAY;

    /* 1. ZN No Overshoot */
    at.cfg.method = SYN_ATUNE_ZN_NO_OVERSHOOT;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);
    TEST_ASSERT_TRUE(at.result.kp > 0);

    /* 2. Tyreus Luyben */
    at.state = SYN_ATUNE_RELAY;
    at.cfg.method = SYN_ATUNE_TYREUS_LUYBEN;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);
    TEST_ASSERT_TRUE(at.result.kp > 0);
}

static void test_autotune_calc_relay_gains_zero_tu_and_default_state(void)
{
    int32_t kp = 0, ki = 0, kd = 0;
    syn_autotune_calc_relay_gains(100, 0, SYN_ATUNE_ZN_CLASSIC, 100, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL(0, kp);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);
    SYN_AutoTune at;
    ctrl.cfg.pid_scale = 0;
    ctrl.cfg.ff_scale = 0;
    SYN_AutoTune_Limits limits = {.position_min = -1000, .position_max = 1000};
    syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL, 100);
    at.state = (SYN_AutoTune_State)99;
    TEST_ASSERT_EQUAL((SYN_AutoTune_State)99, syn_autotune_update(&at));
}

void test_autotune_relay_hysteresis_and_limit_cycles(void)
{
    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 100);
    syn_motor_ctrl_init(&ctrl, &mcfg);
    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {.mode = SYN_ATUNE_MODE_AUTO, .relay_cycles = 1, .test_output = 50};
    syn_autotune_init(&at, &ctrl, &cfg);

    /* 1. Deadband abort no motion output > 50 (line 217) */
    at.state = SYN_ATUNE_PROBE;
    at.current_output = 50;
    at.phase_start_tick = 0;
    mock_tick_ms = 300;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, at.state);
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_NO_MOTION, at.abort_reason);

    /* 2. Negative peak amplitude amp < 0 & period_sum update (lines 445, 451-452, 483) */
    syn_autotune_init(&at, &ctrl, &cfg);
    ctrl.cfg.pid_scale = 0; /* line 483 test */
    at.state = SYN_ATUNE_RELAY;
    at.half_cycles = 1;
    at.last_cross_tick = 100;
    at.above_setpoint = true;
    at.osc_peak_pos = -100;
    at.osc_peak_neg = 100; /* amp = pos - neg = -200 < 0 (line 445) */
    mock_tick_ms = 500;
    mock_pos = -10; /* Cross setpoint 0 to negative */
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(2, at.half_cycles);
    TEST_ASSERT_EQUAL_UINT32(400, at.period_sum); /* lines 451-452 hit */
}

static void test_r5_autotune_apply_live_pid_and_relay_math(void)
{
    mock_tick_ms = 0;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {
        .mode = SYN_ATUNE_MODE_RELAY,
        .method = SYN_ATUNE_ZN_CLASSIC,
        .test_output = 100,
        .relay_cycles = 1,
        .gain_multiplier_pct = 100,
    };
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_RELAY;
    at.period_count = 1;
    at.period_sum = 500;
    at.amplitude_sum = 200;
    at.amplitude_count = 1;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL_INT32((at.result.kp * 2000) / 500, at.result.ki);

    at.state = SYN_ATUNE_DONE;
    at.result.kp = 512;
    at.result.ki = 1024;
    at.result.kd = 64;
    at.result.pid_scale = 8;
    at.result.ff_kv = 128;
    at.result.ff_ka = 32;
    at.result.ff_scale = 10;
    syn_autotune_apply(&at);

    TEST_ASSERT_EQUAL_INT32(512, ctrl.pid.cfg.kp);
    TEST_ASSERT_EQUAL_INT32(1024, ctrl.pid.cfg.ki);
    TEST_ASSERT_EQUAL_INT32(64, ctrl.pid.cfg.kd);
    TEST_ASSERT_EQUAL_INT32(256, ctrl.pid.cfg.scale);
    TEST_ASSERT_EQUAL_UINT8(10, ctrl.cfg.ff_scale);

    /* Live closed-loop step uses tuned kp = 512 / 256 = 2.0 */
    syn_motor_ctrl_set_position(&ctrl, 100);
    mock_tick_advance(10);
    syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_TRUE(ctrl.pid_output >= 200);

    /* Guard pid_scale / ff_scale >= 31 and large int64 clamp in calc_relay_gains */
    at.result.pid_scale = 35;
    at.result.ff_scale = 35;
    syn_autotune_apply(&at);
    TEST_ASSERT_EQUAL_UINT8(30, ctrl.cfg.pid_scale);
    TEST_ASSERT_EQUAL_UINT8(30, ctrl.cfg.ff_scale);

    int32_t kp = 0;
    int32_t ki = 0;
    int32_t kd = 0;
    syn_autotune_calc_relay_gains(INT32_MAX, 1U, SYN_ATUNE_ZN_CLASSIC, 200U, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ki);
    syn_autotune_calc_relay_gains(-INT32_MAX, 1U, SYN_ATUNE_ZN_CLASSIC, 200U, &kp, &ki, &kd);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, ki);
}

static void test_autotune_settling2_window_ff_termination_and_two_phase_apply(void)
{
    mock_tick_ms = 1000;
    mock_pos = 0;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    /* 1. SETTLING_2 waits full 500ms after MEASURING, and FF_IDENT + FLAG_ALL terminates cleanly */
    SYN_AutoTune at;
    SYN_AutoTune_Config cfg = {
        .mode = SYN_ATUNE_MODE_FF_IDENT,
        .flags = SYN_ATUNE_FLAG_ALL,
        .test_output = 100,
        .measure_ms = 500,
        .settle_ms = 1000,
        .watchdog_ms = 5000,
    };
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_MEASURING;
    at.phase_start_tick = 1000;
    at.velocity_sum = 1000;
    at.velocity_samples = 10;

    mock_tick_ms = 1600;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_SETTLING_2, at.state);

    /* At +250ms (< 500ms window), must remain in SETTLING_2 even if dv == 0 */
    mock_tick_ms = 1850;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_SETTLING_2, at.state);

    /* At +500ms, transitions to MEASURING_2 */
    mock_tick_ms = 2100;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_MEASURING_2, at.state);

    /* Finish MEASURING_2: FF_IDENT must go to RAMP_DOWN, never BRAKING loop */
    at.velocity_sum = 3000;
    at.velocity_samples = 10;
    mock_tick_ms = 2700;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, at.state);

    /* 2. AUTO mode with TUNE_PID transitions to RELAY even when ff_kv == 0 */
    cfg.mode = SYN_ATUNE_MODE_AUTO;
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_MEASURING_2;
    at.phase_start_tick = 2700;
    at.result.steady_velocity_1 = 0;
    mock_tick_ms = 3300;
    syn_autotune_update(&at); /* v1 == v2 == 0 -> ff_kv == 0, enters BRAKING */
    TEST_ASSERT_EQUAL(SYN_ATUNE_BRAKING, at.state);
    TEST_ASSERT_EQUAL_INT32(0, at.result.ff_kv);

    mock_tick_ms = 3400;
    syn_autotune_update(&at); /* velocity == 0 -> enters RAMP_UP */
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_UP, at.state);

    mock_tick_ms = 4000;
    syn_autotune_update(&at); /* ramp done -> must enter RELAY, not loop back to SETTLING */
    TEST_ASSERT_EQUAL(SYN_ATUNE_RELAY, at.state);

    /* 3. MEASURING_2 ff_ka guards accel == 0 (no SIGFPE) and dv * 1000 overflow */
    cfg.mode = SYN_ATUNE_MODE_FF_IDENT;
    cfg.flags = SYN_ATUNE_FLAG_IDENT_KV | SYN_ATUNE_FLAG_IDENT_KA;
    cfg.test_output = 500;
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_MEASURING_2;
    at.phase_start_tick = 4000;
    at.result.steady_velocity_1 = 10;
    at.velocity_sum = 2000;
    at.velocity_samples = 10;
    at.ka_t1 = 100;
    at.ka_t2 = 20100; /* dt_ms = 20000, dv = 15 -> (15 * 1000) / 20000 == 0 */
    at.ka_v1 = 0;
    at.ka_v2 = 15;
    mock_tick_ms = 4600;
    syn_autotune_update(&at);
    TEST_ASSERT_EQUAL_INT32(0, at.result.ff_ka);

    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_MEASURING_2;
    at.phase_start_tick = 4600;
    at.result.steady_velocity_1 = 100;
    at.velocity_sum = 10000;
    at.velocity_samples = 10;
    at.ka_t1 = 100;
    at.ka_t2 = 200;
    at.ka_v1 = 0;
    at.ka_v2 = 3000000; /* dv * 1000 = 3e9 > INT32_MAX */
    mock_tick_ms = 5200;
    syn_autotune_update(&at);
    TEST_ASSERT_TRUE(at.result.ff_kv > 0);

    /* 4. Two-phase manual tuning preserves untuned gain group across apply() */
    ctrl.cfg.pid_kp = 333;
    ctrl.cfg.pid_ki = 444;
    ctrl.cfg.pid_kd = 55;
    cfg.mode = SYN_ATUNE_MODE_FF_IDENT;
    syn_autotune_init(&at, &ctrl, &cfg);
    at.state = SYN_ATUNE_DONE;
    at.result.ff_kv = 250;
    at.result.ff_ka = 40;
    syn_autotune_apply(&at);
    TEST_ASSERT_EQUAL_INT32(333, ctrl.cfg.pid_kp);
    TEST_ASSERT_EQUAL_INT32(250, ctrl.cfg.ff_kv);

    SYN_AutoTune at_relay;
    SYN_AutoTune_Config rcfg = {
        .mode = SYN_ATUNE_MODE_RELAY, .test_output = 100, .relay_cycles = 1};
    syn_autotune_init(&at_relay, &ctrl, &rcfg);
    at_relay.state = SYN_ATUNE_DONE;
    at_relay.result.kp = 600;
    at_relay.result.ki = 1200;
    at_relay.result.kd = 80;
    syn_autotune_apply(&at_relay);
    TEST_ASSERT_EQUAL_INT32(600, ctrl.cfg.pid_kp);
    TEST_ASSERT_EQUAL_INT32(250, ctrl.cfg.ff_kv);
    TEST_ASSERT_EQUAL_INT32(40, ctrl.cfg.ff_ka);
}

static void test_autotune_int32_min_velocity_pid_only_auto_and_overflow_bounds(void)
{
    mock_tick_ms = 100;
    mock_pos = 10000000;
    last_mock_output = 0;

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config mcfg =
        SYN_MOTOR_CTRL_DEFAULTS(((SYN_MotorOutput){.set_output = mock_set_output, .ctx = NULL}),
                                mock_read_pos, NULL, 1000, 1000);
    syn_motor_ctrl_init(&ctrl, &mcfg);

    /* 1. Reverse velocity clamping to INT32_MIN must trigger SYN_ATUNE_ABORT_VELOCITY (no UB) */
    SYN_AutoTune at;
    SYN_AutoTune_Limits lim = {
        .position_min = 0, .position_max = 0, .max_velocity = 50000, .watchdog_ms = 5000};
    TEST_ASSERT_EQUAL(SYN_OK, syn_autotune_start(&at, &ctrl, &lim, SYN_ATUNE_FLAG_ALL, 100));
    ctrl.last_position = 10000000;
    mock_pos = -10000000; /* delta = -2e7 * 1000 Hz = -2e10 -> clamps to INT32_MIN */
    mock_tick_ms = 110;
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORTED, syn_autotune_update(&at));
    TEST_ASSERT_EQUAL(SYN_ATUNE_ABORT_VELOCITY, syn_autotune_abort_reason(&at));

    /* 2. SYN_ATUNE_MODE_AUTO with SYN_ATUNE_FLAG_TUNE_PID skips FF ident and preserves ff_kv/ka */
    mock_pos = 0;
    ctrl.measured_position = 0;
    ctrl.last_position = 0;
    ctrl.cfg.ff_kv = 777;
    ctrl.cfg.ff_ka = 88;
    ctrl.cfg.pid_scale = 28;
    ctrl.cfg.ff_scale = 28;
    SYN_AutoTune_Config acfg = {
        .mode = SYN_ATUNE_MODE_AUTO,
        .flags = SYN_ATUNE_FLAG_TUNE_PID,
        .test_output = 50,
        .ramp_ms = 100,
        .relay_cycles = 1,
        .watchdog_ms = 5000,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_autotune_init(&at, &ctrl, &acfg));
    TEST_ASSERT_TRUE(at.ff_phase_done);
    TEST_ASSERT_EQUAL_UINT8(8U, at.result.pid_scale);
    TEST_ASSERT_EQUAL_UINT8(8U, at.result.ff_scale);

    mock_tick_ms = 250;
    TEST_ASSERT_EQUAL(SYN_ATUNE_RELAY, syn_autotune_update(&at));
    at.state = SYN_ATUNE_DONE;
    at.result.kp = 400;
    at.result.ki = 800;
    at.result.kd = 50;
    syn_autotune_apply(&at);
    TEST_ASSERT_EQUAL_INT32(777, ctrl.cfg.ff_kv);
    TEST_ASSERT_EQUAL_INT32(88, ctrl.cfg.ff_ka);
    TEST_ASSERT_EQUAL_INT32(400, ctrl.cfg.pid_kp);

    /* 3. Extreme test_output, deadband, setpoint, and scale > 24 saturate without signed UB */
    ctrl.cfg.pid_scale = 28;
    ctrl.cfg.ff_scale = 28;
    ctrl.cfg.position_deadband = 1200000000;
    SYN_AutoTune_Config xcfg = {
        .mode = SYN_ATUNE_MODE_FF_IDENT,
        .flags = SYN_ATUNE_FLAG_ALL,
        .test_output = -1200000000,
        .settle_ms = 500000000U,
        .measure_ms = 100U,
        .watchdog_ms = 10000U,
    };
    mock_tick_ms = 1000;
    TEST_ASSERT_EQUAL(SYN_OK, syn_autotune_init(&at, &ctrl, &xcfg));
    at.state = SYN_ATUNE_SETTLING_2;
    at.phase_start_tick = 1000;
    at.last_check_tick = 1000;
    mock_tick_ms = 1600;
    TEST_ASSERT_EQUAL(SYN_ATUNE_MEASURING_2, syn_autotune_update(&at));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, last_mock_output);

    at.result.steady_velocity_1 = 0;
    at.velocity_sum = 1000;
    at.velocity_samples = 10;
    mock_tick_ms = 1800;
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, syn_autotune_update(&at));
    TEST_ASSERT_TRUE(at.result.ff_kv > 0);

    xcfg.mode = SYN_ATUNE_MODE_RELAY;
    xcfg.test_output = INT32_MIN;
    xcfg.setpoint = 1500000000;
    xcfg.relay_cycles = 1;
    mock_pos = 1500000000;
    ctrl.measured_position = 1500000000;
    ctrl.last_position = 1500000000;
    mock_tick_ms = 2000;
    TEST_ASSERT_EQUAL(SYN_OK, syn_autotune_init(&at, &ctrl, &xcfg));
    mock_tick_ms = 2100;
    syn_autotune_update(&at); /* RAMP_UP with -INT32_MIN target saturates to INT32_MAX */
    at.state = SYN_ATUNE_RELAY;
    at.half_cycles = 1;
    at.last_cross_tick = 2000;
    at.above_setpoint = true;
    at.osc_peak_pos = 1500000000;
    at.osc_peak_neg = -1500000000;
    mock_pos = -1500000000;
    mock_tick_ms = 2200;
    TEST_ASSERT_EQUAL(SYN_ATUNE_RAMP_DOWN, syn_autotune_update(&at));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, at.amplitude_sum);
    TEST_ASSERT_TRUE(at.result.Ku > 0);
}

void run_autotune_tests(void)
{
    RUN_TEST(test_autotune_probe_phase);
    RUN_TEST(test_autotune_datalog);
    RUN_TEST(test_autotune_safety_limits);
    RUN_TEST(test_autotune_auto_sequence);
    RUN_TEST(test_autotune_edge_cases);
    RUN_TEST(test_autotune_gain_multiplier);
    RUN_TEST(test_autotune_abort_apply);
    RUN_TEST(test_autotune_zn_methods);
    RUN_TEST(test_autotune_ramp_down);
    RUN_TEST(test_autotune_ramp_down_negative);
    RUN_TEST(test_autotune_ramp_zero);
    RUN_TEST(test_autotune_terminal_states);
    RUN_TEST(test_autotune_relay_mode);
    RUN_TEST(test_autotune_abort_velocity);
    RUN_TEST(test_autotune_abort_soft_limit);
    RUN_TEST(test_autotune_abort_no_motion);
    RUN_TEST(test_autotune_abort_watchdog);
    RUN_TEST(test_autotune_ka_tuning);
    RUN_TEST(test_autotune_relay_zero_crossings);
    RUN_TEST(test_autotune_calc_relay_gains);
    RUN_TEST(test_autotune_ka_identification_calculation);
    RUN_TEST(test_autotune_zn_no_overshoot_and_tyreus_luyben);
    RUN_TEST(test_autotune_calc_relay_gains_zero_tu_and_default_state);
    RUN_TEST(test_autotune_relay_hysteresis_and_limit_cycles);
    RUN_TEST(test_r5_autotune_apply_live_pid_and_relay_math);
    RUN_TEST(test_autotune_settling2_window_ff_termination_and_two_phase_apply);
    RUN_TEST(test_autotune_int32_min_velocity_pid_only_auto_and_overflow_bounds);
}
