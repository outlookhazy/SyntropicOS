/**
 * @file test_msp.c
 * @brief Unit tests for Zero-Heap MSP (MultiWii Serial Protocol v1/v2) Encoder & Decoder.
 */

#include "syntropic/proto/syn_msp.h"
#include "unity/unity.h"

void test_msp_init(void)
{
    SYN_MSP_Parser parser;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_msp_init(&parser));
    TEST_ASSERT_EQUAL_UINT32(0, parser.frames_received);
}

void test_msp_encode_and_parse_response(void)
{
    uint8_t payload[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t buf[32];
    size_t out_len = 0;

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_msp_encode_response(SYN_MSP_STATUS, payload, 4, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_INT(10, out_len); /* $M> + size(4) + cmd(101) + 4 payload + checksum */

    SYN_MSP_Parser parser;
    syn_msp_init(&parser);

    SYN_MSP_Frame frame;
    SYN_Status status = SYN_BUSY;

    for (size_t i = 0; i < out_len - 1; i++) {
        status = syn_msp_parse_byte(&parser, buf[i], &frame);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, status);
    }

    status = syn_msp_parse_byte(&parser, buf[out_len - 1], &frame);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_UINT8('>', frame.dir_char);
    TEST_ASSERT_EQUAL_UINT8(SYN_MSP_STATUS, frame.cmd);
    TEST_ASSERT_EQUAL_UINT8(4, frame.payload_len);
    TEST_ASSERT_EQUAL_UINT8(0x01, frame.payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0x04, frame.payload[3]);
}

void test_msp_null_and_error_handling(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_msp_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_msp_parse_byte(NULL, '$', NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_msp_encode_response(101, NULL, 0, NULL, 0, NULL));

    /* Regression: buf_size too small for the frame must return SYN_INVALID_PARAM */
    uint8_t small_buf[4];
    size_t dummy_len = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_msp_encode_response(101, NULL, 4, small_buf, sizeof(small_buf), &dummy_len));

    SYN_MSP_Parser parser;
    syn_msp_init(&parser);

    /* Test corrupt checksum */
    uint8_t bad_frame[6] = {'$', 'M', '<', 0, 101, 0xFF};
    SYN_Status status = SYN_BUSY;
    for (int i = 0; i < 5; i++) {
        (void)syn_msp_parse_byte(&parser, bad_frame[i], NULL);
    }
    status = syn_msp_parse_byte(&parser, bad_frame[5], NULL);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, status);
    TEST_ASSERT_EQUAL_UINT32(1, parser.checksum_errors);

    /* Test header resets on invalid characters */
    syn_msp_init(&parser);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_msp_parse_byte(&parser, '$', NULL));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_msp_parse_byte(&parser, 'X', NULL)); /* Non-M char resets */

    TEST_ASSERT_EQUAL(SYN_BUSY, syn_msp_parse_byte(&parser, '$', NULL));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_msp_parse_byte(&parser, 'M', NULL));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_msp_parse_byte(&parser, 'Z', NULL)); /* Invalid dir char resets */

    /* Test payload length overflow */
    syn_msp_init(&parser);
    syn_msp_parse_byte(&parser, '$', NULL);
    syn_msp_parse_byte(&parser, 'M', NULL);
    syn_msp_parse_byte(&parser, '<', NULL);
    syn_msp_parse_byte(&parser, 250, NULL); /* len = 250 > SYN_MSP_MAX_PAYLOAD */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_msp_parse_byte(&parser, 101, NULL)); /* cmd -> error */

    /* Test default invalid state fallback */
    parser.state = 99;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_msp_parse_byte(&parser, '$', NULL));
}

void test_tier_a_msp_header_resync_and_null_payload(void)
{
    uint8_t buf[32];
    size_t out_len = 0U;

    /* NULL payload with len > 0 and sufficient buf_size must return SYN_INVALID_PARAM */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_msp_encode_response(SYN_MSP_STATUS, NULL, 4U, buf,
                                                                     sizeof(buf), &out_len));

    /* Zero-length payload with NULL payload pointer is valid */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_msp_encode_response(SYN_MSP_API_VERSION, NULL, 0U, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_size_t(6U, out_len);

    /* '$' arriving in STATE_HEADER_M ("$$M>...") and STATE_HEADER_DIR ("$M$M>...") resyncs cleanly
     */
    SYN_MSP_Parser parser;
    SYN_MSP_Frame frame;
    syn_msp_init(&parser);

    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_msp_parse_byte(&parser, '$', &frame));
    for (size_t i = 0U; i < out_len - 1U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_msp_parse_byte(&parser, buf[i], &frame));
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_msp_parse_byte(&parser, buf[out_len - 1U], &frame));
    TEST_ASSERT_EQUAL_UINT8(SYN_MSP_API_VERSION, frame.cmd);

    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_msp_parse_byte(&parser, '$', &frame));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_msp_parse_byte(&parser, 'M', &frame));
    for (size_t i = 0U; i < out_len - 1U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_msp_parse_byte(&parser, buf[i], &frame));
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_msp_parse_byte(&parser, buf[out_len - 1U], &frame));
    TEST_ASSERT_EQUAL_UINT32(2U, parser.frames_received);
}
