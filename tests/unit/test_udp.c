/**
 * @file test_udp.c
 * @brief Unit tests for zero-alloc native UDP stack (syn_udp).
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/net/syn_transport_udp.h"
#include "syntropic/net/syn_udp.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_pack.h"
#include "unity/unity.h"

#include <string.h>

static SYN_ETH s_eth;
static SYN_UDP s_udp;
static uint8_t s_mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
static uint32_t s_ip = 0x0A000001; /* 10.0.0.1 */

static void init_udp_fixture(void)
{
    syn_eth_init(&s_eth, s_mac, s_ip);
    syn_udp_init(&s_udp, &s_eth);
}

void test_udp_init_and_bind(void)
{
    init_udp_fixture();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_init(&s_udp, &s_eth));

    SYN_UdpSocket *sock1 = syn_udp_bind(&s_udp, 5000);
    TEST_ASSERT_NOT_NULL(sock1);
    TEST_ASSERT_TRUE(sock1->is_bound);
    TEST_ASSERT_EQUAL_UINT16(5000, sock1->local_port);

    SYN_UdpSocket *sock1_again = syn_udp_bind(&s_udp, 5000);
    TEST_ASSERT_EQUAL_PTR(sock1, sock1_again);

    SYN_UdpSocket *sock2 = syn_udp_bind(&s_udp, 6000);
    TEST_ASSERT_NOT_NULL(sock2);
    TEST_ASSERT_EQUAL_UINT16(6000, sock2->local_port);

    syn_udp_unbind(sock1);
    TEST_ASSERT_FALSE(sock1->is_bound);
}

void test_udp_process_packet_demux_and_task_wake(void)
{
    init_udp_fixture();
    SYN_UdpSocket *sock = syn_udp_bind(&s_udp, 9000);
    TEST_ASSERT_NOT_NULL(sock);

    SYN_Task dummy_task;
    memset(&dummy_task, 0, sizeof(dummy_task));
    dummy_task.state = SYN_TASK_BLOCKED;
    sock->blocked_task = &dummy_task;

    /* Build raw Ethernet + IPv4 + UDP packet addressed to port 9000 */
    uint8_t pkt[128];
    memset(pkt, 0, sizeof(pkt));

    /* Ethernet */
    memcpy(&pkt[0], s_mac, 6);
    memcpy(&pkt[6], s_mac, 6);
    pkt[12] = 0x08;
    pkt[13] = 0x00;

    /* IPv4 */
    pkt[14] = 0x45;
    syn_poke_u16(20 + 8 + 5, pkt, 16);
    pkt[23] = 17;                      /* UDP */
    syn_poke_u32(0x0A000002, pkt, 26); /* Src IP 10.0.0.2 */
    syn_poke_u32(s_ip, pkt, 30);       /* Dst IP 10.0.0.1 */

    /* UDP */
    syn_poke_u16(1234, pkt, 34); /* Src Port 1234 */
    syn_poke_u16(9000, pkt, 36); /* Dst Port 9000 */
    syn_poke_u16(13, pkt, 38);   /* Length 13 (8 hdr + 5 payload) */
    memcpy(&pkt[42], "HELLO", 5);

    SYN_Status st = syn_udp_process_packet(&s_udp, pkt, 14 + 20 + 13);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);

    /* Verify payload arrived */
    TEST_ASSERT_EQUAL_UINT16(5, sock->rx_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("HELLO", sock->rx_buf, 5);
    TEST_ASSERT_EQUAL_UINT32(0x0A000002, sock->remote_ip);
    TEST_ASSERT_EQUAL_UINT16(1234, sock->remote_port);

    /* Verify task unblocking */
    TEST_ASSERT_EQUAL_INT(SYN_TASK_READY, dummy_task.state);
}

void test_udp_sendto_framing(void)
{
    init_udp_fixture();
    uint8_t tx_out[128];
    size_t tx_len = 0;

    int res =
        syn_udp_sendto(&s_udp, 5000, 0x0A000002, 6000, (const uint8_t *)"TEST", 4, tx_out, &tx_len);
    TEST_ASSERT_EQUAL_INT(4, res);
    TEST_ASSERT_EQUAL_UINT32(14 + 20 + 8 + 4, tx_len);

    /* Check EtherType */
    TEST_ASSERT_EQUAL_UINT16(0x0800, syn_peek_u16(tx_out, 12));
    /* Check IPv4 Proto */
    TEST_ASSERT_EQUAL_UINT8(17, tx_out[23]);
    /* Check UDP Ports */
    TEST_ASSERT_EQUAL_UINT16(5000, syn_peek_u16(tx_out, 34));
    TEST_ASSERT_EQUAL_UINT16(6000, syn_peek_u16(tx_out, 36));
}

