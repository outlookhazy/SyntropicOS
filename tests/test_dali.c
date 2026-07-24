/**
 * @file test_dali.c
 * @brief Unity unit tests for DALI (IEC 62386) Protocol Stack.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_dali.h"

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
    SYN_DALI_SlaveConfig cfg = {
        .short_address = 5,
        .group_mask = (1U << 2),
        .min_level = 10,
        .max_level = 254,
        .power_on_level = 254,
        .system_failure_level = 254
    };
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

    /* Step Up */
    raw_cmd = syn_dali_encode_forward((5 << 1) | 1, SYN_DALI_CMD_STEP_UP);
    syn_dali_decode_forward(raw_cmd, &req);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dali_slave_process(&slave, &req, &resp_data, &has_resp));
    TEST_ASSERT_EQUAL(1, slave.actual_level);

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

void run_dali_tests(void)
{
    RUN_TEST(test_dali_frame_codec);
    RUN_TEST(test_dali_slave_commands);
    RUN_TEST(test_dali_manchester_codec);
}
