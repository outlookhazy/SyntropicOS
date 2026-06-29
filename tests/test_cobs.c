/**
 * @file test_cobs.c
 * @brief Unity tests for syn_cobs.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_cobs.h"

static uint8_t cobs_rx_buf[256];
static size_t  cobs_rx_len = 0;

static void cobs_on_packet(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    memcpy(cobs_rx_buf, data, len);
    cobs_rx_len = len;
}

static void test_cobs(void)
{

    /* Encode / decode roundtrip */
    uint8_t orig[] = { 0x00, 0x11, 0x00, 0x00, 0x22, 0x33 };
    uint8_t encoded[16], decoded[16];
    size_t enc_len = syn_cobs_encode(orig, sizeof(orig), encoded);
    TEST_ASSERT_TRUE(enc_len > 0);

    size_t dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_EQUAL(sizeof(orig), dec_len);
    TEST_ASSERT_EQUAL_INT(0, memcmp(orig, decoded, sizeof(orig)));

    /* Simple data (no zeros) */
    uint8_t simple[] = { 0x01, 0x02, 0x03 };
    enc_len = syn_cobs_encode(simple, 3, encoded);
    dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_EQUAL_INT(3, dec_len);
    TEST_ASSERT_EQUAL_INT(0, memcmp(simple, decoded, 3));

    /* All zeros */
    uint8_t zeros[] = { 0x00, 0x00, 0x00 };
    enc_len = syn_cobs_encode(zeros, 3, encoded);
    dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_EQUAL_INT(3, dec_len);
    TEST_ASSERT_EQUAL_INT(0, memcmp(zeros, decoded, 3));

    /* Streaming decoder */
    SYN_COBS_Decoder dec;
    uint8_t stream_buf[128];
    syn_cobs_decoder_init(&dec, stream_buf, sizeof(stream_buf),
                           cobs_on_packet, NULL);

    /* Feed the encoded packet byte-by-byte, then delimiter */
    enc_len = syn_cobs_encode(simple, 3, encoded);
    cobs_rx_len = 0;
    size_t i;
    for (i = 0; i < enc_len; i++) {
        syn_cobs_decoder_feed(&dec, encoded[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, cobs_rx_len);

    syn_cobs_decoder_feed(&dec, 0x00); /* delimiter */
    TEST_ASSERT_EQUAL_INT(3, cobs_rx_len);
    TEST_ASSERT_EQUAL_INT(0, memcmp(cobs_rx_buf, simple, 3));
}

void run_cobs_tests(void)
{
    RUN_TEST(test_cobs);
}
