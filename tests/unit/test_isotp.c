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
static uint8_t huge_rx_buf[32769];
static uint8_t huge_out_buf[32769];

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
static uint8_t large_tx_buf[5000];
static uint8_t large_rx_buf[5000];

static void test_isotp_canfd_single_frame(void)
{
    SYN_ISOTP_Link node_a;
    SYN_ISOTP_Link node_b;
    syn_isotp_init_fd(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a),
                      true);
    syn_isotp_init_fd(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b),
                      true);

    /* Exercise all 8 ISO 11898-1 CAN-FD DLC padding buckets (<=8, 12, 16, 20, 24, 32, 48, 64) */
    const size_t sf_sizes[] = {3U, 9U, 12U, 16U, 20U, 28U, 32U, 50U};
    const uint8_t expected_dlcs[] = {4U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};

    uint8_t payload[62];
    for (size_t i = 0; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(0xA0U + i);
    }

    for (size_t idx = 0; idx < sizeof(sf_sizes) / sizeof(sf_sizes[0]); idx++) {
        size_t len = sf_sizes[idx];
        TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload, len));

        SYN_CAN_Frame frame;
        TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&node_a, &frame));
        TEST_ASSERT_TRUE(frame.is_fd);
        TEST_ASSERT_EQUAL_UINT8(expected_dlcs[idx], frame.dlc);
        TEST_ASSERT_EQUAL_UINT32(0x7E0U, frame.id);

        if (len <= 7U) {
            TEST_ASSERT_EQUAL_UINT8((uint8_t)len, frame.data[0]);
            TEST_ASSERT_EQUAL_MEMORY(payload, &frame.data[1], len);
        } else {
            TEST_ASSERT_EQUAL_UINT8(0x00U, frame.data[0]);
            TEST_ASSERT_EQUAL_UINT8((uint8_t)len, frame.data[1]);
            TEST_ASSERT_EQUAL_MEMORY(payload, &frame.data[2], len);
        }

        syn_isotp_process_rx_frame(&node_b, &frame);
        uint8_t out[64];
        TEST_ASSERT_EQUAL((ssize_t)len, syn_isotp_receive(&node_b, out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(payload, out, len);
    }
}

static void test_isotp_canfd_multi_frame(void)
{
    SYN_ISOTP_Link sender;
    SYN_ISOTP_Link receiver;
    syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a),
                      true);
    syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b,
                      sizeof(tx_buf_b), true);

    /* 150-byte FD transfer exercises FF (62B) + CF1 (rem=88 > 63 -> 63B) + CF2 (rem=25 <= 63) */
    uint8_t large_fd_payload[150];
    for (size_t i = 0; i < sizeof(large_fd_payload); i++) {
        large_fd_payload[i] = (uint8_t)(i + 1U);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_fd_payload, sizeof(large_fd_payload)));

    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_TRUE(ff_frame.is_fd);
    TEST_ASSERT_EQUAL_UINT8(64U, ff_frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(0x10U, ff_frame.data[0] & 0xF0U);
    TEST_ASSERT_EQUAL_UINT8(150U, ff_frame.data[1]);

    syn_isotp_process_rx_frame(&receiver, &ff_frame);

    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc_frame));
    TEST_ASSERT_EQUAL_UINT8(0x30U, fc_frame.data[0]);

    syn_isotp_process_rx_frame(&sender, &fc_frame);

    SYN_CAN_Frame cf1_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf1_frame));
    TEST_ASSERT_TRUE(cf1_frame.is_fd);
    TEST_ASSERT_EQUAL_UINT8(64U, cf1_frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(0x21U, cf1_frame.data[0]);
    syn_isotp_process_rx_frame(&receiver, &cf1_frame);

    SYN_CAN_Frame cf2_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf2_frame));
    TEST_ASSERT_TRUE(cf2_frame.is_fd);
    TEST_ASSERT_EQUAL_UINT8(32U, cf2_frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(0x22U, cf2_frame.data[0]);
    syn_isotp_process_rx_frame(&receiver, &cf2_frame);

    uint8_t received[256];
    ssize_t res = syn_isotp_receive(&receiver, received, sizeof(received));
    TEST_ASSERT_EQUAL(150, res);
    TEST_ASSERT_EQUAL_MEMORY(large_fd_payload, received, sizeof(large_fd_payload));
}

