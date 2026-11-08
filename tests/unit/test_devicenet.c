/**
 * @file test_devicenet.c
 * @brief Unit tests for ODVA DeviceNet (CIP over CAN) protocol stack implementation.
 */

#include "syntropic/proto/syn_devicenet.h"
#include "syntropic/util/syn_pack.h"
#include "unity/unity.h"

#include <string.h>

static SYN_DeviceNet_Node g_dnet;
static uint8_t g_in_buf[4] = {0x11, 0x22, 0x33, 0x44};
static uint8_t g_out_buf[4] = {0x00};

static void test_devicenet_init_and_config(void)
{
    TEST_ASSERT_FALSE(syn_devicenet_init(NULL, 6, SYN_DEVICENET_BAUD_500K));
    TEST_ASSERT_FALSE(syn_devicenet_init(&g_dnet, 64, SYN_DEVICENET_BAUD_500K)); /* MAC ID > 63 */

    TEST_ASSERT_TRUE(syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K));
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_DUP_MAC_CHECK, g_dnet.state);
    TEST_ASSERT_EQUAL_UINT8(6, g_dnet.devicenet_obj.mac_id);

    /* Assembly configuration */
    TEST_ASSERT_FALSE(syn_devicenet_set_assembly(NULL, g_in_buf, 4, g_out_buf, 4));
    TEST_ASSERT_FALSE(
        syn_devicenet_set_assembly(&g_dnet, g_in_buf, 10, g_out_buf, 4)); /* payload > 8 */
    TEST_ASSERT_TRUE(syn_devicenet_set_assembly(&g_dnet, g_in_buf, 4, g_out_buf, 4));

    /* QuickConnect configuration */
    TEST_ASSERT_FALSE(syn_devicenet_set_quickconnect(NULL, true));
    TEST_ASSERT_TRUE(syn_devicenet_set_quickconnect(&g_dnet, true));
    TEST_ASSERT_TRUE(g_dnet.identity.quick_connect_enabled);
    TEST_ASSERT_EQUAL_UINT32(100, g_dnet.dup_mac_timer_ms);

    /* Serial number configuration */
    syn_devicenet_set_serial_number(NULL, 0x87654321U);
    syn_devicenet_set_serial_number(&g_dnet, 0x87654321U);
    TEST_ASSERT_EQUAL_UINT32(0x87654321U, g_dnet.identity.serial_number);
}

static void test_devicenet_dup_mac_and_state_transitions(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);

    /* Advance timer past 1000ms -> transitions to ONLINE */
    syn_devicenet_poll(&g_dnet, 500);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_DUP_MAC_CHECK, g_dnet.state);
    syn_devicenet_poll(&g_dnet, 600);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_ONLINE, g_dnet.state);

    /* Poll when NULL or already ONLINE is a no-op */
    syn_devicenet_poll(NULL, 100);
    syn_devicenet_poll(&g_dnet, 100);
}

static void test_devicenet_dup_mac_collision(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);

    uint32_t rx_can_id = 0x7C0 | (6 << 3) | 5; /* Dup MAC Check Request for MAC 6 */
    uint8_t rx_data[8] = {0};
    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* Dup MAC Request received while checking -> collision detected -> OFFLINE */
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, rx_can_id, rx_data, 8, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_OFFLINE, g_dnet.state);
}