/**
 * Regression: syn_udp_sendto must result in syn_port_eth_tx being called —
 * previously the frame was built but silently dropped (transmit path was
 * missing the syn_port_eth_tx call).
 */
void test_udp_transport_bridge_transmits(void)
{
    init_udp_fixture();

    /* Inject ARP entry so the ETH layer resolves the dst MAC */
    uint8_t dst_mac[6] = {0x02, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    syn_eth_arp_update(&s_eth, 0x0A000002, dst_mac);

    /* Reset mock ETH tx counters */
    mock_eth_tx_len = 0;
    mock_eth_tx_count = 0;

    /* Build a UDP frame directly via the native stack (the transport bridge
     * wraps this exact call) */
    uint8_t tx_frame[SYN_ETH_MAX_FRAME_LEN];
    size_t tx_len = 0;
    int ret = syn_udp_sendto(&s_udp, 7777, 0x0A000002, 8888, (const uint8_t *)"PING", 4, tx_frame,
                             &tx_len);
    TEST_ASSERT_EQUAL_INT(4, ret);
    TEST_ASSERT_GREATER_THAN(0U, tx_len);

    /* Simulate what the bridge does after syn_udp_sendto succeeds */
    syn_port_eth_tx(tx_frame, tx_len);

    /* syn_port_eth_tx must have been called */
    TEST_ASSERT_GREATER_THAN(0, mock_eth_tx_count);
    TEST_ASSERT_GREATER_OR_EQUAL(14U + 20U + 8U + 4U, mock_eth_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x08, mock_eth_tx_buf[12]); /* EtherType hi */
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_eth_tx_buf[13]); /* EtherType lo */
    TEST_ASSERT_EQUAL_UINT8(17, mock_eth_tx_buf[23]);   /* Protocol: UDP */
}

