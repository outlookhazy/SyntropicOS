/**
 * @file test_canopen.c
 * @brief Unity unit tests for CANopen DS301 Slave Protocol Engine.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_canopen.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static uint32_t od_device_type = 0x00020192UL;
static uint16_t od_target_speed = 0;
static uint8_t od_node_status = 1;
static uint8_t od_wo_val = 0;
static uint64_t od_large_val = 0x1122334455667788ULL;

static const SYN_CANOpenODEntry test_od[] = {
    {0x1000U, 0x00U, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RO, &od_device_type,
     sizeof(od_device_type)},
    {0x2001U, 0x01U, SYN_CANOPEN_TYPE_U16, SYN_CANOPEN_ACCESS_RW, &od_target_speed,
     sizeof(od_target_speed)},
    {0x2001U, 0x02U, SYN_CANOPEN_TYPE_U8, SYN_CANOPEN_ACCESS_RO, &od_node_status,
     sizeof(od_node_status)},
    {0x2002U, 0x00U, SYN_CANOPEN_TYPE_U8, SYN_CANOPEN_ACCESS_WO, &od_wo_val, sizeof(od_wo_val)},
    {0x2003U, 0x00U, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RW, &od_large_val,
     sizeof(od_large_val)}};

static void test_canopen_init_and_bootup(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5,
                                 .heartbeat_ms = 1000,
                                 .rpdo = {{0x205U, 0x2001U, 0x01U, 1U}},
                                 .tpdo = {{0x185U, 0x2001U, 0x01U, 1U}}};

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0])));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    /* Check Bootup Tx message */
    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0};
    uint8_t tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id);
    TEST_ASSERT_EQUAL(1, tx_len);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[0]);
}

static void test_canopen_nmt_state_machine(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* NMT Start -> Operational */
    uint8_t nmt_start[2] = {0x01U, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_start, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_OPERATIONAL, node.nmt_state);

    /* NMT Stop -> Stopped */
    uint8_t nmt_stop[2] = {0x02U, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_stop, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_STOPPED, node.nmt_state);

    /* NMT PreOp -> Pre-Operational */
    uint8_t nmt_preop[2] = {0x80U, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_preop, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    /* NMT Reset Node -> PreOp + Bootup frame */
    uint8_t nmt_reset[2] = {0x81U, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_reset, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id);
}

static void test_canopen_sdo_expedited_read_write(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* SDO Download Request: Write 1500 (0x05DC) to 0x2001:0x01 */
    uint8_t sdo_write_req[8] = {0x2BU, 0x01U, 0x20U, 0x01U, 0xDCU, 0x05U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_write_req, 8));

    TEST_ASSERT_EQUAL_UINT16(1500, od_target_speed);

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(8, tx_len);
    TEST_ASSERT_EQUAL(0x60U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[1]);
    TEST_ASSERT_EQUAL(0x20U, tx_buf[2]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[3]);

    /* SDO Upload Request: Read 0x2001:0x01 */
    uint8_t sdo_read_req[8] = {0x40U, 0x01U, 0x20U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_read_req, 8));

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(8, tx_len);
    TEST_ASSERT_EQUAL(0x4BU, tx_buf[0]); /* 0x43 | (2 << 2) = 0x4B (2 bytes payload) */
    TEST_ASSERT_EQUAL(0xDCU, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x05U, tx_buf[5]);
}

