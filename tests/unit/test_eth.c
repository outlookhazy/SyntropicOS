/**
 * @file test_eth.c
 * @brief Unit tests for Zero-Heap Native Ethernet II & ARP Protocol Engine.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/net/syn_icmp.h"
#include "syntropic/net/syn_igmp.h"
#include "syntropic/net/syn_tcp.h"
#include "syntropic/net/syn_transport_udp.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t MY_MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t PEER_MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
static const uint32_t MY_IP = 0xC0A80164;   /* 192.168.1.100 */
static const uint32_t PEER_IP = 0xC0A801C8; /* 192.168.1.200 */

void test_eth_generate_mac(void)
{
    uint8_t uid[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
    uint8_t mac1[6];
    uint8_t mac2[6];

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_generate_mac(uid, 12, mac1));
    TEST_ASSERT_EQUAL_UINT8(0x02, mac1[0]); /* Bit 1 set -> Locally Administered MAC */

    /* Null parameter error test */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_generate_mac(NULL, 0, NULL));

    /* Deterministic: Same UID produces exact same MAC */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_generate_mac(uid, 12, mac2));
    TEST_ASSERT_EQUAL_INT(0, memcmp(mac1, mac2, 6));

    /* Different UID produces different MAC */
    uid[0] = 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_generate_mac(uid, 12, mac2));
    TEST_ASSERT_NOT_EQUAL(0, memcmp(mac1, mac2, 6));
}

void test_eth_init(void)
{
    SYN_ETH eth;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_init(&eth, MY_MAC, MY_IP));
    TEST_ASSERT_EQUAL_UINT32(MY_IP, eth.ip_addr);
    TEST_ASSERT_EQUAL_INT(0, memcmp(MY_MAC, eth.mac_addr, 6));
}

void test_eth_arp_cache(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t mac_out[6];
    /* Miss must return SYN_NOT_FOUND, not SYN_ERROR */
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, syn_eth_arp_lookup(&eth, PEER_IP, mac_out));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, PEER_IP, PEER_MAC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, PEER_IP, mac_out));
    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, mac_out, 6));
}

/* Regression: arp_lookup must return SYN_NOT_FOUND, not the generic SYN_ERROR. */
void test_eth_arp_lookup_returns_not_found(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t mac_out[6];
    SYN_Status st = syn_eth_arp_lookup(&eth, 0xDEADBEEF, mac_out);
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, st);
    /* Confirm it's specifically NOT SYN_ERROR */
    TEST_ASSERT_NOT_EQUAL((int)SYN_ERROR, (int)st);
}

void test_eth_build_frame(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t payload[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t frame[128];
    size_t frame_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_build_frame(&eth, PEER_MAC, SYN_ETHTYPE_IPV4, payload, 10,
                                                      frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(60, frame_len); /* Padded to 60 bytes minimum */

    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, &frame[0], 6));
    TEST_ASSERT_EQUAL_INT(0, memcmp(MY_MAC, &frame[6], 6));
    TEST_ASSERT_EQUAL_UINT8(0x08, frame[12]);
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[13]);
    TEST_ASSERT_EQUAL_UINT8(1, frame[14]);
    TEST_ASSERT_EQUAL_UINT8(10, frame[23]);
}

void test_eth_process_arp_request(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Construct ARP Request frame asking for MY_IP */
    uint8_t arp_req_frame[60] = {0};

    /* Dst MAC: Broadcast */
    memset(&arp_req_frame[0], 0xFF, 6);
    /* Src MAC */
    memcpy(&arp_req_frame[6], PEER_MAC, 6);
    /* EtherType = ARP (0x0806) */
    arp_req_frame[12] = 0x08;
    arp_req_frame[13] = 0x06;

    /* ARP Payload */
    arp_req_frame[14] = 0;
    arp_req_frame[15] = 1; /* Ethernet */
    arp_req_frame[16] = 0x08;
    arp_req_frame[17] = 0x00; /* IPv4 */
    arp_req_frame[18] = 6;
    arp_req_frame[19] = 4;
    arp_req_frame[20] = 0;
    arp_req_frame[21] = 1; /* Request */

    memcpy(&arp_req_frame[22], PEER_MAC, 6);
    arp_req_frame[28] = (uint8_t)(PEER_IP >> 24);
    arp_req_frame[29] = (uint8_t)(PEER_IP >> 16);
    arp_req_frame[30] = (uint8_t)(PEER_IP >> 8);
    arp_req_frame[31] = (uint8_t)(PEER_IP);

    memset(&arp_req_frame[32], 0, 6);
    arp_req_frame[38] = (uint8_t)(MY_IP >> 24);
    arp_req_frame[39] = (uint8_t)(MY_IP >> 16);
    arp_req_frame[40] = (uint8_t)(MY_IP >> 8);
    arp_req_frame[41] = (uint8_t)(MY_IP);

    uint8_t tx_reply[128];
    size_t tx_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_eth_process_frame(&eth, arp_req_frame, 60, tx_reply, &tx_len));
    TEST_ASSERT_EQUAL_INT(60, tx_len);
    TEST_ASSERT_EQUAL_UINT32(1, eth.arp_requests);

    /* Verify ARP Reply Dst MAC is PEER_MAC */
    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, &tx_reply[0], 6));

    /* Verify ARP cache auto-learned PEER_IP -> PEER_MAC */
    uint8_t mac_learned[6];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, PEER_IP, mac_learned));
    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, mac_learned, 6));
}

