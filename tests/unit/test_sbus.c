/**
 * @file test_sbus.c
 * @brief Unit tests for Zero-Heap SBUS Receiver Decoder.
 */

#include "syntropic/proto/syn_sbus.h"
#include "unity/unity.h"

#include <string.h>

void test_sbus_init(void)
{
    SYN_SBUS_Parser parser;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbus_init(&parser));
    TEST_ASSERT_EQUAL_UINT8(0, parser.idx);
    TEST_ASSERT_EQUAL_UINT32(0, parser.frames_received);
}

void test_sbus_decode_buffer(void)
{
    uint8_t buf[25] = {0};
    buf[0] = 0x0F;  /* Header */
    buf[23] = 0x0C; /* Frame loss (bit 2) + Failsafe (bit 3) */
    buf[24] = 0x00; /* Footer */

    /* Pack channel 0 = 992 (0x03E0) into byte 1 & byte 2 */
    buf[1] = 0xE0;
    buf[2] = 0x03;

    SYN_SBUS_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbus_decode_buffer(buf, &frame));
    TEST_ASSERT_EQUAL_UINT16(992, frame.channels[0]);
    TEST_ASSERT_TRUE(frame.frame_loss);
    TEST_ASSERT_TRUE(frame.failsafe);
    TEST_ASSERT_FALSE(frame.ch17);
    TEST_ASSERT_FALSE(frame.ch18);
}

void test_sbus_streaming_parser(void)
{
    SYN_SBUS_Parser parser;
    syn_sbus_init(&parser);

    uint8_t buf[25] = {0};
    buf[0] = 0x0F;
    buf[1] = 0xAC;
    buf[2] = 0x06; /* Raw 1708 */

    SYN_SBUS_Frame frame;
    SYN_Status status = SYN_BUSY;

    for (int i = 0; i < 24; i++) {
        status = syn_sbus_parse_byte(&parser, buf[i], &frame);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, status);
    }

    /* Final byte triggers frame completion */
    status = syn_sbus_parse_byte(&parser, buf[24], &frame);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_UINT32(1, parser.frames_received);
    TEST_ASSERT_EQUAL_UINT16(1708, frame.channels[0]);

    /* Test parsing frame with failsafe flag and NULL frame output pointer */
    buf[23] = 0x0C; /* Frame loss + Failsafe */
    for (int i = 0; i < 24; i++) {
        syn_sbus_parse_byte(&parser, buf[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbus_parse_byte(&parser, buf[24], NULL));
    TEST_ASSERT_EQUAL_UINT32(2, parser.frames_received);
    TEST_ASSERT_EQUAL_UINT32(1, parser.frame_loss_count);
    TEST_ASSERT_EQUAL_UINT32(1, parser.failsafe_count);
}

void test_sbus_raw_to_us_scaling(void)
{
    TEST_ASSERT_EQUAL_UINT16(1000, syn_sbus_raw_to_us(100));  /* Under-range clamping */
    TEST_ASSERT_EQUAL_UINT16(1000, syn_sbus_raw_to_us(172));  /* Min scale */
    TEST_ASSERT_EQUAL_UINT16(1500, syn_sbus_raw_to_us(992));  /* Mid scale */
    TEST_ASSERT_EQUAL_UINT16(2000, syn_sbus_raw_to_us(1811)); /* Max scale */
    TEST_ASSERT_EQUAL_UINT16(2000, syn_sbus_raw_to_us(2000)); /* Over-range clamping */
}

void test_sbus_null_and_error_handling(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbus_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbus_decode_buffer(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbus_parse_byte(NULL, 0x0F, NULL));

    uint8_t bad_buf[25] = {0x00}; /* Bad header 0x00 (expected 0x0F) */
    SYN_SBUS_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_decode_buffer(bad_buf, &frame));

    SYN_SBUS_Parser parser;
    syn_sbus_init(&parser);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_parse_byte(&parser, 0xAA, &frame));

    /* Stream 24 valid bytes starting with 0x0F, corrupt buf[0] before 25th byte */
    syn_sbus_init(&parser);
    uint8_t buf[25];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x0F;
    for (int i = 0; i < 24; i++) {
        syn_sbus_parse_byte(&parser, buf[i], &frame);
    }
    parser.buf[0] = 0x00; /* Corrupt header */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_parse_byte(&parser, 0x00, &frame));
}

void test_tier_a_sbus_footer_flags_and_resync(void)
{
    SYN_SBUS_Frame frame;
    uint8_t buf[25];
    memset(buf, 0, sizeof(buf));
    buf[0] = SYN_SBUS_HEADER;

    /* Valid SBUS2 footer (low nibble == 0x04, e.g. 0x14) must be accepted */
    buf[24] = 0x14U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbus_decode_buffer(buf, &frame));

    /* Invalid footer byte (e.g. 0x55) must be rejected */
    buf[24] = 0x55U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_decode_buffer(buf, &frame));

    /* Non-zero reserved flag bits in byte 23 (bits 7:4) must be rejected */
    buf[24] = 0x00U;
    buf[23] = 0x10U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_decode_buffer(buf, &frame));

    /* Null frame pointer with valid buf must return SYN_INVALID_PARAM */
    buf[23] = 0x00U;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbus_decode_buffer(buf, NULL));

    /* Overlapping stream resync: 3 noise bytes starting with 0x0F followed by valid 25B frame */
    SYN_SBUS_Parser parser;
    syn_sbus_init(&parser);

    uint8_t real_frame[25];
    memset(real_frame, 0, sizeof(real_frame));
    real_frame[0] = SYN_SBUS_HEADER;
    real_frame[1] = 0xE0U;
    real_frame[2] = 0x03U;  /* ch0 = 992 */
    real_frame[21] = 0xFFU; /* byte 24 of false candidate has footer 0xFF != 0x00/0xX4 */
    real_frame[23] = 0x03U; /* ch17 + ch18 */
    real_frame[24] = 0x04U; /* valid SBUS2 footer */

    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, 0x0FU, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, 0xAAU, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, 0xBBU, &frame));

    /* Feed first 22 bytes of real_frame: 22nd byte completes 25B false candidate -> SYN_ERROR +
     * resync */
    for (size_t i = 0; i < 21U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, real_frame[i], &frame));
    }
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbus_parse_byte(&parser, real_frame[21], &frame));
    TEST_ASSERT_EQUAL_UINT8(22U, parser.idx);

    /* Feed remaining 3 bytes of real_frame -> recovers frame cleanly! */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, real_frame[22], &frame));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_sbus_parse_byte(&parser, real_frame[23], &frame));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbus_parse_byte(&parser, real_frame[24], &frame));
    TEST_ASSERT_EQUAL_UINT32(1U, parser.frames_received);
    TEST_ASSERT_EQUAL_UINT16(992U, frame.channels[0]);
    TEST_ASSERT_TRUE(frame.ch17);
    TEST_ASSERT_TRUE(frame.ch18);
}