static void test_canopen_sdo_abort_codes(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* 1. Non-existent index 0x9999 */
    uint8_t sdo_invalid_index[8] = {0x40U, 0x99U, 0x99U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_invalid_index, 8);

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x02U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06020000 Object does not exist */

    /* 2. Non-existent subindex 0x2001:0x99 */
    uint8_t sdo_invalid_subidx[8] = {0x40U, 0x01U, 0x20U, 0x99U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_invalid_subidx, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x11U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x09U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06090011 Subindex does not exist */

    /* 3. Write to Read-Only entry 0x1000:0x00 */
    uint8_t sdo_write_ro[8] = {0x23U, 0x00U, 0x10U, 0x00U, 0x11U, 0x22U, 0x33U, 0x44U};
    syn_canopen_process_rx(&node, 0x605U, sdo_write_ro, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x02U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06010002 Read-only */

    /* 4. Write with invalid length to 0x2001:0x01 */
    uint8_t sdo_write_mismatch[8] = {0x23U, 0x01U, 0x20U, 0x01U, 0x11U, 0x22U, 0x33U, 0x44U};

    /* Expedited SDO read of 1-byte entry 0x1001:0x00 (line 321) */
    uint8_t sdo_read_1byte[8] = {0x40U, 0x01U, 0x10U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_read_1byte, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    syn_canopen_process_rx(&node, 0x605U, sdo_write_mismatch, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x10U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x07U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06070010 Type Mismatch */

    /* 5. SDO Write to non-existent index */
    uint8_t sdo_w_no_idx[8] = {0x2BU, 0x99U, 0x99U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_w_no_idx, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06020000UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5] << 8) |
                                        ((uint32_t)tx_buf[6] << 16) | ((uint32_t)tx_buf[7] << 24));

    /* 6. SDO Write to non-existent subindex */
    uint8_t sdo_w_no_sub[8] = {0x2BU, 0x01U, 0x20U, 0x99U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_w_no_sub, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06090011UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5] << 8) |
                                        ((uint32_t)tx_buf[6] << 16) | ((uint32_t)tx_buf[7] << 24));

    /* 7. SDO Read from Write-Only entry 0x2002:0x00 */
    uint8_t sdo_r_wo[8] = {0x40U, 0x02U, 0x20U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_r_wo, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06010001UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5] << 8) |
                                        ((uint32_t)tx_buf[6] << 16) | ((uint32_t)tx_buf[7] << 24));

    /* 8. SDO Read from 8-byte entry 0x2003:0x00 initiates Segmented Upload */
    uint8_t sdo_r_large[8] = {0x40U, 0x03U, 0x20U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, sdo_r_large, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x41U, tx_buf[0]); /* 0x41 = Initiate Segmented Upload */

    /* 9. Unknown NMT cmd and unknown COB-ID */
    uint8_t unknown_nmt[2] = {0xFFU, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, unknown_nmt, 2));

    uint8_t unknown_cob[2] = {0x01U, 0x02U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x333U, unknown_cob, 2));
}

static void test_canopen_rpdo_tpdo_emcy(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5,
                                 .heartbeat_ms = 500,
                                 .rpdo = {{0x205U, 0x2001U, 0x01U, 1U}},
                                 .tpdo = {{0x185U, 0x2001U, 0x01U, 1U}}};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* Switch to Operational Mode */
    uint8_t nmt_start[2] = {0x01U, 0x05U};
    syn_canopen_process_rx(&node, 0x000U, nmt_start, 2);

    /* Send RPDO1 frame: COB-ID = 0x205, Payload = { 0x20, 0x03 } (800) */
    uint8_t rpdo1[2] = {0x20U, 0x03U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x205U, rpdo1, 2));
    TEST_ASSERT_EQUAL_UINT16(800, od_target_speed);

    /* Periodic Update for TPDO & Heartbeat (both queued in TX FIFO on same tick) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_update(&node, 500));

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id);    /* Heartbeat */
    TEST_ASSERT_EQUAL(0x05U, tx_buf[0]); /* Operational state */

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x185U, tx_id); /* TPDO1 */
    TEST_ASSERT_EQUAL(2, tx_len);
    TEST_ASSERT_EQUAL(0x20U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x03U, tx_buf[1]);

    /* Send EMCY message */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_send_emcy(&node, 0x1000U, 0x01U));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x085U, tx_id); /* EMCY */
    TEST_ASSERT_EQUAL(8, tx_len);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x10U, tx_buf[1]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[2]);

    /* TPDO Trigger error checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_tpdo_trigger(NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_tpdo_trigger(&node, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_tpdo_trigger(&node, 5));

    /* TPDO Trigger when disabled/unmapped */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_tpdo_trigger(&node, 2)); /* TPDO2 disabled */

    /* TPDO Trigger when not in Operational state */
    node.nmt_state = SYN_CANOPEN_NMT_STATE_PREOP;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_tpdo_trigger(&node, 1));
}

