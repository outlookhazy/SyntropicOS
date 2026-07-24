/**
 * @file test_n2k.c
 * @brief Unit test suite for NMEA 2000 marine CAN protocol engine (syn_n2k).
 */

#include "unity/unity.h"
#include "syntropic/proto/syn_n2k.h"
#include <string.h>

void test_n2k_position_rapid_encode_decode(void)
{
    SYN_N2K_PositionRapid pos_tx = {
        .latitude_1e7  = 377749290,  /* +37.7749290 N */
        .longitude_1e7 = -1224194155 /* -122.4194155 W */
    };
    SYN_CAN_Frame frame;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_encode_position_rapid(0x05, &pos_tx, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_TRUE(frame.extended);

    SYN_N2K_PositionRapid pos_rx;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_decode_position_rapid(&frame, &pos_rx));
    TEST_ASSERT_EQUAL_INT32(pos_tx.latitude_1e7, pos_rx.latitude_1e7);
    TEST_ASSERT_EQUAL_INT32(pos_tx.longitude_1e7, pos_rx.longitude_1e7);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_n2k_encode_position_rapid(0x05, NULL, &frame));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_n2k_decode_position_rapid(NULL, &pos_rx));
}

void test_n2k_cog_sog_rapid_encode_decode(void)
{
    SYN_N2K_CogSogRapid cs_tx = {
        .sid          = 42,
        .cog_ref      = 0,     /* True */
        .cog_rad_1e4  = 31415, /* ~3.1415 rad (180 deg) */
        .sog_m_s_1e2  = 1250   /* 12.50 m/s (~24.3 knots) */
    };
    SYN_CAN_Frame frame;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_encode_cog_sog_rapid(0x12, &cs_tx, &frame));
    TEST_ASSERT_EQUAL_UINT8(8, frame.dlc);
    TEST_ASSERT_TRUE(frame.extended);

    SYN_N2K_CogSogRapid cs_rx;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_decode_cog_sog_rapid(&frame, &cs_rx));
    TEST_ASSERT_EQUAL_UINT8(cs_tx.sid, cs_rx.sid);
    TEST_ASSERT_EQUAL_UINT8(cs_tx.cog_ref, cs_rx.cog_ref);
    TEST_ASSERT_EQUAL_UINT16(cs_tx.cog_rad_1e4, cs_rx.cog_rad_1e4);
    TEST_ASSERT_EQUAL_UINT16(cs_tx.sog_m_s_1e2, cs_rx.sog_m_s_1e2);
}

void test_n2k_heading_and_battery_status(void)
{
    /* Heading test */
    SYN_N2K_VesselHeading h_tx = {
        .sid               = 1,
        .heading_rad_1e4   = 15708, /* 90 deg */
        .deviation_rad_1e4 = -100,
        .variation_rad_1e4 = 250,
        .heading_ref       = 1 /* Magnetic */
    };
    SYN_CAN_Frame frame_h;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_encode_heading(0x01, &h_tx, &frame_h));

    SYN_N2K_VesselHeading h_rx;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_decode_heading(&frame_h, &h_rx));
    TEST_ASSERT_EQUAL_UINT16(h_tx.heading_rad_1e4, h_rx.heading_rad_1e4);
    TEST_ASSERT_EQUAL_INT16(h_tx.deviation_rad_1e4, h_rx.deviation_rad_1e4);
    TEST_ASSERT_EQUAL_INT16(h_tx.variation_rad_1e4, h_rx.variation_rad_1e4);

    /* Battery status test */
    SYN_N2K_BatteryStatus b_tx = {
        .instance        = 0,
        .voltage_1e2     = 1380, /* 13.80V */
        .current_1e1     = 250,  /* 25.0A */
        .temperature_1e1 = 2982, /* 25.0 C (298.2K) */
        .sid             = 10
    };
    SYN_CAN_Frame frame_b;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_encode_battery(0x01, &b_tx, &frame_b));

    SYN_N2K_BatteryStatus b_rx;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_decode_battery(&frame_b, &b_rx));
    TEST_ASSERT_EQUAL_UINT8(b_tx.instance, b_rx.instance);
    TEST_ASSERT_EQUAL_UINT16(b_tx.voltage_1e2, b_rx.voltage_1e2);
    TEST_ASSERT_EQUAL_INT16(b_tx.current_1e1, b_rx.current_1e1);
    TEST_ASSERT_EQUAL_UINT16(b_tx.temperature_1e1, b_rx.temperature_1e1);
}

