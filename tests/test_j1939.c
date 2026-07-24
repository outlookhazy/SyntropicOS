/**
 * @file test_j1939.c
 * @brief Unit test suite for SAE J1939 CAN protocol engine (syn_j1939).
 */

#include "unity/unity.h"
#include "syntropic/proto/syn_j1939.h"
#include <string.h>

void test_j1939_id_pack_unpack_pdu1_pdu2(void)
{
    SYN_J1939_Header hdr;

    /* PDU1 Format (PF < 240, e.g. Request PGN 59904 = 0x00EA00, DA=0x20, SA=0x10, Priority=6) */
    uint32_t can_id_pdu1 = syn_j1939_id_pack(6, SYN_J1939_PGN_REQUEST, 0x10, 0x20);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_id_unpack(can_id_pdu1, &hdr));
    TEST_ASSERT_EQUAL_UINT8(6, hdr.priority);
    TEST_ASSERT_EQUAL_UINT8(0, hdr.dp);
    TEST_ASSERT_EQUAL_UINT8(234, hdr.pf);
    TEST_ASSERT_EQUAL_UINT8(0x20, hdr.da);
    TEST_ASSERT_EQUAL_UINT8(0x10, hdr.sa);
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_REQUEST, hdr.pgn);
    TEST_ASSERT_TRUE(hdr.is_pdu1);

    /* PDU2 Format (PF >= 240, e.g. EEC1 PGN 61444 = 0x00F004, Broadcast, SA=0x00, Priority=3) */
    uint32_t can_id_pdu2 = syn_j1939_id_pack(3, SYN_J1939_PGN_EEC1, 0x00, SYN_J1939_ADDR_GLOBAL);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_id_unpack(can_id_pdu2, &hdr));
    TEST_ASSERT_EQUAL_UINT8(3, hdr.priority);
    TEST_ASSERT_EQUAL_UINT8(0, hdr.dp);
    TEST_ASSERT_EQUAL_UINT8(240, hdr.pf);
    TEST_ASSERT_EQUAL_UINT8(4, hdr.ps);
    TEST_ASSERT_EQUAL_UINT8(0x00, hdr.sa);
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_EEC1, hdr.pgn);
    TEST_ASSERT_FALSE(hdr.is_pdu1);

    /* Unpack invalid params */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_id_unpack(can_id_pdu2, NULL));
}

void test_j1939_name_encode_decode_roundtrip(void)
{
    SYN_J1939_Name name1 = {
        .identity_number    = 123456U,
        .manufacturer_code  = 432U,
        .ecu_instance       = 1U,
        .function_instance  = 2U,
        .function           = 128U,
        .vehicle_system     = 10U,
        .vehicle_system_inst= 1U,
        .industry_group     = 1U, /* On-Highway */
        .arbitrary_addr_cap = true
    };

    uint8_t buf[8];
    syn_j1939_name_encode(&name1, buf);

    SYN_J1939_Name name2;
    syn_j1939_name_decode(buf, &name2);

    TEST_ASSERT_EQUAL_UINT32(name1.identity_number, name2.identity_number);
    TEST_ASSERT_EQUAL_UINT16(name1.manufacturer_code, name2.manufacturer_code);
    TEST_ASSERT_EQUAL_UINT8(name1.ecu_instance, name2.ecu_instance);
    TEST_ASSERT_EQUAL_UINT8(name1.function_instance, name2.function_instance);
    TEST_ASSERT_EQUAL_UINT8(name1.function, name2.function);
    TEST_ASSERT_EQUAL_UINT8(name1.vehicle_system, name2.vehicle_system);
    TEST_ASSERT_EQUAL_UINT8(name1.vehicle_system_inst, name2.vehicle_system_inst);
    TEST_ASSERT_EQUAL_UINT8(name1.industry_group, name2.industry_group);
    TEST_ASSERT_TRUE(name2.arbitrary_addr_cap);

    /* Test null safety */
    syn_j1939_name_encode(NULL, buf);
    syn_j1939_name_decode(NULL, &name2);
}