static SYN_PT_Status eth_coroutine_task(SYN_PT *pt, SYN_ETH *eth, uint32_t ip, uint8_t *mac_out)
{
    PT_BEGIN(pt);
    PT_ETH_WAIT_ARP(pt, eth, ip, mac_out);
    PT_END(pt);
}

void test_eth_coroutine_pt(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    SYN_PT pt;
    PT_INIT(&pt);

    uint8_t mac[6];
    /* First step: ARP lookup fails (not found) -> coroutine yields (PT_IDLE) */
    TEST_ASSERT_EQUAL_INT(PT_IDLE, eth_coroutine_task(&pt, &eth, PEER_IP, mac));

    /* Insert entry into ARP cache */
    syn_eth_arp_update(&eth, PEER_IP, PEER_MAC);

    /* Second step: ARP lookup succeeds -> coroutine completes (PT_ENDED) */
    TEST_ASSERT_EQUAL_INT(PT_ENDED, eth_coroutine_task(&pt, &eth, PEER_IP, mac));
    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, mac, 6));
}

void test_eth_null_checks(void)
{
    SYN_ETH eth;
    uint8_t mac[6];
    uint8_t frame_out[128];
    size_t len = 0;
    uint8_t tx_buf[128];
    size_t tx_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_generate_mac(NULL, 4, mac));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_generate_mac("uid", 0, mac));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_generate_mac("uid", 4, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_init(NULL, MY_MAC, MY_IP));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_init(&eth, NULL, MY_IP));

    syn_eth_init(&eth, MY_MAC, MY_IP);

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_update(NULL, MY_IP, PEER_MAC));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_update(&eth, MY_IP, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_update(&eth, 0, PEER_MAC));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_lookup(NULL, MY_IP, mac));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_lookup(&eth, MY_IP, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(NULL, PEER_MAC, 0x0800, NULL, 0, frame_out, &len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(&eth, NULL, 0x0800, NULL, 0, frame_out, &len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(&eth, PEER_MAC, 0x0800, NULL, 0, NULL, &len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(&eth, PEER_MAC, 0x0800, NULL, 0, frame_out, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_process_frame(NULL, frame_out, 60, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_process_frame(&eth, NULL, 60, tx_buf, &tx_len));
}

void test_eth_runt_and_oversized_frames(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t runt[13] = {0};
    uint8_t tx[128];
    size_t tx_len = 0;

    /* Runt frame (< 14 bytes) must return SYN_INVALID_PARAM */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_process_frame(&eth, runt, 13, tx, &tx_len));

    /* ARP LRU Eviction when cache full (lines 98-109) */
    for (uint32_t i = 1; i <= 9; i++) {
        uint8_t mac_i[6] = {0x02, 0, 0, 0, 0, (uint8_t)i};
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, 0xC0A80100 + i, mac_i));
    }
    uint8_t evicted_mac[6];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, 0xC0A80109, evicted_mac));

    /* Frame build with NULL payload (lines 152-160) */
    uint8_t f_out[128];
    size_t f_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_eth_build_frame(&eth, PEER_MAC, 0x0800, NULL, 0, f_out, &f_len));
    TEST_ASSERT_EQUAL(60, f_len);

    /* Frame build with oversized payload (line 143) */
    /* Build frame with payload_len = 10 (triggers zero-padding up to 60 bytes, line 157) */
    uint8_t short_payload[10] = "123456789";
    uint8_t pad_frame[128];
    size_t pad_frame_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_build_frame(&eth, PEER_MAC, SYN_ETHTYPE_IPV4, short_payload,
                                                  10, pad_frame, &pad_frame_len));
    TEST_ASSERT_EQUAL(60, pad_frame_len);

    /* Process frame with non-matching MAC address (line 187) */
    uint8_t other_mac[6] = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t other_frame[60];
    memcpy(other_frame, pad_frame, 60);
    memcpy(other_frame, other_mac, 6); /* dst_mac != our mac && != broadcast */
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_process_frame(&eth, other_frame, 60, tx, &tx_len));

    /* Process runt ARP frame (len = 30 < 42, line 192) */
    uint8_t runt_arp[30];
    memset(runt_arp, 0, sizeof(runt_arp));
    memcpy(runt_arp, MY_MAC, 6); /* dst = our mac */
    runt_arp[12] = 0x08;
    runt_arp[13] = 0x06; /* ARP */
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_process_frame(&eth, runt_arp, 30, tx, &tx_len));

    uint8_t big_f_out[2000];
    uint8_t big_payload[1600] = {0};
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_eth_build_frame(&eth, PEER_MAC, 0x0800, big_payload, 1600, big_f_out, &f_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_process_frame(&eth, runt, 5, tx, &tx_len));

    /* Building frame exceeding 1514 bytes maximum length must fail */
    static uint8_t large_payload[1510];
    static uint8_t large_out[1600];
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(&eth, PEER_MAC, SYN_ETHTYPE_IPV4, large_payload, 1510,
                                              large_out, &out_len));

    /* ARP update existing entry (lines 87-89) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, 0x0A000001, PEER_MAC));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, 0x0A000001, PEER_MAC));

    /* Process ARP Reply packet (lines 241-242) */
    uint8_t arp_reply_pkt[60] = {0};
    memcpy(&arp_reply_pkt[0], MY_MAC, 6);
    memcpy(&arp_reply_pkt[6], PEER_MAC, 6);
    arp_reply_pkt[12] = 0x08;
    arp_reply_pkt[13] = 0x06; /* ARP */
    arp_reply_pkt[14] = 0x00;
    arp_reply_pkt[15] = 0x01; /* htype = 1 */
    arp_reply_pkt[16] = 0x08;
    arp_reply_pkt[17] = 0x00; /* ptype = 0x0800 */
    arp_reply_pkt[18] = 6;    /* hlen = 6 */
    arp_reply_pkt[19] = 4;    /* plen = 4 */
    arp_reply_pkt[20] = 0x00;
    arp_reply_pkt[21] = 0x02; /* ARP Reply */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_eth_process_frame(&eth, arp_reply_pkt, sizeof(arp_reply_pkt), tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(1, eth.arp_replies);

    /* Process UDP packet (lines 249-251) */
    SYN_UDP dummy_udp;
    syn_udp_init(&dummy_udp, &eth);
    syn_transport_udp_set_instance(&dummy_udp);
    uint8_t udp_eth_pkt[64] = {0};
    memcpy(&udp_eth_pkt[0], MY_MAC, 6);
    memcpy(&udp_eth_pkt[6], PEER_MAC, 6);
    udp_eth_pkt[12] = 0x08;
    udp_eth_pkt[13] = 0x00; /* IPv4 */
    udp_eth_pkt[14] = 0x45;
    udp_eth_pkt[23] = 17; /* UDP */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_eth_process_frame(&eth, udp_eth_pkt, sizeof(udp_eth_pkt), tx, &tx_len));
    syn_transport_udp_set_instance(NULL);

    /* Process IP packet with unknown protocol (ip_proto = 99) */
    udp_eth_pkt[23] = 99;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_eth_process_frame(&eth, udp_eth_pkt, sizeof(udp_eth_pkt), tx, &tx_len));

    /* ARP request for different target IP (lines 208) */
    uint8_t bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t arp_req_other[60] = {0};
    memcpy(&arp_req_other[0], bcast_mac, 6);
    memcpy(&arp_req_other[6], PEER_MAC, 6);
    arp_req_other[12] = 0x08;
    arp_req_other[13] = 0x06; /* ARP */
    arp_req_other[14] = 0x00;
    arp_req_other[15] = 0x01; /* htype = 1 */
    arp_req_other[16] = 0x08;
    arp_req_other[17] = 0x00; /* ptype = 0x0800 */
    arp_req_other[18] = 6;
    arp_req_other[19] = 4;
    arp_req_other[20] = 0x00;
    arp_req_other[21] = 0x01; /* ARP Request */
    arp_req_other[38] = 192;
    arp_req_other[39] = 168;
    arp_req_other[40] = 1;
    arp_req_other[41] = 99; /* Target IP = 192.168.1.99 != MY_IP */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_eth_process_frame(&eth, arp_req_other, sizeof(arp_req_other), tx, &tx_len));
    TEST_ASSERT_EQUAL_INT(0, tx_len);

    /* Odd-length IP checksum (lines 279-281) */
    uint8_t odd_data[5] = {0x12, 0x34, 0x56, 0x78, 0x90};
    TEST_ASSERT_NOT_EQUAL(0, syn_ip_checksum(odd_data, 5));
}

