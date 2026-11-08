/**
 * @file test_sntp.c
 * @brief Unit tests for syn_sntp (Simple NTP client).
 *
 * Tests init, query parsing, epoch calculation, and edge cases
 * using the mock UDP socket infrastructure.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_sntp.h"
#include "unity/unity.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * Build a minimal valid NTP server response packet.
 * Only the fields checked by syn_sntp_query are set:
 *   - Byte 0:  LI=0, VN=4, Mode=4 (server)
 *   - Byte 1:  Stratum (non-zero)
 *   - Bytes 40-43: Transmit timestamp seconds (big-endian, NTP epoch)
 *   - Bytes 44-47: Transmit timestamp fraction (big-endian)
 */
static void build_ntp_response(uint8_t *pkt, uint32_t unix_epoch_s, uint32_t frac)
{
    memset(pkt, 0, SYN_SNTP_PACKET_SIZE);
    pkt[0] = 0x24; /* LI=0, VN=4, Mode=4 (server) */
    pkt[1] = 2;    /* Stratum 2 */

    /* Convert Unix epoch → NTP epoch */
    uint32_t ntp_s = unix_epoch_s + SYN_SNTP_EPOCH_OFFSET;

    pkt[40] = (uint8_t)(ntp_s >> 24);
    pkt[41] = (uint8_t)(ntp_s >> 16);
    pkt[42] = (uint8_t)(ntp_s >> 8);
    pkt[43] = (uint8_t)(ntp_s);
    pkt[44] = (uint8_t)(frac >> 24);
    pkt[45] = (uint8_t)(frac >> 16);
    pkt[46] = (uint8_t)(frac >> 8);
    pkt[47] = (uint8_t)(frac);
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

static void test_sntp_init(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {216, 239, 35, 0}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    TEST_ASSERT_FALSE(sntp.synced);
    TEST_ASSERT_EQUAL_UINT32(0, sntp.epoch_s);
    TEST_ASSERT_EQUAL_UINT32(3600, sntp.sync_interval_s);
    TEST_ASSERT_EQUAL_UINT16(123, sntp.server.port);
    TEST_ASSERT_EQUAL_UINT8(216, sntp.server.ip[0]);
}

static void test_sntp_not_synced_returns_zero(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    TEST_ASSERT_EQUAL_UINT32(0, syn_sntp_get_epoch_s(&sntp));
    TEST_ASSERT_EQUAL_UINT32(0, syn_sntp_get_epoch_ns(&sntp));
    TEST_ASSERT_FALSE(syn_sntp_is_synced(&sntp));
}

static void test_sntp_drift_and_consecutive_sync(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];

    syn_sntp_init(&sntp, &server, 3600);

    /* Sync 1 */
    build_ntp_response(pkt, 1000, 0);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));

    /* Sync 2 after 5 seconds (5000 ms tick advance) */
    mock_tick_advance(5000);
    build_ntp_response(pkt, 1005, 0);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));

    TEST_ASSERT_EQUAL_UINT32(1005, syn_sntp_get_epoch_s(&sntp));
    TEST_ASSERT_EQUAL_INT32(sntp.drift_ppm, syn_sntp_get_drift_ppm(&sntp));
    TEST_ASSERT_EQUAL_INT32(0, syn_sntp_get_drift_ppm(NULL));
}

static void test_sntp_query_success(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    /* Set up mock response: Unix epoch = 1700000000 (Nov 14, 2023) */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);

    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    mock_tick_ms = 5000; /* Simulate 5s uptime at sync time */

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_OK, status);
    TEST_ASSERT_TRUE(syn_sntp_is_synced(&sntp));
    TEST_ASSERT_EQUAL_UINT32(1700000000UL, sntp.epoch_s);
    TEST_ASSERT_EQUAL_UINT32(5000, sntp.sync_tick_ms);
}