void test_udp_extended_edge_cases(void)
{
    /* Null & invalid init/bind checks + ephemeral port 0 allocation (R3-N09) */
    init_udp_fixture();
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_udp_init(NULL, NULL));
    TEST_ASSERT_NULL(syn_udp_bind(NULL, 5000));
    s_udp.next_ephemeral_port = 0U; /* Exercises < 49152 clamp */
    SYN_UdpSocket *eph1 = syn_udp_bind(&s_udp, 0);
    TEST_ASSERT_NOT_NULL(eph1);
    TEST_ASSERT_EQUAL_UINT16(49152U, eph1->local_port);
    /* Pre-bind 49153 and test collision skip + 65535 wraparound */
    SYN_UdpSocket *fixed_eph = syn_udp_bind(&s_udp, 49153U);
    TEST_ASSERT_NOT_NULL(fixed_eph);
    SYN_UdpSocket *eph2 = syn_udp_bind(&s_udp, 0);
    TEST_ASSERT_NOT_NULL(eph2);
    TEST_ASSERT_EQUAL_UINT16(49154U, eph2->local_port);
    s_udp.next_ephemeral_port = 65535U;
    SYN_UdpSocket *eph_wrap = syn_udp_bind(&s_udp, 0);
    TEST_ASSERT_NOT_NULL(eph_wrap);
    TEST_ASSERT_EQUAL_UINT16(65535U, eph_wrap->local_port);
    TEST_ASSERT_EQUAL_UINT16(49152U, s_udp.next_ephemeral_port);
    init_udp_fixture();

    /* Null & invalid checksum checks */
    TEST_ASSERT_EQUAL_UINT16(0, syn_udp_checksum(0, 0, NULL, 0));
    /* Odd payload length for checksum calculation */
    uint8_t odd_payload[9] = {0x12, 0x34, 0x56, 0x78, 0x00, 0x09, 0x00, 0x00, 0xAA};
    TEST_ASSERT_NOT_EQUAL(
        0, syn_udp_checksum(0x0A000001, 0x0A000002, odd_payload, sizeof(odd_payload)));

    /* Truncated IP packet processing */
    uint8_t short_pkt[20] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_udp_process_packet(&s_udp, short_pkt, sizeof(short_pkt)));

    /* Invalid EtherType (0x0806 ARP) */
    uint8_t arp_pkt[64] = {0};
    syn_poke_u16(0x0806, arp_pkt, 12);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_udp_process_packet(&s_udp, arp_pkt, sizeof(arp_pkt)));

    /* Invalid Protocol (TCP = 6) */
    uint8_t tcp_pkt[64] = {0};
    syn_poke_u16(0x0800, tcp_pkt, 12);
    tcp_pkt[14] = 0x45;
    tcp_pkt[23] = 6;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_udp_process_packet(&s_udp, tcp_pkt, sizeof(tcp_pkt)));

    /* Invalid UDP header length (udp_len = 50 > pkt len) */
    uint8_t bad_udplen_pkt[64] = {0};
    syn_poke_u16(0x0800, bad_udplen_pkt, 12);
    bad_udplen_pkt[14] = 0x45;
    bad_udplen_pkt[23] = 17;
    syn_poke_u16(50, bad_udplen_pkt, 38); /* udp_len 50 > 64 - 34 = 30 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_udp_process_packet(&s_udp, bad_udplen_pkt, sizeof(bad_udplen_pkt)));

    /* syn_udp_sendto with high IP values triggering checksum fold (line 197) */
    uint8_t tx[128];
    size_t tx_l = 0;
    TEST_ASSERT_EQUAL(
        4, syn_udp_sendto(&s_udp, 1234, 0xFFFFFFFF, 5678, (const uint8_t *)"test", 4, tx, &tx_l));

    /* Unbound destination port */
    uint8_t unbound_pkt[64] = {0};
    syn_poke_u16(0x0800, unbound_pkt, 12);
    unbound_pkt[14] = 0x45;
    unbound_pkt[23] = 17;
    syn_poke_u16(9999, &unbound_pkt[34], 2); /* Dst port 9999 */
    syn_poke_u16(16, &unbound_pkt[34], 4);   /* UDP length 16 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_udp_process_packet(&s_udp, unbound_pkt, sizeof(unbound_pkt)));

    /* Over-sized payload sendto */
    uint8_t tx_out[2000];
    size_t tx_len = 0;
    TEST_ASSERT_EQUAL_INT(
        -1, syn_udp_sendto(&s_udp, 5000, 0x0A000002, 6000, tx_out, 1600, tx_out, &tx_len));

    /* sendto with null eth instance */
    SYN_UDP no_eth_udp;
    syn_udp_init(&no_eth_udp, NULL);
    TEST_ASSERT_GREATER_THAN(0, syn_udp_sendto(&no_eth_udp, 5000, 0x0A000002, 6000,
                                               (const uint8_t *)"TEST", 4, tx_out, &tx_len));

    /* unbind NULL check */
    syn_udp_unbind(NULL);

    /* Unbind socket with blocked task attached */
    SYN_UdpSocket blocked_sock;
    memset(&blocked_sock, 0, sizeof(blocked_sock));
    SYN_Task dummy_t;
    memset(&dummy_t, 0, sizeof(dummy_t));
    dummy_t.state = SYN_TASK_BLOCKED;
    blocked_sock.blocked_task = &dummy_t;
    syn_udp_unbind(&blocked_sock);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, dummy_t.state);

    /* Max capacity sockets test */
    SYN_UDP max_udp;
    syn_udp_init(&max_udp, NULL);
    for (uint16_t port = 1000; port < 1000 + SYN_UDP_MAX_SOCKETS; port++) {
        TEST_ASSERT_NOT_NULL(syn_udp_bind(&max_udp, port));
    }
    TEST_ASSERT_NULL(syn_udp_bind(&max_udp, 9999));

    /* Null parameter checks for sendto */
    TEST_ASSERT_EQUAL_INT(-1, syn_udp_sendto(NULL, 0, 0, 0, NULL, 0, NULL, NULL));

    /* Zero-length payload sendto */
    TEST_ASSERT_EQUAL_INT(0,
                          syn_udp_sendto(&s_udp, 5000, 0x0A000002, 6000, NULL, 0, tx_out, &tx_len));

    /* process_packet with udp_len < 8 */
    uint8_t short_udplen_pkt[64] = {0};
    syn_poke_u16(0x0800, short_udplen_pkt, 12);
    short_udplen_pkt[14] = 0x45;
    short_udplen_pkt[23] = 17;
    syn_poke_u16(4, &short_udplen_pkt[34], 4); /* udp_len 4 < 8 */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_udp_process_packet(&s_udp, short_udplen_pkt, sizeof(short_udplen_pkt)));

    /* process_packet buffer overflow check (rx_len + payload_len > SYN_UDP_BUF_SIZE) */
    SYN_UDP ovf_udp;
    syn_udp_init(&ovf_udp, NULL);
    SYN_UdpSocket *ovf_sock = syn_udp_bind(&ovf_udp, 7777);
    TEST_ASSERT_NOT_NULL(ovf_sock);
    ovf_sock->rx_len = SYN_UDP_BUF_SIZE - 2;
    uint8_t ovf_pkt[128] = {0};
    syn_poke_u16(0x0800, ovf_pkt, 12);
    ovf_pkt[14] = 0x45;
    ovf_pkt[23] = 17;
    syn_poke_u16(7777, &ovf_pkt[34], 2); /* dst port 7777 */
    syn_poke_u16(18, &ovf_pkt[34], 4);   /* UDP len 18 -> payload 10 > 2 remaining */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&ovf_udp, ovf_pkt, 34 + 18));
}

