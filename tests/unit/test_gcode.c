/**
 * @file test_gcode.c
 * @brief Complete unit tests for G-Code RS-274 Motion Interpreter and Controller (100% coverage).
 */

#include "mocks/mock_port.h"
#include "unity/unity.h"

#include <string.h>
#include <syntropic/motor/syn_gcode.h>

static SYN_GCode_SpindleMode g_last_spindle_mode = SYN_GCODE_SPINDLE_OFF;
static float g_last_spindle_rpm = 0.0f;
static SYN_GCode_CoolantMode g_last_coolant_mode = SYN_GCODE_COOLANT_OFF;
static uint32_t g_last_tool_num = 0U;
static float g_last_dwell_sec = -1.0f;

static void on_test_spindle(SYN_GCode_SpindleMode mode, float rpm, void *user_data)
{
    (void)user_data;
    g_last_spindle_mode = mode;
    g_last_spindle_rpm = rpm;
}

static void on_test_coolant(SYN_GCode_CoolantMode mode, void *user_data)
{
    (void)user_data;
    g_last_coolant_mode = mode;
}

static void on_test_tool(uint32_t tool_num, void *user_data)
{
    (void)user_data;
    g_last_tool_num = tool_num;
}

static void on_test_dwell(float seconds, void *user_data)
{
    (void)user_data;
    g_last_dwell_sec = seconds;
}

/* ── Tests ───────────────────────────────────────────────────────────────── */

void test_gcode_init_and_validation(void)
{
    SYN_GCode_Controller ctrl;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_init(NULL, NULL));

    SYN_GCode_Config cfg = {
        .default_feedrate = 150.0f,
        .max_acceleration = 600.0f,
        .max_jerk = 2500.0f,
        .step_resolution = 0.005f,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_init(&ctrl, &cfg));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_MOTION_RAPID, ctrl.motion_mode);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_DIST_ABSOLUTE, ctrl.dist_mode);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_MM, ctrl.units);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 150.0f, ctrl.feedrate);
}

void test_gcode_line_parsing(void)
{
    SYN_GCode_Block block;

    /* Null validations */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line(NULL, &block));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("G1 X10", NULL));

    /* Linear feed with comments, unknown characters, and whitespace */
    const char *line1 = "  G1 X12.5 Y-4.2 Z0.75 F300.0 (Cutting pass) ; trailing comment ? @";
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_parse_line(line1, &block));
    TEST_ASSERT_TRUE(block.has_g);
    TEST_ASSERT_EQUAL_UINT32(1, block.g_code);
    TEST_ASSERT_TRUE(block.has_x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.5f, block.x);
    TEST_ASSERT_TRUE(block.has_y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -4.2f, block.y);
    TEST_ASSERT_TRUE(block.has_z);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.75f, block.z);
    TEST_ASSERT_TRUE(block.has_f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 300.0f, block.f);

    /* Circular CW Arc with K offset and P dwell */
    const char *line2 = "G2 X20.0 Y10.0 Z1.0 I5.0 J0.0 K2.0 P1.5";
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_parse_line(line2, &block));
    TEST_ASSERT_TRUE(block.has_g);
    TEST_ASSERT_EQUAL_UINT32(2, block.g_code);
    TEST_ASSERT_TRUE(block.has_i);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, block.i);
    TEST_ASSERT_TRUE(block.has_j);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, block.j);
    TEST_ASSERT_TRUE(block.has_k);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, block.k);
    TEST_ASSERT_TRUE(block.has_p);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, block.p);

    /* CCW Arc G3 */
    const char *line3 = "G3 X10 Y5 I2 J3";
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_parse_line(line3, &block));
    TEST_ASSERT_EQUAL_UINT32(3, block.g_code);

    /* Spindle CCW M4, Mist Coolant M7, Tool command, positive signs */
    const char *line4 = "M4 S+15000 T2 X+10.5";
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_parse_line(line4, &block));
    TEST_ASSERT_TRUE(block.has_m);
    TEST_ASSERT_EQUAL_UINT32(4, block.m_code);
    TEST_ASSERT_TRUE(block.has_s);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 15000.0f, block.s);
    TEST_ASSERT_TRUE(block.has_t);
    TEST_ASSERT_EQUAL_UINT32(2, block.t);
    TEST_ASSERT_TRUE(block.has_x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.5f, block.x);

    /* Parse errors: invalid float or uint without digits */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("X+", &block));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("G+", &block));
}

