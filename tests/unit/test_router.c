/**
 * @file test_router.c
 * @brief Unity tests for syn_router — full coverage.
 */

#include "mocks/mock_port.h"
#include "mocks/mock_transport.h"
#include "syntropic/net/syn_router.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int rt_msg_n = 0;
static uint8_t rt_last_type = 0;

static void rt_handler(const SYN_Packet *p, void *c)
{
    (void)c;
    rt_msg_n++;
    rt_last_type = p->type;
}

/* ── Test: basic send, receive, ACK ─────────────────────────────────────── */

static void test_router(void)
{
    mock_transport_reset();

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[8];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 8);
    TEST_ASSERT_EQUAL_HEX8(0x01, rtr.node_id);

    /* Register handler */
    TEST_ASSERT_TRUE(syn_router_register(&rtr, 0x10, rt_handler, NULL));

    /* Send a packet */
    uint8_t pl[] = {0xAA, 0xBB};
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 2, false));
    TEST_ASSERT_EQUAL_size_t(SYN_ROUTER_HEADER_SIZE + 2, rt_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, rt_tx_buf[0]); /* src */
    TEST_ASSERT_EQUAL_HEX8(0x02, rt_tx_buf[1]); /* dst */
    TEST_ASSERT_EQUAL_INT(1, rtr.tx_count);

    /* Receive a packet addressed to us */
    rt_msg_n = 0;
    rt_rx_buf[0] = 0x02;
    rt_rx_buf[1] = 0x01;
    rt_rx_buf[2] = 0x10;
    rt_rx_buf[3] = 0;
    rt_rx_buf[4] = 0;
    rt_rx_buf[5] = 1;
    rt_rx_buf[6] = 0xCC;
    rt_rx_len = 7;
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rt_msg_n);
    TEST_ASSERT_EQUAL_HEX8(0x10, rt_last_type);

    /* Packet addressed to someone else — ignored */
    rt_rx_buf[1] = 0x05;
    rt_rx_len = 7;
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rt_msg_n);

    /* Broadcast (0xFF) — received */
    rt_rx_buf[1] = 0xFF;
    rt_rx_len = 7;
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(2, rt_msg_n);

    /* Unregistered type — dropped */
    rt_rx_buf[1] = 0x01;
    rt_rx_buf[2] = 0x99;
    rt_rx_len = 7;
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rtr.drop_count);

    /* ACK/reliable delivery */
    SYN_PendingAck pend[4];
    syn_router_enable_ack(&rtr, pend, 4, 500, 3);
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 2, true));
    TEST_ASSERT_TRUE((rt_tx_buf[4] & SYN_PKT_FLAG_ACK_REQ) != 0);

    uint8_t aseq = rt_tx_buf[3];
    rt_rx_buf[0] = 0x02;
    rt_rx_buf[1] = 0x01;
    rt_rx_buf[2] = SYN_MSG_ACK;
    rt_rx_buf[3] = aseq;
    rt_rx_buf[4] = SYN_PKT_FLAG_IS_ACK;
    rt_rx_buf[5] = 0;
    rt_rx_len = 6;
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_FALSE(pend[0].active);
}

/* ── Test: receiving packet with ACK_REQ → send_ack executed ──────────── */

static void test_router_send_ack_on_ackreq(void)
{
    mock_transport_reset();
    mock_tick_ms = 0;

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[4];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 4);
    syn_router_register(&rtr, 0x10, rt_handler, NULL);

    /* Send a packet to us with ACK_REQ flag set */
    rt_rx_buf[0] = 0x02;                 /* src = node 2 */
    rt_rx_buf[1] = 0x01;                 /* dst = us */
    rt_rx_buf[2] = 0x10;                 /* type */
    rt_rx_buf[3] = 0x42;                 /* seq = 0x42 */
    rt_rx_buf[4] = SYN_PKT_FLAG_ACK_REQ; /* flags */
    rt_rx_buf[5] = 0;                    /* len */
    rt_rx_len = 6;
    rt_rx_rdy = true;

    rt_tx_len = 0;
    rt_msg_n = 0;
    syn_router_poll(&rtr);

    /* Handler should have been called */
    TEST_ASSERT_EQUAL_INT(1, rt_msg_n);

    /* An ACK should have been sent back */
    TEST_ASSERT_TRUE(rt_tx_len >= (size_t)SYN_ROUTER_HEADER_SIZE);
    /* ACK packet: src=us(0x01), dst=sender(0x02), type=SYN_MSG_ACK, seq=0x42 */
    TEST_ASSERT_EQUAL_HEX8(0x01, rt_tx_buf[0]); /* our src */
    TEST_ASSERT_EQUAL_HEX8(0x02, rt_tx_buf[1]); /* back to sender */
    TEST_ASSERT_EQUAL_HEX8(0x42, rt_tx_buf[3]); /* echoed seq */
    TEST_ASSERT_TRUE((rt_tx_buf[4] & SYN_PKT_FLAG_IS_ACK) != 0);
}

