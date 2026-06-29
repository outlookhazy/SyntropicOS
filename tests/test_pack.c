/**
 * @file test_pack.c
 * @brief Unity tests for syn_pack.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_pack.h"

static void test_pack(void)
{
    uint8_t buf[32];
    size_t pos = 0;

    /* Big-endian pack */
    syn_pack_u8(buf, &pos, 0x42);
    syn_pack_u16(buf, &pos, 0x1234);
    syn_pack_i32(buf, &pos, -12345);
    TEST_ASSERT_EQUAL_size_t(7, pos);

    /* Big-endian unpack */
    pos = 0;
    TEST_ASSERT_EQUAL_HEX8(0x42, syn_unpack_u8(buf, &pos));
    TEST_ASSERT_EQUAL_HEX16(0x1234, syn_unpack_u16(buf, &pos));
    TEST_ASSERT_EQUAL_INT(-12345, syn_unpack_i32(buf, &pos));

    /* Little-endian */
    pos = 0;
    syn_pack_u16_le(buf, &pos, 0xABCD);
    syn_pack_u32_le(buf, &pos, 0x12345678);
    pos = 0;
    TEST_ASSERT_EQUAL_HEX16(0xABCD, syn_unpack_u16_le(buf, &pos));
    TEST_ASSERT_EQUAL_HEX32(0x12345678, syn_unpack_u32_le(buf, &pos));

    /* Bytes roundtrip */
    uint8_t src[] = {1, 2, 3, 4};
    uint8_t dst[4] = {0};
    pos = 0;
    syn_pack_bytes(buf, &pos, src, 4);
    pos = 0;
    syn_unpack_bytes(buf, &pos, dst, 4);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(src, dst, 4);

    /* Signed types */
    pos = 0;
    syn_pack_i8(buf, &pos, -42);
    syn_pack_i16(buf, &pos, -1000);
    pos = 0;
    TEST_ASSERT_EQUAL_INT(-42, syn_unpack_i8(buf, &pos));
    TEST_ASSERT_EQUAL_INT(-1000, syn_unpack_i16(buf, &pos));

    /* Peek */
    pos = 0;
    syn_pack_u16(buf, &pos, 0xBEEF);
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, syn_peek_u16(buf, 0));
}

void run_pack_tests(void)
{
    RUN_TEST(test_pack);
}
