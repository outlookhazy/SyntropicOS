/**
 * @file test_ed25519.c
 * @brief Unit tests for pure C99 Ed25519 key generation, signing & verification (RFC 8032).
 */

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

#include <string.h>

void test_ed25519_rfc8032_test_vector_1(void)
{
    /* RFC 8032 Section 7.1 Test 1 (Empty Message) */
    static const uint8_t SK[32] = {0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a,
                                   0xf4, 0x92, 0xec, 0x2c, 0xc4, 0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32,
                                   0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60};

    static const uint8_t EXP_PK[32] = {0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7,
                                       0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a,
                                       0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25,
                                       0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a};

    static const uint8_t EXP_SIG[64] = {
        0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72, 0x90, 0x86, 0xe2, 0xcc, 0x80,
        0x6e, 0x82, 0x8a, 0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74, 0xd8, 0x73,
        0xe0, 0x65, 0x22, 0x49, 0x01, 0x55, 0x5f, 0xb8, 0x82, 0x15, 0x90, 0xa3, 0x3b,
        0xac, 0xc6, 0x1e, 0x39, 0x70, 0x1c, 0xf9, 0xb4, 0x6b, 0xd2, 0x5b, 0xf5, 0xf0,
        0x59, 0x5b, 0xbe, 0x24, 0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x0b};

    uint8_t pk[32];
    TEST_ASSERT_TRUE(syn_ed25519_publickey(SK, pk));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_PK, pk, 32);

    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(NULL, 0, SK, pk, sig));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_SIG, sig, 64);
    TEST_ASSERT_TRUE(syn_ed25519_verify(sig, NULL, 0, pk));

    /* Auto-derive public key inside sign when pk is NULL */
    uint8_t sig2[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(NULL, 0, SK, NULL, sig2));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_SIG, sig2, 64);
}

void test_ed25519_rfc8032_test_vector_2(void)
{
    /* RFC 8032 Section 7.1 Test 2 (1-byte Message 0x72) */
    static const uint8_t SK[32] = {0x4c, 0xcd, 0x08, 0x9b, 0x28, 0xff, 0x96, 0xda, 0x9d, 0xb6, 0xc3,
                                   0x46, 0xec, 0x11, 0x4e, 0x0f, 0x5b, 0x8a, 0x31, 0x9f, 0x35, 0xab,
                                   0xa6, 0x24, 0xda, 0x8c, 0xf6, 0xed, 0x4f, 0xb8, 0xa6, 0xfb};

    static const uint8_t EXP_PK[32] = {0x3d, 0x40, 0x17, 0xc3, 0xe8, 0x43, 0x89, 0x5a,
                                       0x92, 0xb7, 0x0a, 0xa7, 0x4d, 0x1b, 0x7e, 0xbc,
                                       0x9c, 0x98, 0x2c, 0xcf, 0x2e, 0xc4, 0x96, 0x8c,
                                       0xc0, 0xcd, 0x55, 0xf1, 0x2a, 0xf4, 0x66, 0x0c};

    static const uint8_t MSG[1] = {0x72};

    static const uint8_t EXP_SIG[64] = {
        0x92, 0xa0, 0x09, 0xa9, 0xf0, 0xd4, 0xca, 0xb8, 0x72, 0x0e, 0x82, 0x0b, 0x5f,
        0x64, 0x25, 0x40, 0xa2, 0xb2, 0x7b, 0x54, 0x16, 0x50, 0x3f, 0x8f, 0xb3, 0x76,
        0x22, 0x23, 0xeb, 0xdb, 0x69, 0xda, 0x08, 0x5a, 0xc1, 0xe4, 0x3e, 0x15, 0x99,
        0x6e, 0x45, 0x8f, 0x36, 0x13, 0xd0, 0xf1, 0x1d, 0x8c, 0x38, 0x7b, 0x2e, 0xae,
        0xb4, 0x30, 0x2a, 0xee, 0xb0, 0x0d, 0x29, 0x16, 0x12, 0xbb, 0x0c, 0x00};

    uint8_t pk[32];
    TEST_ASSERT_TRUE(syn_ed25519_publickey(SK, pk));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_PK, pk, 32);

    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(MSG, sizeof(MSG), SK, pk, sig));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_SIG, sig, 64);
    TEST_ASSERT_TRUE(syn_ed25519_verify(sig, MSG, sizeof(MSG), pk));
}

