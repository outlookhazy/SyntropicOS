/**
 * @file test_isotp.c
 * @brief Unity unit tests for ISO 15765-2 (ISO-TP) CAN Transport Protocol.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_isotp.h"

static uint8_t rx_buf_a[512];
static uint8_t tx_buf_a[512];
static uint8_t rx_buf_b[512];
static uint8_t tx_buf_b[512];

static void test_isotp_single_frame(void)
{
    SYN_ISOTP_Link node_a, node_b;
    syn_isotp_init(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    uint8_t payload[] = { 0x02, 0x10, 0x01 }; /* UDS DiagnosticSessionControl request */
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload, sizeof(payload)));

    SYN_CAN_Frame frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&node_a, &frame));
    TEST_ASSERT_EQUAL(0x7E0, frame.id);
    TEST_ASSERT_EQUAL(0x03, frame.data[0]); /* SF, len = 3 */
    TEST_ASSERT_EQUAL(0x02, frame.data[1]);
    TEST_ASSERT_EQUAL(0x10, frame.data[2]);
    TEST_ASSERT_EQUAL(0x01, frame.data[3]);

    /* Ingest into node B */
    syn_isotp_process_rx_frame(&node_b, &frame);

    uint8_t out[16];
    TEST_ASSERT_EQUAL(3, syn_isotp_receive(&node_b, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(payload, out, 3);
}

static void test_isotp_multi_frame_flow(void)
{
    SYN_ISOTP_Link sender, receiver;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    uint8_t large_payload[20];
    for (size_t i = 0; i < sizeof(large_payload); i++) {
        large_payload[i] = (uint8_t)(i + 1);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_payload, sizeof(large_payload)));

    /* 1. Sender produces First Frame (FF) */
    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_EQUAL(0x7E0, ff_frame.id);
    TEST_ASSERT_EQUAL(0x10, ff_frame.data[0] & 0xF0); /* FF */
    TEST_ASSERT_EQUAL(20, ff_frame.data[1]);

    /* Receiver ingests FF */
    syn_isotp_process_rx_frame(&receiver, &ff_frame);

    /* 2. Receiver produces Flow Control (FC) frame */
    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc_frame));
    TEST_ASSERT_EQUAL(0x7E8, fc_frame.id);
    TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]); /* FC, CTS */

    /* Sender ingests FC */
    syn_isotp_process_rx_frame(&sender, &fc_frame);

    /* 3. Sender streams Consecutive Frames (CF) */
    while (sender.tx_state != SYN_ISOTP_TX_IDLE) {
        syn_isotp_step(&sender, 10);
        SYN_CAN_Frame cf_frame;
        if (syn_isotp_get_tx_frame(&sender, &cf_frame)) {
            syn_isotp_process_rx_frame(&receiver, &cf_frame);
        }
    }

    /* Receiver reads completed payload */
    uint8_t received[32];
    ssize_t res = syn_isotp_receive(&receiver, received, sizeof(received));
    TEST_ASSERT_EQUAL(20, res);
    TEST_ASSERT_EQUAL_MEMORY(large_payload, received, 20);
}

static void test_isotp_errors_and_edge_cases(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x100, 0x200, rx_buf_a, 16, tx_buf_a, 16);

    /* Parameter validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(NULL, rx_buf_a, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, NULL, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 5000));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 20)); /* Over buffer size */

    /* Send busy check */
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, rx_buf_a, 5));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_isotp_send(&link, rx_buf_a, 5));

    /* Receive when idle / invalid */
    uint8_t out[16];
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(NULL, out, sizeof(out)));
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, NULL, sizeof(out)));
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, out, sizeof(out)));

    /* Null guards */
    TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(NULL, NULL));
    syn_isotp_process_rx_frame(NULL, NULL);
    syn_isotp_step(NULL, 10);

    /* Flow Control overflow check */
    SYN_CAN_Frame ff_large = { .id = 0x100, .dlc = 8, .data = { 0x10, 0x40, 0, 0, 0, 0, 0, 0 } };
    syn_isotp_process_rx_frame(&link, &ff_large);
    SYN_CAN_Frame fc_overflow;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &fc_overflow));
    TEST_ASSERT_EQUAL(0x32, fc_overflow.data[0]); /* FC Overflow */

    /* Sequence mismatch check */
    link.rx_state = SYN_ISOTP_RX_WAIT_CF;
    link.rx_seq = 2;
    SYN_CAN_Frame cf_bad_seq = { .id = 0x100, .dlc = 8, .data = { 0x21, 0, 0, 0, 0, 0, 0, 0 } };
    syn_isotp_process_rx_frame(&link, &cf_bad_seq);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    /* Wrong CAN ID ignore */
    SYN_CAN_Frame wrong_id_frame = { .id = 0x999, .dlc = 8, .data = { 0x01, 0x10 } };
    syn_isotp_process_rx_frame(&link, &wrong_id_frame);

    /* Flow Control FC Overflow response from receiver to sender */
    link.tx_state = SYN_ISOTP_TX_WAIT_FC;
    SYN_CAN_Frame fc_overflow_rx = { .id = 0x100, .dlc = 8, .data = { 0x32, 0, 0 } };
    syn_isotp_process_rx_frame(&link, &fc_overflow_rx);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);

    /* STmin separation timer step */
    link.tx_st_timer = 20;
    syn_isotp_step(&link, 5);
    TEST_ASSERT_EQUAL(15, link.tx_st_timer);
    syn_isotp_step(&link, 20);
    TEST_ASSERT_EQUAL(0, link.tx_st_timer);

    /* Block Size limit (BS = 1) */
    syn_isotp_send(&link, (const uint8_t*)"1234567890123456", 16);
    SYN_CAN_Frame dummy_frame;
    syn_isotp_get_tx_frame(&link, &dummy_frame); /* FF */
    SYN_CAN_Frame fc_bs1 = { .id = 0x100, .dlc = 8, .data = { 0x30, 1, 0 } }; /* BS = 1 */
    syn_isotp_process_rx_frame(&link, &fc_bs1);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_SEND_CF, link.tx_state);
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &dummy_frame)); /* CF 1 */
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);
}

void run_isotp_tests(void)
{
    RUN_TEST(test_isotp_single_frame);
    RUN_TEST(test_isotp_multi_frame_flow);
    RUN_TEST(test_isotp_errors_and_edge_cases);
}