static void test_isotp_canfd_extended_first_frame(void)
{
    SYN_ISOTP_Link sender;
    SYN_ISOTP_Link receiver;
    syn_isotp_init_fd(&sender, 0x7E8, 0x7E0, large_rx_buf, sizeof(large_rx_buf), large_tx_buf,
                      sizeof(large_tx_buf), true);
    syn_isotp_init_fd(&receiver, 0x7E0, 0x7E8, large_rx_buf, sizeof(large_rx_buf), large_tx_buf,
                      sizeof(large_tx_buf), true);

    large_tx_buf[0] = 0xAAU;
    large_tx_buf[4999] = 0x55U;

    /* 5000 bytes payload triggers 32-bit Extended First Frame (> 4095) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_tx_buf, 5000U));

    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_EQUAL_UINT8(0x10U, ff_frame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, ff_frame.data[1]);
    uint32_t ext_len = ((uint32_t)ff_frame.data[2] << 24) | ((uint32_t)ff_frame.data[3] << 16) |
                       ((uint32_t)ff_frame.data[4] << 8) | (uint32_t)ff_frame.data[5];
    TEST_ASSERT_EQUAL_UINT32(5000U, ext_len);

    /* Ingest 32-bit Extended FF into receiver */
    syn_isotp_process_rx_frame(&receiver, &ff_frame);
    TEST_ASSERT_TRUE(receiver.rx_fc_pending);
    TEST_ASSERT_EQUAL(SYN_ISOTP_FC_CTS, receiver.rx_fc_status);

    /* Also exercise payload_in_ff > ff_len defensive clamp (FF DLC=64, ff_len=20) */
    SYN_CAN_Frame oversized_ff = {0};
    oversized_ff.id = 0x7E0U;
    oversized_ff.dlc = 64U;
    oversized_ff.is_fd = true;
    oversized_ff.data[0] = 0x10U;
    oversized_ff.data[1] = 20U;
    syn_isotp_process_rx_frame(&receiver, &oversized_ff);
    TEST_ASSERT_EQUAL_UINT32(20U, (uint32_t)receiver.rx_len);
}
#endif

static void test_isotp_single_frame(void)
{
    SYN_ISOTP_Link node_a;
    SYN_ISOTP_Link node_b;
    syn_isotp_init(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    uint8_t payload[] = {0x02, 0x10, 0x01};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload, sizeof(payload)));

    SYN_CAN_Frame frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&node_a, &frame));
    TEST_ASSERT_EQUAL(0x7E0, frame.id);
    TEST_ASSERT_EQUAL(0x03, frame.data[0]);
    TEST_ASSERT_EQUAL(0x02, frame.data[1]);
    TEST_ASSERT_EQUAL(0x10, frame.data[2]);
    TEST_ASSERT_EQUAL(0x01, frame.data[3]);

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
    SYN_ISOTP_Link sender;
    SYN_ISOTP_Link receiver;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    uint8_t large_payload[20];
    for (size_t i = 0; i < sizeof(large_payload); i++) {
        large_payload[i] = (uint8_t)(i + 1);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, large_payload, sizeof(large_payload)));

    SYN_CAN_Frame ff_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_frame));
    TEST_ASSERT_EQUAL(0x7E0, ff_frame.id);
    TEST_ASSERT_EQUAL(0x10, ff_frame.data[0] & 0xF0);
    TEST_ASSERT_EQUAL(20, ff_frame.data[1]);

    syn_isotp_process_rx_frame(&receiver, &ff_frame);

    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc_frame));
    TEST_ASSERT_EQUAL(0x7E8, fc_frame.id);
    TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]);

    syn_isotp_process_rx_frame(&sender, &fc_frame);

    while (sender.tx_state != SYN_ISOTP_TX_IDLE) {
        syn_isotp_step(&sender, 10);
        SYN_CAN_Frame cf_frame;
        if (syn_isotp_get_tx_frame(&sender, &cf_frame)) {
            syn_isotp_process_rx_frame(&receiver, &cf_frame);
        }
    }

    uint8_t received[32];
    ssize_t res = syn_isotp_receive(&receiver, received, sizeof(received));
    TEST_ASSERT_EQUAL(20, res);
    TEST_ASSERT_EQUAL_MEMORY(large_payload, received, 20);
}

