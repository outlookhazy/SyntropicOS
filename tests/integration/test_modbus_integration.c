#include "mock_port.h"
#include "syntropic/proto/syn_modbus.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

static void build_mbap_req(uint8_t out[12], uint16_t tx_id, uint8_t fc, uint16_t addr, uint16_t val)
{
    out[0] = (uint8_t)(tx_id >> 8U);
    out[1] = (uint8_t)(tx_id & 0xFFU);
    out[2] = 0x00U;
    out[3] = 0x00U;
    out[4] = 0x00U;
    out[5] = 0x06U;
    out[6] = 0x01U;
    out[7] = fc;
    out[8] = (uint8_t)(addr >> 8U);
    out[9] = (uint8_t)(addr & 0xFFU);
    out[10] = (uint8_t)(val >> 8U);
    out[11] = (uint8_t)(val & 0xFFU);
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_modbus_tcp_integration(void)
{
    const char *host = getenv("MODBUS_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = 5020U;

    printf("[Integration Test] Connecting to Modbus TCP Server at %s:%u...\n", host, port);

    int sock = syn_port_sock_connect_host(host, port);
    if (sock < 0) {
        fail_or_skip("Modbus TCP server not reachable");
        return;
    }
    printf("[Integration Test] Connected to Modbus TCP Server!\n");

    uint8_t req[12];
    uint8_t resp[32];

    /* 1. Read Holding Registers (FC 0x03, Reg 0, Count 2) */
    build_mbap_req(req, 1U, SYN_MB_FC_READ_HOLDING, 0U, 2U);
    ssize_t sent = send(sock, req, sizeof(req), 0);
    TEST_ASSERT_EQUAL_INT((int)sizeof(req), (int)sent);

    ssize_t recvd = recv(sock, resp, sizeof(resp), 0);
    TEST_ASSERT_TRUE(recvd >= 13);
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[6]);
    TEST_ASSERT_EQUAL_UINT8(SYN_MB_FC_READ_HOLDING, resp[7]);
    TEST_ASSERT_EQUAL_UINT8(4U, resp[8]);

    uint16_t reg0 = (uint16_t)(((uint16_t)resp[9] << 8U) | resp[10]);
    uint16_t reg1 = (uint16_t)(((uint16_t)resp[11] << 8U) | resp[12]);
    printf("[Integration Test] Read Holding Reg 0: 0x%04X, Reg 1: 0x%04X\n", reg0, reg1);
    TEST_ASSERT_EQUAL_HEX16(0x1234U, reg0);
    TEST_ASSERT_EQUAL_HEX16(0x5678U, reg1);

    /* 2. Write Single Register (FC 0x06, Reg 2 = 0xCAFE) */
    build_mbap_req(req, 2U, SYN_MB_FC_WRITE_SINGLE, 2U, 0xCAFEU);
    sent = send(sock, req, sizeof(req), 0);
    TEST_ASSERT_EQUAL_INT((int)sizeof(req), (int)sent);

    recvd = recv(sock, resp, sizeof(resp), 0);
    TEST_ASSERT_EQUAL_INT(12, (int)recvd);
    TEST_ASSERT_EQUAL_UINT8(SYN_MB_FC_WRITE_SINGLE, resp[7]);
    TEST_ASSERT_EQUAL_UINT8(0xCAU, resp[10]);
    TEST_ASSERT_EQUAL_UINT8(0xFEU, resp[11]);

    /* Read back Reg 2 to verify persistence */
    build_mbap_req(req, 3U, SYN_MB_FC_READ_HOLDING, 2U, 1U);
    sent = send(sock, req, sizeof(req), 0);
    TEST_ASSERT_EQUAL_INT((int)sizeof(req), (int)sent);

    recvd = recv(sock, resp, sizeof(resp), 0);
    TEST_ASSERT_TRUE(recvd >= 11);
    uint16_t reg2 = (uint16_t)(((uint16_t)resp[9] << 8U) | resp[10]);
    printf("[Integration Test] Verified Written Reg 2: 0x%04X\n", reg2);
    TEST_ASSERT_EQUAL_HEX16(0xCAFEU, reg2);

    /* 3. Write Single Coil (FC 0x05, Coil 1 = ON 0xFF00) */
    build_mbap_req(req, 4U, SYN_MB_FC_WRITE_SINGLE_COIL, 1U, 0xFF00U);
    sent = send(sock, req, sizeof(req), 0);
    TEST_ASSERT_EQUAL_INT((int)sizeof(req), (int)sent);

    recvd = recv(sock, resp, sizeof(resp), 0);
    TEST_ASSERT_EQUAL_INT(12, (int)recvd);
    TEST_ASSERT_EQUAL_UINT8(SYN_MB_FC_WRITE_SINGLE_COIL, resp[7]);
    TEST_ASSERT_EQUAL_UINT8(0xFFU, resp[10]);

    /* 4. Exception Response for unsupported FC (0x2B) */
    build_mbap_req(req, 5U, 0x2BU, 0U, 1U);
    sent = send(sock, req, sizeof(req), 0);
    TEST_ASSERT_EQUAL_INT((int)sizeof(req), (int)sent);

    recvd = recv(sock, resp, sizeof(resp), 0);
    TEST_ASSERT_TRUE(recvd >= 9);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(0x2BU | 0x80U), resp[7]);
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[8]);

    close(sock);
    printf("[Integration Test] End-to-End Modbus TCP Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_modbus_tcp_integration);
    return UNITY_END();
}
