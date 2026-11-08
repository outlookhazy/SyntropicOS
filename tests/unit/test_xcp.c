/**
 * @file test_xcp.c
 * @brief Unit tests for ASAM XCP slave protocol implementation.
 */

#include "syntropic/proto/syn_xcp.h"
#include "unity/unity.h"

#include <string.h>

static SYN_XCP_Slave g_xcp_slave;

static void test_xcp_init_and_connect_disconnect(void)
{
    TEST_ASSERT_FALSE(syn_xcp_init(NULL, 0x1234U));
    TEST_ASSERT_TRUE(syn_xcp_init(&g_xcp_slave, 0x1234U));
    TEST_ASSERT_EQUAL_HEX16(0x1234U, g_xcp_slave.station_id);

    uint8_t cto[8] = {0};
    uint8_t dto[8] = {0};

    /* Command before CONNECT -> ERR_NOT_CONNECTED */
    cto[0] = SYN_XCP_CMD_GET_STATUS;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_NOT_CONNECTED, dto[1]);

    /* CONNECT */
    cto[0] = SYN_XCP_CMD_CONNECT;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_TRUE(g_xcp_slave.connected);

    /* GET_STATUS */
    cto[0] = SYN_XCP_CMD_GET_STATUS;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, dto[2]); /* 0x00 = all resources unlocked */

    /* GET_COMM_MODE_INFO */
    cto[0] = SYN_XCP_CMD_GET_COMM_MODE_INFO;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[2]);

    /* GET_ID */
    cto[0] = SYN_XCP_CMD_GET_ID;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* GET_SEED & UNLOCK */
    cto[0] = SYN_XCP_CMD_GET_SEED;
    cto[1] = 0x00;
    cto[2] = SYN_XCP_RESOURCE_CAL_PAG;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04, dto[1]);

    /* UNLOCK syntax error vs success */
    cto[0] = SYN_XCP_CMD_UNLOCK;
    cto[1] = 0x00;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    cto[1] = 0x04;
    cto[2] = 0x12;
    cto[3] = 0x34;
    cto[4] = 0x56;
    cto[5] = 0x78;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* DISCONNECT */
    cto[0] = SYN_XCP_CMD_DISCONNECT;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_FALSE(g_xcp_slave.connected);
}

static void test_xcp_mta_upload_download(void)
{
    syn_xcp_init(&g_xcp_slave, 0x1234U);
    uint8_t cto[8] = {0};
    uint8_t dto[8] = {0};
    uint8_t buffer[16] = {0};

    cto[0] = SYN_XCP_CMD_CONNECT;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);

    /* SET_MTA with 0 addr */
    TEST_ASSERT_TRUE(syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)buffer));
    cto[0] = SYN_XCP_CMD_SET_MTA;
    cto[3] = 0x01;
    cto[4] = 0;
    cto[5] = 0;
    cto[6] = 0;
    cto[7] = 0;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* SET_MTA with non-zero dummy addr */
    cto[4] = 0x10;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* Restore MTA pointer */
    syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)buffer);

    /* DOWNLOAD 4 bytes */
    cto[0] = SYN_XCP_CMD_DOWNLOAD;
    cto[1] = 0x04;
    cto[2] = 0xDE;
    cto[3] = 0xAD;
    cto[4] = 0xBE;
    cto[5] = 0xEF;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0xDE, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAD, buffer[1]);
    TEST_ASSERT_EQUAL_HEX8(0xBE, buffer[2]);
    TEST_ASSERT_EQUAL_HEX8(0xEF, buffer[3]);

    /* DOWNLOAD_MAX 7 bytes */
    cto[0] = SYN_XCP_CMD_DOWNLOAD_MAX;
    cto[1] = 0x11;
    cto[2] = 0x22;
    cto[3] = 0x33;
    cto[4] = 0x44;
    cto[5] = 0x55;
    cto[6] = 0x66;
    cto[7] = 0x77;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x11, buffer[4]);
    TEST_ASSERT_EQUAL_HEX8(0x77, buffer[10]);

    /* UPLOAD 4 bytes */
    syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)buffer);
    cto[0] = SYN_XCP_CMD_UPLOAD;
    cto[1] = 0x04;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0xDE, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xAD, dto[2]);

    /* SHORT_UPLOAD */
    syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)(buffer + 4));
    cto[0] = SYN_XCP_CMD_SHORT_UPLOAD;
    cto[1] = 0x02;
    cto[3] = 0x01;
    cto[4] = 0;
    cto[5] = 0;
    cto[6] = 0;
    cto[7] = 0;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x11, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0x22, dto[2]);

    /* Out of range UPLOAD / DOWNLOAD */
    cto[0] = SYN_XCP_CMD_UPLOAD;
    cto[1] = 0x08;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    cto[0] = SYN_XCP_CMD_DOWNLOAD;
    cto[1] = 0x08;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    cto[0] = SYN_XCP_CMD_SHORT_UPLOAD;
    cto[1] = 0x08;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);
}

