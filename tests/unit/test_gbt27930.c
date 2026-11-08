/**
 * @file test_gbt27930.c
 * @brief Unit test suite for GB/T 27930 EV DC Fast Charging Protocol.
 */

#include "syntropic/proto/syn_gbt27930.h"
#include "unity/unity.h"

#include <string.h>

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930

static SYN_GBT27930_Session g_bms;
static SYN_GBT27930_Session g_charger;

static void gbt27930_test_setup(void)
{
    syn_gbt27930_init(&g_bms, SYN_GBT27930_ROLE_BMS);
    syn_gbt27930_init(&g_charger, SYN_GBT27930_ROLE_CHARGER);

    g_bms.bms_cfg.battery_type = 0x06U;
    g_bms.bms_cfg.rated_capacity_ah = 2000U;
    g_bms.bms_cfg.rated_voltage_v = 3600U;
    g_bms.bms_cfg.max_charge_volt_v = 4000U; /* 400.0V */
    g_bms.bms_cfg.max_charge_curr_a = 1000U; /* 100.0A */
    g_bms.bms_cfg.max_cell_volt_v = 420U;

    g_charger.charger_cfg.max_output_volt_v = 5000U; /* 500.0V */
    g_charger.charger_cfg.min_output_volt_v = 2000U; /* 200.0V */
    g_charger.charger_cfg.max_output_curr_a = 2500U;
    g_charger.charger_cfg.min_output_curr_a = 50U;
}

static void test_gbt27930_init_and_idle(void)
{
    gbt27930_test_setup();
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_IDLE, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_IDLE, g_charger.state);

    SYN_CAN_Frame frame;
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_bms, 100U, &frame));
}

static void test_gbt27930_handshake_flow(void)
{
    gbt27930_test_setup();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_start_handshake(&g_bms));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_start_handshake(&g_charger));

    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_charger.state);

    /* Step Charger -> produces CHM frame */
    SYN_CAN_Frame chm_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &chm_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &chm_frame));

    /* Step BMS -> produces BHM frame */
    SYN_CAN_Frame bhm_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 250U, &bhm_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bhm_frame));

    /* Step Charger -> produces CRM(0x00) */
    SYN_CAN_Frame crm00;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &crm00));
    TEST_ASSERT_EQUAL_HEX8(0x00U, crm00.data[0]);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &crm00));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_bms.state);

    /* Step BMS -> produces BRM */
    SYN_CAN_Frame brm;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 250U, &brm));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &brm));

    /* Step Charger -> produces CRM(0xAA) and enters PARAM_CONFIG */
    SYN_CAN_Frame crm_aa;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &crm_aa));
    TEST_ASSERT_EQUAL_HEX8(0xAAU, crm_aa.data[0]);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_PARAM_CONFIG, g_charger.state);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &crm_aa));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_PARAM_CONFIG, g_bms.state);
}

static void test_gbt27930_param_and_charging_loop(void)
{
    gbt27930_test_setup();
    g_bms.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    g_charger.state = SYN_GBT27930_STATE_PARAM_CONFIG;

    /* Ready exchange: CRO (Charger) and BRO (BMS) */
    g_bms.ready_for_charging = true;
    g_charger.ready_for_charging = true;

    SYN_CAN_Frame cro_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &cro_frame));

    SYN_CAN_Frame bro_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 250U, &bro_frame));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &cro_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bro_frame));

    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_charger.state);

    /* Active Charging Periodic Step */
    g_bms.telemetry.volt_demand_v = 3800U; /* 380.0V */
    g_bms.telemetry.curr_demand_a = 500U;  /* 50.0A */

    SYN_CAN_Frame bcl_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 50U, &bcl_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcl_frame));
    TEST_ASSERT_EQUAL_UINT16(3800U, g_charger.telemetry.volt_demand_v);

    g_charger.telemetry.measured_volt_v = 3795U;
    g_charger.telemetry.measured_curr_a = 498U;
    SYN_CAN_Frame ccs_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 50U, &ccs_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &ccs_frame));
    TEST_ASSERT_EQUAL_UINT16(3795U, g_bms.telemetry.measured_volt_v);
}