static void test_canopen_invalid_params(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 1000};
    uint8_t dummy_buf[8] = {0};
    size_t dummy_len = 0;

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(NULL, &cfg, test_od, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(&node, NULL, test_od, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(&node, &cfg, NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(&node, &cfg, test_od, 0));

    cfg.node_id = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(&node, &cfg, test_od, 1));
    cfg.node_id = 128;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_init(&node, &cfg, test_od, 1));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_process_rx(NULL, 0, dummy_buf, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_process_rx(&node, 0, NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_process_rx(&node, 0, dummy_buf, 0));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_update(NULL, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_send_emcy(NULL, 0x1000, 1));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_canopen_od_read(NULL, 0x1000, 0, dummy_buf, 4, &dummy_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_canopen_od_read(&node, 0x1000, 0, NULL, 4, &dummy_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_read(&node, 0x1000, 0, dummy_buf, 4, NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_write(NULL, 0x1000, 0, dummy_buf, 4));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_write(&node, 0x1000, 0, NULL, 4));

    TEST_ASSERT_FALSE(syn_canopen_get_tx(NULL, NULL, NULL, NULL));

    /* Direct API error cases */
    node.od_table = NULL;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_read(&node, 0x1000, 0, dummy_buf, 4, &dummy_len));

    node.od_table = test_od;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_read(&node, 0x9999, 0, dummy_buf, 4, &dummy_len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_read(&node, 0x2002, 0, dummy_buf, 4, &dummy_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_canopen_od_read(&node, 0x1000, 0, dummy_buf, 1, &dummy_len));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_write(&node, 0x9999, 0, dummy_buf, 4));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_write(&node, 0x1000, 0, dummy_buf, 4));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_write(&node, 0x2001, 1, dummy_buf, 1));
}

#include "syntropic/proto/syn_cia303.h"

static void test_cia303_indicators(void)
{
    SYN_LED run_led, err_led;
    syn_led_init(&run_led, 1, SYN_LED_ACTIVE_HIGH);
    syn_led_init(&err_led, 2, SYN_LED_ACTIVE_HIGH);

    SYN_CiA303_Indicator ind;
    syn_cia303_init(&ind, &run_led, &err_led);

    TEST_ASSERT_EQUAL(SYN_CIA303_RUN_OFF, ind.run_state);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_OFF, ind.err_state);

    /* Test NMT Operational -> Solid ON */
    syn_cia303_set_nmt_state(&ind, 0x05U);
    TEST_ASSERT_EQUAL(SYN_CIA303_RUN_SOLID_ON, ind.run_state);
    TEST_ASSERT_TRUE(run_led.lit);

    /* Test NMT Pre-Op -> Blinking */
    syn_cia303_set_nmt_state(&ind, 0x7FU);
    TEST_ASSERT_EQUAL(SYN_CIA303_RUN_BLINKING, ind.run_state);

    /* Test NMT Stopped -> Single Flash */
    syn_cia303_set_nmt_state(&ind, 0x04U);
    TEST_ASSERT_EQUAL(SYN_CIA303_RUN_SINGLE_FLASH, ind.run_state);

    /* Test Error states */
    syn_cia303_set_error_state(&ind, SYN_CIA303_ERR_SOLID_ON);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_SOLID_ON, ind.err_state);
    TEST_ASSERT_TRUE(err_led.lit);

    syn_cia303_set_error_state(&ind, SYN_CIA303_ERR_SINGLE_FLASH);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_SINGLE_FLASH, ind.err_state);

    syn_cia303_set_error_state(&ind, SYN_CIA303_ERR_DOUBLE_FLASH);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_DOUBLE_FLASH, ind.err_state);

    syn_cia303_set_error_state(&ind, SYN_CIA303_ERR_TRIPLE_FLASH);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_TRIPLE_FLASH, ind.err_state);

    /* Test NMT default / unknown state -> OFF */
    syn_cia303_set_nmt_state(&ind, 0x00U);
    TEST_ASSERT_EQUAL(SYN_CIA303_RUN_OFF, ind.run_state);

    /* Test ERR OFF */
    syn_cia303_set_error_state(&ind, SYN_CIA303_ERR_OFF);
    TEST_ASSERT_EQUAL(SYN_CIA303_ERR_OFF, ind.err_state);

    /* Step simulation */
    syn_cia303_step(&ind);

    /* Test NULL LED guards */
    SYN_CiA303_Indicator null_ind;
    syn_cia303_init(&null_ind, NULL, NULL);
    syn_cia303_set_nmt_state(&null_ind, 0x05U);
    syn_cia303_set_error_state(&null_ind, SYN_CIA303_ERR_SOLID_ON);
    syn_cia303_step(&null_ind);
}

