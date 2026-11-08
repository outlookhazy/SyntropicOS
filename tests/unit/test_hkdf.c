/**
 * @file test_hkdf.c
 * @brief Unit tests for HKDF-SHA256 (RFC 5869) and TLS 1.3 HKDF-Expand-Label.
 */

#include "syntropic/crypto/syn_hkdf.h"
#include "unity/unity.h"

#include <string.h>

void test_hkdf_rfc5869_case_1(void)
{
    /* RFC 5869 Test Case 1 */
    static const uint8_t ikm[22] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
    static const uint8_t salt[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                     0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    static const uint8_t info[10] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9};

    uint8_t prk[32];
    syn_hkdf_extract(salt, sizeof(salt), ikm, sizeof(ikm), prk);
    TEST_ASSERT_NOT_EQUAL(0, prk[0]);

    uint8_t okm[42];
    bool ok = syn_hkdf_expand(prk, sizeof(prk), info, sizeof(info), okm, sizeof(okm));
    TEST_ASSERT_TRUE(ok);
}

void test_hkdf_rfc5869_case_2_zero_salt(void)
{
    /* RFC 5869 Test Case 2 */
    static const uint8_t ikm[80] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
        0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
        0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};

    static const uint8_t info[80] = {
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
        0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
        0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
        0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
        0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
        0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

    uint8_t prk[32];
    syn_hkdf_extract(NULL, 0, ikm, sizeof(ikm), prk);
    TEST_ASSERT_NOT_EQUAL(0, prk[0]);

    uint8_t okm[82];
    bool ok = syn_hkdf_expand(prk, sizeof(prk), info, sizeof(info), okm, sizeof(okm));
    TEST_ASSERT_TRUE(ok);
}

void test_tls13_expand_label(void)
{
    static const uint8_t secret[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                       0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
                                       0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

    static const uint8_t context[32] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
                                        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};

    uint8_t derived[32];
    bool ok = syn_hkdf_expand_label(secret, 32, "c hs traffic", 12, context, 32, derived, 32);
    TEST_ASSERT_TRUE(ok);

    /* Repeat expansion and verify determinism */
    uint8_t derived2[32];
    ok = syn_hkdf_expand_label(secret, 32, "c hs traffic", 12, context, 32, derived2, 32);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(derived, derived2, 32);

    /* Different label produces different key */
    uint8_t derived3[32];
    ok = syn_hkdf_expand_label(secret, 32, "s hs traffic", 12, context, 32, derived3, 32);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_NOT_EQUAL(0, memcmp(derived, derived3, 32));
}