void test_transport_udp_all_functions(void)
{
    init_udp_fixture();

    /* Test getter and setter for global native UDP instance */
    syn_transport_udp_set_instance(NULL);
    TEST_ASSERT_NULL(syn_transport_udp_get_instance());

    syn_transport_udp_set_instance(&s_udp);
    TEST_ASSERT_EQUAL_PTR(&s_udp, syn_transport_udp_get_instance());
    syn_transport_udp_set_instance(NULL);
}

typedef struct {
    SYN_UdpSocket *sock;
    uint8_t buf[32];
    size_t max_len;
    size_t bytes_read;
    uint32_t src_ip;
    uint16_t src_port;
    bool null_remote_ep;
} UdpBlockReadCtx;

static SYN_PT_Status udp_block_read_coroutine(SYN_PT *pt, SYN_Task *t)
{
    UdpBlockReadCtx *ctx = (t != NULL) ? (UdpBlockReadCtx *)t->user_data : NULL;
    SYN_UdpSocket *sock = (ctx != NULL) ? ctx->sock : NULL;
    uint8_t *buf = (ctx != NULL) ? ctx->buf : NULL;
    size_t max_len = (ctx != NULL) ? ctx->max_len : 0U;
    size_t *bytes_read = (ctx != NULL) ? &ctx->bytes_read : NULL;
    if (pt == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    if (ctx != NULL && ctx->null_remote_ep) {
        PT_UDP_BLOCK_READ(pt, t, sock, buf, max_len, bytes_read, (uint32_t *)NULL,
                          (uint16_t *)NULL);
    } else {
        PT_UDP_BLOCK_READ(pt, t, sock, buf, max_len, bytes_read,
                          (ctx != NULL) ? &ctx->src_ip : (uint32_t *)NULL,
                          (ctx != NULL) ? &ctx->src_port : (uint16_t *)NULL);
    }
    PT_END(pt);
}

void test_udp_pt_block_read_clears_blocked_task_and_null_guards(void)
{
    init_udp_fixture();

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};

    /* 1. Reject NULL pt, NULL task, NULL user_data, and NULL sock/endpoint */
    TEST_ASSERT_EQUAL(PT_EXITED, udp_block_read_coroutine(NULL, &task));
    TEST_ASSERT_EQUAL(PT_EXITED, udp_block_read_coroutine(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, udp_block_read_coroutine(&pt, &task));

    UdpBlockReadCtx ctx = {0};
    ctx.sock = NULL;
    ctx.max_len = sizeof(ctx.buf);
    task.user_data = &ctx;
    TEST_ASSERT_EQUAL(PT_EXITED, udp_block_read_coroutine(&pt, &task));

    /* 2. Bind socket, block via PT_UDP_BLOCK_READ, wake via incoming datagram, verify
     *    sock->blocked_task is cleared to NULL and remote endpoint is captured */
    SYN_UdpSocket *sock = syn_udp_bind(&s_udp, 9100);
    TEST_ASSERT_NOT_NULL(sock);

    ctx.sock = sock;
    ctx.null_remote_ep = false;
    memset(&task, 0, sizeof(task));
    task.state = SYN_TASK_READY;
    task.user_data = &ctx;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_TASK_BLOCKED, task.state);
    TEST_ASSERT_EQUAL_PTR(&task, sock->blocked_task);

    uint8_t pkt[64] = {0};
    syn_poke_u16(0x0800, pkt, 12);      /* IPv4 */
    pkt[14] = 0x45;                     /* IHL=5 */
    pkt[23] = 17;                       /* UDP */
    syn_poke_u32(0x0A000009U, pkt, 26); /* Src IP 10.0.0.9 */
    syn_poke_u32(s_ip, pkt, 30);        /* Dst IP */
    syn_poke_u16(4321, &pkt[34], 0);    /* Src Port */
    syn_poke_u16(9100, &pkt[34], 2);    /* Dst Port */
    syn_poke_u16(8 + 5, &pkt[34], 4);   /* UDP length = 13 */
    memcpy(&pkt[42], "HELLO", 5);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt, 34 + 13));
    TEST_ASSERT_EQUAL(SYN_TASK_READY, task.state);

    TEST_ASSERT_EQUAL(PT_ENDED, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_NULL(sock->blocked_task);
    TEST_ASSERT_EQUAL_UINT(5, ctx.bytes_read);
    TEST_ASSERT_EQUAL_MEMORY("HELLO", ctx.buf, 5);
    TEST_ASSERT_EQUAL_HEX32(0x0A000009U, ctx.src_ip);
    TEST_ASSERT_EQUAL_UINT16(4321, ctx.src_port);

    /* 3. PT_UDP_BLOCK_READ with NULL src_ip / src_port out-pointers */
    ctx.null_remote_ep = true;
    ctx.bytes_read = 0;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_EQUAL_PTR(&task, sock->blocked_task);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt, 34 + 13));
    TEST_ASSERT_EQUAL(PT_ENDED, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_NULL(sock->blocked_task);
    TEST_ASSERT_EQUAL_UINT(5, ctx.bytes_read);

    /* 4. Unbind while blocked -> wakes with bytes_read == 0 and clears blocked_task == NULL */
    ctx.bytes_read = 77;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_EQUAL_PTR(&task, sock->blocked_task);

    syn_udp_unbind(sock);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, task.state);
    TEST_ASSERT_EQUAL(PT_ENDED, udp_block_read_coroutine(&pt, &task));
    TEST_ASSERT_NULL(sock->blocked_task);
    TEST_ASSERT_EQUAL_UINT(0, ctx.bytes_read);
}

