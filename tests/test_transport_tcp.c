#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/net/syn_transport_tcp.h"
#include <string.h>

void test_transport_tcp_send(void)
{
    SYN_Transport t;
    SYN_TransportTcp tcp;
    syn_transport_tcp_init(&t, &tcp, 1);
    mock_sock_connected = true;

    uint8_t data[] = { 0xAA, 0xBB, 0xCC };
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

    uint8_t rx_data[] = { 0x00, 0x04, 0x11, 0x22, 0x33, 0x44 };
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
    uint8_t chunk1[] = { 0x00 };
    mock_sock_set_response(chunk1, 1);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(1, tcp.state);

    /* 2. Feed length LSB */
    uint8_t chunk2[] = { 0x03 };
    mock_sock_set_response(chunk2, 1);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(2, tcp.state);
    TEST_ASSERT_EQUAL_UINT16(3, tcp.payload_len);

    /* 3. Feed partial payload */
    uint8_t chunk3[] = { 0xAA, 0xBB };
    mock_sock_set_response(chunk3, 2);
    TEST_ASSERT_FALSE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT8(2, tcp.state);
    TEST_ASSERT_EQUAL_UINT16(2, tcp.bytes_read);

    /* 4. Feed final payload byte */
    uint8_t chunk4[] = { 0xCC };
    mock_sock_set_response(chunk4, 1);
    TEST_ASSERT_TRUE(syn_transport_recv(&t, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_UINT32(3, out_len);
    TEST_ASSERT_EQUAL_UINT8(0xAA, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, out[1]);
    TEST_ASSERT_EQUAL_UINT8(0xCC, out[2]);
    TEST_ASSERT_EQUAL_UINT8(0, tcp.state);
}

void run_transport_tcp_tests(void)
{
    RUN_TEST(test_transport_tcp_send);
    RUN_TEST(test_transport_tcp_recv_full);
    RUN_TEST(test_transport_tcp_recv_fragmented);
}
