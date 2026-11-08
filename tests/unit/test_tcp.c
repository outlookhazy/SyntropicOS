#include "syntropic/net/syn_eth.h"
#include "syntropic/net/syn_tcp.h"
#include "unity/unity.h"

#include <string.h>

static SYN_ETH eth;
static SYN_TCP tcp;
static SYN_Task task;

void test_tcp_init_and_listen(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;

    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_init(&eth, mac, ip));
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_init(&tcp, &eth));

    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_listen(&tcp, 80));
    TEST_ASSERT_EQUAL(SYN_TCP_LISTEN, tcp.conns[0].state);
    TEST_ASSERT_EQUAL(80, tcp.conns[0].local_port);
}

void test_tcp_checksum_calc(void)
{
    uint32_t src_ip = 0xC0A80101; /* 192.168.1.1 */
    uint32_t dst_ip = 0xC0A80102; /* 192.168.1.2 */
    uint8_t dummy_seg[20] = {0};

    uint16_t csum = syn_tcp_checksum(src_ip, dst_ip, dummy_seg, sizeof(dummy_seg));
    TEST_ASSERT_NOT_EQUAL(0, csum);
}

void test_tcp_syn_ack_handshake(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;

    syn_eth_init(&eth, mac, ip);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    /* Construct raw SYN Ethernet frame */
    uint8_t syn_frame[54];
    memset(syn_frame, 0, sizeof(syn_frame));
    uint8_t client_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x66};
    memcpy(&syn_frame[0], mac, 6);        /* Dst MAC */
    memcpy(&syn_frame[6], client_mac, 6); /* Src MAC */
    syn_frame[12] = 0x08;
    syn_frame[13] = 0x00; /* IPv4 */

    syn_frame[14] = 0x45;
    syn_frame[16] = 0x00;
    syn_frame[17] = 40; /* Length 40 */
    syn_frame[23] = 6;  /* Protocol TCP */
    syn_frame[26] = 169;
    syn_frame[27] = 254;
    syn_frame[28] = 1;
    syn_frame[29] = 1; /* Src IP */
    syn_frame[30] = 169;
    syn_frame[31] = 254;
    syn_frame[32] = 1;
    syn_frame[33] = 100; /* Dst IP */

    syn_frame[34] = 0xC0;
    syn_frame[35] = 0x00; /* Src Port 49152 */
    syn_frame[36] = 0x00;
    syn_frame[37] = 80; /* Dst Port 80 */
    syn_frame[38] = 0x00;
    syn_frame[39] = 0x00;
    syn_frame[40] = 0x00;
    syn_frame[41] = 0x01;             /* Seq 1 */
    syn_frame[46] = 0x50;             /* Data Offset 5 (20 bytes) */
    syn_frame[47] = SYN_TCP_FLAG_SYN; /* SYN Flag */

    uint8_t tx_out[128];
    size_t tx_len = 0;

    SYN_Status st = syn_tcp_process_packet(&tcp, syn_frame, sizeof(syn_frame), tx_out, &tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);
    TEST_ASSERT_GREATER_THAN(0, tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_ACK, tx_out[47]);

    /* Now send ACK packet with matching seq_num and ack_num */
    syn_frame[41] = 0x02;
    syn_frame[42] = (uint8_t)(tcp.conns[0].seq_nxt >> 24);
    syn_frame[43] = (uint8_t)(tcp.conns[0].seq_nxt >> 16);
    syn_frame[44] = (uint8_t)(tcp.conns[0].seq_nxt >> 8);
    syn_frame[45] = (uint8_t)(tcp.conns[0].seq_nxt & 0xFFU);
    syn_frame[47] = SYN_TCP_FLAG_ACK;
    st = syn_tcp_process_packet(&tcp, syn_frame, sizeof(syn_frame), tx_out, &tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);
}