void test_udp_round4_ihl_options_and_task_detach(void)
{
    init_udp_fixture();
    SYN_UdpSocket *sock = syn_udp_bind(&s_udp, 9200);
    TEST_ASSERT_NOT_NULL(sock);

    uint8_t pkt[80] = {0};
    syn_poke_u16(0x0800U, pkt, 12U);
    pkt[23] = 17U;

    /* 1. Reject IHL < 20 and IHL + UDP header exceeding frame length */
    pkt[14] = 0x44U; /* IHL = 16 < 20 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_udp_process_packet(&s_udp, pkt, 50U));

    pkt[14] = 0x4FU; /* IHL = 60 -> 14 + 60 + 8 = 82 > 50 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_udp_process_packet(&s_udp, pkt, 50U));

    /* 2. IPv4 packet with 8B IP options (IHL = 28 -> UDP header at offset 42) */
    SYN_Task wake_task = {0};
    wake_task.state = SYN_TASK_BLOCKED;
    sock->blocked_task = &wake_task;

    memset(pkt, 0, sizeof(pkt));
    syn_poke_u16(0x0800U, pkt, 12U);
    pkt[14] = 0x47U; /* IHL = 28 bytes */
    pkt[23] = 17U;
    syn_poke_u32(0x0A000005U, pkt, 26U);
    syn_poke_u32(s_ip, pkt, 30U);
    /* 8B dummy IP options at [34..41], UDP header at [42..49], payload at [50..53] */
    syn_poke_u16(5555U, &pkt[42], 0U);
    syn_poke_u16(9200U, &pkt[42], 2U);
    syn_poke_u16(8U + 4U, &pkt[42], 4U);
    memcpy(&pkt[50], "OPTS", 4U);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt, 54U));
    TEST_ASSERT_NULL(sock->blocked_task);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, wake_task.state);
    TEST_ASSERT_EQUAL_UINT16(4U, sock->rx_len);
    TEST_ASSERT_EQUAL_MEMORY("OPTS", sock->rx_buf, 4U);
    TEST_ASSERT_EQUAL_UINT16(5555U, sock->remote_port);
}

