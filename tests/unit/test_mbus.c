/**
 * @file test_mbus.c
 * @brief Unity tests for syn_mbus — full coverage.
 */

#include "syntropic/proto/syn_mbus.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static int callback_count = 0;
static SYN_MBUS_Frame last_cb_frame;

static void mbus_on_frame(const SYN_MBUS_Frame *frame, void *ctx)
{
    (void)ctx;
    callback_count++;
    if (frame != NULL) {
        memcpy(&last_cb_frame, frame, sizeof(last_cb_frame));
    }
}

static void reset_test_state(void)
{
    callback_count = 0;
    memset(&last_cb_frame, 0, sizeof(last_cb_frame));
}

/* ── Test: Checksum Calculation ─────────────────────────────────────────── */

static void test_mbus_calc_checksum(void)
{
    uint8_t data1[] = {0x40, 0x01}; /* C=0x40, A=0x01 */
    TEST_ASSERT_EQUAL_HEX8(0x41, syn_mbus_calc_checksum(data1, sizeof(data1)));

    uint8_t data2[] = {0x53, 0x01, 0x51, 0x10, 0x20};
    TEST_ASSERT_EQUAL_HEX8(0xD5, syn_mbus_calc_checksum(data2, sizeof(data2)));

    /* Zero length & NULL data */
    TEST_ASSERT_EQUAL_HEX8(0x00, syn_mbus_calc_checksum(NULL, 0));
    TEST_ASSERT_EQUAL_HEX8(0x00, syn_mbus_calc_checksum(NULL, 10));
}

/* ── Test: Single Character ACK ─────────────────────────────────────────── */

static void test_mbus_single_ack(void)
{
    uint8_t buf[16];
    size_t out_len = 0;

    /* Buffer overflow check */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_encode_ack(buf, 0, &out_len));

    /* Encode ACK */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_encode_ack(buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_UINT(1, out_len);
    TEST_ASSERT_EQUAL_HEX8(0xE5, buf[0]);

    /* Decode ACK */
    SYN_MBUS_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_decode_frame(buf, out_len, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_SINGLE_ACK, frame.type);
    TEST_ASSERT_TRUE(frame.checksum_valid);
}

/* ── Test: Short Frame ─────────────────────────────────────────────────── */

static void test_mbus_short_frame(void)
{
    uint8_t buf[16];
    size_t out_len = 0;

    /* Encode SND_NKE to Address 0x01 */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_mbus_encode_short(SYN_MBUS_C_SND_NKE, 0x01, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_UINT(5, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x10, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x40, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x41, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x16, buf[4]);

    /* Decode Short Frame */
    SYN_MBUS_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_decode_frame(buf, out_len, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_SHORT, frame.type);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_C_SND_NKE, frame.c_field);
    TEST_ASSERT_EQUAL_HEX8(0x01, frame.a_field);
    TEST_ASSERT_TRUE(frame.checksum_valid);

    /* Corrupt Checksum */
    buf[3] = 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(buf, out_len, &frame));

    /* Corrupt Stop Delimiter */
    buf[3] = 0x41;
    buf[4] = 0x00;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(buf, out_len, &frame));
}

/* ── Test: Control & Long Frame ─────────────────────────────────────────── */

static void test_mbus_control_and_long_frame(void)
{
    uint8_t buf[64];
    size_t out_len = 0;

    /* 1. Control frame (no payload) */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_mbus_encode_control(SYN_MBUS_C_SND_UD, 0x02, 0x51, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_UINT(9, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[1]); /* L field = C(1)+A(1)+CI(1) = 3 */
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_C_SND_UD, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[5]);
    TEST_ASSERT_EQUAL_HEX8(0x51, buf[6]);

    SYN_MBUS_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_decode_frame(buf, out_len, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_CONTROL, frame.type);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_C_SND_UD, frame.c_field);
    TEST_ASSERT_EQUAL_HEX8(0x02, frame.a_field);
    TEST_ASSERT_EQUAL_HEX8(0x51, frame.ci_field);
    TEST_ASSERT_EQUAL_UINT(0, frame.payload_len);
    TEST_ASSERT_TRUE(frame.checksum_valid);

    /* 2. Long frame (with payload) */
    uint8_t payload[] = {0x04, 0x13, 0x78, 0x56, 0x34, 0x12}; /* 6 bytes payload */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_mbus_encode_long(SYN_MBUS_C_RSP_UD, 0x05, SYN_MBUS_CI_RSP_DATA_LSB, payload,
                                     sizeof(payload), buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_UINT(15, out_len);  /* 9 + 6 = 15 */
    TEST_ASSERT_EQUAL_HEX8(0x09, buf[1]); /* L field = 3 + 6 = 9 */

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_decode_frame(buf, out_len, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_LONG, frame.type);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_C_RSP_UD, frame.c_field);
    TEST_ASSERT_EQUAL_HEX8(0x05, frame.a_field);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_CI_RSP_DATA_LSB, frame.ci_field);
    TEST_ASSERT_EQUAL_UINT(6, frame.payload_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, frame.payload, 6);
    TEST_ASSERT_TRUE(frame.checksum_valid);
}

