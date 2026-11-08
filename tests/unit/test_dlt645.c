/**
 * @file test_dlt645.c
 * @brief Unity unit tests for DL/T 645 protocol engine.
 */

#include "syntropic/proto/syn_dlt645.h"
#include "unity/unity.h"

#include <string.h>

static SYN_DLT645_Frame last_decoded_frame;
static int callback_count = 0;

static void on_frame_decoded(const SYN_DLT645_Frame *frame, void *ctx)
{
    (void)ctx;
    last_decoded_frame = *frame;
    callback_count++;
}

/* ── Test: Checksum Calculation ─────────────────────────────────────────── */

static void test_dlt645_checksum(void)
{
    uint8_t buf[] = {0x68, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                     0x68, 0x11, 0x04, 0x33, 0x33, 0x33, 0x33};
    uint8_t cs = syn_dlt645_calc_checksum(buf, sizeof(buf));
    TEST_ASSERT_TRUE(cs != 0);

    /* NULL check */
    TEST_ASSERT_EQUAL_UINT8(0, syn_dlt645_calc_checksum(NULL, 10));
}

/* ── Test: 2007 Encoding & Decoding Roundtrip ───────────────────────────── */

static void test_dlt645_2007_roundtrip(void)
{
    SYN_DLT645_Frame tx_frame;
    memset(&tx_frame, 0, sizeof(tx_frame));

    tx_frame.version = SYN_DLT645_VER_2007;
    tx_frame.control = SYN_DLT645_CMD_READ_DATA_RESP;
    tx_frame.data_id = 0x00010000; /* Active Energy DI */
    tx_frame.address[0] = 0x12;
    tx_frame.address[1] = 0x34;
    tx_frame.address[2] = 0x56;
    tx_frame.address[3] = 0x78;
    tx_frame.address[4] = 0x90;
    tx_frame.address[5] = 0x12;

    uint8_t payload[] = {0x12, 0x34, 0x56, 0x78};
    memcpy(tx_frame.payload, payload, sizeof(payload));
    tx_frame.payload_len = sizeof(payload);

    uint8_t encoded[64];
    size_t enc_len = syn_dlt645_encode(&tx_frame, encoded, sizeof(encoded));
    TEST_ASSERT_TRUE(enc_len > 0);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_SOF, encoded[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_EOF, encoded[enc_len - 1]);

    /* Parse encoded frame */
    SYN_DLT645_Frame rx_frame;
    SYN_Status status = syn_dlt645_parse(encoded, enc_len, SYN_DLT645_VER_2007, &rx_frame);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_CMD_READ_DATA_RESP, rx_frame.control);
    TEST_ASSERT_EQUAL_UINT32(0x00010000, rx_frame.data_id);
    TEST_ASSERT_EQUAL_UINT8(4, rx_frame.payload_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, rx_frame.payload, 4);
    TEST_ASSERT_EQUAL_MEMORY(tx_frame.address, rx_frame.address, 6);
}

/* ── Test: 1997 Encoding & Decoding Roundtrip ───────────────────────────── */

static void test_dlt645_1997_roundtrip(void)
{
    SYN_DLT645_Frame tx_frame;
    memset(&tx_frame, 0, sizeof(tx_frame));

    tx_frame.version = SYN_DLT645_VER_1997;
    tx_frame.control = SYN_DLT645_CMD_READ_DATA;
    tx_frame.data_id = 0x9010;         /* 1997 2-byte DI */
    memset(tx_frame.address, 0x99, 6); /* Broadcast address */
    tx_frame.payload_len = 0;

    uint8_t encoded[64];
    size_t enc_len = syn_dlt645_encode(&tx_frame, encoded, sizeof(encoded));
    TEST_ASSERT_TRUE(enc_len > 0);

    SYN_DLT645_Frame rx_frame;
    SYN_Status status = syn_dlt645_parse(encoded, enc_len, SYN_DLT645_VER_1997, &rx_frame);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_CMD_READ_DATA, rx_frame.control);
    TEST_ASSERT_EQUAL_UINT32(0x9010, rx_frame.data_id);
}

/* ── Test: Preamble Stripping & Streaming Decoder ───────────────────────── */

