/**
 * @file test_ocpp_integration.c
 * @brief Containerized 3rd-party OCPP 1.6-J integration test driver for SyntropicOS.
 */

#include "syntropic/port/syn_port_socket.h"
#include "syntropic/proto/syn_ocpp.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static SYN_OCPP_Client g_client;
static SYN_OCPP_Server g_server;

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

void setUp(void)
{
    syn_ocpp_init(&g_client);
    syn_ocpp_server_init(&g_server);
}

void tearDown(void)
{
}

static SYN_Socket connect_with_retry(const char *host, uint16_t port)
{
    for (int attempt = 0; attempt < 20; attempt++) {
        SYN_Socket s = syn_port_sock_connect_host(host, port);
        if (s != SYN_SOCKET_INVALID) {
            return s;
        }
        usleep(100000);
    }
    return SYN_SOCKET_INVALID;
}

static bool recv_exact(SYN_Socket sock, uint8_t *dst, size_t len)
{
    size_t got = 0U;
    while (got < len) {
        int n = syn_port_sock_recv(sock, dst + got, len - got, 5000U);
        if (n <= 0) {
            return false;
        }
        got += (size_t)n;
    }
    return true;
}

static bool read_http_upgrade_headers(SYN_Socket sock, char *buf, size_t cap)
{
    size_t pos = 0U;
    while (pos + 1U < cap) {
        uint8_t ch = 0U;
        if (!recv_exact(sock, &ch, 1U)) {
            return false;
        }
        buf[pos++] = (char)ch;
        buf[pos] = '\0';
        if (pos >= 4U && memcmp(&buf[pos - 4U], "\r\n\r\n", 4U) == 0) {
            return true;
        }
    }
    return false;
}

static const char *recv_ocpp_json(SYN_Socket sock, char *buf, size_t buf_size)
{
    uint8_t hdr[2];
    if (!recv_exact(sock, hdr, 2U)) {
        return NULL;
    }
    bool masked = (hdr[1] & 0x80U) != 0U;
    size_t plen = (size_t)(hdr[1] & 0x7FU);
    if (plen == 126U) {
        uint8_t ext[2];
        if (!recv_exact(sock, ext, 2U)) {
            return NULL;
        }
        plen = ((size_t)ext[0] << 8U) | (size_t)ext[1];
    }
    if (plen >= buf_size) {
        return NULL;
    }
    uint8_t mask_key[4] = {0U, 0U, 0U, 0U};
    if (masked && !recv_exact(sock, mask_key, 4U)) {
        return NULL;
    }
    if (!recv_exact(sock, (uint8_t *)buf, plen)) {
        return NULL;
    }
    if (masked) {
        for (size_t i = 0U; i < plen; i++) {
            buf[i] = (char)((uint8_t)buf[i] ^ mask_key[i % 4U]);
        }
    }
    buf[plen] = '\0';
    return strchr(buf, '[');
}

static size_t format_client_ws_frame(const char *payload, size_t payload_len, uint8_t *out_frame)
{
    const uint8_t mask_key[4] = {0x12U, 0x34U, 0x56U, 0x78U};
    size_t header_len = 0U;
    out_frame[0] = 0x81U;
    if (payload_len < 126U) {
        out_frame[1] = (uint8_t)(0x80U | payload_len);
        memcpy(out_frame + 2U, mask_key, 4U);
        header_len = 6U;
    } else {
        out_frame[1] = (uint8_t)(0x80U | 126U);
        out_frame[2] = (uint8_t)((payload_len >> 8U) & 0xFFU);
        out_frame[3] = (uint8_t)(payload_len & 0xFFU);
        memcpy(out_frame + 4U, mask_key, 4U);
        header_len = 8U;
    }
    for (size_t i = 0U; i < payload_len; i++) {
        out_frame[header_len + i] = (uint8_t)((uint8_t)payload[i] ^ mask_key[i % 4U]);
    }
    return payload_len + header_len;
}

