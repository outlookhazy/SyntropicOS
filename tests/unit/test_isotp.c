/**
 * @file test_isotp.c
 * @brief Unity unit tests for ISO 15765-2 (ISO-TP) CAN Transport Protocol.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_isotp.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static uint8_t rx_buf_a[512];
static uint8_t tx_buf_a[512];
static uint8_t rx_buf_b[512];
static uint8_t tx_buf_b[512];

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
static void test_isotp_canfd_single_frame(void)
{
    SYN_ISOTP_Link node_a, node_b;
    syn_isotp_init_fd(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a),
                      true);
    syn_isotp_init_fd(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b),
                      true);

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
    syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a),
                      true);
    syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                      sizeof(tx_buf_b), true);

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

    /* 2. Receiver produces Flow Control (FC) frame with BS = 8 (0x08) */
    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc_frame));
    TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]);
    TEST_ASSERT_EQUAL(0x08, fc_frame.data[1]); /* BS = 8 default */
    TEST_ASSERT_EQUAL(0x00, fc_frame.data[2]); /* STmin = 0 default */

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
        syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, large_rx_buf, sizeof(large_rx_buf), large_tx_buf,
                          sizeof(large_tx_buf), true);
        syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, large_rx_buf, sizeof(large_rx_buf), large_tx_buf,
                          sizeof(large_tx_buf), true);

        large_tx_buf[0] = 0xAA;
        large_tx_buf[4999] = 0x55;

        /* 5000 bytes payload triggers 32-bit Extended First Frame (> 4095) */
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_tx_buf, 5000));

        SYN_CAN_Frame ff_frame;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
        TEST_ASSERT_EQUAL(0x10, ff_frame.data[0]); /* FF Escape */
        TEST_ASSERT_EQUAL(0x00, ff_frame.data[1]);
        uint32_t ext_len = ((uint32_t)ff_frame.data[2] << 24) | ((uint32_t)ff_frame.data[3] << 16) |
                           ((uint32_t)ff_frame.data[4] << 8) | (uint32_t)ff_frame.data[5];
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
        syn_isotp_init(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        syn_isotp_init(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                       sizeof(tx_buf_b));

        uint8_t payload[] = {0x02, 0x10, 0x01}; /* UDS DiagnosticSessionControl request */
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

        /* CAN FD 8-bit single frame test (data[0] = 0x00, data[1] = len) */
        SYN_CAN_Frame fd_frame = {0};
        fd_frame.id = 0x7E0;
        fd_frame.dlc = 8;
        fd_frame.data[0] = 0x00;
        fd_frame.data[1] = 0x03;
        fd_frame.data[2] = 0x02;
        fd_frame.data[3] = 0x10;
        fd_frame.data[4] = 0x01;
        syn_isotp_process_rx_frame(&node_b, &fd_frame);
        TEST_ASSERT_EQUAL(3, syn_isotp_receive(&node_b, out, sizeof(out)));
    }

    static void test_isotp_multi_frame_flow(void)
    {
        SYN_ISOTP_Link sender, receiver;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        syn_isotp_init(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                       sizeof(tx_buf_b));

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
        TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                          syn_isotp_send(&link, rx_buf_a, 20)); /* Over buffer size */

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
        SYN_CAN_Frame ff_large = {.id = 0x100, .dlc = 8, .data = {0x10, 0x40, 0, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&link, &ff_large);
        SYN_CAN_Frame fc_overflow;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &fc_overflow));
        TEST_ASSERT_EQUAL(0x32, fc_overflow.data[0]); /* FC Overflow */

        /* Sequence mismatch check */
        link.rx_state = SYN_ISOTP_RX_WAIT_CF;
        link.rx_seq = 2;
        SYN_CAN_Frame cf_bad_seq = {.id = 0x100, .dlc = 8, .data = {0x21, 0, 0, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&link, &cf_bad_seq);
        TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

        /* Wrong CAN ID ignore */
        SYN_CAN_Frame wrong_id_frame = {.id = 0x999, .dlc = 8, .data = {0x01, 0x10}};
        syn_isotp_process_rx_frame(&link, &wrong_id_frame);

        /* Flow Control FC Overflow response from receiver to sender */
        link.tx_state = SYN_ISOTP_TX_WAIT_FC;
        SYN_CAN_Frame fc_overflow_rx = {.id = 0x100, .dlc = 8, .data = {0x32, 0, 0}};
        syn_isotp_process_rx_frame(&link, &fc_overflow_rx);
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);

        /* STmin separation timer step */
        link.tx_st_timer_us = 20000;
        syn_isotp_step(&link, 5);
        TEST_ASSERT_EQUAL(15000, link.tx_st_timer_us);
        syn_isotp_step(&link, 20);
        TEST_ASSERT_EQUAL(0, link.tx_st_timer_us);

        /* Block Size limit (BS = 1) */
        syn_isotp_send(&link, (const uint8_t *)"1234567890123456", 16);
        SYN_CAN_Frame dummy_frame;
        syn_isotp_get_tx_frame(&link, &dummy_frame);                          /* FF */
        SYN_CAN_Frame fc_bs1 = {.id = 0x100, .dlc = 8, .data = {0x30, 1, 0}}; /* BS = 1 */
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
        TEST_ASSERT_TRUE(
            syn_isotp_get_tx_frame(&link, &ff)); /* FF sent, entering SYN_ISOTP_TX_WAIT_FC */
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

        syn_isotp_step(&link, 400);                             /* 400 ms passed */
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state); /* Still waiting */

        syn_isotp_step(&link, 150); /* Total 550 ms passed -> N_Bs timeout! */
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state); /* Aborted to IDLE */
        TEST_ASSERT_TRUE(syn_isotp_is_tx_idle(NULL));

        /* 2. N_Cr timeout check (Receiver waiting for Consecutive Frame) */
        SYN_CAN_Frame incoming_ff = {.id = 0x100, .dlc = 8, .data = {0x10, 20, 1, 2, 3, 4, 5, 6}};
        syn_isotp_process_rx_frame(&link, &incoming_ff);
        TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state);

        syn_isotp_step(&link, 400);                             /* 400 ms passed */
        TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state); /* Still waiting */

        syn_isotp_step(&link, 150); /* Total 550 ms passed -> N_Cr timeout! */
        TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state); /* Aborted to IDLE */
    }

    static void test_isotp_full_duplex(void)
    {
        SYN_ISOTP_Link node_a, node_b;
        syn_isotp_init(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        syn_isotp_init(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                       sizeof(tx_buf_b));

        uint8_t payload_a_to_b[30];
        uint8_t payload_b_to_a[40];

        for (size_t i = 0; i < sizeof(payload_a_to_b); i++)
            payload_a_to_b[i] = (uint8_t)(0x10 + i);
        for (size_t i = 0; i < sizeof(payload_b_to_a); i++)
            payload_b_to_a[i] = (uint8_t)(0x80 + i);

        /* Initiate simultaneous bi-directional multi-frame transfers */
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload_a_to_b, sizeof(payload_a_to_b)));
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_b, payload_b_to_a, sizeof(payload_b_to_a)));

        /* Interleave frames bi-directionally until both complete */
        for (int step = 0; step < 50; step++) {
            syn_isotp_step(&node_a, 5);
            syn_isotp_step(&node_b, 5);

            SYN_CAN_Frame frame_a, frame_b;
            bool has_a = syn_isotp_get_tx_frame(&node_a, &frame_a);
            bool has_b = syn_isotp_get_tx_frame(&node_b, &frame_b);

            if (has_a) {
                syn_isotp_process_rx_frame(&node_b, &frame_a);
            }
            if (has_b) {
                syn_isotp_process_rx_frame(&node_a, &frame_b);
            }

            if (!has_a && !has_b && node_a.tx_state == SYN_ISOTP_TX_IDLE &&
                node_b.tx_state == SYN_ISOTP_TX_IDLE) {
                break;
            }
        }

        /* Verify Node B received complete message from Node A */
        uint8_t recv_b[64];
        ssize_t len_b = syn_isotp_receive(&node_b, recv_b, sizeof(recv_b));
        TEST_ASSERT_EQUAL(30, len_b);
        TEST_ASSERT_EQUAL_MEMORY(payload_a_to_b, recv_b, 30);

        /* Verify Node A received complete message from Node B */
        uint8_t recv_a[64];
        ssize_t len_a = syn_isotp_receive(&node_a, recv_a, sizeof(recv_a));
        TEST_ASSERT_EQUAL(40, len_a);
        TEST_ASSERT_EQUAL_MEMORY(payload_b_to_a, recv_a, 40);
    }

    static void test_isotp_wait_fc_state(void)
    {
        SYN_ISOTP_Link sender;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));

        uint8_t payload[20] = {0};
        syn_isotp_send(&sender, payload, sizeof(payload));

        SYN_CAN_Frame ff;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

        /* While waiting for FC, get_tx_frame returns false */
        SYN_CAN_Frame dummy;
        TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(&sender, &dummy));
    }

    static void test_isotp_flow_control_overflow_rejection(void)
    {
        SYN_ISOTP_Link sender;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));

        uint8_t payload[20] = {0};
        syn_isotp_send(&sender, payload, sizeof(payload));

        SYN_CAN_Frame ff;
        syn_isotp_get_tx_frame(&sender, &ff);

        /* Ingest FC with status = SYN_ISOTP_FC_OVERFLOW (0x32), CAN ID = 0x7E8 (rx_id) */
        SYN_CAN_Frame fc_overflow = {.id = 0x7E8, .dlc = 8, .data = {0x32, 0, 0, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&sender, &fc_overflow);

        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, sender.tx_state);
    }

    static void test_isotp_null_receive_and_large_len_clamping(void)
    {
        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

        uint8_t out[16];
        TEST_ASSERT_EQUAL(-1, syn_isotp_receive(NULL, out, sizeof(out)));
        TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, NULL, sizeof(out)));

        /* Clamping test: set link.rx_state = COMPLETE and rx_len = 40000 */
        link.rx_state = SYN_ISOTP_RX_COMPLETE;
        link.rx_len = 40000;
        uint8_t large_out[64];
        ssize_t ret = syn_isotp_receive(&link, large_out, sizeof(large_out));
        TEST_ASSERT_EQUAL(64, ret);
    }

    static void test_isotp_stmin_decoding_and_null_timeouts(void)
    {
        syn_isotp_set_timeouts(NULL, 100, 100);

        SYN_ISOTP_Link sender;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        uint8_t payload[20] = {0};
        syn_isotp_send(&sender, payload, sizeof(payload));

        /* FC with STmin in 0xF1..0xF9 range (0xF5 = 500 us timer) */
        sender.tx_state = SYN_ISOTP_TX_WAIT_FC;
        SYN_CAN_Frame fc_stmin = {.id = 0x7E8, .dlc = 8, .data = {0x30, 0, 0xF5, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&sender, &fc_stmin);
        TEST_ASSERT_EQUAL_UINT8(0xF5, sender.tx_st_min);

        SYN_CAN_Frame cf_frame;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf_frame));

        /* FC with STmin out of valid bounds (0x85) */
        sender.tx_state = SYN_ISOTP_TX_WAIT_FC;
        SYN_CAN_Frame fc_invalid = {.id = 0x7E8, .dlc = 8, .data = {0x30, 0, 0x85, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&sender, &fc_invalid);
        TEST_ASSERT_EQUAL_UINT8(0x85, sender.tx_st_min);
    }

    static void test_isotp_tx_send_cf_zero_remaining_idle(void)
    {
        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x700, 0x708, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

        link.tx_state = SYN_ISOTP_TX_SEND_CF;
        link.tx_len = 10;
        link.tx_offset = 10;

        SYN_CAN_Frame frame;
        TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(&link, &frame));
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);

        /* Invalid frame type (top nibble 4) default case */
        SYN_CAN_Frame invalid_frame = {.id = 0x708, .dlc = 8, .data = {0x40, 0, 0, 0, 0, 0, 0, 0}};
        syn_isotp_process_rx_frame(&link, &invalid_frame);
    }

    static void test_isotp_32bit_extended_first_frame_parsing(void)
    {
        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x700, 0x708, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

        SYN_CAN_Frame frame = {
            .id = 0x700, .dlc = 8, .data = {0x10, 0x00, 0x00, 0x00, 0x00, 0x50, 0xAA, 0xBB}};
        syn_isotp_process_rx_frame(&link, &frame);
        TEST_ASSERT_EQUAL(80, link.rx_expected);
        TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state);
    }

    static void test_isotp_is_tx_idle_helper(void)
    {
        TEST_ASSERT_TRUE(syn_isotp_is_tx_idle(NULL));

        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x700, 0x708, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
        TEST_ASSERT_TRUE(syn_isotp_is_tx_idle(&link));

        uint8_t payload[32] = {0};
        syn_isotp_send(&link, payload, sizeof(payload));
        TEST_ASSERT_FALSE(syn_isotp_is_tx_idle(&link));
    }

    static void test_isotp_tx_consecutive_frame_flow_control(void)
    {
        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x700, 0x708, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

        /* 1. tx_st_timer_us > 0 in SYN_ISOTP_TX_SEND_CF (line 232) */
        link.tx_state = SYN_ISOTP_TX_SEND_CF;
        link.tx_st_timer_us = 5000;
        SYN_CAN_Frame frame;
        TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(&link, &frame));

        /* 2. Reserved STmin value (e.g. 0x80) (line 53) */
        link.tx_st_timer_us = 0;
        link.tx_st_min = 0x80;
        link.tx_len = 10;
        link.tx_offset = 2;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &frame));
        TEST_ASSERT_EQUAL_UINT32(127000, link.tx_st_timer_us);

        /* 3. syn_isotp_set_fc_params coverage (lines 88-93) */
        syn_isotp_set_fc_params(NULL, 0, 0);
        syn_isotp_set_fc_params(&link, 4, 10);
        TEST_ASSERT_EQUAL_UINT8(4, link.rx_fc_bs);
        TEST_ASSERT_EQUAL_UINT8(10, link.rx_fc_stmin);

        /* 4. Single frame sf_len > max_sf_bytes clamping (line 322) */
        SYN_CAN_Frame sf_over = {0};
        sf_over.id = 0x700;
        sf_over.dlc = 8;
        sf_over.data[0] = 0x0F; /* Single frame with sf_len = 15 > 7 max bytes */
        syn_isotp_process_rx_frame(&link, &sf_over);
    }

    static void test_isotp_block_size_flow_control(void)
    {
        SYN_ISOTP_Link sender, receiver;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        syn_isotp_init(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                       sizeof(tx_buf_b));

        /* Configure receiver with BS = 2 */
        syn_isotp_set_fc_params(&receiver, 2, 0);

        uint8_t payload[25];
        for (size_t i = 0; i < sizeof(payload); i++) {
            payload[i] = (uint8_t)(i + 1);
        }

        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, payload, sizeof(payload)));

        /* 1. First Frame (FF) from sender */
        SYN_CAN_Frame ff;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
        syn_isotp_process_rx_frame(&receiver, &ff);

        /* 2. Initial Flow Control (FC) from receiver */
        SYN_CAN_Frame fc1;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc1));
        TEST_ASSERT_EQUAL(2, fc1.data[1]); /* BS = 2 */
        syn_isotp_process_rx_frame(&sender, &fc1);

        /* 3. Sender transmits CF 1 and CF 2 (Block 1) */
        SYN_CAN_Frame cf1, cf2;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf1));
        syn_isotp_process_rx_frame(&receiver, &cf1);

        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf2));
        syn_isotp_process_rx_frame(&receiver, &cf2);

        /* 4. Receiver has reached BS=2 threshold -> triggers intermediate FC CTS frame */
        SYN_CAN_Frame fc2;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc2));
        TEST_ASSERT_EQUAL(0x30, fc2.data[0]); /* FC CTS */
        syn_isotp_process_rx_frame(&sender, &fc2);

        /* 5. Sender transmits remaining CF 3 (Block 2) */
        SYN_CAN_Frame cf3;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf3));
        syn_isotp_process_rx_frame(&receiver, &cf3);

        /* Assembly check */
        uint8_t out[32];
        TEST_ASSERT_EQUAL(25, syn_isotp_receive(&receiver, out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(payload, out, 25);
    }

    static void test_isotp_tx_wait_flow_control(void)
    {
        SYN_ISOTP_Link sender;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        syn_isotp_set_timeouts(&sender, 1000, 1000); /* 1000ms N_Bs */

        uint8_t payload[20] = {0};
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, payload, sizeof(payload)));

        SYN_CAN_Frame ff;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

        /* 1. Receive FC(WAIT) frame from receiver (FS = 0x01) */
        SYN_CAN_Frame fc_wait = {
            .id = 0x7E8, .dlc = 8, .data = {0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        syn_isotp_process_rx_frame(&sender, &fc_wait);

        /* Advance time by 600ms */
        syn_isotp_step(&sender, 600);
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

        /* 2. Receive another FC(WAIT) frame — should reset the 1000ms timer */
        syn_isotp_process_rx_frame(&sender, &fc_wait);

        /* Advance time by another 600ms (total 1200ms elapsed since send start) */
        syn_isotp_step(&sender, 600);
        /* With FC_WAIT resetting the timer, sender remains in WAIT_FC */
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

        /* 3. Receive FC(CTS) frame (FS = 0x00) */
        SYN_CAN_Frame fc_cts = {
            .id = 0x7E8, .dlc = 8, .data = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        syn_isotp_process_rx_frame(&sender, &fc_cts);
        TEST_ASSERT_EQUAL(SYN_ISOTP_TX_SEND_CF, sender.tx_state);

        /* Verify CF can be retrieved */
        SYN_CAN_Frame cf;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf));
        TEST_ASSERT_EQUAL(0x21, cf.data[0]); /* CF seq 1 */
    }

    static void test_isotp_frame_padding(void)
    {
        SYN_ISOTP_Link link;
        syn_isotp_init(&link, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

        /* 1. Default padding value 0xCC for Single Frame */
        TEST_ASSERT_EQUAL_HEX8(SYN_ISOTP_DEFAULT_PADDING, link.padding_byte);
        uint8_t sf_payload[3] = {0x02, 0x10, 0x01};
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, sf_payload, sizeof(sf_payload)));

        SYN_CAN_Frame sf_frame;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &sf_frame));
        TEST_ASSERT_EQUAL(0x03, sf_frame.data[0]);
        TEST_ASSERT_EQUAL(0x02, sf_frame.data[1]);
        TEST_ASSERT_EQUAL(0x10, sf_frame.data[2]);
        TEST_ASSERT_EQUAL(0x01, sf_frame.data[3]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, sf_frame.data[4]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, sf_frame.data[5]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, sf_frame.data[6]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, sf_frame.data[7]);

        /* 2. Custom padding value configured via syn_isotp_set_padding */
        syn_isotp_set_padding(&link, 0xAA);
        TEST_ASSERT_EQUAL_HEX8(0xAA, link.padding_byte);
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, sf_payload, sizeof(sf_payload)));
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &sf_frame));
        TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[4]);
        TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[5]);
        TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[6]);
        TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[7]);

        /* 3. Null guard for syn_isotp_set_padding */
        syn_isotp_set_padding(NULL, 0x55);

        /* 4. Flow Control (FC) frame padding */
        syn_isotp_init(&link, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
        SYN_CAN_Frame incoming_ff = {.id = 0x7E8, .dlc = 8, .data = {0x10, 20, 1, 2, 3, 4, 5, 6}};
        syn_isotp_process_rx_frame(&link, &incoming_ff);
        TEST_ASSERT_TRUE(link.rx_fc_pending);

        SYN_CAN_Frame fc_frame;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &fc_frame));
        TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]); /* FC CTS */
        TEST_ASSERT_EQUAL(0x00, fc_frame.data[1]); /* BS = 0 */
        TEST_ASSERT_EQUAL(0x00, fc_frame.data[2]); /* STmin = 0 */
        TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[3]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[4]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[5]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[6]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[7]);

        /* 5. Last Consecutive Frame (CF) padding */
        SYN_ISOTP_Link sender;
        syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a,
                       sizeof(tx_buf_a));
        uint8_t mf_payload[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, mf_payload, sizeof(mf_payload)));

        SYN_CAN_Frame ff_out;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_out)); /* FF: 6 payload bytes */

        SYN_CAN_Frame fc_cts_in = {
            .id = 0x7E8, .dlc = 8, .data = {0x30, 0, 0, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC}};
        syn_isotp_process_rx_frame(&sender, &fc_cts_in);

        SYN_CAN_Frame cf_out;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(
            &sender, &cf_out));                  /* Last CF: 4 payload bytes + PCI = 5 bytes used */
        TEST_ASSERT_EQUAL(0x21, cf_out.data[0]); /* CF seq 1 */
        TEST_ASSERT_EQUAL(7, cf_out.data[1]);
        TEST_ASSERT_EQUAL(8, cf_out.data[2]);
        TEST_ASSERT_EQUAL(9, cf_out.data[3]);
        TEST_ASSERT_EQUAL(10, cf_out.data[4]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[5]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[6]);
        TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[7]);
    }

    void run_isotp_tests(void)
    {
        RUN_TEST(test_isotp_single_frame);
        RUN_TEST(test_isotp_multi_frame_flow);
        RUN_TEST(test_isotp_full_duplex);
        RUN_TEST(test_isotp_errors_and_edge_cases);
        RUN_TEST(test_isotp_network_layer_timeouts);
        RUN_TEST(test_isotp_wait_fc_state);
        RUN_TEST(test_isotp_flow_control_overflow_rejection);
        RUN_TEST(test_isotp_null_receive_and_large_len_clamping);
        RUN_TEST(test_isotp_stmin_decoding_and_null_timeouts);
        RUN_TEST(test_isotp_tx_send_cf_zero_remaining_idle);
        RUN_TEST(test_isotp_32bit_extended_first_frame_parsing);
        RUN_TEST(test_isotp_is_tx_idle_helper);
        RUN_TEST(test_isotp_tx_consecutive_frame_flow_control);
        RUN_TEST(test_isotp_block_size_flow_control);
        RUN_TEST(test_isotp_tx_wait_flow_control);
        RUN_TEST(test_isotp_frame_padding);
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
        RUN_TEST(test_isotp_canfd_single_frame);
        RUN_TEST(test_isotp_canfd_multi_frame);
        RUN_TEST(test_isotp_canfd_extended_first_frame);
#endif
    }
