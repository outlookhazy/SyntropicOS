/**
 * @file test_j1939.c
 * @brief Unit test suite for SAE J1939 CAN protocol engine (syn_j1939).
 */

#include "syntropic/proto/syn_j1939.h"
#include "unity/unity.h"

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
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_node_init(NULL, 0, NULL));
}

void test_j1939_name_encode_decode_roundtrip(void)
{
    SYN_J1939_Name name1 = {.identity_number = 123456U,
                            .manufacturer_code = 432U,
                            .ecu_instance = 1U,
                            .function_instance = 2U,
                            .function = 128U,
                            .vehicle_system = 10U,
                            .vehicle_system_inst = 1U,
                            .industry_group = 1U, /* On-Highway */
                            .arbitrary_addr_cap = true};

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
    SYN_J1939_Name name = {.identity_number = 999U, .manufacturer_code = 100U};
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
    uint8_t payload[7] = {1, 2, 3, 4, 5, 6, 7};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 1, payload, 7, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_EQUAL_UINT8(1, frame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(1, frame.data[1]);

    /* Invalid build params */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_address_claim(NULL, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_j1939_build_request(0x10, 0x00, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_j1939_build_tp_bam(0x10, 0, 5, &frame)); /* < 9 bytes */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_j1939_build_tp_dt(0x10, 0, payload, 7, &frame)); /* seq 0 */
}

void test_j1939_dm1_encoding(void)
{
    uint8_t buf[32];
    SYN_J1939_DTC dtcs[2] = {{.spn = 100, .fmi = 3, .occurrence_count = 1, .conversion_method = 0},
                             {.spn = 190, .fmi = 2, .occurrence_count = 5, .conversion_method = 0}};

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
    SYN_J1939_Name name = {.identity_number = 1U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_node_init(&receiver, 0x20, &name));

    /* 1. Build & Process TP.CM_BAM frame for PGN 65262 (ET1), total 14 bytes (2 packets) */
    SYN_CAN_Frame bam_frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_bam(0x10, SYN_J1939_PGN_ET1, 14, &bam_frame));

    uint32_t rxd_pgn = 0;
    const uint8_t *rxd_data = NULL;
    size_t rxd_len = 0;

    /* First BAM control frame should return SYN_BUSY */
    TEST_ASSERT_EQUAL_INT(
        SYN_BUSY, syn_j1939_process_frame(&receiver, &bam_frame, &rxd_pgn, &rxd_data, &rxd_len));

    /* 2. Process Packet 1 (TP.DT) */
    uint8_t payload1[7] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};
    SYN_CAN_Frame dt_frame1;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 1, payload1, 7, &dt_frame1));
    TEST_ASSERT_EQUAL_INT(
        SYN_BUSY, syn_j1939_process_frame(&receiver, &dt_frame1, &rxd_pgn, &rxd_data, &rxd_len));

    /* 3. Process Packet 2 (TP.DT) -> completes assembly! */
    uint8_t payload2[7] = {'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    SYN_CAN_Frame dt_frame2;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_j1939_build_tp_dt(0x10, 2, payload2, 7, &dt_frame2));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_j1939_process_frame(&receiver, &dt_frame2, &rxd_pgn, &rxd_data, &rxd_len));

    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_ET1, rxd_pgn);
    TEST_ASSERT_EQUAL_UINT32(14, rxd_len);
    TEST_ASSERT_NOT_NULL(rxd_data);
    TEST_ASSERT_EQUAL_MEMORY("ABCDEFGHIJKLMN", rxd_data, 14);

    /* Test null invalid parameters */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_j1939_process_frame(NULL, &dt_frame2, &rxd_pgn, &rxd_data, &rxd_len));
}

static void test_j1939_dm2_encoding(void)
{
    uint8_t buf[8];
    SYN_J1939_DTC dtc = {.spn = 100, .fmi = 3, .occurrence_count = 1, .conversion_method = 0};
    size_t len = syn_j1939_encode_dm2(buf, sizeof(buf), &dtc, 1, 0x04);
    TEST_ASSERT_EQUAL_UINT32(6, len);
}

