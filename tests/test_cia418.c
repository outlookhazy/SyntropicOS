/**
 * @file test_cia418.c
 * @brief Unit test suite for CANopen CiA 418 Battery profile (syn_cia418).
 */

#include "unity/unity.h"
#include "syntropic/proto/syn_cia418.h"
#include <string.h>

void test_cia418_init_and_od_read(void)
{
    SYN_CiA418_State state;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_init(&state));

    state.soc_0_1pct          = 850;   /* 85.0% */
    state.soh_0_1pct          = 980;   /* 98.0% */
    state.voltage_mv          = 48250; /* 48.250V */
    state.current_ma          = -15000;/* -15.000A (discharging) */
    state.temperature_0_1c    = 255;   /* 25.5°C */
    state.time_to_go_sec      = 25200; /* 7 hours */
    state.min_cell_voltage_mv = 3210;  /* 3.210V */
    state.max_cell_voltage_mv = 3450;  /* 3.450V */
    state.min_cell_temp_0_1c  = 220;   /* 22.0°C */
    state.max_cell_temp_0_1c  = 280;   /* 28.0°C */

    uint8_t data[8];
    size_t len = 0;

    /* SOC 0x6000 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_SOC, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(2, len);
    TEST_ASSERT_EQUAL_UINT16(850, (uint16_t)data[0] | ((uint16_t)data[1] << 8));

    /* SOH 0x6001 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_SOH, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(2, len);
    TEST_ASSERT_EQUAL_UINT16(980, (uint16_t)data[0] | ((uint16_t)data[1] << 8));

    /* Voltage 0x6002 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_VOLTAGE, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(4, len);
    TEST_ASSERT_EQUAL_UINT32(48250, (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24));

    /* Current 0x6003 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_CURRENT, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(4, len);
    int32_t curr_read = (int32_t)((uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24));
    TEST_ASSERT_EQUAL_INT32(-15000, curr_read);

    /* Temperature 0x6004 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_TEMPERATURE, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(2, len);
    TEST_ASSERT_EQUAL_INT16(255, (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8)));

    /* Time To Go 0x6005 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_TIME_TO_GO, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT32(4, len);

    /* Cell Min Voltage 0x6006 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_MIN_CELL_VOLTAGE, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT16(3210, (uint16_t)data[0] | ((uint16_t)data[1] << 8));

    /* Cell Max Voltage 0x6007 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_MAX_CELL_VOLTAGE, 0, data, &len));
    TEST_ASSERT_EQUAL_UINT16(3450, (uint16_t)data[0] | ((uint16_t)data[1] << 8));

    /* Cell Min Temp 0x6008 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_MIN_CELL_TEMP, 0, data, &len));
    TEST_ASSERT_EQUAL_INT16(220, (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8)));

    /* Cell Max Temp 0x6009 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cia418_od_read(&state, SYN_CIA418_OD_MAX_CELL_TEMP, 0, data, &len));
    TEST_ASSERT_EQUAL_INT16(280, (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8)));

    /* Invalid index / subindex */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cia418_od_read(&state, 0x1234, 0, data, &len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_cia418_od_read(&state, SYN_CIA418_OD_SOC, 1, data, &len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_cia418_init(NULL));
}

void run_cia418_tests(void)
{
    RUN_TEST(test_cia418_init_and_od_read);
}