void test_ed25519_rfc8032_test_vector_3(void)
{
    /* RFC 8032 Section 7.1 Test 3 (2-byte Message 0xAF, 0x82) */
    static const uint8_t SK[32] = {0xc5, 0xaa, 0x8d, 0xf4, 0x3f, 0x9f, 0x83, 0x7b, 0xed, 0xb7, 0x44,
                                   0x2f, 0x31, 0xdc, 0xb7, 0xb1, 0x66, 0xd3, 0x85, 0x35, 0x07, 0x6f,
                                   0x09, 0x4b, 0x85, 0xce, 0x3a, 0x2e, 0x0b, 0x44, 0x58, 0xf7};

    static const uint8_t EXP_PK[32] = {0xfc, 0x51, 0xcd, 0x8e, 0x62, 0x18, 0xa1, 0xa3,
                                       0x8d, 0xa4, 0x7e, 0xd0, 0x02, 0x30, 0xf0, 0x58,
                                       0x08, 0x16, 0xed, 0x13, 0xba, 0x33, 0x03, 0xac,
                                       0x5d, 0xeb, 0x91, 0x15, 0x48, 0x90, 0x80, 0x25};

    static const uint8_t MSG[2] = {0xaf, 0x82};

    static const uint8_t EXP_SIG[64] = {
        0x62, 0x91, 0xd6, 0x57, 0xde, 0xec, 0x24, 0x02, 0x48, 0x27, 0xe6, 0x9c, 0x3a,
        0xbe, 0x01, 0xa3, 0x0c, 0xe5, 0x48, 0xa2, 0x84, 0x74, 0x3a, 0x44, 0x5e, 0x36,
        0x80, 0xd7, 0xdb, 0x5a, 0xc3, 0xac, 0x18, 0xff, 0x9b, 0x53, 0x8d, 0x16, 0xf2,
        0x90, 0xae, 0x67, 0xf7, 0x60, 0x98, 0x4d, 0xc6, 0x59, 0x4a, 0x7c, 0x15, 0xe9,
        0x71, 0x6e, 0xd2, 0x8d, 0xc0, 0x27, 0xbe, 0xce, 0xea, 0x1e, 0xc4, 0x0a};

    uint8_t pk[32];
    TEST_ASSERT_TRUE(syn_ed25519_publickey(SK, pk));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_PK, pk, 32);

    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(MSG, sizeof(MSG), SK, pk, sig));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(EXP_SIG, sig, 64);
    TEST_ASSERT_TRUE(syn_ed25519_verify(sig, MSG, sizeof(MSG), pk));
}

void test_ed25519_create_keypair_and_roundtrip(void)
{
    uint8_t seed[32];
    for (size_t i = 0; i < sizeof(seed); i++) {
        seed[i] = (uint8_t)(i * 7U + 3U);
    }

    uint8_t pk[32], sk[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pk, sk, seed));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(seed, sk, 32);

    uint8_t msg[128];
    for (size_t i = 0; i < sizeof(msg); i++) {
        msg[i] = (uint8_t)(i ^ 0x5AU);
    }

    uint8_t sig[64];
    TEST_ASSERT_TRUE(syn_ed25519_sign(msg, sizeof(msg), sk, pk, sig));
    TEST_ASSERT_TRUE(syn_ed25519_verify(sig, msg, sizeof(msg), pk));

    /* Tamper message -> verification must fail */
    msg[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, msg, sizeof(msg), pk));
    msg[0] ^= 0x01;

    /* Tamper signature -> verification must fail */
    sig[0] ^= 0x80;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, msg, sizeof(msg), pk));
    sig[0] ^= 0x80;

    /* Tamper public key -> verification must fail */
    pk[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, msg, sizeof(msg), pk));
}