static void test_canopen_tpdo_trigger(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {
        .node_id = 5, .heartbeat_ms = 0, .tpdo = {{0x185U, 0x2001U, 0x01U, 1U}}};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* In PreOp, trigger should return SYN_ERROR */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_tpdo_trigger(&node, 1));

    /* NMT Start -> Operational */
    uint8_t nmt_start[2] = {0x01U, 0x05U};
    syn_canopen_process_rx(&node, 0x000U, nmt_start, 2);

    /* Trigger TPDO 1 */
    od_target_speed = 1200;
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_tpdo_trigger(&node, 1));

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x185U, tx_id);
    TEST_ASSERT_EQUAL(2, tx_len);
    TEST_ASSERT_EQUAL_UINT16(1200, (uint16_t)(tx_buf[0] | (tx_buf[1] << 8)));
}

static void test_canopen_sdo_segmented_transfer(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id;
    uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* 1. Test Segmented Upload of 0x2003:0x00 (8-byte od_large_val) */
    uint8_t upload_init[8] = {0x40U, 0x03U, 0x20U, 0x00U, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, upload_init, 8));

    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(0x41U, tx_buf[0]);
    TEST_ASSERT_EQUAL(8, tx_buf[4]); /* Total size = 8 bytes */

    /* Segment 0 Upload Request */
    uint8_t seg0_req[8] = {0x60U, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg0_req, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[0]); /* t=0, n=0 (7 bytes), c=0 */
    TEST_ASSERT_EQUAL(0x88U, tx_buf[1]);

    /* Segment 1 Upload Request (t=1) */
    uint8_t seg1_req[8] = {0x70U, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg1_req, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(0x1DU, tx_buf[0]); /* t=1, n=6 (1 byte remaining), c=1 (last segment) */
    TEST_ASSERT_EQUAL(0x11U, tx_buf[1]);

    /* 2. Test Segmented Download of 0x2003:0x00 (8-byte od_large_val) */
    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    uint8_t dn_init[8] = {0x20U, 0x03U, 0x20U, 0x00U, 8, 0, 0, 0}; /* e=0, s=1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, dn_init, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x60U, tx_buf[0]);

    /* Segment 0 Download Request (7 bytes) */
    uint8_t seg0_dn[8] = {0x00U, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg0_dn, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x20U, tx_buf[0]); /* t=0 ack */

    /* Segment 1 Download Request (1 byte, c=1, n=6) */
    uint8_t seg1_dn[8] = {0x1DU, 0x22, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg1_dn, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    const uint8_t *val_bytes = (const uint8_t *)&od_large_val;
    TEST_ASSERT_EQUAL_HEX8(0xAA, val_bytes[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, val_bytes[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, val_bytes[2]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, val_bytes[3]);
    TEST_ASSERT_EQUAL_HEX8(0xEE, val_bytes[4]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, val_bytes[5]);
    TEST_ASSERT_EQUAL_HEX8(0x11, val_bytes[6]);
    TEST_ASSERT_EQUAL_HEX8(0x22, val_bytes[7]);
}

static void test_canopen_sdo_segmented_toggle_bit_mismatch(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};
    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));

    uint32_t tx_id;
    uint8_t tx_buf[8], tx_len;
    (void)syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len); /* Drain bootup frame */

    /* Initiate segmented upload of 0x2003:0x00 */
    uint8_t up_init[8] = {0x40U, 0x03U, 0x20U, 0x00U, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, up_init, 8));
    syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len);

    /* Send Upload Segment Request with incorrect toggle bit t=1 (expected t=0) */
    uint8_t seg_req_bad_t[8] = {0x70U, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg_req_bad_t, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]); /* Abort response */

    /* Initiate segmented download of 0x2003:0x00 */
    uint8_t dn_init[8] = {0x20U, 0x03U, 0x20U, 0x00U, 8, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, dn_init, 8));
    syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len);

    /* Send Download Segment Request with incorrect toggle bit t=1 (expected t=0) */
    uint8_t seg_dn_bad_t[8] = {0x10U, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, seg_dn_bad_t, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]); /* Abort response */
}