static void test_gbt27930_stop_and_error(void)
{
    gbt27930_test_setup();
    g_bms.state = SYN_GBT27930_STATE_CHARGING;
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    syn_gbt27930_stop_charging(&g_bms, 0x01U);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_STOPPING, g_bms.state);

    SYN_CAN_Frame bst_frame;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 10U, &bst_frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bst_frame));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_STOPPING, g_charger.state);
    TEST_ASSERT_EQUAL_UINT8(0x01U, g_charger.stop_reason);

    /* Error handling */
    SYN_CAN_Frame bem;
    memset(&bem, 0, sizeof(bem));
    bem.extended = true;
    bem.dlc = 8U;
    bem.id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_BEM, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bem.data[0] = 0xAAU;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bem));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_charger.state);
    TEST_ASSERT_EQUAL_UINT8(0xAAU, g_charger.fault_code);

    SYN_CAN_Frame err_tx;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &err_tx));
    SYN_J1939_Header hdr;
    syn_j1939_id_unpack(err_tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CEM, hdr.pgn);

    g_bms.state = SYN_GBT27930_STATE_ERROR;
    g_bms.fault_code = 0x11U;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 250U, &err_tx));

    g_charger.state = SYN_GBT27930_STATE_STOPPING;
    g_charger.peer_stopped = false;
    g_charger.stop_reason = 0x02U;
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_charger, 1U, &err_tx));
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 10U, &err_tx));

    g_bms.state = SYN_GBT27930_STATE_CHARGING;
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_bms, 1U, &err_tx));

    g_bms.state = SYN_GBT27930_STATE_STOPPING;
    g_bms.peer_stopped = false;
    g_bms.stop_reason = 0x03U;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 10U, &err_tx));

    g_bms.state = (SYN_GBT27930_State)99;
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_bms, 250U, &err_tx));
    g_charger.state = (SYN_GBT27930_State)99;
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_charger, 250U, &err_tx));
}

static void test_gbt27930_all_pgns_coverage(void)
{
    gbt27930_test_setup();

    /* BCP (BMS Charging Parameter) */
    g_charger.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    SYN_CAN_Frame bcp;
    memset(&bcp, 0, sizeof(bcp));
    bcp.extended = true;
    bcp.dlc = 8U;
    bcp.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCP, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bcp.data[0] = 0xE0U;
    bcp.data[1] = 0x0FU; /* 406.4V */
    bcp.data[2] = 0x64U;
    bcp.data[3] = 0x00U; /* 10.0A */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcp));

    /* CML (Charger Max Output) */
    g_bms.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    SYN_CAN_Frame cml;
    memset(&cml, 0, sizeof(cml));
    cml.extended = true;
    cml.dlc = 8U;
    cml.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CML, SYN_GBT27930_ADDR_CHARGER,
                               SYN_GBT27930_ADDR_BMS);
    cml.data[0] = 0x88U;
    cml.data[1] = 0x13U; /* 500.0V */
    cml.data[2] = 0xD0U;
    cml.data[3] = 0x07U; /* 200.0V */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &cml));

    /* BCS (BMS Overall Status) */
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    SYN_CAN_Frame bcs;
    memset(&bcs, 0, sizeof(bcs));
    bcs.extended = true;
    bcs.dlc = 8U;
    bcs.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCS, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bcs.data[0] = 0x90U;
    bcs.data[1] = 0x0FU; /* 398.4V */
    bcs.data[2] = 0x32U;
    bcs.data[3] = 0x00U; /* 5.0A */
    bcs.data[6] = 85U;   /* SOC 85% */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcs));
    TEST_ASSERT_EQUAL_UINT8(85U, g_charger.telemetry.soc_percent);

    /* CST & CEM processing */
    SYN_CAN_Frame cst;
    memset(&cst, 0, sizeof(cst));
    cst.extended = true;
    cst.dlc = 8U;
    cst.id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_CST, SYN_GBT27930_ADDR_CHARGER,
                               SYN_GBT27930_ADDR_BMS);
    cst.data[0] = 0x02U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &cst));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_STOPPING, g_bms.state);

    SYN_CAN_Frame cem;
    memset(&cem, 0, sizeof(cem));
    cem.extended = true;
    cem.dlc = 8U;
    cem.id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_CEM, SYN_GBT27930_ADDR_CHARGER,
                               SYN_GBT27930_ADDR_BMS);
    cem.data[0] = 0x55U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &cem));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_bms.state);
}

