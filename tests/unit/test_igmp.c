/**
 * @file test_igmp.c
 * @brief Unit tests for Zero-Heap Native IGMPv2 Protocol Engine.
 */

#include "syntropic/net/syn_igmp.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint32_t MY_IP = 0xC0A80164;      /* 192.168.1.100 */
static const uint32_t MDNS_GROUP = 0xE00000FB; /* 224.0.0.251 */

void test_igmp_init(void)
{
    SYN_IGMP igmp;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_init(&igmp));
    TEST_ASSERT_EQUAL_UINT32(0, igmp.reports_sent);
    TEST_ASSERT_EQUAL_UINT32(0, igmp.queries_received);
}

void test_igmp_join_and_leave(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);

    uint8_t join_frame[60];
    size_t join_len = 0;

    /* Join mDNS multicast group 224.0.0.251 */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_join_group(&igmp, &eth, MDNS_GROUP, join_frame, &join_len));
    TEST_ASSERT_EQUAL_INT(60, join_len);
    TEST_ASSERT_EQUAL_UINT32(1, igmp.reports_sent);
    TEST_ASSERT_TRUE(igmp.groups[0].joined);
    TEST_ASSERT_EQUAL_UINT32(MDNS_GROUP, igmp.groups[0].group_ip);

    /* Verify Destination Multicast MAC: 01:00:5E:00:00:FB */
    TEST_ASSERT_EQUAL_UINT8(0x01, join_frame[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, join_frame[1]);
    TEST_ASSERT_EQUAL_UINT8(0x5E, join_frame[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00, join_frame[3]);
    TEST_ASSERT_EQUAL_UINT8(0x00, join_frame[4]);
    TEST_ASSERT_EQUAL_UINT8(0xFB, join_frame[5]);

    /* Verify IGMP Type == 0x16 (v2 Report) */
    TEST_ASSERT_EQUAL_UINT8(SYN_IGMP_TYPE_V2_REPORT, join_frame[34]);

    /* Leave Group */
    uint8_t leave_frame[60];
    size_t leave_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_leave_group(&igmp, &eth, MDNS_GROUP, leave_frame, &leave_len));
    TEST_ASSERT_EQUAL_INT(60, leave_len);
    TEST_ASSERT_EQUAL_UINT32(1, igmp.leaves_sent);
    TEST_ASSERT_FALSE(igmp.groups[0].joined);
    TEST_ASSERT_EQUAL_UINT8(SYN_IGMP_TYPE_V2_LEAVE, leave_frame[34]);
}

void test_igmp_process_query(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);

    uint8_t dummy_frame[60];
    size_t dummy_len = 0;
    syn_igmp_join_group(&igmp, &eth, MDNS_GROUP, dummy_frame, &dummy_len);

    /* Construct incoming IGMP Membership Query (Type 0x11) frame */
    uint8_t query_pkt[60] = {0};
    query_pkt[12] = 0x08;
    query_pkt[13] = 0x00; /* IPv4 */
    query_pkt[14] = 0x45; /* IHL=5 (20B) */
    query_pkt[23] = 2;    /* Protocol = IGMP */
    query_pkt[34] = SYN_IGMP_TYPE_MEMBERSHIP_QUERY;

    uint8_t reply_frame[60];
    size_t reply_len = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_igmp_process_packet(&igmp, &eth, query_pkt, 60, reply_frame, &reply_len));
    TEST_ASSERT_EQUAL_UINT32(1, igmp.queries_received);
    TEST_ASSERT_EQUAL_INT(60, reply_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_IGMP_TYPE_V2_REPORT, reply_frame[34]);
}

void test_igmp_null_checks(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_igmp_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_igmp_join_group(NULL, NULL, 0, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_igmp_leave_group(NULL, NULL, 0, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_igmp_process_packet(NULL, NULL, NULL, 0, NULL, NULL));
}

void test_igmp_group_overflow_and_leaving_unjoined(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);

    uint8_t frame[60];
    size_t len = 0;

    /* Fill all 4 group slots */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, 0xE0000001, frame, &len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, 0xE0000002, frame, &len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, 0xE0000003, frame, &len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, 0xE0000004, frame, &len));

    /* Join 5th group (causes replacement of slot 0) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, 0xE0000005, frame, &len));
    TEST_ASSERT_EQUAL_UINT32(0xE0000005, igmp.groups[0].group_ip);

    /* Leave a group that was never joined */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_leave_group(&igmp, &eth, 0xE0000099, frame, &len));
}