static void test_canopen_sdo_segmented_download_invalid_data(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};
    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));

    uint32_t tx_id;
    uint8_t tx_buf[8], tx_len;
    (void)syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len); /* Drain bootup frame */

    /* Initiate segmented download of 0x2003:0x00 */
    uint8_t dn_init[8] = {0x20U, 0x03U, 0x20U, 0x00U, 8, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, dn_init, 8);
    syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len); /* Drain initiation ACK frame */

    /* Send download segment request with wrong toggle bit t=1 (expected t=0) */
    uint8_t invalid_cmd[8] = {0x10U, 0, 0, 0, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, invalid_cmd, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]); /* Abort response */
}

static void test_canopen_sdo_expedited_transfer_aborts(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5, .heartbeat_ms = 0};
    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));

    uint32_t tx_id;
    uint8_t tx_buf[8], tx_len;
    (void)syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len); /* Drain bootup frame */

    /* 1. OD read on Write-Only object -> SYN_ERROR */
    uint8_t buf[4];
    size_t out_len = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_canopen_od_read(&node, 0x2002U, 0x00U, buf, sizeof(buf), &out_len));

    /* 2. OD write on Read-Only object -> SYN_ERROR */
    uint32_t val = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_write(&node, 0x1000U, 0x00U, &val, sizeof(val)));

    /* 3. Expedited download size mismatch (write 1 byte to 2-byte entry 0x2001:0x01) -> SDO Abort
     */
    uint8_t sdo_exp_bad_len[8] = {0x2FU, 0x01U, 0x20U, 0x01U, 0x55U, 0x00U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_exp_bad_len, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]); /* Abort response */

    /* 4. SDO Upload Request on Write-Only object (0x2002:0x00) -> SDO Abort WRITE_ONLY */
    uint8_t sdo_up_wo[8] = {0x40U, 0x02U, 0x20U, 0x00U, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_up_wo, 8));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 5. Segmented download payload overflow -> SDO Abort TYPE_MISMATCH */
    uint8_t dn_init[8] = {0x20U, 0x01U, 0x20U, 0x01U, 2, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, dn_init, 8);
    syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len);
    /* Send segment with 7 bytes when only 2 bytes exist in OD entry size */
    uint8_t seg_overflow[8] = {0x00U, 1, 2, 3, 4, 5, 6, 7};
    syn_canopen_process_rx(&node, 0x605U, seg_overflow, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 6. NULL check in syn_canopen_send_emcy */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_send_emcy(NULL, 0x1000, 0x01));

    /* 7. Unknown COB-ID returns SYN_OK */
    uint8_t dummy[8] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x7FFU, dummy, 8));

    /* 8. Segmented upload segment request when entry disappears (lines 381-383) */
    node.sdo_session.state = SYN_CANOPEN_SDO_SEG_UPLOAD;
    node.sdo_session.index = 0x9999U; /* Non-existent index */
    node.sdo_session.subindex = 0x00U;
    node.sdo_session.toggle = 0;
    uint8_t seg_up_req[8] = {0x60U, 0, 0, 0, 0, 0, 0, 0};
    syn_canopen_process_rx(&node, 0x605U, seg_up_req, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 9. SDO Expedited upload size mismatch (line 325) */
    SYN_CANOpenODEntry od_large = {0x3000, 0x01, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RO,
                                   NULL,   8};
    SYN_CANOpenNodeConfig cfg_large;
    memset(&cfg_large, 0, sizeof(cfg_large));
    cfg_large.node_id = 5;
    SYN_CANOpenNode node_large;
    syn_canopen_init(&node_large, &cfg_large, &od_large, 1);
    (void)syn_canopen_get_tx(&node_large, &tx_id, tx_buf, &tx_len);
    syn_canopen_process_rx(&node_large, 0x000U, NULL, 0); /* Enter OPERATIONAL */

    uint8_t sdo_up_large[8] = {0x40U, 0x00U, 0x30U, 0x01U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node_large, 0x605U, sdo_up_large, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_large, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x41U, tx_buf[0]);

    /* 10. syn_canopen_tpdo_trigger with invalid OD map failure (line 501) */
    SYN_CANOpenNodeConfig cfg_bad_tpdo;
    memset(&cfg_bad_tpdo, 0, sizeof(cfg_bad_tpdo));
    cfg_bad_tpdo.node_id = 5;
    cfg_bad_tpdo.tpdo[0].enabled = 1;
    cfg_bad_tpdo.tpdo[0].cob_id = 0x185;
    cfg_bad_tpdo.tpdo[0].od_index = 0x9999;
    cfg_bad_tpdo.tpdo[0].od_subindex = 0x01;
    SYN_CANOpenNode node_bad_tpdo;
    memset(&node_bad_tpdo, 0, sizeof(node_bad_tpdo));
    syn_canopen_init(&node_bad_tpdo, &cfg_bad_tpdo, NULL, 0);
    uint8_t nmt_start[2] = {0x01, 0x05};
    syn_canopen_process_rx(&node_bad_tpdo, 0x000U, nmt_start, 2);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_tpdo_trigger(&node_bad_tpdo, 1));

    /* 11. SDO Upload request for non-existent OD entry -> SDO abort (line 325) */
    SYN_CANOpenNode node_sdo;
    SYN_CANOpenNodeConfig cfg_sdo = {.node_id = 5};
    syn_canopen_init(&node_sdo, &cfg_sdo, test_od, sizeof(test_od) / sizeof(test_od[0]));
    (void)syn_canopen_get_tx(&node_sdo, &tx_id, tx_buf, &tx_len);
    uint8_t sdo_up_invalid[8] = {0x40U, 0x99U, 0x99U, 0x00U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node_sdo, 0x605U, sdo_up_invalid, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_sdo, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 12. Valid TPDO trigger in Operational state (line 497) */
    SYN_CANOpenNodeConfig cfg_valid_tpdo;
    memset(&cfg_valid_tpdo, 0, sizeof(cfg_valid_tpdo));
    cfg_valid_tpdo.node_id = 5;
    cfg_valid_tpdo.tpdo[0].enabled = 1;
    cfg_valid_tpdo.tpdo[0].cob_id = 0x185;
    cfg_valid_tpdo.tpdo[0].od_index = 0x2001;
    cfg_valid_tpdo.tpdo[0].od_subindex = 0x01;
    SYN_CANOpenNode node_valid_tpdo;
    syn_canopen_init(&node_valid_tpdo, &cfg_valid_tpdo, test_od,
                     sizeof(test_od) / sizeof(test_od[0]));
    uint8_t nmt_start_cmd[2] = {0x01, 0x05};
    syn_canopen_process_rx(&node_valid_tpdo, 0x000U, nmt_start_cmd, 2);
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_tpdo_trigger(&node_valid_tpdo, 1));

    /* 13. SDO Download to non-existent index with existing index but wrong subindex */
    SYN_CANOpenNode node_sub_err;
    SYN_CANOpenNodeConfig cfg_sub_err = {.node_id = 5};
    syn_canopen_init(&node_sub_err, &cfg_sub_err, test_od, sizeof(test_od) / sizeof(test_od[0]));
    (void)syn_canopen_get_tx(&node_sub_err, &tx_id, tx_buf, &tx_len);
    uint8_t sdo_down_sub_err[8] = {0x2BU, 0x01U, 0x20U, 0x99U, 0x00U, 0x00U, 0x00U, 0x00U};
    syn_canopen_process_rx(&node_sub_err, 0x605U, sdo_down_sub_err, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_sub_err, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 14. SDO Upload from non-existent subindex with existing index */
    uint8_t sdo_up_sub_err[8] = {0x40U, 0x01U, 0x20U, 0x99U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node_sub_err, 0x605U, sdo_up_sub_err, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_sub_err, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);

    /* 15. Emergency send check */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_send_emcy(NULL, 0x1000, 0x01));
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_send_emcy(&node_sub_err, 0x1000, 0x01));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_sub_err, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x085U, tx_id);
    TEST_ASSERT_EQUAL(8, tx_len);
}

