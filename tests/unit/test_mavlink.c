/**
 * @file test_mavlink.c
 * @brief Unit tests for Zero-Heap MAVLink v2 Protocol Parser & Encoder.
 */

#include "syntropic/proto/syn_mavlink.h"
#include "unity/unity.h"

void test_mavlink_init(void)
{
    SYN_MAVLINK_Parser parser;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_init(&parser));
    TEST_ASSERT_EQUAL_UINT32(0, parser.packets_received);
}

void test_mavlink_encode_and_parse_attitude(void)
{
    /* Construct MAVLink v2 Attitude frame payload (28 bytes) */
    uint8_t payload[28] = {0};
    uint8_t buf[64];
    size_t out_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_encode_msg(1, 1, 42, SYN_MAVLINK_MSG_ATTITUDE, 39,
                                                         payload, 28, buf, &out_len));
    TEST_ASSERT_EQUAL_INT(40, out_len); /* 10 header + 28 payload + 2 CRC */

    SYN_MAVLINK_Parser parser;
    syn_mavlink_init(&parser);

    SYN_MAVLINK_Frame frame;
    SYN_Status status = SYN_BUSY;

    for (size_t i = 0; i < out_len - 1; i++) {
        status = syn_mavlink_parse_byte(&parser, buf[i], &frame);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, status);
    }

    status = syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_UINT8(1, frame.sys_id);
    TEST_ASSERT_EQUAL_UINT8(1, frame.comp_id);
    TEST_ASSERT_EQUAL_UINT8(42, frame.seq);
    TEST_ASSERT_EQUAL_UINT32(SYN_MAVLINK_MSG_ATTITUDE, frame.msg_id);
    TEST_ASSERT_EQUAL_UINT8(28, frame.payload_len);
}

void test_mavlink_null_and_crc_error(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mavlink_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mavlink_parse_byte(NULL, 0xFD, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mavlink_encode_msg(1, 1, 0, 0, 0, NULL, 0, NULL, NULL));

    SYN_MAVLINK_Parser parser;
    syn_mavlink_init(&parser);

    /* Corrupt packet (12 bytes: 10 header + 2 bad CRC bytes) */
    uint8_t buf[12] = {0xFD, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0xFF, 0xFF};
    SYN_Status status = SYN_BUSY;
    for (int i = 0; i < 11; i++) {
        status = syn_mavlink_parse_byte(&parser, buf[i], NULL);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, status);
    }
    status = syn_mavlink_parse_byte(&parser, buf[11], NULL);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, status);
    TEST_ASSERT_EQUAL_UINT32(1, parser.crc_errors);
}

void test_mavlink_msg_ids_and_invalid_state_fallback(void)
{
    /* Test encode_msg with sys_status, global_pos_int, vfr_hud, heartbeat, and unknown msg_id */
    uint8_t payload[16] = {0};
    uint8_t buf[32];
    size_t out_len = 0;
    SYN_MAVLINK_Parser parser;
    SYN_MAVLINK_Frame frame;

    /* 1. Heartbeat (msg_id 0, crc_extra 50) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_encode_msg(1, 1, 1, SYN_MAVLINK_MSG_HEARTBEAT, 50,
                                                         payload, 9, buf, &out_len));
    syn_mavlink_init(&parser);
    for (size_t i = 0; i < out_len - 1; i++) {
        syn_mavlink_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame));
    TEST_ASSERT_EQUAL_UINT32(SYN_MAVLINK_MSG_HEARTBEAT, frame.msg_id);

    /* 2. Sys Status (msg_id 1, crc_extra 124) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_encode_msg(1, 1, 2, SYN_MAVLINK_MSG_SYS_STATUS, 124,
                                                         payload, 8, buf, &out_len));
    syn_mavlink_init(&parser);
    for (size_t i = 0; i < out_len - 1; i++) {
        syn_mavlink_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame));

    /* 3. Global Position Int (msg_id 33, crc_extra 96) */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_mavlink_encode_msg(1, 1, 3, SYN_MAVLINK_MSG_GLOBAL_POSITION_INT, 96,
                                                 payload, 8, buf, &out_len));
    syn_mavlink_init(&parser);
    for (size_t i = 0; i < out_len - 1; i++) {
        syn_mavlink_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame));

    /* 4. VFR HUD (msg_id 74, crc_extra 20) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_encode_msg(1, 1, 4, SYN_MAVLINK_MSG_VFR_HUD, 20,
                                                         payload, 8, buf, &out_len));
    syn_mavlink_init(&parser);
    for (size_t i = 0; i < out_len - 1; i++) {
        syn_mavlink_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame));

    /* 5. Zero-payload frame */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_encode_msg(1, 1, 5, 999, 0, NULL, 0, buf, &out_len));
    syn_mavlink_init(&parser);
    for (size_t i = 0; i < out_len - 1; i++) {
        syn_mavlink_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_parse_byte(&parser, buf[out_len - 1], &frame));
    TEST_ASSERT_EQUAL_UINT8(0, frame.payload_len);

    /* Test parser fallback when state is invalid */
    syn_mavlink_init(&parser);
    parser.state = 255; /* Out of range state */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mavlink_parse_byte(&parser, 0, NULL));
    TEST_ASSERT_EQUAL_INT(0, parser.state); /* Resets to STATE_STX */
}

static uint8_t s_pro08_payload[SYN_MAVLINK_MAX_PAYLOAD_LEN];
static uint8_t s_pro08_buf[280];
static SYN_MAVLINK_Parser s_pro08_parser;
static SYN_MAVLINK_Frame s_pro08_frame;

void test_pro08_mavlink_large_payload_and_bounds(void)
{
    size_t out_len = 0U;

    /* 1. Reject NULL payload with non-zero length, NULL out_len, and 24-bit msg_id overflow */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mavlink_encode_msg(1U, 1U, 0U, 0U, 0U, NULL, 10U,
                                                                    s_pro08_buf, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mavlink_encode_msg(1U, 1U, 0U, 0U, 0U, NULL, 0U, s_pro08_buf, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mavlink_encode_msg(1U, 1U, 0U, 0x01000000U, 0U, s_pro08_payload, 4U,
                                                 s_pro08_buf, &out_len));

    /* 2. Max 24-bit msg_id (0xFFFFFF) and max payload_len (255 >= 244) must not wrap uint8_t idx */
    for (size_t i = 0U; i < SYN_MAVLINK_MAX_PAYLOAD_LEN; i++) {
        s_pro08_payload[i] = (uint8_t)(i ^ 0x5AU);
    }

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_mavlink_encode_msg(7U, 9U, 200U, 0xFFFFFFU, 0U, s_pro08_payload, 255U,
                                                 s_pro08_buf, &out_len));
    TEST_ASSERT_EQUAL_UINT(267U, out_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_MAVLINK_STX_V2, s_pro08_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(255U, s_pro08_buf[1]);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mavlink_init(&s_pro08_parser));
    for (size_t i = 0U; i < (out_len - 1U); i++) {
        TEST_ASSERT_EQUAL_INT(
            SYN_BUSY, syn_mavlink_parse_byte(&s_pro08_parser, s_pro08_buf[i], &s_pro08_frame));
    }
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_mavlink_parse_byte(&s_pro08_parser, s_pro08_buf[out_len - 1U], &s_pro08_frame));
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFU, s_pro08_frame.msg_id);
    TEST_ASSERT_EQUAL_UINT8(255U, s_pro08_frame.payload_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(s_pro08_payload, s_pro08_frame.payload, 255U);
}