void test_tcp_task_unblock_on_data(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;

    syn_eth_init(&eth, mac, ip);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    tcp.conns[0].state = SYN_TCP_ESTABLISHED;
    tcp.conns[0].remote_ip = 0xA9FE0101;
    tcp.conns[0].remote_port = 49152;
    tcp.conns[0].local_port = 80;

    /* Setup task in SYN_TASK_BLOCKED state */
    task.state = SYN_TASK_BLOCKED;
    tcp.conns[0].blocked_task = &task;

    /* Construct Data frame */
    uint8_t data_frame[64];
    memset(data_frame, 0, sizeof(data_frame));
    data_frame[14] = 0x45;
    data_frame[16] = 0x00;
    data_frame[17] = 50; /* Length 50 (10 payload bytes) */
    data_frame[23] = 6;
    data_frame[26] = 169;
    data_frame[27] = 254;
    data_frame[28] = 1;
    data_frame[29] = 1;
    data_frame[30] = 169;
    data_frame[31] = 254;
    data_frame[32] = 1;
    data_frame[33] = 100;
    data_frame[34] = 0xC0;
    data_frame[35] = 0x00;
    data_frame[36] = 0x00;
    data_frame[37] = 80;
    data_frame[46] = 0x50;
    data_frame[47] = SYN_TCP_FLAG_PSH | SYN_TCP_FLAG_ACK;
    memcpy(&data_frame[54], "GET /ws", 7);

    uint8_t tx_out[128];
    size_t tx_len = 0;

    SYN_Status st = syn_tcp_process_packet(&tcp, data_frame, 64, tx_out, &tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, task.state); /* Task UNBLOCKED! */
    TEST_ASSERT_EQUAL(10, tcp.conns[0].rx_len);
}

/*
 * Regression: when rx_buf is full, the old code dropped payload bytes but still
 * advanced ack_nxt by the full payload_len — a false ACK.  Verify that ack_nxt
 * only reflects bytes actually accepted into the buffer.
 */
