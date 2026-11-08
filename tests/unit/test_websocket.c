#include "mocks/mock_port.h"
#include "syntropic/net/syn_websocket.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

#include <stdio.h>
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
    const char *headers = "GET /chat HTTP/1.1\r\n"
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

    SYN_WebsocketSession ws = {.sock = SYN_SOCKET_INVALID};
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
    /* Masked: 0x68^0x11 = 0x79, 0x65^0x22 = 0x47, 0x6C^0x33 = 0x5F, 0x6C^0x44 = 0x28, 0x6F^0x11 =
     * 0x7E */
    uint8_t frame[] = {
        0x81,                        /* FIN=1, Opcode=1 */
        0x85,                        /* Mask=1, Len=5 */
        0x11, 0x22, 0x33, 0x44,      /* Masking Key */
        0x79, 0x47, 0x5F, 0x28, 0x7E /* Masked Data */
    };
    mock_sock_set_response(frame, sizeof(frame));
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task = {0};
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
        0x89,                  /* FIN=1, Opcode=9 (ping) */
        0x04,                  /* Mask=0, Len=4 */
        0x70, 0x69, 0x6E, 0x67 /* "ping" */
    };
    mock_sock_set_response(frame, sizeof(frame));
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task = {0};
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
    const char *headers = "GET /chat HTTP/1.1\r\n"
                          "Host: localhost\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: "
                          "dGhlIHNhbXBsZSBub25jZQ=="
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n"
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

    SYN_WebsocketSession ws = {.sock = SYN_SOCKET_INVALID};
    SYN_Status st = syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL);
    /* Long key should still produce a valid upgrade (SHA-1 multi-block) */
    TEST_ASSERT_EQUAL(SYN_OK, st);
}

