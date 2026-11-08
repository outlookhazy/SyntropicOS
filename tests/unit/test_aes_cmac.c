/**
 * @file test_aes_cmac.c
 * @brief Unit tests for AES-CMAC (RFC 4493 / NIST SP 800-38B).
 */

#include "syntropic/crypto/syn_aes_cmac.h"
#include "unity/unity.h"

#include <string.h>

/* RFC 4493 Test Key */
static const uint8_t g_rfc4493_key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                          0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

/* RFC 4493 Test Message (64 bytes) */
static const uint8_t g_rfc4493_msg[64] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};

/* Example 1: len = 0 (empty message) */
static void test_aes_cmac_rfc4493_example1_len0(void)
{
    const uint8_t expected_mac[16] = {0xbb, 0x1d, 0x69, 0x29, 0xe9, 0x59, 0x37, 0x28,
                                      0x7f, 0xa3, 0x7d, 0x12, 0x9b, 0x75, 0x67, 0x46};
    uint8_t mac[16] = {0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, NULL, 0, mac));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_mac, mac, 16);

    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, NULL, 0, expected_mac));
}

/* Example 2: len = 16 (single complete block) */
static void test_aes_cmac_rfc4493_example2_len16(void)
{
    const uint8_t expected_mac[16] = {0x07, 0x0a, 0x16, 0xb4, 0x6b, 0x4d, 0x41, 0x44,
                                      0xf7, 0x9b, 0xdd, 0x9d, 0xd0, 0x4a, 0x28, 0x7c};
    uint8_t mac[16] = {0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 16, mac));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_mac, mac, 16);

    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 16, expected_mac));
}

/* Example 3: len = 40 (multiple blocks with partial trailing block) */
static void test_aes_cmac_rfc4493_example3_len40(void)
{
    const uint8_t expected_mac[16] = {0xdf, 0xa6, 0x67, 0x47, 0xde, 0x9a, 0xe6, 0x30,
                                      0x30, 0xca, 0x32, 0x61, 0x14, 0x97, 0xc8, 0x27};
    uint8_t mac[16] = {0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 40, mac));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_mac, mac, 16);

    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 40, expected_mac));
}

/* Example 4: len = 64 (multiple complete blocks) */
static void test_aes_cmac_rfc4493_example4_len64(void)
{
    const uint8_t expected_mac[16] = {0x51, 0xf0, 0xbe, 0xbf, 0x7e, 0x3b, 0x9d, 0x92,
                                      0xfc, 0x49, 0x74, 0x17, 0x79, 0x36, 0x3c, 0xfe};
    uint8_t mac[16] = {0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 64, mac));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_mac, mac, 16);

    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 64, expected_mac));
}

/* Verify tamper detection and boundary errors */
static void test_aes_cmac_tamper_and_null_checks(void)
{
    uint8_t mac[16] = {0};
    uint8_t bad_mac[16] = {0};

    /* NULL parameter checks for syn_aes_cmac */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_aes_cmac(NULL, g_rfc4493_msg, 16, mac));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 16, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_aes_cmac(g_rfc4493_key, NULL, 16, mac));

    /* NULL parameter checks for syn_aes_cmac_verify */
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(NULL, g_rfc4493_msg, 16, mac));
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 16, NULL));
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(g_rfc4493_key, NULL, 16, mac));

    /* Valid MAC computation */
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 64, mac));

    /* Tampered MAC verification failure */
    memcpy(bad_mac, mac, 16);
    bad_mac[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 64, bad_mac));

    /* Tampered Message verification failure */
    uint8_t tampered_msg[64];
    memcpy(tampered_msg, g_rfc4493_msg, 64);
    tampered_msg[10] ^= 0x80;
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(g_rfc4493_key, tampered_msg, 64, mac));

    /* Tampered Key verification failure */
    uint8_t wrong_key[16];
    memcpy(wrong_key, g_rfc4493_key, 16);
    wrong_key[15] ^= 0x01;
    TEST_ASSERT_FALSE(syn_aes_cmac_verify(wrong_key, g_rfc4493_msg, 64, mac));
}

static void test_tier_a_aes_cmac_block_boundaries(void)
{
    uint8_t mac1[16] = {0};
    uint8_t mac15[16] = {0};
    uint8_t mac17[16] = {0};

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 1U, mac1));
    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 1U, mac1));

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 15U, mac15));
    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 15U, mac15));

    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_cmac(g_rfc4493_key, g_rfc4493_msg, 17U, mac17));
    TEST_ASSERT_TRUE(syn_aes_cmac_verify(g_rfc4493_key, g_rfc4493_msg, 17U, mac17));
}

void run_aes_cmac_tests(void)
{
    RUN_TEST(test_aes_cmac_rfc4493_example1_len0);
    RUN_TEST(test_aes_cmac_rfc4493_example2_len16);
    RUN_TEST(test_aes_cmac_rfc4493_example3_len40);
    RUN_TEST(test_aes_cmac_rfc4493_example4_len64);
    RUN_TEST(test_aes_cmac_tamper_and_null_checks);
    RUN_TEST(test_tier_a_aes_cmac_block_boundaries);
}