static void test_dlt645_streaming_decoder(void)
{
    SYN_DLT645_Frame tx_frame;
    memset(&tx_frame, 0, sizeof(tx_frame));

    tx_frame.version = SYN_DLT645_VER_2007;
    tx_frame.control = SYN_DLT645_CMD_READ_DATA;
    tx_frame.data_id = 0x00010000;
    memset(tx_frame.address, 0x11, 6);

    uint8_t raw[64];
    size_t enc_len = syn_dlt645_encode(&tx_frame, raw, sizeof(raw));

    /* Prepend 4 preamble 0xFE bytes */
    uint8_t stream[80];
    stream[0] = 0xFE;
    stream[1] = 0xFE;
    stream[2] = 0xFE;
    stream[3] = 0xFE;
    memcpy(&stream[4], raw, enc_len);

    callback_count = 0;
    SYN_DLT645_Decoder dec;
    syn_dlt645_decoder_init(&dec, SYN_DLT645_VER_2007, on_frame_decoded, NULL);

    for (size_t i = 0; i < enc_len + 4; i++) {
        syn_dlt645_decoder_feed(&dec, stream[i]);
    }

    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_CMD_READ_DATA, last_decoded_frame.control);
}

/* ── Test: Corrupted & Short Frame Rejection ────────────────────────────── */

static void test_dlt645_error_handling(void)
{
    SYN_DLT645_Frame frame;
    uint8_t short_buf[] = {0x68, 0x11, 0x22};

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dlt645_parse(short_buf, sizeof(short_buf),
                                                              SYN_DLT645_VER_2007, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_dlt645_parse(NULL, 20, SYN_DLT645_VER_2007, &frame));

    /* Corrupted checksum */
    uint8_t bad_cs[] = {0x68, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x68,
                        0x11, 0x04, 0x33, 0x33, 0x33, 0x33, 0xFF, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dlt645_parse(bad_cs, sizeof(bad_cs), SYN_DLT645_VER_2007, &frame));

    /* Encode NULL & capacity errors */
    SYN_DLT645_Frame tx_frame;
    memset(&tx_frame, 0, sizeof(tx_frame));
    tx_frame.version = SYN_DLT645_VER_2007;

    TEST_ASSERT_EQUAL_INT(0, syn_dlt645_encode(NULL, bad_cs, sizeof(bad_cs)));
    TEST_ASSERT_EQUAL_INT(0, syn_dlt645_encode(&tx_frame, NULL, sizeof(bad_cs)));
    TEST_ASSERT_EQUAL_INT(0, syn_dlt645_encode(&tx_frame, bad_cs, 5));

    /* Parse with preambles in buffer directly */
    uint8_t preamble_buf[] = {0xFE, 0xFE, 0x68, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                              0x68, 0x11, 0x04, 0x33, 0x33, 0x33, 0x33, 0x17, 0x16};
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dlt645_parse(preamble_buf, sizeof(preamble_buf), SYN_DLT645_VER_2007, &frame));

    /* Oversized payload length in frame header (> 64 bytes) */
    uint8_t oversized_payload[120];
    memset(oversized_payload, 0, sizeof(oversized_payload));
    oversized_payload[0] = 0x68;
    oversized_payload[7] = 0x68;
    oversized_payload[9] = 100;    /* data_len = 100 > 64 */
    oversized_payload[111] = 0x16; /* EOF at expected_total - 1 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dlt645_parse(oversized_payload, sizeof(oversized_payload),
                                                      SYN_DLT645_VER_2007, &frame));

    /* Decoder NULL and invalid feeds */
    syn_dlt645_decoder_feed(NULL, 0x68);

    SYN_DLT645_Decoder dec;
    syn_dlt645_decoder_init(&dec, SYN_DLT645_VER_2007, NULL, NULL);
    syn_dlt645_decoder_feed(&dec, 0xAA); /* invalid start byte */
    TEST_ASSERT_EQUAL_INT(0, dec.rx_len);

    /* Fill decoder buffer to overflow */
    for (int i = 0; i < 128; i++) {
        syn_dlt645_decoder_feed(&dec, 0x68);
    }
    syn_dlt645_decoder_feed(&dec, 0xAA);
    TEST_ASSERT_EQUAL_INT(0, dec.rx_len);
}

