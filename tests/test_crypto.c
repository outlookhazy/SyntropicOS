/**
 * @file test_crypto.c
 * @brief Unity tests for BLAKE2s, ChaCha20-Poly1305, and X25519.
 *
 * Comprehensive coverage with RFC test vectors and edge cases.
 * All expected values verified against Go (golang.org/x/crypto)
 * and Python (hashlib) reference implementations.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_blake2s.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_x25519.h"

#include <string.h>

#include "syntropic/util/syn_fmt.h"

static void hex2bin(const char *hex, uint8_t *out, size_t len)
{
    syn_fmt_hex_parse(hex, out, len);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BLAKE2s
 * ═══════════════════════════════════════════════════════════════════════════ */

/* RFC 7693 / BLAKE2 reference: BLAKE2s-256("abc") */
static void test_blake2s_abc(void)
{
    uint8_t hash[32];
    syn_blake2s("abc", 3, hash, 32);

    uint8_t expected[32];
    hex2bin("508c5e8c327c14e2e1a72ba34eeb452f"
            "37458b209ed63a294d999b4c86675982", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* BLAKE2s-256("") — empty input */
static void test_blake2s_empty(void)
{
    uint8_t hash[32];
    syn_blake2s("", 0, hash, 32);

    uint8_t expected[32];
    hex2bin("69217a3079908094e11121d042354a7c"
            "1f55b6482ca1a51e1b250dfd1ed0eef9", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* 64 sequential bytes — single full block with diverse word values.
 * This catches the SIGMA[9] transposition bug we found. */
static void test_blake2s_seq64(void)
{
    uint8_t data[64], hash[32];
    for (int i = 0; i < 64; i++) data[i] = (uint8_t)i;

    syn_blake2s(data, 64, hash, 32);

    uint8_t expected[32];
    hex2bin("56f34e8b96557e90c1f24b52d0c89d51"
            "086acf1b00f634cf1dde9233b8eaaa3e", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* 66 bytes — crosses one block boundary (64+2).
 * This is the exact size of HASH(C || identifier) in WireGuard. */
static void test_blake2s_multi_block(void)
{
    uint8_t data[66], hash[32];
    /* C value || "WireGuard v1 zx2c4 Jason@zx2c4.com" */
    hex2bin("60e26daef327efc02ec335e2a025d2d0"
            "16eb4206f87277f52d38d1988b78cd36", data, 32);
    memcpy(data + 32, "WireGuard v1 zx2c4 Jason@zx2c4.com", 34);

    syn_blake2s(data, 66, hash, 32);

    uint8_t expected[32];
    hex2bin("2211b361081ac566691243db458ad532"
            "2d9c6c662293e8b70ee19c65ba079ef3", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* Streaming update(32) + update(34) must equal one-shot(66) */
static void test_blake2s_streaming_matches_oneshot(void)
{
    uint8_t data[66];
    for (int i = 0; i < 66; i++) data[i] = (uint8_t)(i * 7 + 3);

    uint8_t oneshot[32];
    syn_blake2s(data, 66, oneshot, 32);

    uint8_t streamed[32];
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32);
    syn_blake2s_update(&ctx, data, 32);
    syn_blake2s_update(&ctx, data + 32, 34);
    syn_blake2s_final(&ctx, streamed);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(oneshot, streamed, 32);
}

/* Streaming with many small updates */
static void test_blake2s_streaming_byte_at_a_time(void)
{
    uint8_t data[100];
    for (int i = 0; i < 100; i++) data[i] = (uint8_t)i;

    uint8_t oneshot[32];
    syn_blake2s(data, 100, oneshot, 32);

    uint8_t streamed[32];
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32);
    for (int i = 0; i < 100; i++)
        syn_blake2s_update(&ctx, &data[i], 1);
    syn_blake2s_final(&ctx, streamed);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(oneshot, streamed, 32);
}

/* 128 bytes — 2 full blocks, verified against Go */
static void test_blake2s_128_bytes(void)
{
    uint8_t data[128], hash[32];
    for (int i = 0; i < 128; i++) data[i] = (uint8_t)i;

    syn_blake2s(data, 128, hash, 32);

    uint8_t expected[32];
    hex2bin("1fa877de67259d19863a2a34bcc6962a"
            "2b25fcbf5cbecd7ede8f1fa36688a796", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* 256 bytes — 4 full blocks, verified against Go */
static void test_blake2s_256_bytes(void)
{
    uint8_t data[256], hash[32];
    for (int i = 0; i < 256; i++) data[i] = (uint8_t)i;

    syn_blake2s(data, 256, hash, 32);

    uint8_t expected[32];
    hex2bin("5fdeb59f681d975f52c8e69c5502e02a"
            "12a3afcc5836ba58f42784c439228781", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/* Keyed BLAKE2s-256(key=[0..31], data="abc") — verified against Go */
static void test_blake2s_keyed_mac_vector(void)
{
    uint8_t key[32], mac[32];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)i;

    syn_blake2s_mac(key, 32, "abc", 3, mac, 32);

    uint8_t expected[32];
    hex2bin("a281f725754969a702f6fe36fc591b7d"
            "ef866e4b70173ece402fc01c064d6b65", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/* Keyed BLAKE2s with 16-byte output (used by WireGuard mac1) */
static void test_blake2s_keyed_mac_16byte(void)
{
    uint8_t key[32], mac[16];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)i;

    syn_blake2s_mac(key, 32, "abc", 3, mac, 16);

    uint8_t expected[16];
    hex2bin("61ba5f165c194692e09d12520cc4c74a", expected, 16);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 16);
}

/* HMAC-BLAKE2s(key=zeros, data="test") — verified against Go crypto/hmac */
static void test_hmac_blake2s_vector(void)
{
    uint8_t key[32];
    memset(key, 0, 32);
    uint8_t mac[32];

    syn_hmac_blake2s(key, 32, "test", 4, mac);

    uint8_t expected[32];
    hex2bin("61a1686eef8ea4b2f97bd2cd6f852244"
            "39a7502b9e0b0b2d3526083d8ceb597c", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/* HMAC-BLAKE2s streaming must match one-shot */
static void test_hmac_blake2s_streaming(void)
{
    uint8_t key[32];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)(i + 0x10);

    uint8_t data[64];
    for (int i = 0; i < 64; i++) data[i] = (uint8_t)i;

    /* One-shot */
    uint8_t mac_oneshot[32];
    syn_hmac_blake2s(key, 32, data, 64, mac_oneshot);

    /* Streaming */
    uint8_t mac_stream[32];
    SYN_HMAC_BLAKE2s hctx;
    syn_hmac_blake2s_init(&hctx, key, 32);
    syn_hmac_blake2s_update(&hctx, data, 20);
    syn_hmac_blake2s_update(&hctx, data + 20, 44);
    syn_hmac_blake2s_final(&hctx, mac_stream);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(mac_oneshot, mac_stream, 32);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  ChaCha20-Poly1305
 * ═══════════════════════════════════════════════════════════════════════════ */

/* RFC 8439 §2.4.2 — ChaCha20 block function */
static void test_chacha20_block_rfc8439(void)
{
    static const uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    };
    static const uint8_t nonce[12] = {
        0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x4a,
        0x00,0x00,0x00,0x00,
    };
    uint8_t out[64];
    syn_chacha20_block(key, nonce, 1, out);

    uint8_t expected[64];
    hex2bin("10f1e7e4d13b5915500fdd1fa32071c4"
            "c7d1f4c733c068030422aa9ac3d46c4e"
            "d2826446079faa0914c2d705d98b02a2"
            "b5129cd1de164eb9cbd083e8a2503c4e", expected, 64);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, out, 64);
}

/* RFC 8439 §2.4.2 — ChaCha20 keystream XOR */
static void test_chacha20_xor_rfc8439(void)
{
    static const uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    };
    static const uint8_t nonce[12] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4a,
        0x00,0x00,0x00,0x00,
    };
    /* First 64 bytes of RFC 8439 §2.4.2 sunscreen plaintext */
    static const uint8_t plain[64] = {
        0x4c,0x61,0x64,0x69,0x65,0x73,0x20,0x61,
        0x6e,0x64,0x20,0x47,0x65,0x6e,0x74,0x6c,
        0x65,0x6d,0x65,0x6e,0x20,0x6f,0x66,0x20,
        0x74,0x68,0x65,0x20,0x63,0x6c,0x61,0x73,
        0x73,0x20,0x6f,0x66,0x20,0x27,0x39,0x39,
        0x3a,0x20,0x49,0x66,0x20,0x49,0x20,0x63,
        0x6f,0x75,0x6c,0x64,0x20,0x6f,0x66,0x66,
        0x65,0x72,0x20,0x79,0x6f,0x75,0x20,0x6f,
    };

    uint8_t ct[64];
    syn_chacha20_xor(key, nonce, 1, plain, 64, ct);

    /* Encrypting and then encrypting again should produce original */
    uint8_t round_trip[64];
    syn_chacha20_xor(key, nonce, 1, ct, 64, round_trip);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(plain, round_trip, 64);

    /* Ciphertext should differ from plaintext */
    TEST_ASSERT_FALSE(memcmp(ct, plain, 64) == 0);
}

/* AEAD encrypt/decrypt round-trip */
static void test_aead_roundtrip(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0x42, 32);
    memset(nonce, 0, 12);

    uint8_t plain[64], ct[64], tag[16], decrypted[64];
    for (int i = 0; i < 64; i++) plain[i] = (uint8_t)i;

    uint8_t aad[] = "test aad";
    syn_aead_encrypt(key, nonce, aad, sizeof(aad) - 1, plain, 64, ct, tag);

    bool ok = syn_aead_decrypt(key, nonce, aad, sizeof(aad) - 1,
                                ct, 64, tag, decrypted);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(plain, decrypted, 64);
}

/* Corrupted ciphertext rejected */
static void test_aead_tamper_ct(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0xAA, 32);
    memset(nonce, 0, 12);

    uint8_t plain[16] = "hello wireguard";
    uint8_t ct[16], tag[16], decrypted[16];

    syn_aead_encrypt(key, nonce, NULL, 0, plain, 16, ct, tag);
    ct[5] ^= 0xFF;

    bool ok = syn_aead_decrypt(key, nonce, NULL, 0, ct, 16, tag, decrypted);
    TEST_ASSERT_FALSE(ok);
}

/* Corrupted tag rejected */
static void test_aead_tamper_tag(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0xBB, 32);
    memset(nonce, 0, 12);

    uint8_t plain[16] = "hello wireguard";
    uint8_t ct[16], tag[16], decrypted[16];

    syn_aead_encrypt(key, nonce, NULL, 0, plain, 16, ct, tag);
    tag[0] ^= 0x01;

    bool ok = syn_aead_decrypt(key, nonce, NULL, 0, ct, 16, tag, decrypted);
    TEST_ASSERT_FALSE(ok);
}

/* Wrong AAD rejected */
static void test_aead_tamper_aad(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0xCC, 32);
    memset(nonce, 0, 12);

    uint8_t plain[8] = {'t','e','s','t','d','a','t','a'};
    uint8_t ct[8], tag[16], decrypted[8];

    syn_aead_encrypt(key, nonce, (uint8_t *)"good", 4, plain, 8, ct, tag);

    bool ok = syn_aead_decrypt(key, nonce, (uint8_t *)"evil", 4,
                                ct, 8, tag, decrypted);
    TEST_ASSERT_FALSE(ok);
}

/* Empty plaintext (AAD-only mode) */
static void test_aead_empty_plaintext(void)
{
    uint8_t key[32], nonce[12], tag[16];
    memset(key, 0xDD, 32);
    memset(nonce, 0, 12);

    uint8_t aad[] = "metadata";
    syn_aead_encrypt(key, nonce, aad, sizeof(aad) - 1, NULL, 0, NULL, tag);

    bool ok = syn_aead_decrypt(key, nonce, aad, sizeof(aad) - 1,
                                NULL, 0, tag, NULL);
    TEST_ASSERT_TRUE(ok);
}

/* No AAD */
static void test_aead_empty_aad(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0xEE, 32);
    memset(nonce, 0, 12);

    uint8_t plain[32], ct[32], tag[16], decrypted[32];
    memset(plain, 0x55, 32);

    syn_aead_encrypt(key, nonce, NULL, 0, plain, 32, ct, tag);

    bool ok = syn_aead_decrypt(key, nonce, NULL, 0, ct, 32, tag, decrypted);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(plain, decrypted, 32);
}

/* Multi-block (>64 bytes) */
static void test_aead_large(void)
{
    uint8_t key[32], nonce[12];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)i;
    memset(nonce, 0, 12);

    uint8_t plain[256], ct[256], tag[16], decrypted[256];
    for (int i = 0; i < 256; i++) plain[i] = (uint8_t)(i ^ 0xAB);

    syn_aead_encrypt(key, nonce, NULL, 0, plain, 256, ct, tag);

    bool ok = syn_aead_decrypt(key, nonce, NULL, 0, ct, 256, tag, decrypted);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(plain, decrypted, 256);
}

/* Wrong key fails */
static void test_aead_wrong_key(void)
{
    uint8_t key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0, 12);

    uint8_t plain[16] = {0}, ct[16], tag[16], decrypted[16];
    syn_aead_encrypt(key, nonce, NULL, 0, plain, 16, ct, tag);

    /* Try with different key */
    uint8_t wrong_key[32];
    memset(wrong_key, 0x22, 32);
    bool ok = syn_aead_decrypt(wrong_key, nonce, NULL, 0, ct, 16, tag, decrypted);
    TEST_ASSERT_FALSE(ok);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  X25519
 * ═══════════════════════════════════════════════════════════════════════════ */

/* RFC 7748 §6.1 — vector 1 */
static void test_x25519_vector1(void)
{
    uint8_t scalar[32], u_in[32], result[32];
    hex2bin("a546e36bf0527c9d3b16154b82465edd"
            "62144c0ac1fc5a18506a2244ba449ac4", scalar, 32);
    hex2bin("e6db6867583030db3594c1a424b15f7c"
            "726624ec26b3353b10a903a6d0ab1c4c", u_in, 32);

    syn_x25519(result, scalar, u_in);

    uint8_t expected[32];
    hex2bin("c3da55379de9c6908e94ea4df28d084f"
            "32eccf03491c71f754b4075577a28552", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, result, 32);
}

/* RFC 7748 §6.1 — vector 2 */
static void test_x25519_vector2(void)
{
    uint8_t scalar[32], u_in[32], result[32];
    hex2bin("4b66e9d4d1b4673c5ad22691957d6af5"
            "c11b6421e0ea01d42ca4169e7918ba0d", scalar, 32);
    hex2bin("e5210f12786811d3f4b7959d0538ae2c"
            "31dbe7106fc03c3efc4cd549c715a493", u_in, 32);

    syn_x25519(result, scalar, u_in);

    uint8_t expected[32];
    hex2bin("95cbde9476e8907d7aade45cb4b873f8"
            "8b595a68799fa152e6f8f7647aac7957", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, result, 32);
}

/* Clamp verifies correct bit manipulation */
static void test_x25519_clamp(void)
{
    uint8_t key[32];
    memset(key, 0xFF, 32);
    syn_x25519_clamp(key);

    /* Bottom 3 bits of byte 0 cleared */
    TEST_ASSERT_EQUAL_HEX8(0xF8, key[0]);
    /* Top bit of byte 31 cleared, second-to-top set */
    TEST_ASSERT_EQUAL_HEX8(0x7F, key[31]);

    /* Second test: all zeros */
    memset(key, 0x00, 32);
    syn_x25519_clamp(key);
    TEST_ASSERT_EQUAL_HEX8(0x00, key[0]);
    TEST_ASSERT_EQUAL_HEX8(0x40, key[31]);
}

/* Public key derivation: basepoint * scalar */
static void test_x25519_pubkey_deterministic(void)
{
    /* Fixed key: 0x01 repeated → clamped → known public key */
    uint8_t priv[32];
    memset(priv, 0x01, 32);
    syn_x25519_clamp(priv);

    uint8_t pub1[32], pub2[32];
    syn_x25519_pubkey(pub1, priv);
    syn_x25519_pubkey(pub2, priv);

    /* Deterministic */
    TEST_ASSERT_EQUAL_HEX8_ARRAY(pub1, pub2, 32);

    /* Not all zeros */
    uint8_t zeros[32] = {0};
    TEST_ASSERT_FALSE(memcmp(pub1, zeros, 32) == 0);
}

/* Shared secret symmetry: DH(a, B) == DH(b, A) */
static void test_x25519_shared_secret_symmetry(void)
{
    uint8_t a_priv[32], a_pub[32];
    uint8_t b_priv[32], b_pub[32];

    memset(a_priv, 0x01, 32); syn_x25519_clamp(a_priv);
    memset(b_priv, 0x02, 32); syn_x25519_clamp(b_priv);

    syn_x25519_pubkey(a_pub, a_priv);
    syn_x25519_pubkey(b_pub, b_priv);

    uint8_t shared_ab[32], shared_ba[32];
    syn_x25519(shared_ab, a_priv, b_pub);
    syn_x25519(shared_ba, b_priv, a_pub);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(shared_ab, shared_ba, 32);
}

/* Identity: x25519(priv, basepoint) == x25519_pubkey(priv) */
static void test_x25519_identity(void)
{
    uint8_t priv[32];
    memset(priv, 0x77, 32);
    syn_x25519_clamp(priv);

    uint8_t pub1[32];
    syn_x25519_pubkey(pub1, priv);

    /* Basepoint = 9 */
    uint8_t basepoint[32] = {0};
    basepoint[0] = 9;

    uint8_t pub2[32];
    syn_x25519(pub2, priv, basepoint);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(pub1, pub2, 32);
}

/* Low-order point: all zeros → result should be all zeros */
static void test_x25519_low_order(void)
{
    uint8_t priv[32], zeros[32], result[32];
    memset(priv, 0x42, 32);
    syn_x25519_clamp(priv);
    memset(zeros, 0, 32);

    syn_x25519(result, priv, zeros);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zeros, result, 32);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test group runner
 * ═══════════════════════════════════════════════════════════════════════════ */

void run_crypto_tests(void)
{
    /* BLAKE2s */
    RUN_TEST(test_blake2s_abc);
    RUN_TEST(test_blake2s_empty);
    RUN_TEST(test_blake2s_seq64);
    RUN_TEST(test_blake2s_multi_block);
    RUN_TEST(test_blake2s_streaming_matches_oneshot);
    RUN_TEST(test_blake2s_streaming_byte_at_a_time);
    RUN_TEST(test_blake2s_128_bytes);
    RUN_TEST(test_blake2s_256_bytes);
    RUN_TEST(test_blake2s_keyed_mac_vector);
    RUN_TEST(test_blake2s_keyed_mac_16byte);
    RUN_TEST(test_hmac_blake2s_vector);
    RUN_TEST(test_hmac_blake2s_streaming);

    /* ChaCha20-Poly1305 */
    RUN_TEST(test_chacha20_block_rfc8439);
    RUN_TEST(test_chacha20_xor_rfc8439);
    RUN_TEST(test_aead_roundtrip);
    RUN_TEST(test_aead_tamper_ct);
    RUN_TEST(test_aead_tamper_tag);
    RUN_TEST(test_aead_tamper_aad);
    RUN_TEST(test_aead_empty_plaintext);
    RUN_TEST(test_aead_empty_aad);
    RUN_TEST(test_aead_large);
    RUN_TEST(test_aead_wrong_key);

    /* X25519 */
    RUN_TEST(test_x25519_vector1);
    RUN_TEST(test_x25519_vector2);
    RUN_TEST(test_x25519_clamp);
    RUN_TEST(test_x25519_pubkey_deterministic);
    RUN_TEST(test_x25519_shared_secret_symmetry);
    RUN_TEST(test_x25519_identity);
    RUN_TEST(test_x25519_low_order);
}
