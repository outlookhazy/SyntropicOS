/**
 * @file test_ccp.c
 * @brief Unit tests for ASAM CCP v2.1 slave protocol implementation.
 */

#include "syntropic/proto/syn_ccp.h"
#include "unity/unity.h"

#include <string.h>

static SYN_CCP_Slave g_ccp_slave;

static void test_ccp_init_and_connect_disconnect(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};

    /* Attempt command prior to connection -> fails with NOT_CONNECTED */
    cro[0] = SYN_CCP_CMD_GET_CCP_VERSION;
    cro[1] = 0x01;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_PID_CRM, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_NOT_CONNECTED, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[2]);

    /* CONNECT with station address mismatch -> ignored */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x02;
    cro[2] = 0x99;
    cro[3] = 0x99;
    TEST_ASSERT_FALSE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_FALSE(g_ccp_slave.connected);

    /* CONNECT with matching station address -> SUCCESS */
    cro[2] = 0x34;
    cro[3] = 0x12;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_PID_CRM, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, dto[2]);
    TEST_ASSERT_TRUE(g_ccp_slave.connected);

    /* GET_CCP_VERSION */
    cro[0] = SYN_CCP_CMD_GET_CCP_VERSION;
    cro[1] = 0x03;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, dto[3]); /* Main version 2 */
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[4]); /* Release version 1 */

    /* EXCHANGE_ID */
    cro[0] = SYN_CCP_CMD_EXCHANGE_ID;
    cro[1] = 0x04;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x08, dto[3]);

    /* DISCONNECT */
    cro[0] = SYN_CCP_CMD_DISCONNECT;
    cro[1] = 0x05;
    cro[2] = 0x00; /* Temporary disconnect */
    cro[3] = 0x34;
    cro[4] = 0x12;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_FALSE(g_ccp_slave.connected);
}

static void test_ccp_mta_upload_download(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};
    uint8_t buffer[16] = {0};

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* SET_MTA1 via CRO */
    cro[0] = SYN_CCP_CMD_SET_MTA;
    cro[1] = 0x02;
    cro[2] = 0x01; /* MTA1 */
    cro[3] = 0x02; /* Ext */
    cro[4] = 0;
    cro[5] = 0;
    cro[6] = 0;
    cro[7] = 0;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, g_ccp_slave.mta1_ext);

    /* SET_MTA0 via CRO with non-zero dummy low bits */
    cro[2] = 0x00; /* MTA0 */
    cro[4] = 0x10;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* Restore MTA0 address */
    uintptr_t target_addr = (uintptr_t)buffer;
    syn_ccp_set_mta(&g_ccp_slave, 0, 0x01, target_addr);

    /* DNLOAD 4 bytes */
    cro[0] = SYN_CCP_CMD_DNLOAD;
    cro[1] = 0x03;
    cro[2] = 0x04; /* Size */
    cro[3] = 0xAA;
    cro[4] = 0xBB;
    cro[5] = 0xCC;
    cro[6] = 0xDD;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, buffer[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, buffer[2]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, buffer[3]);

    /* DNLOAD_6 */
    cro[0] = SYN_CCP_CMD_DNLOAD_6;
    cro[1] = 0x04;
    cro[2] = 0x11;
    cro[3] = 0x22;
    cro[4] = 0x33;
    cro[5] = 0x44;
    cro[6] = 0x55;
    cro[7] = 0x66;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x11, buffer[4]);
    TEST_ASSERT_EQUAL_HEX8(0x66, buffer[9]);

    /* UPLOAD */
    syn_ccp_set_mta(&g_ccp_slave, 0, 0x01, (uintptr_t)buffer);
    cro[0] = SYN_CCP_CMD_UPLOAD;
    cro[1] = 0x05;
    cro[2] = 0x04;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, dto[3]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, dto[4]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, dto[5]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, dto[6]);

    /* SHORT_UP via CRO */
    syn_ccp_set_mta(&g_ccp_slave, 0, 0x01, (uintptr_t)(buffer + 4));
    cro[0] = SYN_CCP_CMD_SHORT_UP;
    cro[1] = 0x06;
    cro[2] = 0x02; /* Size */
    cro[3] = 0x01; /* Ext */
    cro[4] = 0;
    cro[5] = 0;
    cro[6] = 0;
    cro[7] = 0;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x11, dto[3]);
    TEST_ASSERT_EQUAL_HEX8(0x22, dto[4]);

    /* SHORT_UP with mta0_addr = 0 and non-zero addr offset with size = 0 */
    g_ccp_slave.mta0_addr = 0;
    cro[2] = 0x00; /* Size = 0 */
    cro[4] = 0x10;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* Null MTA setter test */
    syn_ccp_set_mta(NULL, 0, 0, 0);

    /* DISCONNECT with address mismatch (0x5555) */
    cro[0] = SYN_CCP_CMD_DISCONNECT;
    cro[1] = 0x99;
    cro[2] = 0x00;
    cro[3] = 0x55;
    cro[4] = 0x55;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_TRUE(g_ccp_slave.connected); /* Still connected due to mismatch */
}