void test_hkdf_null_and_bounds_checks(void)
{
    uint8_t okm[32];
    TEST_ASSERT_FALSE(syn_hkdf_expand(NULL, 32, NULL, 0, okm, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand(okm, 16, NULL, 0, okm, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand(okm, 32, NULL, 0, NULL, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand(okm, 32, NULL, 0, okm, 0));
    TEST_ASSERT_FALSE(syn_hkdf_expand(okm, 32, NULL, 0, okm, 255 * 32 + 1));

    TEST_ASSERT_FALSE(syn_hkdf_expand_label(NULL, 32, "key", 3, NULL, 0, okm, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand_label(okm, 32, NULL, 3, NULL, 0, okm, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand_label(okm, 32, "key", 3, NULL, 0, NULL, 32));
    TEST_ASSERT_FALSE(syn_hkdf_expand_label(okm, 32, "key", 3, NULL, 0, okm, 0));

    /* Null parameter branches */
    syn_hkdf_extract(NULL, 0, NULL, 10, NULL);
    uint8_t prk_tmp[32];
    syn_hkdf_extract(NULL, 0, NULL, 10, prk_tmp);

    char long_label[300];
    memset(long_label, 'A', sizeof(long_label));
    TEST_ASSERT_FALSE(
        syn_hkdf_expand_label(okm, 32, long_label, sizeof(long_label), NULL, 0, okm, 32));

    uint8_t long_ctx[300];
    memset(long_ctx, 0xFF, sizeof(long_ctx));
    TEST_ASSERT_FALSE(
        syn_hkdf_expand_label(okm, 32, "label", 5, long_ctx, sizeof(long_ctx), okm, 32));
    TEST_ASSERT_TRUE(syn_hkdf_expand_label(okm, 32, "label", 5, NULL, 10, okm, 32));

    /* syn_hkdf wrapper */
    TEST_ASSERT_TRUE(syn_hkdf(NULL, 0, okm, 32, NULL, 10, okm, 32));
}

void test_hkdf_sha384_rfc5869_and_expand_label(void)
{
    static const uint8_t ikm[22] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
    static const uint8_t salt[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                     0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    static const uint8_t info[10] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9};

    uint8_t prk[SYN_SHA384_DIGEST_SIZE];
    syn_hkdf_sha384_extract(salt, sizeof(salt), ikm, sizeof(ikm), prk);
    TEST_ASSERT_NOT_EQUAL(0, prk[0]);

    uint8_t okm[64];
    TEST_ASSERT_TRUE(
        syn_hkdf_sha384_expand(prk, sizeof(prk), info, sizeof(info), okm, sizeof(okm)));

    /* syn_hkdf_sha384 wrapper */
    uint8_t okm2[64];
    TEST_ASSERT_TRUE(syn_hkdf_sha384(salt, sizeof(salt), ikm, sizeof(ikm), info, sizeof(info), okm2,
                                     sizeof(okm2)));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(okm, okm2, sizeof(okm));

    /* Zero salt extract */
    uint8_t prk_zero[SYN_SHA384_DIGEST_SIZE];
    syn_hkdf_sha384_extract(NULL, 0, ikm, sizeof(ikm), prk_zero);
    TEST_ASSERT_NOT_EQUAL(0, prk_zero[0]);

    /* TLS 1.3 Expand-Label SHA-384 */
    uint8_t label_out[32];
    uint8_t label_out2[32];
    uint8_t label_out3[32];
    TEST_ASSERT_TRUE(syn_hkdf_sha384_expand_label(prk, sizeof(prk), "c ap traffic", 12, info,
                                                  sizeof(info), label_out, sizeof(label_out)));
    TEST_ASSERT_TRUE(syn_hkdf_sha384_expand_label(prk, sizeof(prk), "c ap traffic", 12, info,
                                                  sizeof(info), label_out2, sizeof(label_out2)));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(label_out, label_out2, sizeof(label_out));

    TEST_ASSERT_TRUE(syn_hkdf_sha384_expand_label(prk, sizeof(prk), "s ap traffic", 12, info,
                                                  sizeof(info), label_out3, sizeof(label_out3)));
    TEST_ASSERT_NOT_EQUAL(0, memcmp(label_out, label_out3, sizeof(label_out)));
}

void test_hkdf_sha512_rfc5869_and_expand_label(void)
{
    static const uint8_t ikm[22] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
    static const uint8_t salt[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                     0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    static const uint8_t info[10] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9};

    uint8_t prk[SYN_SHA512_DIGEST_SIZE];
    syn_hkdf_sha512_extract(salt, sizeof(salt), ikm, sizeof(ikm), prk);
    TEST_ASSERT_NOT_EQUAL(0, prk[0]);

    uint8_t okm[100];
    TEST_ASSERT_TRUE(
        syn_hkdf_sha512_expand(prk, sizeof(prk), info, sizeof(info), okm, sizeof(okm)));

    /* syn_hkdf_sha512 wrapper */
    uint8_t okm2[100];
    TEST_ASSERT_TRUE(syn_hkdf_sha512(salt, sizeof(salt), ikm, sizeof(ikm), info, sizeof(info), okm2,
                                     sizeof(okm2)));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(okm, okm2, sizeof(okm));

    /* Zero salt extract */
    uint8_t prk_zero[SYN_SHA512_DIGEST_SIZE];
    syn_hkdf_sha512_extract(NULL, 0, ikm, sizeof(ikm), prk_zero);
    TEST_ASSERT_NOT_EQUAL(0, prk_zero[0]);

    /* TLS 1.3 Expand-Label SHA-512 */
    uint8_t label_out[64];
    uint8_t label_out2[64];
    uint8_t label_out3[64];
    TEST_ASSERT_TRUE(syn_hkdf_sha512_expand_label(prk, sizeof(prk), "c ap traffic", 12, info,
                                                  sizeof(info), label_out, sizeof(label_out)));
    TEST_ASSERT_TRUE(syn_hkdf_sha512_expand_label(prk, sizeof(prk), "c ap traffic", 12, info,
                                                  sizeof(info), label_out2, sizeof(label_out2)));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(label_out, label_out2, sizeof(label_out));

    TEST_ASSERT_TRUE(syn_hkdf_sha512_expand_label(prk, sizeof(prk), "s ap traffic", 12, info,
                                                  sizeof(info), label_out3, sizeof(label_out3)));
    TEST_ASSERT_NOT_EQUAL(0, memcmp(label_out, label_out3, sizeof(label_out)));
}

void test_hkdf_sha384_sha512_null_and_bounds_checks(void)
{
    uint8_t okm[48];
    /* SHA-384 bounds checks */
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand(NULL, 48, NULL, 0, okm, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand(okm, 24, NULL, 0, okm, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand(okm, 48, NULL, 0, NULL, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand(okm, 48, NULL, 0, okm, 0));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand(okm, 48, NULL, 0, okm, 255 * 48 + 1));

    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand_label(NULL, 48, "key", 3, NULL, 0, okm, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand_label(okm, 48, NULL, 3, NULL, 0, okm, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand_label(okm, 48, "key", 3, NULL, 0, NULL, 48));
    TEST_ASSERT_FALSE(syn_hkdf_sha384_expand_label(okm, 48, "key", 3, NULL, 0, okm, 0));

    syn_hkdf_sha384_extract(NULL, 0, NULL, 10, NULL);
    uint8_t prk384[48];
    syn_hkdf_sha384_extract(NULL, 0, NULL, 10, prk384);

    char long_label[300];
    memset(long_label, 'A', sizeof(long_label));
    TEST_ASSERT_FALSE(
        syn_hkdf_sha384_expand_label(okm, 48, long_label, sizeof(long_label), NULL, 0, okm, 48));

    uint8_t long_ctx[300];
    memset(long_ctx, 0xFF, sizeof(long_ctx));
    TEST_ASSERT_FALSE(
        syn_hkdf_sha384_expand_label(okm, 48, "label", 5, long_ctx, sizeof(long_ctx), okm, 48));
    TEST_ASSERT_TRUE(syn_hkdf_sha384_expand(okm, 48, NULL, 10, okm, 48));
    TEST_ASSERT_TRUE(syn_hkdf_sha384_expand_label(okm, 48, "label", 5, NULL, 10, okm, 48));

    /* SHA-512 bounds checks */
    uint8_t okm512[64];
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand(NULL, 64, NULL, 0, okm512, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand(okm512, 32, NULL, 0, okm512, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand(okm512, 64, NULL, 0, NULL, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand(okm512, 64, NULL, 0, okm512, 0));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand(okm512, 64, NULL, 0, okm512, 255 * 64 + 1));

    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(NULL, 64, "key", 3, NULL, 0, okm512, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(okm512, 64, NULL, 3, NULL, 0, okm512, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(okm512, 64, "key", 3, NULL, 0, NULL, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(okm512, 64, "key", 3, NULL, 0, okm512, 0));

    syn_hkdf_sha512_extract(NULL, 0, NULL, 10, NULL);
    uint8_t prk512[64];
    syn_hkdf_sha512_extract(NULL, 0, NULL, 10, prk512);

    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(okm512, 64, long_label, sizeof(long_label), NULL,
                                                   0, okm512, 64));
    TEST_ASSERT_FALSE(syn_hkdf_sha512_expand_label(okm512, 64, "label", 5, long_ctx,
                                                   sizeof(long_ctx), okm512, 64));
    TEST_ASSERT_TRUE(syn_hkdf_sha512_expand(okm512, 64, NULL, 10, okm512, 64));
    TEST_ASSERT_TRUE(syn_hkdf_sha512_expand_label(okm512, 64, "label", 5, NULL, 10, okm512, 64));
}

void test_tier_a_hkdf_expand_label_max_and_wraparound(void)
{
    uint8_t secret[64];
    char max_label[249];
    uint8_t max_ctx[255];
    uint8_t out[64];
    size_t ml = sizeof(max_label);
    size_t mc = sizeof(max_ctx);

    memset(secret, 0x42, sizeof(secret));
    memset(max_label, 'x', sizeof(max_label));
    memset(max_ctx, 0xAB, sizeof(max_ctx));

    /* HKDF-1: Maximum valid HkdfLabel (2 + 1 + 255 + 1 + 255 = 514 B) must not overflow stack */
    TEST_ASSERT_TRUE(syn_hkdf_expand_label(secret, 32U, max_label, ml, max_ctx, mc, out, 32U));
    bool ok384 = syn_hkdf_sha384_expand_label(secret, 48U, max_label, ml, max_ctx, mc, out, 48U);
    TEST_ASSERT_TRUE(ok384);
    bool ok512 = syn_hkdf_sha512_expand_label(secret, 64U, max_label, ml, max_ctx, mc, out, 64U);
    TEST_ASSERT_TRUE(ok512);

    /* HKDF-1: Reject out_len > 0xFFFFU before uint16 truncation */
    TEST_ASSERT_FALSE(syn_hkdf_expand_label(secret, 32U, "key", 3U, NULL, 0U, out, 0x10000U));
    ok384 = syn_hkdf_sha384_expand_label(secret, 48U, "key", 3U, NULL, 0U, out, 0x10000U);
    TEST_ASSERT_FALSE(ok384);
    ok512 = syn_hkdf_sha512_expand_label(secret, 64U, "key", 3U, NULL, 0U, out, 0x10000U);
    TEST_ASSERT_FALSE(ok512);

    /* HKDF-2: Reject label_len near SIZE_MAX without size_t wraparound */
    TEST_ASSERT_FALSE(syn_hkdf_expand_label(secret, 32U, "key", SIZE_MAX, NULL, 0U, out, 32U));
    bool ok256 = syn_hkdf_expand_label(secret, 32U, "key", SIZE_MAX - 5U, NULL, 0U, out, 32U);
    TEST_ASSERT_FALSE(ok256);
    ok384 = syn_hkdf_sha384_expand_label(secret, 48U, "key", SIZE_MAX, NULL, 0U, out, 48U);
    TEST_ASSERT_FALSE(ok384);
    ok512 = syn_hkdf_sha512_expand_label(secret, 64U, "key", SIZE_MAX, NULL, 0U, out, 64U);
    TEST_ASSERT_FALSE(ok512);
}

void run_hkdf_tests(void)
{
    RUN_TEST(test_hkdf_rfc5869_case_1);
    RUN_TEST(test_hkdf_rfc5869_case_2_zero_salt);
    RUN_TEST(test_tls13_expand_label);
    RUN_TEST(test_hkdf_null_and_bounds_checks);
    RUN_TEST(test_hkdf_sha384_rfc5869_and_expand_label);
    RUN_TEST(test_hkdf_sha512_rfc5869_and_expand_label);
    RUN_TEST(test_hkdf_sha384_sha512_null_and_bounds_checks);
    RUN_TEST(test_tier_a_hkdf_expand_label_max_and_wraparound);
}
