/**
 * @file test_lin.c
 * @brief Unity tests for LIN bus 2.1/2.2a & ISO 17987 protocol engine (syn_lin).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_lin.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
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
    SYN_LIN_ScheduleSlot slots[] = {{.id = 0x01,
                                     .len = 4,
                                     .checksum_mode = SYN_LIN_CHECKSUM_ENHANCED,
                                     .dir = SYN_LIN_SLOT_PUBLISH,
                                     .delay_ms = 10},
                                    {.id = 0x02,
                                     .len = 2,
                                     .checksum_mode = SYN_LIN_CHECKSUM_CLASSIC,
                                     .dir = SYN_LIN_SLOT_SUBSCRIBE,
                                     .delay_ms = 20}};

    SYN_LIN_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_master_init(NULL, slots, 2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_master_init(&master, slots, 2));

    const SYN_LIN_ScheduleSlot *active = NULL;
    /* First tick immediately emits Slot 0 */
    TEST_ASSERT_TRUE(syn_lin_master_step(&master, 0, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);

    TEST_ASSERT_FALSE(syn_lin_master_step(&master, 5, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);

    /* Timer reaches 10ms Slot 0 delay; step advances to Slot 1 */
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

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x10, 4, SYN_LIN_SLOT_SUBSCRIBE,
                                                          SYN_LIN_CHECKSUM_ENHANCED));

    uint8_t pub_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x11, 4, SYN_LIN_SLOT_PUBLISH,
                                                          SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_set_publish_data(&slave, 0x11, pub_data, 4));

    /* Simulate Master frame sending Sync (0x55), PID for 0x10, 4 data bytes, and checksum */
    uint8_t pid_10 = syn_lin_calc_pid(0x10);
    uint8_t payload[] = {0x11, 0x22, 0x33, 0x44};
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
    SYN_LIN_ScheduleSlot slot = {.id = 0x01, .len = 2, .delay_ms = 10};
    SYN_LIN_Master master;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_master_init(&master, &slot, 0));

    TEST_ASSERT_FALSE(syn_lin_master_step(NULL, 10, NULL));
    master.running = false;
    TEST_ASSERT_FALSE(syn_lin_master_step(&master, 10, NULL));

    /* Null and invalid param checks for slave */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_init(NULL, 0x01));
    SYN_LIN_Slave slave;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_init(&slave, 0x01));

    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_lin_slave_add_frame(NULL, 0x01, 2, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_lin_slave_add_frame(&slave, 0x40, 2, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_lin_slave_add_frame(&slave, 0x01, 0, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_lin_slave_add_frame(&slave, 0x01, 9, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC));

    /* Fill slave frame table to max */
    for (uint8_t i = 0; i < SYN_LIN_SLAVE_MAX_FRAMES; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, i, 2, SYN_LIN_SLOT_PUBLISH,
                                                              SYN_LIN_CHECKSUM_CLASSIC));
    }
    /* Overflow table */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_lin_slave_add_frame(&slave, 0x20, 2, SYN_LIN_SLOT_PUBLISH,
                                                             SYN_LIN_CHECKSUM_CLASSIC));

    /* Update existing frame ID */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_slave_add_frame(&slave, 0x00, 4, SYN_LIN_SLOT_PUBLISH,
                                                          SYN_LIN_CHECKSUM_ENHANCED));

    /* Publish data error paths */
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(NULL, 0x00, data, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, NULL, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, data, 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x00, data, 9));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(&slave, 0x40, data, 4));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_lin_slave_set_publish_data(&slave, 0x30, data, 4)); /* Unregistered ID */

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

    /* Checksum overflow test (sum > 0xFF) */
    uint8_t large_data[4] = {0x80, 0x80, 0x80, 0x80};
    uint8_t cs = syn_lin_calc_checksum(0x00, large_data, 4, SYN_LIN_CHECKSUM_ENHANCED);
    TEST_ASSERT_NOT_EQUAL(0, cs);

    /* Master step null active slot / not running check */
    SYN_LIN_Master m;
    memset(&m, 0, sizeof(m));
    const SYN_LIN_ScheduleSlot *slot_out = (const SYN_LIN_ScheduleSlot *)0x1234;
    TEST_ASSERT_FALSE(syn_lin_master_step(&m, 10, &slot_out));
    TEST_ASSERT_NULL(slot_out);

    /* Default state fallback */
    slave.state = (SYN_LIN_State)99;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&slave, 0x00, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_IDLE, slave.state);

    /* LIN master first-tick slot 0 + subsequent zero dt_ms + NULL active_slot test */
    SYN_LIN_Master master_dt;
    SYN_LIN_ScheduleSlot dt_slot = {.id = 0x01, .len = 2, .delay_ms = 100};
    syn_lin_master_init(&master_dt, &dt_slot, 1);
    TEST_ASSERT_TRUE(syn_lin_master_step(&master_dt, 0, NULL));
    TEST_ASSERT_FALSE(syn_lin_master_step(&master_dt, 0, NULL));
    TEST_ASSERT_TRUE(syn_lin_master_step(&master_dt, 100, NULL));

    /* LIN slave rx_idx >= SYN_LIN_DATA_MAX overflow check in STATE_DATA */
    SYN_LIN_Slave ov_slave;
    syn_lin_slave_init(&ov_slave, 0x01);
    syn_lin_slave_add_frame(&ov_slave, 0x02, 8, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC);
    ov_slave.state = SYN_LIN_STATE_DATA;
    ov_slave.rx_idx = SYN_LIN_DATA_MAX;
    ov_slave.expected_len = 8;
    TEST_ASSERT_FALSE(syn_lin_slave_process_byte(&ov_slave, 0x55, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_LIN_STATE_CHECKSUM, ov_slave.state);

    /* Process byte with NULL out_frame */
    slave.state = SYN_LIN_STATE_CHECKSUM;
    slave.rx_frame.pid = syn_lin_calc_pid(0x01);
    slave.rx_frame.data[0] = 0x11;
    slave.rx_frame.len = 1;
    slave.expected_checksum_mode = SYN_LIN_CHECKSUM_CLASSIC;
    uint8_t chk =
        syn_lin_calc_checksum(slave.rx_frame.pid, slave.rx_frame.data, 1, SYN_LIN_CHECKSUM_CLASSIC);
    TEST_ASSERT_TRUE(syn_lin_slave_process_byte(&slave, chk, NULL));

    /* Attempt setting publish data on SUBSCRIBE slot */
    SYN_LIN_Slave sub_slave;
    syn_lin_slave_init(&sub_slave, 0x02);
    syn_lin_slave_add_frame(&sub_slave, 0x05, 2, SYN_LIN_SLOT_SUBSCRIBE, SYN_LIN_CHECKSUM_CLASSIC);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_lin_slave_set_publish_data(&sub_slave, 0x05, large_data, 2));
}