void test_ed25519_null_and_bounds_checks(void)
{
    uint8_t dummy[64] = {0};
    uint8_t pk[32] = {0}, sk[32] = {0}, seed[32] = {0}, sig[64] = {0};

    /* Public key derivation null guards */
    TEST_ASSERT_FALSE(syn_ed25519_publickey(NULL, pk));
    TEST_ASSERT_FALSE(syn_ed25519_publickey(sk, NULL));

    /* Keypair creation null guards */
    TEST_ASSERT_FALSE(syn_ed25519_create_keypair(NULL, sk, seed));
    TEST_ASSERT_FALSE(syn_ed25519_create_keypair(pk, NULL, seed));
    TEST_ASSERT_FALSE(syn_ed25519_create_keypair(pk, sk, NULL));

    /* Sign null guards */
    TEST_ASSERT_FALSE(syn_ed25519_sign(NULL, 10, sk, pk, sig));
    TEST_ASSERT_FALSE(syn_ed25519_sign(dummy, 10, NULL, pk, sig));
    TEST_ASSERT_FALSE(syn_ed25519_sign(dummy, 10, sk, pk, NULL));

    /* Verify null guards */
    TEST_ASSERT_FALSE(syn_ed25519_verify(NULL, dummy, 10, pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, NULL, 10, pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, dummy, 10, NULL));

    /* Non-canonical S scalar (S[63] top 3 bits != 0) -> must fail */
    sig[63] = 0xE0;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, dummy, 10, pk));
    sig[63] = 0x00;

    /* Invalid/off-curve public key (unpackneg returns -1) */
    uint8_t invalid_pk[32];
    memset(invalid_pk, 0xFF, sizeof(invalid_pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, dummy, 10, invalid_pk));

    /* Off-curve public key where chk != num */
    uint8_t off_curve_pk[32] = {0x02};
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, dummy, 10, off_curve_pk));

    /* Large message signing and verification (> 300 bytes) */
    uint8_t large_msg[512];
    for (size_t i = 0; i < sizeof(large_msg); i++) {
        large_msg[i] = (uint8_t)(i & 0xFFU);
    }
    uint8_t valid_pk[32], valid_sk[32], valid_sig[64];
    uint8_t seed3[32] = {0x42};
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(valid_pk, valid_sk, seed3));
    TEST_ASSERT_TRUE(syn_ed25519_sign(large_msg, sizeof(large_msg), valid_sk, valid_pk, valid_sig));
    TEST_ASSERT_TRUE(syn_ed25519_verify(valid_sig, large_msg, sizeof(large_msg), valid_pk));

    /* Corrupted message should fail verification */
    large_msg[10] ^= 0x01;
    TEST_ASSERT_FALSE(syn_ed25519_verify(valid_sig, large_msg, sizeof(large_msg), valid_pk));
    large_msg[10] ^= 0x01;

    /* Corrupted signature should fail verification */
    valid_sig[0] ^= 0x01;
    TEST_ASSERT_FALSE(syn_ed25519_verify(valid_sig, large_msg, sizeof(large_msg), valid_pk));
}

void test_tier_a_ed25519_rfc8032_canonical_checks(void)
{
    static const uint8_t SK[32] = {0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a,
                                   0xf4, 0x92, 0xec, 0x2c, 0xc4, 0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32,
                                   0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60};
    static const uint8_t L_LO0[8] = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58};
    static const uint8_t L_LO1[8] = {0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14};

    uint8_t pk[32];
    uint8_t sig[64];
    uint8_t h_zero[64] = {0};
    TEST_ASSERT_TRUE(syn_ed25519_publickey(SK, pk));
    TEST_ASSERT_TRUE(syn_ed25519_sign(NULL, 0U, SK, pk, sig));

    /* Direct verify_hash null guards */
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(NULL, h_zero, pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(sig, NULL, pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash(sig, h_zero, NULL));

    /* ED25519-1: S == L (top 3 bits are 0 since 0x10 < 0x20, previously bypassed bitmask check) */
    uint8_t bad_sig[64];
    memcpy(bad_sig, sig, sizeof(bad_sig));
    memset(bad_sig + 32, 0, 32U);
    memcpy(bad_sig + 32, L_LO0, 8U);
    memcpy(bad_sig + 40, L_LO1, 8U);
    bad_sig[63] = 0x10U;
    TEST_ASSERT_FALSE(syn_ed25519_verify(bad_sig, NULL, 0U, pk));

    /* S == L - 1 is canonical (reaches curve equation check and fails mismatch) */
    bad_sig[32] = 0xECU;
    TEST_ASSERT_FALSE(syn_ed25519_verify(bad_sig, NULL, 0U, pk));

    /* ED25519-1: non-canonical public key y == p (2^255 - 19) must be rejected */
    uint8_t y_eq_p_pk[32];
    memset(y_eq_p_pk, 0xFFU, sizeof(y_eq_p_pk));
    y_eq_p_pk[0] = 0xEDU;
    y_eq_p_pk[31] = 0x7FU;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, NULL, 0U, y_eq_p_pk));

    /* ED25519-1: negative zero (y == 1 -> x == 0 with sign bit 0x80) must be rejected */
    uint8_t neg_zero_pk[32] = {0};
    neg_zero_pk[0] = 0x01U;
    neg_zero_pk[31] = 0x80U;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, NULL, 0U, neg_zero_pk));

    /* Canonical identity point (y == 1 -> x == 0 with sign bit 0x00) decodes cleanly */
    uint8_t pos_zero_pk[32] = {0};
    pos_zero_pk[0] = 0x01U;
    TEST_ASSERT_FALSE(syn_ed25519_verify(sig, NULL, 0U, pos_zero_pk));
}