void test_eth_mac_filtering(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Construct frame destined for alien MAC 02:99:88:77:66:55 */
    uint8_t alien_frame[60] = {0x02, 0x99, 0x88, 0x77, 0x66, 0x55};
    memcpy(&alien_frame[6], PEER_MAC, 6);
    alien_frame[12] = 0x08;
    alien_frame[13] = 0x06;

    uint8_t tx[128];
    size_t tx_len = 999;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, alien_frame, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_INT(0, tx_len); /* Dropped by MAC filter, no reply generated */
}

/**
 * Regression: ARP cache full eviction must pick the LRU entry (lowest
 * last_seen_ms), NOT always slot 0.  Fill 8 entries with staggered
 * timestamps so that slot 3 is oldest, then insert a 9th entry and
 * verify that slot 3 is overwritten, not slot 0.
 */
void test_eth_arp_cache_lru_eviction(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Insert 8 entries. Slot 3 gets timestamp=1 (oldest). All others get >10. */
    for (uint32_t i = 0; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        mock_tick_ms = (i == 3) ? 1 : ((i + 2) * 10);
        uint32_t ip = 0x0A000001 + i;
        uint8_t mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, (uint8_t)(i + 1)};
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, ip, mac));
    }

    /* Record the IP that occupies slot 3 (will be the LRU victim) */
    uint32_t slot3_ip = eth.arp_cache[3].ip;
    /* Record slot 0's IP to verify it survives */
    uint32_t slot0_ip = eth.arp_cache[0].ip;

    /* Insert a 9th entry — should evict LRU (slot 3), not slot 0 */
    mock_tick_ms = 1000;
    uint32_t new_ip = 0x0B000001;
    uint8_t new_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0xFF};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, new_ip, new_mac));

    /* New entry must be present */
    uint8_t mac_out[6];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, new_ip, mac_out));
    TEST_ASSERT_EQUAL_INT(0, memcmp(new_mac, mac_out, 6));

    /* LRU victim (slot 3) must be gone */
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, syn_eth_arp_lookup(&eth, slot3_ip, mac_out));

    /* Slot 0 must still be present (it had a newer timestamp) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, slot0_ip, mac_out));
}

void test_eth_arp_cache_eviction_overflow(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Fill 10 entries with monotonically increasing timestamps */
    for (uint32_t i = 1; i <= 10; i++) {
        mock_tick_ms = i * 10;
        uint32_t test_ip = 0x0A000000 | i;
        uint8_t test_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, (uint8_t)i};
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth, test_ip, test_mac));
    }

    /* Most recent entries (9, 10) must survive */
    uint8_t mac_out[6];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, 0x0A000009, mac_out));
    TEST_ASSERT_EQUAL_UINT8(9, mac_out[5]);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth, 0x0A00000A, mac_out));
    TEST_ASSERT_EQUAL_UINT8(10, mac_out[5]);
}