static SYN_PT_Status udp_stack_array_coroutine(SYN_PT *pt, SYN_Task *t)
{
    SYN_UdpSocket *sock = (SYN_UdpSocket *)t->user_data;
    uint8_t local_buf[16];
    size_t n = 0U;
    uint32_t peer_ip = 0U;
    uint16_t peer_port = 0U;

    PT_BEGIN(pt);
    PT_UDP_BLOCK_READ(pt, t, sock, local_buf, sizeof(local_buf), &n, &peer_ip, &peer_port);
    TEST_ASSERT_EQUAL_size_t(3U, n);
    TEST_ASSERT_EQUAL_MEMORY("ONE", local_buf, 3U);
    TEST_ASSERT_EQUAL_HEX32(0x0A000011U, peer_ip);
    TEST_ASSERT_EQUAL_UINT16(1111U, peer_port);
    PT_END(pt);
}

static void build_test_udp_frame(uint8_t *pkt, uint32_t src_ip, uint16_t src_port,
                                 uint16_t dst_port, const void *payload, uint16_t payload_len)
{
    memset(pkt, 0, 42U + payload_len);
    syn_poke_u16(0x0800U, pkt, 12U);
    pkt[14] = 0x45U;
    pkt[23] = 17U;
    syn_poke_u32(src_ip, pkt, 26U);
    syn_poke_u32(s_ip, pkt, 30U);
    syn_poke_u16(src_port, &pkt[34], 0U);
    syn_poke_u16(dst_port, &pkt[34], 2U);
    syn_poke_u16((uint16_t)(8U + payload_len), &pkt[34], 4U);
    if (payload != NULL && payload_len > 0U) {
        memcpy(&pkt[42], payload, payload_len);
    }
}