/** Upgrade without Sec-WebSocket-Key — exercises line 229 */
static void test_websocket_upgrade_no_key(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    const char *headers = "GET /chat HTTP/1.1\r\n"
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

    SYN_WebsocketSession ws = {.sock = SYN_SOCKET_INVALID};
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
/** Recv: 126..128-length extended frames succeed; >128 rejected with Close 1009 */
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

    /* Unmasked text frame with 2-byte extended length = 126 bytes (fits rx_buf[128]) */
    uint8_t frame[130];
    frame[0] = 0x81; /* FIN=1, text */
    frame[1] = 0x7E; /* Mask=0, Len=126 -> use 2-byte ext length */
    frame[2] = 0x00;
    frame[3] = 0x7E; /* 126 */
    memset(&frame[4], 'B', 126);
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &ws;
    for (int i = 0; i < 4; i++) {
        syn_websocket_task(&pt, &task);
    }
    TEST_ASSERT_EQUAL(1, s_msg_callback_count);
    TEST_ASSERT_EQUAL_UINT32(126U, s_last_len);
    TEST_ASSERT_EQUAL(0x01, s_last_opcode);

    /* Masked text frame with 2-byte extended length = 128 bytes (exact rx_buf capacity) */
    uint8_t masked_ext_frame[136];
    masked_ext_frame[0] = 0x81;
    masked_ext_frame[1] = 0xFE; /* Mask=1, Len=126 */
    masked_ext_frame[2] = 0x00;
    masked_ext_frame[3] = 0x80; /* 128 */
    masked_ext_frame[4] = 0x11;
    masked_ext_frame[5] = 0x22;
    masked_ext_frame[6] = 0x33;
    masked_ext_frame[7] = 0x44;
    for (size_t i = 0U; i < 128U; i++) {
        masked_ext_frame[8U + i] = (uint8_t)('C' ^ masked_ext_frame[4U + (i % 4U)]);
    }
    mock_sock_set_response(masked_ext_frame, sizeof(masked_ext_frame));
    for (int i = 0; i < 4; i++) {
        syn_websocket_task(&pt, &task);
    }
    TEST_ASSERT_EQUAL(2, s_msg_callback_count);
    TEST_ASSERT_EQUAL_UINT32(128U, s_last_len);
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
    uint8_t frame[] = {0x88, 0x02, 0x03, 0xE8}; /* opcode=8, len=2, status=1000 (normal) */
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
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
    SYN_Task task = {0};
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
    uint8_t frame[] = {0x81, 0x7F};
    mock_sock_set_response(frame, sizeof(frame));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
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

static void test_websocket_frame_masking_and_runt_payloads(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    /* 1. Header line without \n (line 215) */
    const char *no_newline_headers = "X-Foo: Bar\rSec-WebSocket-Key: key_no_newline";
    SYN_HttpdResponse resp;
    resp.sock = 11;
    resp.buf = (uint8_t *)no_newline_headers;
    resp.buf_size = strlen(no_newline_headers);
    resp.headers_sent = false;
    resp.upgraded = false;

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.path = "/chat";
    req.method = SYN_HTTP_GET;
    req.headers = no_newline_headers;

    SYN_WebsocketSession ws = {.sock = SYN_SOCKET_INVALID};
    SYN_Status st = syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 2. Multi-block SHA-1 update (> 256 bytes, lines 109 & 115) */
    static char long_key[500];
    memset(long_key, 'A', 400);
    long_key[400] = '\0';
    static char long_hdr[600];
    snprintf(long_hdr, sizeof(long_hdr), "Sec-WebSocket-Key: %s\r\n\r\n", long_key);
    req.headers = long_hdr;
    TEST_ASSERT_EQUAL(SYN_OK, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));

    /* Run websocket task until PT_END (line 403) */
    SYN_PT task_pt;
    PT_INIT(&task_pt);
    SYN_Task task_obj = {.user_data = &ws};
    ws.state = SYN_WS_STATE_CLOSED;
    syn_websocket_task(&task_pt, &task_obj);

    /* Test task waiting when not connected (returns PT_IDLE) */
    SYN_PT task_pt2;
    PT_INIT(&task_pt2);
    ws.state = SYN_WS_STATE_CLOSED;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&task_pt2, &task_obj));

    /* 3. send when not connected (line 275) */
    ws.state = SYN_WS_STATE_CLOSED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_send(&ws, 0x01, "test", 4));

    /* Send empty payload when connected (len == 0, data == NULL) */
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.sock = 11;
    TEST_ASSERT_EQUAL(SYN_OK, syn_websocket_send(&ws, 0x01, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_send(&ws, 0x01, NULL, 70000));

    /* Recv text frame with on_message == NULL */
    SYN_WebsocketSession ws_null_cb;
    memset(&ws_null_cb, 0, sizeof(ws_null_cb));
    ws_null_cb.sock = 11;
    ws_null_cb.state = SYN_WS_STATE_CONNECTED;
    ws_null_cb.on_message = NULL;
    uint8_t text_frame[] = {0x81, 0x04, 't', 'e', 's', 't'};
    mock_sock_set_response(text_frame, sizeof(text_frame));
    SYN_PT pt_null_cb;
    PT_INIT(&pt_null_cb);
    SYN_Task task_null_cb = {.user_data = &ws_null_cb};
    for (int i = 0; i < 6; i++) {
        syn_websocket_task(&pt_null_cb, &task_null_cb);
    }

    /* 4. ws_has_work(NULL) (line 318) */
    SYN_Task null_task = {.user_data = NULL};
    SYN_PT null_pt;
    PT_INIT(&null_pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_websocket_task(&null_pt, &null_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_websocket_task(&null_pt, NULL));

    /* 5. 0-length payload frame parsing (e.g. empty unmasked PING, empty masked text) */
    SYN_WebsocketSession ws_empty;
    memset(&ws_empty, 0, sizeof(ws_empty));
    ws_empty.sock = 11;
    ws_empty.state = SYN_WS_STATE_CONNECTED;
    ws_empty.on_message = on_ws_message;
    s_msg_callback_count = 0;
    mock_port_reset();
    mock_sock_connected = true;

    /* Frame 1: Unmasked Empty PING (0x89, 0x00), Frame 2: Masked Empty Text (0x81, 0x80, M1..M4) */
    uint8_t empty_frames[] = {0x89, 0x00, 0x81, 0x80, 0x11, 0x22, 0x33, 0x44};
    mock_sock_set_response(empty_frames, sizeof(empty_frames));
    SYN_PT pt_empty;
    PT_INIT(&pt_empty);
    SYN_Task task_empty = {.user_data = &ws_empty};
    syn_websocket_task(&pt_empty, &task_empty);
    TEST_ASSERT_EQUAL(1, s_msg_callback_count);
    TEST_ASSERT_EQUAL(0, s_last_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, s_last_opcode);
}

static void test_websocket_has_work_invalid_sock_eof_and_sustained_rx(void)
{
    SYN_PT pt;
    SYN_WebsocketSession ws;

    /* 1. Null pt guard (syn_websocket.c:407) */
    SYN_Task dummy_task = {0};
    TEST_ASSERT_EQUAL(PT_EXITED, syn_websocket_task(NULL, &dummy_task));

    /* 2. Connected state with sock == SYN_SOCKET_INVALID hits ws_close_session + continue
     *    (syn_websocket.c:415-417) */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.sock = SYN_SOCKET_INVALID;
    ws.rx_state = 2;
    SYN_Task task = {0};
    task.user_data = &ws;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(0, ws.rx_state);

    /* 3. Parked at PT_WAIT_UNTIL(pt, ws_has_work(ws)) (line 441) while connected, then session
     *    state changes != SYN_WS_STATE_CONNECTED -> evaluates ws_has_work hitting line 309 */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.sock = 11;
    ws.on_message = on_ws_message;
    mock_sock_connected = true;

    uint8_t hi_frame[] = {0x81, 0x02, 'h', 'i'};
    mock_sock_set_response(hi_frame, sizeof(hi_frame));
    memset(&task, 0, sizeof(task));
    task.user_data = &ws;
    PT_INIT(&pt);

    /* Consumes frame, socket now empty -> parks at line 441 PT_WAIT_UNTIL(pt, ws_has_work(ws)) */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CONNECTED, ws.state);

    /* Transition session state away from CONNECTED while parked at line 441 */
    ws.state = SYN_WS_STATE_CLOSED;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));

    /* 4. Abnormal mid-frame EOF (n == 0 after partial header byte) resets sock and rx_state */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.sock = 11;
    mock_sock_connected = true;

    uint8_t partial_hdr[] = {0x81}; /* Opcode only, missing length byte -> rx_state = 1 */
    mock_sock_set_response(partial_hdr, sizeof(partial_hdr));
    memset(&task, 0, sizeof(task));
    task.user_data = &ws;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(1, ws.rx_state);

    /* Peer drops TCP connection mid-frame (recv returns 0) */
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);
    TEST_ASSERT_EQUAL(0, ws.rx_state);
    mock_sock_eof_on_empty = false;

    /* 5. Sustained RX (>64B recv chunk) with contended sched hits PT_CONTINUE_IF_NEEDED (438) */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.sock = 11;
    ws.on_message = on_ws_message;
    mock_sock_connected = true;
    s_msg_callback_count = 0;

    uint8_t burst[80];
    for (size_t i = 0; i < sizeof(burst); i += 2) {
        burst[i] = 0x81;     /* FIN + text */
        burst[i + 1] = 0x00; /* 0-byte unmasked payload */
    }
    mock_sock_set_response(burst, sizeof(burst));

    SYN_Sched sched = {0};
    memset(&task, 0, sizeof(task));
    task.user_data = &ws;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_MORE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(32, s_msg_callback_count);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(40, s_msg_callback_count);
}