/* ── ETH dispatcher routing ──────────────────────────────────────────────── */

/* ICMP injection hook for dispatcher routing test */
static SYN_ICMP g_test_icmp;
static bool g_icmp_injected = false;

struct SYN_ICMP *syn_eth_get_icmp_instance(void)
{
    return (struct SYN_ICMP *)(g_icmp_injected ? &g_test_icmp : NULL);
}

/** Regression: ETH dispatcher must forward ICMP (proto 1) to the ICMP engine. */
void test_eth_dispatch_icmp(void)
{
    static SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);
    syn_icmp_init(&g_test_icmp);
    g_icmp_injected = true;

    /* Minimal ICMP Echo Request frame (type=8, directed at our IP) */
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    memcpy(&frame[0], MY_MAC, 6);
    memcpy(&frame[6], PEER_MAC, 6);
    frame[12] = 0x08;
    frame[13] = 0x00; /* IPv4 */
    frame[14] = 0x45; /* IHL=5 */
    frame[17] = 28;   /* IP total length = 20 + 8 */
    frame[23] = 1;    /* Protocol = ICMP */
    frame[26] = (uint8_t)(PEER_IP >> 24);
    frame[27] = (uint8_t)(PEER_IP >> 16);
    frame[28] = (uint8_t)(PEER_IP >> 8);
    frame[29] = (uint8_t)PEER_IP;
    frame[30] = (uint8_t)(MY_IP >> 24);
    frame[31] = (uint8_t)(MY_IP >> 16);
    frame[32] = (uint8_t)(MY_IP >> 8);
    frame[33] = (uint8_t)MY_IP;
    frame[34] = 8; /* ICMP type echo request */
    frame[35] = 0; /* code */
    uint16_t icmp_csum = syn_icmp_checksum(&frame[34], 8);
    frame[36] = (uint8_t)(icmp_csum >> 8);
    frame[37] = (uint8_t)(icmp_csum & 0xFF);

    uint8_t tx[128];
    size_t tx_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, frame, sizeof(frame), tx, &tx_len));

    /* ICMP engine must have counted the echo request */
    TEST_ASSERT_EQUAL_UINT32(1, g_test_icmp.echo_requests_rx);

    g_icmp_injected = false;
}

/* TCP injection hook for dispatcher routing test */
static SYN_TCP g_test_tcp;
static bool g_tcp_injected = false;

struct SYN_TCP *syn_eth_get_tcp_instance(void)
{
    return (struct SYN_TCP *)(g_tcp_injected ? &g_test_tcp : NULL);
}