static void test_devicenet_explicit_and_polled_messaging(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);
    syn_devicenet_set_assembly(&g_dnet, g_in_buf, 4, g_out_buf, 4);
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;

    uint32_t exp_req_id = 0x400 | (6 << 3) | 4;   /* Group 2 Explicit Req */
    uint32_t exp_resp_id = 0x400 | (6 << 3) | 3;  /* Group 2 Explicit Resp */
    uint32_t poll_cmd_id = 0x400 | (6 << 3) | 5;  /* Group 2 Polled Cmd */
    uint32_t poll_resp_id = 0x400 | (6 << 3) | 6; /* Group 2 Polled Resp */

    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* 1. Explicit Get_Attribute_Single (Class 0x01 Identity, Attr 1 Vendor ID) */
    uint8_t req_get_vendor[4] = {6, 0x0E, 0x01, 1};
    TEST_ASSERT_TRUE(syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_vendor, 4, &tx_can_id,
                                             tx_data, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(exp_resp_id, tx_can_id);
    TEST_ASSERT_EQUAL_HEX8(0x8E, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x5A, tx_data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_data[3]);

    /* Get_Attribute_Single (Class 0x01 Identity, Attr 4 Revision) */
    uint8_t req_get_rev[4] = {6, 0x0E, 0x01, 4};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_rev, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(1, tx_data[2]);
    TEST_ASSERT_EQUAL_UINT8(0, tx_data[3]);

    /* Get_Attribute_Single (Class 0x01 Identity, Attr 6 Serial Number) */
    uint8_t req_get_sn[4] = {6, 0x0E, 0x01, 6};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_sn, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x78, tx_data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x56, tx_data[3]);
    TEST_ASSERT_EQUAL_HEX8(0x34, tx_data[4]);
    TEST_ASSERT_EQUAL_HEX8(0x12, tx_data[5]);

    /* Get_Attribute_Single (Class 0x03 DeviceNet, Attr 1 MAC ID) */
    uint8_t req_get_mac[4] = {6, 0x0E, 0x03, 1};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_mac, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(6, tx_data[2]);

    /* Get_Attribute_Single (Class 0x04 Assembly) */
    uint8_t req_get_asm[4] = {6, 0x0E, 0x04, 3};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_asm, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x11, tx_data[2]);

    /* Test Assembly read with 8-byte input buffer (clamped to 6 bytes payload) */
    uint8_t big_in[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    syn_devicenet_set_assembly(&g_dnet, big_in, 8, g_out_buf, 4);
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_asm, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(8, tx_len);
    TEST_ASSERT_EQUAL_UINT8(1, tx_data[2]);
    TEST_ASSERT_EQUAL_UINT8(6, tx_data[7]);
    syn_devicenet_set_assembly(&g_dnet, g_in_buf, 4, g_out_buf, 4);

    /* Set_Attribute_Single (Class 0x01 Identity, Attr 9 QuickConnect) */
    uint8_t req_set_qc[5] = {6, 0x10, 0x01, 9, 1};
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_set_qc, 5, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_TRUE(g_dnet.identity.quick_connect_enabled);

    /* Set_Attribute_Single (Class 0x03 DeviceNet, Attr 1 MAC ID) */
    uint8_t req_set_mac[5] = {6, 0x10, 0x03, 1, 12};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_set_mac, 5, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(12, g_dnet.devicenet_obj.mac_id);
    g_dnet.devicenet_obj.mac_id = 6; /* Restore MAC ID */

    /* 2. Polled I/O Command */
    uint8_t poll_cmd_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    TEST_ASSERT_TRUE(syn_devicenet_on_can_rx(&g_dnet, poll_cmd_id, poll_cmd_data, 4, &tx_can_id,
                                             tx_data, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(poll_resp_id, tx_can_id);
    TEST_ASSERT_EQUAL_UINT8(4, tx_len);
    TEST_ASSERT_EQUAL_HEX8(0xAA, g_out_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, g_out_buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x11, tx_data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x44, tx_data[3]);
}

static void test_devicenet_error_branches_and_nulls(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;

    uint32_t exp_req_id = 0x400 | (6 << 3) | 4;
    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* Unsupported Class ID -> 0x05 Class not supported */
    uint8_t req_bad_class[4] = {6, 0x0E, 0x99, 1};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_bad_class, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x94, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x05, tx_data[2]);

    /* Unsupported Class 0x01 Attribute ID */
    uint8_t req_bad_attr1[4] = {6, 0x0E, 0x01, 99};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_bad_attr1, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x94, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x14, tx_data[2]);

    /* Unsupported Class 0x03 Attribute ID */
    uint8_t req_bad_attr3[4] = {6, 0x0E, 0x03, 99};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_bad_attr3, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x94, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x14, tx_data[2]);

    /* Get Device Type (Attr 2), Product Code (Attr 3), QuickConnect (Attr 9) */
    uint8_t req_get_dt[4] = {6, 0x0E, 0x01, 2};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_dt, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x0C, tx_data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_data[3]);

    uint8_t req_get_pc[4] = {6, 0x0E, 0x01, 3};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_pc, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, tx_data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_data[3]);

    uint8_t req_get_qc[4] = {6, 0x0E, 0x01, 9};
    g_dnet.identity.quick_connect_enabled = true;
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_qc, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(1, tx_data[2]);

    /* Get Class 0x03 Baud Rate (Attr 2) */
    uint8_t req_get_baud[4] = {6, 0x0E, 0x03, 2};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_baud, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_DEVICENET_BAUD_500K, tx_data[2]);

    /* Set_Attribute_Single short request (< 5 bytes) */
    uint8_t req_set_short[4] = {6, 0x10, 0x01, 9};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_set_short, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x94, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x13, tx_data[2]);

    /* Set_Attribute_Single with invalid MAC ID > 63 -> ignored */
    uint8_t req_set_mac_invalid[5] = {6, 0x10, 0x03, 1, 99};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_set_mac_invalid, 5, &tx_can_id, tx_data,
                            &tx_len);
    TEST_ASSERT_EQUAL_UINT8(6, g_dnet.devicenet_obj.mac_id);

    /* Defending MAC ID in ONLINE state */
    uint32_t dup_req_id = 0x7C0 | (6 << 3) | 5;
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, dup_req_id, tx_data, 8, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(0x7C0 | (6 << 3) | 6, tx_can_id);

    /* Polled I/O with NULL assembly buffers */
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;
    uint32_t poll_cmd_id = 0x400 | (6 << 3) | 5;
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, poll_cmd_id, tx_data, 4, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL_UINT8(0, tx_len);

    /* Unhandled CAN ID returns false */
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, 0x123, tx_data, 4, &tx_can_id, tx_data, &tx_len));

    /* Message received while node is OFFLINE returns false */
    g_dnet.state = SYN_DEVICENET_STATE_OFFLINE;
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_qc, 4, &tx_can_id, tx_data, &tx_len));
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;

    /* Unsupported Service Code -> 0x08 Service not supported */
    uint8_t req_bad_svc[4] = {6, 0x99, 0x01, 1};
    syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_bad_svc, 4, &tx_can_id, tx_data, &tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x94, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x08, tx_data[2]);

    /* Runt explicit request len < 4 */
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_bad_svc, 2, &tx_can_id, tx_data, &tx_len));

    /* NULL guards */
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(NULL, exp_req_id, tx_data, 4, &tx_can_id, tx_data, &tx_len));
}