static void test_isotp_errors_and_edge_cases(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x100, 0x200, rx_buf_a, 16, tx_buf_a, 16);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(NULL, rx_buf_a, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, NULL, 10));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 5000));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, rx_buf_a, 20));

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, rx_buf_a, 5));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_isotp_send(&link, rx_buf_a, 5));

    uint8_t out[16];
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(NULL, out, sizeof(out)));
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, NULL, sizeof(out)));
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, out, sizeof(out)));

    TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(NULL, NULL));
    syn_isotp_process_rx_frame(NULL, NULL);
    syn_isotp_step(NULL, 10);

    SYN_CAN_Frame ff_large = {.id = 0x100, .dlc = 8, .data = {0x10, 0x40, 0, 0, 0, 0, 0, 0}};
    syn_isotp_process_rx_frame(&link, &ff_large);
    SYN_CAN_Frame fc_overflow;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &fc_overflow));
    TEST_ASSERT_EQUAL(0x32, fc_overflow.data[0]);

    link.rx_state = SYN_ISOTP_RX_WAIT_CF;
    link.rx_seq = 2;
    SYN_CAN_Frame cf_bad_seq = {.id = 0x100, .dlc = 8, .data = {0x21, 0, 0, 0, 0, 0, 0, 0}};
    syn_isotp_process_rx_frame(&link, &cf_bad_seq);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame wrong_id_frame = {.id = 0x999, .dlc = 8, .data = {0x01, 0x10}};
    syn_isotp_process_rx_frame(&link, &wrong_id_frame);

    link.tx_state = SYN_ISOTP_TX_WAIT_FC;
    SYN_CAN_Frame fc_overflow_rx = {.id = 0x100, .dlc = 8, .data = {0x32, 0, 0}};
    syn_isotp_process_rx_frame(&link, &fc_overflow_rx);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);

    link.tx_st_timer_us = 20000;
    syn_isotp_step(&link, 5);
    TEST_ASSERT_EQUAL(15000, link.tx_st_timer_us);
    syn_isotp_step(&link, 20);
    TEST_ASSERT_EQUAL(0, link.tx_st_timer_us);

    syn_isotp_send(&link, (const uint8_t *)"1234567890123456", 16);
    SYN_CAN_Frame dummy_frame;
    syn_isotp_get_tx_frame(&link, &dummy_frame);
    SYN_CAN_Frame fc_bs1 = {.id = 0x100, .dlc = 8, .data = {0x30, 1, 0}};
    syn_isotp_process_rx_frame(&link, &fc_bs1);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_SEND_CF, link.tx_state);
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &dummy_frame));
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);
}

static void test_isotp_network_layer_timeouts(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x100, 0x200, rx_buf_a, 64, tx_buf_a, 64);
    syn_isotp_set_timeouts(&link, 500, 500);

    uint8_t payload[20] = {1, 2, 3};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, payload, sizeof(payload)));
    SYN_CAN_Frame ff;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &ff));
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

    syn_isotp_step(&link, 400);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

    syn_isotp_step(&link, 150);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);
    TEST_ASSERT_TRUE(syn_isotp_is_tx_idle(NULL));

    SYN_CAN_Frame incoming_ff = {.id = 0x100, .dlc = 8, .data = {0x10, 20, 1, 2, 3, 4, 5, 6}};
    syn_isotp_process_rx_frame(&link, &incoming_ff);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state);

    syn_isotp_step(&link, 400);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_WAIT_CF, link.rx_state);

    syn_isotp_step(&link, 150);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);
}