static void test_j1939_dtc_log_manager(void)
{
    SYN_J1939_DTCLog log;
    syn_j1939_dtc_log_init(&log);

    TEST_ASSERT_EQUAL(0, log.active_count);
    TEST_ASSERT_EQUAL(0, log.prev_count);

    /* Add active DTC (SPN 190 Engine Speed, FMI 2 Data Erratic) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 190, 2));
    TEST_ASSERT_EQUAL(1, log.active_count);
    TEST_ASSERT_EQUAL(1, log.active_dtcs[0].occurrence_count);

    /* Add duplicate -> increases occurrence count */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 190, 2));
    TEST_ASSERT_EQUAL(1, log.active_count);
    TEST_ASSERT_EQUAL(2, log.active_dtcs[0].occurrence_count);

    /* Clear active -> moves to prev_dtcs (DM2) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 190, 2));
    /* Add another active DTC and DM11 Clear active */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 100, 3));
    TEST_ASSERT_EQUAL(1, log.active_count);
    syn_j1939_dtc_clear_dm11(&log);
    TEST_ASSERT_EQUAL(0, log.active_count);

    /* DM3 Clear -> clears prev_dtcs */
    syn_j1939_dtc_clear_dm3(&log);
    TEST_ASSERT_EQUAL(0, log.prev_count);
}

static void test_j1939_dtc_log_null_checks_and_multi_packet_short_last_frame(void)
{
    /* Null guards for DTC log operations */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_j1939_dtc_add_active(NULL, 100, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_j1939_dtc_clear_active(NULL, 100, 1));

    /* Null guards for name encode/decode and id_unpack */
    syn_j1939_name_encode(NULL, NULL);
    syn_j1939_name_decode(NULL, NULL);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_j1939_id_unpack(0x18FEF100UL, NULL));

    /* Multi-packet BAM assembly where final packet bytes are clipped to total_bytes */
    SYN_J1939_Node node;
    SYN_J1939_Name name = {.identity_number = 1U};
    syn_j1939_node_init(&node, 0x10, &name);

    /* Initiate 10-byte BAM transfer (2 packets: pkt1=7 bytes, pkt2=3 bytes) */
    SYN_CAN_Frame bam_frame;
    syn_j1939_build_tp_bam(0x20, SYN_J1939_PGN_DM1, 10, &bam_frame);

    uint32_t out_pgn = 0;
    const uint8_t *out_data = NULL;
    size_t out_len = 0;
    syn_j1939_process_frame(&node, &bam_frame, &out_pgn, &out_data, &out_len);

    /* Send packet 1 (seq = 1, 7 bytes payload) */
    SYN_CAN_Frame dt1;
    uint8_t d1[7] = {1, 2, 3, 4, 5, 6, 7};
    syn_j1939_build_tp_dt(0x20, 1, d1, 7, &dt1);
    syn_j1939_process_frame(&node, &dt1, &out_pgn, &out_data, &out_len);

    /* Send packet 2 (seq = 2, 7 bytes provided, but total_bytes=10 so 3 bytes copied) */
    SYN_CAN_Frame dt2;
    uint8_t d2[7] = {8, 9, 10, 11, 12, 13, 14};
    syn_j1939_build_tp_dt(0x20, 2, d2, 7, &dt2);
    SYN_Status st = syn_j1939_process_frame(&node, &dt2, &out_pgn, &out_data, &out_len);

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT32(10, out_len);
}