static void test_websocket_round3_regressions(void)
{
    mock_port_reset();

    /* 1. R6-WS: uninitialized/garbage ws struct never closes random FD during upgrade;
     *    explicit syn_websocket_close() cleanly closes prior socket 5 */
    const char *headers = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    SYN_HttpdRequest req = {.path = "/ws", .method = SYN_HTTP_GET, .headers = headers};
    SYN_HttpdResponse resp = {.sock = 11, .buf = (uint8_t *)headers, .buf_size = strlen(headers)};
    SYN_WebsocketSession ws;
    memset(&ws, 0x01, sizeof(ws)); /* Simulates stack garbage where state == 1 */
    mock_sock_connected = true;
    int closes_before = mock_sock_close_count;
    TEST_ASSERT_EQUAL(SYN_OK, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));
    TEST_ASSERT_EQUAL_INT(closes_before, mock_sock_close_count);
    TEST_ASSERT_EQUAL(11, ws.sock);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CONNECTED, ws.state);

    ws.sock = 5;
    syn_websocket_close(NULL);
    syn_websocket_close(&ws);
    TEST_ASSERT_EQUAL_INT(closes_before + 1, mock_sock_close_count);
    TEST_ASSERT_EQUAL(5, mock_sock_last_closed);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);

    /* 2. R3-N07: non-minimal 16-bit extended payload lengths (< 126, including 0) close session */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    uint8_t bad_ext0[] = {0x81, 0x7E, 0x00, 0x00};
    mock_sock_set_response(bad_ext0, sizeof(bad_ext0));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &ws};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);

    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    uint8_t bad_ext125[] = {0x81, 0x7E, 0x00, 0x7D};
    mock_sock_set_response(bad_ext125, sizeof(bad_ext125));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);

    /* 3. R3-N06: readable socket recv error closes session without spinning; PT_END fallthrough */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    mock_sock_recv_fail = true;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);
    mock_sock_recv_fail = false;

    pt.lc = 9999;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_websocket_task(&pt, &task));
}

