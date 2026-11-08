/**
 * @file test_cia402.c
 * @brief Unity unit tests for CANopen CiA 402 Servo Drive Profile Engine.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_canopen.h"
#include "syntropic/proto/syn_cia402.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static void test_cia402_init_and_state_transitions(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 2000,
                            .profile_acceleration = 5000,
                            .profile_deceleration = 5000,
                            .quick_stop_deceleration = 10000,
                            .max_torque = 1000};

    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_init(&drive, &cfg));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, syn_cia402_get_state(&drive));
    TEST_ASSERT_EQUAL_UINT16(0, syn_cia402_get_controlword(&drive));

    uint16_t sw = syn_cia402_get_statusword(&drive);
    TEST_ASSERT_EQUAL_HEX16(SYN_CIA402_SW_SWITCH_ON_DISABLED | SYN_CIA402_SW_TARGET_REACHED, sw);

    /* 1. Shutdown command (0x0006): Switch On Disabled -> Ready to Switch On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_READY_TO_SWITCH_ON, drive.state);
    TEST_ASSERT_EQUAL_UINT16(0x0006U, syn_cia402_get_controlword(&drive));

    /* Test Disable Voltage from Ready to Switch On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0000U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Re-enter Ready to Switch On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_READY_TO_SWITCH_ON, drive.state);

    /* 2. Switch On command (0x0007): Ready to Switch On -> Switched On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCHED_ON, drive.state);

    /* Test Shutdown from Switched On -> Ready to Switch On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_READY_TO_SWITCH_ON, drive.state);
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCHED_ON, drive.state);

    /* Test Disable Voltage from Switched On -> Switch On Disabled */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0000U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Direct Transition 2 + 3 + 4: Enable Operation (0x000F) from Switch On Disabled */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Disable Operation from Operation Enabled -> Switched On (0x0007) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCHED_ON, drive.state);

    /* Re-enter Operation Enabled */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Shutdown from Operation Enabled -> Ready to Switch On (0x0006) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_READY_TO_SWITCH_ON, drive.state);

    /* Direct Transition 3 + 4: Enable Operation from Ready to Switch On */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Disable Voltage from Operation Enabled -> Switch On Disabled (0x0000) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0000U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Direct Transition to Switched On (0x0007) from Switch On Disabled */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCHED_ON, drive.state);

    /* 3. Enable Operation command (0x000F): Switched On -> Operation Enabled */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* 4. Quick Stop command (0x000B): Operation Enabled -> Quick Stop Active */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000BU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);

    /* Disable Voltage during Quick Stop */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0000U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Re-enter Quick Stop and complete ramp to 0 */
    syn_cia402_set_controlword(&drive, 0x000FU);
    syn_cia402_set_controlword(&drive, 0x000BU);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_update(&drive, 100));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);
}

static void test_cia402_profile_position_mode(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 1000,
                            .profile_acceleration = 2000,
                            .profile_deceleration = 2000,
                            .quick_stop_deceleration = 5000,
                            .max_torque = 1000};

    syn_cia402_init(&drive, &cfg);
    syn_cia402_set_controlword(&drive, 0x000FU); /* Enable Operation */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PP);
    TEST_ASSERT_EQUAL(SYN_CIA402_MODE_PP, syn_cia402_get_mode(&drive));
    TEST_ASSERT_EQUAL(SYN_CIA402_MODE_PP, syn_cia402_get_mode_display(&drive));

    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_position(&drive, 1000));

    /* Send New Setpoint (Bit 4) */
    syn_cia402_set_controlword(&drive, 0x001FU);
    syn_cia402_update(&drive, 10);

    TEST_ASSERT_TRUE(drive.setpoint_ack);
    TEST_ASSERT_FALSE(drive.target_reached);

    /* Clear New Setpoint bit */
    syn_cia402_set_controlword(&drive, 0x000FU);
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_FALSE(drive.setpoint_ack);

    /* Advance motion updates until target reached */
    SYN_CIA402_Setpoints setpoints;
    for (int i = 0; i < 500; i++) {
        syn_cia402_step(&drive, 10000, &setpoints);
        if (drive.target_reached) {
            break;
        }
    }

    TEST_ASSERT_TRUE(drive.target_reached);
    TEST_ASSERT_EQUAL_INT32(1000, drive.actual_position);
    TEST_ASSERT_EQUAL_INT32(0, drive.actual_velocity);
    TEST_ASSERT_TRUE(setpoints.power_enabled);
    TEST_ASSERT_TRUE(setpoints.brake_released);
    TEST_ASSERT_EQUAL_INT32(1000, setpoints.position_cmd);

    /* Move negative direction */
    syn_cia402_set_target_position(&drive, -500);
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_FALSE(drive.target_reached);
    for (int i = 0; i < 500; i++) {
        syn_cia402_update(&drive, 10);
        if (drive.target_reached) {
            break;
        }
    }
    TEST_ASSERT_TRUE(drive.target_reached);
    TEST_ASSERT_EQUAL_INT32(-500, drive.actual_position);
}

