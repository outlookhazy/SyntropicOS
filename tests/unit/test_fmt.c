/**
 * @file test_fmt.c
 * @brief Unity tests for syn_fmt.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_fmt.h"
#include "unity/unity.h"

static void test_fmt(void)
{
    char buf[64];
    size_t n;

    /* Signed integer */
    n = syn_fmt_int(buf, sizeof(buf), 12345);
    TEST_ASSERT_EQUAL_INT(5, n);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "12345"));

    n = syn_fmt_int(buf, sizeof(buf), -42);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "-42"));

    n = syn_fmt_int(buf, sizeof(buf), 0);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "0"));

    /* Unsigned */
    n = syn_fmt_uint(buf, sizeof(buf), 4294967295u);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "4294967295"));

    /* Hex */
    n = syn_fmt_hex(buf, sizeof(buf), 0xDEAD, 4);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "DEAD"));

    n = syn_fmt_hex(buf, sizeof(buf), 0x0A, 4);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "000A"));

    /* Q16.16 */
    int32_t q = Q16_FROM_FRAC(355, 113); /* ≈ π ≈ 3.141 */
    n = syn_fmt_q16(buf, sizeof(buf), q, 3);
    (void)n;
    TEST_ASSERT_EQUAL('3', buf[0]);
    TEST_ASSERT_EQUAL('.', buf[1]);
    /* 355/113 ≈ 3.14159 → should be "3.141" */
    TEST_ASSERT_EQUAL('1', buf[2]);

    /* Negative Q16 */
    n = syn_fmt_q16(buf, sizeof(buf), Q16_FROM_INT(-7), 2);
    (void)n;
    TEST_ASSERT_EQUAL('-', buf[0]);
    TEST_ASSERT_EQUAL('7', buf[1]);

    /* Fixed decimal */
    n = syn_fmt_fixed(buf, sizeof(buf), 12345, 3);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "12.345"));

    /* Hex dump */
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    n = syn_fmt_hexdump(buf, sizeof(buf), data, 4);
    (void)n;
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "DE AD BE EF"));

    /* Concat */
    const char *parts[] = {"Hello", " ", "World"};
    n = syn_fmt_concat(buf, sizeof(buf), parts, 3);
    TEST_ASSERT_EQUAL_INT(0, strcmp(buf, "Hello World"));

    /* Truncation safety */
    char tiny[4];
    syn_fmt_int(tiny, sizeof(tiny), 12345);
    TEST_ASSERT_EQUAL('\0', tiny[3]);

    (void)n;
}

/** syn_fmt_hex(0, 0) — exercises line 131 (min_digits forced to 1) */
static void test_fmt_hex_zero(void)
{
    char buf[16];
    size_t n = syn_fmt_hex(buf, sizeof(buf), 0, 0);
    TEST_ASSERT_EQUAL_STRING("0", buf);
    TEST_ASSERT_EQUAL_size_t(1, n);
}

/** syn_fmt_hex truncation — exercises line 146 */
static void test_fmt_hex_truncation(void)
{
    char buf[3]; /* only 2 chars + NUL */
    syn_fmt_hex(buf, sizeof(buf), 0xABCD, 0);
    TEST_ASSERT_EQUAL('\0', buf[2]); /* NUL-terminated */
}

/** syn_fmt_q16 with fractional-only value (<1.0) — exercises line 175 */
static void test_fmt_q16_fractional(void)
{
    char buf[32];
    /* 0.5 in Q16.16 = 0x00008000 */
    int32_t half = (int32_t)0x00008000;
    syn_fmt_q16(buf, sizeof(buf), half, 4);
    /* Should start with "0." */
    TEST_ASSERT_EQUAL('0', buf[0]);
    TEST_ASSERT_EQUAL('.', buf[1]);
}

/** syn_fmt_fixed with negative value — exercises lines 248-249 */
static void test_fmt_fixed_negative(void)
{
    char buf[32];
    syn_fmt_fixed(buf, sizeof(buf), -1234, 2);
    /* -1234 with 2 decimal places = "-12.34" */
    TEST_ASSERT_EQUAL('-', buf[0]);
    TEST_ASSERT_TRUE(strlen(buf) > 1);
}

