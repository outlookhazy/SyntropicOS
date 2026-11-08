/**
 * @file test_net_transport_udp.c
 * @brief Unit tests for syn_transport_udp bridge interface.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/net/syn_transport_udp.h"
#include "syntropic/net/syn_udp.h"
#include "syntropic/util/syn_pack.h"
#include "unity/unity.h"

#include <string.h>

static SYN_ETH s_eth;
static SYN_UDP s_udp;
static uint8_t s_mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
static uint32_t s_ip = 0x0A000001; /* 10.0.0.1 */

static void init_transport_udp_fixture(void)
{
    syn_eth_init(&s_eth, s_mac, s_ip);
    syn_udp_init(&s_udp, &s_eth);
    syn_transport_udp_set_instance(&s_udp);
}

void test_net_transport_udp_instance(void)
{
    syn_transport_udp_set_instance(NULL);
    TEST_ASSERT_NULL(syn_transport_udp_get_instance());

    /* Calls while instance is NULL fail gracefully */
    SYN_SockAddr dst = {.ip = {10, 0, 0, 2}, .port = 7000};
    uint8_t buf[32] = {0};
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, syn_transport_udp_open(0));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(100, "hi", 2, &dst));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_recvfrom(100, buf, sizeof(buf), &dst, 0));
    TEST_ASSERT_FALSE(syn_transport_udp_readable(100));
    syn_transport_udp_close(100);

    init_transport_udp_fixture();
    TEST_ASSERT_EQUAL_PTR(&s_udp, syn_transport_udp_get_instance());

    syn_transport_udp_set_instance(NULL);
    TEST_ASSERT_NULL(syn_transport_udp_get_instance());
}

