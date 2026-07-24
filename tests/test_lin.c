/**
 * @file test_lin.c
 * @brief Unity tests for LIN bus 2.1/2.2a & ISO 17987 protocol engine (syn_lin).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_lin.h"

#if defined(SYN_USE_LIN) && SYN_USE_LIN

static void test_lin_pid_parity(void)
{
    /* Test PID parity calculation and verification for standard LIN IDs */
    uint8_t pid_0 = syn_lin_calc_pid(0x00);
    TEST_ASSERT_EQUAL_HEX8(0x80, pid_0);
    TEST_ASSERT_TRUE(syn_lin_verify_pid(pid_0));

    uint8_t pid_1 = syn_lin_calc_pid(0x01);
    TEST_ASSERT_EQUAL_HEX8(0xC1, pid_1);
    TEST_ASSERT_TRUE(syn_lin_verify_pid(pid_1));

    uint8_t pid_3c = syn_lin_calc_pid(0x3C); /* Diagnostic Master Req */
    TEST_ASSERT_TRUE(syn_lin_verify_pid(pid_3c));

    uint8_t pid_3d = syn_lin_calc_pid(0x3D); /* Diagnostic Slave Resp */
    TEST_ASSERT_TRUE(syn_lin_verify_pid(pid_3d));

    /* Verify invalid PID parity detection */
    TEST_ASSERT_FALSE(syn_lin_verify_pid(pid_0 ^ 0x40)); /* Corrupt P0 */
    TEST_ASSERT_FALSE(syn_lin_verify_pid(pid_0 ^ 0x80)); /* Corrupt P1 */

    /* Verify PID parity across all 64 valid Frame IDs */
    for (uint8_t id = 0; id <= SYN_LIN_ID_MAX; id++) {
        uint8_t p = syn_lin_calc_pid(id);
        TEST_ASSERT_EQUAL_INT(id, p & SYN_LIN_ID_MAX);
        TEST_ASSERT_TRUE(syn_lin_verify_pid(p));
    }
}

static void test_lin_checksum(void)
{
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
    uint8_t pid = syn_lin_calc_pid(0x12);

    /* Test Classic Checksum (Data only) */
    uint8_t cs_classic = syn_lin_calc_checksum(pid, data, 4, SYN_LIN_CHECKSUM_CLASSIC);
    TEST_ASSERT_EQUAL_HEX8((uint8_t)(~(1 + 2 + 3 + 4) & 0xFF), cs_classic);

    /* Test Enhanced Checksum (PID + Data) */
    uint8_t cs_enhanced = syn_lin_calc_checksum(pid, data, 4, SYN_LIN_CHECKSUM_ENHANCED);
    uint16_t expected_sum = pid + 1 + 2 + 3 + 4;
    while (expected_sum > 0xFF) {
        expected_sum -= 0xFF;
    }
    TEST_ASSERT_EQUAL_HEX8((uint8_t)(~expected_sum & 0xFF), cs_enhanced);

    /* Diagnostic frame 0x3C must always use Classic checksum regardless of mode flag */
    uint8_t pid_diag = syn_lin_calc_pid(SYN_LIN_ID_MASTER_REQ);
    uint8_t cs_diag = syn_lin_calc_checksum(pid_diag, data, 4, SYN_LIN_CHECKSUM_ENHANCED);
    TEST_ASSERT_EQUAL_HEX8(cs_classic, cs_diag);

    /* Invalid parameters check */
    TEST_ASSERT_EQUAL_HEX8(0, syn_lin_calc_checksum(pid, NULL, 4, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_HEX8(0, syn_lin_calc_checksum(pid, data, 0, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_HEX8(0, syn_lin_calc_checksum(pid, data, 9, SYN_LIN_CHECKSUM_CLASSIC));
}

static void test_lin_master_schedule(void)
{
    SYN_LIN_ScheduleSlot slots[] = {
        { .id = 0x01, .len = 4, .checksum_mode = SYN_LIN_CHECKSUM_ENHANCED, .dir = SYN_LIN_SLOT_PUBLISH, .delay_ms = 10 },
        { .id = 0x02, .len = 2, .checksum_mode = SYN_LIN_CHECKSUM_CLASSIC, .dir = SYN_LIN_SLOT_SUBSCRIBE, .delay_ms = 20 }
    };

    SYN_LIN_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_ERR_INVALID_PARAM, syn_lin_master_init(NULL, slots, 2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_master_init(&master, slots, 2));

    const SYN_LIN_ScheduleSlot *active = NULL;
    TEST_ASSERT_FALSE(syn_lin_master_step(&master, 5, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);

    /* Timer reaches 10ms slot delay; step advances to next slot */
    TEST_ASSERT_TRUE(syn_lin_master_step(&master, 5, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[1], active);

    TEST_ASSERT_FALSE(syn_lin_master_step(&master, 10, &active));
    TEST_ASSERT_TRUE(syn_lin_master_step(&master, 10, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);
}

static void test_lin_slave_byte_parser(void)
{
    SYN_LIN_Slave slave;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_init(&slave, 0x01));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x10, 4, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_ENHANCED));
    
    uint8_t pub_data[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x11, 4, SYN_LIN_SLOT_PUBLISH, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_set_publish_data(&slave, 0x11, pub_data, 4));

    /* Simulate Master frame sending Sync (0x55), PID for 0x10, 4 data bytes, and checksum */
    uint8_t pid_10 = syn_lin_calc_pid(0x10);
    uint8_t payload[] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t cs = syn_lin_calc_checksum(pid_10, payload, 4, SYN_LIN_CHECKSUM_ENHANCED);

    SYN_LIN_Frame out_frame;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, SYN_LIN_SYNC_BYTE, &out_frame));
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, pid_10, &out_frame));
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, payload[0], &out_frame));
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, payload[1], &out_frame));
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, payload[2], &out_frame));
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, payload[3], &out_frame));

    /* Final Checksum byte completes frame */
    TEST_ASSERT_TRUE(syn_lin_slave_process_byte(&slave, cs, &out_frame));

    TEST_ASSERT_EQUAL_INT(0x10, out_frame.id);
    TEST_ASSERT_EQUAL_INT(4, out_frame.len);
    TEST_ASSERT_TRUE(out_frame.valid_pid);
    TEST_ASSERT_TRUE(out_frame.valid_checksum);
    TEST_ASSERT_EQUAL_INT(0, memcmp(payload, out_frame.data, 4));
}