static void test_websocket_round4_upgrade_and_control_frames(void)
{
    /* 1. R4-N15: Sec-WebSocket-Key smuggled in body after \r\n\r\n or \n\n is rejected */
    mock_port_reset();
    mock_sock_connected = true;
    const char *smuggled_crlf = "Host: localhost\r\n"
                                "\r\n"
                                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n";
    SYN_HttpdRequest req = {.path = "/ws", .method = SYN_HTTP_GET, .headers = smuggled_crlf};
    SYN_HttpdResponse resp = {.sock = 11,
                              .buf = (uint8_t *)smuggled_crlf,
                              .buf_size = strlen(smuggled_crlf),
                              .upgraded = false};
    SYN_WebsocketSession ws = {.sock = SYN_SOCKET_INVALID};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));
    TEST_ASSERT_FALSE(resp.upgraded);

    const char *smuggled_lf = "Host: localhost\n\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n";
    req.headers = smuggled_lf;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));

    const char *eof_hdr = "Host: localhost\n";
    req.headers = eof_hdr;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));

    /* 2. R4-N15: 101 Switching Protocols send failure leaves existing session & resp untouched */
    mock_port_reset();
    mock_sock_connected = true;
    mock_sock_send_fail = true;
    const char *valid_hdr = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    req.headers = valid_hdr;
    resp.sock = 11;
    resp.upgraded = false;
    ws.sock = 5;
    ws.state = SYN_WS_STATE_CONNECTED;
    int closes_before = mock_sock_close_count;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));
    TEST_ASSERT_FALSE(resp.upgraded);
    TEST_ASSERT_EQUAL(5, ws.sock);
    TEST_ASSERT_EQUAL_INT(closes_before, mock_sock_close_count);
    mock_sock_send_fail = false;

    /* 3. R4-N15 (RFC 6455 s5.5): fragmented control frame (FIN == 0) closes session */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    uint8_t frag_ping[] = {0x09, 0x00}; /* FIN=0, opcode=9 (Ping) */
    mock_sock_set_response(frag_ping, sizeof(frag_ping));
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &ws};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);

    /* 4. R4-N15 (RFC 6455 s5.5): control frame with extended length (len >= 126) closes session */
    mock_port_reset();
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    mock_sock_connected = true;
    uint8_t oversize_ping[] = {0x89, 0x7E, 0x00, 0x7E}; /* FIN=1, opcode=9, len=126 */
    mock_sock_set_response(oversize_ping, sizeof(oversize_ping));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, ws.sock);
}

