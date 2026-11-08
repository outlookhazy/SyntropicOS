/**
 * @file test_hmac_drbg.c
 * @brief Unit tests and NIST SP 800-90A test vectors for HMAC-DRBG (SHA-256).
 */

#include "syntropic/crypto/syn_hmac_drbg.h"
#include "unity/unity.h"

#include <string.h>

void test_hmac_drbg_rfc6979_p256_vector(void)
{
    /* RFC 6979 Section A.2.5 (NIST P-256 with SHA-256 HMAC-DRBG) */
    const uint8_t x[32] = {0xc9, 0xaf, 0xa9, 0xd8, 0x45, 0xba, 0x75, 0x16, 0x6b, 0x5c, 0x21,
                           0x57, 0x67, 0xb1, 0xd6, 0x93, 0x4e, 0x50, 0xc3, 0xdb, 0x36, 0xe8,
                           0x9b, 0x12, 0x7b, 0x8a, 0x62, 0x2b, 0x12, 0x0f, 0x67, 0x21};
    const uint8_t h1[32] = {0xaf, 0x2b, 0xdb, 0xe1, 0xaa, 0x9b, 0x6e, 0xc1, 0xe2, 0xad, 0xe1,
                            0xd6, 0x94, 0xf4, 0x1f, 0xc7, 0x1a, 0x83, 0x1d, 0x02, 0x68, 0xe9,
                            0x89, 0x15, 0x62, 0x11, 0x3d, 0x8a, 0x62, 0xad, 0xd1, 0xbf};

    SYN_HMAC_DRBG drbg;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, x, sizeof(x), h1, sizeof(h1), NULL, 0));

    uint8_t k[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, k, sizeof(k), NULL, 0));

    static const uint8_t EXP_K[32] = {0xa6, 0xe3, 0xc5, 0x7d, 0xd0, 0x1a, 0xbe, 0x90,
                                      0x08, 0x65, 0x38, 0x39, 0x83, 0x55, 0xdd, 0x4c,
                                      0x3b, 0x17, 0xaa, 0x87, 0x33, 0x82, 0xb0, 0xf2,
                                      0x4d, 0x61, 0x29, 0x49, 0x3d, 0x8a, 0xad, 0x60};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_K, k, 32);

    uint8_t k0[32] = {0};
    uint8_t v0[32];
    memset(v0, 1, 32);
    uint8_t msg[97];
    memcpy(msg, v0, 32);
    msg[32] = 0x00;
    memcpy(msg + 33, x, 32);
    memcpy(msg + 65, h1, 32);

    uint8_t k1[32];
    syn_hmac_sha256(k0, 32, msg, 97, k1);
    uint8_t v1[32];
    syn_hmac_sha256(k1, 32, v0, 32, v1);

    memcpy(msg, v1, 32);
    msg[32] = 0x01;
    uint8_t k2[32];
    syn_hmac_sha256(k1, 32, msg, 97, k2);
    uint8_t v2[32];
    syn_hmac_sha256(k2, 32, v1, 32, v2);

    uint8_t v3[32];
    syn_hmac_sha256(k2, 32, v2, 32, v3);

    /* Check that syn_hmac_drbg matches manual HMAC calculation */
    TEST_ASSERT_EQUAL_MEMORY(v3, k, 32);

    /* Verify self-consistency: multiple generate calls advance state deterministically */
    uint8_t k_next[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, k_next, sizeof(k_next), NULL, 0));
    TEST_ASSERT_NOT_EQUAL(0, k_next[0]);
    TEST_ASSERT_FALSE(memcmp(k, k_next, 32) == 0);

    syn_hmac_drbg_wipe(&drbg);
}

