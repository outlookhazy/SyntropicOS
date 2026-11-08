/**
 * @file test_pmbus.c
 * @brief Unit tests for PMBus Linear11/Linear16 format converters and command builders.
 */

#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <math.h>

void test_pmbus_linear11_roundtrip(void)
{
    float test_vals[] = {0.0f,   1.0f,    12.5f,    250.75f,  -15.25f,  1024.0f,
                         -50.0f, 0.0001f, -0.0001f, 50000.0f, -50000.0f};
    size_t count = sizeof(test_vals) / sizeof(test_vals[0]);

    for (size_t i = 0; i < count; i++) {
        float original = test_vals[i];
        uint16_t encoded = syn_pmbus_float_to_linear11(original);
        float decoded = syn_pmbus_linear11_to_float(encoded);

        if (fabsf(original) >= 0.01f) {
            float err = fabsf((decoded - original) / original);
            TEST_ASSERT_TRUE_MESSAGE(err < 0.05f, "Linear11 conversion error exceeded tolerance");
        } else {
            float diff = fabsf(decoded - original);
            TEST_ASSERT_TRUE_MESSAGE(diff < 0.001f, "Linear11 absolute error exceeded tolerance");
        }
    }

    /* Edge cases */
    TEST_ASSERT_EQUAL_HEX16(0x0000, syn_pmbus_float_to_linear11(0.0f));
}

void test_pmbus_linear16_roundtrip(void)
{
    /* VOUT_MODE = 0x18 -> Exponent N = -8 */
    uint8_t vout_mode = 0x18;

    float test_volts[] = {0.0f, 1.2f, 3.3f, 5.0f, 12.0f, 48.0f};
    size_t count = sizeof(test_volts) / sizeof(test_volts[0]);

    for (size_t i = 0; i < count; i++) {
        float original = test_volts[i];
        uint16_t encoded = syn_pmbus_float_to_linear16(original, vout_mode);
        float decoded = syn_pmbus_linear16_to_float(encoded, vout_mode);

        float diff = fabsf(decoded - original);
        TEST_ASSERT_TRUE_MESSAGE(diff < 0.01f,
                                 "Linear16 voltage conversion error exceeded tolerance");
    }

    /* Negative voltage clamping to 0 */
    TEST_ASSERT_EQUAL_HEX16(0x0000, syn_pmbus_float_to_linear16(-5.0f, vout_mode));

    /* Positive saturation clamping to 65535 */
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, syn_pmbus_float_to_linear16(1000.0f, vout_mode));

    /* Positive exponent VOUT_MODE */
    uint8_t positive_exp_mode = 0x02; /* N = +2 */
    uint16_t encoded_pos = syn_pmbus_float_to_linear16(16.0f, positive_exp_mode);
    float decoded_pos = syn_pmbus_linear16_to_float(encoded_pos, positive_exp_mode);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 16.0f, decoded_pos);
}

void test_pmbus_encode_read_cmd(void)
{
    SYN_SMBUS_Packet pkt;
    syn_pmbus_encode_read_cmd(&pkt, 0x58, SYN_PMBUS_CMD_READ_VOUT, true);

    TEST_ASSERT_EQUAL_HEX8(0x58, pkt.slave_addr);
    TEST_ASSERT_EQUAL_HEX8(0x8B, pkt.command);
    TEST_ASSERT_EQUAL(SYN_SMBUS_PROTO_READ_WORD, pkt.proto);
    TEST_ASSERT_TRUE(pkt.pec_enabled);

    /* Null pointer check */
    syn_pmbus_encode_read_cmd(NULL, 0x58, SYN_PMBUS_CMD_READ_VOUT, true);
}

void test_pmbus_status_word_decoding(void)
{
    /* Test PMBus status byte parsing (STATUS_BYTE bitmasks) */
    uint8_t status_byte =
        SYN_PMBUS_STATUS_BYTE_OFF | SYN_PMBUS_STATUS_BYTE_VOUT_OV | SYN_PMBUS_STATUS_BYTE_IOUT_OC;
    TEST_ASSERT_TRUE(status_byte & SYN_PMBUS_STATUS_BYTE_OFF);
    TEST_ASSERT_TRUE(status_byte & SYN_PMBUS_STATUS_BYTE_VOUT_OV);
    TEST_ASSERT_TRUE(status_byte & SYN_PMBUS_STATUS_BYTE_IOUT_OC);
    TEST_ASSERT_FALSE(status_byte & SYN_PMBUS_STATUS_BYTE_BUSY);

    /* Zero value conversion check */
    TEST_ASSERT_EQUAL_UINT16(0, syn_pmbus_float_to_linear11(0.0f));

    /* Mantissa overflow / underflow bounds checks */
    TEST_ASSERT_EQUAL_HEX16(0x7BFFU, syn_pmbus_float_to_linear11(1e9f));
    TEST_ASSERT_EQUAL_HEX16(0x7C00U, syn_pmbus_float_to_linear11(-1e9f));
}

void test_r5_a14_pmbus_nan_inf_and_float_clamp(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000U, syn_pmbus_float_to_linear11(NAN));
    TEST_ASSERT_EQUAL_HEX16(0x7BFFU, syn_pmbus_float_to_linear11(INFINITY));
    TEST_ASSERT_EQUAL_HEX16(0x7C00U, syn_pmbus_float_to_linear11(-INFINITY));
    TEST_ASSERT_EQUAL_HEX16(0x7BFFU, syn_pmbus_float_to_linear11(1e20f));
    TEST_ASSERT_EQUAL_HEX16(0x7C00U, syn_pmbus_float_to_linear11(-1e20f));
    TEST_ASSERT_EQUAL_HEX16(0x0000U, syn_pmbus_float_to_linear16(NAN, 0x18U));
}

void run_pmbus_tests(void)
{
    RUN_TEST(test_pmbus_linear11_roundtrip);
    RUN_TEST(test_pmbus_linear16_roundtrip);
    RUN_TEST(test_pmbus_encode_read_cmd);
    RUN_TEST(test_pmbus_status_word_decoding);
    RUN_TEST(test_r5_a14_pmbus_nan_inf_and_float_clamp);
}