static void test_ccp_daq_list_streaming(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};
    uint32_t sample_val = 0x12345678U;

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* GET_DAQ_SIZE */
    cro[0] = SYN_CCP_CMD_GET_DAQ_SIZE;
    cro[1] = 0x02;
    cro[2] = 0x00; /* DAQ List 0 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_MAX_ODT_PER_DAQ, dto[3]);

    /* SET_DAQ_SIZE */
    cro[0] = SYN_CCP_CMD_SET_DAQ_SIZE;
    cro[1] = 0x03;
    cro[2] = 0x00; /* DAQ 0 */
    cro[4] = 0x02; /* 2 ODTs */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* BUILD_PKT (Add ODT 0 entry 0) */
    uintptr_t target_addr = (uintptr_t)&sample_val;
    g_ccp_slave.daq_lists[0].odts[0].entries[0].address = target_addr;
    g_ccp_slave.daq_lists[0].odts[0].entries[0].size = 4;
    g_ccp_slave.daq_lists[0].odts[0].entries[0].host_valid = true;
    g_ccp_slave.daq_lists[0].odts[0].entry_count = 1;
    g_ccp_slave.daq_lists[0].odts[1].entries[0].address = 0x12340000U;
    g_ccp_slave.daq_lists[0].odts[1].entries[0].size = 4;
    g_ccp_slave.daq_lists[0].odts[1].entries[0].host_valid = false;
    g_ccp_slave.daq_lists[0].odts[1].entry_count = 1;

    /* START_STOP (Start DAQ 0) */
    cro[0] = SYN_CCP_CMD_START_STOP;
    cro[1] = 0x05;
    cro[2] = 0x01; /* Start */
    cro[3] = 0x00; /* DAQ 0 */
    cro[5] = 0x01; /* Event channel 1 */
    cro[6] = 0x01; /* Prescaler 1 (LSB) */
    cro[7] = 0x00; /* Prescaler 1 (MSB) */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* Service DAQ tick */
    uint8_t list_idx = 0;
    uint8_t odt_idx = 0;
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 0x01, dto, &list_idx, &odt_idx));
    TEST_ASSERT_EQUAL_HEX8(0x00, dto[0]); /* Packet ID 0 */
    TEST_ASSERT_EQUAL_HEX8(0x78, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x56, dto[2]);
    TEST_ASSERT_EQUAL_HEX8(0x34, dto[3]);
    TEST_ASSERT_EQUAL_HEX8(0x12, dto[4]);

    /* ODT 1 has host_valid == false with region_count == 0 -> must not dereference 0x12340000U */
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 0x01, dto, &list_idx, &odt_idx));
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, dto[1]);

    /* START_STOP_ALL (Stop all) */
    cro[0] = SYN_CCP_CMD_START_STOP_ALL;
    cro[1] = 0x06;
    cro[2] = 0x00; /* Stop */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 0x01, dto, &list_idx, &odt_idx));
}