static void test_devicenet_polled_io_output_overflow(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);
    syn_devicenet_set_assembly(&g_dnet, g_in_buf, 4, g_out_buf, 4);
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;

    uint32_t poll_cmd_id = 0x400 | (6 << 3) | 5;
    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* Send 6 bytes of polled output data when output_len is 4 */
    uint8_t long_poll_cmd[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    TEST_ASSERT_TRUE(syn_devicenet_on_can_rx(&g_dnet, poll_cmd_id, long_poll_cmd, 6, &tx_can_id,
                                             tx_data, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0xAA, g_out_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, g_out_buf[3]); /* Only 4 bytes copied */
}

static void test_devicenet_explicit_msg_fragmentation(void)
{
    syn_devicenet_init(&g_dnet, 6, SYN_DEVICENET_BAUD_500K);
    syn_devicenet_set_assembly(&g_dnet, g_in_buf, 4, g_out_buf, 4);

    /* 1. Timer decrement branch in DUP_MAC_CHECK state */
    syn_devicenet_poll(&g_dnet, 50);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_DUP_MAC_CHECK, g_dnet.state);
    TEST_ASSERT_EQUAL_UINT32(950, g_dnet.dup_mac_timer_ms);

    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;
    uint32_t exp_req_id = 0x400 | (6 << 3) | 4;
    uint32_t tx_can_id = 0;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0;

    /* 2. Get_Attribute_Single for Class 0x01 Attr 1 (Vendor ID) */
    uint8_t req_get_vendor[4] = {6, 0x0E, 0x01, 1};
    TEST_ASSERT_TRUE(syn_devicenet_on_can_rx(&g_dnet, exp_req_id, req_get_vendor, 4, &tx_can_id,
                                             tx_data, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x5A, tx_data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, tx_data[3]);
}

static void test_r5_a18_devicenet_dup_mac_resp_collision(void)
{
    syn_devicenet_init(&g_dnet, 6U, SYN_DEVICENET_BAUD_500K);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_DUP_MAC_CHECK, g_dnet.state);
    TEST_ASSERT_FALSE(g_dnet.dup_mac_failed);

    /* Receiving Group 4 Duplicate MAC ID Check Response (msg_id == 6) forces OFFLINE */
    uint32_t dup_resp_id = 0x7C0U | (6U << 3U) | 6U;
    uint8_t rx_data[4] = {0x78U, 0x56U, 0x34U, 0x12U};
    uint32_t tx_can_id = 0U;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0U;

    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, dup_resp_id, rx_data, 4U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_TRUE(g_dnet.dup_mac_failed);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_OFFLINE, g_dnet.state);

    /* Subsequent poll must NOT transition node to ONLINE */
    syn_devicenet_poll(&g_dnet, 2000U);
    TEST_ASSERT_EQUAL(SYN_DEVICENET_STATE_OFFLINE, g_dnet.state);
}