void test_igmp_non_igmp_packets(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);

    uint8_t short_pkt[30] = {0};
    size_t tx_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_igmp_process_packet(&igmp, &eth, short_pkt, 30, NULL, &tx_len));

    /* ARP packet (ethertype != 0x0800) */
    uint8_t arp_pkt[60] = {0};
    arp_pkt[12] = 0x08;
    arp_pkt[13] = 0x06;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_process_packet(&igmp, &eth, arp_pkt, 60, NULL, &tx_len));

    /* UDP packet (proto = 17 != 2) */
    uint8_t udp_pkt[60] = {0};
    udp_pkt[12] = 0x08;
    udp_pkt[13] = 0x00;
    udp_pkt[14] = 0x45;
    udp_pkt[23] = 17;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_process_packet(&igmp, &eth, udp_pkt, 60, NULL, &tx_len));

    /* Join group with group_ip = 0 -> SYN_INVALID_PARAM */
    uint8_t frame[60];
    size_t len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_igmp_join_group(&igmp, &eth, 0, frame, &len));

    /* Re-join an existing group */
    SYN_IGMP rejoin_igmp;
    syn_igmp_init(&rejoin_igmp);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&rejoin_igmp, &eth, MDNS_GROUP, frame, &len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&rejoin_igmp, &eth, MDNS_GROUP, frame, &len));

    /* Process IGMP query with NULL frame_tx / tx_len */
    uint8_t query_pkt_null[60] = {0};
    query_pkt_null[12] = 0x08;
    query_pkt_null[13] = 0x00;
    query_pkt_null[14] = 0x45;
    query_pkt_null[23] = 2;
    query_pkt_null[34] = SYN_IGMP_TYPE_MEMBERSHIP_QUERY;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_igmp_process_packet(&rejoin_igmp, &eth, query_pkt_null, 60, NULL, NULL));

    /* Process IGMP V2 Report type packet (non-query) */
    uint8_t report_pkt[60] = {0};
    report_pkt[12] = 0x08;
    report_pkt[13] = 0x00;
    report_pkt[14] = 0x45;
    report_pkt[23] = 2;
    report_pkt[34] = SYN_IGMP_TYPE_V2_REPORT;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_process_packet(&rejoin_igmp, &eth, report_pkt, 60, NULL, NULL));

    /* Build report with custom type (neither V2_REPORT nor V2_LEAVE) */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_build_report(&rejoin_igmp, &eth, 99, MDNS_GROUP, frame, &len));
}

void test_r5_igmp_regressions(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    uint8_t frame[60];
    size_t len = 0;

    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, MDNS_GROUP, frame, &len));

    /* 1. Reject IHL < 20 and IHL + 8 exceeding packet length */
    uint8_t bad_ihl[60] = {0};
    bad_ihl[12] = 0x08;
    bad_ihl[13] = 0x00;
    bad_ihl[14] = 0x44; /* IHL = 16 < 20 */
    bad_ihl[23] = 2;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_igmp_process_packet(&igmp, &eth, bad_ihl, 60, frame, &len));

    bad_ihl[14] = 0x4F; /* IHL = 60 -> 14 + 60 + 8 = 82 > 60 */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_igmp_process_packet(&igmp, &eth, bad_ihl, 60, frame, &len));

    /* 2. IGMPv2 Membership Query with RFC 2113 Router Alert option (IHL = 24, IGMP at offset 38) */
    uint8_t ra_query[60] = {0};
    ra_query[12] = 0x08;
    ra_query[13] = 0x00;
    ra_query[14] = 0x46; /* Version 4, IHL = 6 (24 bytes) */
    ra_query[23] = 2;    /* Protocol = IGMP */
    ra_query[34] = 0x94; /* Router Alert option (94 04 00 00) */
    ra_query[35] = 0x04;
    ra_query[38] = SYN_IGMP_TYPE_MEMBERSHIP_QUERY;

    uint8_t reply[60] = {0};
    size_t reply_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_process_packet(&igmp, &eth, ra_query, 60, reply, &reply_len));
    TEST_ASSERT_EQUAL_UINT32(1U, igmp.queries_received);
    TEST_ASSERT_EQUAL_size_t(60U, reply_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_IGMP_TYPE_V2_REPORT, reply[34]);
}