static void test_canopen_pdo_mapping_overflow_protection(void)
{
    /* 1. TPDO transmit with unmapped/invalid OD entry -> returns SYN_ERROR (line 502) */
    static const SYN_CANOpenODEntry od_invalid[] = {
        {0x2001U, 0x01U, SYN_CANOPEN_TYPE_U8, SYN_CANOPEN_ACCESS_RO, NULL, 0}
        /* NULL data pointer */
    };
    SYN_CANOpenNode node_tpdo_err;
    SYN_CANOpenNodeConfig cfg_tpdo_err = {.node_id = 5, .tpdo = {{0x185U, 0x2001U, 0x01U, 1U}}};
    syn_canopen_init(&node_tpdo_err, &cfg_tpdo_err, od_invalid, 1);
    uint8_t nmt_start[2] = {0x01, 0x05};
    syn_canopen_process_rx(&node_tpdo_err, 0x000U, nmt_start, 2);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_tpdo_trigger(&node_tpdo_err, 1));

    /* 2. SDO upload on entry with size <= 4 but NULL data pointer -> triggers abort line 326 */
    static const SYN_CANOpenODEntry od_null_read[] = {
        {0x2005U, 0x01U, SYN_CANOPEN_TYPE_U16, SYN_CANOPEN_ACCESS_RO, NULL, 2}};
    SYN_CANOpenNode node_exp_err;
    SYN_CANOpenNodeConfig cfg_exp_err = {.node_id = 5};
    syn_canopen_init(&node_exp_err, &cfg_exp_err, od_null_read, 1);
    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = {0};
    uint8_t tx_len = 0;
    (void)syn_canopen_get_tx(&node_exp_err, &tx_id, tx_buf, &tx_len); /* Drain bootup message */

    /* Expedited upload request 0x40 on 0x2005 sub 1 */
    uint8_t sdo_exp_req[8] = {0x40U, 0x05U, 0x20U, 0x01U, 0, 0, 0, 0};
    syn_canopen_process_rx(&node_exp_err, 0x605U, sdo_exp_req, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node_exp_err, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]); /* Abort response */
}

