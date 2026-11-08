/**
 * @file test_p256.c
 * @brief Unit tests and RFC test vectors for NIST P-256 (secp256r1) ECC engine.
 */

#include "syntropic/crypto/syn_p256.h"
#include "unity/unity.h"

#include <string.h>

void test_p256_base_mul_and_curve_validation(void)
{
    /* 1. Base point check: scalar = 1 -> G */
    uint8_t scalar_one[32] = {0};
    scalar_one[31] = 1;

    uint8_t pub_x[32], pub_y[32];
    /* Expected Gx = 0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296 */
    static const uint8_t EXP_GX[32] = {0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47,
                                       0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
                                       0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0,
                                       0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96};
    /* Expected Gy = 0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5 */
    static const uint8_t EXP_GY[32] = {0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B,
                                       0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
                                       0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE,
                                       0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5};

    TEST_ASSERT_TRUE(syn_p256_base_mul(scalar_one, pub_x, pub_y));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_GX, pub_x, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_GY, pub_y, 32);
    TEST_ASSERT_TRUE(syn_p256_is_on_curve(pub_x, pub_y));

    /* Null pointer guards */
    TEST_ASSERT_FALSE(syn_p256_base_mul(NULL, pub_x, pub_y));
    TEST_ASSERT_FALSE(syn_p256_base_mul(scalar_one, NULL, pub_y));
    TEST_ASSERT_FALSE(syn_p256_base_mul(scalar_one, pub_x, NULL));
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(NULL, pub_y));
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(pub_x, NULL));

    /* Zero scalar rejection */
    uint8_t zero_scalar[32] = {0};
    TEST_ASSERT_FALSE(syn_p256_base_mul(zero_scalar, pub_x, pub_y));

    /* Off-curve point rejection */
    uint8_t bad_y[32];
    memcpy(bad_y, pub_y, 32);
    bad_y[31] ^= 0x01;
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(pub_x, bad_y));
}

