/**
 * @file test_ymodem.c
 * @brief Unity unit tests for syn_ymodem.h / syn_ymodem.c.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_ymodem.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_crc.h"
#include "unity/unity.h"

#include <string.h>

#define MOCK_STREAM_CAPACITY 8192U

static uint8_t mock_tx_buf[MOCK_STREAM_CAPACITY];
static size_t mock_tx_len = 0;

static uint8_t mock_rx_stream[MOCK_STREAM_CAPACITY];
static size_t mock_rx_head = 0;
static size_t mock_rx_tail = 0;

static SYN_YMODEM_Event last_event;
static uint8_t event_data_copy[2048];
static size_t event_data_len = 0;
static int event_count = 0;

static void mock_putchar(uint8_t b, void *ctx)
{
    (void)ctx;
    if (mock_tx_len < MOCK_STREAM_CAPACITY) {
        mock_tx_buf[mock_tx_len++] = b;
    }
}

static int mock_getchar(uint32_t timeout_ms, void *ctx)
{
    (void)ctx;
    (void)timeout_ms;
    if (mock_rx_head < mock_rx_tail) {
        return mock_rx_stream[mock_rx_head++];
    }
    return -1; /* Timeout */
}

static int mock_event_cb(SYN_YMODEM_Event event, const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    last_event = event;
    event_count++;
    if (data != NULL && len > 0) {
        if (len > sizeof(event_data_copy)) {
            len = sizeof(event_data_copy);
        }
        memcpy(event_data_copy, data, len);
        event_data_len = len;
    } else {
        event_data_len = 0;
    }
    return 0;
}

static void mock_reset_stream(void)
{
    mock_tx_len = 0;
    mock_rx_head = 0;
    mock_rx_tail = 0;
    last_event = (SYN_YMODEM_Event)-1;
    event_data_len = 0;
    event_count = 0;
}

static void mock_feed_byte(uint8_t b)
{
    if (mock_rx_tail < MOCK_STREAM_CAPACITY) {
        mock_rx_stream[mock_rx_tail++] = b;
    }
}

static void mock_feed_packet(uint8_t header, uint8_t seq, const uint8_t *payload,
                             size_t payload_len)
{
    mock_feed_byte(header);
    mock_feed_byte(seq);
    mock_feed_byte((uint8_t)(~seq));
    for (size_t i = 0; i < payload_len; i++) {
        mock_feed_byte(payload[i]);
    }
    uint16_t crc = syn_crc16_ccitt_update(0x0000U, payload, payload_len);
    mock_feed_byte((uint8_t)(crc >> 8));
    mock_feed_byte((uint8_t)(crc & 0xFFU));
}

void test_ymodem_init_and_null_checks(void)
{
    SYN_YMODEM_Receiver rx;
    syn_ymodem_receiver_init(NULL, mock_putchar, mock_getchar, mock_event_cb, NULL);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);

    TEST_ASSERT_EQUAL_PTR(mock_putchar, rx.putchar_fn);
    TEST_ASSERT_EQUAL_PTR(mock_getchar, rx.getchar_fn);

    /* NULL receiver / callbacks */
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_PARAM, syn_ymodem_receive(NULL));

    SYN_YMODEM_Receiver rx_bad;
    syn_ymodem_receiver_init(&rx_bad, NULL, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_PARAM, syn_ymodem_receive(&rx_bad));

    SYN_YMODEM_Receiver rx_null_gc;
    syn_ymodem_receiver_init(&rx_null_gc, mock_putchar, NULL, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_PARAM, syn_ymodem_receive(&rx_null_gc));
}

void test_ymodem_single_file_transfer(void)
{
    mock_reset_stream();

    /* Build Block 0: filename "fw.bin\0128 1000\0" */
    uint8_t block0[128];
    memset(block0, 0, sizeof(block0));
    memcpy(block0, "fw.bin", 7);
    memcpy(&block0[7], "128 1000", 8);

    /* Block 1: 128 bytes data */
    uint8_t data_block[128];
    for (int i = 0; i < 128; i++) {
        data_block[i] = (uint8_t)i;
    }

    /* Feed Block 0 */
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);

    /* Feed Data Block 1 */
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);

    /* Feed EOT sequence: 1st EOT, 2nd EOT */
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);

    /* Feed Empty Block 0 to end session */
    uint8_t empty_block0[128];
    memset(empty_block0, 0, sizeof(empty_block0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty_block0, 128);

    SYN_YMODEM_Receiver rx;
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    SYN_YMODEM_Status st = syn_ymodem_receive(&rx);

    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, st);
    TEST_ASSERT_EQUAL_STRING("fw.bin", rx.filename);
    TEST_ASSERT_EQUAL_UINT32(128, rx.filesize);
    TEST_ASSERT_EQUAL_UINT32(128, rx.bytes_received);
    TEST_ASSERT_TRUE(mock_tx_len > 0);
}

