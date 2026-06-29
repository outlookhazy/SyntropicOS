/**
 * @file test_heartbeat.c
 * @brief Unity tests for syn_heartbeat.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "mocks/mock_transport.h"
#include "syntropic/syntropic.h"
#include "syntropic/net/syn_router.h"
#include "syntropic/net/syn_heartbeat.h"

static int hb_lost_n = 0;
static int hb_found_n = 0;
static void hb_lost(uint8_t n, void *c)  { (void)n; (void)c; hb_lost_n++; }
static void hb_found(uint8_t n, void *c) { (void)n; (void)c; hb_found_n++; }

static void test_heartbeat(void)
{
    mock_transport_reset();

    SYN_Transport hbt = { .send = rt_send, .recv = rt_recv, .ctx = NULL };
    SYN_RouterHandler hbh[8];
    SYN_Router hbr;
    syn_router_init(&hbr, 0x01, &hbt, hbh, 8);

    SYN_HB_Peer peers[4];
    SYN_Heartbeat hb;
    syn_heartbeat_init(&hb, &hbr, peers, 4, 1000, 3000);

    hb_lost_n = 0;
    hb_found_n = 0;
    syn_heartbeat_on_peer_lost(&hb, hb_lost, NULL);
    syn_heartbeat_on_peer_found(&hb, hb_found, NULL);

    /* Add peers */
    TEST_ASSERT_TRUE(syn_heartbeat_add_peer(&hb, 0x02));
    TEST_ASSERT_TRUE(syn_heartbeat_add_peer(&hb, 0x03));
    TEST_ASSERT_TRUE(syn_heartbeat_peer_alive(&hb, 0x02));

    /* Duplicate add is OK */
    TEST_ASSERT_TRUE(syn_heartbeat_add_peer(&hb, 0x02));

    /* Heartbeat interval fires — sends a heartbeat */
    mock_tick_ms += 1100;
    rt_rx_rdy = false;
    syn_heartbeat_update(&hb);
    TEST_ASSERT_TRUE(rt_tx_len > 0);

    /* Peer times out */
    mock_tick_ms += 3100;
    rt_rx_rdy = false;
    syn_heartbeat_update(&hb);
    TEST_ASSERT_FALSE(syn_heartbeat_peer_alive(&hb, 0x02));
    TEST_ASSERT_TRUE(hb_lost_n >= 1);

    /* Receive heartbeat from peer — revival */
    rt_rx_buf[0] = 0x02; rt_rx_buf[1] = 0x01;
    rt_rx_buf[2] = SYN_MSG_HEARTBEAT;
    rt_rx_buf[3] = 0; rt_rx_buf[4] = 0; rt_rx_buf[5] = 0;
    rt_rx_len = 6; rt_rx_rdy = true;
    syn_router_poll(&hbr);
    TEST_ASSERT_TRUE(syn_heartbeat_peer_alive(&hb, 0x02));
    TEST_ASSERT_TRUE(hb_found_n >= 1);
}

void run_heartbeat_tests(void)
{
    RUN_TEST(test_heartbeat);
}
