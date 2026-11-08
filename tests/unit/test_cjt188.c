/**
 * @file test_cjt188.c
 * @brief Unit tests for CJ/T 188 smart metering protocol driver.
 */

#include "syntropic/proto/syn_cjt188.h"
#include "unity/unity.h"

#include <string.h>

void test_cjt188_checksum_calculation(void)
{
    /* Example CJ/T 188 frame from standard:
     * 68 10 44 33 22 11 00 33 78 01 03 1F 90 00 -> CS = 80
     */
    uint8_t frame[] = {0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00,
                       0x33, 0x78, 0x01, 0x03, 0x1F, 0x90, 0x00};
    uint8_t cs = syn_cjt188_checksum(frame, sizeof(frame));
    TEST_ASSERT_EQUAL_HEX8(0x80, cs);
}

void test_cjt188_encode_read_req(void)
{
    uint8_t buf[32];
    uint8_t meter_id[5] = {0x44, 0x33, 0x22, 0x11, 0x00};
    uint8_t vendor_id[2] = {0x33, 0x78};

    size_t len = syn_cjt188_encode_read_req(buf, sizeof(buf), SYN_CJT188_METER_COLD_WATER, meter_id,
                                            vendor_id, SYN_CJT188_DI_READ_METER_DATA, 0x00);
    TEST_ASSERT_EQUAL(20, len);

    /* Verify preamble */
    TEST_ASSERT_EQUAL_HEX8(0xFE, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFE, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0xFE, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xFE, buf[3]);

    /* Verify Start, Type, Control, Length, End */
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_METER_COLD_WATER, buf[5]);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_CTRL_READ_DATA, buf[13]);
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[14]);
    TEST_ASSERT_EQUAL_HEX8(0x80, buf[18]); /* Checksum */
    TEST_ASSERT_EQUAL_HEX8(0x16, buf[19]); /* End byte */
}

void test_cjt188_encode_valve_ctrl(void)
{
    uint8_t buf[32];
    uint8_t meter_id[5] = {0x69, 0x05, 0x90, 0x05, 0x15};
    uint8_t vendor_id[2] = {0x33, 0x78};

    /* Encode Open Valve (0x55) */
    size_t len = syn_cjt188_encode_valve_ctrl(buf, sizeof(buf), SYN_CJT188_METER_COLD_WATER,
                                              meter_id, vendor_id, true, 0x01);
    TEST_ASSERT_EQUAL(21, len);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_CTRL_WRITE_DATA, buf[13]);
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[14]);                  /* Data len */
    TEST_ASSERT_EQUAL_HEX8(0x17, buf[15]);                  /* DI LSB */
    TEST_ASSERT_EQUAL_HEX8(0xA0, buf[16]);                  /* DI MSB */
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[17]);                  /* SER */
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_VALVE_OPEN, buf[18]); /* 0x55 */
    TEST_ASSERT_EQUAL_HEX8(0x16, buf[20]);                  /* End byte */
}

void test_cjt188_parse_valid_frame(void)
{
    /* Sample CJ/T 188 response frame:
     * FE FE FE FE 68 10 44 33 22 11 00 33 78 81 16 1F 90 00 ... CS 16
     */
    uint8_t raw[] = {0xFE, 0xFE, 0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00, 0x33,
                     0x78, 0x81, 0x05, 0x1F, 0x90, 0x00, 0x77, 0x66, 0x2E, 0x16};
    /* Compute checksum over 68 .. 66 (16 bytes) */
    raw[18] = syn_cjt188_checksum(&raw[2], 16);

    SYN_CJT188_Frame frame;
    bool ok = syn_cjt188_parse_frame(raw, sizeof(raw), &frame);
    TEST_ASSERT_TRUE(ok);

    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_METER_COLD_WATER, frame.meter_type);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_CTRL_READ_DATA_RESP, frame.ctrl);
    TEST_ASSERT_EQUAL(0x5, frame.len);
    TEST_ASSERT_EQUAL_HEX16(SYN_CJT188_DI_READ_METER_DATA, frame.data_id);
    TEST_ASSERT_EQUAL(0x00, frame.seq);
    TEST_ASSERT_NOT_NULL(frame.payload);
    TEST_ASSERT_EQUAL(2, frame.payload_len);
    TEST_ASSERT_EQUAL_HEX8(0x77, frame.payload[0]);
    TEST_ASSERT_EQUAL_HEX8(0x66, frame.payload[1]);
}

