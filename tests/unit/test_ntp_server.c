/**
 * @file test_ntp_server.c
 * @brief Unity tests for Zero-Heap NTP Server Protocol Engine.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_ntp_server.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static SYN_NTPServer g_ntp_server;

static uint32_t mock_get_epoch(void)
{
    return 1700000000UL; /* 2023-11-14 22:13:20 UTC */
}

void test_ntp_server_init_and_null_checks(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_init(NULL, 1, mock_get_epoch));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_init(&g_ntp_server, 0, mock_get_epoch));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_init(&g_ntp_server, 16, mock_get_epoch));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_init(&g_ntp_server, 2, mock_get_epoch));

    uint8_t req[48] = {0};
    uint8_t resp[48] = {0};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_process_packet(NULL, req, resp));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_process_packet(&g_ntp_server, NULL, resp));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ntp_server_process_packet(&g_ntp_server, req, NULL));
}

void test_ntp_server_process_packet(void)
{
    syn_ntp_server_init(&g_ntp_server, 2, mock_get_epoch);

    /* Format valid client request (mode = 3) */
    uint8_t req[48] = {0};
    req[0] = 0x23; /* NTPv4 Client request */
    req[40] = 0xAA;
    req[41] = 0xBB;
    req[42] = 0xCC;
    req[43] = 0xDD;

    uint8_t resp[48] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, req, resp));

    /* Verify response fields */
    TEST_ASSERT_EQUAL(0x24, resp[0]); /* Server response mode */
    TEST_ASSERT_EQUAL(2, resp[1]);    /* Stratum 2 */
    TEST_ASSERT_EQUAL('L', resp[12]);
    TEST_ASSERT_EQUAL('O', resp[13]);
    TEST_ASSERT_EQUAL('C', resp[14]);
    TEST_ASSERT_EQUAL('L', resp[15]);

    /* Verify Originate Timestamp matches Client Transmit Timestamp */
    TEST_ASSERT_EQUAL(0xAA, resp[24]);
    TEST_ASSERT_EQUAL(0xBB, resp[25]);
    TEST_ASSERT_EQUAL(0xCC, resp[26]);
    TEST_ASSERT_EQUAL(0xDD, resp[27]);

    /* Verify invalid client mode rejection */
    req[0] = 0x24; /* Mode 4 Server packet, invalid as request */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ntp_server_process_packet(&g_ntp_server, req, resp));

    /* Verify symmetric active client request mode (mode = 1) */
    req[0] = 0x21; /* Mode 1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, req, resp));

    /* Verify processing when get_epoch_sec_cb is NULL */
    syn_ntp_server_init(&g_ntp_server, 1, NULL);
    req[0] = 0x23;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, req, resp));
}

static uint32_t mock_get_epoch_unsynced(void)
{
    return 0U;
}

static void test_tier_a_ntp_server_inplace_unsynced_and_mode2(void)
{
    /* 1. In-place buffer (request_pkt == response_pkt) + Mode 1 -> Mode 2 (0x22) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_init(&g_ntp_server, 3U, mock_get_epoch));
    uint8_t pkt[48] = {0};
    pkt[0] = 0x21U; /* VN=4, Mode=1 (Symmetric Active) */
    const uint8_t tx_ts[8] = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U, 0x88U};
    memcpy(&pkt[40], tx_ts, sizeof(tx_ts));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, pkt, pkt));
    TEST_ASSERT_EQUAL_HEX8(0x22U, pkt[0]); /* LI=0, VN=4, Mode=2 (Symmetric Passive) */
    TEST_ASSERT_EQUAL_UINT8(3U, pkt[1]);
    TEST_ASSERT_EQUAL_MEMORY(tx_ts, &pkt[24], sizeof(tx_ts));

    /* 2. Unsynced callback (returns 0) emits LI=3, stratum=0 (0xE4 for Mode 3, 0xE2 for Mode 1) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_init(&g_ntp_server, 2U, mock_get_epoch_unsynced));
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0x23U;
    memcpy(&pkt[40], tx_ts, sizeof(tx_ts));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, pkt, pkt));
    TEST_ASSERT_EQUAL_HEX8(0xE4U, pkt[0]);
    TEST_ASSERT_EQUAL_UINT8(0U, pkt[1]);
    TEST_ASSERT_EQUAL_MEMORY(tx_ts, &pkt[24], sizeof(tx_ts));

    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0x21U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ntp_server_process_packet(&g_ntp_server, pkt, pkt));
    TEST_ASSERT_EQUAL_HEX8(0xE2U, pkt[0]);
    TEST_ASSERT_EQUAL_UINT8(0U, pkt[1]);
}

void run_ntp_server_tests(void)
{
    RUN_TEST(test_ntp_server_init_and_null_checks);
    RUN_TEST(test_ntp_server_process_packet);
    RUN_TEST(test_tier_a_ntp_server_inplace_unsynced_and_mode2);
}