void test_n2k_environmental_parameters(void)
{
    SYN_N2K_EnvParams env_tx = {
        .sid             = 3,
        .water_temp_1e2  = 29315, /* 20.0 C */
        .air_temp_1e2    = 29815, /* 25.0 C */
        .pressure_pa_1e2 = 10132  /* 1013.2 hPa */
    };
    SYN_CAN_Frame frame;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_encode_environment(0x01, &env_tx, &frame));

    SYN_N2K_EnvParams env_rx;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_decode_environment(&frame, &env_rx));
    TEST_ASSERT_EQUAL_UINT16(env_tx.water_temp_1e2, env_rx.water_temp_1e2);
    TEST_ASSERT_EQUAL_UINT16(env_tx.air_temp_1e2, env_rx.air_temp_1e2);
    TEST_ASSERT_EQUAL_UINT16(env_tx.pressure_pa_1e2, env_rx.pressure_pa_1e2);
}

void test_n2k_fast_packet_reassembly(void)
{
    SYN_N2K_FastPacketRx rx;
    memset(&rx, 0, sizeof(rx));

    uint32_t target_pgn = 129029U; /* GNSS Position Data */
    uint32_t can_id = syn_j1939_id_pack(6, target_pgn, 0x10, SYN_J1939_ADDR_GLOBAL);

    /* Frame 0 (First Frame: seq_id 1, index 0, total 18 bytes) */
    SYN_CAN_Frame f0 = { .id = can_id, .dlc = 8, .extended = true };
    f0.data[0] = (1U << 5) | 0U; /* seq_id=1, index=0 */
    f0.data[1] = 18;             /* total 18 bytes */
    memcpy(&f0.data[2], "HELLO!", 6);

    const uint8_t *payload = NULL;
    size_t len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_n2k_fastpacket_process(&rx, &f0, target_pgn, &payload, &len));

    /* Frame 1 (Second Frame: seq_id 1, index 1) */
    SYN_CAN_Frame f1 = { .id = can_id, .dlc = 8, .extended = true };
    f1.data[0] = (1U << 5) | 1U; /* seq_id=1, index=1 */
    memcpy(&f1.data[1], " WORLD!", 7);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_n2k_fastpacket_process(&rx, &f1, target_pgn, &payload, &len));

    /* Frame 2 (Third Frame: seq_id 1, index 2, final 5 bytes) */
    SYN_CAN_Frame f2 = { .id = can_id, .dlc = 8, .extended = true };
    f2.data[0] = (1U << 5) | 2U; /* seq_id=1, index=2 */
    memcpy(&f2.data[1], "N2K!!", 5);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_n2k_fastpacket_process(&rx, &f2, target_pgn, &payload, &len));

    TEST_ASSERT_EQUAL_UINT32(18, len);
    TEST_ASSERT_NOT_NULL(payload);
    TEST_ASSERT_EQUAL_MEMORY("HELLO! WORLD!N2K!!", payload, 18);

    /* Null invalid params */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_n2k_fastpacket_process(NULL, &f2, target_pgn, &payload, &len));
}

void run_n2k_tests(void)
{
    RUN_TEST(test_n2k_position_rapid_encode_decode);
    RUN_TEST(test_n2k_cog_sog_rapid_encode_decode);
    RUN_TEST(test_n2k_heading_and_battery_status);
    RUN_TEST(test_n2k_environmental_parameters);
    RUN_TEST(test_n2k_fast_packet_reassembly);
}