void test_cjt188_streaming_decoder(void)
{
    SYN_CJT188_Decoder decoder;
    syn_cjt188_decoder_init(&decoder);

    uint8_t raw[] = {0xFE, 0xFE, 0xFE, 0xFE, 0x68, 0x10, 0x44, 0x33, 0x22, 0x11,
                     0x00, 0x33, 0x78, 0x01, 0x03, 0x1F, 0x90, 0x00, 0x80, 0x16};

    SYN_CJT188_Frame frame;
    bool got_frame = false;

    for (size_t i = 0; i < sizeof(raw); i++) {
        if (syn_cjt188_decoder_feed(&decoder, raw[i], &frame)) {
            got_frame = true;
        }
    }

    TEST_ASSERT_TRUE(got_frame);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_METER_COLD_WATER, frame.meter_type);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_CTRL_READ_DATA, frame.ctrl);
    TEST_ASSERT_EQUAL_HEX16(SYN_CJT188_DI_READ_METER_DATA, frame.data_id);
}

void test_cjt188_edge_cases_and_nulls(void)
{
    SYN_CJT188_Frame frame;
    SYN_CJT188_Decoder decoder;
    uint8_t buf[32];
    uint8_t meter_id[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t vendor_id[2] = {0x12, 0x34};

    /* NULL / Invalid checks for checksum, encode read/valve */
    TEST_ASSERT_EQUAL_HEX8(0, syn_cjt188_checksum(NULL, 10));
    TEST_ASSERT_EQUAL_HEX8(0, syn_cjt188_checksum(buf, 0));
    TEST_ASSERT_EQUAL(0,
                      syn_cjt188_encode_read_req(NULL, 32, 0x10, meter_id, vendor_id, 0x901F, 0));
    TEST_ASSERT_EQUAL(0, syn_cjt188_encode_read_req(buf, 10, 0x10, meter_id, vendor_id, 0x901F, 0));
    TEST_ASSERT_EQUAL(0,
                      syn_cjt188_encode_valve_ctrl(NULL, 32, 0x10, meter_id, vendor_id, true, 0));
    TEST_ASSERT_EQUAL(0, syn_cjt188_encode_valve_ctrl(buf, 10, 0x10, meter_id, vendor_id, true, 0));

    /* Parse nulls & short buffer */
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(NULL, 20, &frame));
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(buf, 20, NULL));
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(buf, 5, &frame));

    /* Preamble only */
    uint8_t preamble_only[15] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
                                 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(preamble_only, sizeof(preamble_only), &frame));

    /* Bad start byte, bad end byte, bad checksum */
    uint8_t bad_start[] = {0x69, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00, 0x33,
                           0x78, 0x01, 0x03, 0x1F, 0x90, 0x00, 0x80, 0x16};
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(bad_start, sizeof(bad_start), &frame));

    uint8_t bad_end[] = {0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00, 0x33,
                         0x78, 0x01, 0x03, 0x1F, 0x90, 0x00, 0x80, 0x17};
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(bad_end, sizeof(bad_end), &frame));

    uint8_t bad_cs[] = {0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00, 0x33,
                        0x78, 0x01, 0x03, 0x1F, 0x90, 0x00, 0x81, 0x16};
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(bad_cs, sizeof(bad_cs), &frame));

    /* Frame with data length < 3 (empty payload) */
    uint8_t short_data[] = {0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00,
                            0x33, 0x78, 0x01, 0x00, 0x00, 0x16};
    short_data[11] = syn_cjt188_checksum(short_data, 11);
    TEST_ASSERT_TRUE(syn_cjt188_parse_frame(short_data, sizeof(short_data), &frame));
    TEST_ASSERT_EQUAL(0, frame.data_id);

    /* Decoder null feed and invalid byte feeds */
    syn_cjt188_decoder_init(&decoder);
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(NULL, 0x68, &frame));
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, 0x68, NULL));
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, 0xAA, &frame)); /* non-start byte */

    /* Decoder invalid length byte > 128 */
    syn_cjt188_decoder_init(&decoder);
    syn_cjt188_decoder_feed(&decoder, 0x68, &frame);
    for (int i = 0; i < 9; i++)
        syn_cjt188_decoder_feed(&decoder, 0x00, &frame);
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, 0xFF, &frame)); /* Len 255 > 128 */

    /* Frame where payload length byte L specifies 20 bytes, but buffer size is only 15 bytes */
    uint8_t truncated_len[] = {0x68, 0x10, 0x44, 0x33, 0x22, 0x11, 0x00, 0x33,
                               0x78, 0x01, 0x14, 0x1F, 0x90, 0x00, 0x16};
    TEST_ASSERT_FALSE(syn_cjt188_parse_frame(truncated_len, sizeof(truncated_len), &frame));

    /* 1. Close valve encoding */
    size_t len = syn_cjt188_encode_valve_ctrl(buf, sizeof(buf), SYN_CJT188_METER_COLD_WATER,
                                              meter_id, vendor_id, false, 0x02);
    TEST_ASSERT_EQUAL(21, len);
    TEST_ASSERT_EQUAL_HEX8(SYN_CJT188_VALVE_CLOSE, buf[18]); /* 0x99 */

    /* 2. Decoder init NULL guard */
    syn_cjt188_decoder_init(NULL);

    /* 3. Decoder with oversized expected_len > 128 */
    syn_cjt188_decoder_init(&decoder);
    decoder.in_frame = true;
    decoder.index = 10;
    TEST_ASSERT_FALSE(
        syn_cjt188_decoder_feed(&decoder, 240, &frame)); /* data_len = 240 -> expected_len > 128 */
    TEST_ASSERT_FALSE(decoder.in_frame);

    /* 4. Direct buffer index overflow test (line 223) */
    syn_cjt188_decoder_init(&decoder);
    decoder.in_frame = true;
    decoder.index = 128;
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, 0x00, &frame));
    TEST_ASSERT_EQUAL(0, decoder.index);
    TEST_ASSERT_FALSE(decoder.in_frame);
}