static void test_sntp_get_epoch_advances_with_tick(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    mock_tick_ms = 10000;
    syn_sntp_query(&sntp);

    /* Advance tick by 5 seconds */
    mock_tick_ms = 15000;
    TEST_ASSERT_EQUAL_UINT32(1700000005UL, syn_sntp_get_epoch_s(&sntp));

    /* Advance by 60 seconds total */
    mock_tick_ms = 70000;
    TEST_ASSERT_EQUAL_UINT32(1700000060UL, syn_sntp_get_epoch_s(&sntp));
}

static void test_sntp_get_epoch_ns(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    mock_tick_ms = 0;
    syn_sntp_query(&sntp);

    /* 500ms elapsed → 500,000,000 ns */
    mock_tick_ms = 500;
    TEST_ASSERT_EQUAL_UINT32(500000000UL, syn_sntp_get_epoch_ns(&sntp));

    /* 1500ms → sub-second is 500ms → 500,000,000 ns */
    mock_tick_ms = 1500;
    TEST_ASSERT_EQUAL_UINT32(500000000UL, syn_sntp_get_epoch_ns(&sntp));
}

static void test_sntp_query_timeout(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    /* No response data → recvfrom returns 0 */
    /* mock_udp_rx_len is 0 by default */

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_TIMEOUT, status);
    TEST_ASSERT_FALSE(syn_sntp_is_synced(&sntp));
}

static void test_sntp_query_socket_fail(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    mock_udp_open_ok = false;

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_ERROR, status);
    TEST_ASSERT_FALSE(syn_sntp_is_synced(&sntp));
}

static void test_sntp_query_bad_mode(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    /* Build response with wrong mode (Mode=3 = client, should be 4 or 5) */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    resp[0] = 0x23; /* Mode=3 (client) — invalid for a response */

    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_ERROR, status);
}

static void test_sntp_query_kiss_of_death(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    resp[1] = 0; /* Stratum 0 = kiss-of-death */

    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_ERROR, status);
}

static void test_sntp_query_sendto_fail(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    mock_udp_sendto_fail = true;

    SYN_Status status = syn_sntp_query(&sntp);

    TEST_ASSERT_EQUAL(SYN_ERROR, status);
}

static void test_sntp_request_packet_format(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};

    syn_sntp_init(&sntp, &server, 3600);

    /* Provide response so query completes */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    syn_sntp_query(&sntp);

    /* Verify the request packet that was sent */
    TEST_ASSERT_EQUAL(SYN_SNTP_PACKET_SIZE, mock_udp_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x23, mock_udp_tx_buf[0]); /* LI=0, VN=4, Mode=3 */

    /* Rest of request should be zeros */
    for (int i = 1; i < SYN_SNTP_PACKET_SIZE; i++) {
        TEST_ASSERT_EQUAL_HEX8(0, mock_udp_tx_buf[i]);
    }
}

static void test_sntp_task_success(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    SYN_Task task = {.user_data = &sntp};
    SYN_PT pt;
    PT_INIT(&pt);

    syn_sntp_init(&sntp, &server, 3600);
    sntp.sync_interval_s = 10; /* Short interval for test */

    /* Phase 1: Task should open socket and send request */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    /* Run task until it waits for response or succeeds */
    for (int i = 0; i < 10; i++) {
        syn_sntp_task(&pt, &task);
        if (sntp.synced)
            break;
        mock_tick_ms += 100;
    }

    TEST_ASSERT_TRUE(sntp.synced);
    TEST_ASSERT_EQUAL_UINT32(1700000000UL, sntp.epoch_s);
}

