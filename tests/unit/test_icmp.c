/**
 * @file test_icmp.c
 * @brief Unit tests for Zero-Heap Native ICMP Protocol Engine.
 */

#include "syntropic/net/syn_icmp.h"
#include "unity/unity.h"

#include <string.h>

static const uint8_t MY_MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t PEER_MAC[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
static const uint32_t MY_IP = 0xC0A80164;   /* 192.168.1.100 */
static const uint32_t PEER_IP = 0xC0A801C8; /* 192.168.1.200 */

void test_icmp_init(void)
{
    SYN_ICMP icmp;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_icmp_init(&icmp));
    TEST_ASSERT_EQUAL_UINT32(0, icmp.echo_requests_rx);
    TEST_ASSERT_EQUAL_UINT32(0, icmp.echo_replies_tx);
}

void test_icmp_checksum(void)
{
    uint8_t data[8] = {0x08, 0x00, 0x00, 0x00, 0x12, 0x34, 0x00, 0x01};
    uint16_t csum = syn_icmp_checksum(data, 8);
    TEST_ASSERT_NOT_EQUAL(0, csum);

    /* Verify checksum validation over whole block yields 0 */
    data[2] = (uint8_t)(csum >> 8);
    data[3] = (uint8_t)(csum & 0xFF);
    TEST_ASSERT_EQUAL_UINT16(0, syn_icmp_checksum(data, 8));
}

void test_icmp_build_echo_request(void)
{
    SYN_ICMP icmp;
    SYN_ETH eth;
    syn_icmp_init(&icmp);
    syn_eth_init(&eth, MY_MAC, MY_IP);

    uint8_t payload[8] = {'P', 'I', 'N', 'G', 'T', 'E', 'S', 'T'};
    uint8_t frame[128];
    size_t frame_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 0x1234,
                                                      0x0001, payload, 8, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(60, frame_len); /* 14 + 20 + 8 + 8 = 50 padded to 60 */
    TEST_ASSERT_EQUAL_UINT32(1, icmp.echo_requests_tx);

    /* Verify Ethernet Header */
    TEST_ASSERT_EQUAL_INT(0, memcmp(PEER_MAC, &frame[0], 6));
    TEST_ASSERT_EQUAL_INT(0, memcmp(MY_MAC, &frame[6], 6));
    TEST_ASSERT_EQUAL_UINT8(0x08, frame[12]);
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[13]);

    /* Verify IP Header */
    TEST_ASSERT_EQUAL_UINT8(0x45, frame[14]);
    TEST_ASSERT_EQUAL_UINT8(1, frame[23]); /* Protocol 1 = ICMP */

    /* Verify ICMP Header */
    TEST_ASSERT_EQUAL_UINT8(SYN_ICMP_TYPE_ECHO_REQUEST, frame[34]);
    TEST_ASSERT_EQUAL_UINT8(0x12, frame[38]);
    TEST_ASSERT_EQUAL_UINT8(0x34, frame[39]);
}

void test_icmp_process_echo_request(void)
{
    SYN_ICMP icmp;
    SYN_ETH eth;
    syn_icmp_init(&icmp);
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* Build an inbound Echo Request frame */
    uint8_t in_req[60];
    size_t in_len = 0;
    syn_icmp_build_echo_request(&icmp, &eth, MY_IP, MY_MAC, 0x5555, 0x0002,
                                (const uint8_t *)"HELLO", 5, in_req, &in_len);

    /* Process inbound request */
    uint8_t out_reply[128];
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_icmp_process_packet(&icmp, in_req, in_len, out_reply, &out_len));
    TEST_ASSERT_EQUAL_INT(60, out_len);
    TEST_ASSERT_EQUAL_UINT32(1, icmp.echo_requests_rx);
    TEST_ASSERT_EQUAL_UINT32(1, icmp.echo_replies_tx);

    /* Verify reply ICMP Type is 0 (Echo Reply) */
    TEST_ASSERT_EQUAL_UINT8(SYN_ICMP_TYPE_ECHO_REPLY, out_reply[34]);
}

void test_icmp_null_checks(void)
{
    SYN_ICMP icmp;
    SYN_ETH eth;
    uint8_t frame[128];
    size_t frame_len = 0;
    uint8_t payload[8] = {0};

    syn_icmp_init(&icmp);
    syn_eth_init(&eth, MY_MAC, MY_IP);

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_icmp_init(NULL));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_process_packet(NULL, frame, 60, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_process_packet(&icmp, NULL, 60, frame, &frame_len));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(NULL, &eth, PEER_IP, PEER_MAC, 1, 1, payload,
                                                      8, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, NULL, PEER_IP, PEER_MAC, 1, 1, payload,
                                                      8, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, NULL, 1, 1, payload, 8,
                                                      frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1, payload,
                                                      8, NULL, &frame_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1, payload, 8, frame, NULL));
}