static void test_ccp_seed_unlock_and_cal_page(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* GET_SEED */
    cro[0] = SYN_CCP_CMD_GET_SEED;
    cro[1] = 0x02;
    cro[2] = SYN_CCP_RESOURCE_CAL;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, dto[3]); /* Resource already unlocked */

    cro[2] = 0x80; /* Locked resource bit */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[3]); /* Seed needed */

    /* UNLOCK */
    cro[0] = SYN_CCP_CMD_UNLOCK;
    cro[1] = 0x03;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* SET_CAL_PAGE and GET_ACTIVE_CAL_PAGE */
    cro[0] = SYN_CCP_CMD_SET_CAL_PAGE;
    cro[1] = 0x04;
    cro[2] = 0x02; /* Page 2 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    cro[0] = SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE;
    cro[1] = 0x05;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, dto[4]);
}

static uint32_t ccp_seed_generator(uint8_t resource, void *ctx)
{
    (void)ctx;
    return (uint32_t)resource * 0x11111111U;
}

static bool ccp_unlock_validator(uint8_t resource, uint32_t key, void *ctx)
{
    (void)ctx;
    return key == ((uint32_t)resource * 0x11111111U);
}

static bool ccp_erase_handler(uint32_t addr, uint32_t size, void *ctx)
{
    (void)addr;
    (void)ctx;
    return size > 0;
}

static void test_ccp_callbacks_seed_unlock_erase(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    g_ccp_slave.seed_cb = ccp_seed_generator;
    g_ccp_slave.unlock_cb = ccp_unlock_validator;
    g_ccp_slave.erase_cb = ccp_erase_handler;

    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* GET_SEED using callback */
    cro[0] = SYN_CCP_CMD_GET_SEED;
    cro[1] = 0x02;
    cro[2] = SYN_CCP_RESOURCE_PGM;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* UNLOCK with invalid key -> ACCESS_DENIED */
    cro[0] = SYN_CCP_CMD_UNLOCK;
    cro[1] = 0x03;
    cro[2] = SYN_CCP_RESOURCE_PGM;
    cro[4] = 0x00;
    cro[5] = 0x00;
    cro[6] = 0x00;
    cro[7] = 0x00;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);

    /* UNLOCK with valid key -> SUCCESS */
    uint32_t valid_key = (uint32_t)SYN_CCP_RESOURCE_PGM * 0x11111111U;
    cro[4] = (uint8_t)(valid_key);
    cro[5] = (uint8_t)(valid_key >> 8);
    cro[6] = (uint8_t)(valid_key >> 16);
    cro[7] = (uint8_t)(valid_key >> 24);
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* CLEAR_MEMORY with callback success */
    cro[0] = SYN_CCP_CMD_CLEAR_MEMORY;
    cro[1] = 0x04;
    cro[2] = 0x00;
    cro[3] = 0x10; /* Size 0x1000 */
    cro[4] = 0x00;
    cro[5] = 0x00;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* CLEAR_MEMORY with callback failure (size = 0) -> ACCESS_DENIED */
    cro[3] = 0x00; /* Size 0 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);
}

static void test_ccp_program_and_clear_memory(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};
    uint8_t buffer[16] = {0};

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* SET_MTA1 */
    syn_ccp_set_mta(&g_ccp_slave, 1, 0x01, (uintptr_t)buffer);
    TEST_ASSERT_EQUAL_PTR((void *)(uintptr_t)buffer, (void *)g_ccp_slave.mta1_addr);

    /* CLEAR_MEMORY */
    cro[0] = SYN_CCP_CMD_CLEAR_MEMORY;
    cro[1] = 0x02;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* PROGRAM */
    syn_ccp_set_mta(&g_ccp_slave, 0, 0x01, (uintptr_t)buffer);
    cro[0] = SYN_CCP_CMD_PROGRAM;
    cro[1] = 0x03;
    cro[2] = 0x02; /* 2 bytes */
    cro[3] = 0x55;
    cro[4] = 0xAA;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x55, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, buffer[1]);

    /* PROGRAM via DNLOAD_6 */
    cro[0] = SYN_CCP_CMD_DNLOAD_6;
    cro[1] = 0x04;
    cro[2] = 0x11;
    cro[3] = 0x22;
    cro[4] = 0x33;
    cro[5] = 0x44;
    cro[6] = 0x55;
    cro[7] = 0x66;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x11, buffer[2]);
    TEST_ASSERT_EQUAL_HEX8(0x66, buffer[7]);
}