static void test_sntp_task_fail_and_retry(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    SYN_Task task = {.user_data = &sntp};
    SYN_PT pt;
    PT_INIT(&pt);

    syn_sntp_init(&sntp, &server, 3600);
    sntp.sync_interval_s = 10;

    /* Fail socket open first */
    mock_udp_open_ok = false;
    syn_sntp_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    /* Advance to clear delay */
    mock_tick_ms += 1100;
    mock_udp_open_ok = true;

    /* Fail send */
    mock_udp_sendto_fail = true;
    syn_sntp_task(&pt, &task); /* next step: open ok, send fail */
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    /* Advance to clear delay */
    mock_tick_ms += 2100;
    mock_udp_sendto_fail = false;

    /* Succeed finally */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    for (int i = 0; i < 10; i++) {
        syn_sntp_task(&pt, &task);
        if (sntp.synced)
            break;
        mock_tick_ms += 100;
    }

    TEST_ASSERT_TRUE(sntp.synced);
}

static void test_sntp_task_timeout_and_retry(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    SYN_Task task = {.user_data = &sntp};
    SYN_PT pt;
    PT_INIT(&pt);

    syn_sntp_init(&sntp, &server, 3600);
    sntp.sync_interval_s = 10;

    /* 1. Send ok, but NO response (timeout) */
    mock_udp_rx_count = 0;
    syn_sntp_task(&pt, &task); /* Phase 1: Open/Send */

    /* Advance time beyond timeout */
    mock_tick_ms += SYN_SNTP_TIMEOUT_MS + 100;
    syn_sntp_task(&pt, &task); /* Phase 2: Detect timeout, Close, Start Delay */

    TEST_ASSERT_FALSE(sntp.synced);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    /* 2. Advance time to clear RFC 7252 backoff delay ([1000, 1500] ms on attempt 1) */
    mock_tick_ms += 1500;

    /* 3. Provide valid response now */
    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000000UL, 0);
    SYN_SockAddr from = {.ip = {1, 2, 3, 4}, .port = 123};
    mock_udp_set_response(resp, SYN_SNTP_PACKET_SIZE, &from);

    for (int i = 0; i < 10; i++) {
        syn_sntp_task(&pt, &task);
        if (sntp.synced)
            break;
        mock_tick_ms += 100;
    }

    TEST_ASSERT_TRUE(sntp.synced);
}

static void test_sntp_task_send_fail(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    SYN_Task task = {.user_data = &sntp};
    SYN_PT pt;
    PT_INIT(&pt);

    syn_sntp_init(&sntp, &server, 3600);

    mock_udp_sendto_fail = true;
    syn_sntp_task(&pt, &task); /* Phase 1: Open socket ok */
    syn_sntp_task(&pt, &task); /* Phase 2: Send fails -> triggers line 224 continue */

    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);
    TEST_ASSERT_EQUAL(1, sntp.backoff.attempts);
    mock_udp_sendto_fail = false;
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

