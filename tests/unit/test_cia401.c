/**
 * @file test_cia401.c
 * @brief Unity unit tests for CANopen CiA 401 Generic I/O Profile.
 */

#include "syntropic/proto/syn_cia401.h"
#include "unity/unity.h"

#include <string.h>

static SYN_CiA401_Device g_cia401_dev;
static SYN_CANOpenODEntry g_od_table[16];

void test_cia401_init(void)
{
    syn_cia401_init(&g_cia401_dev, 2, 2, 4, 2);
    TEST_ASSERT_EQUAL(2, g_cia401_dev.digital_in_count);
    TEST_ASSERT_EQUAL(2, g_cia401_dev.digital_out_count);
    TEST_ASSERT_EQUAL(4, g_cia401_dev.analog_in_count);
    TEST_ASSERT_EQUAL(2, g_cia401_dev.analog_out_count);
}

void test_cia401_populate_od(void)
{
    syn_cia401_init(&g_cia401_dev, 1, 1, 2, 1);
    size_t count = syn_cia401_populate_od(&g_cia401_dev, g_od_table, 16);

    /* 1 dig in + 1 dig out + 2 analog in + 1 analog out + 2 analog delta = 7 entries */
    TEST_ASSERT_EQUAL(7, count);
    TEST_ASSERT_EQUAL(SYN_CIA401_OD_DIGITAL_INPUT_8BIT, g_od_table[0].index);
    TEST_ASSERT_EQUAL(SYN_CIA401_OD_DIGITAL_OUTPUT_8BIT, g_od_table[1].index);
    TEST_ASSERT_EQUAL(SYN_CIA401_OD_ANALOG_INPUT_16BIT, g_od_table[2].index);
    TEST_ASSERT_EQUAL(SYN_CIA401_OD_ANALOG_OUTPUT_16BIT, g_od_table[4].index);
}

void test_cia401_analog_delta(void)
{
    syn_cia401_init(&g_cia401_dev, 0, 0, 2, 0);
    g_cia401_dev.analog_delta[0] = 50;
    g_cia401_dev.analog_in[0] = 10;
    g_cia401_dev.prev_analog_in[0] = 10;

    uint8_t ch = 0xFF;
    TEST_ASSERT_FALSE(syn_cia401_check_analog_delta(&g_cia401_dev, &ch));

    /* Exceed delta 50 */
    g_cia401_dev.analog_in[0] = 70;
    TEST_ASSERT_TRUE(syn_cia401_check_analog_delta(&g_cia401_dev, &ch));
    TEST_ASSERT_EQUAL(0, ch);

    /* Decrease by more than delta 50 (negative diff) */
    g_cia401_dev.prev_analog_in[0] = 100;
    g_cia401_dev.analog_in[0] = 10;
    TEST_ASSERT_TRUE(syn_cia401_check_analog_delta(&g_cia401_dev, &ch));
    TEST_ASSERT_EQUAL(0, ch);
}

void test_r5_a18_cia401_wide_analog_delta_no_i16_overflow(void)
{
    /* Exercise max channel clamping in init as well */
    syn_cia401_init(&g_cia401_dev, 10U, 10U, 12U, 12U);
    TEST_ASSERT_EQUAL_UINT8(SYN_CIA401_MAX_DIGITAL_8BIT, g_cia401_dev.digital_in_count);
    TEST_ASSERT_EQUAL_UINT8(SYN_CIA401_MAX_ANALOG_16BIT, g_cia401_dev.analog_in_count);

    /* Full-scale swing (-32768 -> +32767 = 65535 > INT16_MAX) must trigger cleanly */
    g_cia401_dev.analog_delta[0] = 0; /* Disabled delta on ch 0 */
    g_cia401_dev.analog_delta[1] = 1000;
    g_cia401_dev.prev_analog_in[1] = -32768;
    g_cia401_dev.analog_in[1] = 32767;

    uint8_t ch = 0xFFU;
    TEST_ASSERT_TRUE(syn_cia401_check_analog_delta(&g_cia401_dev, &ch));
    TEST_ASSERT_EQUAL_UINT8(1U, ch);
    TEST_ASSERT_EQUAL_INT16(32767, g_cia401_dev.prev_analog_in[1]);

    /* Reverse full-scale swing (+32767 -> -32768) */
    g_cia401_dev.analog_in[1] = -32768;
    TEST_ASSERT_TRUE(syn_cia401_check_analog_delta(&g_cia401_dev, &ch));
    TEST_ASSERT_EQUAL_UINT8(1U, ch);
}

void run_cia401_tests(void)
{
    RUN_TEST(test_cia401_init);
    RUN_TEST(test_cia401_populate_od);
    RUN_TEST(test_cia401_analog_delta);
    RUN_TEST(test_r5_a18_cia401_wide_analog_delta_no_i16_overflow);
}