/* ── Test: Streaming Decoder ────────────────────────────────────────────── */

static void test_mbus_streaming_decoder(void)
{
    reset_test_state();

    SYN_MBUS_Decoder dec;
    syn_mbus_decoder_init(&dec, mbus_on_frame, NULL);

    /* 1. Feed Single ACK */
    syn_mbus_decoder_feed(&dec, SYN_MBUS_ACK_BYTE);
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_SINGLE_ACK, last_cb_frame.type);

    /* 2. Feed Short Frame byte-by-byte */
    reset_test_state();
    uint8_t short_buf[16];
    size_t short_len = 0;
    syn_mbus_encode_short(SYN_MBUS_C_REQ_UD2, 0x0A, short_buf, sizeof(short_buf), &short_len);

    for (size_t i = 0; i < short_len; i++) {
        syn_mbus_decoder_feed(&dec, short_buf[i]);
    }
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_SHORT, last_cb_frame.type);
    TEST_ASSERT_EQUAL_HEX8(SYN_MBUS_C_REQ_UD2, last_cb_frame.c_field);
    TEST_ASSERT_EQUAL_HEX8(0x0A, last_cb_frame.a_field);

    /* 3. Feed Long Frame byte-by-byte */
    reset_test_state();
    uint8_t long_buf[64];
    size_t long_len = 0;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    syn_mbus_encode_long(SYN_MBUS_C_RSP_UD, 0x1F, 0x72, payload, sizeof(payload), long_buf,
                         sizeof(long_buf), &long_len);

    for (size_t i = 0; i < long_len; i++) {
        syn_mbus_decoder_feed(&dec, long_buf[i]);
    }
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_INT(SYN_MBUS_FRAME_TYPE_LONG, last_cb_frame.type);
    TEST_ASSERT_EQUAL_HEX8(0x1F, last_cb_frame.a_field);
    TEST_ASSERT_EQUAL_UINT(3, last_cb_frame.payload_len);

    /* 4. Mismatched length bytes in stream -> should reset cleanly */
    reset_test_state();
    syn_mbus_decoder_feed(&dec, 0x68);
    syn_mbus_decoder_feed(&dec, 0x05);
    syn_mbus_decoder_feed(&dec, 0x06); /* L1 != L2 */
    TEST_ASSERT_EQUAL_INT(0, callback_count);

    /* 5. Decoder with NULL callback */
    reset_test_state();
    SYN_MBUS_Decoder dec_null_cb;
    syn_mbus_decoder_init(&dec_null_cb, NULL, NULL);
    syn_mbus_decoder_feed(&dec_null_cb, SYN_MBUS_ACK_BYTE);
    TEST_ASSERT_EQUAL_INT(0, callback_count);
}

/* ── Test: Null and Invalid Parameters ──────────────────────────────────── */

static void test_mbus_null_and_invalid_params(void)
{
    uint8_t buf[16];
    size_t out_len = 0;
    SYN_MBUS_Frame frame;

    /* Encoders NULL checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_encode_ack(NULL, 10, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_encode_ack(buf, 10, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_encode_short(0x40, 0x01, NULL, 10, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_encode_short(0x40, 0x01, buf, 10, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_mbus_encode_short(0x40, 0x01, buf, 4, &out_len)); /* cap < 5 */

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mbus_encode_long(0x40, 0x01, 0x51, NULL, 5, buf, 10,
                                               &out_len)); /* payload NULL, len > 0 */
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_mbus_encode_long(0x40, 0x01, 0x51, buf, 255, buf, 10, &out_len)); /* payload > 252 */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_mbus_encode_long(0x40, 0x01, 0x51, NULL, 0, buf, 8, &out_len)); /* cap < 9 */

    /* One-shot decoder NULL checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_decode_frame(NULL, 5, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_decode_frame(buf, 5, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_mbus_decode_frame(buf, 0, &frame));

    /* Decoder reset & feed NULL checks */
    syn_mbus_decoder_reset(NULL);
    syn_mbus_decoder_feed(NULL, 0x00);
}

/* ── Test: Corrupted and Invalid Frames ──────────────────────────────────── */