static void test_sntp_kiss_of_death_and_stratum_validation(void)
{
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    syn_sntp_init(&sntp, &server, 3600);

    /* 1. Truncated NTP response (len < 48) -> line 55 */
    uint8_t short_pkt[20];
    memset(short_pkt, 0, sizeof(short_pkt));
    short_pkt[0] = 0x24;
    short_pkt[1] = 2;
    mock_udp_set_response(short_pkt, sizeof(short_pkt), &server);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    /* 2. ntp_s < SYN_SNTP_EPOCH_OFFSET (line 69) */
    uint8_t old_epoch_pkt[SYN_SNTP_PACKET_SIZE];
    memset(old_epoch_pkt, 0, sizeof(old_epoch_pkt));
    old_epoch_pkt[0] = 0x24;
    old_epoch_pkt[1] = 2;
    /* ntp_s = 0x00000001 (less than SYN_SNTP_EPOCH_OFFSET 2208988800UL) */
    old_epoch_pkt[43] = 1;
    mock_udp_set_response(old_epoch_pkt, sizeof(old_epoch_pkt), &server);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    /* 3. Extreme positive & negative drift PPM clamping (lines 85 & 87) */
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(pkt, 1000, 0);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    syn_sntp_query(&sntp);

    /* Huge positive drift: 5s elapsed NTP time, 10s elapsed local tick time -> +1000000 PPM */
    mock_tick_advance(10000);
    build_ntp_response(pkt, 1005, 0);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    syn_sntp_query(&sntp);
    TEST_ASSERT_TRUE(sntp.drift_ppm > 0);

    /* Huge negative drift: 10s elapsed NTP time, 5s elapsed local tick time -> -500000 PPM */
    mock_tick_advance(5000);
    build_ntp_response(pkt, 1015, 0);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    syn_sntp_query(&sntp);

    /* 4. Short packet response < 48 bytes (line 55) */
    uint8_t short_sntp_pkt[20] = {0};
    mock_udp_set_response(short_sntp_pkt, sizeof(short_sntp_pkt), &server);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    /* 5. Send request failure in sntp task (line 224) */
    mock_udp_sendto_fail = true;
    syn_sntp_init(&sntp, &server, 1);
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &sntp};
    syn_sntp_task(&pt, &task);
    mock_udp_sendto_fail = false;

    /* syn_sntp_is_synced(NULL) check for syn_sntp.h inline branch */
    TEST_ASSERT_FALSE(syn_sntp_is_synced(NULL));
    TEST_ASSERT_EQUAL_INT32(0, syn_sntp_get_drift_ppm(NULL));
}

static void test_sntp_task_null_guards_runt_rx_and_n04_second_cycle_retry(void)
{
    mock_port_reset();

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task empty_task = {0};

    /* 1. NULL parameter guards (syn_sntp.c:204-206) */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_sntp_task(NULL, &empty_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_sntp_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_sntp_task(&pt, &empty_task));

    /* 2. R5-N13: Runt UDP packet (0 < n < 48) from server fails fast into backoff */
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {1, 2, 3, 4}, .port = 123};
    syn_sntp_init(&sntp, &server, 10);
    SYN_Task task = {0};
    task.user_data = &sntp;
    PT_INIT(&pt);

    uint8_t runt_pkt[12] = {0x24, 2};
    mock_udp_set_response(runt_pkt, sizeof(runt_pkt), &server);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_FALSE(sntp.synced);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);
    TEST_ASSERT_EQUAL_UINT8(1U, sntp.backoff.attempts);

    /* 3. Regression N-04: after 1st sync succeeds (sntp.synced == true), advance past
     *    sync_interval_s into 2nd sync cycle, drop 1st UDP query so it times out, and verify
     *    syn_sntp_task retries on attempt #2 instead of breaking out early due to sntp.synced */
    mock_port_reset();
    syn_sntp_init(&sntp, &server, 10);
    memset(&task, 0, sizeof(task));
    task.user_data = &sntp;
    PT_INIT(&pt);

    uint8_t resp1[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp1, 1700000000UL, 0);
    mock_udp_set_response(resp1, sizeof(resp1), &server);

    /* Cycle 1: succeeds on attempt #1 and parks on 10s sync interval delay */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_TRUE(sntp.synced);
    TEST_ASSERT_TRUE(sntp.cycle_ok);
    TEST_ASSERT_EQUAL_UINT32(1700000000UL, sntp.epoch_s);

    /* Advance 10,000 ms to enter Cycle 2 with NO UDP response (simulating dropped packet) */
    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_tx_len = 0;
    mock_tick_advance(10000U);

    /* Cycle 2, Attempt #1: sends NTP request and waits up to SYN_SNTP_TIMEOUT_MS */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_SNTP_PACKET_SIZE, mock_udp_tx_len);
    TEST_ASSERT_FALSE(sntp.cycle_ok);
    TEST_ASSERT_TRUE(sntp.synced); /* Still true from Cycle 1 */

    /* Expire Attempt #1 timeout -> must NOT exit retry loop; must enter retry backoff
     * (1000..1500ms) */
    mock_tick_advance(SYN_SNTP_TIMEOUT_MS + 100U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_UINT8(1, sntp.backoff.attempts);
    TEST_ASSERT_FALSE(sntp.cycle_ok);

    /* Advance 1,600 ms (past retry #1 backoff, well below 10,000 ms poll_interval_s!)
     * and inject valid response for Attempt #2 */
    uint8_t resp2[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp2, 1700000014UL, 0);
    mock_udp_set_response(resp2, sizeof(resp2), &server);
    mock_udp_tx_len = 0;
    mock_tick_advance(1600U);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_SNTP_PACKET_SIZE, mock_udp_tx_len); /* Attempt #2 transmitted */
    TEST_ASSERT_TRUE(sntp.cycle_ok);
    TEST_ASSERT_EQUAL_UINT32(1700000014UL, sntp.epoch_s);
}

