/**
 * @file test_dali.c
 * @brief Unity unit tests for DALI (IEC 62386) Protocol Stack.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_dali.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static void test_dali_frame_codec(void)
{
    /* 1. Forward Frame Direct Arc Power Level Control to Short Address 5 */
    uint8_t short_addr_5 = (5 << 1) | 0; /* Direct Arc Level */
    uint16_t enc_forward = syn_dali_encode_forward(short_addr_5, 200);
    SYN_DALI_ForwardFrame ff;
    TEST_ASSERT_TRUE(syn_dali_decode_forward(enc_forward, &ff));
    TEST_ASSERT_EQUAL(SYN_DALI_ADDR_SHORT, ff.addr_type);
    TEST_ASSERT_EQUAL(5, ff.address);
    TEST_ASSERT_TRUE(ff.is_direct);
    TEST_ASSERT_EQUAL(200, ff.data_cmd);

    /* 2. Forward Frame Command to Group Address 2 */
    uint8_t group_addr_2 = (0x80 | (2 << 1)) | 1; /* Command selector */
    enc_forward = syn_dali_encode_forward(group_addr_2, SYN_DALI_CMD_RECALL_MAX);
    TEST_ASSERT_TRUE(syn_dali_decode_forward(enc_forward, &ff));
    TEST_ASSERT_EQUAL(SYN_DALI_ADDR_GROUP, ff.addr_type);
    TEST_ASSERT_EQUAL(2, ff.address);
    TEST_ASSERT_FALSE(ff.is_direct);
    TEST_ASSERT_EQUAL(SYN_DALI_CMD_RECALL_MAX, ff.data_cmd);

    /* 3. Broadcast Command */
    uint8_t bcast_addr = 0xFF;
    enc_forward = syn_dali_encode_forward(bcast_addr, SYN_DALI_CMD_OFF);
    TEST_ASSERT_TRUE(syn_dali_decode_forward(enc_forward, &ff));
    TEST_ASSERT_EQUAL(SYN_DALI_ADDR_BROADCAST, ff.addr_type);
    TEST_ASSERT_FALSE(ff.is_direct);
    TEST_ASSERT_EQUAL(SYN_DALI_CMD_OFF, ff.data_cmd);

    /* 4. Backward Frame Codec */
    uint8_t raw_b = syn_dali_encode_backward(0xA5);
    SYN_DALI_BackwardFrame bf;
    TEST_ASSERT_TRUE(syn_dali_decode_backward(raw_b, &bf));
    TEST_ASSERT_EQUAL(0xA5, bf.data);
}

static void test_dali_slave_commands(void)
{
    SYN_DALI_SlaveState slave;
    SYN_DALI_SlaveConfig cfg = {.short_address = 5,
                                .group_mask = (1U << 2),
                                .min_level = 10,
                                .max_level = 254,
                                .power_on_level = 254,
                                .system_failure_level = 254};
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_init(&slave, &cfg));
    TEST_ASSERT_EQUAL(254, slave.actual_level);
    TEST_ASSERT_TRUE(slave.lamp_on);

    uint8_t resp_data = 0;
    bool has_resp = false;

    /* Direct Arc Level to Short Address 5 */
    uint16_t raw_cmd = syn_dali_encode_forward((5 << 1) | 0, 150);
    SYN_DALI_ForwardFrame req;
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(150, slave.actual_level);
    TEST_ASSERT_FALSE(has_resp);

    /* Direct Arc Level below MIN level -> clamped to min_level (10) */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 0, 5);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(10, slave.actual_level);

    /* Off command */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_OFF);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(0, slave.actual_level);
    TEST_ASSERT_FALSE(slave.lamp_on);

    /* Step Up while lamp OFF does NOT turn on per IEC 62386-102 */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_STEP_UP);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(0, slave.actual_level);
    TEST_ASSERT_FALSE(slave.lamp_on);

    /* Recall Max */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_RECALL_MAX);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(254, slave.actual_level);

    /* Query Actual Level */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_QUERY_ACTUAL_LEVEL);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL(254, resp_data);

    /* DTR0 store & query */
    raw_cmd = syn_dali_encode_forward(SYN_DALI_SPEC_DTR0, 180);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(180, slave.dtr0);

    /* Store DTR as MAX level */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(180, slave.cfg.max_level);

    /* Group Add & Remove */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_ADD_TO_GROUP_BASE + 3);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_TRUE((slave.cfg.group_mask & (1U << 3)) != 0);

    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE + 3);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_FALSE((slave.cfg.group_mask & (1U << 3)) != 0);

    /* Scene Store & Recall */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE + 1);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(180, slave.scenes[1]);

    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_GO_TO_SCENE_BASE + 1);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(180, slave.actual_level);

    /* Command 9: Enable DAPC Sequence */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_ENABLE_DAPC_SEQUENCE);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_TRUE(slave.dapc_sequence_active);

    /* Command 7: Step Down and Off */
    slave.actual_level = slave.cfg.min_level;
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_STEP_DOWN_AND_OFF);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(0, slave.actual_level);
    TEST_ASSERT_FALSE(slave.lamp_on);

    /* Command 8: On and Step Up */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_ON_AND_STEP_UP);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(slave.cfg.min_level, slave.actual_level);
    TEST_ASSERT_TRUE(slave.lamp_on);
}