static void test_cia402_profile_velocity_mode(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 2000,
                            .profile_acceleration = 1000,
                            .profile_deceleration = 1000,
                            .quick_stop_deceleration = 5000,
                            .max_torque = 1000};

    syn_cia402_init(&drive, &cfg);
    syn_cia402_set_controlword(&drive, 0x000FU); /* Enable Operation */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PV);

    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_velocity(&drive, 500));

    /* Advance motion updates for velocity ramping */
    for (int i = 0; i < 100; i++) {
        syn_cia402_update(&drive, 10);
    }

    TEST_ASSERT_TRUE(drive.target_reached);
    TEST_ASSERT_EQUAL_INT32(500, drive.actual_velocity);

    /* Decelerate to negative velocity */
    syn_cia402_set_target_velocity(&drive, -500);
    for (int i = 0; i < 200; i++) {
        syn_cia402_update(&drive, 10);
    }
    TEST_ASSERT_EQUAL_INT32(-500, drive.actual_velocity);
}

static void test_cia402_profile_torque_mode(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_torque = 1000};

    syn_cia402_init(&drive, &cfg);
    syn_cia402_set_controlword(&drive, 0x000FU); /* Enable Operation */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PT);

    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_torque(&drive, 350));
    syn_cia402_update(&drive, 10);

    TEST_ASSERT_EQUAL_INT16(350, drive.actual_torque);
    TEST_ASSERT_TRUE(drive.target_reached);
}

static void test_cia402_csp_csv_modes(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 2000};

    syn_cia402_init(&drive, &cfg);
    syn_cia402_set_controlword(&drive, 0x000FU);

    /* CSP Mode */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_CSP);
    syn_cia402_set_target_position(&drive, 43210);
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL_INT32(43210, drive.actual_position);

    /* CSV Mode */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_CSV);
    syn_cia402_set_target_velocity(&drive, -1200);
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL_INT32(-1200, drive.actual_velocity);

    /* CST Mode */
    syn_cia402_set_mode(&drive, SYN_CIA402_MODE_CST);
    syn_cia402_set_target_torque(&drive, 750);
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL_INT16(750, drive.actual_torque);

    /* Report actual feedback */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_report_actuals(&drive, 100, 200, 300));
    TEST_ASSERT_EQUAL_INT32(100, drive.actual_position);
    TEST_ASSERT_EQUAL_INT32(200, drive.actual_velocity);
    TEST_ASSERT_EQUAL_INT16(300, drive.actual_torque);
}

static void test_cia402_fault_handling_and_reset(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 1000};

    syn_cia402_init(&drive, &cfg);
    syn_cia402_set_controlword(&drive, 0x000FU);

    /* Trigger Overcurrent Fault (0x2310) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_trigger_fault(&drive, 0x2310U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_FAULT, drive.state);
    TEST_ASSERT_EQUAL_UINT16(0x2310U, drive.error_code);

    uint16_t sw = syn_cia402_get_statusword(&drive);
    TEST_ASSERT_TRUE((sw & SYN_CIA402_SW_FAULT) != 0);

    /* Try sending Enable Operation while in Fault -> stays in Fault */
    syn_cia402_set_controlword(&drive, 0x000FU);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_FAULT, drive.state);

    /* Send Fault Reset (Bit 7 rising edge: 0 -> 0x0080) */
    syn_cia402_set_controlword(&drive, 0x0080U);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);
    TEST_ASSERT_EQUAL_UINT16(0U, drive.error_code);

    /* Test Fault Reaction Active transition to Fault when stopped */
    drive.state = SYN_CIA402_STATE_FAULT_REACTION_ACTIVE;
    drive.actual_velocity = 0;
    drive.current_speed = 0.0f;
    syn_cia402_step(&drive, 1000, NULL);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_FAULT, drive.state);
}