void test_tcp_rx_buf_full_no_false_ack(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;

    syn_eth_init(&eth, mac, ip);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    tcp.conns[0].state = SYN_TCP_ESTABLISHED;
    tcp.conns[0].remote_ip = 0xA9FE0101;
    tcp.conns[0].remote_port = 49152;
    tcp.conns[0].local_port = 80;
    tcp.conns[0].ack_nxt = 1000U;

    /* Fill the rx_buf completely */
    tcp.conns[0].rx_len = SYN_TCP_BUF_SIZE;

    /* Build a data frame carrying 5 bytes starting at seq 1000 */
    uint8_t data_frame[64];
    memset(data_frame, 0, sizeof(data_frame));
    data_frame[14] = 0x45;
    data_frame[16] = 0x00;
    data_frame[17] = 45; /* IP len = 20+20+5 */
    data_frame[23] = 6;
    data_frame[26] = 169;
    data_frame[27] = 254;
    data_frame[28] = 1;
    data_frame[29] = 1;
    data_frame[30] = 169;
    data_frame[31] = 254;
    data_frame[32] = 1;
    data_frame[33] = 100;
    data_frame[34] = 0xC0;
    data_frame[35] = 0x00; /* Src port 49152 */
    data_frame[36] = 0x00;
    data_frame[37] = 80; /* Dst port 80 */
    /* seq_num = 1000 */
    data_frame[38] = 0x00;
    data_frame[39] = 0x00;
    data_frame[40] = 0x03;
    data_frame[41] = 0xE8;
    data_frame[46] = 0x50;
    data_frame[47] = SYN_TCP_FLAG_PSH | SYN_TCP_FLAG_ACK;
    memcpy(&data_frame[54], "HELLO", 5);

    uint8_t tx_out[128];
    size_t tx_len = 0;

    SYN_Status st = syn_tcp_process_packet(&tcp, data_frame, 59, tx_out, &tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Buffer was full — ack_nxt must equal seq (1000), not seq+5 (1005) */
    TEST_ASSERT_EQUAL_UINT32(1000U, tcp.conns[0].ack_nxt);
    /* rx_len must remain at SYN_TCP_BUF_SIZE — nothing was accepted */
    TEST_ASSERT_EQUAL_UINT16((uint16_t)SYN_TCP_BUF_SIZE, tcp.conns[0].rx_len);
}

static void test_tcp_fin_close_handshake(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;

    syn_eth_init(&eth, mac, ip);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    /* Move connection to ESTABLISHED */
    tcp.conns[0].state = SYN_TCP_ESTABLISHED;
    tcp.conns[0].remote_port = 49152;
    tcp.conns[0].remote_ip = 0xA9FE0101;
    tcp.conns[0].ack_nxt = 256U;
    task.state = SYN_TASK_BLOCKED;
    tcp.conns[0].blocked_task = &task;

    /* Build raw FIN frame */
    uint8_t fin_frame[54];
    memset(fin_frame, 0, sizeof(fin_frame));
    memcpy(&fin_frame[0], mac, 6);
    fin_frame[12] = 0x08;
    fin_frame[13] = 0x00;
    fin_frame[14] = 0x45;
    fin_frame[17] = 40;
    fin_frame[23] = 6;
    fin_frame[26] = 169;
    fin_frame[27] = 254;
    fin_frame[28] = 1;
    fin_frame[29] = 1;
    fin_frame[30] = 169;
    fin_frame[31] = 254;
    fin_frame[32] = 1;
    fin_frame[33] = 100;
    fin_frame[34] = 0xC0;
    fin_frame[35] = 0x00;
    fin_frame[36] = 0x00;
    fin_frame[37] = 80;
    fin_frame[38] = 0x00;
    fin_frame[39] = 0x00;
    fin_frame[40] = 0x01;
    fin_frame[41] = 0x00; /* seq = 256 */
    fin_frame[46] = 0x50;
    fin_frame[47] = SYN_TCP_FLAG_FIN;

    uint8_t tx_out[128];
    size_t tx_len = 0;

    SYN_Status st = syn_tcp_process_packet(&tcp, fin_frame, sizeof(fin_frame), tx_out, &tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT32(257, tcp.conns[0].ack_nxt);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, task.state);
}

static void test_tcp_null_params_and_non_tcp_proto(void)
{
    SYN_TCP local_tcp;
    memset(&local_tcp, 0, sizeof(local_tcp));
    uint8_t frame[64] = {0};
    uint8_t tx_out[64];
    size_t tx_len = 0;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_tcp_init(NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_tcp_listen(NULL, 0));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_tcp_process_packet(NULL, frame, sizeof(frame), tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_tcp_process_packet(&local_tcp, NULL, sizeof(frame), tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_tcp_process_packet(&local_tcp, frame, 50, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_tcp_process_packet(&local_tcp, frame, sizeof(frame), NULL, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_tcp_process_packet(&local_tcp, frame, sizeof(frame), tx_out, NULL));

    /* Non-TCP frame (proto != 6) */
    memset(frame, 0, sizeof(frame));
    frame[14] = 0x45;
    frame[17] = 40;
    frame[23] = 17; /* UDP */
    frame[46] = 0x50;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_tcp_process_packet(&local_tcp, frame, sizeof(frame), tx_out, &tx_len));

    /* Unhandled TCP port -> SYN_ERROR */
    frame[23] = 6;
    frame[36] = 0x1F;
    frame[37] = 0x90; /* Dst Port 8080 */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_tcp_process_packet(&local_tcp, frame, sizeof(frame), tx_out, &tx_len));

    /* Test odd-length checksum calculation */
    uint8_t odd_data[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint16_t csum = syn_tcp_checksum(0xC0A80101, 0xC0A80102, odd_data, 5);
    TEST_ASSERT_NOT_EQUAL(0, csum);

    /* Test max capacity listeners */
    SYN_TCP full_tcp;
    syn_tcp_init(&full_tcp, &eth);
    for (uint16_t p = 1000; p < 1000 + SYN_TCP_MAX_CONNS; p++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_listen(&full_tcp, p));
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_listen(&full_tcp, 9999));

    /* Process packet for unmanaged port / listening port without match */
    frame[36] = 0x00;
    frame[37] = 80; /* Managed port 80, but state CLOSED (no match) */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_tcp_process_packet(&full_tcp, frame, sizeof(frame), tx_out, &tx_len));
}

typedef struct {
    SYN_TcpConn *conn;
    uint8_t buf[32];
    size_t max_len;
    size_t bytes_read;
} TcpBlockReadCtx;

static SYN_PT_Status tcp_block_read_coroutine(SYN_PT *pt, SYN_Task *t)
{
    TcpBlockReadCtx *ctx = (t != NULL) ? (TcpBlockReadCtx *)t->user_data : NULL;
    SYN_TcpConn *conn = (ctx != NULL) ? ctx->conn : NULL;
    uint8_t *buf = (ctx != NULL) ? ctx->buf : NULL;
    size_t max_len = (ctx != NULL) ? ctx->max_len : 0U;
    size_t *bytes_read = (ctx != NULL) ? &ctx->bytes_read : NULL;
    if (pt == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    PT_TCP_BLOCK_READ(pt, t, conn, buf, max_len, bytes_read);
    PT_END(pt);
}

static void test_tcp_pt_block_read_clears_blocked_task_and_null_guards(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task local_task = {0};

    /* 1. Reject NULL pt, NULL task, NULL user_data, and NULL conn */
    TEST_ASSERT_EQUAL(PT_EXITED, tcp_block_read_coroutine(NULL, &local_task));
    TEST_ASSERT_EQUAL(PT_EXITED, tcp_block_read_coroutine(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, tcp_block_read_coroutine(&pt, &local_task));

    TcpBlockReadCtx ctx = {0};
    ctx.conn = NULL;
    ctx.max_len = sizeof(ctx.buf);
    local_task.user_data = &ctx;
    TEST_ASSERT_EQUAL(PT_EXITED, tcp_block_read_coroutine(&pt, &local_task));

    /* 2. Establish connection, block via PT_TCP_BLOCK_READ, wake on RX data, verify
     *    conn->blocked_task is cleared to NULL after waking */
    test_tcp_syn_ack_handshake();
    SYN_TcpConn *conn = &tcp.conns[0];
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, conn->state);

    ctx.conn = conn;
    ctx.bytes_read = 0;
    memset(&local_task, 0, sizeof(local_task));
    local_task.state = SYN_TASK_READY;
    local_task.user_data = &ctx;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, tcp_block_read_coroutine(&pt, &local_task));
    TEST_ASSERT_EQUAL(SYN_TASK_BLOCKED, local_task.state);
    TEST_ASSERT_EQUAL_PTR(&local_task, conn->blocked_task);

    /* Deliver 4-byte TCP payload ("PING") to wake blocked coroutine */
    uint8_t data_frame[58];
    memset(data_frame, 0, sizeof(data_frame));
    uint8_t srv_mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t cli_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x66};
    memcpy(&data_frame[0], srv_mac, 6);
    memcpy(&data_frame[6], cli_mac, 6);
    data_frame[12] = 0x08;
    data_frame[13] = 0x00;
    data_frame[14] = 0x45;
    data_frame[16] = 0x00;
    data_frame[17] = 44; /* 20B IP + 20B TCP + 4B payload */
    data_frame[23] = 6;
    data_frame[26] = 169;
    data_frame[27] = 254;
    data_frame[28] = 1;
    data_frame[29] = 1;
    data_frame[30] = 169;
    data_frame[31] = 254;
    data_frame[32] = 1;
    data_frame[33] = 100;
    data_frame[34] = (uint8_t)(conn->remote_port >> 8);
    data_frame[35] = (uint8_t)(conn->remote_port & 0xFFU);
    data_frame[36] = 0x00;
    data_frame[37] = 80;
    data_frame[38] = (uint8_t)(conn->ack_nxt >> 24);
    data_frame[39] = (uint8_t)(conn->ack_nxt >> 16);
    data_frame[40] = (uint8_t)(conn->ack_nxt >> 8);
    data_frame[41] = (uint8_t)(conn->ack_nxt & 0xFFU);
    data_frame[46] = 0x50; /* Data offset = 5 (20B) */
    data_frame[47] = SYN_TCP_FLAG_ACK | SYN_TCP_FLAG_PSH;
    memcpy(&data_frame[54], "PING", 4);

    uint8_t tx_out[128];
    size_t tx_len = 0;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_tcp_process_packet(&tcp, data_frame, sizeof(data_frame), tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TASK_READY, local_task.state);

    /* Resume coroutine -> copies 4 bytes and clears conn->blocked_task == NULL */
    TEST_ASSERT_EQUAL(PT_ENDED, tcp_block_read_coroutine(&pt, &local_task));
    TEST_ASSERT_NULL(conn->blocked_task);
    TEST_ASSERT_EQUAL_UINT(4, ctx.bytes_read);
    TEST_ASSERT_EQUAL_MEMORY("PING", ctx.buf, 4);

    /* 3. Block again on empty rx_buf, then close connection -> wakes with bytes_read == 0
     *    and clears conn->blocked_task == NULL */
    ctx.bytes_read = 99;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, tcp_block_read_coroutine(&pt, &local_task));
    TEST_ASSERT_EQUAL_PTR(&local_task, conn->blocked_task);

    conn->state = SYN_TCP_CLOSED;
    local_task.state = SYN_TASK_READY;
    TEST_ASSERT_EQUAL(PT_ENDED, tcp_block_read_coroutine(&pt, &local_task));
    TEST_ASSERT_NULL(conn->blocked_task);
    TEST_ASSERT_EQUAL_UINT(0, ctx.bytes_read);
}

