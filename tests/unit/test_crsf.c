/**
 * @file test_crsf.c
 * @brief Unit tests for Zero-Heap CRSF (TBS Crossfire / ExpressLRS) Protocol Parser.
 */

#include "syntropic/proto/syn_crsf.h"
#include "unity/unity.h"

void test_crsf_init(void)
{
    SYN_CRSF_Parser parser;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_crsf_init(&parser));
    TEST_ASSERT_EQUAL_UINT8(0, parser.idx);
    TEST_ASSERT_EQUAL_UINT32(0, parser.packets_received);
}

void test_crsf_crc8_calculation(void)
{
    /* Test CRC8 DVB-S2 (poly 0xD5) */
    uint8_t data[] = {0x16, 0x00, 0x00};
    uint8_t crc = syn_crsf_calc_crc(data, sizeof(data));
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_crsf_parse_rc_channels(void)
{
    SYN_CRSF_Parser parser;
    syn_crsf_init(&parser);

    /* Construct 26-byte CRSF RC channels packet */
    uint8_t pkt[26] = {0};
    pkt[0] = 0xC8; /* Addr */
    pkt[1] = 24;   /* Payload length (1 type + 22 channels + 1 crc) */
    pkt[2] = 0x16; /* RC Channels frame type */

    /* Pack channel 0 = 992 (0x03E0) into payload[0..1] */
    pkt[3] = 0xE0;
    pkt[4] = 0x03;

    /* Compute expected CRC over bytes 2..24 */
    pkt[25] = syn_crsf_calc_crc(&pkt[2], 23);

    SYN_CRSF_FrameType type;
    SYN_Status status = SYN_BUSY;

    for (int i = 0; i < 25; i++) {
        status = syn_crsf_parse_byte(&parser, pkt[i], &type);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, status);
    }

    status = syn_crsf_parse_byte(&parser, pkt[25], &type);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_INT(SYN_CRSF_TYPE_RC_CHANNELS, type);
    TEST_ASSERT_EQUAL_UINT16(992, parser.last_channels.channels[0]);

    /* Test parsing with NULL type_out pointer */
    for (int i = 0; i < 25; i++) {
        syn_crsf_parse_byte(&parser, pkt[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_crsf_parse_byte(&parser, pkt[25], NULL));
    TEST_ASSERT_EQUAL_UINT32(2, parser.packets_received);
}

void test_crsf_parse_link_stats(void)
{
    SYN_CRSF_Parser parser;
    syn_crsf_init(&parser);

    /* Construct 14-byte CRSF Link Statistics packet */
    uint8_t pkt[14] = {0};
    pkt[0] = 0xC8; /* Addr */
    pkt[1] = 12;   /* Payload len: 1 type + 10 stats + 1 crc */
    pkt[2] = 0x14; /* Link Statistics frame type */

    pkt[3] = 90;             /* Uplink RSSI 1 */
    pkt[4] = 85;             /* Uplink RSSI 2 */
    pkt[5] = 99;             /* Uplink Quality % */
    pkt[6] = (uint8_t)(-10); /* Uplink SNR */

    pkt[13] = syn_crsf_calc_crc(&pkt[2], 11);

    SYN_CRSF_FrameType type;
    for (int i = 0; i < 13; i++) {
        syn_crsf_parse_byte(&parser, pkt[i], &type);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_crsf_parse_byte(&parser, pkt[13], &type));
    TEST_ASSERT_EQUAL_INT(SYN_CRSF_TYPE_LINK_STATISTICS, type);
    TEST_ASSERT_EQUAL_UINT8(90, parser.last_link_stats.uplink_rssi1);
    TEST_ASSERT_EQUAL_UINT8(99, parser.last_link_stats.uplink_quality);
    TEST_ASSERT_EQUAL_INT8(-10, parser.last_link_stats.uplink_snr);
}

void test_crsf_raw_to_us_scaling(void)
{
    TEST_ASSERT_EQUAL_UINT16(988, syn_crsf_raw_to_us(100));   /* Under-range */
    TEST_ASSERT_EQUAL_UINT16(988, syn_crsf_raw_to_us(170));   /* Min */
    TEST_ASSERT_EQUAL_UINT16(1472, syn_crsf_raw_to_us(992));  /* Mid */
    TEST_ASSERT_EQUAL_UINT16(2012, syn_crsf_raw_to_us(1908)); /* Max */
    TEST_ASSERT_EQUAL_UINT16(2012, syn_crsf_raw_to_us(2500)); /* Over-range */
}

void test_crsf_null_and_error_handling(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_crsf_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_crsf_parse_byte(NULL, 0xC8, NULL));
    TEST_ASSERT_EQUAL_UINT8(0, syn_crsf_calc_crc(NULL, 10));

    SYN_CRSF_Parser parser;
    syn_crsf_init(&parser);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, 0x00, NULL)); /* Bad addr */

    /* Invalid payload length < 2 */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_crsf_parse_byte(&parser, 0xC8, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, 1, NULL));
    TEST_ASSERT_EQUAL_UINT8(0, parser.idx);

    /* Invalid payload length > max */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_crsf_parse_byte(&parser, 0xC8, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, 100, NULL));
    TEST_ASSERT_EQUAL_UINT8(0, parser.idx);

    /* CRC error test */
    uint8_t bad_pkt[6] = {0xC8, 4, 0x16, 0x01, 0x02, 0x00}; /* Bad CRC 0x00 */
    for (int i = 0; i < 5; i++) {
        syn_crsf_parse_byte(&parser, bad_pkt[i], NULL);
    }
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, bad_pkt[5], NULL));
    TEST_ASSERT_EQUAL_UINT32(1, parser.crc_errors);
}