void test_net_transport_udp_open_io_readable_and_close(void)
{
    mock_port_reset();
    init_transport_udp_fixture();

    /* 1. Ephemeral port open (port == 0) + explicit port open */
    SYN_Socket s0 = syn_transport_udp_open(0);
    TEST_ASSERT_EQUAL(100, s0);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(49152U, s_udp.sockets[0].local_port);

    SYN_Socket s1 = syn_transport_udp_open(5683);
    TEST_ASSERT_EQUAL(101, s1);
    TEST_ASSERT_EQUAL_UINT16(5683, s_udp.sockets[1].local_port);

    /* Exhaust remaining slots -> open fails with SYN_SOCKET_INVALID */
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, syn_transport_udp_open(0));
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, syn_transport_udp_open(0));
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, syn_transport_udp_open(0));

    /* 2. Invalid handle & parameter checks */
    SYN_SockAddr peer = {.ip = {10, 0, 0, 9}, .port = 8888};
    uint8_t rx_buf[32];
    TEST_ASSERT_FALSE(syn_transport_udp_readable(99));
    TEST_ASSERT_FALSE(syn_transport_udp_readable(100 + SYN_UDP_MAX_SOCKETS));
    TEST_ASSERT_FALSE(syn_transport_udp_readable(s1));
    syn_transport_udp_close(99);
    syn_transport_udp_close(100 + SYN_UDP_MAX_SOCKETS);

    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(99, "A", 1, &peer));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(100 + SYN_UDP_MAX_SOCKETS, "A", 1, &peer));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(s1, "A", 1, NULL));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(s1, rx_buf, 2000, &peer));

    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_recvfrom(99, rx_buf, sizeof(rx_buf), &peer, 0));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_recvfrom(100 + SYN_UDP_MAX_SOCKETS, rx_buf,
                                                         sizeof(rx_buf), &peer, 0));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_recvfrom(s1, NULL, sizeof(rx_buf), &peer, 0));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_recvfrom(s1, rx_buf, 0, &peer, 0));
    TEST_ASSERT_EQUAL_INT(0, syn_transport_udp_recvfrom(s1, rx_buf, sizeof(rx_buf), &peer, 0));

    /* 3. Transmit datagram -> emits Ethernet frame via syn_port_eth_tx */
    mock_eth_tx_count = 0;
    mock_eth_tx_len = 0;
    TEST_ASSERT_EQUAL_INT(4, syn_transport_udp_sendto(s1, "PING", 4, &peer));
    TEST_ASSERT_EQUAL_INT(1, mock_eth_tx_count);
    TEST_ASSERT_EQUAL_size_t(46U, mock_eth_tx_len);

    /* 4. Inject two back-to-back inbound UDP datagrams from distinct peers -> verify datagram
     *    boundary preservation, truncation of oversized datagram, and per-packet source address */
    uint8_t pkt[64] = {0};
    syn_poke_u16(0x0800U, pkt, 12U);
    pkt[14] = 0x45U;
    pkt[23] = 17U;
    syn_poke_u32(0x0A000009U, pkt, 26U);
    syn_poke_u32(s_ip, pkt, 30U);
    syn_poke_u16(4321U, &pkt[34], 0U);
    syn_poke_u16(5683U, &pkt[34], 2U);
    syn_poke_u16(8U + 6U, &pkt[34], 4U);
    memcpy(&pkt[42], "ABCDEF", 6U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt, 48U));

    uint8_t pkt2[64] = {0};
    syn_poke_u16(0x0800U, pkt2, 12U);
    pkt2[14] = 0x45U;
    pkt2[23] = 17U;
    syn_poke_u32(0x0A00000AU, pkt2, 26U);
    syn_poke_u32(s_ip, pkt2, 30U);
    syn_poke_u16(5555U, &pkt2[34], 0U);
    syn_poke_u16(5683U, &pkt2[34], 2U);
    syn_poke_u16(8U + 4U, &pkt2[34], 4U);
    memcpy(&pkt2[42], "CDEF", 4U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt2, 46U));

    TEST_ASSERT_TRUE(syn_transport_udp_readable(s1));

    /* 1st recvfrom truncates 6B datagram to 2B max_len and advances to 2nd datagram */
    SYN_SockAddr src_out = {0};
    TEST_ASSERT_EQUAL_INT(2, syn_transport_udp_recvfrom(s1, rx_buf, 2U, &src_out, 0));
    TEST_ASSERT_EQUAL_MEMORY("AB", rx_buf, 2U);
    TEST_ASSERT_EQUAL_UINT8(10, src_out.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(9, src_out.ip[3]);
    TEST_ASSERT_EQUAL_UINT16(4321U, src_out.port);
    TEST_ASSERT_TRUE(syn_transport_udp_readable(s1));

    /* 2nd recvfrom reads distinct 4B datagram with NULL from */
    TEST_ASSERT_EQUAL_INT(4, syn_transport_udp_recvfrom(s1, rx_buf, sizeof(rx_buf), NULL, 0));
    TEST_ASSERT_EQUAL_MEMORY("CDEF", rx_buf, 4U);
    TEST_ASSERT_FALSE(syn_transport_udp_readable(s1));

    /* 5. Close socket -> unbound checks in sendto, recvfrom, readable */
    syn_transport_udp_close(s1);
    TEST_ASSERT_FALSE(syn_transport_udp_readable(s1));
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(s1, "X", 1, &peer));
    TEST_ASSERT_EQUAL_INT(0, syn_transport_udp_recvfrom(s1, rx_buf, sizeof(rx_buf), NULL, 0));

    syn_transport_udp_set_instance(NULL);
}

static SYN_Status mock_failing_eth_tx(const void *frame, size_t len)
{
    (void)frame;
    (void)len;
    return SYN_ERROR;
}

void test_net_transport_udp_eth_tx_failure(void)
{
    mock_port_reset();
    init_transport_udp_fixture();

    SYN_Socket s = syn_transport_udp_open(5684);
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, s);

    SYN_SockAddr peer = {.ip = {10, 0, 0, 9}, .port = 8888};
    syn_transport_udp_set_eth_tx(mock_failing_eth_tx);
    TEST_ASSERT_EQUAL_INT(-1, syn_transport_udp_sendto(s, "PING", 4, &peer));

    syn_transport_udp_set_eth_tx(NULL);
    mock_eth_tx_count = 0;
    TEST_ASSERT_EQUAL_INT(4, syn_transport_udp_sendto(s, "PING", 4, &peer));
    TEST_ASSERT_EQUAL_INT(1, mock_eth_tx_count);

    syn_transport_udp_close(s);
    syn_transport_udp_set_instance(NULL);
}

void run_net_transport_udp_tests(void)
{
    RUN_TEST(test_net_transport_udp_instance);
    RUN_TEST(test_net_transport_udp_open_io_readable_and_close);
    RUN_TEST(test_net_transport_udp_eth_tx_failure);
}
