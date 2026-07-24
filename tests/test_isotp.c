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

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
static void test_isotp_canfd_single_frame(void)
{
    SYN_ISOTP_Link node_a, node_b;
    syn_isotp_init_fd(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a), true);
    syn_isotp_init_fd(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b), true);

    uint8_t payload[32];
    for (size_t i = 0; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(0xA0 + i);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload, sizeof(payload)));

    SYN_CAN_Frame frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&node_a, &frame));
    TEST_ASSERT_TRUE(frame.is_fd);
    TEST_ASSERT_EQUAL(0x7E0, frame.id);
    TEST_ASSERT_EQUAL(0x00, frame.data[0]); /* SF Escape */
    TEST_ASSERT_EQUAL(32, frame.data[1]);   /* SF_DL = 32 */
    TEST_ASSERT_EQUAL_MEMORY(payload, &frame.data[2], 32);

    /* Ingest into node B */
    syn_isotp_process_rx_frame(&node_b, &frame);

    uint8_t out[64];
    TEST_ASSERT_EQUAL(32, syn_isotp_receive(&node_b, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(payload, out, 32);
}

static void test_isotp_canfd_multi_frame(void)
{
    SYN_ISOTP_Link sender, receiver;
    syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a), true);
    syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b), true);

    uint8_t large_fd_payload[120];
    for (size_t i = 0; i < sizeof(large_fd_payload); i++) {
        large_fd_payload[i] = (uint8_t)(i + 1);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_fd_payload, sizeof(large_fd_payload)));

    /* 1. Sender produces CAN FD First Frame (64 bytes frame, 62 bytes payload) */
    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_TRUE(ff_frame.is_fd);
    TEST_ASSERT_EQUAL(64, ff_frame.dlc);
    TEST_ASSERT_EQUAL(0x10, ff_frame.data[0] & 0xF0); /* FF */
    TEST_ASSERT_EQUAL(120, ff_frame.data[1]);

    syn_isotp_process_rx_frame(&receiver, &ff_frame);

    /* 2. Receiver produces Flow Control (FC) frame */
    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc_frame));
    TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]);

    syn_isotp_process_rx_frame(&sender, &fc_frame);

    /* 3. Sender produces CAN FD Consecutive Frame (CF 1 with 58 bytes remaining) */
    SYN_CAN_Frame cf_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf_frame));
    TEST_ASSERT_TRUE(cf_frame.is_fd);
    TEST_ASSERT_EQUAL(0x21, cf_frame.data[0]); /* CF, seq = 1 */

    syn_isotp_process_rx_frame(&receiver, &cf_frame);

    /* Receiver reads assembled 120-byte payload */
    uint8_t received[256];
    ssize_t res = syn_isotp_receive(&receiver, received, sizeof(received));
    TEST_ASSERT_EQUAL(120, res);
static uint8_t large_tx_buf[5000];
static uint8_t large_rx_buf[5000];

static void test_isotp_canfd_extended_first_frame(void)
{
    SYN_ISOTP_Link sender, receiver;
    syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, large_rx_buf, sizeof(large_rx_buf), large_tx_buf, sizeof(large_tx_buf), true);
    syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, large_rx_buf, sizeof(large_rx_buf), large_tx_buf, sizeof(large_tx_buf), true);

    large_tx_buf[0] = 0xAA;
    large_tx_buf[4999] = 0x55;

    /* 5000 bytes payload triggers 32-bit Extended First Frame (> 4095) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_tx_buf, 5000));

    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_EQUAL(0x10, ff_frame.data[0]); /* FF Escape */
    TEST_ASSERT_EQUAL(0x00, ff_frame.data[1]);
    uint32_t ext_len = ((uint32_t)ff_frame.data[2] << 24) |
                       ((uint32_t)ff_frame.data[3] << 16) |
                       ((uint32_t)ff_frame.data[4] << 8)  |
                       (uint32_t)ff_frame.data[5];
    TEST_ASSERT_EQUAL_UINT32(5000, ext_len);

    /* Ingest 32-bit Extended FF into receiver */
    syn_isotp_process_rx_frame(&receiver, &ff_frame);
    TEST_ASSERT_TRUE(receiver.rx_fc_pending);
    TEST_ASSERT_EQUAL(SYN_ISOTP_FC_CTS, receiver.rx_fc_status);
}
#endif

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
    link.tx_st_timer_us = 20000;
    syn_isotp_step(&link, 5);
    TEST_ASSERT_EQUAL(15000, link.tx_st_timer_us);
    syn_isotp_step(&link, 20);
    TEST_ASSERT_EQUAL(0, link.tx_st_timer_us);

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

static void test_isotp_network_layer_timeouts(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x100, 0x200, rx_buf_a, 64, tx_buf_a, 64);
    syn_isotp_set_timeouts(&link, 500, 500); /* 500ms custom N_Bs & N_Cr */

    /* 1. N_Bs timeout check (Sender waiting for Flow Control) */
    uint8_t payload[20] = {1, 2, 3};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, payload, sizeof(payload)));
    SYN_CAN_Frame ff;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &ff)); /* FF sent, entering SYN_ISOTP_TX_WAIT_FC */
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

    syn_isotp_step(&link, 400); /* 400 ms passed */
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state); /* Still waiting */

    syn_isotp_step(&link, 150); /* Total 550 ms passed -> N_Bs timeout! */
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state); /* Aborted to IDLE */

    /* 2. N_Cr timeout check (Receiver waiting for Consecutive Frame) */
    SYN_CAN_Frame incoming_ff = { .id = 0x100, .dlc = 8, .data = { 0x10, 20, 1, 2, 3, 4, 5, 6 } };
    syn_isotp_process_rx_frame(&link, &incoming_ff);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state);

    syn_isotp_step(&link, 400); /* 400 ms passed */
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state); /* Still waiting */

    syn_isotp_step(&link, 150); /* Total 550 ms passed -> N_Cr timeout! */
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state); /* Aborted to IDLE */
}

void run_isotp_tests(void)
{
    RUN_TEST(test_isotp_single_frame);
    RUN_TEST(test_isotp_multi_frame_flow);
    RUN_TEST(test_isotp_errors_and_edge_cases);
    RUN_TEST(test_isotp_network_layer_timeouts);
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    RUN_TEST(test_isotp_canfd_single_frame);
    RUN_TEST(test_isotp_canfd_multi_frame);
    RUN_TEST(test_isotp_canfd_extended_first_frame);
#endif
}