static void test_cia402_canopen_od_binding(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config drive_cfg = {.max_profile_velocity = 1000};
    syn_cia402_init(&drive, &drive_cfg);

    SYN_CANOpenODEntry od_table[16];
    size_t od_count = syn_cia402_populate_od(od_table, 16, &drive);
    TEST_ASSERT_EQUAL(10, od_count);

    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig node_cfg = {.node_id = 1, .heartbeat_ms = 0};
    syn_cia402_set_controlword(&drive, 0x0000U);
    syn_canopen_init(&node, &node_cfg, od_table, od_count);

    /* Flush bootup Tx */
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* SDO Download: Write 0x000F to 0x6040:0x00 (Controlword) */
    uint8_t sdo_write_cw[8] = {0x2BU, 0x40U, 0x60U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x601U, sdo_write_cw, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len));
    TEST_ASSERT_EQUAL_HEX8(0x60U, dummy_buf[0]);

    /* Trigger Drive Controlword Update */
    syn_cia402_set_controlword(&drive, drive.controlword);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* SDO Upload: Read 0x6041:0x00 (Statusword) */
    uint8_t sdo_read_sw[8] = {0x40U, 0x41U, 0x60U, 0x00U, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x601U, sdo_read_sw, 8));

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x581U, tx_id);
    TEST_ASSERT_EQUAL(0x4BU, tx_buf[0]);
    uint16_t sw_read = (uint16_t)tx_buf[4] | ((uint16_t)tx_buf[5] << 8);
    TEST_ASSERT_EQUAL_HEX16(drive.statusword, sw_read);
}