static void test_pro22_devicenet_set_attr_errors_frag_and_runt_explicit(void)
{
    syn_devicenet_init(&g_dnet, 6U, SYN_DEVICENET_BAUD_500K);
    g_dnet.state = SYN_DEVICENET_STATE_ONLINE;

    uint32_t exp_req_id = 0x400U | (6U << 3U) | 4U;
    uint32_t tx_can_id = 0U;
    uint8_t tx_data[8] = {0};
    uint8_t tx_len = 0U;

    /* 1. Runt (< 4B) and fragmented (data[0] & 0x80) Explicit frames do NOT set explicit_connected
     */
    uint8_t runt_exp[3] = {6U, 0x0EU, 0x01U};
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, runt_exp, 3U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_FALSE(g_dnet.explicit_connected);

    uint8_t frag_exp[4] = {0x86U, 0x0EU, 0x01U, 1U};
    TEST_ASSERT_FALSE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, frag_exp, 4U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_FALSE(g_dnet.explicit_connected);

    /* 2. Set_Attribute_Single returns 0x94 error on invalid MAC ID >= 64 & unsupported class/attr
     */
    uint8_t bad_mac[5] = {6U, 0x10U, 0x03U, 1U, 64U};
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, bad_mac, 5U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_TRUE(g_dnet.explicit_connected);
    TEST_ASSERT_EQUAL_HEX8(0x94U, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x09U, tx_data[2]);
    TEST_ASSERT_EQUAL_UINT8(3U, tx_len);

    uint8_t bad_attr1[5] = {6U, 0x10U, 0x01U, 1U, 0U};
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, bad_attr1, 5U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x94U, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x14U, tx_data[2]);

    uint8_t bad_attr3[5] = {6U, 0x10U, 0x03U, 99U, 0U};
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, bad_attr3, 5U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x94U, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x14U, tx_data[2]);

    uint8_t bad_cls[5] = {6U, 0x10U, 0x99U, 1U, 0U};
    TEST_ASSERT_TRUE(
        syn_devicenet_on_can_rx(&g_dnet, exp_req_id, bad_cls, 5U, &tx_can_id, tx_data, &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x94U, tx_data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x05U, tx_data[2]);
}

void run_devicenet_tests(void)
{
    RUN_TEST(test_devicenet_init_and_config);
    RUN_TEST(test_devicenet_dup_mac_and_state_transitions);
    RUN_TEST(test_devicenet_dup_mac_collision);
    RUN_TEST(test_devicenet_explicit_and_polled_messaging);
    RUN_TEST(test_devicenet_error_branches_and_nulls);
    RUN_TEST(test_devicenet_polled_io_output_overflow);
    RUN_TEST(test_devicenet_explicit_msg_fragmentation);
    RUN_TEST(test_r5_a18_devicenet_dup_mac_resp_collision);
    RUN_TEST(test_pro22_devicenet_set_attr_errors_frag_and_runt_explicit);
}