static void test_mbus_corrupted_frames(void)
{
    SYN_MBUS_Frame frame;

    /* Short frame len < 5 */
    uint8_t short_short[] = {0x10, 0x40, 0x01, 0x41};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mbus_decode_frame(short_short, sizeof(short_short), &frame));

    /* Long frame len < 9 */
    uint8_t long_short[] = {0x68, 0x03, 0x03, 0x68, 0x40, 0x01, 0x51, 0x92};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mbus_decode_frame(long_short, sizeof(long_short), &frame));

    /* Long frame l1 != l2 */
    uint8_t bad_l1_l2[] = {0x68, 0x03, 0x04, 0x68, 0x40, 0x01, 0x51, 0x92, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(bad_l1_l2, sizeof(bad_l1_l2), &frame));

    /* Long frame corrupted second SOF */
    uint8_t bad_sof2[] = {0x68, 0x03, 0x03, 0x00, 0x40, 0x01, 0x51, 0x92, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(bad_sof2, sizeof(bad_sof2), &frame));

    /* Long frame len < expected_total */
    uint8_t bad_total_len[] = {0x68, 0x05, 0x05, 0x68, 0x40, 0x01, 0x51, 0x92, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_mbus_decode_frame(bad_total_len, sizeof(bad_total_len), &frame));

    /* Long frame bad stop byte */
    uint8_t bad_stop[] = {0x68, 0x03, 0x03, 0x68, 0x40, 0x01, 0x51, 0x92, 0xFF};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(bad_stop, sizeof(bad_stop), &frame));

    /* Long frame l1 < 3 */
    uint8_t bad_l1_small[] = {0x68, 0x02, 0x02, 0x68, 0x40, 0x01, 0x51, 0x92, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_mbus_decode_frame(bad_l1_small, sizeof(bad_l1_small), &frame));

    /* Long frame corrupted checksum */
    uint8_t bad_chk[] = {0x68, 0x03, 0x03, 0x68, 0x40, 0x01, 0x51, 0x00, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_mbus_decode_frame(bad_chk, sizeof(bad_chk), &frame));

    /* Unknown SOF byte */
    uint8_t unknown_sof[] = {0xFF, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_mbus_decode_frame(unknown_sof, sizeof(unknown_sof), &frame));
}

/* ── Test: Streaming Decoder Edge Cases ─────────────────────────────────── */

static void test_mbus_streaming_decoder_edge_cases(void)
{
    reset_test_state();
    SYN_MBUS_Decoder dec;
    syn_mbus_decoder_init(&dec, mbus_on_frame, NULL);

    /* 1. Feed long frame with corrupted second SOF */
    uint8_t bad_sof2[] = {0x68, 0x03, 0x03, 0xFF};
    for (size_t i = 0; i < sizeof(bad_sof2); i++) {
        syn_mbus_decoder_feed(&dec, bad_sof2[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, callback_count);

    /* 3. Feed long frame with L1 != L2 mismatch */
    reset_test_state();
    uint8_t bad_l1_l2[] = {0x68, 0x03, 0x04};
    for (size_t i = 0; i < sizeof(bad_l1_l2); i++) {
        syn_mbus_decoder_feed(&dec, bad_l1_l2[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, callback_count);

    /* 6. Short frame with corrupted stop byte in streaming decoder */
    reset_test_state();
    uint8_t bad_short_stop[] = {0x10, 0x40, 0x01, 0x41, 0x00};
    for (size_t i = 0; i < sizeof(bad_short_stop); i++) {
        syn_mbus_decoder_feed(&dec, bad_short_stop[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, callback_count);

    /* 7. Long frame with corrupted stop byte in streaming decoder */
    reset_test_state();
    uint8_t bad_long_stop[] = {0x68, 0x03, 0x03, 0x68, 0x40, 0x01, 0x51, 0x92, 0x00};
    for (size_t i = 0; i < sizeof(bad_long_stop); i++) {
        syn_mbus_decoder_feed(&dec, bad_long_stop[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, callback_count);
}

static void test_tier_a_mbus_leading_sof_noise_recovery(void)
{
    reset_test_state();
    SYN_MBUS_Decoder dec;
    syn_mbus_decoder_init(&dec, mbus_on_frame, NULL);

    uint8_t payload[2] = {0xAAU, 0x55U};
    uint8_t long_frame[32];
    size_t long_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mbus_encode_long(0x08U, 0x05U, 0x72U, payload, 2U, long_frame,
                                                       sizeof(long_frame), &long_len));

    /* Feed spurious 0x68 noise byte immediately before real Long Frame */
    syn_mbus_decoder_feed(&dec, SYN_MBUS_START_LONG);
    for (size_t i = 0U; i < long_len; i++) {
        syn_mbus_decoder_feed(&dec, long_frame[i]);
    }

    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL(SYN_MBUS_FRAME_TYPE_LONG, last_cb_frame.type);
    TEST_ASSERT_EQUAL_UINT8(0x08U, last_cb_frame.c_field);
    TEST_ASSERT_EQUAL_UINT8(0x05U, last_cb_frame.a_field);
    TEST_ASSERT_EQUAL_UINT8(0x72U, last_cb_frame.ci_field);
    TEST_ASSERT_EQUAL_UINT8(2U, last_cb_frame.payload_len);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, last_cb_frame.payload[0]);
    TEST_ASSERT_EQUAL_HEX8(0x55U, last_cb_frame.payload[1]);
}

void run_mbus_tests(void)
{
    RUN_TEST(test_mbus_calc_checksum);
    RUN_TEST(test_mbus_single_ack);
    RUN_TEST(test_mbus_short_frame);
    RUN_TEST(test_mbus_control_and_long_frame);
    RUN_TEST(test_mbus_streaming_decoder);
    RUN_TEST(test_mbus_null_and_invalid_params);
    RUN_TEST(test_mbus_corrupted_frames);
    RUN_TEST(test_mbus_streaming_decoder_edge_cases);
    RUN_TEST(test_tier_a_mbus_leading_sof_noise_recovery);
}