/** Regression: ETH dispatcher must forward TCP (proto 6) to the TCP engine. */
void test_eth_dispatch_tcp(void)
{
    static SYN_ETH eth;
    uint8_t mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint32_t ip = 0xA9FE0164;
    syn_eth_init(&eth, mac, ip);

    syn_tcp_init(&g_test_tcp, &eth);
    syn_tcp_listen(&g_test_tcp, 80);
    g_tcp_injected = true;

    /* Minimal TCP SYN frame */
    uint8_t frame[54];
    memset(frame, 0, sizeof(frame));
    memcpy(&frame[0], mac, 6);
    memcpy(&frame[6], PEER_MAC, 6);
    frame[12] = 0x08;
    frame[13] = 0x00;
    frame[14] = 0x45;
    frame[16] = 0x00;
    frame[17] = 40;
    frame[23] = 6; /* TCP */
    frame[26] = 0xC0;
    frame[27] = 0xA8;
    frame[28] = 0x01;
    frame[29] = 0x01;
    frame[30] = (uint8_t)(ip >> 24);
    frame[31] = (uint8_t)(ip >> 16);
    frame[32] = (uint8_t)(ip >> 8);
    frame[33] = (uint8_t)ip;
    frame[34] = 0xC0;
    frame[35] = 0x00; /* src port */
    frame[36] = 0x00;
    frame[37] = 80; /* dst port 80 */
    frame[38] = 0;
    frame[39] = 0;
    frame[40] = 0;
    frame[41] = 1;    /* seq */
    frame[46] = 0x50; /* data offset 5 */
    frame[47] = SYN_TCP_FLAG_SYN;

    uint8_t tx[128];
    size_t tx_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, frame, sizeof(frame), tx, &tx_len));

    /* TCP engine must have moved to SYN_RCVD and emitted SYN+ACK */
    TEST_ASSERT_EQUAL_INT(SYN_TCP_SYN_RCVD, g_test_tcp.conns[0].state);
    TEST_ASSERT_GREATER_THAN(0, tx_len);

    g_tcp_injected = false;
}

void test_eth_multiprotocol_interleaving(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t frame_arp[60] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(&frame_arp[6], PEER_MAC, 6);
    frame_arp[12] = 0x08;
    frame_arp[13] = 0x06;

    uint8_t frame_ecat[60] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    memcpy(&frame_ecat[6], PEER_MAC, 6);
    frame_ecat[12] = 0x88;
    frame_ecat[13] = 0xA4; /* EtherCAT */

    uint8_t tx[128];
    size_t tx_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, frame_arp, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, frame_ecat, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(2, eth.frames_rx);
}

void test_eth_arp_reply_and_oversized_payload(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Construct ARP Reply frame */
    uint8_t arp_reply_frame[60] = {0};
    memcpy(&arp_reply_frame[0], MY_MAC, 6);
    memcpy(&arp_reply_frame[6], PEER_MAC, 6);
    arp_reply_frame[12] = 0x08;
    arp_reply_frame[13] = 0x06; /* EtherType = ARP */
    arp_reply_frame[14] = 0x00;
    arp_reply_frame[15] = 0x01; /* HW = Ethernet */
    arp_reply_frame[16] = 0x08;
    arp_reply_frame[17] = 0x00; /* Proto = IPv4 */
    arp_reply_frame[20] = 0x00;
    arp_reply_frame[21] = 0x02; /* Oper = Reply */

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, arp_reply_frame, 60, NULL, NULL));
    TEST_ASSERT_EQUAL_UINT32(1, eth.arp_replies);

    /* Oversized payload build frame error */
    static uint8_t payload[1600];
    static uint8_t frame_out[1600];
    size_t frame_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_eth_build_frame(&eth, PEER_MAC, SYN_ETHTYPE_IPV4, payload,
                                              sizeof(payload), frame_out, &frame_len));
}

static void test_eth_weak_hooks_and_udp_dispatch(void)
{
    SYN_ETH eth;
    SYN_UDP udp;
    syn_eth_init(&eth, MY_MAC, MY_IP);
    syn_udp_init(&udp, &eth);
    syn_transport_udp_set_instance(&udp);

    /* Construct IPv4 UDP frame (proto=17) */
    uint8_t udp_frame[60] = {0};
    memcpy(&udp_frame[0], MY_MAC, 6);
    memcpy(&udp_frame[6], PEER_MAC, 6);
    udp_frame[12] = 0x08;
    udp_frame[13] = 0x00; /* IPv4 */
    udp_frame[14] = 0x45;
    udp_frame[23] = 17; /* UDP */

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, udp_frame, 60, NULL, NULL));
    syn_transport_udp_set_instance(NULL);

    /* Construct ARP Reply frame (oper=2) */
    uint8_t arp_reply[42] = {0};
    memcpy(&arp_reply[0], MY_MAC, 6);
    memcpy(&arp_reply[6], PEER_MAC, 6);
    arp_reply[12] = 0x08;
    arp_reply[13] = 0x06; /* ARP */
    arp_reply[14] = 0x00;
    arp_reply[15] = 0x01; /* htype = Ethernet */
    arp_reply[16] = 0x08;
    arp_reply[17] = 0x00; /* ptype = IPv4 */
    arp_reply[20] = 0x00;
    arp_reply[21] = 0x02; /* Reply */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, arp_reply, 42, NULL, NULL));
    TEST_ASSERT_EQUAL_UINT32(1, eth.arp_replies);

    /* Test ARP update existing entry */
    uint8_t mac_new[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x99};
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_arp_update(&eth, 0x0A000002, PEER_MAC));
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_arp_update(&eth, 0x0A000002, mac_new));
}