void test_tier_a_igmp_regressions(void)
{
    SYN_IGMP igmp;
    SYN_ETH eth;
    uint8_t frame[60];
    size_t len = 0U;
    const uint32_t second_group = 0xE0000101UL; /* 224.0.1.1 */

    syn_igmp_init(&igmp);
    syn_eth_init(&eth, MAC, MY_IP);

    /* NET-01: IPv4 header checksum valid after TTL=1 override */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, MDNS_GROUP, frame, &len));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_ip_checksum(&frame[14], 20U));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_ip_checksum(&frame[34], 8U));

    /* Join second group in slot 1, then leave slot 0 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, second_group, frame, &len));

    uint8_t leave_frame[60];
    size_t leave_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_leave_group(&igmp, &eth, MDNS_GROUP, leave_frame, &leave_len));

    /* NET-02a: Leave sent to 224.0.0.2 (01:00:5E:00:00:02) with group_ip in IGMP payload */
    TEST_ASSERT_EQUAL_UINT16(0U, syn_ip_checksum(&leave_frame[14], 20U));
    TEST_ASSERT_EQUAL_UINT8(0x01U, leave_frame[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, leave_frame[1]);
    TEST_ASSERT_EQUAL_UINT8(0x5EU, leave_frame[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, leave_frame[3]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, leave_frame[4]);
    TEST_ASSERT_EQUAL_UINT8(0x02U, leave_frame[5]);
    TEST_ASSERT_EQUAL_UINT8(224U, leave_frame[30]);
    TEST_ASSERT_EQUAL_UINT8(0U, leave_frame[31]);
    TEST_ASSERT_EQUAL_UINT8(0U, leave_frame[32]);
    TEST_ASSERT_EQUAL_UINT8(2U, leave_frame[33]);
    TEST_ASSERT_EQUAL_UINT8(0xFBU, leave_frame[41]);

    /* NET-02a: Re-joining second_group must not duplicate into vacant slot 0 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_igmp_join_group(&igmp, &eth, second_group, frame, &len));
    TEST_ASSERT_FALSE(igmp.groups[0].joined);
    TEST_ASSERT_TRUE(igmp.groups[1].joined);
    TEST_ASSERT_EQUAL_UINT32(second_group, igmp.groups[1].group_ip);

    /* NET-02a: Group-Specific Query for unjoined group produces no report */
    uint8_t gsq[60] = {0};
    gsq[12] = 0x08U;
    gsq[13] = 0x00U;
    gsq[14] = 0x45U;
    gsq[23] = 2U;
    gsq[34] = SYN_IGMP_TYPE_MEMBERSHIP_QUERY;
    gsq[38] = 224U;
    gsq[39] = 0U;
    gsq[40] = 0U;
    gsq[41] = 0x99U;

    uint8_t reply[60] = {0};
    size_t reply_len = 99U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_process_packet(&igmp, &eth, gsq, 60U, reply, &reply_len));
    TEST_ASSERT_EQUAL_size_t(0U, reply_len);

    /* Group-Specific Query for joined second_group triggers report for second_group */
    gsq[40] = 1U;
    gsq[41] = 1U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_igmp_process_packet(&igmp, &eth, gsq, 60U, reply, &reply_len));
    TEST_ASSERT_EQUAL_size_t(60U, reply_len);
    TEST_ASSERT_EQUAL_UINT8(1U, reply[40]);
    TEST_ASSERT_EQUAL_UINT8(1U, reply[41]);
}