void test_j1939_build_frames(void)
{
    SYN_CAN_Frame frame;

    /* Address claim frame */
    SYN_J1939_Node node;
    SYN_J1939_Name name = { .identity_number = 999U, .manufacturer_code = 100U };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_node_init(&node, 0x80, &name));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_address_claim(&node, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_TRUE(frame.extended);

    /* Request frame */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_request(0x10, 0x00, SYN_J1939_PGN_EEC1, &frame));
    TEST_ASSERT_EQUAL_UINT8(3, frame.dlc);
    TEST_ASSERT_EQUAL_UINT8((SYN_J1939_PGN_EEC1 & 0xFF), frame.data[0]);

    /* TP.CM_BAM frame */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_bam(0x10, SYN_J1939_PGN_DM1, 24, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_BAM, frame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(24, frame.data[1]);
    TEST_ASSERT_EQUAL_UINT8(4, frame.data[3]); /* 24 bytes / 7 = 4 packets */

    /* TP.DT frame */
    uint8_t payload[7] = { 1, 2, 3, 4, 5, 6, 7 };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 1, payload, 7, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(1, frame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(1, frame.data[1]);

    /* Invalid build params */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_address_claim(NULL, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_request(0x10, 0x00, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_tp_bam(0x10, 0, 5, &frame)); /* < 9 bytes */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_tp_dt(0x10, 0, payload, 7, &frame)); /* seq 0 */
}

void test_j1939_dm1_encoding(void)
{
    uint8_t buf[32];
    SYN_J1939_DTC dtcs[2] = {
        { .spn = 100, .fmi = 3, .occurrence_count = 1, .conversion_method = 0 },
        { .spn = 190, .fmi = 2, .occurrence_count = 5, .conversion_method = 0 }
    };

    /* No DTCs */
    size_t len0 = syn_j1939_encode_dm1(buf, sizeof(buf), NULL, 0, 0x00);
    TEST_ASSERT_EQUAL_UINT32(6, len0);
    TEST_ASSERT_EQUAL_UINT8(0, buf[2]);

    /* 2 DTCs */
    size_t len2 = syn_j1939_encode_dm1(buf, sizeof(buf), dtcs, 2, 0x01);
    TEST_ASSERT_EQUAL_UINT32(10, len2);
    TEST_ASSERT_EQUAL_UINT8(0x01, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(100, buf[2]);

    /* Buffer capacity limits */
    TEST_ASSERT_EQUAL_UINT32(0, syn_j1939_encode_dm1(NULL, 0, dtcs, 2, 0));
}

void test_j1939_tp_bam_multi_packet_assembly(void)
{
    SYN_J1939_Node receiver;
    SYN_J1939_Name name = { .identity_number = 1U };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_node_init(&receiver, 0x20, &name));

    /* 1. Build & Process TP.CM_BAM frame for PGN 65262 (ET1), total 14 bytes (2 packets) */
    SYN_CAN_Frame bam_frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_bam(0x10, SYN_J1939_PGN_ET1, 14, &bam_frame));

    uint32_t rxd_pgn = 0;
    const uint8_t *rxd_data = NULL;
    size_t rxd_len = 0;

    /* First BAM control frame should return SYN_BUSY */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_j1939_process_frame(&receiver, &bam_frame, &rxd_pgn, &rxd_data, &rxd_len));

    /* 2. Process Packet 1 (TP.DT) */
    uint8_t payload1[7] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
    SYN_CAN_Frame dt_frame1;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 1, payload1, 7, &dt_frame1));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_j1939_process_frame(&receiver, &dt_frame1, &rxd_pgn, &rxd_data, &rxd_len));

    /* 3. Process Packet 2 (TP.DT) -> completes assembly! */
    uint8_t payload2[7] = { 'H', 'I', 'J', 'K', 'L', 'M', 'N' };
    SYN_CAN_Frame dt_frame2;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 2, payload2, 7, &dt_frame2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_process_frame(&receiver, &dt_frame2, &rxd_pgn, &rxd_data, &rxd_len));

    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_ET1, rxd_pgn);
    TEST_ASSERT_EQUAL_UINT32(14, rxd_len);
    TEST_ASSERT_NOT_NULL(rxd_data);
    TEST_ASSERT_EQUAL_MEMORY("ABCDEFGHIJKLMN", rxd_data, 14);

    /* Test null invalid parameters */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_process_frame(NULL, &dt_frame2, &rxd_pgn, &rxd_data, &rxd_len));
}

void run_j1939_tests(void)
{
    RUN_TEST(test_j1939_id_pack_unpack_pdu1_pdu2);
    RUN_TEST(test_j1939_name_encode_decode_roundtrip);
    RUN_TEST(test_j1939_build_frames);
    RUN_TEST(test_j1939_dm1_encoding);
    RUN_TEST(test_j1939_tp_bam_multi_packet_assembly);
}