void test_gcode_execution_and_modal_states(void)
{
    SYN_Interpolator interp;
    SYN_GCode_Config cfg = {
        .interpolator = &interp,
        .default_feedrate = 200.0f,
        .max_acceleration = 1000.0f,
        .max_jerk = 5000.0f,
        .step_resolution = 0.01f,
        .on_spindle = on_test_spindle,
        .on_coolant = on_test_coolant,
        .on_tool = on_test_tool,
    };

    SYN_GCode_Controller ctrl;
    syn_gcode_init(&ctrl, &cfg);

    /* Null validation */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_block(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(NULL, "G0"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, NULL));
    TEST_ASSERT_FALSE(syn_gcode_step(NULL, NULL));

    /* 1. Modal Planes: G17, G18, G19 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G17"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_PLANE_XY, ctrl.plane);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G18"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_PLANE_XZ, ctrl.plane);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G19"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_PLANE_YZ, ctrl.plane);

    /* 2. Modal Units: G20, G21 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G20"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_INCH, ctrl.units);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G21"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_MM, ctrl.units);

    /* 3. Modal Distance: G90, G91 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G91"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_DIST_INCREMENTAL, ctrl.dist_mode);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G90"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_DIST_ABSOLUTE, ctrl.dist_mode);

    /* 4. Spindle Modes: M3, M4, M5, M99 (default) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M3 S12000"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_SPINDLE_CW, g_last_spindle_mode);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 12000.0f, g_last_spindle_rpm);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M4 S8000"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_SPINDLE_CCW, g_last_spindle_mode);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 8000.0f, g_last_spindle_rpm);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M5"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_SPINDLE_OFF, g_last_spindle_mode);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M30"));

    /* 5. Coolant Modes: M7, M8, M9 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M7"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_COOLANT_MIST, g_last_coolant_mode);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M8"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_COOLANT_FLOOD, g_last_coolant_mode);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "M9"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_COOLANT_OFF, g_last_coolant_mode);

    /* 6. Linear Move (G1) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G1 X10.0 Y5.0 Z0.0 F200.0"));
    TEST_ASSERT_TRUE(ctrl.move_in_progress);

    SYN_Vector3F pos;
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }
    TEST_ASSERT_FALSE(ctrl.move_in_progress);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 10.0f, ctrl.current_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 5.0f, ctrl.current_pos.y);

    /* 7. Incremental Move (G91, then G1) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G91"));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G1 X5.0 Y-2.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 15.0f, ctrl.current_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 3.0f, ctrl.current_pos.y);

    /* Switch back to G90 */
    syn_gcode_execute_line(&ctrl, "G90");

    /* 8. Circular Arc Move (G2 CW & G3 CCW) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G2 X25.0 Y3.0 I5.0 J0.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G3 X15.0 Y3.0 I-5.0 J0.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }

    /* 9. Coordinate offset (G92) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G92 X0.0 Y0.0 Z1.0"));
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 15.0f, ctrl.offset_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, -1.0f, ctrl.offset_pos.z);

    /* 10. Rapid Move (G0) & Dwell (G4) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G0 X10.0 Y0.0 Z-5.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G4 P0.2"));

    /* 11. Incremental Z move */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G91"));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G1 Z2.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
        /* stepping */
    }
    syn_gcode_execute_line(&ctrl, "G90");

    /* 12. Tool change execution */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "T3"));
    TEST_ASSERT_EQUAL_UINT32(3, g_last_tool_num);

    /* 13. Step when idle */
    TEST_ASSERT_FALSE(syn_gcode_step(&ctrl, &pos));

    /* 14. Controller without interpolator direct move */
    SYN_GCode_Controller ctrl_raw;
    syn_gcode_init(&ctrl_raw, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl_raw, "G1 X12.0 Y34.0 Z56.0"));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.0f, ctrl_raw.current_pos.x);

    /* 15. Invalid line execution and unknown letter / G / M rejection */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "X+"));
    SYN_GCode_Block q_block;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("Q10 H2", &q_block));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "G99"));
}

