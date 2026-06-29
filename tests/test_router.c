/**
 * @file test_router.c
 * @brief Unity tests for syn_router.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "mocks/mock_transport.h"
#include "syntropic/syntropic.h"
#include "syntropic/net/syn_router.h"

static int rt_msg_n = 0;
static uint8_t rt_last_type = 0;

static void rt_handler(const SYN_Packet *p, void *c)
{
    (void)c;
    rt_msg_n++;
    rt_last_type = p->type;
}

static void test_router(void)
{
    mock_transport_reset();

    SYN_Transport tr = { .send = rt_send, .recv = rt_recv, .ctx = NULL };
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
    rt_rx_buf[0] = 0x02; rt_rx_buf[1] = 0x01; rt_rx_buf[2] = 0x10;
    rt_rx_buf[3] = 0;    rt_rx_buf[4] = 0;    rt_rx_buf[5] = 1;
    rt_rx_buf[6] = 0xCC;
    rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rt_msg_n);
    TEST_ASSERT_EQUAL_HEX8(0x10, rt_last_type);

    /* Packet addressed to someone else — ignored */
    rt_rx_buf[1] = 0x05; rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rt_msg_n);

    /* Broadcast (0xFF) — received */
    rt_rx_buf[1] = 0xFF; rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(2, rt_msg_n);

    /* Unregistered type — dropped */
    rt_rx_buf[1] = 0x01; rt_rx_buf[2] = 0x99;
    rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_EQUAL_INT(1, rtr.drop_count);

    /* ACK/reliable delivery */
    SYN_PendingAck pend[4];
    syn_router_enable_ack(&rtr, pend, 4, 500, 3);
    TEST_ASSERT_TRUE(syn_router_send(&rtr, 0x02, 0x10, pl, 2, true));
    TEST_ASSERT_TRUE((rt_tx_buf[4] & SYN_PKT_FLAG_ACK_REQ) != 0);

    uint8_t aseq = rt_tx_buf[3];
    rt_rx_buf[0] = 0x02; rt_rx_buf[1] = 0x01;
    rt_rx_buf[2] = SYN_MSG_ACK;
    rt_rx_buf[3] = aseq;
    rt_rx_buf[4] = SYN_PKT_FLAG_IS_ACK;
    rt_rx_buf[5] = 0;
    rt_rx_len = 6; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_FALSE(pend[0].active);
}

void run_router_tests(void)
{
    RUN_TEST(test_router);
}