void test_icmp_process_packet_invalid_headers(void)
{
    SYN_ICMP icmp;
    syn_icmp_init(&icmp);

    /* 1. EtherType not IPv4 (e.g. 0x0806 ARP) */
    uint8_t arp_pkt[60] = {0};
    arp_pkt[12] = 0x08;
    arp_pkt[13] = 0x06;
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, arp_pkt, sizeof(arp_pkt), NULL, NULL));

    /* 2. Truncated IP header (ip_hl < 20) */
    uint8_t trunc_ip[60] = {0};
    trunc_ip[12] = 0x08;
    trunc_ip[13] = 0x00;
    trunc_ip[14] = 0x43; /* ihl = 3 -> 12 bytes < 20 */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_icmp_process_packet(&icmp, trunc_ip, sizeof(trunc_ip), NULL, NULL));

    /* 3. Non-ICMP protocol (proto = 17 UDP) */
    uint8_t udp_pkt[60] = {0};
    udp_pkt[12] = 0x08;
    udp_pkt[13] = 0x00;
    udp_pkt[14] = 0x45; /* ihl = 5 */
    udp_pkt[23] = 17;   /* UDP */
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, udp_pkt, sizeof(udp_pkt), NULL, NULL));

    /* 4. Truncated ICMP header (packet length shorter than 14 + 20 + 4, e.g. 35 bytes) */
    uint8_t short_icmp[35] = {0};
    short_icmp[12] = 0x08;
    short_icmp[13] = 0x00;
    short_icmp[14] = 0x45;
    short_icmp[23] = 1; /* ICMP */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_icmp_process_packet(&icmp, short_icmp, sizeof(short_icmp), NULL, NULL));

    /* 5. Inbound Echo Reply frame (type = 0) */
    uint8_t echo_reply_frame[60] = {0};
    echo_reply_frame[12] = 0x08;
    echo_reply_frame[13] = 0x00;
    echo_reply_frame[14] = 0x45;
    echo_reply_frame[17] = 28; /* ip_total_len = 20 + 8 */
    echo_reply_frame[23] = 1;  /* ICMP */
    echo_reply_frame[34] = SYN_ICMP_TYPE_ECHO_REPLY;
    uint16_t rep_csum = syn_icmp_checksum(&echo_reply_frame[34], 8);
    echo_reply_frame[36] = (uint8_t)(rep_csum >> 8);
    echo_reply_frame[37] = (uint8_t)(rep_csum & 0xFF);
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, echo_reply_frame, 60, NULL, NULL));
    TEST_ASSERT_EQUAL_UINT32(1, icmp.echo_replies_rx);

    /* 6. Build echo request with oversized payload > 1500 bytes */
    SYN_ETH eth;
    syn_eth_init(&eth, MY_MAC, MY_IP);
    static uint8_t huge_payload[1600] = {0};
    static uint8_t huge_frame[1600];
    size_t huge_len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1,
                                                  huge_payload, 1500, huge_frame, &huge_len));

    /* 7. Short Echo Request frame (< 60 bytes) to test reply padding to 60 bytes */
    uint8_t short_req[42] = {0};
    short_req[0] = 0x02;
    short_req[5] = 0x01;
    short_req[6] = 0x02;
    short_req[11] = 0x02;
    short_req[12] = 0x08;
    short_req[13] = 0x00;
    short_req[14] = 0x45;
    short_req[17] = 28; /* ip_total_len = 20 + 8 */
    short_req[23] = 1;  /* ICMP */
    short_req[34] = SYN_ICMP_TYPE_ECHO_REQUEST;
    uint16_t req_csum = syn_icmp_checksum(&short_req[34], 8);
    short_req[36] = (uint8_t)(req_csum >> 8);
    short_req[37] = (uint8_t)(req_csum & 0xFF);
    uint8_t short_reply[128];
    size_t short_reply_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_icmp_process_packet(&icmp, short_req, 42, short_reply, &short_reply_len));
    TEST_ASSERT_EQUAL(60, short_reply_len);

    /* 8. Oversized Echo Request frame (> 1514 bytes IP total len) rejected */
    uint8_t over_req[1600] = {0};
    memcpy(over_req, short_req, 42);
    over_req[16] = (uint8_t)(1502U >> 8);
    over_req[17] = (uint8_t)(1502U & 0xFFU); /* 14 + 1502 = 1516 > 1514 */
    uint8_t over_reply[1600];
    size_t over_reply_len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_icmp_process_packet(&icmp, over_req, 1520, over_reply, &over_reply_len));
    TEST_ASSERT_EQUAL(0, over_reply_len);

    /* 9. ip_hl header length larger than packet length */
    uint8_t invalid_ip_hl[60] = {0};
    invalid_ip_hl[12] = 0x08;
    invalid_ip_hl[13] = 0x00;
    invalid_ip_hl[14] = 0x4F; /* ihl = 15 -> 60 bytes, 14 + 60 = 74 > 60 */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_icmp_process_packet(&icmp, invalid_ip_hl, 60, NULL, NULL));

    /* 10. Echo Request with NULL frame_tx or NULL tx_len pointers */
    uint8_t null_tx_req[60] = {0};
    memcpy(null_tx_req, short_req, 42);
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, null_tx_req, 60, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, null_tx_req, 60, short_reply, NULL));

    /* 11. Unsupported ICMP type (e.g. Type 3 Destination Unreachable) */
    uint8_t unk_type[60] = {0};
    memcpy(unk_type, short_req, 42);
    unk_type[34] = 3; /* Destination Unreachable */
    unk_type[36] = 0;
    unk_type[37] = 0;
    uint16_t unk_csum = syn_icmp_checksum(&unk_type[34], 8);
    unk_type[36] = (uint8_t)(unk_csum >> 8);
    unk_type[37] = (uint8_t)(unk_csum & 0xFF);
    TEST_ASSERT_EQUAL(SYN_OK, syn_icmp_process_packet(&icmp, unk_type, 60, NULL, NULL));
}