static void test_isotp_full_duplex(void)
{
    SYN_ISOTP_Link node_a;
    SYN_ISOTP_Link node_b;
    syn_isotp_init(&node_a, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&node_b, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    uint8_t payload_a_to_b[30];
    uint8_t payload_b_to_a[40];

    for (size_t i = 0; i < sizeof(payload_a_to_b); i++) {
        payload_a_to_b[i] = (uint8_t)(0x10 + i);
    }
    for (size_t i = 0; i < sizeof(payload_b_to_a); i++) {
        payload_b_to_a[i] = (uint8_t)(0x80 + i);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_a, payload_a_to_b, sizeof(payload_a_to_b)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&node_b, payload_b_to_a, sizeof(payload_b_to_a)));

    for (int step = 0; step < 50; step++) {
        syn_isotp_step(&node_a, 5);
        syn_isotp_step(&node_b, 5);

        SYN_CAN_Frame frame_a;
        SYN_CAN_Frame frame_b;
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

    uint8_t recv_b[64];
    ssize_t len_b = syn_isotp_receive(&node_b, recv_b, sizeof(recv_b));
    TEST_ASSERT_EQUAL(30, len_b);
    TEST_ASSERT_EQUAL_MEMORY(payload_a_to_b, recv_b, 30);

    uint8_t recv_a[64];
    ssize_t len_a = syn_isotp_receive(&node_a, recv_a, sizeof(recv_a));
    TEST_ASSERT_EQUAL(40, len_a);
    TEST_ASSERT_EQUAL_MEMORY(payload_b_to_a, recv_a, 40);
}

static void test_isotp_wait_fc_state(void)
{
    SYN_ISOTP_Link sender;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

    uint8_t payload[20] = {0};
    syn_isotp_send(&sender, payload, sizeof(payload));

    SYN_CAN_Frame ff;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

    SYN_CAN_Frame dummy;
    TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(&sender, &dummy));
}

static void test_isotp_flow_control_overflow_rejection(void)
{
    SYN_ISOTP_Link sender;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

    uint8_t payload[20] = {0};
    syn_isotp_send(&sender, payload, sizeof(payload));

    SYN_CAN_Frame ff;
    syn_isotp_get_tx_frame(&sender, &ff);

    SYN_CAN_Frame fc_overflow = {.id = 0x7E8, .dlc = 8, .data = {0x32, 0, 0, 0, 0, 0, 0, 0}};
    syn_isotp_process_rx_frame(&sender, &fc_overflow);

    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, sender.tx_state);
}

static void test_isotp_null_receive_and_large_len_clamping(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x7E8, 0x7E0, huge_rx_buf, sizeof(huge_rx_buf), tx_buf_a,
                   sizeof(tx_buf_a));

    uint8_t out[16];
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(NULL, out, sizeof(out)));
    TEST_ASSERT_EQUAL(-1, syn_isotp_receive(&link, NULL, sizeof(out)));

    /* Clamping to max_len when max_len < rx_len */
    link.rx_state = SYN_ISOTP_RX_COMPLETE;
    link.rx_len = 256U;
    uint8_t small_out[64];
    TEST_ASSERT_EQUAL(64, syn_isotp_receive(&link, small_out, sizeof(small_out)));

    /* Clamping > 32767U defensive 16-bit ssize_t cast guard */
    link.rx_state = SYN_ISOTP_RX_COMPLETE;
    link.rx_len = 32769U;
    ssize_t ret = syn_isotp_receive(&link, huge_out_buf, sizeof(huge_out_buf));
    TEST_ASSERT_EQUAL(32767, ret);
}