void test_ymodem_1024b_block_transfer(void)
{
    mock_reset_stream();

    /* Block 0 with 1024-byte filesize */
    uint8_t block0[128];
    memset(block0, 0, sizeof(block0));
    memcpy(block0, "app.bin", 8);
    memcpy(&block0[8], "1024 1000", 9);

    uint8_t data1024[1024];
    for (int i = 0; i < 1024; i++) {
        data1024[i] = (uint8_t)(i & 0xFF);
    }

    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_packet(SYN_YMODEM_STX, 0x01, data1024, 1024);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);

    uint8_t empty0[128];
    memset(empty0, 0, sizeof(empty0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);

    SYN_YMODEM_Receiver rx;
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    SYN_YMODEM_Status st = syn_ymodem_receive(&rx);

    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, st);
    TEST_ASSERT_EQUAL_STRING("app.bin", rx.filename);
    TEST_ASSERT_EQUAL_UINT32(1024, rx.filesize);
}

void test_ymodem_crc_error_and_timeout(void)
{
    mock_reset_stream();

    /* Timeout on initial request */
    SYN_YMODEM_Receiver rx;
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_TIMEOUT, syn_ymodem_receive(&rx));

    /* CRC error packet */
    mock_reset_stream();
    uint8_t bad_pkt[133];
    bad_pkt[0] = SYN_YMODEM_SOH;
    bad_pkt[1] = 0x00;
    bad_pkt[2] = 0xFF;
    memset(&bad_pkt[3], 0x55, 128);
    bad_pkt[131] = 0xDE; /* Bad CRC */
    bad_pkt[132] = 0xAD;

    for (int i = 0; i < 133; i++) {
        mock_feed_byte(bad_pkt[i]);
    }

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CRC, syn_ymodem_receive(&rx));
}

void test_ymodem_cancel(void)
{
    mock_reset_stream();
    mock_feed_byte(SYN_YMODEM_CAN);

    SYN_YMODEM_Receiver rx;
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CANCEL, syn_ymodem_receive(&rx));
}

static int cancelling_event_cb(SYN_YMODEM_Event event, const uint8_t *data, size_t len, void *ctx)
{
    (void)data;
    (void)len;
    (void)ctx;
    if (event == SYN_YMODEM_EVENT_FILE_START) {
        return -1; /* Reject FILE_START */
    }
    return 0;
}

static int cancelling_data_event_cb(SYN_YMODEM_Event event, const uint8_t *data, size_t len,
                                    void *ctx)
{
    (void)data;
    (void)len;
    (void)ctx;
    if (event == SYN_YMODEM_EVENT_DATA) {
        return -1; /* Reject DATA */
    }
    return 0;
}

