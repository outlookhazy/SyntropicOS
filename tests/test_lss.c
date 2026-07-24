/**
 * @file test_lss.c
 * @brief Unity unit tests for CANopen CiA 305 LSS Master & Slave.
 */

#include "unity/unity.h"
#include "syntropic/proto/syn_lss.h"
#include <string.h>

static SYN_LSSSlave g_lss_slave;
static SYN_LSSAddress g_lss_addr = {
    .vendor_id = 0x000000A5,
    .product_code = 0x12345678,
    .revision_no = 0x00010002,
    .serial_no = 0x99887766
};

void test_lss_init(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0xFF);
    TEST_ASSERT_EQUAL(SYN_LSS_MODE_OPERATION, g_lss_slave.mode);
    TEST_ASSERT_EQUAL(0xFF, g_lss_slave.node_id);
    TEST_ASSERT_EQUAL(SYN_LSS_BAUD_250K, g_lss_slave.baud_table);
    TEST_ASSERT_EQUAL(0x000000A5, g_lss_slave.addr.vendor_id);
}

void test_lss_switch_mode(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0xFF);

    SYN_CAN_Frame req, resp;
    syn_lss_master_build_switch_mode(&req, SYN_LSS_MODE_CONFIGURATION);

    TEST_ASSERT_FALSE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_EQUAL(SYN_LSS_MODE_CONFIGURATION, g_lss_slave.mode);

    syn_lss_master_build_switch_mode(&req, SYN_LSS_MODE_OPERATION);
    TEST_ASSERT_FALSE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_EQUAL(SYN_LSS_MODE_OPERATION, g_lss_slave.mode);
}

void test_lss_configure_node_id(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0xFF);
    g_lss_slave.mode = SYN_LSS_MODE_CONFIGURATION;

    SYN_CAN_Frame req, resp;
    syn_lss_master_build_config_node_id(&req, 0x20);

    TEST_ASSERT_TRUE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_EQUAL(0x20, g_lss_slave.node_id);
    TEST_ASSERT_EQUAL(SYN_LSS_COB_ID_SLAVE, resp.id);
    TEST_ASSERT_EQUAL(SYN_LSS_CS_CONFIGURE_NODE_ID, resp.data[0]);
    TEST_ASSERT_EQUAL(0, resp.data[1]);
}

void test_lss_configure_bit_timing(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0xFF);
    g_lss_slave.mode = SYN_LSS_MODE_CONFIGURATION;

    SYN_CAN_Frame req, resp;
    syn_lss_master_build_config_bit_timing(&req, SYN_LSS_BAUD_500K);

    TEST_ASSERT_TRUE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_EQUAL(SYN_LSS_BAUD_500K, g_lss_slave.baud_table);
    TEST_ASSERT_EQUAL(SYN_LSS_CS_CONFIGURE_BIT_TIMING, resp.data[0]);
    TEST_ASSERT_EQUAL(0, resp.data[1]);
}

void test_lss_store_config(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0xFF);
    g_lss_slave.mode = SYN_LSS_MODE_CONFIGURATION;

    SYN_CAN_Frame req, resp;
    syn_lss_master_build_store_config(&req);

    TEST_ASSERT_TRUE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_TRUE(g_lss_slave.store_req);
    TEST_ASSERT_EQUAL(SYN_LSS_CS_STORE_CONFIG, resp.data[0]);
    TEST_ASSERT_EQUAL(0, resp.data[1]);
}

void test_lss_inquire_address(void)
{
    syn_lss_slave_init(&g_lss_slave, &g_lss_addr, 0x10);
    g_lss_slave.mode = SYN_LSS_MODE_CONFIGURATION;

    SYN_CAN_Frame req, resp;
    memset(&req, 0, sizeof(req));
    req.id = SYN_LSS_COB_ID_MASTER;
    req.dlc = 8;
    req.data[0] = SYN_LSS_CS_INQUIRE_VENDOR;

    TEST_ASSERT_TRUE(syn_lss_slave_process(&g_lss_slave, &req, &resp));
    TEST_ASSERT_EQUAL(SYN_LSS_CS_INQUIRE_VENDOR, resp.data[0]);
    uint32_t vendor = (uint32_t)resp.data[1] | ((uint32_t)resp.data[2] << 8) |
                     ((uint32_t)resp.data[3] << 16) | ((uint32_t)resp.data[4] << 24);
    TEST_ASSERT_EQUAL(0x000000A5, vendor);
}

void run_lss_tests(void)
{
    RUN_TEST(test_lss_init);
    RUN_TEST(test_lss_switch_mode);
    RUN_TEST(test_lss_configure_node_id);
    RUN_TEST(test_lss_configure_bit_timing);
    RUN_TEST(test_lss_store_config);
    RUN_TEST(test_lss_inquire_address);
}
