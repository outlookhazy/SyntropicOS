/**
 * @file test_sha256.c
 * @brief Unity tests for syn_sha256 and syn_hmac (HMAC-SHA256).
 *
 * SHA-256 test vectors from NIST FIPS 180-4 / CSRC examples.
 * HMAC-SHA256 test vectors from RFC 4231.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/util/syn_sha256.h"
#include "syntropic/util/syn_hmac.h"

#include <string.h>

/* ── Helper ─────────────────────────────────────────────────────────────── */

/**
 * @brief Convert a hex string to binary.
 * @param hex   Hex string (even length, lowercase).
 * @param out   Output buffer.
 * @param len   Number of bytes to produce.
 */
static void hex_to_bin(const char *hex, uint8_t *out, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        unsigned hi, lo;
        char c;

        c = hex[i * 2];
        if (c >= '0' && c <= '9')      hi = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') hi = (unsigned)(c - 'a' + 10);
        else                           hi = 0;

        c = hex[i * 2 + 1];
        if (c >= '0' && c <= '9')      lo = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') lo = (unsigned)(c - 'a' + 10);
        else                           lo = 0;

        out[i] = (uint8_t)((hi << 4) | lo);
    }
}

/* ── SHA-256 Tests ──────────────────────────────────────────────────────── */

