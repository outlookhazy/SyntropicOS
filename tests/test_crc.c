/**
 * @file test_crc.c
 * @brief Unity tests for syn_crc.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_crc.h"

static void test_crc(void)
{

    const uint8_t test_data[] = "123456789";
    size_t len = 9;

    /* CRC-16 CCITT — standard check value is 0x29B1 */
    uint16_t crc16 = syn_crc16_ccitt(test_data, len);
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc16);

    /* CRC-16 Modbus — standard check value is 0x4B37 */
    uint16_t crcmod = syn_crc16_modbus(test_data, len);
    TEST_ASSERT_EQUAL_HEX16(0x4B37, crcmod);

    /* CRC-32 — standard check value is 0xCBF43926 */
    uint32_t crc32 = syn_crc32(test_data, len);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926u, crc32);

    /* Incremental CRC-16 */
    uint16_t inc = SYN_CRC16_CCITT_INIT;
    inc = syn_crc16_ccitt_update(inc, test_data, 5);
    inc = syn_crc16_ccitt_update(inc, test_data + 5, 4);
    TEST_ASSERT_EQUAL_HEX16(0x29B1, inc);

    /* Empty data */
    TEST_ASSERT_EQUAL_HEX32(0x00000000u, syn_crc32(NULL, 0));
}

void run_crc_tests(void)
{
    RUN_TEST(test_crc);
}
