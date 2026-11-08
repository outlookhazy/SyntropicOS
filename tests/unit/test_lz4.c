#include "syntropic/util/syn_lz4.h"
#include "unity/unity.h"

#include <stdint.h>
#include <string.h>

static SYN_Lz4 g_lz4;

void test_lz4_init_and_null_checks(void)
{
    uint8_t src[16] = {0};
    uint8_t dst[64] = {0};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_lz4_init(NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_lz4_init(&g_lz4));

    TEST_ASSERT_EQUAL(0, syn_lz4_compress_bound(SIZE_MAX - 10));
    TEST_ASSERT_TRUE(syn_lz4_compress_bound(100) > 100);

    TEST_ASSERT_EQUAL(0, syn_lz4_compress(NULL, src, sizeof(src), dst, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_compress(&g_lz4, NULL, sizeof(src), dst, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_compress(&g_lz4, src, sizeof(src), NULL, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_compress(&g_lz4, src, 0, dst, sizeof(dst)));

    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(NULL, sizeof(src), dst, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(src, sizeof(src), NULL, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(src, 0, dst, sizeof(dst)));
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(src, sizeof(src), dst, 0));
}

void test_lz4_repetitive_text_roundtrip(void)
{
    const char *input = "SyntropicOS SyntropicOS SyntropicOS SyntropicOS SyntropicOS "
                        "SyntropicOS SyntropicOS SyntropicOS SyntropicOS SyntropicOS";
    size_t in_len = strlen(input);

    uint8_t compressed[256];
    uint8_t decompressed[256];

    size_t comp_size = syn_lz4_compress(&g_lz4, input, in_len, compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);
    TEST_ASSERT_TRUE(comp_size < in_len);

    size_t decomp_size =
        syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(in_len, decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(input, decompressed, in_len);
}

void test_lz4_small_payload(void)
{
    const char *small = "Abc";
    size_t in_len = strlen(small);

    uint8_t compressed[32];
    uint8_t decompressed[32];

    size_t comp_size = syn_lz4_compress(&g_lz4, small, in_len, compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    size_t decomp_size =
        syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(in_len, decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(small, decompressed, in_len);
}

void test_lz4_binary_telemetry_roundtrip(void)
{
    uint8_t telemetry[128];
    for (size_t i = 0; i < sizeof(telemetry); i++) {
        telemetry[i] = (uint8_t)(i % 16);
    }

    uint8_t compressed[256];
    uint8_t decompressed[256];

    size_t comp_size =
        syn_lz4_compress(&g_lz4, telemetry, sizeof(telemetry), compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    size_t decomp_size =
        syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(sizeof(telemetry), decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(telemetry, decompressed, sizeof(telemetry));
}

void test_lz4_ultra_long_literal_and_match_roundtrip(void)
{
    /* Generate 600 unique non-matching literal bytes followed by 600 repeated match bytes */
    uint8_t input[1200];
    for (size_t i = 0; i < 600; i++) {
        input[i] = (uint8_t)(i & 0xFF);
    }
    memset(input + 600, 0xAA, 600);

    uint8_t compressed[2048];
    uint8_t decompressed[2048];

    size_t comp_size =
        syn_lz4_compress(&g_lz4, input, sizeof(input), compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    size_t decomp_size =
        syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(sizeof(input), decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(input, decompressed, sizeof(input));
}

void test_lz4_buffer_overflow_and_corrupt_rejection(void)
{
    const char *input = "RepetitiveDataRepetitiveDataRepetitiveDataRepetitiveData";
    size_t in_len = strlen(input);

    uint8_t compressed[256];
    uint8_t decompressed[256];

    size_t comp_size = syn_lz4_compress(&g_lz4, input, in_len, compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    /* Destination buffer too small during compression */
    TEST_ASSERT_EQUAL(0, syn_lz4_compress(&g_lz4, input, in_len, compressed, 1));
    TEST_ASSERT_EQUAL(0, syn_lz4_compress(&g_lz4, input, in_len, compressed, 5));

    /* Destination buffer too small during decompression */
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(compressed, comp_size, decompressed, 10));

    /* Truncated compressed payload tests */
    for (size_t truncated_len = 1; truncated_len < comp_size; truncated_len++) {
        syn_lz4_decompress(compressed, truncated_len, decompressed, sizeof(decompressed));
    }

    /* Corrupt match offset = 0 */
    uint8_t corrupt_offset_zero[16] = {0x00, 0x01, 'A', 0x00, 0x00};
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(corrupt_offset_zero, sizeof(corrupt_offset_zero),
                                            decompressed, sizeof(decompressed)));

    /* Corrupt match offset larger than decompressed history */
    uint8_t corrupt_offset_out[16] = {0x01, 'A', 0x50, 0x00};
    TEST_ASSERT_EQUAL(0, syn_lz4_decompress(corrupt_offset_out, sizeof(corrupt_offset_out),
                                            decompressed, sizeof(decompressed)));

    /* Match length exceeds destination capacity during decompression (both extended & short) */
    uint8_t match_overflow[16] = {0x1F, 'A', 0x01, 0x00, 0xFF, 0xFF, 0x0A};
    TEST_ASSERT_EQUAL(0,
                      syn_lz4_decompress(match_overflow, sizeof(match_overflow), decompressed, 5));
    uint8_t short_match_overflow[10] = {0x12, 'A', 0x01, 0x00, 0x50, 'B', 'B', 'B', 'B', 'B'};
    TEST_ASSERT_EQUAL(
        0, syn_lz4_decompress(short_match_overflow, sizeof(short_match_overflow), decompressed, 4));
}

void test_lz4_short_input_long_literal(void)
{
    uint8_t input_short[4] = {'A', 'B', 'C', 'D'};
    uint8_t compressed[64];
    uint8_t decompressed[64];

    size_t comp_size = syn_lz4_compress(&g_lz4, input_short, 4, compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    size_t decomp_size =
        syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(4, decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(input_short, decompressed, 4);

    uint8_t input_18[18];
    for (size_t i = 0; i < 18; i++) {
        input_18[i] = (uint8_t)i;
    }

    comp_size = syn_lz4_compress(&g_lz4, input_18, 18, compressed, sizeof(compressed));
    TEST_ASSERT_TRUE(comp_size > 0);

    decomp_size = syn_lz4_decompress(compressed, comp_size, decompressed, sizeof(decompressed));
    TEST_ASSERT_EQUAL(18, decomp_size);
    TEST_ASSERT_EQUAL_MEMORY(input_18, decompressed, 18);
}

static void test_r5_lz4_mflimit_and_lastliterals(void)
{
    uint8_t comp[1200];
    uint8_t decomp[1200];

    /* 1. R5-S15: All inputs < 13 bytes (1..12) must emit pure literal sequence */
    for (size_t len = 1U; len <= 12U; len++) {
        uint8_t repeated[12];
        memset(repeated, 'A', sizeof(repeated));
        size_t csz = syn_lz4_compress(&g_lz4, repeated, len, comp, sizeof(comp));
        TEST_ASSERT_EQUAL_UINT32((uint32_t)(1U + len), (uint32_t)csz);
        TEST_ASSERT_EQUAL_HEX8((uint8_t)(len << 4), comp[0]);
        size_t dsz = syn_lz4_decompress(comp, csz, decomp, sizeof(decomp));
        TEST_ASSERT_EQUAL_UINT32((uint32_t)len, (uint32_t)dsz);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(repeated, decomp, len);
    }

    /* 2. R5-S15: >= 13B all-identical input caps match so final sequence has >= 5 literals */
    for (size_t len = 13U; len <= 32U; len++) {
        uint8_t rep[32];
        memset(rep, 'Q', sizeof(rep));
        size_t csz = syn_lz4_compress(&g_lz4, rep, len, comp, sizeof(comp));
        TEST_ASSERT_TRUE(csz >= 6U);
        /* Final sequence in rep compression must end with >= 5 literal 'Q' bytes (token >= 0x50,
         * match nibble == 0) */
        uint8_t last_tok = comp[csz - 6U];
        TEST_ASSERT_EQUAL_HEX8(0x50U, last_tok);
        size_t dsz = syn_lz4_decompress(comp, csz, decomp, sizeof(decomp));
        TEST_ASSERT_EQUAL_UINT32((uint32_t)len, (uint32_t)dsz);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(rep, decomp, len);
    }

    /* 3. Exhaustive capacity & truncation sweep over long literals + long match + long tail */
    uint8_t mixed[900];
    uint32_t lfsr = 0xACE1U;
    for (size_t i = 0U; i < 300U; i++) {
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1U) & 0xB400U);
        mixed[i] = (uint8_t)(lfsr & 0xFFU);
    }
    memset(mixed + 300U, 0x5AU, 300U);
    for (size_t i = 600U; i < 900U; i++) {
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1U) & 0xB400U);
        mixed[i] = (uint8_t)(lfsr & 0xFFU);
    }

    size_t full_csz = syn_lz4_compress(&g_lz4, mixed, sizeof(mixed), comp, sizeof(comp));
    TEST_ASSERT_TRUE(full_csz > 0U);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)sizeof(mixed),
                             (uint32_t)syn_lz4_decompress(comp, full_csz, decomp, sizeof(decomp)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(mixed, decomp, sizeof(mixed));

    for (size_t cap = 0U; cap < full_csz; cap++) {
        uint8_t small_out[1200];
        TEST_ASSERT_EQUAL_UINT32(
            0U, (uint32_t)syn_lz4_compress(&g_lz4, mixed, sizeof(mixed), small_out, cap));
    }
    for (size_t trunc = 1U; trunc < full_csz; trunc++) {
        (void)syn_lz4_decompress(comp, trunc, decomp, sizeof(decomp));
    }
    for (size_t dcap = 1U; dcap < sizeof(mixed); dcap++) {
        TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_lz4_decompress(comp, full_csz, decomp, dcap));
    }

    /* 4. Final literal >= 15 bytes overflowing extended length byte at dst_capacity == 1 */
    uint8_t incomp20[20];
    for (size_t i = 0U; i < sizeof(incomp20); i++) {
        incomp20[i] = (uint8_t)(i * 7U + 1U);
    }
    uint8_t tiny_dst[2];
    TEST_ASSERT_EQUAL_UINT32(
        0U, (uint32_t)syn_lz4_compress(&g_lz4, incomp20, sizeof(incomp20), tiny_dst, 1U));
}

static void test_tier_a_lz4_hash_bounds_and_workspace_roundtrip(void)
{
    /* UTL-01: Verify hash table sizing invariant and high-entropy + match round-trip */
    TEST_ASSERT_TRUE(SYN_LZ4_HASH_SIZE >= 64U);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)(SYN_LZ4_HASH_SIZE & (SYN_LZ4_HASH_SIZE - 1U)));
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(SYN_LZ4_HASH_SIZE * sizeof(uint16_t)),
                             (uint32_t)sizeof(SYN_Lz4));

    uint8_t payload[512];
    uint8_t comp[600];
    uint8_t decomp[512];
    for (size_t i = 0U; i < 256U; i++) {
        payload[i] = (uint8_t)((i * 131U) ^ 0xA5U);
    }
    memcpy(payload + 256U, payload, 256U);

    size_t csz = syn_lz4_compress(&g_lz4, payload, sizeof(payload), comp, sizeof(comp));
    TEST_ASSERT_TRUE(csz > 0U);
    TEST_ASSERT_TRUE(csz < sizeof(payload));

    size_t dsz = syn_lz4_decompress(comp, csz, decomp, sizeof(decomp));
    TEST_ASSERT_EQUAL_UINT32((uint32_t)sizeof(payload), (uint32_t)dsz);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, decomp, sizeof(payload));
}

void run_lz4_tests(void)
{
    RUN_TEST(test_lz4_init_and_null_checks);
    RUN_TEST(test_lz4_repetitive_text_roundtrip);
    RUN_TEST(test_lz4_small_payload);
    RUN_TEST(test_lz4_binary_telemetry_roundtrip);
    RUN_TEST(test_lz4_ultra_long_literal_and_match_roundtrip);
    RUN_TEST(test_lz4_short_input_long_literal);
    RUN_TEST(test_lz4_buffer_overflow_and_corrupt_rejection);
    RUN_TEST(test_r5_lz4_mflimit_and_lastliterals);
    RUN_TEST(test_tier_a_lz4_hash_bounds_and_workspace_roundtrip);
}