/** NIST: SHA-256("") */
static void test_sha256_empty(void)
{
    uint8_t hash[32];
    syn_sha256("", 0, hash);

    uint8_t expected[32];
    hex_to_bin("e3b0c44298fc1c149afbf4c8996fb924"
               "27ae41e4649b934ca495991b7852b855",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/** NIST: SHA-256("abc") */
static void test_sha256_abc(void)
{
    uint8_t hash[32];
    syn_sha256("abc", 3, hash);

    uint8_t expected[32];
    hex_to_bin("ba7816bf8f01cfea414140de5dae2223"
               "b00361a396177a9cb410ff61f20015ad",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/** NIST: SHA-256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") */
static void test_sha256_448bit(void)
{
    const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    uint8_t hash[32];
    syn_sha256(msg, strlen(msg), hash);

    uint8_t expected[32];
    hex_to_bin("248d6a61d20638b8e5c026930c3e6039"
               "a33ce45964ff2167f6ecedd419db06c1",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/** NIST: SHA-256("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn
 *                 hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu") */
static void test_sha256_896bit(void)
{
    const char *msg = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                      "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    uint8_t hash[32];
    syn_sha256(msg, strlen(msg), hash);

    uint8_t expected[32];
    hex_to_bin("cf5b16a778af8380036ce59e7b049237"
               "0b249b11e8f07a51afac45037afee9d1",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, hash, 32);
}

/** Streaming: feed data in multiple update() calls, verify same hash. */
static void test_sha256_streaming(void)
{
    const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    size_t total = strlen(msg);

    /* One-shot reference */
    uint8_t ref[32];
    syn_sha256(msg, total, ref);

    /* Feed byte by byte */
    SYN_SHA256 ctx;
    syn_sha256_init(&ctx);
    size_t i;
    for (i = 0; i < total; i++) {
        syn_sha256_update(&ctx, msg + i, 1);
    }
    uint8_t hash[32];
    syn_sha256_final(&ctx, hash);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref, hash, 32);

    /* Feed in two chunks */
    syn_sha256_init(&ctx);
    syn_sha256_update(&ctx, msg, 7);
    syn_sha256_update(&ctx, msg + 7, total - 7);
    syn_sha256_final(&ctx, hash);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref, hash, 32);
}

/** Verify update with NULL data and zero length is a no-op. */
static void test_sha256_null_update(void)
{
    uint8_t ref[32];
    syn_sha256("abc", 3, ref);

    SYN_SHA256 ctx;
    syn_sha256_init(&ctx);
    syn_sha256_update(&ctx, "abc", 3);
    syn_sha256_update(&ctx, NULL, 0);
    uint8_t hash[32];
    syn_sha256_final(&ctx, hash);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref, hash, 32);
}

/* ── HMAC-SHA256 Tests (RFC 4231) ───────────────────────────────────────── */

/** RFC 4231 Test Case 1: 20-byte key, "Hi There" */
static void test_hmac_case1(void)
{
    uint8_t key[20];
    memset(key, 0x0b, 20);
    const char *data = "Hi There";

    uint8_t mac[32];
    syn_hmac_sha256(key, 20, data, strlen(data), mac);

    uint8_t expected[32];
    hex_to_bin("b0344c61d8db38535ca8afceaf0bf12b"
               "881dc200c9833da726e9376c2e32cff7",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/** RFC 4231 Test Case 2: key = "Jefe", data = "what do ya want for nothing?" */
static void test_hmac_case2(void)
{
    const char *key = "Jefe";
    const char *data = "what do ya want for nothing?";

    uint8_t mac[32];
    syn_hmac_sha256(key, strlen(key), data, strlen(data), mac);

    uint8_t expected[32];
    hex_to_bin("5bdcc146bf60754e6a042426089575c7"
               "5a003f089d2739839dec58b964ec3843",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/** RFC 4231 Test Case 3: 20-byte key = 0xaa, 50-byte data = 0xdd */
static void test_hmac_case3(void)
{
    uint8_t key[20];
    memset(key, 0xaa, 20);
    uint8_t data[50];
    memset(data, 0xdd, 50);

    uint8_t mac[32];
    syn_hmac_sha256(key, 20, data, 50, mac);

    uint8_t expected[32];
    hex_to_bin("773ea91e36800e46854db8ebd09181a7"
               "2959098b3ef8c122d9635514ced565fe",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/** RFC 4231 Test Case 4: 25-byte key, 50-byte data */
static void test_hmac_case4(void)
{
    uint8_t key[25];
    unsigned i;
    for (i = 0; i < 25; i++) key[i] = (uint8_t)(i + 1);
    uint8_t data[50];
    memset(data, 0xcd, 50);

    uint8_t mac[32];
    syn_hmac_sha256(key, 25, data, 50, mac);

    uint8_t expected[32];
    hex_to_bin("82558a389a443c0ea4cc819899f2083a"
               "85f0faa3e578f8077a2e3ff46729665b",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/** HMAC streaming: init → update in chunks → final matches one-shot. */
static void test_hmac_streaming(void)
{
    const char *key = "Jefe";
    const char *data = "what do ya want for nothing?";
    size_t data_len = strlen(data);

    /* One-shot reference */
    uint8_t ref[32];
    syn_hmac_sha256(key, strlen(key), data, data_len, ref);

    /* Streaming */
    SYN_HMAC_SHA256 ctx;
    syn_hmac_sha256_init(&ctx, key, strlen(key));
    syn_hmac_sha256_update(&ctx, data, 10);
    syn_hmac_sha256_update(&ctx, data + 10, data_len - 10);
    uint8_t mac[32];
    syn_hmac_sha256_final(&ctx, mac);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref, mac, 32);
}

/** HMAC with a key longer than the block size (> 64 bytes). */
static void test_hmac_long_key(void)
{
    /* RFC 4231 Test Case 6: 131-byte key */
    uint8_t key[131];
    memset(key, 0xaa, 131);
    const char *data = "Test Using Larger Than Block-Size Key - Hash Key First";

    uint8_t mac[32];
    syn_hmac_sha256(key, 131, data, strlen(data), mac);

    uint8_t expected[32];
    hex_to_bin("60e431591ee0b67f0d8a26aacbf5b77f"
               "8e0bc6213728c5140546040f0ee37f54",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mac, 32);
}

/* ── Coverage gap tests ─────────────────────────────────────────────────── */

/**
 * Feed 1 byte then 63 bytes to trigger the partial-buffer-fills-to-64
 * path in syn_sha256_update (lines 161-163 in syn_sha256.c).
 */
static void test_sha256_buffer_fill_boundary(void)
{
    /* Use a known 64-byte message */
    uint8_t msg[64];
    size_t i;
    for (i = 0; i < 64; i++) msg[i] = (uint8_t)i;

    /* One-shot reference */
    uint8_t ref[32];
    syn_sha256(msg, 64, ref);

    /* Streaming: 1 byte (partial buffer), then 63 bytes (fills to 64) */
    SYN_SHA256 ctx;
    syn_sha256_init(&ctx);
    syn_sha256_update(&ctx, msg, 1);       /* buf_len = 1 */
    syn_sha256_update(&ctx, msg + 1, 63);  /* fills to 64 → transform */
    uint8_t hash[32];
    syn_sha256_final(&ctx, hash);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref, hash, 32);
}

/**
 * Trigger the total_len_lo overflow carry (line 148 in syn_sha256.c).
 * We fake the internal state so total_len_lo is near UINT32_MAX, then
 * update with enough data to cause a 32-bit wrap.
 */
static void test_sha256_length_overflow_carry(void)
{
    SYN_SHA256 ctx;
    syn_sha256_init(&ctx);

    /* Manually set total_len_lo near the overflow point.
     * total_len_lo is in bits, so set it to (UINT32_MAX - 7),
     * meaning we need > 1 byte (8 bits) to overflow. */
    ctx.total_len_lo = 0xFFFFFFF8u;  /* UINT32_MAX - 7 */
    ctx.total_len_hi = 0;

    /* Feed 1 byte = 8 bits → total_len_lo wraps to 0, carry fires */
    uint8_t byte = 0xAA;
    syn_sha256_update(&ctx, &byte, 1);

    /* Verify the carry happened */
    TEST_ASSERT_EQUAL_HEX32(0x00000000u, ctx.total_len_lo);
    TEST_ASSERT_EQUAL_HEX32(1, ctx.total_len_hi);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_sha256_tests(void)
{
    /* SHA-256 */
    RUN_TEST(test_sha256_empty);
    RUN_TEST(test_sha256_abc);
    RUN_TEST(test_sha256_448bit);
    RUN_TEST(test_sha256_896bit);
    RUN_TEST(test_sha256_streaming);
    RUN_TEST(test_sha256_null_update);
    RUN_TEST(test_sha256_buffer_fill_boundary);
    RUN_TEST(test_sha256_length_overflow_carry);

    /* HMAC-SHA256 */
    RUN_TEST(test_hmac_case1);
    RUN_TEST(test_hmac_case2);
    RUN_TEST(test_hmac_case3);
    RUN_TEST(test_hmac_case4);
    RUN_TEST(test_hmac_streaming);
    RUN_TEST(test_hmac_long_key);
}