/* ── Test: pending table full — queue_pending returns false ──────────────── */

static void test_router_pending_table_full(void)
{
    mock_transport_reset();
    mock_tick_ms = 0;

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[4];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 4);

    /* Enable ACK with only 2 pending slots */
    SYN_PendingAck pend[2];
    syn_router_enable_ack(&rtr, pend, 2, 500, 3);
    syn_router_register(&rtr, 0x10, rt_handler, NULL);

    uint8_t pl[] = {0x01};

    /* Send twice — fills both slots */
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 1, true));
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 1, true));

    /* Third send — queue_pending will fail (table full) but send itself
     * still succeeds; reliable flag just can't be tracked */
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 1, true));
    /* Both pending slots should still be active */
    TEST_ASSERT_TRUE(pend[0].active);
    TEST_ASSERT_TRUE(pend[1].active);
}

/* ── Test: retry mechanism — check_retries retransmits ──────────────────── */

static void test_router_check_retries_retransmit(void)
{
    mock_transport_reset();
    mock_tick_ms = 0;

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[4];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 4);

    SYN_PendingAck pend[4];
    syn_router_enable_ack(&rtr, pend, 4, 100, 3); /* timeout=100ms, max=3 */
    syn_router_register(&rtr, 0x10, rt_handler, NULL);

    uint8_t pl[] = {0xDE, 0xAD};
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 2, true));
    TEST_ASSERT_TRUE(pend[0].active);
    TEST_ASSERT_EQUAL_INT(0, pend[0].retries);

    /* Advance past timeout — poll should retransmit */
    mock_tick_advance(200);
    syn_router_poll(&rtr);

    /* Retries count should have incremented (retransmit executed) */
    TEST_ASSERT_EQUAL_INT(1, pend[0].retries);
    /* Packet should still be pending (not dropped yet, max=3) */
    TEST_ASSERT_TRUE(pend[0].active);
}

/* ── Test: max retries exceeded — packet dropped ────────────────────────── */

static void test_router_max_retries_drop(void)
{
    mock_transport_reset();
    mock_tick_ms = 0;

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[4];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 4);

    SYN_PendingAck pend[4];
    syn_router_enable_ack(&rtr, pend, 4, 50, 2); /* timeout=50ms, max=2 */
    syn_router_register(&rtr, 0x10, rt_handler, NULL);

    uint8_t pl[] = {0x01};
    syn_router_send(&rtr, 0x02, 0x10, pl, 1, true);
    TEST_ASSERT_TRUE(pend[0].active);

    int drop_before = rtr.drop_count;

    /* Retry 1 */
    mock_tick_advance(60);
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, pend[0].retries);

    /* Retry 2 */
    mock_tick_advance(60);
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(2, pend[0].retries);

    /* Now retries >= max_retries → packet dropped */
    mock_tick_advance(60);
    syn_router_poll(&rtr);
    TEST_ASSERT_FALSE(pend[0].active);
    TEST_ASSERT_EQUAL_INT(drop_before + 1, rtr.drop_count);
}

/* ── Test: deserialize failure → drop_count++ (line 277) ──────────────── */

static void test_router_deserialize_failure(void)
{
    mock_transport_reset();
    mock_tick_ms = 0;

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[4];
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 4);

    int drop_before = rtr.drop_count;

    /* Send a packet that's too short to deserialize (< HEADER_SIZE) */
    rt_rx_buf[0] = 0x01;
    rt_rx_buf[1] = 0x01;
    rt_rx_len = 2; /* far too short — deserialize will fail */
    rt_rx_rdy = true;

    syn_router_poll(&rtr);
    /* drop_count should have increased */
    TEST_ASSERT_EQUAL_INT(drop_before + 1, rtr.drop_count);
}

