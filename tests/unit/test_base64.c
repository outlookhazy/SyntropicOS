/**
 * @file test_base64.c
 * @brief Unit tests for RFC 4648 Base64 & Base64URL codec.
 */

#include "syntropic/util/syn_base64.h"
#include "unity/unity.h"

#include <string.h>

void test_base64_rfc4648_standard_test_vectors(void)
{
    /* Test vectors from RFC 4648 Section 10 */
    const char *inputs[] = {"", "f", "fo", "foo", "foob", "fooba", "foobar"};
    const char *expected[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"};

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
        char encoded[32];
        size_t enc_len = 0;
        TEST_ASSERT_TRUE(syn_base64_encode((const uint8_t *)inputs[i], strlen(inputs[i]), encoded,
                                           sizeof(encoded), &enc_len));
        TEST_ASSERT_EQUAL_STRING(expected[i], encoded);
        TEST_ASSERT_EQUAL(strlen(expected[i]), enc_len);

        /* Roundtrip decoding */
        uint8_t decoded[32];
        size_t dec_len = 0;
        TEST_ASSERT_TRUE(syn_base64_decode(encoded, enc_len, decoded, sizeof(decoded), &dec_len));
        TEST_ASSERT_EQUAL(strlen(inputs[i]), dec_len);
        if (dec_len > 0) {
            TEST_ASSERT_EQUAL_MEMORY(inputs[i], decoded, dec_len);
        }
    }
}