static void test_tcp_round3_bounds_rst_and_fin_psh(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    syn_eth_init(&eth, mac, 0xA9FE0164U);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    uint8_t pkt[70];
    uint8_t tx_out[128];
    size_t tx_len = 0;

    /* 1. R3-N02: Malformed IHL (< 20), IHL exceeding len, data_offset < 20, ip_len < hdrs,
     *    and ip_len exceeding wire len (prevents 512B OOB read) */
    memset(pkt, 0, sizeof(pkt));
    pkt[14] = 0x44; /* IHL = 16 < 20 */
    pkt[23] = 6;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    pkt[14] = 0x4F; /* IHL = 60 -> 14 + 60 + 20 = 94 > 54 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    pkt[14] = 0x45;
    pkt[17] = 40;
    pkt[46] = 0x40; /* data_offset = 16 < 20 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    pkt[46] = 0x50;
    pkt[17] = 30; /* ip_len = 30 < 20 + 20 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    pkt[16] = 0x02;
    pkt[17] = 0x00; /* ip_len = 512 > 54 - 14 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    /* 2. LISTEN non-SYN ignored, SYN with tcp.eth == NULL uses zero MAC fallback,
     *    SYN_RCVD non-ACK ignored */
    memset(pkt, 0, sizeof(pkt));
    pkt[14] = 0x45;
    pkt[17] = 40;
    pkt[23] = 6;
    pkt[26] = 10;
    pkt[29] = 1;
    pkt[34] = 0xC0;
    pkt[37] = 80;
    pkt[46] = 0x50;
    pkt[47] = SYN_TCP_FLAG_ACK; /* Non-SYN on LISTEN */
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_LISTEN, tcp.conns[0].state);

    tcp.eth = NULL;
    pkt[47] = SYN_TCP_FLAG_SYN;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);
    tcp.eth = &eth;

    pkt[47] = SYN_TCP_FLAG_PSH; /* Non-ACK in SYN_RCVD */
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);

    pkt[41] = 1U;
    pkt[42] = (uint8_t)(tcp.conns[0].seq_nxt >> 24);
    pkt[43] = (uint8_t)(tcp.conns[0].seq_nxt >> 16);
    pkt[44] = (uint8_t)(tcp.conns[0].seq_nxt >> 8);
    pkt[45] = (uint8_t)(tcp.conns[0].seq_nxt & 0xFFU);
    pkt[47] = SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);

    /* Pure 0-byte ACK in ESTABLISHED is a valid no-op */
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));

    /* 3. R3-N02: RST on ESTABLISHED closes connection and wakes blocked_task (clearing pointer) */
    SYN_Task rst_task = {0};
    rst_task.state = SYN_TASK_BLOCKED;
    tcp.conns[0].blocked_task = &rst_task;
    pkt[47] = SYN_TCP_FLAG_RST;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, rst_task.state);
    TEST_ASSERT_NULL(tcp.conns[0].blocked_task);

    /* 4. R3-N02: FIN+PSH+ACK with IPv4 options (IHL=24) delivers final payload BEFORE closing */
    syn_tcp_listen(&tcp, 80);
    tcp.conns[0].state = SYN_TCP_ESTABLISHED;
    tcp.conns[0].remote_ip = 0x0A000001U;
    tcp.conns[0].remote_port = 0xC000U;
    tcp.conns[0].local_port = 80;
    tcp.conns[0].ack_nxt = 100U;
    rst_task.state = SYN_TASK_BLOCKED;
    tcp.conns[0].blocked_task = &rst_task;

    memset(pkt, 0, sizeof(pkt));
    pkt[14] = 0x46; /* IPv4 IHL = 24 bytes (4B IP option) */
    pkt[17] = 48;   /* 24B IP + 20B TCP + 4B payload = 48 */
    pkt[23] = 6;
    pkt[26] = 10;
    pkt[29] = 1;
    /* TCP header starts at 14 + 24 = 38 */
    pkt[38] = 0xC0;
    pkt[39] = 0x00;
    pkt[40] = 0x00;
    pkt[41] = 80;
    pkt[45] = 100;  /* seq_num = 100 */
    pkt[50] = 0x50; /* data_offset = 20 */
    pkt[51] = SYN_TCP_FLAG_FIN | SYN_TCP_FLAG_PSH | SYN_TCP_FLAG_ACK;
    memcpy(&pkt[58], "LAST", 4);

    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 62, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT16(4, tcp.conns[0].rx_len);
    TEST_ASSERT_EQUAL_MEMORY("LAST", tcp.conns[0].rx_buf, 4);
    TEST_ASSERT_EQUAL_UINT32(105U, tcp.conns[0].ack_nxt);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, rst_task.state);
    TEST_ASSERT_NULL(tcp.conns[0].blocked_task);
}