static void test_dlt645_frame_checksum_and_address_mismatch(void)
{
    SYN_DLT645_Frame frame;

    /* 1. SOF1 / SOF2 mismatch */
    uint8_t bad_sof[15] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x68,
                           0x11, 0x04, 0x33, 0x33, 0x33, 0x33, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dlt645_parse(bad_sof, sizeof(bad_sof), SYN_DLT645_VER_2007, &frame));

    /* 2. data_len < di_len (e.g. data_len = 1 < 4) */
    uint8_t small_datalen[15] = {0x68, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x68,
                                 0x11, 0x01, 0x33, 0x33, 0x33, 0x33, 0x16};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dlt645_parse(small_datalen, sizeof(small_datalen),
                                                      SYN_DLT645_VER_2007, &frame));

    /* 3. rem < total_frame_len (truncated frame buffer) */
    uint8_t trunc_frame[12] = {0x68, 0x11, 0x11, 0x11, 0x11, 0x11,
                               0x11, 0x68, 0x11, 0x08, 0x33, 0x33};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dlt645_parse(trunc_frame, sizeof(trunc_frame),
                                                              SYN_DLT645_VER_2007, &frame));

    /* 4. EOF mismatch (0xAA instead of 0x16) */
    uint8_t bad_eof[16] = {0x68, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x68,
                           0x11, 0x04, 0x33, 0x33, 0x33, 0x33, 0x17, 0xAA};
    bad_eof[14] = syn_dlt645_calc_checksum(bad_eof, 14);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dlt645_parse(bad_eof, sizeof(bad_eof), SYN_DLT645_VER_2007, &frame));

    /* 5. Preambles leaving rem < 12 */
    uint8_t only_preambles[14] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
                                  0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_dlt645_parse(only_preambles, sizeof(only_preambles), SYN_DLT645_VER_2007, &frame));

    /* NULL parameter safety checks */
    TEST_ASSERT_EQUAL_UINT8(0, syn_dlt645_calc_checksum(NULL, 0));
    TEST_ASSERT_EQUAL_INT(0, syn_dlt645_encode(NULL, NULL, 0));

    /* 6. Oversized payload_bytes > 200 */
    uint8_t huge_datalen[225];
    memset(huge_datalen, 0x33, sizeof(huge_datalen));
    huge_datalen[0] = 0x68;
    huge_datalen[7] = 0x68;
    huge_datalen[8] = 0x11;
    huge_datalen[9] = 210; /* data_len = 210, payload_bytes = 210 - 4 = 206 > 200 */
    huge_datalen[220] = syn_dlt645_calc_checksum(huge_datalen, 220);
    huge_datalen[221] = 0x16;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dlt645_parse(huge_datalen, 222, SYN_DLT645_VER_2007, &frame));

    /* 7. Decoder reset on overflow with non-SOF start byte */
    SYN_DLT645_Decoder dec_overflow;
    syn_dlt645_decoder_init(&dec_overflow, SYN_DLT645_VER_2007, NULL, NULL);
    syn_dlt645_decoder_feed(&dec_overflow, 0x68);
    for (int i = 0; i < 130; i++) {
        syn_dlt645_decoder_feed(&dec_overflow, 0x11);
    }
    TEST_ASSERT_EQUAL_INT(0, dec_overflow.rx_len);

    /* 8. syn_dlt645_encode data_len > 255 guard */
    SYN_DLT645_Frame bad_enc_frame;
    memset(&bad_enc_frame, 0, sizeof(bad_enc_frame));
    bad_enc_frame.version = SYN_DLT645_VER_2007;
    bad_enc_frame.payload_len = 253;
    uint8_t enc_buf[300];
    TEST_ASSERT_EQUAL_size_t(0, syn_dlt645_encode(&bad_enc_frame, enc_buf, sizeof(enc_buf)));
}