static void test_j1939_single_frame_rx_and_dm2_encoding(void)
{
    SYN_J1939_Node node;
    SYN_J1939_Name name = {.identity_number = 1U};
    syn_j1939_node_init(&node, 0x10, &name);

    /* Construct single-frame CAN packet (e.g. EEC1 PGN 61444, SA=0x20) */
    uint32_t can_id = syn_j1939_id_pack(3, SYN_J1939_PGN_EEC1, 0x20, SYN_J1939_ADDR_GLOBAL);
    uint8_t payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    SYN_CAN_Frame frame = {.id = can_id, .dlc = 8, .extended = true};
    memcpy(frame.data, payload, 8);

    uint32_t out_pgn = 0;
    const uint8_t *out_data = NULL;
    size_t out_len = 0;

    SYN_Status st = syn_j1939_process_frame(&node, &frame, &out_pgn, &out_data, &out_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_EEC1, out_pgn);
    TEST_ASSERT_EQUAL(8, out_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, out_data, 8);

    /* DM2 encoding test */
    uint8_t dm2_buf[32];
    size_t len = syn_j1939_encode_dm2(dm2_buf, sizeof(dm2_buf), NULL, 0, 0);
    TEST_ASSERT_EQUAL(6, len);
}

static void test_j1939_pgn_filter_and_claim_conflict(void)
{
    /* 1. DM1 encoding buffer truncation (break in dtc_list loop) */
    SYN_J1939_DTC dtcs[3] = {{.spn = 100, .fmi = 1, .occurrence_count = 1, .conversion_method = 0},
                             {.spn = 200, .fmi = 2, .occurrence_count = 1, .conversion_method = 0},
                             {.spn = 300, .fmi = 3, .occurrence_count = 1, .conversion_method = 0}};
    uint8_t buf[8];
    /* Buffer size = 8 fits header (2) + 1 DTC (4) = 6 bytes, breaks on 2nd DTC */
    size_t len = syn_j1939_encode_dm1(buf, sizeof(buf), dtcs, 3, 0);
    TEST_ASSERT_EQUAL(6, len);

    /* DM1 zero DTC count with small buffer (buf_size = 5) -> returns 2 (line 210) */
    uint8_t buf5[5];
    TEST_ASSERT_EQUAL(2, syn_j1939_encode_dm1(buf5, 5, NULL, 0, 0));

    /* 2. Active DTC log overflow (max logged DTCs = 16) */
    SYN_J1939_DTCLog log;
    syn_j1939_dtc_log_init(&log);
    for (uint32_t i = 0; i < SYN_J1939_MAX_LOGGED_DTCS; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 1000 + i, 1));
    }
    /* 17th DTC fails */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_j1939_dtc_add_active(&log, 9999, 1));

    /* 3. Shift elements when clearing active DTC */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 1000, 1));
    TEST_ASSERT_EQUAL(SYN_J1939_MAX_LOGGED_DTCS - 1, log.active_count);

    /* Clear non-existent DTC -> returns SYN_ERROR (line 369) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_j1939_dtc_clear_active(&log, 99999, 1));
}

static void test_r5_a15_unicast_rts_cts_eomack_and_da_dlc_filters(void)
{
    SYN_J1939_Node ecu;
    SYN_J1939_Name name = {.identity_number = 42U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_node_init(&ecu, 0x25U, &name));

    SYN_CAN_Frame tx_resp;
    TEST_ASSERT_FALSE(syn_j1939_get_tx(NULL, &tx_resp));
    TEST_ASSERT_FALSE(syn_j1939_get_tx(&ecu, NULL));
    TEST_ASSERT_FALSE(syn_j1939_get_tx(&ecu, &tx_resp));

    /* Builder parameter validation */
    SYN_CAN_Frame rts;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 16U, 0xFFU, NULL));
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_j1939_build_tp_rts(0x10U, SYN_J1939_ADDR_GLOBAL, SYN_J1939_PGN_DM1, 16U, 0xFFU, &rts));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 16U, 0U, &rts));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 8U, 0xFFU, &rts));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 1800U, 0xFFU, &rts));

    uint32_t out_pgn = 0U;
    const uint8_t *out_data = NULL;
    size_t out_len = 0U;

    /* 1. Reject RTS addressed to another ECU (DA=0x30 != ecu.sa=0x25) */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_j1939_build_tp_rts(0x10U, 0x30U, SYN_J1939_PGN_DM1, 16U, 0xFFU, &rts));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &rts, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);
    TEST_ASSERT_FALSE(syn_j1939_get_tx(&ecu, &tx_resp));

    /* Reject BAM with unicast DA != 0xFF, unknown ctrl byte, and short DLC < 8 */
    SYN_CAN_Frame bad_cm = rts;
    bad_cm.data[0] = SYN_J1939_TP_CTRL_BAM;
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &bad_cm, &out_pgn, &out_data, &out_len));
    bad_cm.data[0] = 99U;
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &bad_cm, &out_pgn, &out_data, &out_len));
    bad_cm.dlc = 4U;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_process_frame(&ecu, &bad_cm, &out_pgn, &out_data, &out_len));

    /* Reject RTS with mismatched packet count or reserved max_packets == 0 */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 16U, 0xFFU, &rts));
    rts.data[3] = 9U;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &rts, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);

    rts.data[3] = 3U;
    rts.data[4] = 0U;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &rts, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);

    /* 2. Accept unicast RTS (16 bytes = 3 packets, windowed max_packets = 2) -> emits TP.CM_CTS */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 16U, 2U, &rts));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &rts, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tp_rx.active);
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));

    /* Broadcast BAM from another peer must NOT overwrite an active unicast RTS/CTS session */
    SYN_CAN_Frame stray_bam;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_bam(0x88U, SYN_J1939_PGN_ET1, 14U, &stray_bam));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &stray_bam, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.is_bam);
    TEST_ASSERT_EQUAL_UINT8(0x10U, ecu.tp_rx.sa);

    SYN_J1939_Header resp_hdr;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_id_unpack(tx_resp.id, &resp_hdr));
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_TP_CM, resp_hdr.pgn);
    TEST_ASSERT_EQUAL_UINT8(0x25U, resp_hdr.sa);
    TEST_ASSERT_EQUAL_UINT8(0x10U, resp_hdr.da);
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_CTS, tx_resp.data[0]);
    TEST_ASSERT_EQUAL_UINT8(2U, tx_resp.data[1]); /* 2 packets granted */
    TEST_ASSERT_EQUAL_UINT8(1U, tx_resp.data[2]); /* Next seq = 1 */

    /* 3. Filter TP.DT: reject short DLC < 8, wrong DA, and out-of-order seq */
    SYN_CAN_Frame dt;
    uint8_t p1[7] = {'0', '1', '2', '3', '4', '5', '6'};
    uint8_t p2[7] = {'7', '8', '9', 'A', 'B', 'C', 'D'};
    uint8_t p3[2] = {'E', 'F'};

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x30U, 1U, p1, 7U, &dt));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_EQUAL_UINT8(1U, ecu.tp_rx.next_seq);

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x25U, 1U, p1, 7U, &dt));
    dt.dlc = 5U;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x25U, 2U, p2, 7U, &dt));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));

    /* Deliver seq 1 & seq 2 -> completes first 2-packet CTS window and emits CTS for seq 3 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x25U, 1U, p1, 7U, &dt));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(syn_j1939_get_tx(&ecu, &tx_resp));

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x25U, 2U, p2, 7U, &dt));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_CTS, tx_resp.data[0]);
    TEST_ASSERT_EQUAL_UINT8(1U, tx_resp.data[1]); /* 1 packet left */
    TEST_ASSERT_EQUAL_UINT8(3U, tx_resp.data[2]); /* Next seq = 3 */

    /* Deliver final seq 3 -> returns SYN_OK AND queues TP.CM_ACK (EoMAck = 19) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_dt_da(0x10U, 0x25U, 3U, p3, 2U, &dt));
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_process_frame(&ecu, &dt, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_DM1, out_pgn);
    TEST_ASSERT_EQUAL_UINT32(16U, out_len);
    TEST_ASSERT_EQUAL_MEMORY("0123456789ABCDEF", out_data, 16U);

    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_id_unpack(tx_resp.id, &resp_hdr));
    TEST_ASSERT_EQUAL_UINT8(0x25U, resp_hdr.sa);
    TEST_ASSERT_EQUAL_UINT8(0x10U, resp_hdr.da);
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_ACK, tx_resp.data[0]);
    TEST_ASSERT_EQUAL_UINT8(16U, tx_resp.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0U, tx_resp.data[2]);
    TEST_ASSERT_EQUAL_UINT8(3U, tx_resp.data[3]);

    /* Also verify single-window RTS (max_packets = 0xFF) and TP.CM_ABORT */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 10U, 0xFFU, &rts));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_j1939_process_frame(&ecu, &rts, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL_UINT8(2U, tx_resp.data[1]);

    /* Connection Abort from unrelated SA is ignored; from active peer closes session */
    SYN_CAN_Frame abort_frame = rts;
    abort_frame.data[0] = SYN_J1939_TP_CTRL_ABORT;
    abort_frame.id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, 0x99U, 0x25U);
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &abort_frame, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tp_rx.active);

    abort_frame.id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, 0x10U, 0x25U);
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &abort_frame, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);
}