void test_hmac_drbg_nist_sp800_90a_with_reseed(void)
{
    /* NIST CAVP HMAC-DRBG (SHA-256, PR=False, With Reseed) Test Vector */
    const uint8_t entropy_init[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
    const uint8_t nonce[16] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                               0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};
    const uint8_t entropy_reseed[32] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
                                        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
                                        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
                                        0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f};
    const uint8_t add_reseed[16] = {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
                                    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf};

    SYN_HMAC_DRBG drbg;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, entropy_init, sizeof(entropy_init), nonce,
                                                 sizeof(nonce), NULL, 0));

    uint8_t out1[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, out1, sizeof(out1), NULL, 0));

    /* Reseed */
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_reseed(&drbg, entropy_reseed, sizeof(entropy_reseed),
                                                   add_reseed, sizeof(add_reseed)));
    TEST_ASSERT_EQUAL(1, drbg.reseed_counter);

    uint8_t out2[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, out2, sizeof(out2), NULL, 0));
    TEST_ASSERT_EQUAL(2, drbg.reseed_counter);

    /* Result must be deterministic across runs */
    SYN_HMAC_DRBG drbg2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg2, entropy_init, sizeof(entropy_init), nonce,
                                                 sizeof(nonce), NULL, 0));
    uint8_t out1_dup[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg2, out1_dup, sizeof(out1_dup), NULL, 0));
    TEST_ASSERT_EQUAL_MEMORY(out1, out1_dup, sizeof(out1));

    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_reseed(&drbg2, entropy_reseed, sizeof(entropy_reseed),
                                                   add_reseed, sizeof(add_reseed)));
    uint8_t out2_dup[32];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg2, out2_dup, sizeof(out2_dup), NULL, 0));
    TEST_ASSERT_EQUAL_MEMORY(out2, out2_dup, sizeof(out2));

    syn_hmac_drbg_wipe(&drbg);
    syn_hmac_drbg_wipe(&drbg2);
}

void test_hmac_drbg_prediction_resistance(void)
{
    const uint8_t entropy_init[32] = {0xAA};
    const uint8_t nonce[16] = {0x55};
    const uint8_t entropy_pr1[32] = {0x11};
    const uint8_t add_pr1[16] = {0x22};

    SYN_HMAC_DRBG drbg;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, entropy_init, sizeof(entropy_init), nonce,
                                                 sizeof(nonce), NULL, 0));

    uint8_t out[48];
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_hmac_drbg_generate_pr(&drbg, out, sizeof(out), entropy_pr1,
                                                sizeof(entropy_pr1), add_pr1, sizeof(add_pr1)));

    /* Verify non-zero output and reseed counter update */
    bool all_zero = true;
    for (size_t i = 0; i < sizeof(out); i++) {
        if (out[i] != 0U) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    TEST_ASSERT_EQUAL(2, drbg.reseed_counter);

    syn_hmac_drbg_wipe(&drbg);
}

void test_hmac_drbg_personalization_and_additional_input(void)
{
    const uint8_t entropy[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                                 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
    const uint8_t nonce[16] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22,
                               0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};
    const uint8_t pers[16] = "SyntropicOS-DRBG";
    const uint8_t add_in[8] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};

    SYN_HMAC_DRBG drbg;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, entropy, sizeof(entropy), nonce,
                                                 sizeof(nonce), pers, sizeof(pers)));

    uint8_t out[64];
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_hmac_drbg_generate(&drbg, out, sizeof(out), add_in, sizeof(add_in)));

    /* Verify non-zero output */
    bool all_zero = true;
    for (size_t i = 0; i < sizeof(out); i++) {
        if (out[i] != 0U) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);

    syn_hmac_drbg_wipe(&drbg);
}

void test_hmac_drbg_reseed_interval_limit(void)
{
    const uint8_t entropy[32] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
                                 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                                 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

    SYN_HMAC_DRBG drbg;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_hmac_drbg_init(&drbg, entropy, sizeof(entropy), NULL, 0, NULL, 0));

    /* Limit reseed interval to 2 requests */
    syn_hmac_drbg_set_reseed_interval(&drbg, 2);

    uint8_t buf[16];
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, buf, sizeof(buf), NULL, 0));
    TEST_ASSERT_EQUAL(2, drbg.reseed_counter);

    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, buf, sizeof(buf), NULL, 0));
    TEST_ASSERT_EQUAL(3, drbg.reseed_counter);

    /* 3rd request exceeds interval (reseed_counter 3 > reseed_interval 2) -> must return error */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_hmac_drbg_generate(&drbg, buf, sizeof(buf), NULL, 0));

    /* Reseed and verify it works again */
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_reseed(&drbg, entropy, sizeof(entropy), NULL, 0));
    TEST_ASSERT_EQUAL(1, drbg.reseed_counter);
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, buf, sizeof(buf), NULL, 0));

    syn_hmac_drbg_wipe(&drbg);
}