static void test_dlt645_decoder_embedded_eof_and_null_cb(void)
{
    /* 1. Decoder with NULL callback */
    SYN_DLT645_Frame tx_frame;
    memset(&tx_frame, 0, sizeof(tx_frame));
    tx_frame.version = SYN_DLT645_VER_2007;
    tx_frame.control = SYN_DLT645_CMD_READ_DATA;
    tx_frame.data_id = 0x00010000;
    memset(tx_frame.address, 0x22, 6);

    uint8_t raw[64];
    size_t enc_len = syn_dlt645_encode(&tx_frame, raw, sizeof(raw));

    SYN_DLT645_Decoder dec_nocb;
    syn_dlt645_decoder_init(&dec_nocb, SYN_DLT645_VER_2007, NULL, NULL);
    for (size_t i = 0; i < enc_len; i++) {
        syn_dlt645_decoder_feed(&dec_nocb, raw[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, dec_nocb.rx_len); /* Resets on frame completion even with NULL cb */

    /* 2. Payload contains 0x16 (which after +0x33 becomes 0x49, but unencoded payload 0xE3 after
     * +0x33 is 0x16) */
    SYN_DLT645_Frame tx_embed;
    memset(&tx_embed, 0, sizeof(tx_embed));
    tx_embed.version = SYN_DLT645_VER_2007;
    tx_embed.control = SYN_DLT645_CMD_READ_DATA_RESP;
    tx_embed.data_id = 0x00010000;
    memset(tx_embed.address, 0x33, 6);
    tx_embed.payload[0] = 0xE3; /* 0xE3 + 0x33 = 0x16 (embedded EOF byte!) */
    tx_embed.payload[1] = 0x55;
    tx_embed.payload_len = 2;

    uint8_t embed_raw[64];
    size_t embed_len = syn_dlt645_encode(&tx_embed, embed_raw, sizeof(embed_raw));
    TEST_ASSERT_TRUE(embed_len > 0);

    callback_count = 0;
    SYN_DLT645_Decoder dec_embed;
    syn_dlt645_decoder_init(&dec_embed, SYN_DLT645_VER_2007, on_frame_decoded, NULL);
    for (size_t i = 0; i < embed_len; i++) {
        syn_dlt645_decoder_feed(&dec_embed, embed_raw[i]);
    }
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_UINT8(SYN_DLT645_CMD_READ_DATA_RESP, last_decoded_frame.control);
    TEST_ASSERT_EQUAL_UINT8(0xE3, last_decoded_frame.payload[0]);
}

static void test_r5_dlt645_bad_checksum_resync(void)
{
    SYN_DLT645_Frame good_tx;
    memset(&good_tx, 0, sizeof(good_tx));
    good_tx.version = SYN_DLT645_VER_2007;
    good_tx.control = SYN_DLT645_CMD_READ_DATA_RESP;
    good_tx.data_id = 0x00020000U;
    memset(good_tx.address, 0x44, SYN_DLT645_ADDR_LEN);
    good_tx.payload[0] = 0xAAU;
    good_tx.payload[1] = 0xBBU;
    good_tx.payload_len = 2U;

    uint8_t good_raw[64];
    size_t good_len = syn_dlt645_encode(&good_tx, good_raw, sizeof(good_raw));
    TEST_ASSERT_TRUE(good_len > 0U);

    /* 1. Bad-checksum 16-byte frame (has inner 0x68 at index 7) followed by valid frame */
    uint8_t bad_cs[] = {0x68U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x68U,
                        0x11U, 0x04U, 0x33U, 0x33U, 0x33U, 0x33U, 0xFFU, 0x16U};
    SYN_DLT645_Decoder dec;
    syn_dlt645_decoder_init(&dec, SYN_DLT645_VER_2007, on_frame_decoded, NULL);
    callback_count = 0;

    for (size_t i = 0U; i < sizeof(bad_cs); i++) {
        syn_dlt645_decoder_feed(&dec, bad_cs[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, callback_count);

    for (size_t i = 0U; i < good_len; i++) {
        syn_dlt645_decoder_feed(&dec, good_raw[i]);
    }
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_EQUAL_UINT32(0x00020000U, last_decoded_frame.data_id);
    TEST_ASSERT_EQUAL_UINT8(0xAAU, last_decoded_frame.payload[0]);

    /* 2. Spurious 0x68 + garbage prefix immediately preceding valid frame */
    callback_count = 0;
    uint8_t spurious_prefix[] = {0x68U, 0x99U, 0x88U};
    for (size_t i = 0U; i < sizeof(spurious_prefix); i++) {
        syn_dlt645_decoder_feed(&dec, spurious_prefix[i]);
    }
    for (size_t i = 0U; i < good_len; i++) {
        syn_dlt645_decoder_feed(&dec, good_raw[i]);
    }
    TEST_ASSERT_EQUAL_INT(1, callback_count);

    /* 3. Corrupt 12-byte frame with NO second 0x68 -> resets rx_len to 0 */
    uint8_t no_second_sof[12] = {0x68U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U,
                                 0x11U, 0x00U, 0x11U, 0x04U, 0x33U, 0x16U};
    for (size_t i = 0U; i < sizeof(no_second_sof); i++) {
        syn_dlt645_decoder_feed(&dec, no_second_sof[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, dec.rx_len);
}

void run_dlt645_tests(void)
{
    RUN_TEST(test_dlt645_checksum);
    RUN_TEST(test_dlt645_2007_roundtrip);
    RUN_TEST(test_dlt645_1997_roundtrip);
    RUN_TEST(test_dlt645_streaming_decoder);
    RUN_TEST(test_dlt645_error_handling);
    RUN_TEST(test_dlt645_frame_checksum_and_address_mismatch);
    RUN_TEST(test_dlt645_decoder_embedded_eof_and_null_cb);
    RUN_TEST(test_r5_dlt645_bad_checksum_resync);
}