void test_r5_n07_udp_datagram_boundaries_and_waddress(void)
{
    init_udp_fixture();
    SYN_UdpSocket *sock = syn_udp_bind(&s_udp, 9300U);
    TEST_ASSERT_NOT_NULL(sock);

    /* 1. Exercise syn_udp_pt_args_valid & syn_udp_recv NULL guards */
    SYN_PT pt;
    SYN_Task t = {0};
    uint8_t out[16];
    size_t br = 99U;
    PT_INIT(&pt);
    TEST_ASSERT_FALSE(syn_udp_pt_args_valid(NULL, &t, sock, out, &br));
    TEST_ASSERT_FALSE(syn_udp_pt_args_valid(&pt, NULL, sock, out, &br));
    TEST_ASSERT_FALSE(syn_udp_pt_args_valid(&pt, &t, NULL, out, &br));
    TEST_ASSERT_FALSE(syn_udp_pt_args_valid(&pt, &t, sock, NULL, &br));
    TEST_ASSERT_FALSE(syn_udp_pt_args_valid(&pt, &t, sock, out, NULL));
    TEST_ASSERT_TRUE(syn_udp_pt_args_valid(&pt, &t, sock, out, &br));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_udp_recv(NULL, out, sizeof(out), &br, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_udp_recv(sock, NULL, sizeof(out), &br, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_udp_recv(sock, out, sizeof(out), NULL, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, sizeof(out), &br, NULL, NULL));
    TEST_ASSERT_EQUAL_size_t(0U, br);

    /* 2. Queue two back-to-back datagrams from distinct peers before reading */
    uint8_t pkt1[64];
    uint8_t pkt2[64];
    build_test_udp_frame(pkt1, 0x0A000011U, 1111U, 9300U, "ONE", 3U);
    build_test_udp_frame(pkt2, 0x0A000022U, 2222U, 9300U, "TWOTWO", 6U);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt1, 45U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt2, 48U));
    TEST_ASSERT_EQUAL_UINT8(2U, sock->dgram_count);
    TEST_ASSERT_EQUAL_UINT16(3U, sock->rx_len);
    TEST_ASSERT_EQUAL_HEX32(0x0A000011U, sock->remote_ip);
    TEST_ASSERT_EQUAL_UINT16(1111U, sock->remote_port);

    /* Dequeue 1st datagram via stack-array PT_UDP_BLOCK_READ (-Werror=address check) */
    PT_INIT(&pt);
    t.state = SYN_TASK_READY;
    t.user_data = sock;
    TEST_ASSERT_EQUAL(PT_ENDED, udp_stack_array_coroutine(&pt, &t));

    /* Dequeue 2nd datagram via syn_udp_recv -> distinct peer metadata & exact 6B boundary */
    uint32_t src_ip = 0U;
    uint16_t src_port = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, sizeof(out), &br, &src_ip, &src_port));
    TEST_ASSERT_EQUAL_size_t(6U, br);
    TEST_ASSERT_EQUAL_MEMORY("TWOTWO", out, 6U);
    TEST_ASSERT_EQUAL_HEX32(0x0A000022U, src_ip);
    TEST_ASSERT_EQUAL_UINT16(2222U, src_port);
    TEST_ASSERT_EQUAL_UINT8(0U, sock->dgram_count);

    /* 3. Zero-length datagram + descriptor ring full drop + legacy transport drain sync */
    uint8_t pkt0[48];
    build_test_udp_frame(pkt0, 0x0A000033U, 3333U, 9300U, NULL, 0U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt0, 42U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, sizeof(out), &br, &src_ip, &src_port));
    TEST_ASSERT_EQUAL_size_t(0U, br);
    TEST_ASSERT_EQUAL_HEX32(0x0A000033U, src_ip);

    for (uint8_t i = 0U; i < SYN_UDP_MAX_DGRAMS + 1U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt1, 45U));
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_UDP_MAX_DGRAMS, sock->dgram_count);
    /* Simulate legacy direct drain of front datagram (rx_len -> 0 with remaining > 0) */
    sock->rx_len = 0U;
    for (uint8_t i = 0U; i < SYN_UDP_MAX_DGRAMS - 1U; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, 2U, &br, NULL, NULL));
        TEST_ASSERT_EQUAL_size_t(2U, br);
    }
    /* Single datagram legacy direct drain (rx_len -> 0 with remaining == 0) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_process_packet(&s_udp, pkt1, 45U));
    sock->rx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, sizeof(out), &br, NULL, NULL));
    TEST_ASSERT_EQUAL_size_t(0U, br);

    /* Legacy direct fill (dgram_count == 0 && rx_len > 0) with non-NULL, NULL, and 0 max_len */
    memcpy(sock->rx_buf, "LEG", 3U);
    sock->rx_len = 3U;
    sock->remote_ip = 0x0A000044U;
    sock->remote_port = 4444U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, sizeof(out), &br, &src_ip, &src_port));
    TEST_ASSERT_EQUAL_size_t(3U, br);
    TEST_ASSERT_EQUAL_MEMORY("LEG", out, 3U);
    TEST_ASSERT_EQUAL_HEX32(0x0A000044U, src_ip);
    TEST_ASSERT_EQUAL_UINT16(4444U, src_port);

    sock->rx_len = 2U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_udp_recv(sock, out, 0U, &br, NULL, NULL));
    TEST_ASSERT_EQUAL_size_t(0U, br);
}

void run_udp_tests(void)
{
    RUN_TEST(test_udp_init_and_bind);
    RUN_TEST(test_udp_process_packet_demux_and_task_wake);
    RUN_TEST(test_udp_sendto_framing);
    RUN_TEST(test_udp_transport_bridge_transmits);
    RUN_TEST(test_udp_extended_edge_cases);
    RUN_TEST(test_transport_udp_all_functions);
    RUN_TEST(test_udp_pt_block_read_clears_blocked_task_and_null_guards);
    RUN_TEST(test_udp_round4_ihl_options_and_task_detach);
    RUN_TEST(test_r5_n07_udp_datagram_boundaries_and_waddress);
}