static void test_ccp_extended_cro_and_daq(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};
    uint8_t buffer[16] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60};

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* SET_DAQ_SIZE */
    cro[0] = SYN_CCP_CMD_SET_DAQ_SIZE;
    cro[1] = 0x02;
    cro[2] = 0x00; /* DAQ 0 */
    cro[4] = 0x01; /* 1 ODT */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));

    /* BUILD_PKT via CRO (uses MTA0) */
    syn_ccp_set_mta(&g_ccp_slave, 0, 0x01, (uintptr_t)buffer);
    cro[0] = SYN_CCP_CMD_BUILD_PKT;
    cro[1] = 0x03;
    cro[2] = 0x00; /* DAQ 0 */
    cro[3] = 0x00; /* ODT 0 */
    cro[4] = 0x00; /* Element 0 */
    cro[5] = 0x02; /* 2 bytes */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    /* START_STOP (prescaler = 2) */
    cro[0] = SYN_CCP_CMD_START_STOP;
    cro[1] = 0x03;
    cro[2] = 0x01; /* Start */
    cro[3] = 0x00; /* DAQ 0 */
    cro[5] = 0x01; /* Event channel 1 */
    cro[6] = 0x02; /* Prescaler 2 */
    cro[7] = 0x00;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);

    uint8_t list_idx = 0;
    uint8_t odt_idx = 0;
    /* Tick 1: prescaler counter incremented, no frame sent */
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 0x01, dto, &list_idx, &odt_idx));
    /* Tick 2: prescaler threshold reached -> frame sent */
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 0x01, dto, &list_idx, &odt_idx));
    TEST_ASSERT_EQUAL_HEX8(0x00, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x10, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x20, dto[2]);

    /* Stop DAQ 0 */
    cro[0] = SYN_CCP_CMD_START_STOP;
    cro[1] = 0x04;
    cro[2] = 0x00; /* Stop */
    cro[3] = 0x00;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_FALSE(g_ccp_slave.daq_lists[0].running);
}

static void test_ccp_null_checks_and_bounds(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};
    uint8_t list_idx = 0;
    uint8_t odt_idx = 0;

    syn_ccp_set_mta(NULL, 0, 0, 0);

    TEST_ASSERT_FALSE(syn_ccp_process_cro(NULL, cro, dto));
    TEST_ASSERT_FALSE(syn_ccp_process_cro(&g_ccp_slave, NULL, dto));
    TEST_ASSERT_FALSE(syn_ccp_process_cro(&g_ccp_slave, cro, NULL));

    TEST_ASSERT_FALSE(syn_ccp_service_daq(NULL, 0, dto, &list_idx, &odt_idx));
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 0, NULL, &list_idx, &odt_idx));

    /* Connect */
    cro[0] = SYN_CCP_CMD_CONNECT;
    cro[1] = 0x01;
    cro[2] = 0x34;
    cro[3] = 0x12;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    /* Unknown command code */
    cro[0] = 0x99;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_CMD_UNKNOWN, dto[1]);

    /* Oversized DNLOAD length */
    cro[0] = SYN_CCP_CMD_DNLOAD;
    cro[2] = 0x08; /* > 5 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Oversized UPLOAD length */
    cro[0] = SYN_CCP_CMD_UPLOAD;
    cro[2] = 0x08; /* > 5 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Oversized SHORT_UP length */
    cro[0] = SYN_CCP_CMD_SHORT_UP;
    cro[2] = 0x08; /* > 5 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Oversized PROGRAM length */
    cro[0] = SYN_CCP_CMD_PROGRAM;
    cro[2] = 0x08; /* > 5 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Out of bounds DAQ list size */
    cro[0] = SYN_CCP_CMD_SET_DAQ_SIZE;
    cro[2] = 0x80; /* Invalid DAQ index */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Out of bounds GET_DAQ_SIZE index */
    cro[0] = SYN_CCP_CMD_GET_DAQ_SIZE;
    cro[2] = 0x80;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Out of bounds BUILD_PKT index */
    cro[0] = SYN_CCP_CMD_BUILD_PKT;
    cro[2] = 0x80;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* Out of bounds START_STOP index */
    cro[0] = SYN_CCP_CMD_START_STOP;
    cro[3] = 0x80;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_PARAM_OUT_OF_RANGE, dto[1]);

    /* SET_MTA with addr = 0 & mta_num = 0 (line 119) */
    cro[0] = SYN_CCP_CMD_SET_MTA;
    cro[2] = 0;    /* mta_num = 0 */
    cro[3] = 0x55; /* ext */
    cro[4] = 0;
    cro[5] = 0;
    cro[6] = 0;
    cro[7] = 0; /* addr = 0 */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(0x55, g_ccp_slave.mta0_ext);

    /* daq_event when slave disconnected (line 307) */
    g_ccp_slave.connected = false;
    uint8_t daq_dto[8], l_idx, o_idx;
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 0x01, daq_dto, &l_idx, &o_idx));

    /* DAQ service with current_odt_idx >= odt_count (line 324) and odt_count == 0 (line 329) */
    g_ccp_slave.connected = true;
    g_ccp_slave.daq_lists[0].running = true;
    g_ccp_slave.daq_lists[0].event_channel = 0x02;
    g_ccp_slave.daq_lists[0].prescaler = 1;
    g_ccp_slave.daq_lists[0].cycle_counter = 0;
    g_ccp_slave.daq_lists[0].odt_count = 0; /* zero ODT count -> line 329 */
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 0x02, daq_dto, &l_idx, &o_idx));

    g_ccp_slave.daq_lists[0].odt_count = 1;
    g_ccp_slave.daq_lists[0].current_odt_idx =
        5; /* invalid current_odt_idx >= odt_count -> lines 324-325 */
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 0x02, daq_dto, &l_idx, &o_idx));
}

