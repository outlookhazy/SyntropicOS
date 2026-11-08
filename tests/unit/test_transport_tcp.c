#include "mocks/mock_port.h"
#include "syntropic/net/syn_transport_tcp.h"
#include "unity/unity.h"

#include <string.h>

void test_transport_tcp_send(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    TEST_ASSERT_TRUE(syn_transport_send(&t, data, sizeof(data)));

    /* Verify 2-byte length + payload */
    TEST_ASSERT_EQUAL_UINT32(5, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x03, mock_sock_tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0xAA, mock_sock_tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, mock_sock_tx_buf[3]);
    TEST_ASSERT_EQUAL_UINT8(0xCC, mock_sock_tx_buf[4]);
}

void test_transport_tcp_recv_full(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    uint8_t rx_data[] = {0x00, 0x04, 0x11, 0x22, 0x33, 0x44};
    mock_sock_set_response(rx_data, sizeof(rx_data));

    uint8_t out[16];
    size_t out_len = 0;
    TEST_ASSERT_TRUE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT32(4, out_len);
    TEST_ASSERT_EQUAL_UINT8(0x11, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0x22, out[1]);
    TEST_ASSERT_EQUAL_UINT8(0x33, out[2]);
    TEST_ASSERT_EQUAL_UINT8(0x44, out[3]);
}

void test_transport_tcp_recv_fragmented(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    uint8_t out[16];
    size_t out_len = 0;

    /* 1. Feed length MSB */
    uint8_t chunk1[] = {0x00};
    mock_sock_set_response(chunk1, 1);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(1, tcp.state);

    /* 2. Feed length LSB */
    uint8_t chunk2[] = {0x03};
    mock_sock_set_response(chunk2, 1);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(2, tcp.state);
    TEST_ASSERT_EQUAL_UINT16(3, tcp.payload_len);

    /* 3. Feed partial payload */
    uint8_t chunk3[] = {0xAA, 0xBB};
    mock_sock_set_response(chunk3, 2);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(2, tcp.state);
    TEST_ASSERT_EQUAL_UINT16(2, tcp.bytes_read);

    /* 4. Feed final payload byte */
    uint8_t chunk4[] = {0xCC};
    mock_sock_set_response(chunk4, 1);
    TEST_ASSERT_TRUE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT32(3, out_len);
    TEST_ASSERT_EQUAL_UINT8(0xAA, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, out[1]);
    TEST_ASSERT_EQUAL_UINT8(0xCC, out[2]);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);
}

/** send fails on header write — exercises line 36 */
static void test_transport_tcp_send_header_fail(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;
    mock_sock_send_fail = true; /* fail immediately */

    uint8_t data[] = {0x01};
    bool ok = syn_transport_send(&t, data, sizeof(data));
    TEST_ASSERT_FALSE(ok);
    mock_sock_send_fail = false;
}

/** send fails on payload write — exercises line 41 */
static void test_transport_tcp_send_payload_fail(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;
    /* Fail after 2 bytes (the header) so the payload write fails */
    mock_sock_send_fail_after_bytes = 2;

    uint8_t data[] = {0x01, 0x02};
    bool ok = syn_transport_send(&t, data, sizeof(data));
    TEST_ASSERT_FALSE(ok);
    mock_sock_send_fail_after_bytes = -1;
}

/** recv: oversized payload length — exercises lines 84-85 */
static void test_transport_tcp_recv_oversized(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    /* Send a length header larger than the internal rx_buf */
    uint8_t rx[] = {0xFF, 0xFF}; /* length = 65535, far exceeds rx_buf */
    mock_sock_set_response(rx, sizeof(rx));

    uint8_t out[16];
    size_t out_len;
    /* State 0: read high byte */
    bool ok = syn_transport_recv(&t, out, sizeof(out), &out_len);
    TEST_ASSERT_FALSE(ok);
    /* State 1→oversized: resets to 0 */
    mock_sock_set_response(rx + 1, 1);
    ok = syn_transport_recv(&t, out, sizeof(out), &out_len);
    TEST_ASSERT_FALSE(ok); /* oversized → reset to state 0 */
}