static void test_dali_manchester_codec(void)
{
    uint8_t raw_byte = 0xD5;
    uint8_t bit_out[16];
    TEST_ASSERT_EQUAL(16, syn_dali_manchester_encode_byte(raw_byte, bit_out));

    uint8_t decoded = 0;
    TEST_ASSERT_TRUE(syn_dali_manchester_decode_byte(bit_out, &decoded));
    TEST_ASSERT_EQUAL(0xD5, decoded);

    /* Invalid transition check */
    bit_out[0] = 0;
    bit_out[1] = 0; /* Invalid Manchester pair */
    TEST_ASSERT_FALSE(syn_dali_manchester_decode_byte(bit_out, &decoded));
}

static void test_dali_forward_frame_settling_and_timing(void)
{
    SYN_DALI_SlaveState slave;
    SYN_DALI_SlaveConfig cfg = {.short_address = 10,
                                .group_mask = 0x0505,
                                .min_level = 10,
                                .max_level = 250,
                                .power_on_level = 200,
                                .system_failure_level = 150};
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_init(&slave, &cfg));

    uint8_t resp_data = 0;
    bool has_resp = false;
    SYN_DALI_ForwardFrame req;

    /* Up / Down / Step Down commands */
    slave.actual_level = 100;
    syn_dali_decode_forward(syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_UP), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(101, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_DOWN), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(100, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STEP_DOWN), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(99, slave.actual_level);

    /* Recall Min & Reset */
    syn_dali_decode_forward(syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_RECALL_MIN), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(10, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_RESET), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(254, slave.actual_level);
    slave.cfg.group_mask = 0x0505U;

    /* Store DTR registers */
    slave.dtr0 = 120;
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(254, slave.dtr0);

    slave.dtr0 = 15;
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(15, slave.cfg.min_level);

    slave.dtr0 = 160;
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(160, slave.cfg.system_failure_level);

    slave.dtr0 = 180;
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(180, slave.cfg.power_on_level);

    slave.dtr0 = 5;
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(5, slave.cfg.fade_time);

    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(5, slave.cfg.fade_rate);

    slave.dtr0 = (20 << 1);
    syn_dali_decode_forward(
        syn_dali_encode_forward((10 << 1) | 1, SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(20, slave.cfg.short_address);

    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_ENABLE_WRITE_MEMORY), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(slave.write_mem_enabled);

    /* Queries */
    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_STATUS),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_CONTENT_DTR),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.dtr0, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_CONTENT_DTR1),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.dtr1, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_CONTENT_DTR2),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.dtr2, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_GROUPS_0_7),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0x05, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_GROUPS_8_15),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0x05, resp_data);

    slave.random_address = 0x123456;
    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_RANDOM_ADDR_H), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0x12, resp_data);

    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_RANDOM_ADDR_M), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0x34, resp_data);

    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_RANDOM_ADDR_L), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0x56, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_MAX_LEVEL),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.cfg.max_level, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_MIN_LEVEL),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.cfg.min_level, resp_data);

    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_POWER_ON_LEVEL), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.cfg.power_on_level, resp_data);

    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.cfg.system_failure_level, resp_data);

    /* Scene Remove */
    syn_dali_decode_forward(
        syn_dali_encode_forward((20 << 1) | 1, SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE + 1), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(SYN_DALI_MASK_LEVEL, slave.scenes[1]);

    /* Special commands */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_DTR1, 0xAA), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0xAA, slave.dtr1);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_DTR2, 0xBB), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0xBB, slave.dtr2);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_INITIALISE, 0x00), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(slave.initialise_active);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_RANDOMISE, 0xAB), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0xAB1234, slave.random_address);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_SEARCHADDRH, 0xAB), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_SEARCHADDRM, 0x12), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_SEARCHADDRL, 0x34), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0xAB1234, slave.search_address);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_COMPARE, 0x00), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL(0xFF, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_PROGRAM_SHORT_ADDR, (25 << 1)),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(25, slave.cfg.short_address);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_VERIFY_SHORT_ADDR, (25 << 1)),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_QUERY_SHORT_ADDR, 0x00), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL((25 << 1) | 1, resp_data);

    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_TERMINATE, 0x00), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_FALSE(slave.initialise_active);

    /* Additional edge-case coverage */
    SYN_DALI_SlaveState mask_slave;
    SYN_DALI_SlaveConfig mask_cfg = {.power_on_level = SYN_DALI_MASK_LEVEL, .max_level = 240};
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_init(&mask_slave, &mask_cfg));
    TEST_ASSERT_EQUAL(240, mask_slave.actual_level);

    /* Direct Arc Level set to 0 */
    syn_dali_decode_forward(syn_dali_encode_forward((25 << 1) | 0, 0), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0, slave.actual_level);
    TEST_ASSERT_FALSE(slave.lamp_on);

    /* Broadcast & Group command matching */
    syn_dali_decode_forward(syn_dali_encode_forward(0xFF, SYN_DALI_CMD_RECALL_MAX), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(slave.cfg.max_level, slave.actual_level);

    uint8_t group0_addr = (0x80 | (0 << 1)) | 1;
    syn_dali_decode_forward(syn_dali_encode_forward(group0_addr, SYN_DALI_CMD_OFF), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0, slave.actual_level);

    /* Step Down below min_level */
    slave.actual_level = slave.cfg.min_level;
    syn_dali_decode_forward(syn_dali_encode_forward((25 << 1) | 1, SYN_DALI_CMD_STEP_DOWN), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(0, slave.actual_level);
    TEST_ASSERT_FALSE(slave.lamp_on);

    /* Step Down and Off above min_level */
    slave.actual_level = 100;
    syn_dali_decode_forward(syn_dali_encode_forward((25 << 1) | 1, SYN_DALI_CMD_STEP_DOWN_AND_OFF),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(99, slave.actual_level);

    /* On and Step Up when already ON */
    slave.lamp_on = true;
    slave.actual_level = 100;
    syn_dali_decode_forward(syn_dali_encode_forward((25 << 1) | 1, SYN_DALI_CMD_ON_AND_STEP_UP),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(101, slave.actual_level);

    /* Verify short address mismatch path */
    has_resp = false;
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_VERIFY_SHORT_ADDR, (40 << 1)),
                            &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_FALSE(has_resp);

    /* Query short address when unassigned */
    SYN_DALI_SlaveState unassigned_slave;
    SYN_DALI_SlaveConfig unassigned_cfg = {.short_address = SYN_DALI_SHORT_ADDR_UNASSIGNED};
    syn_dali_slave_init(&unassigned_slave, &unassigned_cfg);
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_QUERY_SHORT_ADDR, 0x00), &req);
    syn_dali_slave_process(&unassigned_slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL(0xFF, resp_data);

    /* Unknown special command fallback */
    syn_dali_decode_forward(syn_dali_encode_forward(0xFD, 0x00), &req);
    syn_dali_slave_process(&slave, &req, &resp_data, &has_resp);

    /* Invalid parameters checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dali_slave_init(NULL, &cfg));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dali_slave_process(NULL, &req, &resp_data, &has_resp));
    TEST_ASSERT_FALSE(syn_dali_decode_forward(0, NULL));
    TEST_ASSERT_FALSE(syn_dali_decode_backward(0, NULL));
    TEST_ASSERT_EQUAL(0, syn_dali_manchester_encode_byte(0, NULL));
    TEST_ASSERT_FALSE(syn_dali_manchester_decode_byte(NULL, &resp_data));

    /* Direct arc power control level > max_level clamp (line 143) */
    SYN_DALI_SlaveState clamp_slave;
    SYN_DALI_SlaveConfig clamp_cfg = {.short_address = 25, .max_level = 200};
    syn_dali_slave_init(&clamp_slave, &clamp_cfg);
    syn_dali_decode_forward(syn_dali_encode_forward(0xFE, 250), &req);
    syn_dali_slave_process(&clamp_slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_EQUAL(200, clamp_slave.actual_level);

    /* QUERY STATUS with control gear failure, lamp failure, unassigned address (lines 263, 265,
     * 269) */
    unassigned_slave.control_gear_failure = true;
    unassigned_slave.lamp_failure = true;
    unassigned_slave.lamp_on = true;
    syn_dali_decode_forward(syn_dali_encode_forward(0xFF, SYN_DALI_CMD_QUERY_STATUS), &req);
    syn_dali_slave_process(&unassigned_slave, &req, &resp_data, &has_resp);
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_HEX8((1U << 0) | (1U << 1) | (1U << 2) | (1U << 6), resp_data);
}

static void test_r5_dali_iec62386_reset_and_up_commands(void)
{
    SYN_DALI_SlaveConfig cfg = {
        .short_address = 3U,
        .power_on_level = 120U,
        .system_failure_level = 80U,
        .min_level = 10U,
        .max_level = 200U,
        .fade_time = 4U,
        .fade_rate = 3U,
        .group_mask = 0x1234U,
    };
    SYN_DALI_SlaveState slave;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_init(&slave, &cfg));
    slave.scenes[0] = 99U;
    slave.dapc_sequence_active = true;
    slave.write_mem_enabled = true;

    SYN_DALI_ForwardFrame req;
    uint8_t resp = 0U;
    bool has_resp = false;

    /* 1. Turn lamp OFF: UP and STEP_UP must NOT turn lamp on */
    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_OFF), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(slave.lamp_on);
    TEST_ASSERT_EQUAL_UINT8(0U, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_UP), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(slave.lamp_on);
    TEST_ASSERT_EQUAL_UINT8(0U, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_STEP_UP), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(slave.lamp_on);
    TEST_ASSERT_EQUAL_UINT8(0U, slave.actual_level);

    /* ON_AND_STEP_UP turns lamp ON at min_level (10), then UP/STEP_UP increment */
    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_ON_AND_STEP_UP),
                            &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(slave.lamp_on);
    TEST_ASSERT_EQUAL_UINT8(10U, slave.actual_level);

    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_STEP_UP), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(11U, slave.actual_level);

    /* 2. IEC 62386-102 Table 11 RESET restores factory operating parameters (preserving short_addr)
     */
    syn_dali_decode_forward(syn_dali_encode_forward((3U << 1) | 1U, SYN_DALI_CMD_RESET), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(254U, slave.actual_level);
    TEST_ASSERT_TRUE(slave.lamp_on);
    TEST_ASSERT_EQUAL_UINT8(254U, slave.cfg.power_on_level);
    TEST_ASSERT_EQUAL_UINT8(254U, slave.cfg.system_failure_level);
    TEST_ASSERT_EQUAL_UINT8(1U, slave.cfg.min_level);
    TEST_ASSERT_EQUAL_UINT8(254U, slave.cfg.max_level);
    TEST_ASSERT_EQUAL_UINT8(0U, slave.cfg.fade_time);
    TEST_ASSERT_EQUAL_UINT8(7U, slave.cfg.fade_rate);
    TEST_ASSERT_EQUAL_UINT16(0U, slave.cfg.group_mask);
    TEST_ASSERT_EQUAL_UINT8(SYN_DALI_MASK_LEVEL, slave.scenes[0]);
    TEST_ASSERT_EQUAL_UINT8(3U, slave.cfg.short_address);
    TEST_ASSERT_FALSE(slave.dapc_sequence_active);
    TEST_ASSERT_FALSE(slave.write_mem_enabled);

    /* Direct Arc Power with MASK_LEVEL (0xFF) leaves actual_level and lamp_on unchanged */
    syn_dali_decode_forward(syn_dali_encode_forward(3U << 1, SYN_DALI_MASK_LEVEL), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(254U, slave.actual_level);
    TEST_ASSERT_TRUE(slave.lamp_on);
}

static void test_tier_a_dali_queries_withdraw_unassign_and_clamps(void)
{
    SYN_DALI_SlaveConfig cfg = {
        .short_address = 63U,
        .min_level = 20U,
        .max_level = 200U,
        .power_on_level = 100U,
        .system_failure_level = 100U,
    };
    SYN_DALI_SlaveState slave;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_init(&slave, &cfg));

    SYN_DALI_ForwardFrame req;
    uint8_t resp = 0U;
    bool has_resp = false;

    /* 1. Missing queries: CONTROL_GEAR, LAMP_FAILURE, LAMP_POWER_ON, DEVICE_TYPE, PHYSICAL_MIN */
    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_QUERY_CONTROL_GEAR), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, resp);

    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_QUERY_LAMP_FAILURE), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(has_resp);
    slave.lamp_failure = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, resp);

    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_QUERY_LAMP_POWER_ON), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, resp);
    slave.lamp_on = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(has_resp);

    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_QUERY_DEVICE_TYPE), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_UINT8(0U, resp);

    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_QUERY_PHYSICAL_MIN_LEVEL),
                            &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(has_resp);
    TEST_ASSERT_EQUAL_UINT8(1U, resp);

    /* 2. Clamp STORE_DTR_AS_MIN_LEVEL to [1, max_level] and MAX_LEVEL to [min_level, 254] */
    slave.dtr0 = 0U;
    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL),
                            &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(1U, slave.cfg.min_level);

    slave.dtr0 = 250U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(200U, slave.cfg.min_level);

    slave.cfg.min_level = 50U;
    slave.dtr0 = 10U;
    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL),
                            &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(50U, slave.cfg.max_level);

    slave.dtr0 = 255U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_UINT8(254U, slave.cfg.max_level);

    /* 3. Short address 0xFF unassignment in STORE_DTR_AS_SHORT_ADDR & PROGRAM_SHORT_ADDR */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_VERIFY_SHORT_ADDR, 0xFFU), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(has_resp); /* 0xFF must not verify against short address 63! */

    slave.dtr0 = 0xFFU;
    syn_dali_decode_forward(syn_dali_encode_forward(0xFFU, SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR),
                            &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_HEX8(SYN_DALI_SHORT_ADDR_UNASSIGNED, slave.cfg.short_address);

    /* 4. WITHDRAW during binary search & COMPARE suppression, cleared by INITIALISE/TERMINATE */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_INITIALISE, 0x00U), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    slave.random_address = 0x112233U;
    slave.search_address = 0x112234U;

    /* WITHDRAW ignored when random_address != search_address */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_WITHDRAW, 0x00U), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(slave.withdrawn);

    slave.search_address = 0x112233U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_TRUE(slave.withdrawn);

    /* COMPARE must NOT reply once withdrawn */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_COMPARE, 0x00U), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(has_resp);

    /* PROGRAM_SHORT_ADDR with 0xFF deletes short address */
    slave.cfg.short_address = 12U;
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_PROGRAM_SHORT_ADDR, 0xFFU), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_EQUAL_HEX8(SYN_DALI_SHORT_ADDR_UNASSIGNED, slave.cfg.short_address);

    /* TERMINATE clears withdrawn */
    syn_dali_decode_forward(syn_dali_encode_forward(SYN_DALI_SPEC_TERMINATE, 0x00U), &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp, &has_resp));
    TEST_ASSERT_FALSE(slave.withdrawn);
}

void run_dali_tests(void)
{
    RUN_TEST(test_dali_frame_codec);
    RUN_TEST(test_dali_slave_commands);
    RUN_TEST(test_dali_manchester_codec);
    RUN_TEST(test_dali_forward_frame_settling_and_timing);
    RUN_TEST(test_r5_dali_iec62386_reset_and_up_commands);
    RUN_TEST(test_tier_a_dali_queries_withdraw_unassign_and_clamps);
}
