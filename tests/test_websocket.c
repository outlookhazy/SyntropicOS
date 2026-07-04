#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/net/syn_websocket.h"
#include <string.h>

static int s_msg_callback_count = 0;
static uint8_t s_last_payload[128];
static size_t s_last_len = 0;
static uint8_t s_last_opcode = 0;

static void on_ws_message(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx)
{
    (void)ctx;
    s_msg_callback_count++;
    s_last_len = len < sizeof(s_last_payload) ? len : sizeof(s_last_payload);
    memcpy(s_last_payload, payload, s_last_len);
    s_last_opcode = opcode;
}

void test_websocket_upgrade(void)
{
    mock_port_reset();

    /* 1. Simulate httpd headers parsed into response buffer */
    const char *headers =
        "GET /chat HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "\r\n";

    SYN_HttpdResponse resp;
    resp.sock = 11;
    resp.buf = (uint8_t *)headers; /* raw request headers for parser */
    resp.buf_size = strlen(headers);
    resp.headers_sent = false;
    resp.upgraded = false;

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.path = "/chat";
    req.method = SYN_HTTP_GET;
    req.headers = headers;

    SYN_WebsocketSession ws;
    mock_sock_connected = true;

    SYN_Status st = syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL);

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_TRUE(resp.upgraded);
    TEST_ASSERT_EQUAL(11, ws.sock);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CONNECTED, ws.state);

    /* Verify switching protocols header & key accept value */
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    const char *tx = (const char *)mock_sock_tx_buf;
    TEST_ASSERT_NOT_NULL(strstr(tx, "101 Switching Protocols"));
    TEST_ASSERT_NOT_NULL(strstr(tx, "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));
}

void test_websocket_send(void)
{
    mock_port_reset();
    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;

    mock_sock_connected = true;
    const char *msg = "hello";
    SYN_Status st = syn_websocket_send(&ws, 0x01, msg, strlen(msg));

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT32(7, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x81, mock_sock_tx_buf[0]); /* FIN=1, OP=1 (text) */
    TEST_ASSERT_EQUAL_UINT8(0x05, mock_sock_tx_buf[1]); /* MASK=0, LEN=5 */
    TEST_ASSERT_EQUAL_STRING_LEN("hello", &mock_sock_tx_buf[2], 5);
}

void test_websocket_recv_masked_text(void)
{
    mock_port_reset();
    s_msg_callback_count = 0;

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.on_message = on_ws_message;

    /* Text frame: "hello" masked with key 0x11, 0x22, 0x33, 0x44 */
    /* "hello" = 0x68, 0x65, 0x6C, 0x6C, 0x6F */
    /* Masked: 0x68^0x11 = 0x79, 0x65^0x22 = 0x47, 0x6C^0x33 = 0x5F, 0x6C^0x44 = 0x28, 0x6F^0x11 = 0x7E */
    uint8_t frame[] = {
        0x81,                   /* FIN=1, Opcode=1 */
        0x85,                   /* Mask=1, Len=5 */
        0x11, 0x22, 0x33, 0x44, /* Masking Key */
        0x79, 0x47, 0x5F, 0x28, 0x7E /* Masked Data */
    };
    mock_sock_set_response(frame, sizeof(frame));
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task;
    task.user_data = &ws;

    /* Run task repeatedly until all bytes read */
    for (int i = 0; i < (int)sizeof(frame); i++) {
        syn_websocket_task(&pt, &task);
    }

    TEST_ASSERT_EQUAL(1, s_msg_callback_count);
    TEST_ASSERT_EQUAL_UINT32(5, s_last_len);
    TEST_ASSERT_EQUAL(0x01, s_last_opcode);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", s_last_payload, 5);
}

void test_websocket_ping_pong(void)
{
    mock_port_reset();

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;

    /* Ping frame with "ping" data, unmasked for simple server rx simulation */
    uint8_t frame[] = {
        0x89,                   /* FIN=1, Opcode=9 (ping) */
        0x04,                   /* Mask=0, Len=4 */
        0x70, 0x69, 0x6E, 0x67  /* "ping" */
    };
    mock_sock_set_response(frame, sizeof(frame));
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task;
    task.user_data = &ws;

    for (int i = 0; i < (int)sizeof(frame); i++) {
        syn_websocket_task(&pt, &task);
    }

    /* Verify it replied with PONG frame (Opcode=0x0A) */
    TEST_ASSERT_EQUAL_UINT32(6, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x8A, mock_sock_tx_buf[0]); /* FIN=1, OP=0x0A (pong) */
    TEST_ASSERT_EQUAL_UINT8(0x04, mock_sock_tx_buf[1]); /* LEN=4 */
    TEST_ASSERT_EQUAL_STRING_LEN("ping", &mock_sock_tx_buf[2], 4);
}

/** Long WS key — SHA1 processes >64 bytes, exercises line 114 (multi-block) */
static void test_websocket_upgrade_long_key(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    /* Key: ~100 chars + GUID 36 chars = ~136 bytes → exceeds 128, so
     * sha1_update processes 2 full blocks and hits the inner loop (line 114) */
    const char *headers =
        "GET /chat HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA_EXTRA\r\n"
        "\r\n";

    SYN_HttpdResponse resp;
    resp.sock = 11;
    resp.buf = (uint8_t *)headers;
    resp.buf_size = strlen(headers);
    resp.headers_sent = false;
    resp.upgraded = false;

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.path = "/chat";
    req.method = SYN_HTTP_GET;
    req.headers = headers;

    SYN_WebsocketSession ws;
    SYN_Status st = syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL);
    /* Long key should still produce a valid upgrade (SHA-1 multi-block) */
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

/** Upgrade without Sec-WebSocket-Key — exercises line 229 */
static void test_websocket_upgrade_no_key(void)
{

    mock_port_reset();
    mock_sock_connected = true;

    const char *headers =
        "GET /chat HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "\r\n"; /* No Sec-WebSocket-Key */

    SYN_HttpdResponse resp;
    resp.sock = 11;
    resp.buf = (uint8_t *)headers;
    resp.buf_size = strlen(headers);
    resp.headers_sent = false;
    resp.upgraded = false;

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.path = "/chat";
    req.method = SYN_HTTP_GET;
    req.headers = headers;

    SYN_WebsocketSession ws;
    SYN_Status st = syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL);
    TEST_ASSERT_EQUAL(SYN_ERROR, st); /* key not found */
}