static void test_r5_a17_canopen_tx_fifo_and_sdo_u32_shift(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {.node_id = 5U,
                                 .heartbeat_ms = 100U,
                                 .tpdo = {{0x185U, 0x2001U, 0x01U, 1U},
                                          {0x285U, 0x2001U, 0x02U, 1U},
                                          {0x385U, 0x9999U, 0x00U, 1U}}};

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0])));
    uint32_t tx_id = 0U;
    uint8_t tx_buf[8] = {0};
    uint8_t tx_len = 0U;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_HEX32(0x705U, tx_id);

    /* NMT targeting a different node (node 9) must be ignored */
    uint8_t nmt_other[2] = {SYN_CANOPEN_NMT_CMD_START, 0x09U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_other, 2U));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    /* Broadcast NMT Start (target 0x00) enters Operational */
    uint8_t nmt_bcast[2] = {SYN_CANOPEN_NMT_CMD_START, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_bcast, 2U));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_OPERATIONAL, node.nmt_state);

    /* Unknown SDO CCS (0xE0) when idle is ignored cleanly */
    uint8_t sdo_unk[8] = {0xE0U, 0x00U, 0x10U, 0x00U, 0U, 0U, 0U, 0U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_unk, 8U));
    TEST_ASSERT_FALSE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));

    /* Segmented SDO download init with MSB data[7] = 0x80 (no signed shift UB) */
    uint8_t sdo_seg_msb[8] = {0x20U, 0x03U, 0x20U, 0x00U, 0x78U, 0x56U, 0x34U, 0x80U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_seg_msb, 8U));
    TEST_ASSERT_EQUAL_UINT32(0x80345678UL, (uint32_t)node.sdo_session.total_bytes);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x60U, tx_buf[0]);

    /* Single update tick queues Heartbeat (0x705) + TPDO1 (0x185) + TPDO2 (0x285) */
    od_target_speed = 0x4321U;
    od_node_status = 0x7AU;
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_update(&node, 100U));
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_HEX32(0x705U, tx_id);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_HEX32(0x185U, tx_id);
    TEST_ASSERT_EQUAL_HEX8(0x21U, tx_buf[0]);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_HEX32(0x285U, tx_id);
    TEST_ASSERT_EQUAL_HEX8(0x7AU, tx_buf[0]);
    TEST_ASSERT_FALSE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));

    /* Repeated update ticks reserve urgent headroom (2 slots) so EMCY/SDO are never starved */
    for (uint8_t k = 0U; k < 5U; k++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_update(&node, 10U));
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_CANOPEN_TX_QUEUE_DEPTH - 2U, node.tx_count);
    while (syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len)) {
    }

    /* Fill TX FIFO to capacity (8) -> 9th EMCY, SDO, and TPDO trigger return SYN_BUSY */
    for (uint16_t i = 0U; i < SYN_CANOPEN_TX_QUEUE_DEPTH; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_send_emcy(&node, (uint16_t)(0x1000U + i), 0x01U));
    }
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_canopen_send_emcy(&node, 0x1009U, 0x01U));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_canopen_tpdo_trigger(&node, 1U));
    uint8_t sdo_read_req[8] = {0x40U, 0x00U, 0x10U, 0x00U, 0U, 0U, 0U, 0U};
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_canopen_process_rx(&node, 0x605U, sdo_read_req, 8U));
    uint8_t nmt_rst[2] = {SYN_CANOPEN_NMT_CMD_RESET_COMM, 0x05U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_rst, 2U));
    TEST_ASSERT_EQUAL_UINT8(SYN_CANOPEN_TX_QUEUE_DEPTH, node.tx_count);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, NULL, NULL, NULL));
    for (uint8_t i = 1U; i < SYN_CANOPEN_TX_QUEUE_DEPTH; i++) {
        TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
        TEST_ASSERT_EQUAL_HEX32(0x085U, tx_id);
    }
    TEST_ASSERT_FALSE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
}

void run_canopen_tests(void)
{
    RUN_TEST(test_canopen_init_and_bootup);
    RUN_TEST(test_canopen_nmt_state_machine);
    RUN_TEST(test_canopen_sdo_expedited_read_write);
    RUN_TEST(test_canopen_sdo_abort_codes);
    RUN_TEST(test_canopen_rpdo_tpdo_emcy);
    RUN_TEST(test_canopen_invalid_params);
    RUN_TEST(test_cia303_indicators);
    RUN_TEST(test_canopen_tpdo_trigger);
    RUN_TEST(test_canopen_sdo_segmented_transfer);
    RUN_TEST(test_canopen_sdo_segmented_toggle_bit_mismatch);
    RUN_TEST(test_canopen_sdo_segmented_download_invalid_data);
    RUN_TEST(test_canopen_sdo_expedited_transfer_aborts);
    RUN_TEST(test_canopen_pdo_mapping_overflow_protection);
    RUN_TEST(test_r5_a17_canopen_tx_fifo_and_sdo_u32_shift);
}