typedef struct {
    SYN_PT child_pt;
    SYN_Ed25519_Ctx ed_ctx;
    bool valid;
    uint32_t completed;
} Ed25519VerifyTaskCtx;

static SYN_PT_Status ed25519_verify_parent_task(SYN_PT *pt, SYN_Task *task)
{
    Ed25519VerifyTaskCtx *t = (Ed25519VerifyTaskCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_SPAWN_IF_NEEDED(pt, task, &t->child_pt,
                       syn_ed25519_verify_pt(&t->child_pt, &t->ed_ctx, &t->valid));
    t->completed++;
    PT_END(pt);
}

static SYN_PT_Status ed25519_peer_more_task(SYN_PT *pt, SYN_Task *task)
{
    uint32_t *runs = (uint32_t *)task->user_data;
    (*runs)++;
    PT_BEGIN(pt);
    for (;;) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

void test_ed25519_resumable_steps_and_pt(void)
{
    static const uint8_t SK[32] = {0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a,
                                   0xf4, 0x92, 0xec, 0x2c, 0xc4, 0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32,
                                   0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60};
    uint8_t msg[600];
    for (size_t i = 0U; i < sizeof(msg); i++) {
        msg[i] = (uint8_t)(i & 0xFFU);
    }

    uint8_t exp_pk[32];
    uint8_t out_pk[32];
    uint8_t exp_sig[64];
    uint8_t out_sig[64];
    SYN_Ed25519_Ctx ctx;
    SYN_PT pt;

    TEST_ASSERT_TRUE(syn_ed25519_publickey(SK, exp_pk));
    TEST_ASSERT_TRUE(syn_ed25519_sign(msg, sizeof(msg), SK, exp_pk, exp_sig));

    syn_ed25519_wipe(NULL);
    memset(&ctx, 0, sizeof(ctx));

    /* Public key guards + PT */
    TEST_ASSERT_FALSE(syn_ed25519_publickey_init(NULL, SK));
    TEST_ASSERT_FALSE(syn_ed25519_publickey_init(&ctx, NULL));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_publickey_pt(&pt, &ctx, out_pk));

    TEST_ASSERT_TRUE(syn_ed25519_publickey_init(&ctx, SK));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_publickey_pt(NULL, &ctx, out_pk));
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_TRUE(syn_ed25519_publickey_init(&ctx, SK));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_publickey_pt(&pt, NULL, out_pk));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_publickey_pt(&pt, &ctx, NULL));
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_sign_pt(&pt, &ctx, out_sig));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_verify_pt(&pt, &ctx, NULL));

    TEST_ASSERT_TRUE(syn_ed25519_publickey_init(&ctx, SK));
    PT_INIT(&pt);
    uint32_t yields = 0U;
    while (syn_ed25519_publickey_pt(&pt, &ctx, out_pk) == PT_MORE) {
        yields++;
    }
    TEST_ASSERT_EQUAL_UINT32(641U, yields); /* 1 setup + 512 ladder + 127 inv + 1 pack */
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp_pk, out_pk, 32);

    /* Sign guards + PT (with NULL public_key and explicit public_key) */
    TEST_ASSERT_FALSE(syn_ed25519_sign_init(NULL, msg, sizeof(msg), SK, exp_pk));
    TEST_ASSERT_FALSE(syn_ed25519_sign_init(&ctx, NULL, 1U, SK, exp_pk));
    TEST_ASSERT_FALSE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), NULL, exp_pk));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_sign_pt(&pt, &ctx, out_sig));

    TEST_ASSERT_TRUE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), SK, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_publickey_pt(&pt, &ctx, out_pk));
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_TRUE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), SK, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_sign_pt(NULL, &ctx, out_sig));
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_TRUE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), SK, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_sign_pt(&pt, NULL, out_sig));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_sign_pt(&pt, &ctx, NULL));
    TEST_ASSERT_FALSE(ctx.active);

    TEST_ASSERT_TRUE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), SK, NULL));
    PT_INIT(&pt);
    while (syn_ed25519_sign_pt(&pt, &ctx, out_sig) == PT_MORE) {
    }
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp_sig, out_sig, 64);

    memset(out_sig, 0, sizeof(out_sig));
    TEST_ASSERT_TRUE(syn_ed25519_sign_init(&ctx, msg, sizeof(msg), SK, exp_pk));
    PT_INIT(&pt);
    while (syn_ed25519_sign_pt(&pt, &ctx, out_sig) == PT_MORE) {
    }
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp_sig, out_sig, 64);

    /* Verify & verify_hash guards + PT */
    bool valid = false;
    uint8_t h_digest[64];
    SYN_SHA512 sha;
    syn_sha512_init(&sha);
    syn_sha512_update(&sha, exp_sig, 32U);
    syn_sha512_update(&sha, exp_pk, 32U);
    syn_sha512_update(&sha, msg, sizeof(msg));
    syn_sha512_final(&sha, h_digest);
    TEST_ASSERT_TRUE(syn_ed25519_verify_hash(exp_sig, h_digest, exp_pk));

    uint8_t bad_sig[64];
    memcpy(bad_sig, exp_sig, sizeof(bad_sig));
    bad_sig[63] = 0xE0U;
    uint8_t bad_pk[32];
    memset(bad_pk, 0xFFU, sizeof(bad_pk));

    TEST_ASSERT_FALSE(syn_ed25519_verify_init(NULL, exp_sig, msg, sizeof(msg), exp_pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash_init(NULL, exp_sig, h_digest, exp_pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash_init(&ctx, bad_sig, h_digest, exp_pk));
    TEST_ASSERT_FALSE(syn_ed25519_verify_hash_init(&ctx, exp_sig, h_digest, bad_pk));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_verify_pt(&pt, &ctx, &valid));

    TEST_ASSERT_TRUE(syn_ed25519_verify_init(&ctx, exp_sig, msg, sizeof(msg), exp_pk));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_verify_pt(NULL, &ctx, &valid));
    TEST_ASSERT_FALSE(ctx.active);
    TEST_ASSERT_TRUE(syn_ed25519_verify_init(&ctx, exp_sig, msg, sizeof(msg), exp_pk));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_verify_pt(&pt, NULL, &valid));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_ed25519_verify_pt(&pt, &ctx, NULL));
    TEST_ASSERT_FALSE(ctx.active);

    TEST_ASSERT_TRUE(syn_ed25519_verify_init(&ctx, exp_sig, msg, sizeof(msg), exp_pk));
    PT_INIT(&pt);
    while (syn_ed25519_verify_pt(&pt, &ctx, &valid) == PT_MORE) {
    }
    TEST_ASSERT_TRUE(valid);

    valid = false;
    TEST_ASSERT_TRUE(syn_ed25519_verify_hash_init(&ctx, exp_sig, h_digest, exp_pk));
    PT_INIT(&pt);
    while (syn_ed25519_verify_pt(&pt, &ctx, &valid) == PT_MORE) {
    }
    TEST_ASSERT_TRUE(valid);

    /* PT_SPAWN_IF_NEEDED under SYN_Sched with contended peer */
    Ed25519VerifyTaskCtx tctx;
    memset(&tctx, 0, sizeof(tctx));
    TEST_ASSERT_TRUE(syn_ed25519_verify_init(&tctx.ed_ctx, exp_sig, msg, sizeof(msg), exp_pk));

    uint32_t peer_runs = 0U;
    SYN_Task tasks[2];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "ed25519v", ed25519_verify_parent_task, SYN_PRIO_HIGH, &tctx);
    syn_task_create(&tasks[1], "peer", ed25519_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(0U, tctx.completed);
    TEST_ASSERT_TRUE(PT_IS_RUNNING(&tctx.child_pt));
    while (tctx.completed == 0U) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_GREATER_THAN_UINT32(600U, peer_runs);
    TEST_ASSERT_TRUE(tctx.valid);
}

void run_ed25519_tests(void)
{
    RUN_TEST(test_ed25519_rfc8032_test_vector_1);
    RUN_TEST(test_ed25519_rfc8032_test_vector_2);
    RUN_TEST(test_ed25519_rfc8032_test_vector_3);
    RUN_TEST(test_ed25519_create_keypair_and_roundtrip);
    RUN_TEST(test_ed25519_null_and_bounds_checks);
    RUN_TEST(test_tier_a_ed25519_rfc8032_canonical_checks);
    RUN_TEST(test_ed25519_resumable_steps_and_pt);
}
