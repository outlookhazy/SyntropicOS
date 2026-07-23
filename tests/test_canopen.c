/**
 * @file test_canopen.c
 * @brief Unity unit tests for CANopen DS301 Slave Protocol Engine.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_canopen.h"

static uint32_t od_device_type = 0x00020192UL;
static uint16_t od_target_speed = 0;
static uint8_t  od_node_status = 1;
static uint8_t  od_wo_val = 0;
static uint64_t od_large_val = 0x1122334455667788ULL;

static const SYN_CANOpenODEntry test_od[] = {
    { 0x1000U, 0x00U, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RO, &od_device_type, sizeof(od_device_type) },
    { 0x2001U, 0x01U, SYN_CANOPEN_TYPE_U16, SYN_CANOPEN_ACCESS_RW, &od_target_speed, sizeof(od_target_speed) },
    { 0x2001U, 0x02U, SYN_CANOPEN_TYPE_U8,  SYN_CANOPEN_ACCESS_RO, &od_node_status, sizeof(od_node_status) },
    { 0x2002U, 0x00U, SYN_CANOPEN_TYPE_U8,  SYN_CANOPEN_ACCESS_WO, &od_wo_val, sizeof(od_wo_val) },
    { 0x2003U, 0x00U, SYN_CANOPEN_TYPE_U32, SYN_CANOPEN_ACCESS_RO, &od_large_val, sizeof(od_large_val) }
};

static void test_canopen_init_and_bootup(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {
        .node_id = 5,
        .heartbeat_ms = 1000,
        .rpdo = {
            { 0x205U, 0x2001U, 0x01U, 1U }
        },
        .tpdo = {
            { 0x185U, 0x2001U, 0x01U, 1U }
        }
    };

    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0])));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    /* Check Bootup Tx message */
    uint32_t tx_id = 0;
    uint8_t tx_buf[8] = { 0 };
    uint8_t tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id);
    TEST_ASSERT_EQUAL(1, tx_len);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[0]);
}

static void test_canopen_nmt_state_machine(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = { .node_id = 5, .heartbeat_ms = 0 };

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id; uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* NMT Start -> Operational */
    uint8_t nmt_start[2] = { 0x01U, 0x05U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_start, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_OPERATIONAL, node.nmt_state);

    /* NMT Stop -> Stopped */
    uint8_t nmt_stop[2] = { 0x02U, 0x05U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_stop, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_STOPPED, node.nmt_state);

    /* NMT PreOp -> Pre-Operational */
    uint8_t nmt_preop[2] = { 0x80U, 0x05U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_preop, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    /* NMT Reset Node -> PreOp + Bootup frame */
    uint8_t nmt_reset[2] = { 0x81U, 0x05U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, nmt_reset, 2));
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_PREOP, node.nmt_state);

    uint32_t tx_id = 0; uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id);
}