static void test_cia402_invalid_params(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 1000};

    syn_cia402_init(&drive, &cfg);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_init(NULL, &cfg));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_init(&drive, NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_set_controlword(NULL, 0x000F));
    TEST_ASSERT_EQUAL_HEX16(0, syn_cia402_get_statusword(NULL));
    TEST_ASSERT_EQUAL_HEX16(0, syn_cia402_get_controlword(NULL));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_FAULT, syn_cia402_get_state(NULL));
    TEST_ASSERT_EQUAL(0, syn_cia402_get_mode(NULL));
    TEST_ASSERT_EQUAL(0, syn_cia402_get_mode_display(NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_set_mode(NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_set_target_position(NULL, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_set_target_velocity(NULL, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_set_target_torque(NULL, 100));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_update(NULL, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_step(NULL, 1000, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_report_actuals(NULL, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_feed_feedback(NULL, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_cia402_trigger_fault(NULL, 0x1000));

    SYN_CANOpenODEntry entries[16];
    TEST_ASSERT_EQUAL(0, syn_cia402_populate_od(NULL, 16, &drive));
    TEST_ASSERT_EQUAL(0, syn_cia402_populate_od(entries, 16, NULL));
    TEST_ASSERT_EQUAL(0, syn_cia402_populate_od(entries, 5, &drive));

    /* Branch coverage cases */
    drive.state = (SYN_CiA402State)99;
    drive.target_reached = false;
    drive.setpoint_ack = false;
    TEST_ASSERT_EQUAL_HEX16(0, syn_cia402_get_statusword(&drive));

    drive.state = SYN_CIA402_STATE_NOT_READY;
    drive.target_reached = false;
    TEST_ASSERT_EQUAL_HEX16(0, syn_cia402_get_statusword(&drive));

    /* Test NOT_READY transition to SWITCH_ON_DISABLED in eval_fsm */
    drive.state = SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON;
    syn_cia402_set_controlword(&drive, 0x0000U);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Test QUICK_STOP_ACTIVE transition to OPERATION_ENABLED if 0x000F */
    drive.state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
    syn_cia402_set_controlword(&drive, 0x000FU);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Test default invalid state in eval_fsm */
    drive.state = (SYN_CiA402State)99;
    syn_cia402_set_controlword(&drive, 0x0000U);

    /* Test FAULT_REACTION_ACTIVE with non-zero speed */
    drive.state = SYN_CIA402_STATE_FAULT_REACTION_ACTIVE;
    drive.actual_velocity = 100;
    drive.current_speed = 100.0f;
    syn_cia402_step(&drive, 1000, NULL);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_FAULT_REACTION_ACTIVE, drive.state);

    /* Test Quick Stop with zero speed */
    drive.state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
    drive.current_speed = 0.0f;
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    /* Test step with dt_us = 0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_step(&drive, 0, NULL));

    drive.state = SYN_CIA402_STATE_OPERATION_ENABLED;
    drive.mode_of_operation = 99;
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_update(&drive, 0)); /* dt_ms = 0 fallback */

    /* PV mode clamp checks */
    drive.mode_of_operation = SYN_CIA402_MODE_PV;
    drive.current_speed = 990.0f;
    drive.target_velocity = 1000;
    syn_cia402_update(&drive, 100);
    TEST_ASSERT_EQUAL_INT32(1000, drive.actual_velocity);

    drive.current_speed = -990.0f;
    drive.target_velocity = -1000;
    syn_cia402_update(&drive, 100);
    TEST_ASSERT_EQUAL_INT32(-1000, drive.actual_velocity);

    /* Quick Stop deceleration from non-zero speed */
    drive.state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
    drive.current_speed = 50.0f;
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);

    drive.state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
    drive.current_speed = -50.0f;
    syn_cia402_update(&drive, 10);
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCH_ON_DISABLED, drive.state);
}

static void test_r5_a17_quick_stop_reenable_and_fractional_step(void)
{
    SYN_CiA402Drive drive;
    SYN_CiA402Config cfg = {.max_profile_velocity = 500U,
                            .profile_acceleration = 5000U,
                            .profile_deceleration = 5000U,
                            .quick_stop_deceleration = 10000U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_init(&drive, &cfg));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Enter Quick Stop Active via 0x000B (bit 2 Quick Stop = 0, active-low) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000BU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);

    /* Re-writing 0x000B or toggling bit 4 (0x001B) while bit 2 == 0 MUST stay in QUICK_STOP */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x001BU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000BU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);

    /* Re-asserting Quick Stop bit 2 == 1 with Switch On + Enable Op (0x000F) re-enables */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_OPERATION_ENABLED, drive.state);

    /* Sub-count PP motion: 500 counts/s * 1000 us = 0.5 counts/tick -> advances 5 counts in 10ms */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PP));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_position(&drive, 5));
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_step(&drive, 1000U, NULL));
    }
    TEST_ASSERT_EQUAL_INT32(5, drive.actual_position);
    TEST_ASSERT_TRUE(drive.target_reached);

    /* External actual_position sync + sub-count PV motion (250 counts/s * 1000 us = 0.25/tick) */
    drive.actual_position = 100;
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PV));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_velocity(&drive, 250));
    drive.current_speed = 250.0f;
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_step(&drive, 1000U, NULL));
    }
    TEST_ASSERT_EQUAL_INT32(102, drive.actual_position);

    /* Also exercise non-zero residual speed during Quick Stop ramp (partial decel step) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000BU));
    drive.current_speed = 500.0f;
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_step(&drive, 1000U, NULL));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);
    drive.current_speed = -500.0f;
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_step(&drive, 1000U, NULL));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_QUICK_STOP_ACTIVE, drive.state);

    /* Exercise READY_TO_SWITCH_ON stay branch (0x0006 again) & SWITCHED_ON stay branch (0x0007) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0000U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_READY_TO_SWITCH_ON, drive.state);
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_CIA402_STATE_SWITCHED_ON, drive.state);
}

static void test_pro22_cia402_update_large_dt_ms_saturation(void)
{
    SYN_CIA402_Axis drive;
    SYN_CIA402_Config cfg = {
        .max_profile_velocity = 10000U,
        .profile_acceleration = 100000U,
        .profile_deceleration = 100000U,
        .quick_stop_deceleration = 100000U,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_init(&drive, &cfg));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_mode(&drive, SYN_CIA402_MODE_PV));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0006U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x0007U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_controlword(&drive, 0x000FU));
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_set_target_velocity(&drive, 10));
    drive.current_speed = 10.0f;

    /* dt_ms = 0 defaults to 1000 us */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_update(&drive, 0U));

    /* dt_ms = 5000000 (5e9 us > UINT32_MAX) saturates to UINT32_MAX (~4294.967s) -> > 40000 counts
     */
    TEST_ASSERT_EQUAL(SYN_OK, syn_cia402_update(&drive, 5000000U));
    TEST_ASSERT_TRUE(drive.actual_position > 40000);
}

void run_cia402_tests(void)
{
    RUN_TEST(test_cia402_init_and_state_transitions);
    RUN_TEST(test_cia402_profile_position_mode);
    RUN_TEST(test_cia402_profile_velocity_mode);
    RUN_TEST(test_cia402_profile_torque_mode);
    RUN_TEST(test_cia402_csp_csv_modes);
    RUN_TEST(test_cia402_fault_handling_and_reset);
    RUN_TEST(test_cia402_canopen_od_binding);
    RUN_TEST(test_cia402_invalid_params);
    RUN_TEST(test_r5_a17_quick_stop_reenable_and_fractional_step);
    RUN_TEST(test_pro22_cia402_update_large_dt_ms_saturation);
}