void test_gcode_protothread(void)
{
    SYN_Interpolator interp;
    SYN_GCode_Config cfg = {
        .interpolator = &interp,
        .default_feedrate = 200.0f,
        .max_acceleration = 1000.0f,
        .max_jerk = 5000.0f,
        .step_resolution = 0.01f,
    };
    SYN_GCode_Controller ctrl;
    syn_gcode_init(&ctrl, &cfg);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &ctrl};

    /* Test protothread when idle */
    SYN_PT_Status st = syn_gcode_task_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, st);

    /* Test protothread when move is active */
    syn_gcode_execute_line(&ctrl, "G1 X10.0");
    TEST_ASSERT_TRUE(ctrl.move_in_progress);

    st = syn_gcode_task_pt(&pt, &task);
    TEST_ASSERT_EQUAL_INT(PT_MORE, st);

    /* Step protothread to completion -> final step returns PT_IDLE and clears move_in_progress */
    int guard = 0;
    while (st == PT_MORE && guard++ < 10000) {
        st = syn_gcode_task_pt(&pt, &task);
    }
    TEST_ASSERT_EQUAL_INT(PT_IDLE, st);
    TEST_ASSERT_FALSE(ctrl.move_in_progress);

    /* Move in progress when interpolator == NULL -> clears move_in_progress and returns false
     * (syn_gcode.c:478-479) */
    ctrl.cfg.interpolator = NULL;
    ctrl.move_in_progress = true;
    SYN_Vector3F pos = {0};
    TEST_ASSERT_FALSE(syn_gcode_step(&ctrl, &pos));
    TEST_ASSERT_FALSE(ctrl.move_in_progress);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_gcode_task_pt(NULL, &task));
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_gcode_task_pt(&pt, NULL));
    SYN_Task null_task = {.user_data = NULL};
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_gcode_task_pt(&pt, &null_task));
}

void test_gcode_r4_p13_regressions(void)
{
    SYN_GCode_Block block;

    /* 1. Reject uint32_t overflow in G, M, T words (G4294967297 must not wrap to G1) */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("G4294967297 X10", &block));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("M4294967296", &block));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_parse_line("T99999999999", &block));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_parse_line("T4294967295", &block));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, block.t);

    /* 2. Multi-G word block applies all modal groups and preserves G4 active motion mode */
    SYN_Interpolator interp;
    SYN_GCode_Config cfg = {
        .interpolator = &interp,
        .default_feedrate = 100.0f,
        .max_acceleration = 1000.0f,
        .max_jerk = 5000.0f,
        .step_resolution = 0.01f,
    };
    SYN_GCode_Controller ctrl;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_init(&ctrl, &cfg));
    ctrl.dist_mode = SYN_GCODE_DIST_INCREMENTAL;
    ctrl.units = SYN_GCODE_UNITS_INCH;
    ctrl.plane = SYN_GCODE_PLANE_YZ;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G17 G90 G21 G1 X10.0 F200.0"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_PLANE_XY, ctrl.plane);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_DIST_ABSOLUTE, ctrl.dist_mode);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_MM, ctrl.units);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_MOTION_LINEAR, ctrl.motion_mode);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 200.0f / 60.0f, ctrl.feedrate);

    SYN_Vector3F pos;
    while (syn_gcode_step(&ctrl, &pos)) {
    }
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 10.0f, ctrl.current_pos.x);

    /* Non-modal G4 dwell must NOT overwrite modal motion_mode (G1 stays active for X20) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G4 P0.5"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_MOTION_LINEAR, ctrl.motion_mode);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "X20.0"));
    TEST_ASSERT_TRUE(ctrl.move_in_progress);
    while (syn_gcode_step(&ctrl, &pos)) {
    }
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 20.0f, ctrl.current_pos.x);

    /* 3. G20 inch scaling (25.4 mm/in) on coordinates, incremental moves, arcs, G92, and F */
    SYN_GCode_Controller raw;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_init(&raw, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&raw, "G20 G90 G1 X1.0 Y2.0 Z0.5 F10.0"));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_INCH, raw.units);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 254.0f / 60.0f, raw.feedrate);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.current_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.8f, raw.current_pos.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.7f, raw.current_pos.z);

    /* Incremental move in inches */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&raw, "G91 X0.5 Y-1.0 Z0.5"));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 38.1f, raw.current_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.current_pos.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.current_pos.z);

    /* G92 work coordinate offset in inches */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&raw, "G90 G92 X0.5 Y0.0 Z0.0"));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.offset_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.offset_pos.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.offset_pos.z);

    /* G28 non-modal home (partial axis and full home, preserving motion_mode) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&raw, "G28 X0"));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, raw.current_pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.4f, raw.current_pos.y);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&raw, "G28"));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, raw.current_pos.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, raw.current_pos.z);
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_MOTION_LINEAR, raw.motion_mode);

    /* G2 circular arc in inches with I/J/K offsets on interpolated controller */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "G28"));
    while (syn_gcode_step(&ctrl, &pos)) {
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_gcode_execute_line(&ctrl, "G20 G90 G2 X1.0 Y0.0 I0.5 J0.0 K0.0"));
    while (syn_gcode_step(&ctrl, &pos)) {
    }
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 25.4f, ctrl.current_pos.x);

    /* Legacy caller passing hand-constructed block with only has_g / g_code */
    SYN_GCode_Block legacy = {.has_g = true, .g_code = 21U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_block(&raw, &legacy));
    TEST_ASSERT_EQUAL_INT(SYN_GCODE_UNITS_MM, raw.units);
}