void test_tier_a_crsf_short_payload_guards(void)
{
    SYN_CRSF_Parser parser;
    SYN_CRSF_FrameType type = (SYN_CRSF_FrameType)0;
    syn_crsf_init(&parser);

    /* Poison internal buffer with non-zero bytes to catch stale overread */
    for (size_t i = 0; i < sizeof(parser.buf); i++) {
        parser.buf[i] = 0xFFU;
    }

    /* Short RC_CHANNELS packet (payload_len = 4 < 24) with VALID CRC must be rejected */
    uint8_t short_rc[6] = {SYN_CRSF_ADDR_FC, 4U, SYN_CRSF_TYPE_RC_CHANNELS, 0x01U, 0x02U, 0U};
    short_rc[5] = syn_crsf_calc_crc(&short_rc[2], 3U);
    for (size_t i = 0; i < 5U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_crsf_parse_byte(&parser, short_rc[i], &type));
    }
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, short_rc[5], &type));
    TEST_ASSERT_EQUAL_UINT32(0U, parser.crc_errors);
    TEST_ASSERT_EQUAL_UINT32(0U, parser.packets_received);
    TEST_ASSERT_EQUAL_UINT16(0U, parser.last_channels.channels[15]);

    /* Short LINK_STATISTICS packet (payload_len = 5 < 12) with VALID CRC must be rejected */
    uint8_t short_ls[7] = {
        SYN_CRSF_ADDR_FC, 5U, SYN_CRSF_TYPE_LINK_STATISTICS, 0x10U, 0x20U, 0x30U, 0U};
    short_ls[6] = syn_crsf_calc_crc(&short_ls[2], 4U);
    for (size_t i = 0; i < 6U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_crsf_parse_byte(&parser, short_ls[i], &type));
    }
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_crsf_parse_byte(&parser, short_ls[6], &type));
    TEST_ASSERT_EQUAL_UINT32(0U, parser.crc_errors);
    TEST_ASSERT_EQUAL_UINT32(0U, parser.packets_received);
    TEST_ASSERT_EQUAL_UINT8(0U, parser.last_link_stats.downlink_quality);

    /* Other frame types (e.g. BATTERY = 0x08) with short valid payload still succeed */
    uint8_t batt_pkt[6] = {SYN_CRSF_ADDR_FC, 4U, SYN_CRSF_TYPE_BATTERY, 0xAAU, 0x55U, 0U};
    batt_pkt[5] = syn_crsf_calc_crc(&batt_pkt[2], 3U);
    for (size_t i = 0; i < 5U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_crsf_parse_byte(&parser, batt_pkt[i], &type));
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_crsf_parse_byte(&parser, batt_pkt[5], &type));
    TEST_ASSERT_EQUAL_INT(SYN_CRSF_TYPE_BATTERY, type);
    TEST_ASSERT_EQUAL_UINT32(1U, parser.packets_received);
}