static void test_eth_generate_mac_null_checks(void)
{
    uint8_t mac[6];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_eth_generate_mac(NULL, 4, mac));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_eth_generate_mac("uid", 0, mac));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_eth_generate_mac("uid", 3, NULL));
}

static void test_eth_weak_instance_getters(void)
{
    TEST_ASSERT_NULL(syn_eth_get_icmp_instance());
    TEST_ASSERT_NULL(syn_eth_get_tcp_instance());
}

#include "syntropic/util/syn_pack.h"

static void test_eth_runt_arp_frame(void)
{
    SYN_ETH eth;
    uint8_t mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    syn_eth_init(&eth, mac, 0x0A000001);
    uint8_t runt[20] = {0};
    memcpy(runt, mac, 6);
    syn_poke_u16(SYN_ETHTYPE_ARP, runt, 12);
    TEST_ASSERT_EQUAL(SYN_OK, syn_eth_process_frame(&eth, runt, sizeof(runt), NULL, NULL));
    TEST_ASSERT_EQUAL_UINT32(0U, eth.arp_requests);
    TEST_ASSERT_EQUAL_UINT32(0U, eth.arp_replies);
}

void test_eth_header_packing_helpers(void)
{
    uint8_t buf[64] = {0};
    uint8_t dst_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t src_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    /* Null / bounds parameter checks */
    TEST_ASSERT_EQUAL_INT(0, syn_ip_checksum(NULL, 0));
    TEST_ASSERT_EQUAL_INT(0, syn_ip_checksum(buf, 0));
    TEST_ASSERT_EQUAL_INT(0, syn_eth_pack_header(NULL, 14, dst_mac, src_mac, SYN_ETHTYPE_IPV4));
    TEST_ASSERT_EQUAL_INT(0, syn_eth_pack_header(buf, 10, dst_mac, src_mac, SYN_ETHTYPE_IPV4));
    TEST_ASSERT_EQUAL_INT(0, syn_eth_pack_header(buf, 14, NULL, src_mac, SYN_ETHTYPE_IPV4));
    TEST_ASSERT_EQUAL_INT(0, syn_eth_pack_header(buf, 14, dst_mac, NULL, SYN_ETHTYPE_IPV4));
    TEST_ASSERT_EQUAL_INT(0, syn_ip_pack_header(NULL, 20, 0xC0A80101, 0xC0A80102, 17, 8, 1));
    TEST_ASSERT_EQUAL_INT(0, syn_ip_pack_header(buf, 15, 0xC0A80101, 0xC0A80102, 17, 8, 1));

    /* Valid Ethernet II header pack */
    TEST_ASSERT_EQUAL_INT(
        14, syn_eth_pack_header(buf, sizeof(buf), dst_mac, src_mac, SYN_ETHTYPE_IPV4));
    TEST_ASSERT_EQUAL_MEMORY(dst_mac, &buf[0], 6);
    TEST_ASSERT_EQUAL_MEMORY(src_mac, &buf[6], 6);
    TEST_ASSERT_EQUAL_UINT8(0x08, buf[12]);
    TEST_ASSERT_EQUAL_UINT8(0x00, buf[13]);

    /* Valid IPv4 header pack */
    size_t ip_len =
        syn_ip_pack_header(&buf[14], sizeof(buf) - 14, 0xC0A80101, 0xC0A80102, 17, 8, 0x1234);
    TEST_ASSERT_EQUAL_INT(20, ip_len);
    TEST_ASSERT_EQUAL_UINT8(0x45, buf[14]);
    TEST_ASSERT_EQUAL_UINT8(17, buf[23]); /* UDP */
    TEST_ASSERT_EQUAL_UINT8(192, buf[26]);
    TEST_ASSERT_EQUAL_UINT8(168, buf[27]);
}

