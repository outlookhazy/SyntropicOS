/**
 * @file test_autoip.c
 * @brief Unit tests for Zero-Heap Native RFC 3927 AutoIP Engine.
 */

#include "syntropic/net/syn_autoip.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

void test_autoip_init(void)
{
    SYN_AUTOIP autoip;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_init(&autoip, MAC));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_INIT, autoip.state);

    /* Candidate IP should fall inside 169.254.1.1 .. 169.254.254.255 */
    TEST_ASSERT_EQUAL_UINT32(0xA9FE0000UL, autoip.ip_addr & 0xFFFF0000UL);
}

void test_autoip_probe_and_announce(void)
{
    SYN_AUTOIP autoip;
    syn_autoip_init(&autoip, MAC);

    uint8_t probe_frame[60];
    size_t probe_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_frame, &probe_len));
    TEST_ASSERT_EQUAL_INT(60, probe_len);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_PROBE, autoip.state);
    TEST_ASSERT_EQUAL_UINT8(1, autoip.probe_count);

    uint8_t announce_frame[60];
    size_t announce_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_autoip_build_announce(&autoip, MAC, announce_frame, &announce_len));
    TEST_ASSERT_EQUAL_INT(60, announce_len);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_ANNOUNCE, autoip.state);
    TEST_ASSERT_EQUAL_UINT8(1, autoip.announce_count);
}

static void test_autoip_round4_simultaneous_probe_conflict(void)
{
    SYN_AUTOIP autoip;
    uint8_t probe_frame[60];
    size_t probe_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_init(&autoip, MAC));
    uint32_t initial_ip = autoip.ip_addr;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_frame, &probe_len));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_PROBE, autoip.state);

    /* Peer ARP Probe for a DIFFERENT target IP (sender_ip == 0) does not collide */
    uint8_t other_probe[60] = {0};
    memcpy(other_probe, probe_frame, sizeof(other_probe));
    other_probe[41] ^= 0x01U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, NULL, other_probe, 60));

    /* Simultaneous peer ARP Probe (sender_ip == 0, target_ip == autoip.ip_addr) in PROBE state */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_init(&autoip, MAC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_frame, &probe_len));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_autoip_process_arp(&autoip, NULL, probe_frame, 60));
    TEST_ASSERT_EQUAL_UINT8(1U, autoip.collisions);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_INIT, autoip.state);
    TEST_ASSERT_NOT_EQUAL(initial_ip, autoip.ip_addr);
}

void test_autoip_process_arp_binding(void)
{
    SYN_AUTOIP autoip;
    SYN_ETH eth;
    uint8_t probe_buf[60];
    size_t probe_len = 0;

    syn_autoip_init(&autoip, MAC);
    syn_eth_init(&eth, MAC, 0);

    /* ARP frame from different sender IP (no conflict) */
    uint8_t arp_frame[60] = {0};
    arp_frame[12] = 0x08;
    arp_frame[13] = 0x06;
    arp_frame[28] = 10;
    arp_frame[29] = 0;
    arp_frame[30] = 0;
    arp_frame[31] = 1;

    /* While probing (< 3 probes), non-conflicting ARP must NOT jump to BOUND */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, arp_frame, 60));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_INIT, autoip.state);
    TEST_ASSERT_EQUAL_UINT32(0U, eth.ip_addr);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_buf, &probe_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, arp_frame, 60));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_PROBE, autoip.state);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_buf, &probe_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, arp_frame, 60));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_PROBE, autoip.state);

    /* After 3rd RFC 3927 probe completes without conflict, AutoIP binds cleanly */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, probe_buf, &probe_len));
    TEST_ASSERT_EQUAL_UINT8(3U, autoip.probe_count);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, arp_frame, 60));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_BOUND, autoip.state);
    TEST_ASSERT_EQUAL_UINT32(autoip.ip_addr, eth.ip_addr);
    TEST_ASSERT_EQUAL_UINT32(SYN_AUTOIP_NETMASK, eth.netmask);

    test_autoip_round4_simultaneous_probe_conflict();
}

static SYN_PT_Status autoip_coroutine_task(SYN_PT *pt, SYN_AUTOIP *autoip)
{
    PT_BEGIN(pt);
    PT_AUTOIP_WAIT_BOUND(pt, autoip);
    PT_END(pt);
}

void test_autoip_coroutine_pt(void)
{
    SYN_AUTOIP autoip;
    syn_autoip_init(&autoip, MAC);

    SYN_PT pt;
    PT_INIT(&pt);

    /* First step: AutoIP state != BOUND -> coroutine yields (PT_IDLE) */
    TEST_ASSERT_EQUAL_INT(PT_IDLE, autoip_coroutine_task(&pt, &autoip));

    autoip.state = SYN_AUTOIP_STATE_BOUND;

    /* Second step: AutoIP state == BOUND -> coroutine completes (PT_ENDED) */
    TEST_ASSERT_EQUAL_INT(PT_ENDED, autoip_coroutine_task(&pt, &autoip));
}