/** syn_fmt_fixed with value < divisor (int_part == 0) — exercises line 264 */
static void test_fmt_fixed_small(void)
{
    char buf[32];
    /* val=5, places=2 → int_part=0, frac=05 → "0.05" */
    syn_fmt_fixed(buf, sizeof(buf), 5, 2);
    TEST_ASSERT_EQUAL_STRING("0.05", buf);
}

/** syn_fmt_uint truncation — exercises line 73 (buf full) */
static void test_fmt_str_truncation(void)
{
    char buf[4];                                /* room for 3 chars + NUL */
    syn_fmt_uint(buf, sizeof(buf), 123456789u); /* 9 digits won't fit */
    TEST_ASSERT_EQUAL('\0', buf[3]);
    TEST_ASSERT_EQUAL('1', buf[0]);
    TEST_ASSERT_EQUAL('2', buf[1]);
    TEST_ASSERT_EQUAL('3', buf[2]);
}

/** syn_fmt_q16 truncation — exercises line 208 */
static void test_fmt_q16_truncation(void)
{
    char buf[4]; /* very small */
    /* 3.14159 in Q16.16 */
    int32_t pi = (int32_t)((314159L * 65536L) / 100000L);
    syn_fmt_q16(buf, sizeof(buf), pi, 2);
    TEST_ASSERT_EQUAL('\0', buf[3]);
}

/** syn_fmt_hexdump truncation — exercises line 232 */
static void test_fmt_hexdump_truncation(void)
{
    char buf[4]; /* room for only 1 byte "XX" + space + NUL */
    uint8_t data[] = {0xAB, 0xCD, 0xEF};
    syn_fmt_hexdump(buf, sizeof(buf), data, sizeof(data));
    TEST_ASSERT_EQUAL('\0', buf[3]);
}

/** syn_fmt_fixed truncation — exercises line 291 */
static void test_fmt_fixed_truncation(void)
{
    char buf[3]; /* tiny */
    syn_fmt_fixed(buf, sizeof(buf), 1234567, 2);
    TEST_ASSERT_EQUAL('\0', buf[2]);
}

/** syn_fmt_concat truncation — exercises line 313 */
static void test_fmt_concat_truncation(void)
{
    char buf[5];
    const char *parts[] = {"Hello", " ", "World"};
    syn_fmt_concat(buf, sizeof(buf), parts, 3);
    TEST_ASSERT_EQUAL('\0', buf[4]);
}

static void test_fmt_hex_parse_and_bounds(void)
{
    char buf[32];
    uint8_t bin[4];

    /* Upper case hex parse */
    TEST_ASSERT_EQUAL_size_t(2, syn_fmt_hex_parse("DEAD", bin, 4));
    TEST_ASSERT_EQUAL_HEX8(0xDE, bin[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAD, bin[1]);

    /* min_digits > 8 capped at 8 */
    syn_fmt_hex(buf, sizeof(buf), 0x123, 16);
    TEST_ASSERT_EQUAL_STRING("00000123", buf);

    /* frac_digits > 6 capped at 6 */
    syn_fmt_q16(buf, sizeof(buf), Q16_ONE, 10);

    /* Null & zero checks */
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hex_parse(NULL, bin, 4));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hex_parse("FF", NULL, 4));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hex_parse("FF", bin, 0));

    /* Non-hex character parse return -1 (line 175 of syn_fmt.c) */
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hex_parse("GZ", bin, 1));

    /* places > 9 clamp (line 294 of syn_fmt.c) */
    syn_fmt_fixed(buf, sizeof(buf), 123, 10);

    /* Uppercase prefix & NULL checks in syn_str_prefix_icase (line 130 of syn_fmt.h) */
    TEST_ASSERT_TRUE(syn_str_prefix_icase("foobar", "FOO"));
    TEST_ASSERT_FALSE(syn_str_prefix_icase(NULL, "FOO"));
    TEST_ASSERT_FALSE(syn_str_prefix_icase("foobar", NULL));

    /* Zero size buffer parameter guards */
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_uint(buf, 0, 100));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_int(buf, 0, -100));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hex(buf, 0, 0xAA, 2));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_q16(buf, 0, Q16_ONE, 2));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hexdump(buf, 0, bin, 2));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_hexdump(buf, 10, NULL, 0));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_fixed(buf, 0, 100, 2));
    TEST_ASSERT_EQUAL_size_t(0, syn_fmt_concat(buf, 0, NULL, 0));
}