static void test_gbt27930_edge_cases_and_nulls(void)
{
    syn_gbt27930_init(NULL, SYN_GBT27930_ROLE_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gbt27930_start_handshake(NULL));
    syn_gbt27930_stop_charging(NULL, 0x01U);

    SYN_CAN_Frame frame;
    memset(&frame, 0, sizeof(frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gbt27930_process_rx_frame(NULL, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gbt27930_process_rx_frame(&g_bms, NULL));

    frame.extended = false;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_gbt27930_process_rx_frame(&g_bms, &frame));

    TEST_ASSERT_FALSE(syn_gbt27930_step(NULL, 10U, &frame));
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_bms, 10U, NULL));

    /* Unknown PGN frame */
    frame.extended = true;
    frame.id = syn_j1939_id_pack(6U, 0x00FF99U, SYN_GBT27930_ADDR_CHARGER, SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &frame));
}

static void test_gbt27930_timeouts(void)
{
    gbt27930_test_setup();

    /* 1. Handshake Phase Timeout (5,000ms limit) */
    syn_gbt27930_start_handshake(&g_bms);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_bms.state);

    SYN_CAN_Frame tx;
    syn_gbt27930_step(&g_bms, 4900U, &tx);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_bms.state);

    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_bms, 200U, &tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_bms.state);
    TEST_ASSERT_EQUAL_UINT8(0x01U, g_bms.fault_code);

    SYN_J1939_Header hdr;
    syn_j1939_id_unpack(tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_BEM, hdr.pgn);

    /* 2. Charging Phase Timeout (1,000ms limit) */
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    syn_gbt27930_step(&g_charger, 900U, &tx);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_charger.state);

    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 200U, &tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_charger.state);
    TEST_ASSERT_EQUAL_UINT8(0x01U, g_charger.fault_code);

    syn_j1939_id_unpack(tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CEM, hdr.pgn);
}

static void test_gbt27930_short_dlc_branches(void)
{
    gbt27930_test_setup();
    SYN_CAN_Frame f;
    memset(&f, 0, sizeof(f));
    f.extended = true;
    f.dlc = 1U;

    /* BHM dlc < 2 & BRM dlc < 8 */
    g_charger.state = SYN_GBT27930_STATE_HANDSHAKE;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BHM, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRM, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));

    /* CRM data != 0x00 && != 0xAA, and dlc = 0 */
    g_bms.state = SYN_GBT27930_STATE_HANDSHAKE;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    f.data[0] = 0x55U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.dlc = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CHM, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.dlc = 1U;

    /* BCP dlc < 6 */
    g_charger.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCP, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));

    /* CTS dlc < 7 & CML dlc < 8 */
    g_bms.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CTS, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CML, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));

    /* BRO data == 0x00, data == 0x55, dlc == 0 */
    g_charger.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRO, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    f.data[0] = 0x00U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.data[0] = 0x55U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.dlc = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.dlc = 1U;

    /* CRO data == 0x00, data == 0x55, dlc == 0 */
    g_bms.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRO, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    f.data[0] = 0x00U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.data[0] = 0x55U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.dlc = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    f.dlc = 1U;

    /* BCL dlc < 5, BCS dlc < 7, CCS dlc < 6 */
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCL, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCS, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    g_bms.state = SYN_GBT27930_STATE_CHARGING;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CCS, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));

    /* BSD dlc < 7 & CSD dlc < 4 */
    g_charger.state = SYN_GBT27930_STATE_END;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BSD, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    g_bms.state = SYN_GBT27930_STATE_END;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CSD, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));

    /* BST / BEM dlc = 0, plus BST/BEM in IDLE/END */
    f.dlc = 0U;
    g_charger.state = SYN_GBT27930_STATE_IDLE;
    f.id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_BST, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_BEM, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));

    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    f.id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_BST, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
    f.id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_BEM, SYN_GBT27930_ADDR_BMS,
                             SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &f));
}