static void test_r5_a11_resource_locks_mem_regions_and_multi_daq(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t ram_buf[16] = {0};
    uint8_t rom_buf[8] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18};
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};

    /* Region registration validation and full-table checks */
    TEST_ASSERT_FALSE(syn_ccp_register_mem_region(NULL, 0x20000000U, ram_buf, 16U, true));
    TEST_ASSERT_FALSE(syn_ccp_register_mem_region(&g_ccp_slave, 0x20000000U, NULL, 16U, true));
    TEST_ASSERT_FALSE(syn_ccp_register_mem_region(&g_ccp_slave, 0x20000000U, ram_buf, 0U, true));
    TEST_ASSERT_TRUE(
        syn_ccp_register_mem_region(&g_ccp_slave, 0x20000000U, ram_buf, sizeof(ram_buf), true));
    TEST_ASSERT_TRUE(
        syn_ccp_register_mem_region(&g_ccp_slave, 0x08000000U, rom_buf, sizeof(rom_buf), false));
    TEST_ASSERT_TRUE(syn_ccp_register_mem_region(&g_ccp_slave, 0x30000000U, ram_buf, 4U, true));
    TEST_ASSERT_TRUE(syn_ccp_register_mem_region(&g_ccp_slave, 0x40000000U, ram_buf, 4U, true));
    TEST_ASSERT_FALSE(syn_ccp_register_mem_region(&g_ccp_slave, 0x50000000U, ram_buf, 4U, true));

    /* Connect via broadcast 0x0000 and lock all resources */
    cro[0] = SYN_CCP_CMD_CONNECT;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    g_ccp_slave.unlocked_resources = 0U;

    /* Locked CAL, PGM, and DAQ commands must return SYN_CCP_ERR_RESOURCE_LOCKED */
    cro[0] = SYN_CCP_CMD_DNLOAD;
    cro[2] = 2U;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_RESOURCE_LOCKED, dto[1]);

    cro[0] = SYN_CCP_CMD_SET_CAL_PAGE;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_RESOURCE_LOCKED, dto[1]);

    cro[0] = SYN_CCP_CMD_PROGRAM;
    cro[2] = 2U;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_RESOURCE_LOCKED, dto[1]);

    cro[0] = SYN_CCP_CMD_GET_DAQ_SIZE;
    cro[2] = 0U;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_RESOURCE_LOCKED, dto[1]);

    uint8_t l_idx = 0U, o_idx = 0U;
    syn_ccp_daq_event(NULL, 1U);
    syn_ccp_daq_event(&g_ccp_slave, 1U);
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));

    /* Unlock via GET_SEED + UNLOCK (with pending_unlock_resource fallback when cro[2]==0) */
    g_ccp_slave.seed_cb = ccp_seed_generator;
    g_ccp_slave.unlock_cb = ccp_unlock_validator;
    uint8_t all_mask = SYN_CCP_RESOURCE_CAL | SYN_CCP_RESOURCE_DAQ | SYN_CCP_RESOURCE_PGM;
    cro[0] = SYN_CCP_CMD_GET_SEED;
    cro[2] = all_mask;
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);

    uint32_t key = (uint32_t)all_mask * 0x11111111U;
    cro[0] = SYN_CCP_CMD_UNLOCK;
    cro[2] = 0U;
    cro[4] = (uint8_t)key;
    cro[5] = (uint8_t)(key >> 8U);
    cro[6] = (uint8_t)(key >> 16U);
    cro[7] = (uint8_t)(key >> 24U);
    syn_ccp_process_cro(&g_ccp_slave, cro, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, dto[3]); /* 0 = no protected resources remaining */

    /* 32-bit wire SET_MTA + DNLOAD + SHORT_UP on 64-bit host via registered region */
    uint8_t set_mta_ram[8] = {SYN_CCP_CMD_SET_MTA, 1U, 0U, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_ccp_process_cro(&g_ccp_slave, set_mta_ram, dto);
    uint8_t set_mta1_wire[8] = {SYN_CCP_CMD_SET_MTA, 1U, 1U, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_ccp_process_cro(&g_ccp_slave, set_mta1_wire, dto);
    syn_ccp_set_mta(&g_ccp_slave, 9U, 0U, 0U);

    uint8_t dn_ram[8] = {SYN_CCP_CMD_DNLOAD, 2U, 4U, 0x11U, 0x22U, 0x33U, 0x44U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x44U, ram_buf[7]);

    uint8_t sup_ram[8] = {SYN_CCP_CMD_SHORT_UP, 3U, 4U, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_ccp_process_cro(&g_ccp_slave, sup_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(0x11U, dto[3]);
    TEST_ASSERT_EQUAL_HEX8(0x44U, dto[6]);

    /* Host pointer within registered region also resolves cleanly */
    syn_ccp_set_mta(&g_ccp_slave, 0U, 0U, (uintptr_t)&ram_buf[0]);
    uint8_t pgm_ok[8] = {SYN_CCP_CMD_PROGRAM, 4U, 2U, 0x99U, 0x88U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, pgm_ok, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x99U, ram_buf[0]);

    /* Out-of-bounds, unmapped, and read-only write attempts must return ACCESS_DENIED */
    uint8_t set_mta_ro[8] = {SYN_CCP_CMD_SET_MTA, 5U, 0U, 0U, 0x00U, 0x00U, 0x00U, 0x08U};
    syn_ccp_process_cro(&g_ccp_slave, set_mta_ro, dto);
    syn_ccp_process_cro(&g_ccp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);
    syn_ccp_process_cro(&g_ccp_slave, pgm_ok, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);

    uint8_t set_mta_oob[8] = {SYN_CCP_CMD_SET_MTA, 6U, 0U, 0U, 0x0EU, 0x00U, 0x00U, 0x20U};
    syn_ccp_process_cro(&g_ccp_slave, set_mta_oob, dto);
    syn_ccp_process_cro(&g_ccp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);

    uint8_t sup_unmapped[8] = {SYN_CCP_CMD_SHORT_UP, 7U, 2U, 0U, 0x00U, 0x00U, 0x00U, 0x90U};
    syn_ccp_process_cro(&g_ccp_slave, sup_unmapped, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_ACCESS_DENIED, dto[1]);

    /* Multi-list & multi-ODT round-robin DAQ + per-list prescaler isolation */
    ram_buf[0] = 0xAAU;
    ram_buf[4] = 0xBBU;
    rom_buf[0] = 0xCCU;

    uint8_t sz0[8] = {SYN_CCP_CMD_SET_DAQ_SIZE, 1U, 0U, 0U, 2U, 0U, 0U, 0U};
    uint8_t sz1[8] = {SYN_CCP_CMD_SET_DAQ_SIZE, 2U, 1U, 0U, 1U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, sz0, dto);
    syn_ccp_process_cro(&g_ccp_slave, sz1, dto);

    /* List 0 ODT 0 -> 0x20000000 (1B), List 0 ODT 1 -> 0x20000004 (1B), List 1 ODT 0 -> 0x08000000
     */
    uint8_t m0[8] = {SYN_CCP_CMD_SET_MTA, 1U, 0U, 0U, 0x00U, 0x00U, 0x00U, 0x20U};
    uint8_t b00[8] = {SYN_CCP_CMD_BUILD_PKT, 2U, 0U, 0U, 0U, 1U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, m0, dto);
    syn_ccp_process_cro(&g_ccp_slave, b00, dto);

    uint8_t m1[8] = {SYN_CCP_CMD_SET_MTA, 3U, 0U, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    uint8_t b01[8] = {SYN_CCP_CMD_BUILD_PKT, 4U, 0U, 1U, 0U, 1U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, m1, dto);
    syn_ccp_process_cro(&g_ccp_slave, b01, dto);

    syn_ccp_process_cro(&g_ccp_slave, set_mta_ro, dto);
    uint8_t b10[8] = {SYN_CCP_CMD_BUILD_PKT, 5U, 1U, 0U, 0U, 1U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, b10, dto);

    uint8_t st0[8] = {SYN_CCP_CMD_START_STOP, 6U, 1U, 0U, 0U, 1U, 0U, 0U}; /* prescaler 0 -> 1 */
    uint8_t st1[8] = {SYN_CCP_CMD_START_STOP, 7U, 1U, 1U, 0U, 1U, 1U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, st0, dto);
    syn_ccp_process_cro(&g_ccp_slave, st1, dto);

    /* Drain 3 ODTs across List 0 (2 ODTs) and List 1 (1 ODT) in round-robin order */
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(0U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(0U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, dto[1]);

    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(1U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(0U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0xCCU, dto[1]);

    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(0U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(1U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0xBBU, dto[1]);
    /* End-of-tick drain terminates cleanly without re-stepping prescalers */
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));

    /* Explicit syn_ccp_daq_event + bounded while() drain across next tick */
    syn_ccp_daq_event(&g_ccp_slave, 1U);
    uint8_t drained = 0U;
    while (drained < 8U && syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx)) {
        drained++;
    }
    TEST_ASSERT_EQUAL_UINT8(3U, drained);

    /* Zero-size DNLOAD/PROGRAM paths */
    uint8_t dn_zero[8] = {SYN_CCP_CMD_DNLOAD, 8U, 0U, 0U, 0U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, dn_zero, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    uint8_t pgm_zero[8] = {SYN_CCP_CMD_PROGRAM, 9U, 0U, 0U, 0U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, pgm_zero, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, NULL, &o_idx));
    TEST_ASSERT_FALSE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, NULL));

    /* DISCONNECT with mode 0x01 (end of session) vs invalid mode 0x99 (ignored) */
    uint8_t disc_bad_mode[8] = {SYN_CCP_CMD_DISCONNECT, 10U, 0x99U, 0x34U, 0x12U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, disc_bad_mode, dto);
    TEST_ASSERT_TRUE(g_ccp_slave.connected);

    uint8_t disc_perm[8] = {SYN_CCP_CMD_DISCONNECT, 11U, 0x01U, 0x34U, 0x12U, 0U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, disc_perm, dto);
    TEST_ASSERT_FALSE(g_ccp_slave.connected);
}

static void test_ccp_pro17_wraparound_daq_clear_tear_and_disconnect(void)
{
    syn_ccp_init(&g_ccp_slave, 0x1234U);
    uint8_t high_ram[16] = {0x10U, 0x20U, 0x30U, 0x40U, 0xA1U, 0xA2U, 0xA3U, 0xA4U,
                            0xB1U, 0xB2U, 0xB3U, 0xB4U, 0xC1U, 0xC2U, 0xC3U, 0xC4U};
    uint8_t cro[8] = {0};
    uint8_t dto[8] = {0};

    /* 1. 32-bit region wraparound guard (0xFFFFFFF8 + 16 wraps in u32) */
    TEST_ASSERT_TRUE(
        syn_ccp_register_mem_region(&g_ccp_slave, 0xFFFFFFF8U, high_ram, sizeof(high_ram), true));

    uint8_t conn[8] = {SYN_CCP_CMD_CONNECT, 1U, 0x34U, 0x12U, 0U, 0U, 0U, 0U};
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, conn, dto));

    uint8_t sup_high[8] = {SYN_CCP_CMD_SHORT_UP, 2U, 4U, 0U, 0xFCU, 0xFFU, 0xFFU, 0xFFU};
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, sup_high, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_CCP_ERR_SUCCESS, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xA1U, dto[3]);
    TEST_ASSERT_EQUAL_HEX8(0xA4U, dto[6]);

    /* 2. SET_DAQ_SIZE clears stale ODT entries, and ccp_pack_odt stops without tearing when
     *    entry->size (4) > remaining payload bytes (3 after first 4-byte entry) */
    uint8_t sz0[8] = {SYN_CCP_CMD_SET_DAQ_SIZE, 3U, 0U, 0U, 1U, 0U, 0U, 0U};
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, sz0, dto));

    uint8_t mta_e0[8] = {SYN_CCP_CMD_SET_MTA, 4U, 0U, 0U, 0xF8U, 0xFFU, 0xFFU, 0xFFU};
    uint8_t bld_e0[8] = {SYN_CCP_CMD_BUILD_PKT, 5U, 0U, 0U, 0U, 4U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, mta_e0, dto);
    syn_ccp_process_cro(&g_ccp_slave, bld_e0, dto);

    uint8_t mta_e1[8] = {SYN_CCP_CMD_SET_MTA, 6U, 0U, 0U, 0xFCU, 0xFFU, 0xFFU, 0xFFU};
    uint8_t bld_e1[8] = {SYN_CCP_CMD_BUILD_PKT, 7U, 0U, 0U, 1U, 4U, 0U, 0U};
    syn_ccp_process_cro(&g_ccp_slave, mta_e1, dto);
    syn_ccp_process_cro(&g_ccp_slave, bld_e1, dto);

    uint8_t st0[8] = {SYN_CCP_CMD_START_STOP, 8U, 1U, 0U, 0U, 1U, 1U, 0U};
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, st0, dto));

    uint8_t l_idx = 0U;
    uint8_t o_idx = 0U;
    TEST_ASSERT_TRUE(syn_ccp_service_daq(&g_ccp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_HEX8(0x10U, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x40U, dto[4]);
    /* Second 4-byte entry did not fit in remaining 3 bytes -> zero-padded, NOT torn! */
    TEST_ASSERT_EQUAL_HEX8(0x00U, dto[5]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, dto[6]);
    TEST_ASSERT_EQUAL_HEX8(0x00U, dto[7]);

    /* Reconfiguring DAQ size clears old ODT entries */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, sz0, dto));
    TEST_ASSERT_EQUAL_UINT8(0U, g_ccp_slave.daq_lists[0].odts[0].entry_count);

    /* 3. DISCONNECT stops all running DAQ lists */
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, st0, dto));
    TEST_ASSERT_TRUE(g_ccp_slave.daq_lists[0].running);
    cro[0] = SYN_CCP_CMD_DISCONNECT;
    cro[1] = 9U;
    cro[2] = 0x00U;
    cro[3] = 0x00U; /* Broadcast disconnect */
    cro[4] = 0x00U;
    TEST_ASSERT_TRUE(syn_ccp_process_cro(&g_ccp_slave, cro, dto));
    TEST_ASSERT_FALSE(g_ccp_slave.connected);
    for (uint8_t i = 0U; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
        TEST_ASSERT_FALSE(g_ccp_slave.daq_lists[i].running);
    }
}

void run_ccp_tests(void)
{
    RUN_TEST(test_ccp_init_and_connect_disconnect);
    RUN_TEST(test_ccp_mta_upload_download);
    RUN_TEST(test_ccp_daq_list_streaming);
    RUN_TEST(test_ccp_seed_unlock_and_cal_page);
    RUN_TEST(test_ccp_callbacks_seed_unlock_erase);
    RUN_TEST(test_ccp_program_and_clear_memory);
    RUN_TEST(test_ccp_extended_cro_and_daq);
    RUN_TEST(test_ccp_null_checks_and_bounds);
    RUN_TEST(test_r5_a11_resource_locks_mem_regions_and_multi_daq);
    RUN_TEST(test_ccp_pro17_wraparound_daq_clear_tear_and_disconnect);
}