/** recv: zero-length (empty) packet completes in a single call */
static void test_transport_tcp_recv_empty_packet(void)
{
    mock_port_reset();
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    /* Length = 0x0000: completes immediately after reading 2-byte header */
    uint8_t rx[] = {0x00, 0x00};
    mock_sock_set_response(rx, sizeof(rx));

    uint8_t out[16];
    size_t out_len = 99;
    TEST_ASSERT_TRUE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_size_t(0, out_len);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);

    /* Zero-length send writes 2-byte header only */
    mock_sock_tx_len = 0;
    TEST_ASSERT_TRUE(syn_transport_send(&t, NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(2, mock_sock_tx_len);
}
/** recv: completed packet larger than output buffer — exercises lines 109-110 */
static void test_transport_tcp_recv_outbuf_too_small(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    /* Frame a 5-byte payload: length header = 0x0005, then 5 data bytes */
    uint8_t rx[] = {0x00, 0x05, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    mock_sock_set_response(rx, sizeof(rx));

    /* But provide a 2-byte output buffer — smaller than payload */
    uint8_t out[2];
    size_t out_len;
    bool ok = false;
    for (int i = 0; i < 10 && !ok; i++) {
        ok = syn_transport_recv(&t, out, sizeof(out), &out_len);
    }
    /* Completed packet but output buffer too small → returns false (lines 109-110) */
    TEST_ASSERT_FALSE(ok);
}

/** send/recv with invalid socket handle — exercises lines 31 and 66 */
static void test_transport_tcp_invalid_socket(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, SYN_SOCKET_INVALID);

    uint8_t dummy[4] = {1, 2, 3, 4};
    size_t out_len = 0;
    TEST_ASSERT_FALSE(syn_transport_send(&t, dummy, 4));
    TEST_ASSERT_FALSE(syn_transport_recv(&t, dummy, sizeof(dummy), &out_len));
}

static void test_transport_tcp_has_data(void)
{
    mock_port_reset();
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    /* Socket invalid or NULL context -> false */
    SYN_Transport t_inv;
    SYN_TransportTcp tcp_inv;
    syn_transport_tcp_init(&t_inv, &tcp_inv, SYN_SOCKET_INVALID);
    TEST_ASSERT_FALSE(syn_transport_has_data(&t_inv));
    SYN_Transport t_null_ctx = {.has_data = t.has_data, .ctx = NULL};
    TEST_ASSERT_FALSE(syn_transport_has_data(&t_null_ctx));
    TEST_ASSERT_FALSE(syn_transport_has_data(NULL));
    TEST_ASSERT_FALSE(syn_transport_send(NULL, NULL, 0));
    SYN_Transport t_nosend = {0};
    TEST_ASSERT_FALSE(syn_transport_send(&t_nosend, (const uint8_t *)"a", 1));

    /* Socket connected, no data ready -> false */
    TEST_ASSERT_FALSE(syn_transport_has_data(&t));

    /* Socket connected with data -> true */
    uint8_t rx[] = {0x00, 0x05};
    mock_sock_set_response(rx, sizeof(rx));
    TEST_ASSERT_TRUE(syn_transport_has_data(&t));

    /* Partial packet in progress (tcp.state > 0) with NO unread socket bytes -> MUST be false
     * (R3-N01 regression: prevents cooperative PT_MORE / PT_WAIT_UNTIL spin) */
    uint8_t out[16];
    size_t out_len = 0;
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_TRUE(tcp.state > 0);
    TEST_ASSERT_FALSE(syn_transport_has_data(&t));

    /* Arrival of remaining bytes makes has_data true again */
    uint8_t rem[] = {1, 2, 3, 4, 5};
    mock_sock_set_response(rem, sizeof(rem));
    TEST_ASSERT_TRUE(syn_transport_has_data(&t));
    TEST_ASSERT_TRUE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_size_t(5, out_len);

    /* t->has_data == NULL hook -> true */
    t.has_data = NULL;
    TEST_ASSERT_TRUE(syn_transport_has_data(&t));
}

static void test_transport_tcp_eof_and_recv_error(void)
{
    mock_port_reset();
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 5);
    mock_sock_connected = true;

    /* 1. Partial header followed by EOF (n == 0) closes and invalidates socket */
    uint8_t hdr1[] = {0x00};
    mock_sock_set_response(hdr1, 1);
    uint8_t out[16];
    size_t out_len = 0;
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(1, tcp.state);

    mock_sock_eof_on_empty = true;
    TEST_ASSERT_TRUE(syn_transport_has_data(&t));
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, tcp.sock);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);
    TEST_ASSERT_FALSE(syn_transport_has_data(&t));

    /* 2. Readable socket returning recv < 0 (mock_sock_recv_fail) closes and invalidates socket */
    mock_port_reset();
    syn_transport_tcp_init(&t, &tcp, 6);
    mock_sock_connected = true;
    mock_sock_recv_fail = true;
    TEST_ASSERT_TRUE(syn_transport_has_data(&t));
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, tcp.sock);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);
    TEST_ASSERT_FALSE(syn_transport_has_data(&t));
}

static void test_transport_tcp_null_data_and_oversized_close(void)
{
    mock_port_reset();
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 7);
    mock_sock_connected = true;

    /* R4-N02: len > 0 with data == NULL rejected before writing 2-byte length header */
    TEST_ASSERT_FALSE(syn_transport_send(&t, NULL, 4));
    TEST_ASSERT_EQUAL_size_t(0, mock_sock_tx_len);

    /* R4-N02: payload_len (129) > sizeof(tcp.rx_buf) (128) closes socket to prevent desync */
    uint8_t stream[] = {0x00, 0x81, 0x00, 0x02, 'O', 'K'};
    mock_sock_set_response(stream, sizeof(stream));

    uint8_t out[16];
    size_t out_len = 0;
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, tcp.sock);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);
    TEST_ASSERT_FALSE(mock_sock_connected);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
}

void run_transport_tcp_tests(void)
{
    RUN_TEST(test_transport_tcp_send);
    RUN_TEST(test_transport_tcp_recv_full);
    RUN_TEST(test_transport_tcp_recv_fragmented);
    RUN_TEST(test_transport_tcp_send_header_fail);
    RUN_TEST(test_transport_tcp_send_payload_fail);
    RUN_TEST(test_transport_tcp_recv_oversized);
    RUN_TEST(test_transport_tcp_recv_empty_packet);
    RUN_TEST(test_transport_tcp_recv_outbuf_too_small);
    RUN_TEST(test_transport_tcp_invalid_socket);
    RUN_TEST(test_transport_tcp_has_data);
    RUN_TEST(test_transport_tcp_eof_and_recv_error);
    RUN_TEST(test_transport_tcp_null_data_and_oversized_close);
}