static void test_xcp_daq_list_streaming(void)
{
    syn_xcp_init(&g_xcp_slave, 0x1234U);
    uint8_t cto[8] = {0};
    uint8_t dto[8] = {0};
    uint16_t var1 = 0x1234U;
    uint32_t var2 = 0xABCDEF01U;

    cto[0] = SYN_XCP_CMD_CONNECT;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);

    /* SET_DAQ_PTR List 0, ODT 0, Element 0 */
    cto[0] = SYN_XCP_CMD_SET_DAQ_PTR;
    cto[2] = 0x00;
    cto[3] = 0x00;
    cto[4] = 0x00;
    cto[5] = 0x00;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* WRITE_DAQ element 0 (var1, 2 bytes) */
    cto[0] = SYN_XCP_CMD_WRITE_DAQ;
    cto[2] = 0x02;
    cto[3] = 0x01;
    syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)&var1);
    cto[4] = 0;
    cto[5] = 0;
    cto[6] = 0;
    cto[7] = 0;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* WRITE_DAQ element 1 (var2, 4 bytes) */
    cto[0] = SYN_XCP_CMD_WRITE_DAQ;
    cto[2] = 0x04;
    cto[3] = 0x01;
    syn_xcp_set_mta(&g_xcp_slave, 0x01, (uintptr_t)&var2);
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* SET_DAQ_LIST_MODE with Prescaler = 2 */
    cto[0] = SYN_XCP_CMD_SET_DAQ_LIST_MODE;
    cto[1] = 0x01;
    cto[2] = 0x00;
    cto[3] = 0x00;
    cto[4] = 0x01; /* Event channel 1 */
    cto[5] = 0x02; /* Prescaler 2 */
    cto[6] = 0x00;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* START_STOP_DAQ_LIST Start */
    cto[0] = SYN_XCP_CMD_START_STOP_DAQ_LIST;
    cto[1] = 0x01; /* Start */
    cto[2] = 0x00;
    cto[3] = 0x00;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, dto[1]); /* ODT count */

    /* First tick: cycle_counter = 1 (< 2), service_daq returns false */
    uint8_t daq_dto[8] = {0};
    uint8_t list_idx = 0;
    uint8_t odt_idx = 0;
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 0x01U, daq_dto, &list_idx, &odt_idx));

    /* Second tick: cycle_counter = 2, service_daq returns true */
    TEST_ASSERT_TRUE(syn_xcp_service_daq(&g_xcp_slave, 0x01U, daq_dto, &list_idx, &odt_idx));
    TEST_ASSERT_EQUAL_HEX8(0x00, daq_dto[0]); /* ODT index 0 */
    TEST_ASSERT_EQUAL_HEX8(0x34, daq_dto[1]); /* var1 byte 0 */
    TEST_ASSERT_EQUAL_HEX8(0x12, daq_dto[2]); /* var1 byte 1 */
    TEST_ASSERT_EQUAL_HEX8(0x01, daq_dto[3]); /* var2 byte 0 */
    TEST_ASSERT_EQUAL_HEX8(0xEF, daq_dto[4]); /* var2 byte 1 */

    /* START_STOP_SYNCH Stop */
    cto[0] = SYN_XCP_CMD_START_STOP_SYNCH;
    cto[1] = 0x00; /* Stop */
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_FALSE(g_xcp_slave.daq_lists[0].running);
}