static void test_lin_edge_cases(void)
{
    /* Null and invalid param checks for master */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_master_init(NULL, NULL, 0));
    SYN_LIN_ScheduleSlot slot = { .id = 0x01, .len = 2, .delay_ms = 10 };
    SYN_LIN_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_master_init(&master, &slot, 0));

    TEST_ASSERT_FALSE(syn_lin_master_step(NULL, 10, NULL));
    master.running = false;
    TEST_ASSERT_FALSE(syn_lin_master_step(&master, 10, NULL));

    /* Null and invalid param checks for slave */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_init(NULL, 0x01));
    SYN_LIN_Slave slave;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_init(&slave, 0x01));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_add_frame(NULL, 0x01, 2, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_add_frame(&slave, 0x40, 2, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_add_frame(&slave, 0x01, 0, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_add_frame(&slave, 0x01, 9, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));

    /* Fill slave frame table to max */
    for (uint8_t i = 0; i < SYN_LIN_SLAVE_MAX_FRAMES; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, i, 2, SYN_LIN_SLOT_PUBLISH, SYN_LIN_CHECKSUM_CLASSIC));
    }
    /* Overflow table */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_lin_slave_add_frame(&slave, 0x20, 2, SYN_LIN_SLOT_PUBLISH, SYN_LIN_CHECKSUM_CLASSIC));

    /* Update existing frame ID */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x00, 4, SYN_LIN_SLOT_PUBLISH, SYN_LIN_CHECKSUM_ENHANCED));

    /* Publish data error paths */
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(NULL, 0x00, data, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, NULL, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, data, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, data, 9));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x40, data, 4));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_lin_slave_set_publish_data(&slave, 0x30, data, 4)); /* Unregistered ID */

    /* Publish data truncation check (len 5 > slot len 4) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_set_publish_data(&slave, 0x00, data, 5));

    /* Process byte edge cases */
    SYN_LIN_Frame frame;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(NULL, 0x00, &frame));

    /* Idle non-sync byte */
    slave.state = SYN_LIN_STATE_IDLE;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, 0x00, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_IDLE, slave.state);

    /* Sync state handling */
    slave.state = SYN_LIN_STATE_SYNC;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, 0x00, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_IDLE, slave.state);

    slave.state = SYN_LIN_STATE_SYNC;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, SYN_LIN_SYNC_BYTE, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_PID, slave.state);

    /* Invalid PID byte */
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, 0xFF, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_IDLE, slave.state);

    /* Unknown unregistered PID byte (defaults to len 8) */
    uint8_t unk_pid = syn_lin_calc_pid(0x35); /* Unregistered ID */
    slave.state = SYN_LIN_STATE_PID;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, unk_pid, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_DATA, slave.state);
    TEST_ASSERT_EQUAL_INT(8, slave.expected_len);

    /* Default state fallback */
    slave.state = (SYN_LIN_State)99;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, 0x00, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_IDLE, slave.state);
}

void run_lin_tests(void)
{
    RUN_TEST(test_lin_pid_parity);
    RUN_TEST(test_lin_checksum);
    RUN_TEST(test_lin_master_schedule);
    RUN_TEST(test_lin_slave_byte_parser);
    RUN_TEST(test_lin_edge_cases);
}

#endif /* SYN_USE_LIN */