static void test_sntp_round3_regressions(void)
{
    mock_port_reset();

    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {10, 0, 0, 1}, .port = 123};

    /* 1. R3-N08: syn_sntp_init clamps 0 -> 1 and > 2147483 -> 2147483 */
    syn_sntp_init(&sntp, &server, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, sntp.sync_interval_s);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    syn_sntp_init(&sntp, &server, 0xFFFFFFFFU);
    TEST_ASSERT_EQUAL_UINT32(2147483U, sntp.sync_interval_s);

    /* 2. R3-N08: runtime sync_interval_s == 0U guards against zero-delay flood */
    mock_port_reset();
    syn_sntp_init(&sntp, &server, 10U);
    sntp.sync_interval_s = 0U;

    uint8_t resp[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(resp, 1700000100UL, 0);
    mock_udp_set_response(resp, sizeof(resp), &server);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &sntp;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_TRUE(sntp.synced);

    /* At +500 ms (< 1000 ms floor), task must remain sleeping and not send another request */
    mock_udp_tx_len = 0;
    mock_tick_advance(500U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_size_t(0U, mock_udp_tx_len);

    /* At +1000 ms total, floor expires and cycle 2 sends a request */
    mock_tick_advance(500U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_size_t(SYN_SNTP_PACKET_SIZE, mock_udp_tx_len);

    /* 3. R3-N12 & R4-T11: mid-flight PT_INIT restart while udp_sock is open closes exact socket */
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);
    SYN_Socket leaked_candidate = sntp.udp_sock;
    int closes_before = mock_sock_close_count;
    mock_udp_open_ok = false;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(closes_before + 1, mock_sock_close_count);
    TEST_ASSERT_EQUAL(leaked_candidate, mock_sock_last_closed);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);
    mock_udp_open_ok = true;

    sntp.sync_interval_s = 3000000U; /* R3-N08: > 2147483U clamps to 2147483000 ms */
    build_ntp_response(resp, 1700000200UL, 0);
    mock_udp_set_response(resp, sizeof(resp), &server);
    PT_INIT(&pt);
    uint32_t base_tick = mock_tick_ms;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_UINT32(base_tick + 2147483000U, task.delay_until);

    /* 4. Send failure backoff resume (covers continue after send failure) */
    mock_port_reset();
    syn_sntp_init(&sntp, &server, 5U);
    PT_INIT(&pt);
    memset(&task, 0, sizeof(task));
    task.user_data = &sntp;
    mock_udp_sendto_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_EQUAL_UINT8(1U, sntp.backoff.attempts);
    mock_udp_sendto_fail = false;
    mock_tick_advance(2000U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    /* 5. R3-N06: mock_udp_recv_fail during poll yields PT_IDLE cleanly without spinning */
    mock_udp_recv_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    mock_udp_recv_fail = false;
    syn_port_sock_close(sntp.udp_sock);
    sntp.udp_sock = SYN_SOCKET_INVALID;

    /* 6. Unmatched protothread line falls through to PT_END */
    pt.lc = 0xFFFFU;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_sntp_task(&pt, &task));
}