void test_autoip_null_checks(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_autoip_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_autoip_build_probe(NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_autoip_build_announce(NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_autoip_process_arp(NULL, NULL, NULL, 0));

    /* Non-ARP frame -> return SYN_OK */
    SYN_AUTOIP autoip;
    syn_autoip_init(&autoip, MAC);
    uint8_t ip_frame[60] = {0};
    ip_frame[12] = 0x08;
    ip_frame[13] = 0x00; /* IPv4 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_autoip_process_arp(&autoip, NULL, ip_frame, 60));

    /* ARP Conflict: sender IP matches candidate autoip->ip_addr */
    uint8_t conflict_arp[60] = {0};
    conflict_arp[12] = 0x08;
    conflict_arp[13] = 0x06;
    conflict_arp[28] = (uint8_t)(autoip.ip_addr >> 24);
    conflict_arp[29] = (uint8_t)(autoip.ip_addr >> 16);
    conflict_arp[30] = (uint8_t)(autoip.ip_addr >> 8);
    conflict_arp[31] = (uint8_t)(autoip.ip_addr);

    TEST_ASSERT_EQUAL(SYN_BUSY, syn_autoip_process_arp(&autoip, NULL, conflict_arp, 60));
    TEST_ASSERT_EQUAL_UINT8(1, autoip.collisions);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_INIT, autoip.state);

    /* ARP Conflict with octet4 = 254 -> rollover to 1 */
    autoip.ip_addr = 0xA9FE01FE; /* 169.254.1.254 */
    conflict_arp[28] = 0xA9;
    conflict_arp[29] = 0xFE;
    conflict_arp[30] = 0x01;
    conflict_arp[31] = 0xFE;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_autoip_process_arp(&autoip, NULL, conflict_arp, 60));
    TEST_ASSERT_EQUAL_UINT32(0xA9FE0101, autoip.ip_addr);
}

void test_r5_autoip_regressions(void)
{
    SYN_AUTOIP autoip;
    uint8_t frame[60];
    size_t frame_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_init(&autoip, MAC));

    /* 1. 3 probes -> 2 announcements transitions PROBE -> ANNOUNCE -> BOUND */
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, frame, &frame_len));
    }
    TEST_ASSERT_EQUAL_UINT8(3U, autoip.probe_count);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_announce(&autoip, MAC, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_ANNOUNCE, autoip.state);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_announce(&autoip, MAC, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_BOUND, autoip.state);

    /* 2. Conflict resets probe_count & announce_count back to 0 */
    uint8_t conflict[60] = {0};
    conflict[12] = 0x08;
    conflict[13] = 0x06;
    conflict[28] = (uint8_t)(autoip.ip_addr >> 24U);
    conflict[29] = (uint8_t)(autoip.ip_addr >> 16U);
    conflict[30] = (uint8_t)(autoip.ip_addr >> 8U);
    conflict[31] = (uint8_t)(autoip.ip_addr);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_autoip_process_arp(&autoip, NULL, conflict, 60U));
    TEST_ASSERT_EQUAL_UINT8(0U, autoip.probe_count);
    TEST_ASSERT_EQUAL_UINT8(0U, autoip.announce_count);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_INIT, autoip.state);

    /* 3. Short ARP frame (< 42B) rejected and eth == NULL post-probe bind safe */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_autoip_process_arp(&autoip, NULL, conflict, 20U));
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, frame, &frame_len));
    }
    uint8_t ok_arp[60] = {0};
    ok_arp[12] = 0x08;
    ok_arp[13] = 0x06;
    ok_arp[28] = 192;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, NULL, ok_arp, 60U));
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_BOUND, autoip.state);
}

void test_tier_a_autoip_regressions(void)
{
    SYN_AUTOIP autoip;
    SYN_ETH eth;
    uint8_t frame[60];
    size_t frame_len = 0U;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_init(&autoip, MAC));
    syn_eth_init(&eth, MAC, 0U);
    uint32_t candidate_ip = autoip.ip_addr;

    /* NET-13a: reflected self-originated ARP Probe and Announcement must be ignored */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_probe(&autoip, MAC, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, frame, frame_len));
    TEST_ASSERT_EQUAL_UINT8(0U, autoip.collisions);
    TEST_ASSERT_EQUAL_UINT32(candidate_ip, autoip.ip_addr);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_PROBE, autoip.state);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_build_announce(&autoip, MAC, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_autoip_process_arp(&autoip, &eth, frame, frame_len));
    TEST_ASSERT_EQUAL_UINT8(0U, autoip.collisions);
    TEST_ASSERT_EQUAL_UINT32(candidate_ip, autoip.ip_addr);
    TEST_ASSERT_EQUAL_INT(SYN_AUTOIP_STATE_ANNOUNCE, autoip.state);
}