void test_base64url_rfc4648_test_vectors(void)
{
    /* Test vectors containing +/ vs -_ in standard vs URL safe */
    const uint8_t bin[] = {0xFB, 0xFF, 0xFE, 0x14, 0xFB, 0xE0};
    char b64_std[32];
    size_t std_len = 0;
    TEST_ASSERT_TRUE(syn_base64_encode(bin, sizeof(bin), b64_std, sizeof(b64_std), &std_len));
    TEST_ASSERT_EQUAL_STRING("+//+FPvg", b64_std);

    uint8_t std_dec[32];
    size_t std_dec_len = 0;
    TEST_ASSERT_TRUE(syn_base64_decode(b64_std, std_len, std_dec, sizeof(std_dec), &std_dec_len));
    TEST_ASSERT_EQUAL(sizeof(bin), std_dec_len);
    TEST_ASSERT_EQUAL_MEMORY(bin, std_dec, sizeof(bin));

    char b64_url[32];
    size_t url_len = 0;
    TEST_ASSERT_TRUE(
        syn_base64url_encode(bin, sizeof(bin), b64_url, sizeof(b64_url), false, &url_len));
    TEST_ASSERT_EQUAL_STRING("-__-FPvg", b64_url);

    /* Roundtrip decode unpadded Base64URL */
    uint8_t dec[32];
    size_t dec_len = 0;
    TEST_ASSERT_TRUE(syn_base64url_decode(b64_url, url_len, dec, sizeof(dec), &dec_len));
    TEST_ASSERT_EQUAL(sizeof(bin), dec_len);
    TEST_ASSERT_EQUAL_MEMORY(bin, dec, sizeof(bin));

    /* Padded Base64URL test */
    const uint8_t single[] = {0xFB};
    char b64_url_pad[16];
    TEST_ASSERT_TRUE(
        syn_base64url_encode(single, 1, b64_url_pad, sizeof(b64_url_pad), true, &url_len));
    TEST_ASSERT_EQUAL_STRING("-w==", b64_url_pad);
    TEST_ASSERT_TRUE(syn_base64url_decode(b64_url_pad, url_len, dec, sizeof(dec), &dec_len));
    TEST_ASSERT_EQUAL(1, dec_len);
    TEST_ASSERT_EQUAL(0xFB, dec[0]);

    /* Remainder 2 Base64URL test with and without padding */
    const uint8_t two_bytes[] = {0xFB, 0xFF};
    char b64_url_two[16];
    TEST_ASSERT_TRUE(
        syn_base64url_encode(two_bytes, 2, b64_url_two, sizeof(b64_url_two), false, &url_len));
    TEST_ASSERT_EQUAL_STRING("-_8", b64_url_two);
    TEST_ASSERT_TRUE(syn_base64url_decode(b64_url_two, url_len, dec, sizeof(dec), &dec_len));
    TEST_ASSERT_EQUAL(2, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(two_bytes, dec, 2);

    TEST_ASSERT_TRUE(
        syn_base64url_encode(two_bytes, 2, b64_url_two, sizeof(b64_url_two), true, &url_len));
    TEST_ASSERT_EQUAL_STRING("-_8=", b64_url_two);
    TEST_ASSERT_TRUE(syn_base64url_decode(b64_url_two, url_len, dec, sizeof(dec), &dec_len));
    TEST_ASSERT_EQUAL(2, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(two_bytes, dec, 2);
}

void test_base64_whitespace_and_newlines(void)
{
    const char input_with_ws[] = "  Z  m \r\n 9 v   \t Y m F y \n ";
    uint8_t decoded[32];
    size_t dec_len = 0;
    TEST_ASSERT_TRUE(syn_base64_decode(input_with_ws, 0, decoded, sizeof(decoded), &dec_len));
    TEST_ASSERT_EQUAL(6, dec_len);
    TEST_ASSERT_EQUAL_MEMORY("foobar", decoded, 6);
}

void test_base64_null_and_bounds_checks(void)
{
    char buf[16] = {0};
    uint8_t bin[16] = {0};
    size_t len = 0;

    /* NULL destination */
    TEST_ASSERT_FALSE(syn_base64_encode(bin, 4, NULL, 16, &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zg==", 4, NULL, 16, &len));
    TEST_ASSERT_FALSE(syn_base64_decode(NULL, 4, bin, 16, &len));
    TEST_ASSERT_FALSE(syn_base64url_encode(bin, 4, NULL, 16, false, &len));
    TEST_ASSERT_FALSE(syn_base64url_decode(NULL, 4, bin, 16, &len));

    /* Destination buffer too small */
    TEST_ASSERT_FALSE(syn_base64_encode(bin, 4, buf, 3, &len)); /* needs 9 bytes */
    TEST_ASSERT_FALSE(syn_base64url_encode(bin, 4, buf, 2, false, &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zm9vYmFy", 8, bin, 2, &len)); /* needs 6 bytes */

    /* Invalid characters */
    TEST_ASSERT_FALSE(syn_base64_decode("Zm9v!mFy", 8, bin, 16, &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zm9v@mFy", 8, bin, 16, &len));

    /* Invalid padding in middle */
    TEST_ASSERT_FALSE(syn_base64_decode("Zm=vYmFy", 8, bin, 16, &len));

    /* Helper size calculations */
    TEST_ASSERT_EQUAL(1, syn_base64_calc_encoded_size(0));
    TEST_ASSERT_EQUAL(5, syn_base64_calc_encoded_size(1));
    TEST_ASSERT_EQUAL(5, syn_base64_calc_encoded_size(2));
    TEST_ASSERT_EQUAL(5, syn_base64_calc_encoded_size(3));
    TEST_ASSERT_EQUAL(9, syn_base64_calc_encoded_size(4));

    TEST_ASSERT_TRUE(syn_base64_calc_max_decoded_size(4) >= 3);
}

void test_r5_base64_strict_padding_and_pad_bits(void)
{
    char enc[16] = {0};
    uint8_t bin[16] = {0};
    size_t len = 99U;

    /* Padding before 2 valid symbols in quartet */
    TEST_ASSERT_FALSE(syn_base64_decode("====", 4, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("=AAA", 4, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("A===", 4, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zm9v====", 8, bin, sizeof(bin), &len));

    /* Incomplete or excess '=' padding */
    TEST_ASSERT_FALSE(syn_base64_decode("Zg=", 3, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zg===", 5, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zm8==", 5, bin, sizeof(bin), &len));

    /* Non-whitespace data after '=' */
    TEST_ASSERT_FALSE(syn_base64_decode("Zg==Zg==", 8, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("Zm8=A", 5, bin, sizeof(bin), &len));

    /* Trailing single 6-bit symbol (1 mod 4 length) */
    TEST_ASSERT_FALSE(syn_base64_decode("A", 1, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("YWJjZ", 5, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64url_decode("A", 1, bin, sizeof(bin), &len));

    /* Non-zero unused pad bits on 2-symbol and 3-symbol final quartets */
    TEST_ASSERT_FALSE(syn_base64_decode("AB==", 4, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("AB", 2, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64url_decode("AB", 2, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("ABC=", 4, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64_decode("ABC", 3, bin, sizeof(bin), &len));
    TEST_ASSERT_FALSE(syn_base64url_decode("ABC", 3, bin, sizeof(bin), &len));

    /* Valid unpadded Base64URL and whitespace around padding */
    TEST_ASSERT_TRUE(syn_base64url_decode("-w", 2, bin, sizeof(bin), &len));
    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)len);
    TEST_ASSERT_EQUAL_HEX8(0xFB, bin[0]);

    TEST_ASSERT_TRUE(syn_base64_decode(" Zg== \r\n ", 0, bin, sizeof(bin), NULL));
    TEST_ASSERT_EQUAL_HEX8((uint8_t)'f', bin[0]);

    /* NULL src with non-zero length and NULL out_len on encoders */
    TEST_ASSERT_FALSE(syn_base64_encode(NULL, 3, enc, sizeof(enc), &len));
    TEST_ASSERT_FALSE(syn_base64url_encode(NULL, 3, enc, sizeof(enc), false, &len));
    TEST_ASSERT_TRUE(syn_base64_encode((const uint8_t *)"a", 1, enc, sizeof(enc), NULL));
    TEST_ASSERT_TRUE(syn_base64url_encode((const uint8_t *)"a", 1, enc, sizeof(enc), false, NULL));
}

static void test_tier_a_base64_size_overflow_guards(void)
{
    char enc[16] = {0};
    const uint8_t bin[4] = {1U, 2U, 3U, 4U};
    size_t len = 0U;

    /* UTL-06a: Overflowing lengths must return 0 / false instead of wrapping size_t */
    TEST_ASSERT_EQUAL_size_t(0U, syn_base64_calc_encoded_size(SIZE_MAX));
    TEST_ASSERT_EQUAL_size_t(0U, syn_base64_calc_encoded_size(SYN_BASE64_MAX_RAW_LEN + 1U));
    TEST_ASSERT_TRUE(syn_base64_calc_encoded_size(SYN_BASE64_MAX_RAW_LEN) > 0U);

    TEST_ASSERT_EQUAL_size_t(0U, syn_base64_calc_max_decoded_size(SIZE_MAX));
    TEST_ASSERT_EQUAL_size_t(6U, syn_base64_calc_max_decoded_size(5U));

    TEST_ASSERT_FALSE(syn_base64_encode(bin, SIZE_MAX, enc, sizeof(enc), &len));
    TEST_ASSERT_FALSE(syn_base64_encode(bin, SYN_BASE64_MAX_RAW_LEN + 1U, enc, SIZE_MAX, &len));
    TEST_ASSERT_FALSE(syn_base64url_encode(bin, SIZE_MAX, enc, SIZE_MAX, false, &len));
    TEST_ASSERT_FALSE(syn_base64url_encode(bin, SIZE_MAX, enc, SIZE_MAX, true, &len));

    /* Base64URL empty payload (rem == 0) without padding */
    TEST_ASSERT_TRUE(syn_base64url_encode(bin, 0U, enc, sizeof(enc), false, &len));
    TEST_ASSERT_EQUAL_size_t(0U, len);
    TEST_ASSERT_EQUAL_STRING("", enc);
}

void run_base64_tests(void)
{
    RUN_TEST(test_base64_rfc4648_standard_test_vectors);
    RUN_TEST(test_base64url_rfc4648_test_vectors);
    RUN_TEST(test_base64_whitespace_and_newlines);
    RUN_TEST(test_base64_null_and_bounds_checks);
    RUN_TEST(test_r5_base64_strict_padding_and_pad_bits);
    RUN_TEST(test_tier_a_base64_size_overflow_guards);
}