static void test_xcp_bounds_and_null_checks(void)
{
    uint8_t cto[8] = {0};
    uint8_t dto[8] = {0};

    TEST_ASSERT_FALSE(syn_xcp_set_mta(NULL, 0, 0));
    TEST_ASSERT_FALSE(syn_xcp_process_cto(NULL, cto, dto));
    TEST_ASSERT_FALSE(syn_xcp_process_cto(&g_xcp_slave, NULL, dto));
    TEST_ASSERT_FALSE(syn_xcp_process_cto(&g_xcp_slave, cto, NULL));

    uint8_t l = 0, o = 0;
    TEST_ASSERT_FALSE(syn_xcp_service_daq(NULL, 1, dto, &l, &o));
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1, NULL, &l, &o));
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1, dto, NULL, &o));
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1, dto, &l, NULL));

    syn_xcp_init(&g_xcp_slave, 0x1234U);
    cto[0] = SYN_XCP_CMD_CONNECT;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);

    /* SYNCH command */
    cto[0] = SYN_XCP_CMD_SYNCH;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_CMD_SYNTAX, dto[1]);

    /* Unknown command fallback */
    cto[0] = 0x11;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_CMD_UNKNOWN, dto[1]);

    /* Out of bounds SET_DAQ_PTR */
    cto[0] = SYN_XCP_CMD_SET_DAQ_PTR;
    cto[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    /* Out of bounds WRITE_DAQ */
    g_xcp_slave.current_daq_ptr_idx = SYN_XCP_MAX_ENTRIES_PER_ODT;
    cto[0] = SYN_XCP_CMD_WRITE_DAQ;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    /* Out of bounds SET_DAQ_LIST_MODE */
    cto[0] = SYN_XCP_CMD_SET_DAQ_LIST_MODE;
    cto[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    /* Out of bounds START_STOP_DAQ_LIST */
    cto[0] = SYN_XCP_CMD_START_STOP_DAQ_LIST;
    cto[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_ERR, dto[0]);

    /* service_daq when list not running */
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1, dto, &l, &o));

    /* 1. SHORT_UPLOAD with non-zero mta_addr (line 168) */
    static const uint8_t test_var_static[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    syn_xcp_set_mta(&g_xcp_slave, 1, (uintptr_t)test_var_static);
    cto[0] = SYN_XCP_CMD_SHORT_UPLOAD;
    cto[1] = 2; /* size = 2 */
    cto[2] = 0;
    cto[3] = 0x01; /* ext */
    cto[4] = 0;
    cto[5] = 0;
    cto[6] = 0;
    cto[7] = 0;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL_HEX8(0xAA, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, dto[2]);

    /* 2. SET_DAQ_LIST_MODE with prescaler = 0 (line 271) */
    cto[0] = SYN_XCP_CMD_SET_DAQ_LIST_MODE;
    cto[1] = 0x01; /* mode */
    cto[2] = 0x00; /* list_idx = 0 */
    cto[3] = 0x00;
    cto[4] = 0x01; /* event_channel = 1 */
    cto[5] = 0x00; /* prescaler = 0 -> line 271 sets prescaler = 1 */
    cto[6] = 0x00;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));
    TEST_ASSERT_EQUAL(1, g_xcp_slave.daq_lists[0].prescaler);

    /* 3. WRITE_DAQ with addr = 0 using mta_addr (line 242) */
    g_xcp_slave.current_daq_ptr_idx = 0;
    cto[0] = SYN_XCP_CMD_WRITE_DAQ;
    cto[1] = 0x00; /* bit_offset */
    cto[2] = 0x02; /* size = 2 */
    cto[3] = 0x01; /* ext */
    cto[4] = 0;
    cto[5] = 0;
    cto[6] = 0;
    cto[7] = 0;
    TEST_ASSERT_TRUE(syn_xcp_process_cto(&g_xcp_slave, cto, dto));

    /* 4. syn_xcp_service_daq with current_odt_idx >= list->odt_count (line 523) */
    g_xcp_slave.daq_lists[0].running = true;
    g_xcp_slave.daq_lists[0].event_channel = 1;
    g_xcp_slave.daq_lists[0].odt_count = 1;
    g_xcp_slave.daq_lists[0].current_odt_idx = 5U;
    l = 0;
    o = 0;
    TEST_ASSERT_TRUE(syn_xcp_service_daq(&g_xcp_slave, 1, dto, &l, &o));
    TEST_ASSERT_EQUAL_UINT8(0U, o);

    /* Reset global slave state */
    syn_xcp_init(&g_xcp_slave, 0x1234U);
}