static void test_r5_n12_websocket_oversize_unmasked_and_pipelined(void)
{
    /* 1. R5-N12: Inbound frame > sizeof(ws.rx_buf) (129B) sends Close 1009 & closes session */
    mock_port_reset();
    s_msg_callback_count = 0;
    SYN_WebsocketSession ws;
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    ws.on_message = on_ws_message;
    mock_sock_connected = true;

    uint8_t oversize_129[133];
    oversize_129[0] = 0x81U;
    oversize_129[1] = 0x7EU;
    oversize_129[2] = 0x00U;
    oversize_129[3] = 0x81U; /* 129 > 128 */
    memset(&oversize_129[4], 'X', 129U);
    mock_sock_set_response(oversize_129, sizeof(oversize_129));
    mock_sock_tx_len = 0U;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &ws};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(0, s_msg_callback_count);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL_size_t(4U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x88U, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, mock_sock_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03U, mock_sock_tx_buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xF1U, mock_sock_tx_buf[3]); /* 1009 Message Too Big */

    /* 2. R5-N12: Server-side upgraded session rejects unmasked client frame with Close 1002 */
    mock_port_reset();
    mock_sock_connected = true;
    const char *hdr = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    SYN_HttpdRequest req = {.path = "/ws", .method = SYN_HTTP_GET, .headers = hdr};
    SYN_HttpdResponse resp = {.sock = 11, .buf = (uint8_t *)hdr, .buf_size = strlen(hdr)};
    memset(&ws, 0, sizeof(ws));
    TEST_ASSERT_EQUAL(SYN_OK, syn_websocket_upgrade(&req, &resp, &ws, on_ws_message, NULL));
    TEST_ASSERT_TRUE(ws.is_server);

    mock_sock_tx_len = 0U;
    uint8_t unmasked_client[] = {0x81U, 0x02U, 'n', 'o'};
    mock_sock_set_response(unmasked_client, sizeof(unmasked_client));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(0, s_msg_callback_count);
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);
    TEST_ASSERT_EQUAL_size_t(4U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x88U, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, mock_sock_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03U, mock_sock_tx_buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xEAU, mock_sock_tx_buf[3]); /* 1002 Protocol Error */

    /* 3. R5-N12: Pipelined masked client frame in HTTP Upgrade buffer is delivered without drop */
    mock_port_reset();
    mock_sock_connected = true;
    s_msg_callback_count = 0;

    /* Park task at PT_WAIT_UNTIL(pt, ws_has_work(ws)) first, then upgrade with pipelined frame */
    memset(&ws, 0, sizeof(ws));
    ws.sock = 11;
    ws.state = SYN_WS_STATE_CONNECTED;
    uint8_t warmup[] = {0x81U, 0x00U};
    mock_sock_set_response(warmup, sizeof(warmup));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));

    uint8_t raw_upgrade[128];
    size_t hlen = strlen(hdr);
    memcpy(raw_upgrade, hdr, hlen);
    const uint8_t masked_hi[] = {
        0x81U, 0x82U, 0xAAU, 0x55U, 0xAAU, 0x55U, (uint8_t)('o' ^ 0xAAU), (uint8_t)('k' ^ 0x55U)};
    memcpy(raw_upgrade + hlen, masked_hi, sizeof(masked_hi));

    SYN_HttpdRequest pipe_req = {.path = "/ws",
                                 .method = SYN_HTTP_GET,
                                 .headers = (const char *)raw_upgrade,
                                 .body_buffered_offset = hlen,
                                 .body_buffered_len = sizeof(masked_hi)};
    SYN_HttpdResponse pipe_resp = {.sock = 11, .buf = raw_upgrade, .buf_size = sizeof(raw_upgrade)};
    mock_sock_rx_len = 0U;
    mock_sock_rx_pos = 0U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_websocket_upgrade(&pipe_req, &pipe_resp, &ws, on_ws_message, NULL));
    TEST_ASSERT_EQUAL_UINT16(sizeof(masked_hi), ws.pending_rx_len);

    /* Socket has ZERO unread bytes, yet ws_has_work wakes on pending_rx and delivers "ok"! */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(1, s_msg_callback_count);
    TEST_ASSERT_EQUAL_UINT32(2U, s_last_len);
    TEST_ASSERT_EQUAL_STRING_LEN("ok", s_last_payload, 2U);

    /* 4. Invalid pipelined unmasked frame closes session during pending_rx drain */
    const uint8_t bad_pipe[] = {0x81U, 0x02U, 'x', 'y'};
    memcpy(raw_upgrade + hlen, bad_pipe, sizeof(bad_pipe));
    pipe_req.body_buffered_len = sizeof(bad_pipe);
    pipe_resp.upgraded = false;
    mock_sock_connected = true;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_websocket_upgrade(&pipe_req, &pipe_resp, &ws, on_ws_message, NULL));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_websocket_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WS_STATE_CLOSED, ws.state);

    /* 5. Upgrade buffered bounds guards (overflow > 256, NULL buf, offset+len > buf_size) */
    pipe_req.body_buffered_len = sizeof(ws.pending_rx) + 1U;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_websocket_upgrade(&pipe_req, &pipe_resp, &ws, on_ws_message, NULL));

    pipe_req.body_buffered_len = 4U;
    pipe_resp.buf = NULL;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_websocket_upgrade(&pipe_req, &pipe_resp, &ws, on_ws_message, NULL));

    pipe_resp.buf = raw_upgrade;
    pipe_resp.buf_size = hlen + 2U;
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_websocket_upgrade(&pipe_req, &pipe_resp, &ws, on_ws_message, NULL));
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
    RUN_TEST(test_websocket_frame_masking_and_runt_payloads);
    RUN_TEST(test_websocket_has_work_invalid_sock_eof_and_sustained_rx);
    RUN_TEST(test_websocket_round3_regressions);
    RUN_TEST(test_websocket_round4_upgrade_and_control_frames);
    RUN_TEST(test_r5_n12_websocket_oversize_unmasked_and_pipelined);
}
