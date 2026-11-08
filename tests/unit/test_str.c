/**
 * @file test_str.c
 * @brief Unit tests for syn_str string parsing, trimming, and tokenization utilities.
 */

#include "syntropic/util/syn_str.h"
#include "unity/unity.h"

#include <limits.h>
#include <string.h>

static void test_str_split_basic_and_checksum(void)
{
    char buf1[] = "alpha,beta,gamma";
    char *tokens[4] = {NULL};

    size_t n = syn_str_split(buf1, ',', tokens, 4U);
    TEST_ASSERT_EQUAL_UINT32(3U, (uint32_t)n);
    TEST_ASSERT_EQUAL_STRING("alpha", tokens[0]);
    TEST_ASSERT_EQUAL_STRING("beta", tokens[1]);
    TEST_ASSERT_EQUAL_STRING("gamma", tokens[2]);

    /* NMEA '*' checksum terminator strips checksum suffix */
    char nmea[] = "$GPGGA,123519,4807.038,N*47";
    char *ntok[6] = {NULL};
    n = syn_str_split(nmea, ',', ntok, 6U);
    TEST_ASSERT_EQUAL_UINT32(4U, (uint32_t)n);
    TEST_ASSERT_EQUAL_STRING("$GPGGA", ntok[0]);
    TEST_ASSERT_EQUAL_STRING("123519", ntok[1]);
    TEST_ASSERT_EQUAL_STRING("4807.038", ntok[2]);
    TEST_ASSERT_EQUAL_STRING("N", ntok[3]);

    /* max_tokens cap stops splitting once array is full */
    char capped[] = "one:two:three";
    char *ctok[2] = {NULL};
    n = syn_str_split(capped, ':', ctok, 2U);
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)n);
    TEST_ASSERT_EQUAL_STRING("one", ctok[0]);
    TEST_ASSERT_EQUAL_STRING("two:three", ctok[1]);
}

static void test_str_split_null_and_empty(void)
{
    char sample[] = "abc";
    char *tokens[2] = {NULL};

    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_str_split(NULL, ',', tokens, 2U));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_str_split(sample, ',', NULL, 2U));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_str_split(sample, ',', tokens, 0U));

    char empty[] = "";
    size_t n = syn_str_split(empty, ',', tokens, 2U);
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)n);
    TEST_ASSERT_EQUAL_STRING("", tokens[0]);
}

static void test_str_trim_cases(void)
{
    TEST_ASSERT_NULL(syn_str_trim(NULL));

    char both[] = " \t\r\n hello world \t\n ";
    char *res = syn_str_trim(both);
    TEST_ASSERT_EQUAL_STRING("hello world", res);

    char single[] = "x";
    TEST_ASSERT_EQUAL_STRING("x", syn_str_trim(single));

    char all_ws[] = "   \t\r\n  ";
    res = syn_str_trim(all_ws);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("", res);

    char empty[] = "";
    res = syn_str_trim(empty);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("", res);
}

static void test_str_to_i32_cases(void)
{
    int32_t val = 0;

    TEST_ASSERT_FALSE(syn_str_to_i32(NULL, &val));
    TEST_ASSERT_FALSE(syn_str_to_i32("", &val));
    TEST_ASSERT_FALSE(syn_str_to_i32("123", NULL));
    TEST_ASSERT_FALSE(syn_str_to_i32("abc", &val));
    TEST_ASSERT_FALSE(syn_str_to_i32("99999999999999999999999999999", &val));
    TEST_ASSERT_FALSE(syn_str_to_i32("3000000000", &val));
    TEST_ASSERT_FALSE(syn_str_to_i32("-3000000000", &val));

    TEST_ASSERT_TRUE(syn_str_to_i32("0", &val));
    TEST_ASSERT_EQUAL_INT32(0, val);

    TEST_ASSERT_TRUE(syn_str_to_i32("  -42", &val));
    TEST_ASSERT_EQUAL_INT32(-42, val);

    TEST_ASSERT_TRUE(syn_str_to_i32("2147483647", &val));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val);

    TEST_ASSERT_TRUE(syn_str_to_i32("-2147483648", &val));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val);
}

static void test_str_to_u32_cases(void)
{
    uint32_t val = 0U;

    TEST_ASSERT_FALSE(syn_str_to_u32(NULL, &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("123", NULL));
    TEST_ASSERT_FALSE(syn_str_to_u32("xyz", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("   ", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("-1", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("  -99", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("99999999999999999999999999999", &val));
    TEST_ASSERT_FALSE(syn_str_to_u32("5000000000", &val));

    TEST_ASSERT_TRUE(syn_str_to_u32("0", &val));
    TEST_ASSERT_EQUAL_UINT32(0U, val);

    TEST_ASSERT_TRUE(syn_str_to_u32("  12345", &val));
    TEST_ASSERT_EQUAL_UINT32(12345U, val);

    TEST_ASSERT_TRUE(syn_str_to_u32("4294967295", &val));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, val);
}

static void test_r5_str_trailing_garbage_rejected(void)
{
    int32_t i_val = 77;
    uint32_t u_val = 88U;

    TEST_ASSERT_FALSE(syn_str_to_i32("123xyz", &i_val));
    TEST_ASSERT_FALSE(syn_str_to_i32("-42abc", &i_val));
    TEST_ASSERT_FALSE(syn_str_to_i32("123 ", &i_val));
    TEST_ASSERT_FALSE(syn_str_to_i32("0x10", &i_val));
    TEST_ASSERT_FALSE(syn_str_to_i32("12.34", &i_val));
    TEST_ASSERT_EQUAL_INT32(77, i_val);

    TEST_ASSERT_FALSE(syn_str_to_u32("123xyz", &u_val));
    TEST_ASSERT_FALSE(syn_str_to_u32("456 ", &u_val));
    TEST_ASSERT_FALSE(syn_str_to_u32("0x10", &u_val));
    TEST_ASSERT_FALSE(syn_str_to_u32("99u", &u_val));
    TEST_ASSERT_EQUAL_UINT32(88U, u_val);
}

void run_str_tests(void)
{
    RUN_TEST(test_str_split_basic_and_checksum);
    RUN_TEST(test_str_split_null_and_empty);
    RUN_TEST(test_str_trim_cases);
    RUN_TEST(test_str_to_i32_cases);
    RUN_TEST(test_str_to_u32_cases);
    RUN_TEST(test_r5_str_trailing_garbage_rejected);
}