void test_r5_cjt188_streaming_nonzero_payload(void)
{
    SYN_CJT188_Decoder decoder;
    syn_cjt188_decoder_init(&decoder);

    /* Read Data Response (0x81) with L=8 (DI 2B + SER 1B + 5B BCD meter flow payload) */
    uint8_t resp[] = {
        0xFEU, 0xFEU, 0x68U, 0x10U, 0x44U, 0x33U, 0x22U, 0x11U, 0x00U, 0x33U, 0x78U, 0x81U,
        0x08U, 0x1FU, 0x90U, 0x02U, 0x12U, 0x34U, 0x56U, 0x78U, 0x2CU, 0x00U, 0x16U,
    };
    resp[21] = syn_cjt188_checksum(&resp[2], 19U);

    SYN_CJT188_Frame frame;
    memset(&frame, 0, sizeof(frame));
    bool got_frame = false;
    for (size_t i = 0U; i < sizeof(resp); i++) {
        if (syn_cjt188_decoder_feed(&decoder, resp[i], &frame)) {
            got_frame = true;
        }
    }

    TEST_ASSERT_TRUE(got_frame);
    TEST_ASSERT_FALSE(decoder.in_frame);
    TEST_ASSERT_EQUAL(0U, decoder.index);
    TEST_ASSERT_NOT_NULL(frame.payload);
    TEST_ASSERT_EQUAL(5U, frame.payload_len);
    /* Payload bytes in zero-copy decoder->buf must NOT be zeroed upon completion */
    TEST_ASSERT_EQUAL_HEX8(0x12U, frame.payload[0]);
    TEST_ASSERT_EQUAL_HEX8(0x34U, frame.payload[1]);
    TEST_ASSERT_EQUAL_HEX8(0x56U, frame.payload[2]);
    TEST_ASSERT_EQUAL_HEX8(0x78U, frame.payload[3]);
    TEST_ASSERT_EQUAL_HEX8(0x2CU, frame.payload[4]);
}

