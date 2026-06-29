/**
 * @file test_fmt.c
 * @brief Unity tests for syn_fmt.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_fmt.h"

static void test_fmt(void)
{

    char buf[64];
    size_t n;

    /* Signed integer */
    n = syn_fmt_int(buf, sizeof(buf), 12345);
    TEST_ASSERT_EQUAL_INT(5, n);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "12345"));

    n = syn_fmt_int(buf, sizeof(buf), -42);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "-42"));

    n = syn_fmt_int(buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "0"));

    /* Unsigned */
    n = syn_fmt_uint(buf, sizeof(buf), 4294967295u);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "4294967295"));

    /* Hex */
    n = syn_fmt_hex(buf, sizeof(buf), 0xDEAD, 4);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "DEAD"));

    n = syn_fmt_hex(buf, sizeof(buf), 0x0A, 4);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "000A"));

    /* Q16.16 */
    int32_t q = Q16_FROM_FRAC(355, 113); /* ≈ π ≈ 3.141 */
    n = syn_fmt_q16(buf, sizeof(buf), q, 3);
    TEST_ASSERT_EQUAL('3', buf[0]);
    TEST_ASSERT_EQUAL('.', buf[1]);
    /* 355/113 ≈ 3.14159 → should be "3.141" */
    TEST_ASSERT_EQUAL('1', buf[2]);

    /* Negative Q16 */
    n = syn_fmt_q16(buf, sizeof(buf), Q16_FROM_INT(-7), 2);
    TEST_ASSERT_EQUAL('-', buf[0]);
    TEST_ASSERT_EQUAL('7', buf[1]);

    /* Fixed decimal */
    n = syn_fmt_fixed(buf, sizeof(buf), 12345, 3);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "12.345"));

    /* Hex dump */
    uint8_t data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    n = syn_fmt_hexdump(buf, sizeof(buf), data, 4);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "DE AD BE EF"));

    /* Concat */
    const char *parts[] = { "Hello", " ", "World" };
    n = syn_fmt_concat(buf, sizeof(buf), parts, 3);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "Hello World"));

    /* Truncation safety */
    char tiny[4];
    syn_fmt_int(tiny, sizeof(tiny), 12345);
    TEST_ASSERT_EQUAL('\0', tiny[3]);

    (void)n;
}

void run_fmt_tests(void)
{
    RUN_TEST(test_fmt);
}