static void test_isotp_stmin_decoding_and_null_timeouts(void)
{
    syn_isotp_set_timeouts(NULL, 100, 100);

    SYN_ISOTP_Link sender;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    uint8_t payload[20] = {0};
    syn_isotp_send(&sender, payload, sizeof(payload));

    sender.tx_state = SYN_ISOTP_TX_WAIT_FC;
    SYN_CAN_Frame fc_stmin = {.id = 0x7E8, .dlc = 8, .data = {0x30, 0, 0xF5, 0, 0, 0, 0, 0}};
    syn_isotp_process_rx_frame(&sender, &fc_stmin);
    TEST_ASSERT_EQUAL_UINT8(0xF5, sender.tx_st_min);

    SYN_CAN_Frame cf_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf_frame));

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

    SYN_CAN_Frame invalid_frame = {.id = 0x700, .dlc = 8, .data = {0x40, 0, 0, 0, 0, 0, 0, 0}};
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

    link.tx_state = SYN_ISOTP_TX_SEND_CF;
    link.tx_st_timer_us = 5000;
    SYN_CAN_Frame frame;
    TEST_ASSERT_FALSE(syn_isotp_get_tx_frame(&link, &frame));

    link.tx_st_timer_us = 0;
    link.tx_st_min = 0x80;
    link.tx_len = 10;
    link.tx_offset = 2;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &frame));
    TEST_ASSERT_EQUAL_UINT32(127000, link.tx_st_timer_us);

    syn_isotp_set_fc_params(NULL, 0, 0);
    syn_isotp_set_fc_params(&link, 4, 10);
    TEST_ASSERT_EQUAL_UINT8(4, link.rx_fc_bs);
    TEST_ASSERT_EQUAL_UINT8(10, link.rx_fc_stmin);

    SYN_CAN_Frame sf_over = {0};
    sf_over.id = 0x700;
    sf_over.dlc = 8;
    sf_over.data[0] = 0x0F;
    syn_isotp_process_rx_frame(&link, &sf_over);
}

static void test_isotp_block_size_flow_control(void)
{
    SYN_ISOTP_Link sender;
    SYN_ISOTP_Link receiver;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_init(&receiver, 0x7E0, 0x7E8, rx_buf_b, sizeof(rx_buf_b), tx_buf_b, sizeof(tx_buf_b));

    syn_isotp_set_fc_params(&receiver, 2, 0);

    uint8_t payload[25];
    for (size_t i = 0; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(i + 1);
    }

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, payload, sizeof(payload)));

    SYN_CAN_Frame ff;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
    syn_isotp_process_rx_frame(&receiver, &ff);

    SYN_CAN_Frame fc1;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc1));
    TEST_ASSERT_EQUAL(2, fc1.data[1]);
    syn_isotp_process_rx_frame(&sender, &fc1);

    SYN_CAN_Frame cf1;
    SYN_CAN_Frame cf2;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf1));
    syn_isotp_process_rx_frame(&receiver, &cf1);

    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf2));
    syn_isotp_process_rx_frame(&receiver, &cf2);

    SYN_CAN_Frame fc2;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&receiver, &fc2));
    TEST_ASSERT_EQUAL(0x30, fc2.data[0]);
    syn_isotp_process_rx_frame(&sender, &fc2);

    SYN_CAN_Frame cf3;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf3));
    syn_isotp_process_rx_frame(&receiver, &cf3);

    uint8_t out[32];
    TEST_ASSERT_EQUAL(25, syn_isotp_receive(&receiver, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(payload, out, 25);
}

static void test_isotp_tx_wait_flow_control(void)
{
    SYN_ISOTP_Link sender;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    syn_isotp_set_timeouts(&sender, 1000, 1000);

    uint8_t payload[20] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, payload, sizeof(payload)));

    SYN_CAN_Frame ff;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff));
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

    SYN_CAN_Frame fc_wait = {
        .id = 0x7E8, .dlc = 8, .data = {0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    syn_isotp_process_rx_frame(&sender, &fc_wait);

    syn_isotp_step(&sender, 600);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

    syn_isotp_process_rx_frame(&sender, &fc_wait);

    syn_isotp_step(&sender, 600);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, sender.tx_state);

    SYN_CAN_Frame fc_cts = {
        .id = 0x7E8, .dlc = 8, .data = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    syn_isotp_process_rx_frame(&sender, &fc_cts);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_SEND_CF, sender.tx_state);

    SYN_CAN_Frame cf;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf));
    TEST_ASSERT_EQUAL(0x21, cf.data[0]);
}