static void test_r5_fmt_null_and_zero_buf(void)
{
    char buf[32];
    const uint8_t data[2] = {0x12U, 0x34U};
    const char *parts_with_null[3] = {"Hi", NULL, "!"};

    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_uint(NULL, 16U, 42U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_int(NULL, 16U, -42));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_hex(NULL, 16U, 0xBEEFU, 4U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_q16(NULL, 16U, Q16_ONE, 2U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_hexdump(NULL, 16U, data, 2U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_fixed(NULL, 16U, 123, 2U));
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_concat(NULL, 16U, parts_with_null, 3U));

    /* NULL input arrays with non-zero count */
    buf[0] = 'X';
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_hexdump(buf, sizeof(buf), NULL, 2U));
    TEST_ASSERT_EQUAL_CHAR('\0', buf[0]);

    buf[0] = 'X';
    TEST_ASSERT_EQUAL_size_t(0U, syn_fmt_concat(buf, sizeof(buf), NULL, 2U));
    TEST_ASSERT_EQUAL_CHAR('\0', buf[0]);

    /* Concat with NULL fragment skipped */
    TEST_ASSERT_EQUAL_size_t(3U, syn_fmt_concat(buf, sizeof(buf), parts_with_null, 3U));
    TEST_ASSERT_EQUAL_STRING("Hi!", buf);

    /* Zero fractional places and INT32_MIN edge cases */
    TEST_ASSERT_EQUAL_size_t(1U, syn_fmt_uint(buf, sizeof(buf), 0U));
    TEST_ASSERT_EQUAL_STRING("0", buf);
    TEST_ASSERT_EQUAL_size_t(11U, syn_fmt_int(buf, sizeof(buf), INT32_MIN));
    TEST_ASSERT_EQUAL_STRING("-2147483648", buf);
    TEST_ASSERT_EQUAL_size_t(1U, syn_fmt_q16(buf, sizeof(buf), Q16_FROM_INT(5), 0U));
    TEST_ASSERT_EQUAL_STRING("5", buf);
    TEST_ASSERT_EQUAL_size_t(2U, syn_fmt_fixed(buf, sizeof(buf), 42, 0U));
    TEST_ASSERT_EQUAL_STRING("42", buf);

    /* syn_fmt_parse_uint NULL guard and valid parse */
    TEST_ASSERT_EQUAL_UINT32(0U, syn_fmt_parse_uint(NULL));
    TEST_ASSERT_EQUAL_UINT32(4290U, syn_fmt_parse_uint("4290xyz"));
}

void run_fmt_tests(void)
{
    RUN_TEST(test_fmt);
    RUN_TEST(test_fmt_hex_zero);
    RUN_TEST(test_fmt_hex_truncation);
    RUN_TEST(test_fmt_q16_fractional);
    RUN_TEST(test_fmt_fixed_negative);
    RUN_TEST(test_fmt_fixed_small);
    RUN_TEST(test_fmt_str_truncation);
    RUN_TEST(test_fmt_q16_truncation);
    RUN_TEST(test_fmt_hexdump_truncation);
    RUN_TEST(test_fmt_fixed_truncation);
    RUN_TEST(test_fmt_concat_truncation);
    RUN_TEST(test_fmt_hex_parse_and_bounds);
    RUN_TEST(test_r5_fmt_null_and_zero_buf);
}