void test_ymodem_edge_cases(void)
{
    SYN_YMODEM_Receiver rx;

    /* Invalid Block 0 header byte (e.g. 0xFF instead of SOH/STX) */
    mock_reset_stream();
    mock_feed_byte(0xFF);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_SEQUENCE, syn_ymodem_receive(&rx));

    /* Block 0 with invalid sequence number (0x05 instead of 0x00) */
    mock_reset_stream();
    uint8_t block0[128];
    memset(block0, 0, sizeof(block0));
    memcpy(block0, "test.bin", 8);
    mock_feed_packet(SYN_YMODEM_SOH, 0x05, block0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_SEQUENCE, syn_ymodem_receive(&rx));

    /* Sequence number complement mismatch */
    mock_reset_stream();
    mock_feed_byte(SYN_YMODEM_SOH);
    mock_feed_byte(0x00);
    mock_feed_byte(0x00); /* Mismatch: should be 0xFF */
    for (int i = 0; i < 130; i++) {
        mock_feed_byte(0x00);
    }
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_SEQUENCE, syn_ymodem_receive(&rx));

    /* Callback rejection on FILE_START */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, cancelling_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CANCEL, syn_ymodem_receive(&rx));

    /* Double CAN sequence during data transfer */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_byte(SYN_YMODEM_CAN);
    mock_feed_byte(SYN_YMODEM_CAN);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CANCEL, syn_ymodem_receive(&rx));

    /* Out of order data block sequence number */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_packet(SYN_YMODEM_SOH, 0x05, block0, 128); /* Expected 1, got 5 */
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_SEQUENCE, syn_ymodem_receive(&rx));

    /* Duplicate packet retransmission (seq 0 again when expected 1) */
    mock_reset_stream();
    uint8_t data_block[128];
    memset(data_block, 0xAA, 128);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128); /* Duplicate seq 0 */
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    uint8_t empty0[128] = {0};
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));

    /* Truncated payload test (filesize = 50 bytes in 128-byte block) */
    mock_reset_stream();
    uint8_t block0_tr[128] = {0};
    memcpy(block0_tr, "short.bin", 10);
    memcpy(&block0_tr[10], "50 1000", 7);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0_tr, 128);
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(0x00); /* Non-EOT response after NAK */
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));

    /* Data block with invalid header byte then valid block */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_byte(0xFF); /* Invalid data block header byte */
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));

    /* Data block with bad CRC then valid block */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_byte(SYN_YMODEM_SOH);
    mock_feed_byte(0x01);
    mock_feed_byte(0xFE);
    for (int i = 0; i < 128; i++)
        mock_feed_byte(0x55);
    mock_feed_byte(0xBA);
    mock_feed_byte(0xDD); /* Bad CRC */
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));

    /* Callback rejection on DATA event */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0, 128);
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, cancelling_data_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CANCEL, syn_ymodem_receive(&rx));

    /* Partial packet timeout during payload read */
    mock_reset_stream();
    mock_feed_byte(SYN_YMODEM_SOH);
    mock_feed_byte(0x00);
    mock_feed_byte(0xFF);
    mock_feed_byte(0x55); /* Partial byte; timeout on remaining 129 bytes */
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_TIMEOUT, syn_ymodem_receive(&rx));

    /* Block 0 with filename only and no file size string */
    mock_reset_stream();
    uint8_t block0_nosize[128] = {0};
    memcpy(block0_nosize, "nosize.bin", 11); /* NULL terminated, no size string after */
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, block0_nosize, 128);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, 128);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_EQUAL_STRING("nosize.bin", rx.filename);
}

static void test_r5_ymodem_regressions(void)
{
    SYN_YMODEM_Receiver rx;
    uint8_t empty0[128] = {0};
    uint8_t data_block[128];
    memset(data_block, 0x3CU, sizeof(data_block));

    /* 1. R5-S14: >= 64B filename must truncate into rx.filename without corrupting rx.filesize */
    mock_reset_stream();
    uint8_t long_fn_blk0[128];
    memset(long_fn_blk0, 0, sizeof(long_fn_blk0));
    /* 80-char filename ('a' * 76 + ".bin") followed by '\0' and "  128 1000" */
    memset(long_fn_blk0, 'a', 76U);
    memcpy(&long_fn_blk0[76], ".bin", 4U);
    long_fn_blk0[80] = '\0';
    memcpy(&long_fn_blk0[81], "  128 1000", 10U);

    mock_feed_packet(SYN_YMODEM_SOH, 0x00, long_fn_blk0, sizeof(long_fn_blk0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, sizeof(data_block));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, sizeof(empty0));

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(SYN_YMODEM_MAX_FILENAME - 1U),
                             (uint32_t)strlen(rx.filename));
    TEST_ASSERT_EQUAL_UINT32(128U, rx.filesize);
    TEST_ASSERT_EQUAL_UINT32(128U, rx.bytes_received);

    /* 2. R5-S14: Block 0 without any NUL terminator (bounded scan, no OOB strtoul) */
    mock_reset_stream();
    uint8_t nonul_blk0[128];
    memset(nonul_blk0, '9', sizeof(nonul_blk0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, nonul_blk0, sizeof(nonul_blk0));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, sizeof(empty0));

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_EQUAL_UINT32(0U, rx.filesize);

    /* 3. R5-S14: Oversized ASCII filesize saturates at UINT32_MAX without UB */
    mock_reset_stream();
    uint8_t huge_sz_blk0[128];
    memset(huge_sz_blk0, 0, sizeof(huge_sz_blk0));
    memcpy(huge_sz_blk0, "huge.bin", 8U);
    memcpy(&huge_sz_blk0[9], "999999999999999", 15U);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, huge_sz_blk0, sizeof(huge_sz_blk0));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, sizeof(empty0));

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, rx.filesize);

    /* 4. R5-S14: Repeated data NAKs hit SYN_YMODEM_MAX_RETRIES and abort with CAN, CAN */
    mock_reset_stream();
    uint8_t valid_blk0[128] = {0};
    memcpy(valid_blk0, "retry.bin", 10U);
    memcpy(&valid_blk0[10], "64", 2U);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, valid_blk0, sizeof(valid_blk0));
    for (uint8_t r = 0U; r < SYN_YMODEM_MAX_RETRIES; r++) {
        mock_feed_byte(0xFFU);
    }
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_CRC, syn_ymodem_receive(&rx));
    TEST_ASSERT_TRUE(mock_tx_len >= 2U);
    TEST_ASSERT_EQUAL_HEX8(SYN_YMODEM_CAN, mock_tx_buf[mock_tx_len - 2U]);
    TEST_ASSERT_EQUAL_HEX8(SYN_YMODEM_CAN, mock_tx_buf[mock_tx_len - 1U]);

    /* 5. Timeout during data header wait, timeout mid-data-block, single CAN glitch, & extra data
     * past filesize */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, valid_blk0, sizeof(valid_blk0));
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_TIMEOUT, syn_ymodem_receive(&rx));

    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, valid_blk0, sizeof(valid_blk0));
    mock_feed_byte(SYN_YMODEM_SOH);
    mock_feed_byte(0x01U);
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_ERR_TIMEOUT, syn_ymodem_receive(&rx));

    /* Single CAN followed by non-CAN byte + extra block after filesize already met */
    mock_reset_stream();
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, valid_blk0, sizeof(valid_blk0));
    mock_feed_byte(SYN_YMODEM_CAN);
    mock_feed_byte(0x7FU);
    mock_feed_packet(SYN_YMODEM_SOH, 0x01, data_block, sizeof(data_block));
    mock_feed_packet(SYN_YMODEM_SOH, 0x02, data_block, sizeof(data_block));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00, empty0, sizeof(empty0));
    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_EQUAL_UINT32(64U, rx.bytes_received);
}