static void test_tcp_round4_seq_ack_rst_and_partial_fin(void)
{
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    syn_eth_init(&eth, mac, 0xA9FE0164U);
    syn_tcp_init(&tcp, &eth);
    syn_tcp_listen(&tcp, 80);

    uint8_t pkt[70];
    uint8_t tx_out[128];
    size_t tx_len = 0;

    memset(pkt, 0, sizeof(pkt));
    pkt[14] = 0x45;
    pkt[17] = 40;
    pkt[23] = 6;
    pkt[26] = 10;
    pkt[29] = 1;
    pkt[34] = 0xC0;
    pkt[37] = 80;
    pkt[41] = 10U; /* seq_num = 10 */
    pkt[46] = 0x50;

    /* 1. RST on LISTEN ignored (even if SYN+RST) */
    pkt[47] = SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_RST;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_LISTEN, tcp.conns[0].state);

    /* Open SYN_RCVD (ack_nxt = 11) */
    pkt[47] = SYN_TCP_FLAG_SYN;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT32(11U, tcp.conns[0].ack_nxt);

    /* 2. SYN_RCVD rejects wrong seq_num, wrong ack_num, and off-sequence RST */
    pkt[41] = 99U; /* wrong seq_num */
    pkt[42] = (uint8_t)(tcp.conns[0].seq_nxt >> 24);
    pkt[43] = (uint8_t)(tcp.conns[0].seq_nxt >> 16);
    pkt[44] = (uint8_t)(tcp.conns[0].seq_nxt >> 8);
    pkt[45] = (uint8_t)(tcp.conns[0].seq_nxt & 0xFFU);
    pkt[47] = SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);

    pkt[41] = 11U; /* matching seq_num, wrong ack_num */
    pkt[45] ^= 0xFFU;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);

    pkt[41] = 99U; /* off-sequence RST ignored */
    pkt[47] = SYN_TCP_FLAG_RST;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);

    /* Matching ACK completes handshake */
    pkt[41] = 11U;
    pkt[42] = (uint8_t)(tcp.conns[0].seq_nxt >> 24);
    pkt[43] = (uint8_t)(tcp.conns[0].seq_nxt >> 16);
    pkt[44] = (uint8_t)(tcp.conns[0].seq_nxt >> 8);
    pkt[45] = (uint8_t)(tcp.conns[0].seq_nxt & 0xFFU);
    pkt[47] = SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);

    /* 3. Out-of-order pure ACK, payload, FIN, and RST rejected in ESTABLISHED */
    pkt[41] = 50U;
    pkt[47] = SYN_TCP_FLAG_ACK;
    tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL_size_t(0U, tx_len);

    pkt[47] = SYN_TCP_FLAG_FIN | SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);
    TEST_ASSERT_GREATER_THAN_size_t(0U, tx_len);

    pkt[17] = 44; /* 4B out-of-order payload */
    pkt[47] = SYN_TCP_FLAG_PSH | SYN_TCP_FLAG_ACK;
    memcpy(&pkt[54], "SKIP", 4);
    tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 58, tx_out, &tx_len));
    TEST_ASSERT_EQUAL_UINT16(0U, tcp.conns[0].rx_len);
    TEST_ASSERT_EQUAL_UINT32(11U, tcp.conns[0].ack_nxt);
    TEST_ASSERT_GREATER_THAN_size_t(0U, tx_len);

    /* 4. Partial-buffer FIN+PSH+ACK accepts space (2B) without closing until full payload fits */
    tcp.conns[0].rx_len = SYN_TCP_BUF_SIZE - 2U;
    pkt[17] = 45; /* 5B payload "ABCDE" */
    pkt[41] = 11U;
    pkt[47] = SYN_TCP_FLAG_FIN | SYN_TCP_FLAG_PSH | SYN_TCP_FLAG_ACK;
    memcpy(&pkt[54], "ABCDE", 5);
    tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 59, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT16(SYN_TCP_BUF_SIZE, tcp.conns[0].rx_len);
    TEST_ASSERT_EQUAL_UINT32(13U, tcp.conns[0].ack_nxt);
    TEST_ASSERT_EQUAL_UINT8(SYN_TCP_FLAG_ACK, tx_out[47]);

    /* Drain rx_buf and retransmit remaining 3B ("CDE") + FIN at seq 13 -> closes cleanly */
    tcp.conns[0].rx_len = 0U;
    pkt[17] = 43;
    pkt[41] = 13U;
    memcpy(&pkt[54], "CDE", 3);
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 57, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT16(3U, tcp.conns[0].rx_len);
    TEST_ASSERT_EQUAL_UINT32(17U, tcp.conns[0].ack_nxt);
}