static void test_lin_null_checks(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_lin_slave_init(NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_lin_slave_add_frame(NULL, 0, 0, SYN_LIN_SLOT_SUBSCRIBE,
                                                                 SYN_LIN_CHECKSUM_CLASSIC));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_lin_slave_set_publish_data(NULL, 0, NULL, 0));

    /* Diagnostic frame 0x3C / 0x3D classic checksum fallback test */
    uint8_t diag_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t pid_3c = syn_lin_calc_pid(0x3C);
    uint8_t cs_enhanced = syn_lin_calc_checksum(pid_3c, diag_data, 8, SYN_LIN_CHECKSUM_ENHANCED);
    uint8_t cs_classic = syn_lin_calc_checksum(pid_3c, diag_data, 8, SYN_LIN_CHECKSUM_CLASSIC);
    TEST_ASSERT_EQUAL_UINT8(cs_classic, cs_enhanced); /* Overridden to Classic for 0x3C */
}

static void test_lin_pro23_master_slot_timer_saturation_and_sentinel(void)
{
    SYN_LIN_ScheduleSlot slots[2] = {
        {.id = 0x01U,
         .len = 2U,
         .checksum_mode = SYN_LIN_CHECKSUM_ENHANCED,
         .dir = SYN_LIN_SLOT_PUBLISH,
         .delay_ms = UINT32_MAX},
        {.id = 0x02U,
         .len = 2U,
         .checksum_mode = SYN_LIN_CHECKSUM_CLASSIC,
         .dir = SYN_LIN_SLOT_SUBSCRIBE,
         .delay_ms = 50U},
    };
    SYN_LIN_Master master;
    const SYN_LIN_ScheduleSlot *active = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lin_master_init(&master, slots, 2U));
    TEST_ASSERT_FALSE(master.started);

    /* Initial step emits Slot 0 and sets started = true */
    TEST_ASSERT_TRUE(syn_lin_master_step(&master, 0U, &active));
    TEST_ASSERT_TRUE(master.started);
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);

    /* Accumulate near UINT32_MAX, then saturate past UINT32_MAX -> advances to Slot 1 (not reset!)
     */
    TEST_ASSERT_FALSE(syn_lin_master_step(&master, UINT32_MAX - 10U, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[0], active);
    TEST_ASSERT_TRUE(syn_lin_master_step(&master, 100U, &active));
    TEST_ASSERT_EQUAL_PTR(&slots[1], active);
    TEST_ASSERT_EQUAL_UINT32(0U, master.slot_timer);
}

void run_lin_tests(void)
{
    RUN_TEST(test_lin_pid_parity);
    RUN_TEST(test_lin_checksum);
    RUN_TEST(test_lin_master_schedule);
    RUN_TEST(test_lin_slave_byte_parser);
    RUN_TEST(test_lin_edge_cases);
    RUN_TEST(test_lin_null_checks);
    RUN_TEST(test_lin_pro23_master_slot_timer_saturation_and_sentinel);
}

#endif /* SYN_USE_LIN */