/* ── Test: register handler until capacity full → returns false ─────────── */

static void test_router_register_full(void)
{
    mock_transport_reset();

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[2]; /* only 2 slots */
    SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 2);

    TEST_ASSERT_TRUE(syn_router_register(&rtr, 0x01, rt_handler, NULL));
    TEST_ASSERT_TRUE(syn_router_register(&rtr, 0x02, rt_handler, NULL));
    /* Third registration exceeds capacity */
    TEST_ASSERT_FALSE(syn_router_register(&rtr, 0x03, rt_handler, NULL));

    /* Test deserialize oversized payload length > SYN_ROUTER_MAX_PAYLOAD (line 62) */
    uint8_t over_len_buf[SYN_ROUTER_HEADER_SIZE + 10] = {0x02, 0x01, 0x10, 0x01, 0x00, 250};
    rt_rx_len = sizeof(over_len_buf);
    memcpy(rt_rx_buf, over_len_buf, rt_rx_len);
    rt_rx_rdy = true;
    syn_router_poll(&rtr);

    /* Test deserialize truncated payload len < header + pkt->len (line 64) */
    uint8_t trunc_payload_buf[SYN_ROUTER_HEADER_SIZE + 2] = {0x02, 0x01, 0x10, 0x01, 0x00, 10};
    rt_rx_len = sizeof(trunc_payload_buf);
    memcpy(rt_rx_buf, trunc_payload_buf, rt_rx_len);
    rt_rx_rdy = true;
    syn_router_poll(&rtr);

    /* Test sending ACK-required packet when pending table is NULL (lines 105 & 130) */
    /* Reliable send when pending table is NULL (lines 267-269) */
    uint8_t d[3] = {1, 2, 3};
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, d, 3, true));

    /* Receive ACK when pending table is NULL (line 105) */
    uint8_t ack_pkt[SYN_ROUTER_HEADER_SIZE] = {0x02, 0x01, SYN_MSG_ACK, 0x01, SYN_PKT_FLAG_IS_ACK,
                                               0};
    rt_rx_len = sizeof(ack_pkt);
    memcpy(rt_rx_buf, ack_pkt, rt_rx_len);
    rt_rx_rdy = true;
    syn_router_poll(&rtr);
}

static void test_tier_a_router_regressions(void)
{
    mock_transport_reset();

    SYN_Transport tr = {.send = rt_send, .recv = rt_recv, .ctx = NULL};
    SYN_RouterHandler rh[2];
    SYN_PendingAck pend[2];
    SYN_Router rtr;
    uint8_t buf[SYN_ROUTER_MAX_PAYLOAD + 4U] = {0};

    syn_router_init(&rtr, 0x01U, &tr, rh, 2U);
    syn_router_enable_ack(&rtr, pend, 2U, 100U, 2U);

    /* 1. Runtime guards: NULL router, oversized payload (> 64), and NULL data with len > 0 */
    TEST_ASSERT_FALSE(syn_router_send(NULL, 0x02U, 0x10U, buf, 1U, false));
    TEST_ASSERT_FALSE(syn_router_send(&rtr, 0x02U, 0x10U, buf, 65U, false));
    TEST_ASSERT_FALSE(syn_router_send(&rtr, 0x02U, 0x10U, NULL, 4U, true));

    /* Zero-length payload with NULL data remains valid */
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02U, 0x10U, NULL, 0U, true));
    TEST_ASSERT_TRUE(pend[0].active);
    pend[0].active = false;

    /* 2. Broadcast (dst == 0xFF) with reliable == true must not queue into pending ACK table */
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0xFFU, 0x10U, buf, 2U, true));
    TEST_ASSERT_FALSE(pend[0].active);
    TEST_ASSERT_FALSE(pend[1].active);
    TEST_ASSERT_EQUAL_HEX8(0U, rt_tx_buf[4]);
}

/* ── Test runner ─────────────────────────────────────────────────────── */

void run_router_tests(void)
{
    RUN_TEST(test_router);
    RUN_TEST(test_router_send_ack_on_ackreq);
    RUN_TEST(test_router_pending_table_full);
    RUN_TEST(test_router_check_retries_retransmit);
    RUN_TEST(test_router_max_retries_drop);
    RUN_TEST(test_router_deserialize_failure);
    RUN_TEST(test_router_register_full);
    RUN_TEST(test_tier_a_router_regressions);
}