static void test_j1939_pro16_tp_concurrency_ext_filter_and_dm2_dedup(void)
{
    SYN_J1939_Node ecu;
    SYN_J1939_Name name = {.identity_number = 77U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_node_init(&ecu, 0x25U, &name));

    uint32_t out_pgn = 0U;
    const uint8_t *out_data = NULL;
    size_t out_len = 0U;

    /* 1. Reject Standard 11-bit CAN frames in syn_j1939_process_frame */
    SYN_CAN_Frame std_frame = {.id = 0x123U, .dlc = 8U, .extended = false};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_j1939_process_frame(&ecu, &std_frame, &out_pgn, &out_data, &out_len));

    /* 2. Active BAM session must NOT be overwritten by a second BAM, and concurrent RTS receives
     *    TP.Conn_Abort (reason 1) without clobbering the active BAM session */
    SYN_CAN_Frame bam1;
    SYN_CAN_Frame bam2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_bam(0x10U, SYN_J1939_PGN_ET1, 10U, &bam1));
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_build_tp_bam(0x30U, SYN_J1939_PGN_DM1, 14U, &bam2));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &bam1, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tp_rx.active);
    TEST_ASSERT_EQUAL_UINT8(0x10U, ecu.tp_rx.sa);

    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &bam2, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_EQUAL_UINT8(0x10U, ecu.tp_rx.sa);
    TEST_ASSERT_EQUAL_UINT32(SYN_J1939_PGN_ET1, ecu.tp_rx.pgn);

    SYN_CAN_Frame rts_collide;
    SYN_CAN_Frame tx_resp;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_j1939_build_tp_rts(0x44U, 0x25U, SYN_J1939_PGN_DM2, 16U, 4U, &rts_collide));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &rts_collide, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_EQUAL_UINT8(0x10U, ecu.tp_rx.sa);
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_ABORT, tx_resp.data[0]);
    TEST_ASSERT_EQUAL_UINT8(1U, tx_resp.data[1]);
    SYN_J1939_Header abort_hdr;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_id_unpack(tx_resp.id, &abort_hdr));
    TEST_ASSERT_EQUAL_UINT8(0x44U, abort_hdr.da);

    /* Abort BAM session, then start unicast RTS and verify:
     * (a) competing RTS from Peer B (0x44) does NOT overwrite Peer A's pending CTS in tx_data;
     * (b) duplicate RTS from Peer A (0x10) aborts session and clears tp_rx.active;
     * (c) TP_CTRL_ABORT clears tx_pending on a fresh RTS. */
    ecu.tp_rx.active = false;
    SYN_CAN_Frame rts_ok;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_j1939_build_tp_rts(0x10U, 0x25U, SYN_J1939_PGN_DM1, 14U, 2U, &rts_ok));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &rts_ok, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tx_pending);

    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &rts_collide, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tp_rx.active);
    TEST_ASSERT_EQUAL_UINT8(0x10U, ecu.tp_rx.sa);
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_CTS, tx_resp.data[0]);
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_id_unpack(tx_resp.id, &abort_hdr));
    TEST_ASSERT_EQUAL_UINT8(0x10U, abort_hdr.da);

    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &rts_ok, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);
    TEST_ASSERT_TRUE(syn_j1939_get_tx(&ecu, &tx_resp));
    TEST_ASSERT_EQUAL_UINT8(SYN_J1939_TP_CTRL_ABORT, tx_resp.data[0]);

    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &rts_ok, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_TRUE(ecu.tp_rx.active);
    TEST_ASSERT_TRUE(ecu.tx_pending);

    SYN_CAN_Frame abort_rx = rts_ok;
    abort_rx.data[0] = SYN_J1939_TP_CTRL_ABORT;
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_j1939_process_frame(&ecu, &abort_rx, &out_pgn, &out_data, &out_len));
    TEST_ASSERT_FALSE(ecu.tp_rx.active);
    TEST_ASSERT_FALSE(ecu.tx_pending);
    TEST_ASSERT_FALSE(syn_j1939_get_tx(&ecu, &tx_resp));

    /* 3. Deduplicate (spn, fmi) in prev_dtcs on repeated syn_j1939_dtc_clear_active & saturate */
    SYN_J1939_DTCLog log;
    syn_j1939_dtc_log_init(&log);
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 500U, 4U));
    log.active_dtcs[0].occurrence_count = 100U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 500U, 4U));
    TEST_ASSERT_EQUAL(1U, log.prev_count);
    TEST_ASSERT_EQUAL_UINT8(100U, log.prev_dtcs[0].occurrence_count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 500U, 4U));
    log.active_dtcs[0].occurrence_count = 20U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 500U, 4U));
    TEST_ASSERT_EQUAL(1U, log.prev_count);
    TEST_ASSERT_EQUAL_UINT8(120U, log.prev_dtcs[0].occurrence_count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 500U, 4U));
    log.active_dtcs[0].occurrence_count = 20U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 500U, 4U));
    TEST_ASSERT_EQUAL(1U, log.prev_count);
    TEST_ASSERT_EQUAL_UINT8(126U, log.prev_dtcs[0].occurrence_count);

    /* Fill remaining prev_dtcs slots and verify graceful drop when prev_dtcs is full */
    for (uint32_t idx = 1U; idx < SYN_J1939_MAX_LOGGED_DTCS; idx++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 600U + idx, 1U));
        TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 600U + idx, 1U));
    }
    TEST_ASSERT_EQUAL(SYN_J1939_MAX_LOGGED_DTCS, log.prev_count);
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_add_active(&log, 900U, 2U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_j1939_dtc_clear_active(&log, 900U, 2U));
    TEST_ASSERT_EQUAL(SYN_J1939_MAX_LOGGED_DTCS, log.prev_count);
}

void run_j1939_tests(void)
{
    RUN_TEST(test_j1939_id_pack_unpack_pdu1_pdu2);
    RUN_TEST(test_j1939_name_encode_decode_roundtrip);
    RUN_TEST(test_j1939_build_frames);
    RUN_TEST(test_j1939_dm1_encoding);
    RUN_TEST(test_j1939_tp_bam_multi_packet_assembly);
    RUN_TEST(test_j1939_dm2_encoding);
    RUN_TEST(test_j1939_dtc_log_manager);
    RUN_TEST(test_j1939_dtc_log_null_checks_and_multi_packet_short_last_frame);
    RUN_TEST(test_j1939_single_frame_rx_and_dm2_encoding);
    RUN_TEST(test_j1939_pgn_filter_and_claim_conflict);
    RUN_TEST(test_r5_a15_unicast_rts_cts_eomack_and_da_dlc_filters);
    RUN_TEST(test_j1939_pro16_tp_concurrency_ext_filter_and_dm2_dedup);
}
