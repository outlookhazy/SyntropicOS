/**
 * @file test_ecat_integration.c
 * @brief End-to-end integration test for EtherCAT stack against pysoem daemon container.
 */

#include "mock_port.h"
#include "syntropic/proto/syn_ethercat.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static size_t recv_ecat_frame(SYN_Socket sock, uint8_t *buf, size_t expected)
{
    size_t total = 0U;
    while (total < expected) {
        int n = syn_port_sock_recv(sock, buf + total, expected - total, 5000U);
        if (n <= 0) {
            break;
        }
        total += (size_t)n;
    }
    return total;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ethercat_container_integration(void)
{
    const char *host = getenv("ETHERCAT_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = 10884U;

    printf("[Integration Test] Connecting to EtherCAT Server at %s:%u...\n", host, port);
    SYN_Socket sock = connect_with_retry(host, port);
    if (sock == SYN_SOCKET_INVALID) {
        fail_or_skip("EtherCAT pysoem daemon container not reachable");
        return;
    }

    SYN_EcatNode node;
    syn_ecat_init(&node, 0x1001U, NULL);

    uint8_t tx_frame[64];
    SYN_EcatDatagram dg;
    memset(&dg, 0, sizeof(dg));
    dg.cmd = SYN_ECAT_CMD_FPRD;
    dg.idx = 0x01U;
    dg.addr = 0x10010000U;

    uint8_t payload[4] = {0xAAU, 0xBBU, 0xCCU, 0xDDU};
    size_t tx_len = syn_ecat_build_datagram_frame(tx_frame, sizeof(tx_frame), &dg, payload, 4U);
    TEST_ASSERT_EQUAL_UINT32(18U, (uint32_t)tx_len);

    int sent = syn_port_sock_send_all(sock, tx_frame, tx_len);
    TEST_ASSERT_EQUAL_INT((int)tx_len, sent);

    uint8_t rx_frame[64];
    size_t recvd = recv_ecat_frame(sock, rx_frame, tx_len);
    syn_port_sock_close(sock);
    TEST_ASSERT_EQUAL_UINT32(18U, (uint32_t)recvd);

    uint16_t wkc = 0U;
    SYN_Status st = syn_ecat_parse_frame(&node, rx_frame, recvd, &wkc);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT16(1U, wkc);
    TEST_ASSERT_EQUAL_UINT16(1U, node.wkc_last);
    printf("[Integration Test] End-to-End EtherCAT Integration PASS (WKC=%u)!\n", wkc);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ethercat_container_integration);
    return UNITY_END();
}