void test_p256_ecdh_rfc5903_test_vector(void)
{
    /* RFC 5903 / NIST test vector for ECDH */
    /* Alice private key */
    static const uint8_t ALICE_PRIV[32] = {0xC8, 0x8F, 0x01, 0xF5, 0x10, 0xD9, 0xAC, 0x3F,
                                           0x70, 0xA2, 0x92, 0xDA, 0x23, 0xF6, 0x35, 0x2F,
                                           0xAC, 0xCA, 0x30, 0x98, 0x5C, 0x34, 0x4D, 0x2E,
                                           0x98, 0x63, 0x72, 0xC0, 0xEC, 0x97, 0x07, 0x3B};

    /* Bob private key */
    static const uint8_t BOB_PRIV[32] = {0x7D, 0x40, 0xE0, 0x7E, 0x74, 0xF4, 0x9A, 0x71,
                                         0x48, 0xC3, 0x32, 0x10, 0xCA, 0x19, 0xBD, 0x9B,
                                         0x9D, 0x04, 0x50, 0x77, 0x92, 0xEC, 0xF7, 0x3E,
                                         0x87, 0x7E, 0x10, 0x64, 0x79, 0x80, 0x0C, 0x85};

    uint8_t alice_pub_x[32], alice_pub_y[32];
    uint8_t bob_pub_x[32], bob_pub_y[32];

    TEST_ASSERT_TRUE(syn_p256_base_mul(ALICE_PRIV, alice_pub_x, alice_pub_y));
    TEST_ASSERT_TRUE(syn_p256_base_mul(BOB_PRIV, bob_pub_x, bob_pub_y));

    uint8_t alice_shared[32];
    uint8_t bob_shared[32];

    TEST_ASSERT_TRUE(syn_p256_ecdh(ALICE_PRIV, bob_pub_x, bob_pub_y, alice_shared));
    TEST_ASSERT_TRUE(syn_p256_ecdh(BOB_PRIV, alice_pub_x, alice_pub_y, bob_shared));

    /* ECDH secret must match on both sides */
    TEST_ASSERT_EQUAL_HEX8_ARRAY(alice_shared, bob_shared, 32);

    /* Test null checks in point_mul / ecdh */
    TEST_ASSERT_FALSE(syn_p256_point_mul(NULL, bob_pub_x, bob_pub_y, alice_shared, bob_pub_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(ALICE_PRIV, NULL, bob_pub_y, alice_shared, bob_pub_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(ALICE_PRIV, bob_pub_x, NULL, alice_shared, bob_pub_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(ALICE_PRIV, bob_pub_x, bob_pub_y, NULL, bob_pub_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(ALICE_PRIV, bob_pub_x, bob_pub_y, alice_shared, NULL));

    /* Test zero scalar rejection */
    uint8_t zero[32] = {0};
    TEST_ASSERT_FALSE(syn_p256_point_mul(zero, bob_pub_x, bob_pub_y, alice_shared, bob_pub_y));
}

void test_p256_ecdsa_verify_rfc6979_test_vectors(void)
{
    /* FIPS 186-4 / RFC 6979 Section A.2.5 Test Vector for P-256 / SHA-256 */
    /* Message: "sample" */
    /* SHA-256("sample") = af2bdbe1aa9b6ec1e2ade1d694f41fc71a831d0268e9891562113d8a62add1bf */
    static const uint8_t HASH[32] = {0xAF, 0x2B, 0xDB, 0xE1, 0xAA, 0x9B, 0x6E, 0xC1,
                                     0xE2, 0xAD, 0xE1, 0xD6, 0x94, 0xF4, 0x1F, 0xC7,
                                     0x1A, 0x83, 0x1D, 0x02, 0x68, 0xE9, 0x89, 0x15,
                                     0x62, 0x11, 0x3D, 0x8A, 0x62, 0xAD, 0xD1, 0xBF};

    /* Private key: c9afa9d845ba75166b5c215767b1d6934e50c3db36e89b127b8a622b120f6721 */
    static const uint8_t PRIV[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                     0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                     0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                     0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};

    uint8_t pub_x[32], pub_y[32];
    TEST_ASSERT_TRUE(syn_p256_base_mul(PRIV, pub_x, pub_y));

    /* Expected public key from vector:
       Ux = 60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6
       Uy = 7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299 */
    static const uint8_t EXP_UX[32] = {0x60, 0xFE, 0xD4, 0xBA, 0x25, 0x5A, 0x9D, 0x31,
                                       0xC9, 0x61, 0xEB, 0x74, 0xC6, 0x35, 0x6D, 0x68,
                                       0xC0, 0x49, 0xB8, 0x92, 0x3B, 0x61, 0xFA, 0x6C,
                                       0xE6, 0x69, 0x62, 0x2E, 0x60, 0xF2, 0x9F, 0xB6};
    static const uint8_t EXP_UY[32] = {0x79, 0x03, 0xFE, 0x10, 0x08, 0xB8, 0xBC, 0x99,
                                       0xA4, 0x1A, 0xE9, 0xE9, 0x56, 0x28, 0xBC, 0x64,
                                       0xF2, 0xF1, 0xB2, 0x0C, 0x2D, 0x7E, 0x9F, 0x51,
                                       0x77, 0xA3, 0xC2, 0x94, 0xD4, 0x46, 0x22, 0x99};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_UX, pub_x, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_UY, pub_y, 32);

    static const uint8_t NONCE_K[32] = {0xA6, 0xE3, 0xC5, 0x7D, 0xD0, 0x1A, 0xBE, 0x90,
                                        0x08, 0x65, 0x38, 0x39, 0x83, 0x55, 0xDD, 0x4C,
                                        0x3B, 0x17, 0xAA, 0x87, 0x33, 0x82, 0xB0, 0xF2,
                                        0x4D, 0x61, 0x29, 0x49, 0x3D, 0x8A, 0xAD, 0x60};

    uint8_t sig_r[32], sig_s[32];
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(PRIV, NONCE_K, HASH, sig_r, sig_s));

    /* Verify signature */
    TEST_ASSERT_TRUE(syn_p256_verify_ecdsa(HASH, sig_r, sig_s, pub_x, pub_y));

    /* Deterministic ECDSA signature (RFC 6979) */
    uint8_t det_r[32], det_s[32];
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa_deterministic(PRIV, HASH, det_r, det_s));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(sig_r, det_r, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(sig_s, det_s, 32);
    TEST_ASSERT_TRUE(syn_p256_verify_ecdsa(HASH, det_r, det_s, pub_x, pub_y));

    /* Null & invalid guards on deterministic sign */
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(NULL, HASH, det_r, det_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(PRIV, NULL, det_r, det_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(PRIV, HASH, NULL, det_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(PRIV, HASH, det_r, NULL));

    /* Null guards on sign */
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(NULL, NONCE_K, HASH, sig_r, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(PRIV, NULL, HASH, sig_r, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(PRIV, NONCE_K, NULL, sig_r, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(PRIV, NONCE_K, HASH, NULL, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(PRIV, NONCE_K, HASH, sig_r, NULL));

    /* Zero/invalid nonce and zero/overflow private key on sign -> must fail */
    uint8_t zero_nonce[32] = {0};
    uint8_t max_priv[32];
    memset(max_priv, 0xFFU, sizeof(max_priv));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(PRIV, zero_nonce, HASH, sig_r, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(zero_nonce, NONCE_K, HASH, sig_r, sig_s));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(max_priv, NONCE_K, HASH, sig_r, sig_s));

    /* Large hash (e >= N) reduction test */
    uint8_t large_hash[32], lr_r[32], lr_s[32];
    memset(large_hash, 0xFF, sizeof(large_hash));
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(PRIV, NONCE_K, large_hash, lr_r, lr_s));
    TEST_ASSERT_TRUE(syn_p256_verify_ecdsa(large_hash, lr_r, lr_s, pub_x, pub_y));

    /* Signature generation where s = 0 (e = -d*r mod n) -> must fail */
    uint8_t one_priv[32] = {0}, one_nonce[32] = {0};
    one_priv[31] = 0x01;
    one_nonce[31] = 0x01;
    static const uint8_t e_zero_s[32] = {0x94, 0xE8, 0x2E, 0x0C, 0x1E, 0xD3, 0xBD, 0xB9,
                                         0x07, 0x43, 0x19, 0x1A, 0x9C, 0x5B, 0xBF, 0x0D,
                                         0x45, 0xE3, 0x7D, 0x2C, 0x79, 0x2C, 0x6A, 0xE3,
                                         0xFF, 0x18, 0x91, 0x7D, 0x23, 0xCA, 0x62, 0xBB};
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa(one_priv, one_nonce, e_zero_s, sig_r, sig_s));

    /* Tampered hash -> must fail */
    uint8_t bad_hash[32];
    memcpy(bad_hash, HASH, 32);
    bad_hash[0] ^= 0xFF;
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(bad_hash, sig_r, sig_s, pub_x, pub_y));

    /* Tampered r -> must fail */
    uint8_t bad_r[32];
    memcpy(bad_r, sig_r, 32);
    bad_r[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, bad_r, sig_s, pub_x, pub_y));

    /* Tampered s -> must fail */
    uint8_t bad_s[32];
    memcpy(bad_s, sig_s, 32);
    bad_s[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, bad_s, pub_x, pub_y));

    /* Zero r or s -> must fail */
    uint8_t zero[32] = {0};
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, zero, sig_s, pub_x, pub_y));
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, zero, pub_x, pub_y));

    /* Null pointer checks */
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(NULL, sig_r, sig_s, pub_x, pub_y));
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, NULL, sig_s, pub_x, pub_y));
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, NULL, pub_x, pub_y));
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, sig_s, NULL, pub_y));
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, sig_s, pub_x, NULL));

    /* Off curve verification */
    uint8_t bad_pub_y[32];
    memcpy(bad_pub_y, pub_y, 32);
    bad_pub_y[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(HASH, sig_r, sig_s, pub_x, bad_pub_y));

    /* Verification point addition resulting in point at infinity (e = -r mod n when Q = G) */
    static const uint8_t GX_BYTES[32] = {0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47,
                                         0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
                                         0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0,
                                         0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96};
    static const uint8_t GY_BYTES[32] = {0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B,
                                         0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
                                         0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE,
                                         0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5};
    uint8_t one_r[32] = {0}, one_s[32] = {0};
    one_r[31] = 0x01;
    one_s[31] = 0x01;
    static const uint8_t neg_one_hash[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                             0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
                                             0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x50};
    TEST_ASSERT_FALSE(syn_p256_verify_ecdsa(neg_one_hash, one_r, one_s, GX_BYTES, GY_BYTES));

    /* Scalar multiplication null and off-curve guards */
    uint8_t out_x[32], out_y[32];
    TEST_ASSERT_FALSE(syn_p256_base_mul(NULL, out_x, out_y));
    TEST_ASSERT_FALSE(syn_p256_base_mul(PRIV, NULL, out_y));
    TEST_ASSERT_FALSE(syn_p256_base_mul(PRIV, out_x, NULL));
    TEST_ASSERT_FALSE(syn_p256_base_mul(zero, out_x, out_y));

    TEST_ASSERT_FALSE(syn_p256_point_mul(NULL, pub_x, pub_y, out_x, out_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(PRIV, NULL, pub_y, out_x, out_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(PRIV, pub_x, NULL, out_x, out_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(PRIV, pub_x, pub_y, NULL, out_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(PRIV, pub_x, pub_y, out_x, NULL));
    TEST_ASSERT_FALSE(syn_p256_point_mul(PRIV, pub_x, bad_pub_y, out_x, out_y));
    TEST_ASSERT_FALSE(syn_p256_point_mul(zero, pub_x, pub_y, out_x, out_y));

    /* ECDH null and bounds */
    uint8_t secret[32];
    TEST_ASSERT_FALSE(syn_p256_ecdh(NULL, pub_x, pub_y, secret));
    TEST_ASSERT_FALSE(syn_p256_ecdh(PRIV, NULL, pub_y, secret));
    TEST_ASSERT_FALSE(syn_p256_ecdh(PRIV, pub_x, NULL, secret));
    TEST_ASSERT_FALSE(syn_p256_ecdh(PRIV, pub_x, pub_y, NULL));
    TEST_ASSERT_FALSE(syn_p256_ecdh(PRIV, pub_x, bad_pub_y, secret));

    /* Curve validation null and bounds */
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(NULL, pub_y));
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(pub_x, NULL));
    uint8_t overflow_coord[32];
    memset(overflow_coord, 0xFF, 32);
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(overflow_coord, pub_y));
    TEST_ASSERT_FALSE(syn_p256_is_on_curve(pub_x, overflow_coord));
}