void test_r5_gcode_feedrate_and_strict_validation(void)
{
    SYN_GCode_Controller ctrl;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_init(&ctrl, NULL));
    /* Default 100 mm/min converts to 100/60 = 1.6667 mm/s */
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f / 60.0f, ctrl.feedrate);

    /* G4 dwell callback, step countdown, zero-dwell, and PT non-blocking sleep */
    g_last_dwell_sec = -1.0f;
    SYN_GCode_Config dcfg = {.step_resolution = 0.01f, .on_dwell = on_test_dwell};
    SYN_GCode_Controller dctrl;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_init(&dctrl, &dcfg));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&dctrl, "G4 P0"));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, g_last_dwell_sec);
    TEST_ASSERT_EQUAL_UINT32(0U, dctrl.dwell_steps);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&dctrl, "G4 P0.03"));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.03f, g_last_dwell_sec);
    TEST_ASSERT_EQUAL_UINT32(3U, dctrl.dwell_steps);
    SYN_Vector3F dpos = {0};
    TEST_ASSERT_TRUE(syn_gcode_step(&dctrl, &dpos));
    TEST_ASSERT_TRUE(syn_gcode_step(&dctrl, &dpos));
    TEST_ASSERT_TRUE(syn_gcode_step(&dctrl, &dpos));
    TEST_ASSERT_FALSE(syn_gcode_step(&dctrl, &dpos));

    /* Protothread G4 dwell sleep across mock ticks */
    mock_port_reset();
    SYN_PT dpt;
    PT_INIT(&dpt);
    SYN_Task dtask = {.user_data = &dctrl};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&dctrl, "G4 P0.025"));
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_gcode_task_pt(&dpt, &dtask));
    mock_tick_advance(10U);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_gcode_task_pt(&dpt, &dtask));
    mock_tick_advance(20U);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_gcode_task_pt(&dpt, &dtask));
    TEST_ASSERT_EQUAL_UINT32(0U, dtask.delay_until);

    /* G94 mm/min -> mm/s conversion: F600 mm/min == 10.0 mm/s */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gcode_execute_line(&ctrl, "N10 G94 G21 G1 X5.0 F600.0"));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, ctrl.feedrate);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, ctrl.current_pos.x);

    /* "G999 X10" must error out and NEVER move axes silently */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "G999 X100.0"));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, ctrl.current_pos.x);

    /* Unknown M-code and unrecognized letters must error without moving axes */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "M999 X100.0"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "Q5 X100.0"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "N+"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "F0"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "S-10"));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_line(&ctrl, "P-1"));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, ctrl.current_pos.x);

    /* Direct execute_block with invalid g_code or m_code also rejected */
    SYN_GCode_Block bad_g = {.has_g = true, .g_code = 999U, .has_x = true, .x = 50.0f};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_block(&ctrl, &bad_g));
    SYN_GCode_Block bad_m = {.has_m = true, .m_code = 999U, .has_x = true, .x = 50.0f};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gcode_execute_block(&ctrl, &bad_m));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, ctrl.current_pos.x);
}

void run_gcode_tests(void)
{
    RUN_TEST(test_gcode_init_and_validation);
    RUN_TEST(test_gcode_line_parsing);
    RUN_TEST(test_gcode_execution_and_modal_states);
    RUN_TEST(test_gcode_protothread);
    RUN_TEST(test_gcode_r4_p13_regressions);
    RUN_TEST(test_r5_gcode_feedrate_and_strict_validation);
}