static void test_eth_frame_filtering_and_protocol_dispatch(void)
{
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* 1. arp_update with ip == 0 */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_eth_arp_update(&eth, 0, PEER_MAC));

    /* 2. build_frame with NULL payload */
    uint8_t frame_buf[64];
    size_t frame_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_build_frame(&eth, PEER_MAC, SYN_ETHTYPE_IPV4, NULL, 0,
                                                      frame_buf, &frame_len));
    TEST_ASSERT_EQUAL_INT(60, frame_len);

    /* 3. process ARP request for different IP (not MY_IP) */
    uint8_t arp_req[60] = {0};
    memset(&arp_req[0], 0xFF, 6); /* Broadcast */
    memcpy(&arp_req[6], PEER_MAC, 6);
    arp_req[12] = 0x08;
    arp_req[13] = 0x06; /* ARP */
    arp_req[14] = 0x00;
    arp_req[15] = 0x01; /* Ethernet */
    arp_req[16] = 0x08;
    arp_req[17] = 0x00; /* IPv4 */
    arp_req[20] = 0x00;
    arp_req[21] = 0x01; /* Request */
    arp_req[38] = 10;
    arp_req[39] = 0;
    arp_req[40] = 0;
    arp_req[41] = 99; /* Different IP */
    uint8_t tx[128];
    size_t tx_len = 99;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, arp_req, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_INT(0, tx_len);

    /* 4. ARP with invalid htype (e.g. 2) */
    arp_req[15] = 0x02;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, arp_req, 60, tx, &tx_len));

    /* 5. IPv4 frame with unknown protocol (e.g. 99) */
    uint8_t ip_unk[60] = {0};
    memcpy(&ip_unk[0], MY_MAC, 6);
    memcpy(&ip_unk[6], PEER_MAC, 6);
    ip_unk[12] = 0x08;
    ip_unk[13] = 0x00; /* IPv4 */
    ip_unk[14] = 0x45;
    ip_unk[23] = 99; /* Unknown proto */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, ip_unk, 60, tx, &tx_len));

    /* 6. Odd length IP checksum */
    uint8_t odd_data[21] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5,
                            0xF4, 0xF3, 0xF2, 0xF1, 0xF0, 0xEF, 0xEE, 0xED, 0xEC, 0xEB};
    uint16_t csum_odd = syn_ip_checksum(odd_data, 21);
    TEST_ASSERT_NOT_EQUAL(0, csum_odd);

    /* 7. Short IPv4 frame (< 34 bytes) */
    uint8_t short_ip[30] = {0};
    memcpy(&short_ip[0], MY_MAC, 6);
    memcpy(&short_ip[6], PEER_MAC, 6);
    short_ip[12] = 0x08;
    short_ip[13] = 0x00; /* IPv4 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, short_ip, 30, tx, &tx_len));

    /* 8. ICMP and TCP dispatch with NULL tx_buf / tx_len */
    uint8_t icmp_frame[60] = {0};
    memcpy(&icmp_frame[0], MY_MAC, 6);
    memcpy(&icmp_frame[6], PEER_MAC, 6);
    icmp_frame[12] = 0x08;
    icmp_frame[13] = 0x00;
    icmp_frame[14] = 0x45;
    icmp_frame[23] = 1; /* ICMP */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, icmp_frame, 60, NULL, NULL));

    uint8_t tcp_frame[60] = {0};
    memcpy(&tcp_frame[0], MY_MAC, 6);
    memcpy(&tcp_frame[6], PEER_MAC, 6);
    tcp_frame[12] = 0x08;
    tcp_frame[13] = 0x00;
    tcp_frame[14] = 0x45;
    tcp_frame[23] = 6; /* TCP */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, tcp_frame, 60, NULL, NULL));

    /* 9. ARP with invalid ptype (e.g. 0x0000) */
    arp_req[15] = 0x01; /* htype = 1 */
    arp_req[16] = 0x00;
    arp_req[17] = 0x00; /* ptype = 0 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, arp_req, 60, tx, &tx_len));

    /* 10. Fill ARP cache and then call syn_eth_arp_update for existing entry */
    SYN_ETH eth_full;
    syn_eth_init(&eth_full, MY_MAC, MY_IP);
    uint8_t dummy_mac[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
    for (size_t i = 0; i < SYN_ETH_ARP_CACHE_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth_full, 0xC0A80101 + i, dummy_mac));
    }
    /* Update existing entry in full cache */
    dummy_mac[5] = 0x99;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth_full, 0xC0A80101, dummy_mac));
    uint8_t lookup_mac[6];
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_lookup(&eth_full, 0xC0A80101, lookup_mac));
    TEST_ASSERT_EQUAL_INT(0, memcmp(dummy_mac, lookup_mac, 6));

    /* 11. ARP cache LRU eviction when full with new IP */
    uint8_t new_mac[6] = {0x02, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_arp_update(&eth_full, 0xC0A80199, new_mac));

    /* 12. Odd-length buffer checksum calculation */
    uint8_t odd_buf[21] = {0x45, 0x00, 0x00, 0x15, 0x00, 0x01, 0x00, 0x00, 0x40, 0x11, 0x00,
                           0x00, 0xC0, 0xA8, 0x01, 0x01, 0xC0, 0xA8, 0x01, 0x02, 0xAA};
    TEST_ASSERT_NOT_EQUAL(0, syn_ip_checksum(odd_buf, sizeof(odd_buf)));

    /* 13. Frame addressed to different MAC (filtered out) */
    uint8_t wrong_mac_frame[60] = {0};
    uint8_t other_mac[6] = {0x02, 0xFE, 0xDC, 0xBA, 0x98, 0x76};
    memcpy(&wrong_mac_frame[0], other_mac, 6);
    memcpy(&wrong_mac_frame[6], PEER_MAC, 6);
    wrong_mac_frame[12] = 0x08;
    wrong_mac_frame[13] = 0x00;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, wrong_mac_frame, 60, tx, &tx_len));
}