static void test_sntp_round4_li_stratum_sender_and_poll_window(void)
{
    mock_port_reset();

    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {10, 20, 30, 40}, .port = 123};
    SYN_SockAddr spoofer = {.ip = {9, 9, 9, 9}, .port = 123};
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];

    syn_sntp_init(&sntp, &server, 60U);

    /* 1. R4-N13: LI == 3 (alarm unsynchronized) rejected */
    build_ntp_response(pkt, 1700001000UL, 0U);
    pkt[0] = 0xE4U; /* LI=3, VN=4, Mode=4 */
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    /* 2. R4-N13: stratum == 16 (> 15) rejected; stratum == 15 + Mode=5 accepted */
    build_ntp_response(pkt, 1700001000UL, 0U);
    pkt[1] = 16U;
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    build_ntp_response(pkt, 1700001000UL, 0U);
    pkt[0] = 0x25U; /* LI=0, VN=4, Mode=5 (broadcast) */
    pkt[1] = 15U;
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));

    /* 3. R4-N13: off-path sender IP rejected when server IP is non-zero; wildcard IP accepts */
    build_ntp_response(pkt, 1700001100UL, 0U);
    mock_udp_set_response(pkt, sizeof(pkt), &spoofer);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    SYN_SockAddr wildcard = {.ip = {0, 0, 0, 0}, .port = 123};
    SYN_SNTP any_sntp;
    syn_sntp_init(&any_sntp, &wildcard, 60U);
    mock_udp_set_response(pkt, sizeof(pkt), &spoofer);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&any_sntp));

    /* 4. R4-N13 + R5-N13: spoofed off-path IP stays in poll window; valid server succeeds */
    mock_port_reset();
    syn_sntp_init(&sntp, &server, 60U);

    uint8_t good_pkt[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(good_pkt, 1700001234UL, 0U);

    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_inject_packet(good_pkt, sizeof(good_pkt), &spoofer);
    mock_udp_inject_packet(good_pkt, sizeof(good_pkt), &server);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &sntp};

    /* Tick 1: consumes spoofed packet -> stays in poll window */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_FALSE(sntp.synced);
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);

    /* Tick 2: consumes genuine server packet -> sync succeeds without retry! */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_TRUE(sntp.synced);
    TEST_ASSERT_EQUAL_UINT8(0U, sntp.backoff.attempts);
    TEST_ASSERT_EQUAL_UINT32(1700001234UL, sntp.epoch_s);
}

static void test_r5_n13_sntp_kod_fast_fail(void)
{
    mock_port_reset();

    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {10, 20, 30, 40}, .port = 123};
    syn_sntp_init(&sntp, &server, 60U);

    /* Stratum 0 Kiss-o'-Death from server must fail fast at t=0 without waiting 5s timeout */
    uint8_t kod_pkt[SYN_SNTP_PACKET_SIZE];
    build_ntp_response(kod_pkt, 1700002000UL, 0U);
    kod_pkt[1] = 0U; /* Stratum 0 KoD */
    memcpy(&kod_pkt[12], "RATE", 4U);
    mock_udp_set_response(kod_pkt, sizeof(kod_pkt), &server);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &sntp};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_sntp_task(&pt, &task));
    TEST_ASSERT_FALSE(sntp.synced);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, sntp.udp_sock);
    TEST_ASSERT_EQUAL_UINT8(1U, sntp.backoff.attempts);
}