static void test_canopen_sdo_expedited_read_write(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = { .node_id = 5, .heartbeat_ms = 0 };

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id; uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* SDO Download Request: Write 1500 (0x05DC) to 0x2001:0x01 */
    uint8_t sdo_write_req[8] = { 0x2BU, 0x01U, 0x20U, 0x01U, 0xDCU, 0x05U, 0x00U, 0x00U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x605U, sdo_write_req, 8));

    TEST_ASSERT_EQUAL_UINT16(1500, od_target_speed);

    uint32_t tx_id = 0; uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x585U, tx_id);
    TEST_ASSERT_EQUAL(8, tx_len);
    TEST_ASSERT_EQUAL(0x60U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[1]);
    TEST_ASSERT_EQUAL(0x20U, tx_buf[2]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[3]);

    /* SDO Upload Request: Read 0x2001:0x01 */
    uint8_t sdo_read_req[8] = { 0x40U, 0x01U, 0x20U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U };
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
    SYN_CANOpenNodeConfig cfg = { .node_id = 5, .heartbeat_ms = 0 };

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id; uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* 1. Non-existent index 0x9999 */
    uint8_t sdo_invalid_index[8] = { 0x40U, 0x99U, 0x99U, 0x00U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_invalid_index, 8);

    uint32_t tx_id = 0; uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x02U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06020000 Object does not exist */

    /* 2. Non-existent subindex 0x2001:0x99 */
    uint8_t sdo_invalid_subidx[8] = { 0x40U, 0x01U, 0x20U, 0x99U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_invalid_subidx, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x11U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x09U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06090011 Subindex does not exist */

    /* 3. Write to Read-Only entry 0x1000:0x00 */
    uint8_t sdo_write_ro[8] = { 0x23U, 0x00U, 0x10U, 0x00U, 0x11U, 0x22U, 0x33U, 0x44U };
    syn_canopen_process_rx(&node, 0x605U, sdo_write_ro, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x02U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x01U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06010002 Read-only */

    /* 4. Write with invalid length to 0x2001:0x01 */
    uint8_t sdo_write_mismatch[8] = { 0x23U, 0x01U, 0x20U, 0x01U, 0x11U, 0x22U, 0x33U, 0x44U };
    syn_canopen_process_rx(&node, 0x605U, sdo_write_mismatch, 8);

    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x80U, tx_buf[0]);
    TEST_ASSERT_EQUAL(0x10U, tx_buf[4]);
    TEST_ASSERT_EQUAL(0x00U, tx_buf[5]);
    TEST_ASSERT_EQUAL(0x07U, tx_buf[6]);
    TEST_ASSERT_EQUAL(0x06U, tx_buf[7]); /* 0x06070010 Type Mismatch */

    /* 5. SDO Write to non-existent index */
    uint8_t sdo_w_no_idx[8] = { 0x2BU, 0x99U, 0x99U, 0x00U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_w_no_idx, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06020000UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5]<<8) | ((uint32_t)tx_buf[6]<<16) | ((uint32_t)tx_buf[7]<<24));

    /* 6. SDO Write to non-existent subindex */
    uint8_t sdo_w_no_sub[8] = { 0x2BU, 0x01U, 0x20U, 0x99U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_w_no_sub, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06090011UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5]<<8) | ((uint32_t)tx_buf[6]<<16) | ((uint32_t)tx_buf[7]<<24));

    /* 7. SDO Read from Write-Only entry 0x2002:0x00 */
    uint8_t sdo_r_wo[8] = { 0x40U, 0x02U, 0x20U, 0x00U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_r_wo, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06010001UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5]<<8) | ((uint32_t)tx_buf[6]<<16) | ((uint32_t)tx_buf[7]<<24));

    /* 8. SDO Read from 8-byte entry 0x2003:0x00 (read_len > 4) */
    uint8_t sdo_r_large[8] = { 0x40U, 0x03U, 0x20U, 0x00U, 0, 0, 0, 0 };
    syn_canopen_process_rx(&node, 0x605U, sdo_r_large, 8);
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x06070010UL, (uint32_t)tx_buf[4] | ((uint32_t)tx_buf[5]<<8) | ((uint32_t)tx_buf[6]<<16) | ((uint32_t)tx_buf[7]<<24));

    /* 9. Unknown NMT cmd and unknown COB-ID */
    uint8_t unknown_nmt[2] = { 0xFFU, 0x05U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x000U, unknown_nmt, 2));

    uint8_t unknown_cob[2] = { 0x01U, 0x02U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x333U, unknown_cob, 2));
}

static void test_canopen_rpdo_tpdo_emcy(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = {
        .node_id = 5,
        .heartbeat_ms = 500,
        .rpdo = {
            { 0x205U, 0x2001U, 0x01U, 1U }
        },
        .tpdo = {
            { 0x185U, 0x2001U, 0x01U, 1U }
        }
    };

    syn_canopen_init(&node, &cfg, test_od, sizeof(test_od) / sizeof(test_od[0]));
    uint32_t dummy_id; uint8_t dummy_buf[8], dummy_len;
    syn_canopen_get_tx(&node, &dummy_id, dummy_buf, &dummy_len);

    /* Switch to Operational Mode */
    uint8_t nmt_start[2] = { 0x01U, 0x05U };
    syn_canopen_process_rx(&node, 0x000U, nmt_start, 2);

    /* Send RPDO1 frame: COB-ID = 0x205, Payload = { 0x20, 0x03 } (800) */
    uint8_t rpdo1[2] = { 0x20U, 0x03U };
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_process_rx(&node, 0x205U, rpdo1, 2));
    TEST_ASSERT_EQUAL_UINT16(800, od_target_speed);

    /* Periodic Update for TPDO & Heartbeat */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_update(&node, 500));

    uint32_t tx_id = 0; uint8_t tx_buf[8] = {0}, tx_len = 0;
    TEST_ASSERT_TRUE(syn_canopen_get_tx(&node, &tx_id, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(0x705U, tx_id); /* Heartbeat */
    TEST_ASSERT_EQUAL(0x05U, tx_buf[0]); /* Operational state */

    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_update(&node, 0));
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
}

static void test_canopen_invalid_params(void)
{
    SYN_CANOpenNode node;
    SYN_CANOpenNodeConfig cfg = { .node_id = 5, .heartbeat_ms = 1000 };
    uint8_t dummy_buf[8] = { 0 };
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

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_read(NULL, 0x1000, 0, dummy_buf, 4, &dummy_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_read(&node, 0x1000, 0, NULL, 4, &dummy_len));
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
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_read(&node, 0x1000, 0, dummy_buf, 1, &dummy_len));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_write(&node, 0x9999, 0, dummy_buf, 4));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_canopen_od_write(&node, 0x1000, 0, dummy_buf, 4));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_od_write(&node, 0x2001, 1, dummy_buf, 1));
}

void run_canopen_tests(void)
{
    RUN_TEST(test_canopen_init_and_bootup);
    RUN_TEST(test_canopen_nmt_state_machine);
    RUN_TEST(test_canopen_sdo_expedited_read_write);
    RUN_TEST(test_canopen_sdo_abort_codes);
    RUN_TEST(test_canopen_rpdo_tpdo_emcy);
    RUN_TEST(test_canopen_invalid_params);
}