void test_tier_a_cjt188_resync_on_len_cs_and_eof_errors(void)
{
    uint8_t meter_id[5] = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U};
    uint8_t vendor_id[2] = {0x12U, 0x34U};
    uint8_t encoded[32];
    size_t enc_len =
        syn_cjt188_encode_read_req(encoded, sizeof(encoded), SYN_CJT188_METER_GAS, meter_id,
                                   vendor_id, SYN_CJT188_DI_READ_METER_DATA, 0x07U);
    TEST_ASSERT_EQUAL(20U, enc_len);
    const uint8_t *real_frame = &encoded[4]; /* 16-byte frame starting at 0x68 */

    SYN_CJT188_Decoder decoder;
    SYN_CJT188_Frame frame;

    /* 1. Resync on oversized length byte (buf[10] = 0xFF) when real 0x68 sits at buf[3] (< 11B rem)
     */
    syn_cjt188_decoder_init(&decoder);
    uint8_t noise_len[3] = {0x68U, 0xAAU, 0xBBU};
    for (size_t i = 0U; i < sizeof(noise_len); i++) {
        TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, noise_len[i], &frame));
    }
    /* Copy real_frame, with vendor_id[0] at real_frame[7] == false_hdr[10] set to 0xFF */
    uint8_t real_hi_v[16];
    memcpy(real_hi_v, real_frame, sizeof(real_hi_v));
    real_hi_v[7] =
        0xFFU; /* At false index 10 -> expected_len = 268 > 128 -> resyncs to real_hi_v[0]! */
    real_hi_v[14] = syn_cjt188_checksum(real_hi_v, 14U);

    bool got = false;
    for (size_t i = 0U; i < sizeof(real_hi_v); i++) {
        if (syn_cjt188_decoder_feed(&decoder, real_hi_v[i], &frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    TEST_ASSERT_EQUAL_HEX8(0x07U, frame.seq);

    /* 2. Resync on EOF/checksum mismatch when >= 11B of real frame are already buffered,
     * including skipping an intermediate 0x68 whose shifted buf[10] > 128! */
    syn_cjt188_decoder_init(&decoder);
    /* False candidate: [0]=0x68, [1]=0x68 (intermediate!), [2..9]=0, [10]=0x03 (expected_len=16),
     * [11]=0xFF (shifted index 10 for [1]=0x68 -> expected_len=268 > 128!), [12..15]=start of
     * real_frame */
    uint8_t prefix[12] = {0x68U, 0x68U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0x03U, 0xFFU};
    for (size_t i = 0U; i < sizeof(prefix); i++) {
        TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, prefix[i], &frame));
    }
    got = false;
    for (size_t i = 0U; i < 16U; i++) {
        if (syn_cjt188_decoder_feed(&decoder, real_frame[i], &frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    TEST_ASSERT_EQUAL_HEX16(SYN_CJT188_DI_READ_METER_DATA, frame.data_id);

    /* 3. Resync where >= 11B of real frame with valid expected_len <= 128 are already in buffer */
    syn_cjt188_decoder_init(&decoder);
    uint8_t prefix2[2] = {
        0x68U, 0x11U}; /* real_frame[8] is at false index 10: vendor_id[1]=0x34 -> len=65 <= 128 */
    /* Make false candidate expected_len = 14 by setting real_frame[8] = 0x01 */
    uint8_t real_v1[16];
    memcpy(real_v1, real_frame, sizeof(real_v1));
    real_v1[8] =
        0x01U; /* False index 10 = 1 -> false expected_len = 14; at index 14 rem = 12 >= 11! */
    real_v1[14] = syn_cjt188_checksum(real_v1, 14U);

    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, prefix2[0], &frame));
    TEST_ASSERT_FALSE(syn_cjt188_decoder_feed(&decoder, prefix2[1], &frame));
    got = false;
    for (size_t i = 0U; i < sizeof(real_v1); i++) {
        if (syn_cjt188_decoder_feed(&decoder, real_v1[i], &frame)) {
            got = true;
        }
    }
    TEST_ASSERT_TRUE(got);
    TEST_ASSERT_EQUAL_HEX8(0x07U, frame.seq);
}

void run_cjt188_tests(void)
{
    RUN_TEST(test_cjt188_checksum_calculation);
    RUN_TEST(test_cjt188_encode_read_req);
    RUN_TEST(test_cjt188_encode_valve_ctrl);
    RUN_TEST(test_cjt188_parse_valid_frame);
    RUN_TEST(test_cjt188_streaming_decoder);
    RUN_TEST(test_cjt188_edge_cases_and_nulls);
    RUN_TEST(test_r5_cjt188_streaming_nonzero_payload);
    RUN_TEST(test_tier_a_cjt188_resync_on_len_cs_and_eof_errors);
}