static SYN_PT_Status tcp_stack_array_coroutine(SYN_PT *pt, SYN_Task *t)
{
    uint8_t stack_buf[16];
    size_t n = 0U;
    PT_BEGIN(pt);
    PT_TCP_BLOCK_READ(pt, t, &tcp.conns[0], stack_buf, sizeof(stack_buf), &n);
    TEST_ASSERT_EQUAL_size_t(2U, n);
    TEST_ASSERT_EQUAL_MEMORY("OK", stack_buf, 2U);
    PT_END(pt);
}

static void test_r5_n07_tcp_multi_accept_close_and_waddress(void)
{
    SYN_PT pt;
    SYN_Task t = {0};
    uint8_t buf[8] = {0};
    size_t br = 0U;

    /* 1. Exercise all syn_tcp_pt_args_valid NULL guards */
    PT_INIT(&pt);
    TEST_ASSERT_FALSE(syn_tcp_pt_args_valid(NULL, &t, &tcp.conns[0], buf, &br));
    TEST_ASSERT_FALSE(syn_tcp_pt_args_valid(&pt, NULL, &tcp.conns[0], buf, &br));
    TEST_ASSERT_FALSE(syn_tcp_pt_args_valid(&pt, &t, NULL, buf, &br));
    TEST_ASSERT_FALSE(syn_tcp_pt_args_valid(&pt, &t, &tcp.conns[0], NULL, &br));
    TEST_ASSERT_FALSE(syn_tcp_pt_args_valid(&pt, &t, &tcp.conns[0], buf, NULL));
    TEST_ASSERT_TRUE(syn_tcp_pt_args_valid(&pt, &t, &tcp.conns[0], buf, &br));

    /* 2. Client 1 connects & closes via FIN -> Client 2 connects without re-init! */
    test_tcp_fin_close_handshake();
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);
    TEST_ASSERT_TRUE(tcp.listening);
    TEST_ASSERT_EQUAL_UINT16(80U, tcp.listen_port);

    uint8_t pkt[60] = {0};
    uint8_t tx_out[128];
    size_t tx_len = 0U;
    pkt[12] = 0x08;
    pkt[13] = 0x00;
    pkt[14] = 0x45;
    pkt[17] = 40;
    pkt[23] = 6;
    pkt[26] = 10;
    pkt[27] = 0;
    pkt[28] = 0;
    pkt[29] = 42;
    pkt[30] = 169;
    pkt[31] = 254;
    pkt[32] = 1;
    pkt[33] = 100;
    pkt[34] = 0xD0;
    pkt[35] = 0x01; /* Client 2 src port 53249 */
    pkt[36] = 0x00;
    pkt[37] = 80; /* dst port 80 */
    pkt[41] = 100U;
    pkt[46] = 0x50;

    /* Stray ACK or SYN+RST on closed listener must not re-arm */
    pkt[47] = SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54U, tx_out, &tx_len));
    pkt[47] = SYN_TCP_FLAG_SYN | SYN_TCP_FLAG_RST;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54U, tx_out, &tx_len));

    /* Valid Client 2 SYN re-arms closed listener into SYN_RCVD -> ESTABLISHED */
    pkt[47] = SYN_TCP_FLAG_SYN;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54U, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, tcp.conns[0].state);
    TEST_ASSERT_GREATER_THAN_size_t(0U, tx_len);

    pkt[41] = 101U;
    pkt[42] = (uint8_t)(tcp.conns[0].seq_nxt >> 24U);
    pkt[43] = (uint8_t)(tcp.conns[0].seq_nxt >> 16U);
    pkt[44] = (uint8_t)(tcp.conns[0].seq_nxt >> 8U);
    pkt[45] = (uint8_t)(tcp.conns[0].seq_nxt & 0xFFU);
    pkt[47] = SYN_TCP_FLAG_ACK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&tcp, pkt, 54U, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_ESTABLISHED, tcp.conns[0].state);

    /* Client 3 SYN while Client 2 holds sole slot -> rejected cleanly */
    pkt[35] = 0x02; /* port 53250 */
    pkt[47] = SYN_TCP_FLAG_SYN;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_tcp_process_packet(&tcp, pkt, 54U, tx_out, &tx_len));

    /* Stack-array PT_TCP_BLOCK_READ (-Werror=address check) */
    memcpy(tcp.conns[0].rx_buf, "OK", 2U);
    tcp.conns[0].rx_len = 2U;
    PT_INIT(&pt);
    t.state = SYN_TASK_READY;
    TEST_ASSERT_EQUAL(PT_ENDED, tcp_stack_array_coroutine(&pt, &t));

    /* 3. syn_tcp_close parameter checks & listener re-arm vs non-listener close */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_tcp_close(NULL, &tcp.conns[0]));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_tcp_close(&tcp, NULL));

    t.state = SYN_TASK_BLOCKED;
    tcp.conns[0].blocked_task = &t;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_close(&tcp, &tcp.conns[0]));
    TEST_ASSERT_EQUAL(SYN_TCP_LISTEN, tcp.conns[0].state);
    TEST_ASSERT_EQUAL(SYN_TASK_READY, t.state);

    tcp.conns[0].local_port = 8080U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_close(&tcp, &tcp.conns[0]));
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);

    tcp.listening = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_close(&tcp, &tcp.conns[0]));
    TEST_ASSERT_EQUAL(SYN_TCP_CLOSED, tcp.conns[0].state);

    /* 4. Multi-port listener table + NULL tcp.eth safety on listen and re-arm */
    SYN_TCP headless_tcp = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_listen(&headless_tcp, 443U));
    TEST_ASSERT_EQUAL_UINT32(0U, headless_tcp.conns[0].local_ip);
    headless_tcp.listen_port = 8443U; /* listen_ports[0] still retains 443U */
    headless_tcp.conns[0].state = SYN_TCP_CLOSED;
    pkt[36] = 0x01U;
    pkt[37] = 0xBBU; /* dst_port = 443 */
    pkt[47] = SYN_TCP_FLAG_SYN;
    TEST_ASSERT_EQUAL(SYN_OK, syn_tcp_process_packet(&headless_tcp, pkt, 54U, tx_out, &tx_len));
    TEST_ASSERT_EQUAL(SYN_TCP_SYN_RCVD, headless_tcp.conns[0].state);
    TEST_ASSERT_EQUAL_UINT16(443U, headless_tcp.conns[0].local_port);
}

void run_tcp_tests(void)
{
    RUN_TEST(test_tcp_init_and_listen);
    RUN_TEST(test_tcp_checksum_calc);
    RUN_TEST(test_tcp_syn_ack_handshake);
    RUN_TEST(test_tcp_task_unblock_on_data);
    RUN_TEST(test_tcp_rx_buf_full_no_false_ack);
    RUN_TEST(test_tcp_fin_close_handshake);
    RUN_TEST(test_tcp_null_params_and_non_tcp_proto);
    RUN_TEST(test_tcp_pt_block_read_clears_blocked_task_and_null_guards);
    RUN_TEST(test_tcp_round3_bounds_rst_and_fin_psh);
    RUN_TEST(test_tcp_round4_seq_ack_rst_and_partial_fin);
    RUN_TEST(test_r5_n07_tcp_multi_accept_close_and_waddress);
}