static void test_tier_a_sntp_stepback_frac_null_and_port(void)
{
    mock_port_reset();

    /* 1. NULL guards on epoch getters */
    TEST_ASSERT_EQUAL_UINT32(0U, syn_sntp_get_epoch_s(NULL));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_sntp_get_epoch_ns(NULL));

    /* 2. Fractional NTP seconds + rollover carry into seconds */
    SYN_SNTP sntp;
    SYN_SockAddr server = {.ip = {10, 20, 30, 40}, .port = 123U};
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];

    syn_sntp_init(&sntp, &server, 60U);
    build_ntp_response(pkt, 1700005000UL, 0xC0000000UL); /* 0.75s = 750,000,000 ns */
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));
    TEST_ASSERT_EQUAL_UINT32(1700005000UL, syn_sntp_get_epoch_s(&sntp));
    TEST_ASSERT_EQUAL_UINT32(750000000UL, syn_sntp_get_epoch_ns(&sntp));

    /* +500 ms -> 750,000,000 + 500,000,000 = 1,250,000,000 ns -> +1s carry + 250,000,000 ns */
    mock_tick_advance(500U);
    TEST_ASSERT_EQUAL_UINT32(1700005001UL, syn_sntp_get_epoch_s(&sntp));
    TEST_ASSERT_EQUAL_UINT32(250000000UL, syn_sntp_get_epoch_ns(&sntp));

    /* 3. Server clock step-back does not underflow uint32_t into bogus drift_ppm */
    mock_tick_advance(5000U);
    build_ntp_response(pkt, 1700004990UL, 0U);
    mock_udp_set_response(pkt, sizeof(pkt), &server);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));
    TEST_ASSERT_EQUAL_INT32(0, syn_sntp_get_drift_ppm(&sntp));
    TEST_ASSERT_EQUAL_UINT32(1700004990UL, syn_sntp_get_epoch_s(&sntp));

    /* 4. Source port check: wrong port rejected, port 0 wildcard accepted */
    SYN_SockAddr wrong_port = {.ip = {10, 20, 30, 40}, .port = 9999U};
    mock_udp_set_response(pkt, sizeof(pkt), &wrong_port);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    SYN_SockAddr wildcard_ip = {.ip = {0, 0, 0, 0}, .port = 123U};
    syn_sntp_init(&sntp, &wildcard_ip, 60U);
    mock_udp_set_response(pkt, sizeof(pkt), &wrong_port);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_sntp_query(&sntp));

    SYN_SockAddr wildcard_port = {.ip = {10, 20, 30, 40}, .port = 0U};
    syn_sntp_init(&sntp, &wildcard_port, 60U);
    mock_udp_set_response(pkt, sizeof(pkt), &wrong_port);
    TEST_ASSERT_EQUAL(SYN_OK, syn_sntp_query(&sntp));
}

void run_sntp_tests(void)
{
    RUN_TEST(test_sntp_init);
    RUN_TEST(test_sntp_not_synced_returns_zero);
    RUN_TEST(test_sntp_query_success);
    RUN_TEST(test_sntp_drift_and_consecutive_sync);
    RUN_TEST(test_sntp_get_epoch_advances_with_tick);
    RUN_TEST(test_sntp_get_epoch_ns);
    RUN_TEST(test_sntp_query_timeout);
    RUN_TEST(test_sntp_query_socket_fail);
    RUN_TEST(test_sntp_query_bad_mode);
    RUN_TEST(test_sntp_query_kiss_of_death);
    RUN_TEST(test_sntp_query_sendto_fail);
    RUN_TEST(test_sntp_request_packet_format);
    RUN_TEST(test_sntp_task_success);
    RUN_TEST(test_sntp_task_fail_and_retry);
    RUN_TEST(test_sntp_task_timeout_and_retry);
    RUN_TEST(test_sntp_task_send_fail);
    RUN_TEST(test_sntp_kiss_of_death_and_stratum_validation);
    RUN_TEST(test_sntp_task_null_guards_runt_rx_and_n04_second_cycle_retry);
    RUN_TEST(test_sntp_round3_regressions);
    RUN_TEST(test_sntp_round4_li_stratum_sender_and_poll_window);
    RUN_TEST(test_r5_n13_sntp_kod_fast_fail);
    RUN_TEST(test_tier_a_sntp_stepback_frac_null_and_port);
}