static void test_isotp_frame_padding(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));

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

    syn_isotp_set_padding(&link, 0xAA);
    TEST_ASSERT_EQUAL_HEX8(0xAA, link.padding_byte);
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, sf_payload, sizeof(sf_payload)));
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &sf_frame));
    TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[4]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[5]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[6]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, sf_frame.data[7]);

    syn_isotp_set_padding(NULL, 0x55);

    syn_isotp_init(&link, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    SYN_CAN_Frame incoming_ff = {.id = 0x7E8, .dlc = 8, .data = {0x10, 20, 1, 2, 3, 4, 5, 6}};
    syn_isotp_process_rx_frame(&link, &incoming_ff);
    TEST_ASSERT_TRUE(link.rx_fc_pending);

    SYN_CAN_Frame fc_frame;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &fc_frame));
    TEST_ASSERT_EQUAL(0x30, fc_frame.data[0]);
    TEST_ASSERT_EQUAL(0x00, fc_frame.data[1]);
    TEST_ASSERT_EQUAL(0x00, fc_frame.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[3]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[4]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[5]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[6]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, fc_frame.data[7]);

    SYN_ISOTP_Link sender;
    syn_isotp_init(&sender, 0x7E8, 0x7E0, rx_buf_a, sizeof(rx_buf_a), tx_buf_a, sizeof(tx_buf_a));
    uint8_t mf_payload[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&sender, mf_payload, sizeof(mf_payload)));

    SYN_CAN_Frame ff_out;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &ff_out));

    SYN_CAN_Frame fc_cts_in = {
        .id = 0x7E8, .dlc = 8, .data = {0x30, 0, 0, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC}};
    syn_isotp_process_rx_frame(&sender, &fc_cts_in);

    SYN_CAN_Frame cf_out;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&sender, &cf_out));
    TEST_ASSERT_EQUAL(0x21, cf_out.data[0]);
    TEST_ASSERT_EQUAL(7, cf_out.data[1]);
    TEST_ASSERT_EQUAL(8, cf_out.data[2]);
    TEST_ASSERT_EQUAL(9, cf_out.data[3]);
    TEST_ASSERT_EQUAL(10, cf_out.data[4]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[5]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[6]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, cf_out.data[7]);
}

static uint8_t pro15_classic_tx[4100];
static uint8_t pro15_classic_rx[4100];

