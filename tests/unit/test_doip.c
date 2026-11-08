/**
 * @file test_doip.c
 * @brief Unity unit tests for SyntropicOS ISO 13400-2 (DoIP) Protocol Engine.
 */

#include "syntropic/proto/syn_doip.h"
#include "syntropic/proto/syn_uds.h"
#include "unity/unity.h"

#include <string.h>

static SYN_DoIP_Server g_doip;
static SYN_UDS_Server g_uds;

static void setup_doip_test(void)
{
    syn_doip_init(&g_doip, 0x1001);
    syn_uds_init(&g_uds);
}

void test_doip_init_and_identifiers(void)
{
    setup_doip_test();
    TEST_ASSERT_FALSE(syn_doip_init(NULL, 0x1001));
    TEST_ASSERT_FALSE(syn_doip_set_identifiers(NULL, NULL, NULL, NULL));

    uint8_t vin[17] = {'S', 'Y', 'N', 'T', 'R', 'O', 'P', 'I', 'C',
                       'O', 'S', '1', '2', '3', '4', '5', '6'};
    uint8_t eid[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t gid[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    TEST_ASSERT_TRUE(syn_doip_set_identifiers(&g_doip, vin, eid, gid));
    TEST_ASSERT_EQUAL_MEMORY(vin, g_doip.vin, 17);
    TEST_ASSERT_EQUAL_MEMORY(eid, g_doip.eid, 6);
    TEST_ASSERT_EQUAL_MEMORY(gid, g_doip.gid, 6);
}

void test_doip_header_parse_encode(void)
{
    setup_doip_test();
    SYN_DoIP_Header hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                           .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                           .payload_type = SYN_DOIP_TYPE_VEHICLE_ID_REQ,
                           .payload_length = 0};

    uint8_t buf[16] = {0};
    TEST_ASSERT_EQUAL_UINT16(8, syn_doip_encode_header(&hdr, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT16(0, syn_doip_encode_header(NULL, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT16(0, syn_doip_encode_header(&hdr, NULL, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT16(0, syn_doip_encode_header(&hdr, buf, 7));

    SYN_DoIP_Header parsed_hdr;
    TEST_ASSERT_FALSE(syn_doip_parse_header(NULL, sizeof(buf), &parsed_hdr));
    TEST_ASSERT_FALSE(syn_doip_parse_header(buf, 7, &parsed_hdr));
    TEST_ASSERT_TRUE(syn_doip_parse_header(buf, sizeof(buf), &parsed_hdr));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_PROTOCOL_VERSION, parsed_hdr.protocol_version);
    TEST_ASSERT_EQUAL_HEX16(SYN_DOIP_TYPE_VEHICLE_ID_REQ, parsed_hdr.payload_type);

    /* Test invalid protocol version */
    buf[0] = 0xFF;
    TEST_ASSERT_FALSE(syn_doip_parse_header(buf, sizeof(buf), &parsed_hdr));
}

void test_doip_vehicle_identification_request(void)
{
    setup_doip_test();
    uint8_t vin[17] = {'S', 'Y', 'N', 'T', 'R', 'O', 'P', 'I', 'C',
                       'O', 'S', '1', '2', '3', '4', '5', '6'};
    uint8_t eid[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t gid[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    syn_doip_set_identifiers(&g_doip, vin, eid, gid);

    uint8_t rx[8] = {0x02, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    uint8_t tx[64] = {0};
    uint16_t tx_len = 0;

    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx, sizeof(rx), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(41, tx_len); /* 8-byte header + 33-byte payload */

    SYN_DoIP_Header resp_hdr;
    TEST_ASSERT_TRUE(syn_doip_parse_header(tx, tx_len, &resp_hdr));
    TEST_ASSERT_EQUAL_HEX16(SYN_DOIP_TYPE_VEHICLE_ANNOUNCEMENT, resp_hdr.payload_type);
    TEST_ASSERT_EQUAL_MEMORY(vin, &tx[8], 17);
}

void test_doip_routing_activation_request(void)
{
    setup_doip_test();
    uint8_t rx[15] = {
        0x02, 0xFD, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, /* Header (payload_len = 7) */
        0x0E, 0x80,                                     /* Tester logical address 0x0E80 */
        0x00,                                           /* Activation type 0x00 */
        0x00, 0x00, 0x00, 0x00                          /* Reserved 4 bytes */
    };
    uint8_t tx[64] = {0};
    uint16_t tx_len = 0;

    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx, sizeof(rx), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(17, tx_len); /* 8-byte header + 9-byte payload */
    TEST_ASSERT_TRUE(g_doip.routing_activated);
    TEST_ASSERT_EQUAL_HEX16(0x0E80, g_doip.active_tester_address);

    SYN_DoIP_Header resp_hdr;
    TEST_ASSERT_TRUE(syn_doip_parse_header(tx, tx_len, &resp_hdr));
    TEST_ASSERT_EQUAL_HEX16(SYN_DOIP_TYPE_ROUTING_ACTIVATION_RESP, resp_hdr.payload_type);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_ROUTING_SUCCESS, tx[12]);
}

void test_doip_diagnostic_message_uds_loopback(void)
{
    setup_doip_test();
    /* Register VIN DID 0xF190 in UDS */
    uint8_t vin_data[] = "SYNTROPICOS123456";
    syn_uds_register_did(&g_uds, 0xF190, vin_data, sizeof(vin_data) - 1, false);

    uint8_t tx[256] = {0};
    uint16_t tx_len = 0;

    /* Activate routing for tester 0x0E80 first per ISO 13400-2 */
    uint8_t route_req[15] = {0x02, 0xFD, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07,
                             0x0E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, route_req, sizeof(route_req), tx,
                                          sizeof(tx), &tx_len));

    /* DoIP Diagnostic Request: Tester 0x0E80 -> ECU 0x1001, UDS ReadDID 0xF190 (0x22 0xF1 0x90) */
    uint8_t rx[15] = {
        0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, /* Header (payload_len = 7) */
        0x0E, 0x80,                                     /* Source Address 0x0E80 */
        0x10, 0x01,                                     /* Target Address 0x1001 */
        0x22, 0xF1, 0x90                                /* UDS Request: 0x22 ReadDID 0xF190 */
    };

    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx, sizeof(rx), tx, sizeof(tx), &tx_len));
    /* 13-byte 0x8002 ACK + (8 hdr + 4 addresses + 3 UDS hdr + 17 VIN) 0x8001 response */
    TEST_ASSERT_EQUAL_UINT16(13 + 8 + 4 + 3 + 17, tx_len);

    SYN_DoIP_Header ack_hdr;
    TEST_ASSERT_TRUE(syn_doip_parse_header(tx, tx_len, &ack_hdr));
    TEST_ASSERT_EQUAL_HEX16(SYN_DOIP_TYPE_DIAGNOSTIC_MSG_ACK, ack_hdr.payload_type);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_ACK_OK, tx[12]);

    SYN_DoIP_Header resp_hdr;
    TEST_ASSERT_TRUE(syn_doip_parse_header(&tx[13], (uint16_t)(tx_len - 13U), &resp_hdr));
    TEST_ASSERT_EQUAL_HEX16(SYN_DOIP_TYPE_DIAGNOSTIC_MSG, resp_hdr.payload_type);

    /* Verify UDS Positive Response (0x62 0xF1 0x90 ...) at tx[13 + 12..] */
    TEST_ASSERT_EQUAL_HEX8(0x62, tx[25]);
    TEST_ASSERT_EQUAL_HEX8(0xF1, tx[26]);
    TEST_ASSERT_EQUAL_HEX8(0x90, tx[27]);
}

void test_doip_error_nacks(void)
{
    setup_doip_test();
    uint8_t tx[64] = {0};
    uint16_t tx_len = 0;

    /* Invalid protocol version pattern */
    uint8_t rx_bad_ver[8] = {0xFF, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, rx_bad_ver, 8, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INCORRECT_PATTERN, tx[8]);

    /* Invalid payload type */
    uint8_t rx_bad_type[8] = {0x02, 0xFD, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx_bad_type, 8, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE, tx[8]);
    TEST_ASSERT_FALSE(syn_doip_process_msg(&g_doip, &g_uds, rx_bad_type, 8, tx, 8, &tx_len));

    /* Null pointer check */
    TEST_ASSERT_FALSE(syn_doip_process_msg(NULL, &g_uds, rx_bad_type, 8, tx, sizeof(tx), &tx_len));

    /* Truncated payload length vs header claim */
    uint8_t rx_trunc[8] = {0x02, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, rx_trunc, 8, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    /* Buffer capacity too small for vehicle announcement */
    uint8_t rx_veh_id[8] = {0x02, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_doip_process_msg(&g_doip, &g_uds, rx_veh_id, 8, tx, 10, &tx_len));

    /* Routing activation short payload (<7) */
    uint8_t rx_route_short[10] = {0x02, 0xFD, 0x00, 0x05, 0x00, 0x00, 0x00, 0x02, 0x0E, 0x80};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx_route_short, 10, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    /* Buffer capacity too small for routing activation response */
    uint8_t rx_route[15] = {0x02, 0xFD, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07,
                            0x0E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_doip_process_msg(&g_doip, &g_uds, rx_route, 15, tx, 10, &tx_len));

    /* Diagnostic message short payload (<4) */
    uint8_t rx_diag_short[10] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00, 0x02, 0x0E, 0x80};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx_diag_short, 10, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    /* Target address mismatch diagnostic message NACK */
    uint8_t rx_wrong_ta[15] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07,
                               0x0E, 0x80, 0x99, 0x99, 0x22, 0xF1, 0x90};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, rx_wrong_ta, 15, tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(13, tx_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_NACK_UNKNOWN_TA,
                           tx[12]); /* NACK code: Unknown target address */

    /* Buffer capacity too small for diagnostic message NACK */
    TEST_ASSERT_FALSE(syn_doip_process_msg(&g_doip, &g_uds, rx_wrong_ta, 15, tx, 10, &tx_len));

    /* Buffer capacity too small for diagnostic message response */
    uint8_t rx_diag[15] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07,
                           0x0E, 0x80, 0x10, 0x01, 0x22, 0xF1, 0x90};
    TEST_ASSERT_FALSE(syn_doip_process_msg(&g_doip, &g_uds, rx_diag, 15, tx, 10, &tx_len));
}

void test_doip_payload_length_integer_overflow(void)
{
    setup_doip_test();
    /* Craft a DoIP packet claiming 0xFFFFFFFF payload bytes in an 8-byte packet */
    uint8_t rx_overflow[8] = {0x02, 0xFD, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t tx[64] = {0};
    uint16_t tx_len = 0;

    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, rx_overflow, sizeof(rx_overflow), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(9, tx_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);
}

void test_r5_a10_routing_code_ack_suppression_and_id_filters(void)
{
    setup_doip_test();
    uint8_t vin[17] = {'S', 'Y', 'N', 'T', 'R', 'O', 'P', 'I', 'C',
                       'O', 'S', '1', '2', '3', '4', '5', '6'};
    uint8_t eid[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t gid[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    syn_doip_set_identifiers(&g_doip, vin, eid, gid);
    TEST_ASSERT_TRUE(syn_doip_set_identifiers(&g_doip, NULL, NULL, NULL));

    uint8_t tx[128] = {0};
    uint16_t tx_len = 0;

    /* 0x0001 with non-zero payload length -> Generic NACK 0x04 */
    uint8_t bad_id_req[9] = {0x02, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, bad_id_req, sizeof(bad_id_req), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    /* 0x0002 EID filter: bad len, mismatch (silent 0B), match (41B announcement) */
    uint8_t bad_eid_len[10] = {0x02, 0xFD, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x11};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, bad_eid_len, sizeof(bad_eid_len), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    uint8_t eid_req[14] = {0x02, 0xFD, 0x00, 0x02, 0x00, 0x00, 0x00,
                           0x06, 0x00, 0x11, 0x22, 0x33, 0x44, 0x99};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, eid_req, sizeof(eid_req), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(0U, tx_len);

    eid_req[13] = 0x55;
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, eid_req, sizeof(eid_req), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(41U, tx_len);

    /* 0x0003 VIN filter: bad len, mismatch (silent 0B), match (41B announcement) */
    uint8_t bad_vin_len[10] = {0x02, 0xFD, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 'S', 'Y'};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, bad_vin_len, sizeof(bad_vin_len), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx[8]);

    uint8_t vin_req[25] = {0x02, 0xFD, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11};
    memcpy(&vin_req[8], vin, 17);
    vin_req[24] = 'X';
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, vin_req, sizeof(vin_req), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(0U, tx_len);

    vin_req[24] = '6';
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, vin_req, sizeof(vin_req), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(41U, tx_len);

    /* 0x8001 before routing activation -> Diagnostic NACK 0x02 Invalid SA */
    uint8_t diag_tester[14] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00,
                               0x06, 0x0E, 0x80, 0xFF, 0xFF, 0x3E, 0x80};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, diag_tester, sizeof(diag_tester), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(13U, tx_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_NACK_INVALID_SA, tx[12]);

    /* Activate routing (0x0E80) and verify ISO 13400-2 Table 25 code 0x10 */
    uint8_t route_req[15] = {0x02, 0xFD, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07,
                             0x0E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, route_req, sizeof(route_req), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(0x10U, tx[12]);

    /* Wrong SA (0x0E81) when routing active -> 0x02 Invalid SA */
    uint8_t wrong_sa[14] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00,
                            0x06, 0x0E, 0x81, 0x10, 0x01, 0x3E, 0x00};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, wrong_sa, sizeof(wrong_sa), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_NACK_INVALID_SA, tx[12]);

    /* Unknown TA (0x9999) when routing active -> 0x03 Unknown TA */
    uint8_t wrong_ta[14] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00,
                            0x06, 0x0E, 0x80, 0x99, 0x99, 0x3E, 0x00};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, &g_uds, wrong_ta, sizeof(wrong_ta), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_NACK_UNKNOWN_TA, tx[12]);

    /* Functional TesterPresent with suppressPosRspMsgIndicationBit (0x3E 0x80):
     * emits ONLY 13-byte 0x8002 ACK and suppresses empty 0x8001 frame! */
    TEST_ASSERT_TRUE(syn_doip_process_msg(&g_doip, &g_uds, diag_tester, sizeof(diag_tester), tx,
                                          sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(13U, tx_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_DOIP_DIAG_ACK_OK, tx[12]);

    /* Also test NULL uds / empty UDS payload & output buffer capacity check on activated route */
    uint8_t empty_uds[12] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00,
                             0x00, 0x04, 0x0E, 0x80, 0x10, 0x01};
    TEST_ASSERT_TRUE(
        syn_doip_process_msg(&g_doip, NULL, empty_uds, sizeof(empty_uds), tx, sizeof(tx), &tx_len));
    TEST_ASSERT_EQUAL_UINT16(13U, tx_len);

    uint8_t phys_tp[14] = {0x02, 0xFD, 0x80, 0x01, 0x00, 0x00, 0x00,
                           0x06, 0x0E, 0x80, 0x10, 0x01, 0x3E, 0x00};
    TEST_ASSERT_FALSE(
        syn_doip_process_msg(&g_doip, &g_uds, phys_tp, sizeof(phys_tp), tx, 15U, &tx_len));
}

void run_doip_tests(void)
{
    RUN_TEST(test_doip_init_and_identifiers);
    RUN_TEST(test_doip_header_parse_encode);
    RUN_TEST(test_doip_vehicle_identification_request);
    RUN_TEST(test_doip_routing_activation_request);
    RUN_TEST(test_doip_diagnostic_message_uds_loopback);
    RUN_TEST(test_doip_error_nacks);
    RUN_TEST(test_doip_payload_length_integer_overflow);
    RUN_TEST(test_r5_a10_routing_code_ack_suppression_and_id_filters);
}