static const char *send_and_recv_ocpp(SYN_Socket sock, const char *tx, size_t plen, char *rx,
                                      size_t rx_cap)
{
    uint8_t ws_frame[1024];
    size_t frame_len = format_client_ws_frame(tx, plen, ws_frame);
    int sent = syn_port_sock_send_all(sock, ws_frame, frame_len);
    TEST_ASSERT_EQUAL_INT((int)frame_len, sent);
    const char *json = recv_ocpp_json(sock, rx, rx_cap);
    TEST_ASSERT_NOT_NULL(json);
    return json;
}

static void verify_ocpp_transactions(SYN_Socket sock, char *tx, size_t tx_cap, char *rx,
                                     size_t rx_cap)
{
    size_t plen = 0U;
    const char *json = NULL;
    SYN_Status st;

    st = syn_ocpp_format_authorize(&g_client, "RFID-TAG-123", tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);

    st = syn_ocpp_format_start_transaction(&g_client, 1U, "RFID-TAG-123", 1000, tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    json = send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);
    st = syn_ocpp_process_message(&g_client, json, strlen(json), NULL, 0U, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(1001, g_client.active_transaction_id);

    SYN_OCPP_MeterValues meter = {5000U, 230U, 160U, 3680U, 50U};
    st = syn_ocpp_format_meter_values(&g_client, 1U, &meter, tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);

    const char *reason = "EVDisconnected";
    st = syn_ocpp_format_stop_transaction(&g_client, 1001, 15000, reason, tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);

    SYN_OCPP_DisplayMessage disp = {201U, "Rate Alert", "Off-peak energy tariff active", 60U};
    st = syn_ocpp_format_display_message(&g_client, &disp, tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);

    SYN_OCPP_V2GMode mode = SYN_OCPP_V2G_DYNAMIC;
    st = syn_ocpp_format_v2g_energy_transfer(&g_client, mode, -10000, tx, tx_cap, &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx, plen, rx, rx_cap);
}

void test_ocpp_client_against_python_csms(void)
{
    const char *host = getenv("OCPP_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    SYN_Socket sock = connect_with_retry(host, 9001U);
    if (sock == SYN_SOCKET_INVALID) {
        fail_or_skip("Python OCPP CSMS server on port 9001 not reachable");
        return;
    }

    const char *ws_handshake = "GET /CP001 HTTP/1.1\r\n"
                               "Host: 127.0.0.1:9001\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Protocol: ocpp1.6\r\n"
                               "Sec-WebSocket-Version: 13\r\n\r\n";
    size_t hs_len = strlen(ws_handshake);
    TEST_ASSERT_EQUAL_INT((int)hs_len, syn_port_sock_send_all(sock, ws_handshake, hs_len));

    char rx_buf[2048];
    TEST_ASSERT_TRUE(read_http_upgrade_headers(sock, rx_buf, sizeof(rx_buf)));
    TEST_ASSERT_NOT_NULL(strstr(rx_buf, "101 Switching Protocols"));

    char tx_buf[512];
    size_t plen = 0U;
    SYN_OCPP_ChargePointInfo info = {"SyntropicOS-Vendor", "EVSE-v1", "SN-1001", "v1.0.0"};
    SYN_Status st;
    st = syn_ocpp_format_boot_notification(&g_client, &info, tx_buf, sizeof(tx_buf), &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    const char *json = send_and_recv_ocpp(sock, tx_buf, plen, rx_buf, sizeof(rx_buf));
    st = syn_ocpp_process_message(&g_client, json, strlen(json), NULL, 0U, NULL);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(SYN_OCPP_REGISTRATION_ACCEPTED, g_client.registration_status);

    st = syn_ocpp_format_heartbeat(&g_client, tx_buf, sizeof(tx_buf), &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx_buf, plen, rx_buf, sizeof(rx_buf));

    SYN_OCPP_ChargePointStatus cp_st = SYN_OCPP_STATUS_CHARGING;
    st = syn_ocpp_format_status_notification(&g_client, 1U, cp_st, "NoError", tx_buf,
                                             sizeof(tx_buf), &plen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    (void)send_and_recv_ocpp(sock, tx_buf, plen, rx_buf, sizeof(rx_buf));

    verify_ocpp_transactions(sock, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf));
    syn_port_sock_close(sock);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ocpp_client_against_python_csms);
    return UNITY_END();
}