static void test_isotp_pro15_frame_validation_and_saturation(void)
{
    SYN_ISOTP_Link link;
    syn_isotp_init(&link, 0x700U, 0x708U, pro15_classic_rx, sizeof(pro15_classic_rx),
                   pro15_classic_tx, sizeof(pro15_classic_tx));

    /* 1. Classic CAN send > 4095 bytes rejected even when tx_buf_size >= 4096 */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_isotp_send(&link, pro15_classic_tx, 4096U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, pro15_classic_tx, 4095U));
    link.tx_state = SYN_ISOTP_TX_IDLE;

    /* 2. dlc == 0 ignored */
    SYN_CAN_Frame zero_dlc = {.id = 0x700U, .dlc = 0U, .data = {0x02U, 0x11U, 0x22U}};
    syn_isotp_process_rx_frame(&link, &zero_dlc);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    /* 3. Malformed Single Frames dropped instead of clamped */
    SYN_CAN_Frame sf_dlc1_esc = {.id = 0x700U, .dlc = 1U, .data = {0x00U}};
    syn_isotp_process_rx_frame(&link, &sf_dlc1_esc);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame sf_zero_esc = {.id = 0x700U, .dlc = 8U, .data = {0x00U, 0x00U, 0xAAU}};
    syn_isotp_process_rx_frame(&link, &sf_zero_esc);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame sf_esc_over = {.id = 0x700U, .dlc = 8U, .data = {0x00U, 0x07U, 0xAAU}};
    syn_isotp_process_rx_frame(&link, &sf_esc_over);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame sf_short_dlc = {.id = 0x700U, .dlc = 4U, .data = {0x05U, 1U, 2U, 3U}};
    syn_isotp_process_rx_frame(&link, &sf_short_dlc);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_ISOTP_Link tiny_link;
    syn_isotp_init(&tiny_link, 0x700U, 0x708U, rx_buf_a, 2U, tx_buf_a, 8U);
    SYN_CAN_Frame sf_over_buf = {.id = 0x700U, .dlc = 8U, .data = {0x03U, 1U, 2U, 3U}};
    syn_isotp_process_rx_frame(&tiny_link, &sf_over_buf);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, tiny_link.rx_state);

    /* 4. Malformed First Frames (dlc < 8, ff_len <= 7, FD escape <= 4095) and idle CF ignored */
    SYN_CAN_Frame ff_short = {.id = 0x700U, .dlc = 7U, .data = {0x10U, 0x20U, 1U, 2U, 3U, 4U, 5U}};
    syn_isotp_process_rx_frame(&link, &ff_short);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame ff_tiny_len = {.id = 0x700U, .dlc = 8U, .data = {0x10U, 0x05U, 1U, 2U}};
    syn_isotp_process_rx_frame(&link, &ff_tiny_len);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

    SYN_CAN_Frame cf_idle = {.id = 0x700U, .dlc = 8U, .data = {0x21U, 1U}};
    syn_isotp_process_rx_frame(&link, &cf_idle);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, link.rx_state);

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    SYN_ISOTP_Link fd_link;
    syn_isotp_init_fd(&fd_link, 0x700U, 0x708U, pro15_classic_rx, sizeof(pro15_classic_rx),
                      pro15_classic_tx, sizeof(pro15_classic_tx), true);
    SYN_CAN_Frame fd_bad_esc = {.id = 0x700U,
                                .dlc = 64U,
                                .is_fd = true,
                                .data = {0x10U, 0x00U, 0x00U, 0x00U, 0x0FU, 0xFFU}};
    syn_isotp_process_rx_frame(&fd_link, &fd_bad_esc);
    TEST_ASSERT_EQUAL(SYN_ISOTP_RX_IDLE, fd_link.rx_state);
    TEST_ASSERT_FALSE(fd_link.rx_fc_pending);
#endif

    /* 5. FlowControl: idle FC ignored, short FC ignored, invalid FS (> 2) aborts TX */
    SYN_CAN_Frame fc_bad_fs = {.id = 0x700U, .dlc = 8U, .data = {0x33U, 0U, 0U}};
    syn_isotp_process_rx_frame(&link, &fc_bad_fs);

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, pro15_classic_tx, 20U));
    SYN_CAN_Frame ff_out;
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &ff_out));
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

    SYN_CAN_Frame fc_short = {.id = 0x700U, .dlc = 2U, .data = {0x30U, 0U}};
    syn_isotp_process_rx_frame(&link, &fc_short);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_WAIT_FC, link.tx_state);

    syn_isotp_process_rx_frame(&link, &fc_bad_fs);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);
    TEST_ASSERT_EQUAL_UINT32(0U, link.tx_timeout_timer_us);

    /* 6. ms -> us conversion saturates at UINT32_MAX without 32-bit wraparound */
    syn_isotp_set_timeouts(&link, 0U, 0U);
    TEST_ASSERT_EQUAL_UINT32(SYN_ISOTP_DEFAULT_N_BS_MS * 1000U, link.n_bs_timeout_us);
    syn_isotp_set_timeouts(&link, 5000000U, 5000000U);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, link.n_bs_timeout_us);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, link.n_cr_timeout_us);

    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&link, pro15_classic_tx, 20U));
    TEST_ASSERT_TRUE(syn_isotp_get_tx_frame(&link, &ff_out));
    syn_isotp_step(&link, 5000000U);
    TEST_ASSERT_EQUAL(SYN_ISOTP_TX_IDLE, link.tx_state);
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
    RUN_TEST(test_isotp_pro15_frame_validation_and_saturation);
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    RUN_TEST(test_isotp_canfd_single_frame);
    RUN_TEST(test_isotp_canfd_multi_frame);
    RUN_TEST(test_isotp_canfd_extended_first_frame);
#endif
}