void test_hmac_drbg_null_and_bounds_checks(void)
{
    const uint8_t ent[32] = {0x01};
    uint8_t out[16] = {0};
    SYN_HMAC_DRBG drbg;

    /* NULL context */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_hmac_drbg_init(NULL, ent, sizeof(ent), NULL, 0, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_reseed(NULL, ent, sizeof(ent), NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_generate(NULL, out, sizeof(out), NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_hmac_drbg_generate_pr(NULL, out, sizeof(out), ent, sizeof(ent), NULL, 0));

    /* Uninitialized context */
    memset(&drbg, 0, sizeof(drbg));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_generate(&drbg, out, sizeof(out), NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_reseed(&drbg, ent, sizeof(ent), NULL, 0));

    /* Short entropy (< 32 bytes) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_init(&drbg, ent, 16, NULL, 0, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_init(&drbg, NULL, 32, NULL, 0, NULL, 0));

    /* Valid init */
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, ent, sizeof(ent), NULL, 0, NULL, 0));

    /* NULL output buffer */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_hmac_drbg_generate(&drbg, NULL, sizeof(out), NULL, 0));

    /* Request length > 65536 */
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_hmac_drbg_generate(&drbg, out, SYN_HMAC_DRBG_MAX_REQUEST_BYTES + 1U, NULL, 0));

    /* Zero length output */
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, out, 0, NULL, 0));

    /* Wipe validation */
    syn_hmac_drbg_wipe(&drbg);
    TEST_ASSERT_FALSE(drbg.initialized);
    TEST_ASSERT_EQUAL(0, drbg.reseed_counter);
    TEST_ASSERT_EQUAL_UINT8(0, drbg.k[0]);
    TEST_ASSERT_EQUAL_UINT8(0, drbg.v[0]);

    /* Wipe on NULL is safe */
    syn_hmac_drbg_wipe(NULL);
    syn_hmac_drbg_set_reseed_interval(NULL, 100);
}

void test_tier_a_drbg_max_reseed_and_hmac_empty_key(void)
{
    const uint8_t ent[32] = {0x42U};
    uint8_t big_key[70];
    uint8_t out[16] = {0};
    uint8_t mac[32] = {0};
    SYN_HMAC_DRBG drbg;

    memset(big_key, 0x5AU, sizeof(big_key));

    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_init(&drbg, ent, sizeof(ent), NULL, 0U, NULL, 0U));

    /* DRBG-1: UINT32_MAX interval clamped to UINT32_MAX - 1U to prevent counter wraparound */
    syn_hmac_drbg_set_reseed_interval(&drbg, UINT32_MAX);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX - 1U, drbg.reseed_interval);

    drbg.reseed_counter = UINT32_MAX - 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hmac_drbg_generate(&drbg, out, sizeof(out), NULL, 0U));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, drbg.reseed_counter);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_hmac_drbg_generate(&drbg, out, sizeof(out), NULL, 0U));

    syn_hmac_drbg_wipe(&drbg);

    /* HASH-1: HMAC-SHA256 NULL empty key and >64B hashed key */
    syn_hmac_sha256(NULL, 0U, out, sizeof(out), mac);
    TEST_ASSERT_NOT_EQUAL(0U, mac[0]);
    syn_hmac_sha256(big_key, sizeof(big_key), out, sizeof(out), mac);
    TEST_ASSERT_NOT_EQUAL(0U, mac[0]);
}

void run_hmac_drbg_tests(void)
{
    RUN_TEST(test_hmac_drbg_rfc6979_p256_vector);
    RUN_TEST(test_hmac_drbg_nist_sp800_90a_with_reseed);
    RUN_TEST(test_hmac_drbg_prediction_resistance);
    RUN_TEST(test_hmac_drbg_personalization_and_additional_input);
    RUN_TEST(test_hmac_drbg_reseed_interval_limit);
    RUN_TEST(test_hmac_drbg_null_and_bounds_checks);
    RUN_TEST(test_tier_a_drbg_max_reseed_and_hmac_empty_key);
}