static void test_tier_a_ymodem_explicit_zero_byte_file(void)
{
    SYN_YMODEM_Receiver rx;
    uint8_t empty0[128] = {0};
    uint8_t pad_block[128];
    memset(pad_block, 0x1AU, sizeof(pad_block));

    /* Block 0 specifies explicit "0" file size: padding SOH block must write 0 bytes! */
    mock_reset_stream();
    uint8_t zero_blk0[128] = {0};
    memcpy(zero_blk0, "empty.bin", 10U);
    memcpy(&zero_blk0[10], "0 1700000000", 12U);

    mock_feed_packet(SYN_YMODEM_SOH, 0x00U, zero_blk0, sizeof(zero_blk0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x01U, pad_block, sizeof(pad_block));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00U, empty0, sizeof(empty0));

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_TRUE(rx.has_file_size);
    TEST_ASSERT_EQUAL_UINT32(0U, rx.filesize);
    TEST_ASSERT_EQUAL_UINT32(0U, rx.bytes_received);
    /* Only FILE_START, FILE_END, and SESSION_END events (zero DATA events!) */
    TEST_ASSERT_EQUAL_INT(3, event_count);

    /* Contrast: Block 0 with NO file size string still accepts full data blocks */
    mock_reset_stream();
    uint8_t nosize_blk0[128] = {0};
    memcpy(nosize_blk0, "raw.bin", 8U);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00U, nosize_blk0, sizeof(nosize_blk0));
    mock_feed_packet(SYN_YMODEM_SOH, 0x01U, pad_block, sizeof(pad_block));
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_byte(SYN_YMODEM_EOT);
    mock_feed_packet(SYN_YMODEM_SOH, 0x00U, empty0, sizeof(empty0));

    syn_ymodem_receiver_init(&rx, mock_putchar, mock_getchar, mock_event_cb, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_YMODEM_OK, syn_ymodem_receive(&rx));
    TEST_ASSERT_FALSE(rx.has_file_size);
    TEST_ASSERT_EQUAL_UINT32(128U, rx.bytes_received);
    TEST_ASSERT_EQUAL_INT(4, event_count);
}

void run_ymodem_tests(void)
{
    RUN_TEST(test_ymodem_init_and_null_checks);
    RUN_TEST(test_ymodem_single_file_transfer);
    RUN_TEST(test_ymodem_1024b_block_transfer);
    RUN_TEST(test_ymodem_crc_error_and_timeout);
    RUN_TEST(test_ymodem_cancel);
    RUN_TEST(test_ymodem_edge_cases);
    RUN_TEST(test_r5_ymodem_regressions);
    RUN_TEST(test_tier_a_ymodem_explicit_zero_byte_file);
}
