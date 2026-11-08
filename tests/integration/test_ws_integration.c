/**
 * @file test_ws_integration.c
 * @brief End-to-end WebSocket integration test against Node.js ws server.
 */

#include "mock_port.h"
#include "syntropic/net/syn_websocket.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool ws_msg_received = false;
static char ws_msg_payload[128] = {0};

static bool integration_skip_allowed(void)
{
    const char *v = getenv("SYN_ALLOW_SKIP_INTEGRATION");
    return (v != NULL && strcmp(v, "1") == 0);
}

static void fail_or_skip(const char *msg)
{
    if (integration_skip_allowed()) {
        TEST_IGNORE_MESSAGE(msg);
    } else {
        TEST_FAIL_MESSAGE(msg);
    }
}

static void on_ws_message(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx)
{
    (void)opcode;
    (void)ctx;
    if (len < sizeof(ws_msg_payload)) {
        memcpy(ws_msg_payload, payload, len);
        ws_msg_payload[len] = '\0';
    }
    ws_msg_received = true;
}

static SYN_Socket connect_with_retry(const char *host, uint16_t port)
{
    for (int attempt = 0; attempt < 15; attempt++) {
        SYN_Socket s = syn_port_sock_connect_host(host, port);
        if (s != SYN_SOCKET_INVALID) {
            return s;
        }
        usleep(100000);
    }
    return SYN_SOCKET_INVALID;
}

static bool read_http_upgrade_headers(SYN_Socket sock, char *buf, size_t cap)
{
    size_t pos = 0U;
    while (pos + 1U < cap) {
        char ch = '\0';
        int n = syn_port_sock_recv(sock, &ch, 1U, 3000U);
        if (n <= 0) {
            return false;
        }
        buf[pos++] = ch;
        buf[pos] = '\0';
        if (pos >= 4U && memcmp(&buf[pos - 4U], "\r\n\r\n", 4U) == 0) {
            return true;
        }
    }
    return false;
}

static size_t build_masked_text_frame(const char *msg, uint8_t *out)
{
    const uint8_t mask[4] = {0x37U, 0xFAU, 0x21U, 0x3DU};
    size_t len = strlen(msg);
    out[0] = 0x81U;
    out[1] = (uint8_t)(0x80U | (uint8_t)len);
    memcpy(&out[2], mask, 4U);
    for (size_t i = 0U; i < len; i++) {
        out[6U + i] = (uint8_t)((uint8_t)msg[i] ^ mask[i % 4U]);
    }
    return 6U + len;
}

void setUp(void)
{
    ws_msg_received = false;
    memset(ws_msg_payload, 0, sizeof(ws_msg_payload));
}

void tearDown(void)
{
}

void test_websocket_embedded_server(void)
{
    const char *host = getenv("WS_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = (strcmp(host, "127.0.0.1") == 0) ? 10081U : 8080U;

    printf("[Integration Test] Connecting to Node.js WS Server at %s:%u...\n", host, port);
    SYN_Socket client_sock = connect_with_retry(host, port);
    if (client_sock == SYN_SOCKET_INVALID) {
        fail_or_skip("Node.js WebSocket server container not reachable");
        return;
    }

    const char *upgrade_req = "GET / HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "Upgrade: websocket\r\n"
                              "Connection: Upgrade\r\n"
                              "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                              "Sec-WebSocket-Version: 13\r\n\r\n";
    size_t req_len = strlen(upgrade_req);
    int sent = syn_port_sock_send_all(client_sock, upgrade_req, req_len);
    TEST_ASSERT_EQUAL_INT((int)req_len, sent);

    char hdr_buf[512];
    TEST_ASSERT_TRUE(read_http_upgrade_headers(client_sock, hdr_buf, sizeof(hdr_buf)));
    TEST_ASSERT_NOT_NULL(strstr(hdr_buf, "101 Switching Protocols"));
    TEST_ASSERT_NOT_NULL(strstr(hdr_buf, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));

    SYN_WebsocketSession ws_session;
    memset(&ws_session, 0, sizeof(ws_session));
    ws_session.sock = client_sock;
    ws_session.state = SYN_WS_STATE_CONNECTED;
    ws_session.on_message = on_ws_message;

    const char *test_msg = "Hello Node.js WebSocket Server!";
    uint8_t tx_frame[128];
    size_t frame_len = build_masked_text_frame(test_msg, tx_frame);
    sent = syn_port_sock_send_all(client_sock, tx_frame, frame_len);
    TEST_ASSERT_EQUAL_INT((int)frame_len, sent);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    memset(&task, 0, sizeof(task));
    task.user_data = &ws_session;

    for (int i = 0; i < 200 && !ws_msg_received; i++) {
        mock_tick_advance(10U);
        syn_websocket_task(&pt, &task);
        usleep(10000);
    }

    syn_port_sock_close(ws_session.sock);
    TEST_ASSERT_TRUE(ws_msg_received);
    TEST_ASSERT_EQUAL_STRING(test_msg, ws_msg_payload);
    printf("[Integration Test] End-to-End WebSocket Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_websocket_embedded_server);
    return UNITY_END();
}