static uint8_t test_xcp_seed_cb(uint8_t resource, uint8_t seed_out[6], void *ctx)
{
    (void)ctx;
    seed_out[0] = resource;
    seed_out[1] = 0xA5U;
    return 8U; /* Clamped to 6U */
}

static bool test_xcp_unlock_cb(uint8_t resource, const uint8_t *key, uint8_t key_len, void *ctx)
{
    (void)ctx;
    return (key_len == 2U && key[0] == resource && key[1] == 0x5AU);
}

static void test_r5_a11_xcp_locks_regions_and_multi_daq(void)
{
    syn_xcp_init(&g_xcp_slave, 0x1234U);
    uint8_t ram_buf[16] = {0};
    uint8_t rom_buf[8] = {0x10U, 0x20U, 0x30U, 0x40U, 0x50U, 0x60U, 0x70U, 0x80U};
    uint8_t cto[8] = {0};
    uint8_t dto[8] = {0};

    /* Region registration checks */
    TEST_ASSERT_FALSE(syn_xcp_register_mem_region(NULL, 0x20000000U, ram_buf, 16U, true));
    TEST_ASSERT_FALSE(syn_xcp_register_mem_region(&g_xcp_slave, 0x20000000U, NULL, 16U, true));
    TEST_ASSERT_FALSE(syn_xcp_register_mem_region(&g_xcp_slave, 0x20000000U, ram_buf, 0U, true));
    TEST_ASSERT_TRUE(
        syn_xcp_register_mem_region(&g_xcp_slave, 0x20000000U, ram_buf, sizeof(ram_buf), true));
    TEST_ASSERT_TRUE(
        syn_xcp_register_mem_region(&g_xcp_slave, 0x08000000U, rom_buf, sizeof(rom_buf), false));
    TEST_ASSERT_TRUE(syn_xcp_register_mem_region(&g_xcp_slave, 0x30000000U, ram_buf, 4U, true));
    TEST_ASSERT_TRUE(syn_xcp_register_mem_region(&g_xcp_slave, 0x40000000U, ram_buf, 4U, true));
    TEST_ASSERT_FALSE(syn_xcp_register_mem_region(&g_xcp_slave, 0x50000000U, ram_buf, 4U, true));

    /* Connect and lock resources */
    cto[0] = SYN_XCP_CMD_CONNECT;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);
    g_xcp_slave.unlocked_resources = 0U;

    cto[0] = SYN_XCP_CMD_GET_STATUS;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);
    TEST_ASSERT_NOT_EQUAL(0U, dto[2]);

    /* Locked CAL_PAG and DAQ commands must return SYN_XCP_ERR_ACCESS_LOCKED */
    cto[0] = SYN_XCP_CMD_DOWNLOAD;
    cto[1] = 2U;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_ACCESS_LOCKED, dto[1]);

    cto[0] = SYN_XCP_CMD_SET_DAQ_PTR;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_ACCESS_LOCKED, dto[1]);

    cto[0] = SYN_XCP_CMD_START_STOP_SYNCH;
    syn_xcp_process_cto(&g_xcp_slave, cto, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_ACCESS_LOCKED, dto[1]);

    uint8_t l_idx = 0U, o_idx = 0U;
    syn_xcp_daq_event(NULL, 1U);
    syn_xcp_daq_event(&g_xcp_slave, 1U);
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx));

    /* Default key verification rejects bad key and overlong key_len */
    uint8_t over_key[8] = {SYN_XCP_CMD_UNLOCK, 7U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, over_key, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_CMD_SYNTAX, dto[1]);

    uint8_t bad_key[8] = {SYN_XCP_CMD_UNLOCK, 4U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, bad_key, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_ACCESS_DENIED, dto[1]);

    /* Custom seed/unlock callbacks */
    g_xcp_slave.seed_cb = test_xcp_seed_cb;
    g_xcp_slave.unlock_cb = test_xcp_unlock_cb;
    uint8_t mask = SYN_XCP_RESOURCE_CAL_PAG | SYN_XCP_RESOURCE_DAQ;
    uint8_t seed_req[8] = {SYN_XCP_CMD_GET_SEED, 0U, mask, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, seed_req, dto);
    TEST_ASSERT_EQUAL_UINT8(6U, dto[1]);

    uint8_t ok_key[8] = {SYN_XCP_CMD_UNLOCK, 2U, mask, 0x5AU, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, ok_key, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_RESOURCE_STIM | SYN_XCP_RESOURCE_PGM, dto[1]);

    /* 32-bit wire SET_MTA + DOWNLOAD + SHORT_UPLOAD + host-ptr + bounds/read-only checks */
    uint8_t mta_ram[8] = {SYN_XCP_CMD_SET_MTA, 0, 0, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_xcp_process_cto(&g_xcp_slave, mta_ram, dto);
    uint8_t dn_ram[8] = {SYN_XCP_CMD_DOWNLOAD, 4U, 0xD1U, 0xD2U, 0xD3U, 0xD4U, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD4U, ram_buf[7]);

    uint8_t sup_ram[8] = {SYN_XCP_CMD_SHORT_UPLOAD, 4U, 0, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_xcp_process_cto(&g_xcp_slave, sup_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(0xD1U, dto[1]);

    /* Host pointer inside registered region + zero-size upload/download */
    syn_xcp_set_mta(&g_xcp_slave, 0U, (uintptr_t)&ram_buf[4]);
    uint8_t up_host[8] = {SYN_XCP_CMD_UPLOAD, 2U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, up_host, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD1U, dto[1]);
    TEST_ASSERT_EQUAL_HEX8(0xD2U, dto[2]);

    syn_xcp_set_mta(&g_xcp_slave, 0U, (uintptr_t)&ram_buf[0]);
    uint8_t dn_zero[8] = {SYN_XCP_CMD_DOWNLOAD, 0U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, dn_zero, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);
    uint8_t up_zero[8] = {SYN_XCP_CMD_UPLOAD, 0U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, up_zero, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_PID_RES, dto[0]);

    /* Write-protected, out-of-range, and unmapped checks */
    uint8_t mta_ro[8] = {SYN_XCP_CMD_SET_MTA, 0, 0, 0U, 0x00U, 0x00U, 0x00U, 0x08U};
    syn_xcp_process_cto(&g_xcp_slave, mta_ro, dto);
    syn_xcp_process_cto(&g_xcp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_WRITE_PROTECTED, dto[1]);

    uint8_t mta_oob[8] = {SYN_XCP_CMD_SET_MTA, 0, 0, 0U, 0x0EU, 0x00U, 0x00U, 0x20U};
    syn_xcp_process_cto(&g_xcp_slave, mta_oob, dto);
    syn_xcp_process_cto(&g_xcp_slave, dn_ram, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_OUT_OF_RANGE, dto[1]);

    uint8_t sup_unmap[8] = {SYN_XCP_CMD_SHORT_UPLOAD, 2U, 0, 0U, 0x00U, 0x00U, 0x00U, 0x90U};
    syn_xcp_process_cto(&g_xcp_slave, sup_unmap, dto);
    TEST_ASSERT_EQUAL_HEX8(SYN_XCP_ERR_ACCESS_DENIED, dto[1]);

    /* Multi-list & multi-ODT round-robin DAQ via explicit 32-bit WRITE_DAQ addresses */
    ram_buf[0] = 0x51U;
    ram_buf[4] = 0x52U;
    rom_buf[0] = 0x53U;

    uint8_t ptr00[8] = {SYN_XCP_CMD_SET_DAQ_PTR, 0, 0U, 0U, 0U, 0U, 0, 0};
    uint8_t w00[8] = {SYN_XCP_CMD_WRITE_DAQ, 0, 1U, 0U, 0x00U, 0x00U, 0x00U, 0x20U};
    syn_xcp_process_cto(&g_xcp_slave, ptr00, dto);
    syn_xcp_process_cto(&g_xcp_slave, w00, dto);

    uint8_t ptr01[8] = {SYN_XCP_CMD_SET_DAQ_PTR, 0, 0U, 0U, 1U, 0U, 0, 0};
    uint8_t w01[8] = {SYN_XCP_CMD_WRITE_DAQ, 0, 1U, 0U, 0x04U, 0x00U, 0x00U, 0x20U};
    syn_xcp_process_cto(&g_xcp_slave, ptr01, dto);
    syn_xcp_process_cto(&g_xcp_slave, w01, dto);

    uint8_t ptr10[8] = {SYN_XCP_CMD_SET_DAQ_PTR, 0, 1U, 0U, 0U, 0U, 0, 0};
    uint8_t w10[8] = {SYN_XCP_CMD_WRITE_DAQ, 0, 1U, 0U, 0x00U, 0x00U, 0x00U, 0x08U};
    syn_xcp_process_cto(&g_xcp_slave, ptr10, dto);
    syn_xcp_process_cto(&g_xcp_slave, w10, dto);

    uint8_t mode0[8] = {SYN_XCP_CMD_SET_DAQ_LIST_MODE, 1U, 0U, 0U, 1U, 1U, 0U, 0U};
    uint8_t mode1[8] = {SYN_XCP_CMD_SET_DAQ_LIST_MODE, 1U, 1U, 0U, 1U, 1U, 0U, 0U};
    syn_xcp_process_cto(&g_xcp_slave, mode0, dto);
    syn_xcp_process_cto(&g_xcp_slave, mode1, dto);

    uint8_t start_all[8] = {SYN_XCP_CMD_START_STOP_SYNCH, 1U, 0, 0, 0, 0, 0, 0};
    syn_xcp_process_cto(&g_xcp_slave, start_all, dto);

    TEST_ASSERT_TRUE(syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(0U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(0U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0x51U, dto[1]);

    TEST_ASSERT_TRUE(syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(1U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(0U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0x53U, dto[1]);

    TEST_ASSERT_TRUE(syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx));
    TEST_ASSERT_EQUAL_UINT8(0U, l_idx);
    TEST_ASSERT_EQUAL_UINT8(1U, o_idx);
    TEST_ASSERT_EQUAL_HEX8(0x52U, dto[1]);
    /* End-of-tick drain terminates cleanly without re-stepping prescalers */
    TEST_ASSERT_FALSE(syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx));

    /* Explicit syn_xcp_daq_event + bounded while() drain across next tick */
    syn_xcp_daq_event(&g_xcp_slave, 1U);
    uint8_t drained = 0U;
    while (drained < 8U && syn_xcp_service_daq(&g_xcp_slave, 1U, dto, &l_idx, &o_idx)) {
        drained++;
    }
    TEST_ASSERT_EQUAL_UINT8(3U, drained);
}

void run_xcp_tests(void)
{
    RUN_TEST(test_xcp_init_and_connect_disconnect);
    RUN_TEST(test_xcp_mta_upload_download);
    RUN_TEST(test_xcp_daq_list_streaming);
    RUN_TEST(test_xcp_bounds_and_null_checks);
    RUN_TEST(test_r5_a11_xcp_locks_regions_and_multi_daq);
}
