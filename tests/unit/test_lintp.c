/**
 * @file test_lintp.c
 * @brief Unity tests for ISO 17987-2 LIN Transport Protocol (syn_lintp).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_lintp.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

#if !defined(SYN_USE_LINTP) || SYN_USE_LINTP

static uint8_t g_rx_buf[256];
static uint8_t g_tx_buf[256];

static void test_lintp_init_and_idle(void)
{
    SYN_LINTP_Link link;
    syn_lintp_init(&link, 0x01U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));
    TEST_ASSERT_TRUE(syn_lintp_is_tx_idle(&link));

    uint8_t dummy_out[8];
    TEST_ASSERT_FALSE(syn_lintp_get_tx_frame(&link, dummy_out));
    TEST_ASSERT_FALSE(syn_lintp_get_tx_frame(&link, NULL));

    uint8_t recv[32];
    TEST_ASSERT_EQUAL_INT(0, syn_lintp_receive(&link, recv, sizeof(recv)));
}

static void test_lintp_single_frame_tx_rx(void)
{
    SYN_LINTP_Link tx_link;
    SYN_LINTP_Link rx_link;
    syn_lintp_init(&tx_link, 0x01U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));
    syn_lintp_init(&rx_link, 0x02U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));

    uint8_t payload[4] = {0x22U, 0xF1U, 0x90U, 0xAAU};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lintp_send(&tx_link, 0x02U, payload, sizeof(payload)));
    TEST_ASSERT_FALSE(syn_lintp_is_tx_idle(&tx_link));

    uint8_t frame[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, frame));
    TEST_ASSERT_TRUE(syn_lintp_is_tx_idle(&tx_link));
    TEST_ASSERT_EQUAL_HEX8(0x02U, frame[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04U, frame[1]);
    TEST_ASSERT_EQUAL_MEMORY(payload, &frame[2], 4U);

    syn_lintp_process_rx_frame(&rx_link, frame);

    uint8_t rx_payload[32];
    ssize_t rx_bytes = syn_lintp_receive(&rx_link, rx_payload, sizeof(rx_payload));
    TEST_ASSERT_EQUAL_INT(4, rx_bytes);
    TEST_ASSERT_EQUAL_MEMORY(payload, rx_payload, 4U);
}

static void test_lintp_multi_frame_tx_rx(void)
{
    SYN_LINTP_Link tx_link;
    SYN_LINTP_Link rx_link;
    syn_lintp_init(&tx_link, 0x01U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));
    syn_lintp_init(&rx_link, 0x02U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));

    uint8_t payload[18];
    for (int i = 0; i < 18; i++) {
        payload[i] = (uint8_t)(0x10 + i);
    }

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lintp_send(&tx_link, 0x02U, payload, sizeof(payload)));
    TEST_ASSERT_FALSE(syn_lintp_is_tx_idle(&tx_link));

    uint8_t frame_ff[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, frame_ff));
    TEST_ASSERT_EQUAL_HEX8(0x02U, frame_ff[0]);
    TEST_ASSERT_EQUAL_HEX8(0x10U, frame_ff[1]);
    TEST_ASSERT_EQUAL_HEX8(18U, frame_ff[2]);
    TEST_ASSERT_EQUAL_MEMORY(&payload[0], &frame_ff[3], 5U);
    syn_lintp_process_rx_frame(&rx_link, frame_ff);

    uint8_t frame_cf1[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, frame_cf1));
    TEST_ASSERT_EQUAL_HEX8(0x02U, frame_cf1[0]);
    TEST_ASSERT_EQUAL_HEX8(0x21U, frame_cf1[1]);
    TEST_ASSERT_EQUAL_MEMORY(&payload[5], &frame_cf1[2], 6U);
    syn_lintp_process_rx_frame(&rx_link, frame_cf1);

    uint8_t frame_cf2[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, frame_cf2));
    TEST_ASSERT_EQUAL_HEX8(0x02U, frame_cf2[0]);
    TEST_ASSERT_EQUAL_HEX8(0x22U, frame_cf2[1]);
    TEST_ASSERT_EQUAL_MEMORY(&payload[11], &frame_cf2[2], 6U);
    syn_lintp_process_rx_frame(&rx_link, frame_cf2);

    uint8_t frame_cf3[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, frame_cf3));
    TEST_ASSERT_EQUAL_HEX8(0x02U, frame_cf3[0]);
    TEST_ASSERT_EQUAL_HEX8(0x23U, frame_cf3[1]);
    TEST_ASSERT_EQUAL_HEX8(payload[17], frame_cf3[2]);
    syn_lintp_process_rx_frame(&rx_link, frame_cf3);

    TEST_ASSERT_FALSE(syn_lintp_get_tx_frame(&tx_link, frame_cf3));
    TEST_ASSERT_TRUE(syn_lintp_is_tx_idle(&tx_link));

    uint8_t rx_payload[32];
    ssize_t rx_bytes = syn_lintp_receive(&rx_link, rx_payload, sizeof(rx_payload));
    TEST_ASSERT_EQUAL_INT(18, rx_bytes);
    TEST_ASSERT_EQUAL_MEMORY(payload, rx_payload, 18U);
}

static void test_lintp_sn_error_and_timeout(void)
{
    SYN_LINTP_Link rx_link;
    syn_lintp_init(&rx_link, 0x02U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));

    uint8_t ff_frame[8] = {0x02U, 0x10U, 0x0CU, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U};
    syn_lintp_process_rx_frame(&rx_link, ff_frame);

    uint8_t bad_cf[8] = {0x02U, 0x23U, 0x06U, 0x07U, 0x08U, 0x09U, 0x0AU, 0x0BU};
    syn_lintp_process_rx_frame(&rx_link, bad_cf);

    uint8_t recv[32];
    TEST_ASSERT_EQUAL_INT(0, syn_lintp_receive(&rx_link, recv, sizeof(recv)));

    syn_lintp_process_rx_frame(&rx_link, ff_frame);
    syn_lintp_step(&rx_link, 100U);
    syn_lintp_step(&rx_link, 1500U);

    uint8_t valid_cf1[8] = {0x02U, 0x21U, 0x06U, 0x07U, 0x08U, 0x09U, 0x0AU, 0x0BU};
    syn_lintp_process_rx_frame(&rx_link, valid_cf1);
    TEST_ASSERT_EQUAL_INT(0, syn_lintp_receive(&rx_link, recv, sizeof(recv)));
}

static void test_lintp_edge_cases_and_nulls(void)
{
    syn_lintp_init(NULL, 0x01U, g_rx_buf, sizeof(g_rx_buf), NULL, 0U);
    syn_lintp_set_timeouts(NULL, 500U, 500U);
    syn_lintp_set_padding(NULL, 0xAAU);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(NULL, 0x02U, g_rx_buf, 4U));

    SYN_LINTP_Link link;
    syn_lintp_init(&link, 0x02U, g_rx_buf, 10U, g_tx_buf, 24U);
    uint8_t payload8[8] = {0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(&link, 0x02U, NULL, 4U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(&link, 0x02U, payload8, 0U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(&link, 0x02U, payload8, 5000U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(&link, 0x02U, g_rx_buf, 30U));
    TEST_ASSERT_FALSE(syn_lintp_get_tx_frame(NULL, payload8));
    syn_lintp_process_rx_frame(NULL, payload8);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_receive(NULL, payload8, 4U));
    syn_lintp_step(NULL, 100U);
    TEST_ASSERT_TRUE(syn_lintp_is_tx_idle(NULL));

    syn_lintp_set_timeouts(&link, 0U, 0U);
    TEST_ASSERT_EQUAL_UINT32(1000U, link.timer_n_as_ms);
    TEST_ASSERT_EQUAL_UINT32(1000U, link.timer_n_cr_ms);
    syn_lintp_set_timeouts(&link, 500U, 600U);
    TEST_ASSERT_EQUAL_UINT32(500U, link.timer_n_as_ms);
    TEST_ASSERT_EQUAL_UINT32(600U, link.timer_n_cr_ms);

    syn_lintp_set_padding(&link, 0xAAU);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, link.padding_byte);

    uint8_t big_payload[20] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lintp_send(&link, 0x02U, big_payload, sizeof(big_payload)));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_lintp_send(&link, 0x02U, payload8, 4U));

    syn_lintp_step(&link, 100U);
    syn_lintp_step(&link, 600U);
    TEST_ASSERT_TRUE(syn_lintp_is_tx_idle(&link));

    syn_lintp_process_rx_frame(&link, NULL);

    uint8_t wrong_nad_frame[8] = {0x05U, 0x04U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U};
    syn_lintp_process_rx_frame(&link, wrong_nad_frame);

    uint8_t invalid_sf0[8] = {0x02U, 0x00U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U};
    syn_lintp_process_rx_frame(&link, invalid_sf0);

    uint8_t invalid_sf7[8] = {0x02U, 0x07U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U};
    syn_lintp_process_rx_frame(&link, invalid_sf7);

    uint8_t invalid_ff5[8] = {0x02U, 0x10U, 0x05U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U};
    syn_lintp_process_rx_frame(&link, invalid_ff5);

    uint8_t invalid_ff20[8] = {0x02U, 0x10U, 0x14U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U};
    syn_lintp_process_rx_frame(&link, invalid_ff20);

    uint8_t unknown_pci[8] = {0x02U, 0x30U, 0x22U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U};
    syn_lintp_process_rx_frame(&link, unknown_pci);

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_receive(&link, NULL, 10U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_receive(&link, g_rx_buf, 0U));
}

static void lintp_queue_stack_payload_helper(SYN_LINTP_Link *link)
{
    uint8_t stack_buf[16];
    for (uint8_t i = 0U; i < 16U; i++) {
        stack_buf[i] = (uint8_t)(0xA0U + i);
    }
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lintp_send(link, 0x02U, stack_buf, sizeof(stack_buf)));
    memset(stack_buf, 0x00, sizeof(stack_buf));
}

static void test_r5_a04_stack_tx_copy_and_cf_nad_check(void)
{
    SYN_LINTP_Link tx_link;
    SYN_LINTP_Link rx_link;
    syn_lintp_init(&tx_link, 0x01U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));
    syn_lintp_init(&rx_link, 0x02U, g_rx_buf, sizeof(g_rx_buf), g_tx_buf, sizeof(g_tx_buf));

    /* Queue multi-frame TX from transient stack frame and clobber caller stack buffer */
    lintp_queue_stack_payload_helper(&tx_link);

    uint8_t f[8];
    TEST_ASSERT_TRUE(syn_lintp_get_tx_frame(&tx_link, f)); /* FF */
    syn_lintp_process_rx_frame(&rx_link, f);

    /* Inject broadcast/functional CF with mismatched NAD (0x7F != rx_nad 0x02) -> ignored! */
    uint8_t spoof_cf[8] = {
        SYN_LINTP_NAD_BROADCAST, 0x21U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    syn_lintp_process_rx_frame(&rx_link, spoof_cf);
    uint8_t func_sf[8] = {
        SYN_LINTP_NAD_FUNCTIONAL, 0x21U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    syn_lintp_process_rx_frame(&rx_link, func_sf);

    while (syn_lintp_get_tx_frame(&tx_link, f)) {
        syn_lintp_process_rx_frame(&rx_link, f);
    }

    uint8_t out[20];
    ssize_t got = syn_lintp_receive(&rx_link, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(16, got);
    for (uint8_t i = 0U; i < 16U; i++) {
        TEST_ASSERT_EQUAL_HEX8((uint8_t)(0xA0U + i), out[i]);
    }

    /* Also cover Single-Frame TX with (NULL, 0), SF busy, NULL rx_buf, and truncated receive */
    SYN_LINTP_Link no_tx;
    syn_lintp_init(&no_tx, 0x01U, NULL, 0U, NULL, 0U);
    uint8_t sf_data[4] = {1U, 2U, 3U, 4U};
    uint8_t mf_data[10] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lintp_send(&no_tx, 0x02U, mf_data, 10U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lintp_send(&no_tx, 0x02U, sf_data, 4U));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_lintp_send(&no_tx, 0x02U, sf_data, 4U));
    syn_lintp_step(&no_tx, 1000U); /* N_As timeout on pending SF */

    uint8_t sf_in[8] = {0x01U, 0x04U, 1U, 2U, 3U, 4U, 0U, 0U};
    uint8_t ff_in[8] = {0x01U, 0x10U, 0x08U, 1U, 2U, 3U, 4U, 5U};
    syn_lintp_process_rx_frame(&no_tx, sf_in); /* rx_buf == NULL */
    syn_lintp_process_rx_frame(&no_tx, ff_in); /* rx_buf == NULL */

    SYN_LINTP_Link tiny_rx;
    syn_lintp_init(&tiny_rx, 0x01U, g_rx_buf, 2U, g_tx_buf, 16U);
    syn_lintp_process_rx_frame(&tiny_rx, sf_in); /* sf_len (4) > rx_buf_size (2) */

    syn_lintp_init(&tiny_rx, 0x01U, g_rx_buf, 16U, g_tx_buf, 16U);
    syn_lintp_process_rx_frame(&tiny_rx, sf_in);
    TEST_ASSERT_EQUAL_INT(2, syn_lintp_receive(&tiny_rx, out, 2U));
}

void run_lintp_tests(void)
{
    RUN_TEST(test_lintp_init_and_idle);
    RUN_TEST(test_lintp_single_frame_tx_rx);
    RUN_TEST(test_lintp_multi_frame_tx_rx);
    RUN_TEST(test_lintp_sn_error_and_timeout);
    RUN_TEST(test_lintp_edge_cases_and_nulls);
    RUN_TEST(test_r5_a04_stack_tx_copy_and_cf_nad_check);
}

#endif /* SYN_USE_LINTP */