static void test_r5_a01_full_loopback_and_timeout_isolation(void)
{
    gbt27930_test_setup();
    g_bms.telemetry.volt_demand_v = 4000U;
    g_bms.telemetry.curr_demand_a = 800U;
    g_bms.telemetry.measured_volt_v = 3990U;
    g_bms.telemetry.measured_curr_a = 795U;
    g_bms.telemetry.soc_percent = 50U;
    g_bms.telemetry.max_cell_volt_v = 415U;
    g_bms.telemetry.min_cell_temp_c = 70U;
    g_bms.telemetry.max_cell_temp_c = 80U;
    g_charger.telemetry.measured_volt_v = 3992U;
    g_charger.telemetry.measured_curr_a = 796U;
    g_charger.telemetry.cumulative_time_min = 42U;
    g_charger.telemetry.output_energy_kwh = 185U; /* 18.5 kWh */
    g_charger.sync_time_bcd[5] = 0x26U;
    g_charger.sync_time_bcd[6] = 0x20U;

    /* Start both nodes from IDLE -> HANDSHAKE */
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_IDLE, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_IDLE, g_charger.state);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_start_handshake(&g_bms));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_start_handshake(&g_charger));

    /* Pure back-to-back loopback: HANDSHAKE -> PARAM_CONFIG (BRO/CRO 0x00) -> app ready -> CHARGING
     */
    SYN_CAN_Frame c_tx;
    SYN_CAN_Frame b_tx;
    bool saw_bro00 = false;
    bool saw_cro00 = false;
    for (int tick = 0; tick < 50; tick++) {
        bool has_c = syn_gbt27930_step(&g_charger, 50U, &c_tx);
        bool has_b = syn_gbt27930_step(&g_bms, 50U, &b_tx);
        if (has_c) {
            SYN_J1939_Header ch;
            syn_j1939_id_unpack(c_tx.id, &ch);
            if (ch.pgn == SYN_GBT27930_PGN_CRO && c_tx.data[0] == 0x00U) {
                saw_cro00 = true;
            }
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &c_tx));
        }
        if (has_b) {
            SYN_J1939_Header bh;
            syn_j1939_id_unpack(b_tx.id, &bh);
            if (bh.pgn == SYN_GBT27930_PGN_BRO && b_tx.data[0] == 0x00U) {
                saw_bro00 = true;
            }
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &b_tx));
        }
        if (saw_bro00) {
            g_bms.ready_for_charging = true;
        }
        if (saw_cro00) {
            g_charger.ready_for_charging = true;
        }
        if (g_bms.state == SYN_GBT27930_STATE_CHARGING &&
            g_charger.state == SYN_GBT27930_STATE_CHARGING) {
            break;
        }
    }
    TEST_ASSERT_TRUE(saw_bro00);
    TEST_ASSERT_TRUE(saw_cro00);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_charger.state);
    TEST_ASSERT_TRUE(g_bms.cts_received);
    TEST_ASSERT_EQUAL_HEX8(0x20U, g_bms.sync_time_bcd[6]);
    TEST_ASSERT_TRUE(g_bms.cml_received);
    TEST_ASSERT_TRUE(g_charger.brm_received);
    TEST_ASSERT_TRUE(g_charger.bcp_received);

    /* Run several CHARGING ticks (25 ms BMS BCL cadence) so BCL, BCS, and CCS all exchange */
    for (int tick = 0; tick < 4; tick++) {
        if (syn_gbt27930_step(&g_bms, 25U, &b_tx)) {
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &b_tx));
        }
        if (syn_gbt27930_step(&g_charger, 50U, &c_tx)) {
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &c_tx));
        }
    }
    TEST_ASSERT_EQUAL_UINT16(4000U, g_charger.telemetry.volt_demand_v);
    TEST_ASSERT_EQUAL_UINT8(50U, g_charger.telemetry.soc_percent);
    TEST_ASSERT_EQUAL_UINT16(3992U, g_bms.telemetry.measured_volt_v);
    TEST_ASSERT_EQUAL_UINT16(42U, g_bms.telemetry.cumulative_time_min);

    /* Stop charging -> STOPPING -> END (with BSD & CSD statistics exchange) -> bounded IDLE */
    g_bms.telemetry.soc_percent = 95U;
    syn_gbt27930_stop_charging(&g_bms, 0x01U);
    for (int tick = 0; tick < 20; tick++) {
        bool has_b = syn_gbt27930_step(&g_bms, 50U, &b_tx);
        bool has_c = syn_gbt27930_step(&g_charger, 50U, &c_tx);
        if (has_b) {
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &b_tx));
        }
        if (has_c) {
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &c_tx));
        }
        if (g_bms.stats_received && g_charger.stats_received) {
            break;
        }
    }
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_END, g_bms.state);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_END, g_charger.state);
    TEST_ASSERT_TRUE(g_bms.stats_received);
    TEST_ASSERT_TRUE(g_charger.stats_received);
    TEST_ASSERT_EQUAL_UINT8(95U, g_charger.telemetry.soc_percent);
    TEST_ASSERT_EQUAL_UINT16(185U, g_bms.telemetry.output_energy_kwh);

    /* Bounded STATE_END transitions to IDLE once final stats frames have been sent */
    (void)syn_gbt27930_step(&g_bms, 250U, &b_tx);
    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_bms, 250U, &b_tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_IDLE, g_bms.state);

    /* Also cover default CTS (0xFF), Charger END CST substep, and END timeout -> ERROR */
    SYN_GBT27930_Session c2;
    SYN_GBT27930_Session b2;
    syn_gbt27930_init(&c2, SYN_GBT27930_ROLE_CHARGER);
    syn_gbt27930_init(&b2, SYN_GBT27930_ROLE_BMS);
    c2.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    b2.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 250U, &c_tx)); /* CTS (default 0xFF) */
    TEST_ASSERT_EQUAL_HEX8(0xFFU, c_tx.data[6]);
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 250U, &c_tx)); /* CML -> cml_sent = true */
    TEST_ASSERT_TRUE(syn_gbt27930_step(&b2, 250U, &b_tx)); /* BCP */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&c2, &b_tx));
    TEST_ASSERT_FALSE(c2.ready_for_charging);
    c2.ready_for_charging = true;
    /* Peer ready arriving BEFORE local ready_tx_sent -> transitions on local BRO/CRO TX */
    c2.peer_ready_for_charging = true;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 250U, &c_tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, c2.state);

    b2.ready_for_charging = true;
    b2.peer_ready_for_charging = true;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&b2, 250U, &b_tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, b2.state);

    /* Charger END substep=1 before stats_received emits CST; 5s timeout enters ERROR */
    c2.state = SYN_GBT27930_STATE_END;
    c2.stats_received = false;
    c2.timeout_ms = 0U;
    c2.tx_substep = 1U;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 250U, &c_tx));
    SYN_J1939_Header hdr;
    syn_j1939_id_unpack(c_tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CST, hdr.pgn);
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 5000U, &c_tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, c2.state);

    /* Charger initiated stop -> BMS receives CST while already having sent BST */
    syn_gbt27930_stop_charging(&c2, 0x02U);
    TEST_ASSERT_TRUE(syn_gbt27930_step(&c2, 10U, &c_tx)); /* CST */
    syn_gbt27930_stop_charging(&b2, 0x01U);
    b2.peer_stopped = true;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&b2, 10U, &b_tx)); /* BST -> b2 enters END */
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_END, b2.state);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&c2, &b_tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_END, c2.state);

    /* Timeout isolation: random/wrong-state CAN frames MUST NOT reset timeout_ms */
    syn_gbt27930_init(&b2, SYN_GBT27930_ROLE_BMS);
    syn_gbt27930_start_handshake(&b2);
    SYN_CAN_Frame noise;
    memset(&noise, 0, sizeof(noise));
    noise.extended = true;
    noise.dlc = 8U;
    noise.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CCS, SYN_GBT27930_ADDR_CHARGER,
                                 SYN_GBT27930_ADDR_BMS);
    for (int i = 0; i < 11; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&b2, &noise));
        (void)syn_gbt27930_step(&b2, 500U, &b_tx);
    }
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, b2.state);
    TEST_ASSERT_EQUAL_UINT8(0x01U, b2.fault_code);
}