static void test_r5_n07_eth_mcast_igmp_and_ihl(void)
{
    SYN_ETH eth;
    SYN_IGMP igmp;
    uint8_t join_tx[60];
    size_t join_len = 0;
    uint8_t tx[128];
    size_t tx_len = 0;

    syn_eth_init(&eth, MY_MAC, MY_IP);
    syn_igmp_init(&igmp);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_join_group(&igmp, &eth, 0xE00000FBUL, join_tx, &join_len));

    /* 1. Default IGMP hook is NULL -> multicast IGMP query accepted by MAC filter, ignored */
    syn_eth_set_igmp_instance(NULL);
    TEST_ASSERT_NULL(syn_eth_get_igmp_instance());

    uint8_t mcast_query[60] = {0};
    /* Multicast MAC 01:00:5E:00:00:01 (224.0.0.1 All-Hosts) */
    mcast_query[0] = 0x01;
    mcast_query[1] = 0x00;
    mcast_query[2] = 0x5E;
    mcast_query[3] = 0x00;
    mcast_query[4] = 0x00;
    mcast_query[5] = 0x01;
    memcpy(&mcast_query[6], PEER_MAC, 6);
    mcast_query[12] = 0x08;
    mcast_query[13] = 0x00;
    mcast_query[14] = 0x46; /* IPv4 IHL=6 (24B with Router Alert option) */
    mcast_query[23] = 2;    /* Protocol = IGMP */
    mcast_query[34] = 0x94; /* Router Alert option */
    mcast_query[35] = 0x04;
    mcast_query[38] = SYN_IGMP_TYPE_MEMBERSHIP_QUERY;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, mcast_query, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(0U, igmp.queries_received);
    TEST_ASSERT_EQUAL_size_t(0U, tx_len);

    /* 2. Inject IGMP instance -> Router Alert IGMP query dispatched & emits IGMPv2 Report */
    syn_eth_set_igmp_instance(&igmp);
    TEST_ASSERT_EQUAL_PTR(&igmp, syn_eth_get_igmp_instance());
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, mcast_query, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(1U, igmp.queries_received);
    TEST_ASSERT_EQUAL_size_t(60U, tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_IGMP_TYPE_V2_REPORT, tx[34]);

    /* 3. Non-IPv4 multicast MACs (01:00:5E:80:00:01, 01:00:5F:00:00:01, 01:80:C2:00:00:00)
     *    must be rejected by eth_accept_dst_mac */
    uint8_t bad_mcast[60];
    memcpy(bad_mcast, mcast_query, sizeof(bad_mcast));
    bad_mcast[3] = 0x80; /* High bit of byte 3 set -> invalid IPv4 multicast MAC */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, bad_mcast, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(1U, igmp.queries_received);

    bad_mcast[3] = 0x00;
    bad_mcast[2] = 0x5F;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, bad_mcast, 60, tx, &tx_len));
    bad_mcast[1] = 0x80;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, bad_mcast, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(1U, igmp.queries_received);

    /* 4. IPv4 frames with IHL < 20 or 14 + IHL > len ignored safely */
    uint8_t bad_ihl[60];
    memcpy(bad_ihl, mcast_query, sizeof(bad_ihl));
    bad_ihl[14] = 0x44; /* IHL = 16 < 20 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, bad_ihl, 60, tx, &tx_len));
    bad_ihl[14] = 0x4F; /* IHL = 60 -> 14 + 60 = 74 > 60 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_eth_process_frame(&eth, bad_ihl, 60, tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT32(1U, igmp.queries_received);

    syn_eth_set_igmp_instance(NULL);
}

void run_eth_tests(void)
{
    RUN_TEST(test_eth_generate_mac);
    RUN_TEST(test_eth_init);
    RUN_TEST(test_eth_arp_cache);
    RUN_TEST(test_eth_arp_lookup_returns_not_found);
    RUN_TEST(test_eth_build_frame);
    RUN_TEST(test_eth_process_arp_request);
    RUN_TEST(test_eth_coroutine_pt);
    RUN_TEST(test_eth_null_checks);
    RUN_TEST(test_eth_runt_and_oversized_frames);
    RUN_TEST(test_eth_mac_filtering);
    RUN_TEST(test_eth_arp_cache_lru_eviction);
    RUN_TEST(test_eth_arp_cache_eviction_overflow);
    RUN_TEST(test_eth_dispatch_icmp);
    RUN_TEST(test_eth_dispatch_tcp);
    RUN_TEST(test_eth_multiprotocol_interleaving);
    RUN_TEST(test_eth_arp_reply_and_oversized_payload);
    RUN_TEST(test_eth_weak_hooks_and_udp_dispatch);
    RUN_TEST(test_eth_generate_mac_null_checks);
    RUN_TEST(test_eth_weak_instance_getters);
    RUN_TEST(test_eth_runt_arp_frame);
    RUN_TEST(test_eth_header_packing_helpers);
    RUN_TEST(test_eth_frame_filtering_and_protocol_dispatch);
    RUN_TEST(test_r5_n07_eth_mcast_igmp_and_ihl);
}