/** Send a frame with len in [126..65535] — exercises lines 288-292 */
static void test_websocket_send_medium_frame(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    /* Build a connected session directly */
    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 5;
    ws.state = SYN_WS_STATE_CONNECTED;

    /* Send 200 bytes (>125, so it uses the 126-style 2-byte length) */
    static uint8_t payload[200];
    memset(payload, 'A', sizeof(payload));
    SYN_Status st = syn_websocket_send(&ws, 0x01, payload, sizeof(payload));
    TEST_ASSERT_EQUAL(SYN_OK, st);
    /* Header byte 1 should be 126 */
    TEST_ASSERT_EQUAL_UINT8(126, mock_sock_tx_buf[1]);
    /* Header bytes 2-3 should be big-endian 200 */
    TEST_ASSERT_EQUAL_UINT8(0, mock_sock_tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(200, mock_sock_tx_buf[3]);
}

/** Send header fails — exercises lines 299-300 */
static void test_websocket_send_header_fail(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 5;
    ws.state = SYN_WS_STATE_CONNECTED;

    mock_sock_send_fail = true;
    uint8_t d = 0x42;
    SYN_Status st = syn_websocket_send(&ws, 0x01, &d, 1);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    mock_sock_send_fail = false;
}

/** Send payload fails — exercises lines 305-306 */
static void test_websocket_send_payload_fail(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 5;
    ws.state = SYN_WS_STATE_CONNECTED;

    /* Fail after the 2-byte header is sent but payload send fails */
    mock_sock_send_fail_after_bytes = 2;
    uint8_t d = 0x42;
    SYN_Status st = syn_websocket_send(&ws, 0x01, &d, 1);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    mock_sock_send_fail_after_bytes = -1;
}
/** Recv: 126-length unmasked frame — exercises lines 341-344, 352-357 */
static void test_websocket_recv_extended_len(void)
{
    mock_port_reset();
    s_msg_callback_count = 0;

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.on_message = on_ws_message;
    mock_sock_connected = true;

    /* Unmasked text frame with 2-byte extended length = 200 bytes */
    uint8_t frame[204];
    frame[0] = 0x81;  /* FIN=1, text */
    frame[1] = 0x7E;  /* Mask=0, Len=126 → use 2-byte ext length */
    frame[2] = 0x00;  /* high byte of 200 */
    frame[3] = 0xC8;  /* low byte of 200 (0xC8 = 200) */
    memset(&frame[4], 'B', 200);
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &ws;
    for (int i = 0; i < 300; i++) {
        syn_websocket_task(&pt, &task);
    }
    TEST_ASSERT_EQUAL(1, s_msg_callback_count);
    TEST_ASSERT_EQUAL(0x01, s_last_opcode);
}

/** Recv: close frame — exercises lines 378-380 */
static void test_websocket_recv_close(void)
{
    mock_port_reset();

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;

    /* Close frame with 2-byte status code (RFC 6455: close frames carry a 2-byte code) */
    uint8_t frame[] = { 0x88, 0x02, 0x03, 0xE8 }; /* opcode=8, len=2, status=1000 (normal) */
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &ws;
    for (int i = 0; i < 10; i++) {
        syn_websocket_task(&pt, &task);
    }
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
}

/** Recv: peer disconnects (recv returns 0) — exercises lines 397-398 */
static void test_websocket_recv_peer_disconnect(void)
{
    mock_port_reset();

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true; /* recv returns 0 instead of -1 */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &ws;
    syn_websocket_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    mock_sock_eof_on_empty = false;
}

/** Recv: too-large frame (len==127) — exercises lines 347-349 */
static void test_websocket_recv_too_large(void)
{
    mock_port_reset();

    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;

    /* Frame with len=127 (8-byte extended length, unsupported) */
    uint8_t frame[] = { 0x81, 0x7F };
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &ws;
    for (int i = 0; i < 5; i++) {
        syn_websocket_task(&pt, &task);
    }
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
}

static void test_websocket_send_too_large(void)
{
    SYN_WebsocketSession ws;
    ws.state = SYN_WS_STATE_CONNECTED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_send(&ws, 0x01, NULL, 0x10000));
}

void run_websocket_tests(void)
{
    RUN_TEST(test_websocket_upgrade);
    RUN_TEST(test_websocket_send);
    RUN_TEST(test_websocket_recv_masked_text);
    RUN_TEST(test_websocket_ping_pong);
    RUN_TEST(test_websocket_upgrade_long_key);
    RUN_TEST(test_websocket_upgrade_no_key);
    RUN_TEST(test_websocket_send_medium_frame);
    RUN_TEST(test_websocket_send_header_fail);
    RUN_TEST(test_websocket_send_payload_fail);
    RUN_TEST(test_websocket_recv_extended_len);
    RUN_TEST(test_websocket_recv_close);
    RUN_TEST(test_websocket_recv_peer_disconnect);
    RUN_TEST(test_websocket_recv_too_large);
    RUN_TEST(test_websocket_send_too_large);
}
