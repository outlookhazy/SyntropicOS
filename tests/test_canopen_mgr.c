/**
 * @file test_canopen_mgr.c
 * @brief Unity unit tests for CANopen CiA 302 Manager, SDO Client & NMT Master.
 */

#include "unity/unity.h"
#include "syntropic/proto/syn_canopen_mgr.h"
#include <string.h>

static SYN_CANOpenManager g_can_mgr;

void test_canopen_mgr_init(void)
{
    syn_canopen_mgr_init(&g_can_mgr);
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_IDLE, g_can_mgr.sdo_client.state);
}

void test_canopen_mgr_build_nmt(void)
{
    SYN_CAN_Frame frame;
    syn_canopen_mgr_build_nmt(&frame, 0x05, SYN_CANOPEN_NMT_CMD_START);

    TEST_ASSERT_EQUAL(0x000, frame.id);
    TEST_ASSERT_EQUAL(2, frame.dlc);
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_CMD_START, frame.data[0]);
    TEST_ASSERT_EQUAL(0x05, frame.data[1]);
}

void test_canopen_mgr_sdo_read(void)
{
    syn_canopen_mgr_init(&g_can_mgr);

    SYN_CAN_Frame req, resp;
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 0x10, 0x1000, 0x00));
    TEST_ASSERT_EQUAL(0x610, req.id);
    TEST_ASSERT_EQUAL(0x40, req.data[0]);
    TEST_ASSERT_EQUAL(0x00, req.data[1]);
    TEST_ASSERT_EQUAL(0x10, req.data[2]);

    /* Simulate SDO Upload Response from slave 0x10 */
    memset(&resp, 0, sizeof(resp));
    resp.id = 0x580 + 0x10;
    resp.dlc = 8;
    resp.data[0] = 0x43; /* 4 bytes data */
    resp.data[1] = 0x00;
    resp.data[2] = 0x10;
    resp.data[3] = 0x00;
    resp.data[4] = 0xA1;
    resp.data[5] = 0xB2;
    resp.data[6] = 0xC3;
    resp.data[7] = 0xD4;

    syn_canopen_mgr_process_frame(&g_can_mgr, &resp);
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_SUCCESS, g_can_mgr.sdo_client.state);
    TEST_ASSERT_EQUAL(4, g_can_mgr.sdo_client.data_len);
    TEST_ASSERT_EQUAL_HEX8(0xA1, g_can_mgr.sdo_client.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD4, g_can_mgr.sdo_client.data[3]);
}

void test_canopen_mgr_sdo_write(void)
{
    syn_canopen_mgr_init(&g_can_mgr);

    SYN_CAN_Frame req, resp;
    uint32_t val = 0x12345678;
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_mgr_sdo_write_init(&g_can_mgr, &req, 0x05, 0x6040, 0x00, &val, 4));
    TEST_ASSERT_EQUAL(0x605, req.id);
    TEST_ASSERT_EQUAL(0x23, req.data[0]); /* CS=1, n=0, e=1, s=1 */

    /* Simulate SDO Download Response */
    memset(&resp, 0, sizeof(resp));
    resp.id = 0x580 + 0x05;
    resp.dlc = 8;
    resp.data[0] = 0x60;
    resp.data[1] = 0x40;
    resp.data[2] = 0x60;
    resp.data[3] = 0x00;

    syn_canopen_mgr_process_frame(&g_can_mgr, &resp);
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_SUCCESS, g_can_mgr.sdo_client.state);
}

void test_canopen_mgr_heartbeat_monitor(void)
{
    syn_canopen_mgr_init(&g_can_mgr);

    SYN_CAN_Frame hb;
    memset(&hb, 0, sizeof(hb));
    hb.id = 0x705; /* Node 5 */
    hb.dlc = 1;
    hb.data[0] = SYN_CANOPEN_NMT_STATE_OPERATIONAL;

    syn_canopen_mgr_process_frame(&g_can_mgr, &hb);
    TEST_ASSERT_TRUE(g_can_mgr.nodes[5].online);
    TEST_ASSERT_EQUAL(SYN_CANOPEN_NMT_STATE_OPERATIONAL, g_can_mgr.nodes[5].nmt_state);

    /* Step past 3000ms timeout */
    syn_canopen_mgr_step(&g_can_mgr, 3500);
    TEST_ASSERT_FALSE(g_can_mgr.nodes[5].online);
}

void test_canopen_mgr_extra_coverage(void)
{
    syn_canopen_mgr_init(&g_can_mgr);

    SYN_CAN_Frame req, resp;
    uint32_t val = 42;

    /* Invalid parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 0, 0x1000, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 128, 0x1000, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_mgr_sdo_write_init(&g_can_mgr, &req, 0, 0x1000, 0, &val, 4));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_mgr_sdo_write_init(&g_can_mgr, &req, 1, 0x1000, 0, &val, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_canopen_mgr_sdo_write_init(&g_can_mgr, &req, 1, 0x1000, 0, &val, 5));

    /* Initiate SDO Read and verify BUSY on subsequent calls */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 10, 0x1000, 0));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 10, 0x1000, 0));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_canopen_mgr_sdo_write_init(&g_can_mgr, &req, 10, 0x1000, 0, &val, 4));

    /* SDO Abort Domain Transfer Response */
    memset(&resp, 0, sizeof(resp));
    resp.id = 0x580 + 10;
    resp.dlc = 8;
    resp.data[0] = 0x80; /* Abort */
    resp.data[4] = 0x02;
    resp.data[5] = 0x00;
    resp.data[6] = 0x01;
    resp.data[7] = 0x06; /* 0x06010002 Attempt to write read-only */

    syn_canopen_mgr_process_frame(&g_can_mgr, &resp);
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_ERROR, g_can_mgr.sdo_client.state);
    TEST_ASSERT_EQUAL(0x06010002UL, g_can_mgr.sdo_client.abort_code);

    /* SDO Timeout Test */
    TEST_ASSERT_EQUAL(SYN_OK, syn_canopen_mgr_sdo_read_init(&g_can_mgr, &req, 10, 0x1000, 0));
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_WAIT_READ, g_can_mgr.sdo_client.state);
    syn_canopen_mgr_step(&g_can_mgr, 1200); /* Exceed 1000ms timeout */
    TEST_ASSERT_EQUAL(SYN_SDO_CLIENT_STATE_ERROR, g_can_mgr.sdo_client.state);
    TEST_ASSERT_EQUAL(0x05040000UL, g_can_mgr.sdo_client.abort_code);
}

void run_canopen_mgr_tests(void)
{
    RUN_TEST(test_canopen_mgr_init);
    RUN_TEST(test_canopen_mgr_build_nmt);
    RUN_TEST(test_canopen_mgr_sdo_read);
    RUN_TEST(test_canopen_mgr_sdo_write);
    RUN_TEST(test_canopen_mgr_heartbeat_monitor);
    RUN_TEST(test_canopen_mgr_extra_coverage);
}