void test_r5_icmp_regressions(void)
{
    SYN_ICMP icmp;
    SYN_ETH eth;
    uint8_t frame[128];
    size_t frame_len = 0;
    uint8_t dummy_payload[32];

    memset(dummy_payload, 0xAB, sizeof(dummy_payload));
    syn_icmp_init(&icmp);
    syn_eth_init(&eth, MY_MAC, MY_IP);

    /* 1. Reject >65507B payload before uint16_t wraparound (65508 and 65536) and NULL payload */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1,
                                                      dummy_payload, 65508U, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1,
                                                      dummy_payload, 65536U, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1, NULL,
                                                      4U, frame, &frame_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_icmp_build_echo_request(&icmp, &eth, PEER_IP, PEER_MAC, 1, 1,
                                                              NULL, 0U, frame, &frame_len));

    /* 2. Unpadded >= 60B Echo Request and Echo Reply round-trip (14 ETH + 20 IP + 8 ICMP + 32) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_icmp_build_echo_request(
                                      &icmp, &eth, MY_IP, MY_MAC, 0x2222, 0x0009, dummy_payload,
                                      sizeof(dummy_payload), frame, &frame_len));
    TEST_ASSERT_EQUAL_size_t(74U, frame_len);
    uint8_t reply[128];
    size_t reply_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_icmp_process_packet(&icmp, frame, frame_len, reply, &reply_len));
    TEST_ASSERT_EQUAL_size_t(74U, reply_len);
    TEST_ASSERT_EQUAL_UINT16(0U, syn_icmp_checksum(&reply[34], 40U));

    /* 3. 60B Ethernet-padded Echo Request with non-zero runt trailer bytes ignored */
    uint8_t padded_req[60];
    size_t padded_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_icmp_build_echo_request(&icmp, &eth, MY_IP, MY_MAC, 0x3333,
                                                              0x0001, (const uint8_t *)"ABCDE", 5U,
                                                              padded_req, &padded_len));
    TEST_ASSERT_EQUAL_size_t(60U, padded_len);
    /* Corrupt Ethernet trailer padding [47..59]; IP total length is 33 (offset 14 + 33 = 47) */
    memset(&padded_req[47], 0xEE, 13U);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_icmp_process_packet(&icmp, padded_req, 60U, reply, &reply_len));
    TEST_ASSERT_EQUAL_size_t(60U, reply_len);
    TEST_ASSERT_EQUAL_UINT16(33U, ((uint16_t)reply[16] << 8U) | reply[17]);
    TEST_ASSERT_EQUAL_UINT16(0U, syn_icmp_checksum(&reply[34], 13U));
    for (size_t i = 47U; i < 60U; i++) {
        TEST_ASSERT_EQUAL_UINT8(0U, reply[i]);
    }

    /* 4. Reject runt frame (< 34B), truncated ip_total_len, and corrupt ICMP checksum */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_process_packet(&icmp, padded_req, 20U, reply, &reply_len));
    uint8_t bad_len_pkt[60];
    memcpy(bad_len_pkt, padded_req, sizeof(bad_len_pkt));
    bad_len_pkt[16] = 0U;
    bad_len_pkt[17] = 60U; /* 14 + 60 = 74 > 60 */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_icmp_process_packet(&icmp, bad_len_pkt, 60U, reply, &reply_len));

    uint8_t bad_csum_pkt[60];
    memcpy(bad_csum_pkt, padded_req, sizeof(bad_csum_pkt));
    bad_csum_pkt[36] ^= 0xFFU;
    uint32_t prev_err = icmp.errors;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_icmp_process_packet(&icmp, bad_csum_pkt, 60U, reply, &reply_len));
    TEST_ASSERT_EQUAL_UINT32(prev_err + 1U, icmp.errors);
}