static void test_tier_a_gbt27930_regressions(void)
{
    SYN_CAN_Frame f;
    SYN_CAN_Frame tx;
    SYN_J1939_Header hdr;

    /* 1. GBT-1 & GBT-3: J1939 SA/DA filtering + CRM(0x00) prerequisite before CRM(0xAA) */
    gbt27930_test_setup();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_start_handshake(&g_bms));

    memset(&f, 0, sizeof(f));
    f.extended = true;
    f.dlc = 8U;

    /* Premature CRM(0xAA) before CRM(0x00) rejected */
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, SYN_GBT27930_ADDR_CHARGER,
                             SYN_GBT27930_ADDR_BMS);
    f.data[0] = 0xAAU;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_HANDSHAKE, g_bms.state);

    /* Spoofed SA (0x99) and mismatched DA (0x12) ignored; broadcast DA (0xFF) accepted */
    f.data[0] = 0x00U;
    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, 0x99U, SYN_GBT27930_ADDR_BMS);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    TEST_ASSERT_FALSE(g_bms.crm00_received);

    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, SYN_GBT27930_ADDR_CHARGER, 0x12U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    TEST_ASSERT_FALSE(g_bms.crm00_received);

    f.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, SYN_GBT27930_ADDR_CHARGER, 0xFFU);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
    TEST_ASSERT_TRUE(g_bms.crm00_received);

    /* Repeated CRM(0x00) does NOT reset 5s recognition timeout */
    for (int i = 0; i < 11; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &f));
        (void)syn_gbt27930_step(&g_bms, 500U, &tx);
    }
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_bms.state);

    /* 2. GBT-3: Duplicate BRM in PARAM_CONFIG triggers CRM(0xAA) retransmit; CTS/CML alternate
     * until BRO */
    gbt27930_test_setup();
    g_charger.state = SYN_GBT27930_STATE_PARAM_CONFIG;

    SYN_CAN_Frame brm;
    memset(&brm, 0, sizeof(brm));
    brm.extended = true;
    brm.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRM, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    brm.dlc = 4U; /* Short BRM ignored */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &brm));
    TEST_ASSERT_FALSE(g_charger.crm_aa_retransmit);

    brm.dlc = 8U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &brm));
    TEST_ASSERT_TRUE(g_charger.crm_aa_retransmit);
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx));
    syn_j1939_id_unpack(tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CRM, hdr.pgn);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, tx.data[0]);

    /* Receive BCP (unpacking max_cell_volt_v, max_temp_c, soc_percent) + repeated BCP/BRO(0x00) */
    SYN_CAN_Frame bcp;
    memset(&bcp, 0, sizeof(bcp));
    bcp.extended = true;
    bcp.dlc = 8U;
    bcp.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCP, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bcp.data[4] = 0xA4U;
    bcp.data[5] = 0x01U; /* 420 (4.20V) */
    bcp.data[6] = 95U;
    bcp.data[7] = 62U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcp));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcp));
    TEST_ASSERT_EQUAL_UINT16(420U, g_charger.bms_cfg.max_cell_volt_v);
    TEST_ASSERT_EQUAL_UINT16(95U, g_charger.bms_cfg.max_temp_c);
    TEST_ASSERT_EQUAL_UINT8(62U, g_charger.telemetry.soc_percent);

    /* Before BRO arrives, Charger keeps alternating CTS and CML even after bcp_received && cml_sent
     */
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx)); /* CTS */
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx)); /* CML -> cml_sent = true */
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx)); /* Still CTS (no BRO yet!) */
    syn_j1939_id_unpack(tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CTS, hdr.pgn);

    /* Repeated CML on BMS & repeated BRO(0x00) on Charger */
    g_bms.state = SYN_GBT27930_STATE_PARAM_CONFIG;
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx)); /* CML */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &tx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_bms, &tx));

    SYN_CAN_Frame bro00;
    memset(&bro00, 0, sizeof(bro00));
    bro00.extended = true;
    bro00.dlc = 1U;
    bro00.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRO, SYN_GBT27930_ADDR_BMS,
                                 SYN_GBT27930_ADDR_CHARGER);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bro00));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bro00));
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 250U, &tx)); /* Now emits CRO(0x00) */
    syn_j1939_id_unpack(tx.id, &hdr);
    TEST_ASSERT_EQUAL_HEX32(SYN_GBT27930_PGN_CRO, hdr.pgn);

    /* 3. GBT-2 & GBT-4: Separate BCL (1s) vs BCS (5s) watchdogs, BSM/BCS fields, sub-period timer &
     * UINT32_MAX saturation */
    gbt27930_test_setup();
    g_charger.state = SYN_GBT27930_STATE_CHARGING;

    SYN_CAN_Frame bcl;
    memset(&bcl, 0, sizeof(bcl));
    bcl.extended = true;
    bcl.dlc = 5U;
    bcl.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCL, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);

    /* Continuous BCL without BCS stays alive past 1000ms (up to 4950ms), then faults at 5000ms */
    for (int i = 0; i < 11; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcl));
        (void)syn_gbt27930_step(&g_charger, 450U, &tx);
        TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_CHARGING, g_charger.state);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcl));
    (void)syn_gbt27930_step(&g_charger, 100U, &tx);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_charger.state);

    /* Continuous BCS without BCL faults at 1000ms */
    gbt27930_test_setup();
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    SYN_CAN_Frame bcs;
    memset(&bcs, 0, sizeof(bcs));
    bcs.extended = true;
    bcs.dlc = 8U;
    bcs.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCS, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bcs.data[4] = 0x9FU;
    bcs.data[5] = 0x01U; /* 415 */
    bcs.data[7] = 33U;   /* 33 min remaining */
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bcs));
        (void)syn_gbt27930_step(&g_charger, 200U, &tx);
    }
    TEST_ASSERT_EQUAL_UINT16(415U, g_charger.telemetry.max_cell_volt_v);
    TEST_ASSERT_EQUAL_UINT16(33U, g_charger.telemetry.remaining_time_min);
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_charger.state);

    /* BSM short DLC ignored, valid 7B BSM unpacked; sub-interval step + UINT32_MAX saturation */
    gbt27930_test_setup();
    g_charger.state = SYN_GBT27930_STATE_CHARGING;
    SYN_CAN_Frame bsm;
    memset(&bsm, 0, sizeof(bsm));
    bsm.extended = true;
    bsm.id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BSM, SYN_GBT27930_ADDR_BMS,
                               SYN_GBT27930_ADDR_CHARGER);
    bsm.dlc = 3U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bsm));
    bsm.dlc = 7U;
    bsm.data[1] = 82U;
    bsm.data[3] = 74U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_gbt27930_process_rx_frame(&g_charger, &bsm));
    TEST_ASSERT_EQUAL_UINT8(82U, g_charger.telemetry.max_cell_temp_c);
    TEST_ASSERT_EQUAL_UINT8(74U, g_charger.telemetry.min_cell_temp_c);

    TEST_ASSERT_FALSE(syn_gbt27930_step(&g_charger, 30U, &tx));
    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, 30U, &tx));
    TEST_ASSERT_EQUAL_UINT32(10U, g_charger.timer_ms);

    TEST_ASSERT_TRUE(syn_gbt27930_step(&g_charger, UINT32_MAX, &tx));
    TEST_ASSERT_EQUAL_INT(SYN_GBT27930_STATE_ERROR, g_charger.state);
}

void run_gbt27930_tests(void)
{
    RUN_TEST(test_gbt27930_init_and_idle);
    RUN_TEST(test_gbt27930_handshake_flow);
    RUN_TEST(test_gbt27930_param_and_charging_loop);
    RUN_TEST(test_gbt27930_stop_and_error);
    RUN_TEST(test_gbt27930_all_pgns_coverage);
    RUN_TEST(test_gbt27930_edge_cases_and_nulls);
    RUN_TEST(test_gbt27930_timeouts);
    RUN_TEST(test_gbt27930_short_dlc_branches);
    RUN_TEST(test_r5_a01_full_loopback_and_timeout_isolation);
    RUN_TEST(test_tier_a_gbt27930_regressions);
}

#endif /* SYN_USE_GBT27930 */