void test_tier_a_p256_ct_and_rfc6979_bounds(void)
{
    static const uint8_t PRIV[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                     0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                     0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                     0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t ORDER_N[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                        0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
                                        0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51};

    uint8_t zero_key[32] = {0};
    uint8_t h_small[32] = {0};
    uint8_t h_large[32];
    uint8_t pub_x[32];
    uint8_t pub_y[32];
    uint8_t r1[32];
    uint8_t s1[32];
    uint8_t r2[32];
    uint8_t s2[32];

    h_small[31] = 0x05U;
    memcpy(h_large, ORDER_N, sizeof(h_large));
    h_large[31] = 0x56U; /* n + 5 */

    /* P256-3: deterministic ECDSA rejects priv_key == 0 and priv_key >= n immediately */
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(zero_key, h_small, r1, s1));
    TEST_ASSERT_FALSE(syn_p256_sign_ecdsa_deterministic(ORDER_N, h_small, r1, s1));

    /* P256-3: RFC 6979 bits2octets reduces h_large = n + 5 to 5 before HMAC-DRBG init */
    TEST_ASSERT_TRUE(syn_p256_base_mul(PRIV, pub_x, pub_y));
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa_deterministic(PRIV, h_small, r1, s1));
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa_deterministic(PRIV, h_large, r2, s2));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(r1, r2, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(s1, s2, 32);
    TEST_ASSERT_TRUE(syn_p256_verify_ecdsa(h_large, r2, s2, pub_x, pub_y));

    /* P256-1a: constant-time scalar mul with leading/internal zero nibbles */
    uint8_t ka[32] = {0};
    uint8_t kb[32] = {0};
    uint8_t kab[32] = {0};
    uint8_t qa_x[32];
    uint8_t qa_y[32];
    uint8_t lhs_x[32];
    uint8_t lhs_y[32];
    uint8_t rhs_x[32];
    uint8_t rhs_y[32];

    ka[30] = 0x10U;
    ka[31] = 0x03U; /* 0x1003 = 4099 */
    kb[31] = 0x07U; /* 7 */
    kab[30] = 0x70U;
    kab[31] = 0x15U; /* 4099 * 7 = 28693 = 0x7015 */

    TEST_ASSERT_TRUE(syn_p256_base_mul(ka, qa_x, qa_y));
    TEST_ASSERT_TRUE(syn_p256_point_mul(kb, qa_x, qa_y, lhs_x, lhs_y));
    TEST_ASSERT_TRUE(syn_p256_base_mul(kab, rhs_x, rhs_y));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(rhs_x, lhs_x, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(rhs_y, lhs_y, 32);

    /* Coincident finite point addition P == Q during ECDSA verify (u1 = u2 = 1, Q = G) */
    uint8_t d_one[32] = {0};
    uint8_t k_two[32] = {0};
    uint8_t gx[32];
    uint8_t gy[32];
    uint8_t two_gx[32];
    uint8_t two_gy[32];
    d_one[31] = 0x01U;
    k_two[31] = 0x02U;
    TEST_ASSERT_TRUE(syn_p256_base_mul(d_one, gx, gy));
    TEST_ASSERT_TRUE(syn_p256_base_mul(k_two, two_gx, two_gy));
    TEST_ASSERT_TRUE(syn_p256_sign_ecdsa(d_one, k_two, two_gx, r1, s1));
    TEST_ASSERT_TRUE(syn_p256_verify_ecdsa(two_gx, r1, s1, gx, gy));

    /* Failed ECDH wipes output buffer */
    uint8_t secret[32];
    memset(secret, 0xAAU, sizeof(secret));
    TEST_ASSERT_FALSE(syn_p256_ecdh(zero_key, pub_x, pub_y, secret));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_key, secret, 32);
}

void run_p256_tests(void)
{
    RUN_TEST(test_p256_base_mul_and_curve_validation);
    RUN_TEST(test_p256_ecdh_rfc5903_test_vector);
    RUN_TEST(test_p256_ecdsa_verify_rfc6979_test_vectors);
    RUN_TEST(test_tier_a_p256_ct_and_rfc6979_bounds);
}
