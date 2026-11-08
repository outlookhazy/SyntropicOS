/**
 * @file test_cose.c
 * @brief Unit tests for zero-heap CBOR Object Signing and Encryption (COSE - RFC 9052).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_cose.h"
#include "unity/unity.h"

#include <string.h>

void test_cose_sign1_eddsa_roundtrip_and_tamper(void)
{
    /* Generate Ed25519 keypair */
    uint8_t seed[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
                        0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t pubkey[32], seckey[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pubkey, seckey, seed));

    static const uint8_t payload[] = "Sensor Telemetry: Temperature=21.5C, Humidity=45%";
    static const uint8_t kid[] = "node-01";
    static const uint8_t aad[] = "coap://sensors/temp";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;

    /* Sign with EdDSA (-8) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, seckey, pubkey, kid,
                                                        sizeof(kid) - 1, payload,
                                                        sizeof(payload) - 1, aad, sizeof(aad) - 1,
                                                        cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    /* Verify with correct public key and AAD */
    SYN_COSE_Sign1Message parsed;
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey, sizeof(pubkey), aad,
                                                sizeof(aad) - 1, &parsed));

    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_EDDSA, parsed.alg);
    TEST_ASSERT_EQUAL(sizeof(kid) - 1, parsed.kid_len);
    TEST_ASSERT_EQUAL_MEMORY(kid, parsed.kid, parsed.kid_len);
    TEST_ASSERT_EQUAL(sizeof(payload) - 1, parsed.payload_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, parsed.payload, parsed.payload_len);
    TEST_ASSERT_EQUAL(64, parsed.signature_len);

    /* Verify without parsed output struct (NULL) */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey, sizeof(pubkey), aad,
                                                sizeof(aad) - 1, NULL));

    /* Verify with mismatched external AAD -> must fail */
    static const uint8_t bad_aad[] = "coap://sensors/wrong";
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey, sizeof(pubkey),
                                                bad_aad, sizeof(bad_aad) - 1, NULL));

    /* Verify with wrong public key -> must fail */
    uint8_t wrong_pubkey[32];
    memcpy(wrong_pubkey, pubkey, sizeof(pubkey));
    wrong_pubkey[0] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(cose_msg, cose_msg_len, wrong_pubkey,
                                                sizeof(wrong_pubkey), aad, sizeof(aad) - 1, NULL));

    /* Verify with corrupted signature -> must fail */
    cose_msg[cose_msg_len - 1] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey, sizeof(pubkey), aad,
                                                sizeof(aad) - 1, NULL));
}

void test_cose_sign1_es256_roundtrip_and_tamper(void)
{
    /* Generate P-256 keypair */
    static const uint8_t PRIV[32] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                                     0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                     0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                                     0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    uint8_t pub_x[32], pub_y[32];
    TEST_ASSERT_TRUE(syn_p256_base_mul(PRIV, pub_x, pub_y));

    uint8_t pubkey_65[65];
    pubkey_65[0] = 0x04;
    memcpy(pubkey_65 + 1, pub_x, 32);
    memcpy(pubkey_65 + 33, pub_y, 32);

    uint8_t pubkey_64[64];
    memcpy(pubkey_64, pub_x, 32);
    memcpy(pubkey_64 + 32, pub_y, 32);

    static const uint8_t payload[] = "Actuator Command: Valve=OPEN";
    static const uint8_t kid[] = "act-p256";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;

    /* Sign with ES256 (-7) */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_sign1_create(SYN_COSE_ALGO_ES256, PRIV, pubkey_65, kid,
                                                sizeof(kid) - 1, payload, sizeof(payload) - 1, NULL,
                                                0, cose_msg, sizeof(cose_msg), &cose_msg_len));

    /* Verify with 65-byte uncompressed pubkey */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey_65, 65, NULL, 0, NULL));

    /* Verify with 64-byte raw pubkey */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey_64, 64, NULL, 0, NULL));

    /* Wrong key length -> error */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey_65, 63, NULL, 0, NULL));

    /* Tampered signature -> error */
    cose_msg[cose_msg_len - 5] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_sign1_verify(cose_msg, cose_msg_len, pubkey_65, 65, NULL, 0, NULL));
}

void test_cose_encrypt0_chacha20poly1305_roundtrip_and_tamper(void)
{
    static const uint8_t key[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    static const uint8_t iv[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                   0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    static const uint8_t kid[] = "sym-node";
    static const uint8_t plaintext[] = "Confidential Industrial Control Message 0x12345678";
    static const uint8_t aad[] = "coap://valve/control";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;

    /* Create COSE_Encrypt0 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_CHACHA20_POLY1305, key, iv, sizeof(iv), kid,
                                      sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    /* Decrypt COSE_Encrypt0 */
    uint8_t dec_buf[128];
    size_t dec_len = 0;
    SYN_COSE_Encrypt0Message parsed;
    memset(&parsed, 0, sizeof(parsed));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));

    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_CHACHA20_POLY1305, parsed.alg);
    TEST_ASSERT_EQUAL(12, parsed.iv_len);
    TEST_ASSERT_EQUAL_MEMORY(iv, parsed.iv, 12);
    TEST_ASSERT_EQUAL(sizeof(kid) - 1, parsed.kid_len);
    TEST_ASSERT_EQUAL_MEMORY(kid, parsed.kid, parsed.kid_len);

    /* Decrypt with wrong key -> must fail authentication */
    uint8_t bad_key[32];
    memcpy(bad_key, key, 32);
    bad_key[0] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, bad_key, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));

    /* Decrypt with mismatched AAD -> must fail */
    static const uint8_t wrong_aad[] = "coap://valve/wrong";
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key, wrong_aad,
                                                    sizeof(wrong_aad) - 1, dec_buf, sizeof(dec_buf),
                                                    &dec_len, NULL));

    /* Corrupted ciphertext -> must fail */
    cose_msg[cose_msg_len - 1] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));
}

void test_cose_null_and_boundary_checks(void)
{
    uint8_t dummy[64] = {0};
    size_t out_len = 0;

    /* syn_cose_sign1_create null checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, NULL, dummy, NULL, 0, dummy,
                                                10, NULL, 0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, dummy, dummy, NULL, 0, dummy,
                                                10, NULL, 0, NULL, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, dummy, dummy, NULL, 0, dummy,
                                                10, NULL, 0, dummy, sizeof(dummy), NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, dummy, dummy, NULL, 0, NULL,
                                                10, NULL, 0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_create((SYN_COSE_Algorithm)999, dummy, dummy,
                                                           NULL, 0, dummy, 10, NULL, 0, dummy,
                                                           sizeof(dummy), &out_len));

    /* Buffer too small for sign1 create */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, dummy, dummy, NULL, 0, dummy,
                                                10, NULL, 0, dummy, 5, &out_len));

    /* syn_cose_sign1_verify null checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(NULL, 10, dummy, 32, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(dummy, 0, dummy, 32, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(dummy, 10, NULL, 32, NULL, 0, NULL));

    /* Invalid CBOR / non-array message */
    uint8_t non_array_cbor[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(non_array_cbor, sizeof(non_array_cbor),
                                                           dummy, 32, NULL, 0, NULL));

    /* syn_cose_encrypt0_create null checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, NULL,
                                                              dummy, 12, NULL, 0, dummy, 10, NULL,
                                                              0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, dummy, NULL, 12, NULL,
                                            0, dummy, 10, NULL, 0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, dummy, dummy, 10, NULL,
                                            0, dummy, 10, NULL, 0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_create((SYN_COSE_Algorithm)999, dummy, dummy,
                                                              12, NULL, 0, dummy, 10, NULL, 0,
                                                              dummy, sizeof(dummy), &out_len));

    /* syn_cose_encrypt0_decrypt null checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(NULL, 10, dummy, NULL, 0, dummy,
                                                               sizeof(dummy), &out_len, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(dummy, 0, dummy, NULL, 0, dummy,
                                                               sizeof(dummy), &out_len, NULL));

    /* CCM invalid IV length checks (create) */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_AES_CCM_16_64_128, dummy, dummy, 6, NULL,
                                            0, dummy, 10, NULL, 0, dummy, sizeof(dummy), &out_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_AES_CCM_16_64_128, dummy, dummy, 14, NULL,
                                            0, dummy, 10, NULL, 0, dummy, sizeof(dummy), &out_len));

    /* CCM invalid IV length checks (decrypt) */
    uint8_t bad_ccm_cbor[128];
    SYN_CborWriter cw;
    syn_cbor_writer_init(&cw, bad_ccm_cbor, sizeof(bad_ccm_cbor));
    syn_cbor_write_array_begin(&cw, 3);
    uint8_t prot[16];
    SYN_CborWriter pw;
    syn_cbor_writer_init(&pw, prot, sizeof(prot));
    syn_cbor_write_map_begin(&pw, 1);
    syn_cbor_write_uint(&pw, SYN_COSE_HEADER_ALG);
    syn_cbor_write_uint(&pw, SYN_COSE_ALGO_AES_CCM_16_64_128);
    syn_cbor_write_bytes(&cw, prot, syn_cbor_writer_len(&pw));
    syn_cbor_write_map_begin(&cw, 1);
    syn_cbor_write_uint(&cw, SYN_COSE_HEADER_IV);
    uint8_t short_iv[5] = {1, 2, 3, 4, 5};
    syn_cbor_write_bytes(&cw, short_iv, sizeof(short_iv));
    uint8_t dummy_ct[16] = {0};
    syn_cbor_write_bytes(&cw, dummy_ct, sizeof(dummy_ct));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(bad_ccm_cbor, syn_cbor_writer_len(&cw), dummy,
                                                    NULL, 0, dummy, sizeof(dummy), &out_len, NULL));

    /* Empty plaintext encryption and decryption (payload = NULL, length = 0) */
    uint8_t empty_ct_msg[256];
    size_t empty_ct_len = 0;
    static const uint8_t key32[32] = {0x42};
    static const uint8_t iv12[12] = {0x24};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, key32, iv12, 12,
                                                   NULL, 0, NULL, 0, NULL, 0, empty_ct_msg,
                                                   sizeof(empty_ct_msg), &empty_ct_len));
    uint8_t empty_dec[16];
    size_t empty_dec_len = 99;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(empty_ct_msg, empty_ct_len, key32, NULL,
                                                            0, empty_dec, sizeof(empty_dec),
                                                            &empty_dec_len, NULL));
    TEST_ASSERT_EQUAL(0, empty_dec_len);

    /* Output plaintext buffer too small -> error */
    uint8_t small_dec[2];
    size_t small_dec_len = 0;
    uint8_t non_empty_msg[256];
    size_t non_empty_len = 0;
    static const uint8_t sample_pt[] = "0123456789";
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, key32, iv12, 12,
                                                   NULL, 0, sample_pt, 10, NULL, 0, non_empty_msg,
                                                   sizeof(non_empty_msg), &non_empty_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(non_empty_msg, non_empty_len, key32, NULL, 0,
                                                    small_dec, 5, &small_dec_len, NULL));

    /* Empty payload signing (payload = NULL, len = 0) */
    uint8_t ed_seed[32] = {0x99};
    uint8_t ed_pub[32], ed_sec[32];
    syn_ed25519_create_keypair(ed_pub, ed_sec, ed_seed);
    uint8_t empty_sign_msg[256];
    size_t empty_sign_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, ed_sec, ed_pub, NULL,
                                                        0, NULL, 0, NULL, 0, empty_sign_msg,
                                                        sizeof(empty_sign_msg), &empty_sign_len));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_sign1_verify(empty_sign_msg, empty_sign_len, ed_pub, 32, NULL, 0, NULL));

    /* Invalid protected alg for sign1 (e.g. alg = 99) */
    uint8_t bad_alg_sign1[64] = {
        0x84,                         /* Array of 4 */
        0x44, 0xA1, 0x01, 0x18, 0x63, /* Protected: { 1: 99 } */
        0xA0,                         /* Unprotected: {} */
        0x40,                         /* Payload: "" */
        0x40                          /* Signature */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(bad_alg_sign1, sizeof(bad_alg_sign1),
                                                           ed_pub, 32, NULL, 0, NULL));

    /* Invalid protected alg for encrypt0 (e.g. alg = -8) */
    uint8_t bad_alg_enc0[64] = {
        0x83,                                                    /* Array of 3 */
        0x43, 0xA1, 0x01, 0x27,                                  /* Protected: { 1: -8 } */
        0xA1, 0x05, 0x4C, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Unprotected: { 5: 12-byte IV }
                                                                  */
        0x50, 0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 16-byte tag */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_decrypt(bad_alg_enc0, sizeof(bad_alg_enc0), key32, NULL, 0,
                                             small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Invalid IV length (e.g. 8 bytes instead of 12) */
    uint8_t bad_iv_enc0[64] = {
        0x83,                                           /* Array of 3 */
        0x44, 0xA1, 0x01, 0x18, 0x18,                   /* Protected: { 1: 24 } */
        0xA1, 0x05, 0x48, 0,    0,    0, 0, 0, 0, 0, 0, /* Unprotected: { 5: 8-byte IV } */
        0x50, 0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 16-byte tag */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_decrypt(bad_iv_enc0, sizeof(bad_iv_enc0), key32, NULL, 0,
                                             small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Truncated ciphertext (< 16 bytes) */
    uint8_t short_ct_enc0[64] = {
        0x83,                         /* Array of 3 */
        0x44, 0xA1, 0x01, 0x18, 0x18, /* Protected: { 1: 24 } */
        0xA1, 0x05, 0x4C, 0,    0,    0, 0, 0, 0,
        0,    0,    0,    0,    0,    0,         /* Unprotected: { 5: 12-byte IV } */
        0x48, 0,    0,    0,    0,    0, 0, 0, 0 /* 8-byte ciphertext */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_decrypt(short_ct_enc0, sizeof(short_ct_enc0), key32, NULL, 0,
                                             small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Extra/unknown parameters in header test */
    uint8_t custom_sign1[64] = {
        0x84,                               /* Array of 4 */
        0x45, 0xA2, 0x01, 0x27, 0x0A, 0x05, /* Protected: { 1: -8, 10: 5 } */
        0xA1, 0x0B, 0x06,                   /* Unprotected: { 11: 6 } */
        0x40,                               /* Empty payload */
        0x40                                /* Short signature -> fails sig read */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(custom_sign1, sizeof(custom_sign1),
                                                           ed_pub, 32, NULL, 0, NULL));

    /* Large payload exceeding 1024-byte sig_structure buffer */
    uint8_t huge_payload[1100];
    memset(huge_payload, 0xAA, sizeof(huge_payload));
    uint8_t huge_out[1200];
    size_t huge_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, ed_sec, ed_pub, NULL, 0,
                                                huge_payload, sizeof(huge_payload), NULL, 0,
                                                huge_out, sizeof(huge_out), &huge_len));

    /* Large plaintext exceeding 1024-byte ct_tag_buf */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, key32, iv12, 12,
                                                   NULL, 0, huge_payload, sizeof(huge_payload),
                                                   NULL, 0, huge_out, sizeof(huge_out), &huge_len));

    /* Small output buffer for encrypt0 create */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305,
                                                              key32, iv12, 12, NULL, 0, sample_pt,
                                                              10, NULL, 0, huge_out, 5, &huge_len));

    /* Protected header is not a bstr in sign1 */
    uint8_t not_bstr_prot_sign1[] = {0x84, 0x01, 0xA0, 0x40, 0x40};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(not_bstr_prot_sign1, sizeof(not_bstr_prot_sign1),
                                                ed_pub, 32, NULL, 0, NULL));

    /* Unprotected header is not a map in sign1 */
    uint8_t not_map_unprot_sign1[] = {0x84, 0x43, 0xA1, 0x01, 0x27, 0x01, 0x40, 0x40};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(not_map_unprot_sign1, sizeof(not_map_unprot_sign1),
                                                ed_pub, 32, NULL, 0, NULL));

    /* Payload is not a bstr in sign1 */
    uint8_t not_bstr_payload_sign1[] = {0x84, 0x43, 0xA1, 0x01, 0x27, 0xA0, 0x01, 0x40};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(not_bstr_payload_sign1,
                                                           sizeof(not_bstr_payload_sign1), ed_pub,
                                                           32, NULL, 0, NULL));

    /* Protected header is not a bstr in encrypt0 */
    uint8_t not_bstr_prot_enc0[] = {0x83, 0x01, 0xA0, 0x40};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(not_bstr_prot_enc0, sizeof(not_bstr_prot_enc0),
                                                    key32, NULL, 0, small_dec, sizeof(small_dec),
                                                    &small_dec_len, NULL));

    /* Encrypt0 array item count != 3 (e.g. array of 2) */
    uint8_t wrong_array_enc0[] = {0x82, 0x44, 0xA1, 0x01, 0x18, 0x18, 0xA0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(
                                         wrong_array_enc0, sizeof(wrong_array_enc0), key32, NULL, 0,
                                         small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Huge external AAD > 512 bytes for Encrypt0 create -> build_enc_structure fails */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, key32, iv12, 12, NULL,
                                            0, sample_pt, 10, huge_payload, sizeof(huge_payload),
                                            huge_out, sizeof(huge_out), &huge_len));

    /* EdDSA verify with invalid public key length (!= 32) */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(empty_sign_msg, empty_sign_len, ed_pub,
                                                           16, NULL, 0, NULL));

    /* Protected header with extra/unknown labels in Encrypt0 */
    uint8_t extra_prot_enc0[64] = {
        0x83,                                     /* Array of 3 */
        0x45, 0xA2, 0x01, 0x18, 0x18, 0x0A, 0x05, /* Protected: { 1: 24, 10: 5 } */
        0xA1, 0x05, 0x4C, 0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0,      /* Unprotected: { 5:
                                                                                * 12-byte IV }
                                                                                */
        0x50, 0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 16-byte tag */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(
                                         extra_prot_enc0, sizeof(extra_prot_enc0), key32, NULL, 0,
                                         small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Unprotected header is not a map in Encrypt0 */
    uint8_t not_map_unprot_enc0[] = {
        0x83,                         /* Array of 3 */
        0x44, 0xA1, 0x01, 0x18, 0x18, /* Protected: { 1: 24 } */
        0x01,                         /* Unprotected: 1 (not a map) */
        0x50, 0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 16-byte tag */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR,
        syn_cose_encrypt0_decrypt(not_map_unprot_enc0, sizeof(not_map_unprot_enc0), key32, NULL, 0,
                                  small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Unprotected header with extra/unknown labels in Encrypt0 */
    uint8_t extra_unprot_enc0[64] = {
        0x83,                                                       /* Array of 3 */
        0x44, 0xA1, 0x01, 0x18, 0x18,                               /* Protected: { 1: 24 } */
        0xA2, 0x05, 0x4C, 0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Unprotected: { 5: 12-byte IV,
                                                                    10: 5 } */
        0x0A, 0x05, 0x50, 0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 16-byte tag */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(
                                         extra_unprot_enc0, sizeof(extra_unprot_enc0), key32, NULL,
                                         0, small_dec, sizeof(small_dec), &small_dec_len, NULL));

    /* Huge external AAD > 512 bytes for Encrypt0 decrypt -> build_enc_structure fails */
    uint8_t dec_buf[256];
    size_t dec_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(non_empty_msg, non_empty_len, key32,
                                                    huge_payload, sizeof(huge_payload), dec_buf,
                                                    sizeof(dec_buf), &dec_len, NULL));

    /* Huge external AAD > 1024 bytes for Sign1 verify */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(empty_sign_msg, empty_sign_len, ed_pub, 32,
                                                huge_payload, sizeof(huge_payload), NULL));

    /* syn_cose_mac0_create null and boundary checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, NULL, 32,
                                                          NULL, 0, sample_pt, 10, NULL, 0, huge_out,
                                                          sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 0,
                                                          NULL, 0, sample_pt, 10, NULL, 0, huge_out,
                                                          sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32,
                                                          NULL, 0, sample_pt, 10, NULL, 0, NULL,
                                                          sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32,
                                                          NULL, 0, sample_pt, 10, NULL, 0, huge_out,
                                                          sizeof(huge_out), NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32, NULL, 0, NULL,
                                               10, NULL, 0, huge_out, sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_create((SYN_COSE_Algorithm)999, key32, 32, NULL,
                                                          0, sample_pt, 10, NULL, 0, huge_out,
                                                          sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32, NULL, 0,
                                               sample_pt, 10, NULL, 0, huge_out, 5, &huge_len));

    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32, NULL, 0,
                                               huge_payload, sizeof(huge_payload), NULL, 0,
                                               huge_out, sizeof(huge_out), &huge_len));

    /* syn_cose_mac0_verify null and boundary checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(NULL, 10, key32, 32, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(huge_out, 0, key32, 32, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(huge_out, 10, NULL, 32, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(huge_out, 10, key32, 0, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(not_bstr_prot_sign1, sizeof(not_bstr_prot_sign1),
                                               key32, 32, NULL, 0, NULL));

    /* MAC0 array items != 4 (array of 3) */
    uint8_t mac0_arr3[] = {0x83, 0x44, 0xA1, 0x01, 0x05, 0xA0, 0x40};
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_mac0_verify(mac0_arr3, sizeof(mac0_arr3), key32, 32, NULL, 0, NULL));

    /* MAC0 protected header with unknown field (skip) and unsupported alg */
    uint8_t mac0_bad_alg[64] = {
        0x84,                                     /* Array of 4 */
        0x45, 0xA2, 0x01, 0x18, 0x63, 0x0A, 0x05, /* Protected: { 1: 99, 10: 5 } */
        0xA0,                                     /* Unprotected: {} */
        0x40,                                     /* Payload: "" */
        0x40                                      /* Tag */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(mac0_bad_alg, sizeof(mac0_bad_alg), key32,
                                                          32, NULL, 0, NULL));

    /* MAC0 unprotected header not a map */
    uint8_t mac0_not_map_unprot[64] = {
        0x84,                   /* Array of 4 */
        0x43, 0xA1, 0x01, 0x05, /* Protected: { 1: 5 } (HMAC_256_256) */
        0x01,                   /* Unprotected: 1 (not map) */
        0x40,                   /* Payload: "" */
        0x40                    /* Tag */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(mac0_not_map_unprot, sizeof(mac0_not_map_unprot),
                                               key32, 32, NULL, 0, NULL));

    /* MAC0 unprotected header with unknown field (skip) */
    uint8_t mac0_unprot_skip[64] = {
        0x84,                   /* Array of 4 */
        0x43, 0xA1, 0x01, 0x05, /* Protected: { 1: 5 } */
        0xA1, 0x0B, 0x05,       /* Unprotected: { 11: 5 } */
        0x40,                   /* Payload: "" */
        0x40                    /* Tag (short -> tag mismatch) */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR,
        syn_cose_mac0_verify(mac0_unprot_skip, sizeof(mac0_unprot_skip), key32, 32, NULL, 0, NULL));

    /* MAC0 payload not a byte string */
    uint8_t mac0_payload_not_bstr[64] = {
        0x84,                   /* Array of 4 */
        0x43, 0xA1, 0x01, 0x05, /* Protected: { 1: 5 } */
        0xA0,                   /* Unprotected: {} */
        0x01,                   /* Payload: 1 (not bstr) */
        0x40                    /* Tag */
    };
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(mac0_payload_not_bstr, sizeof(mac0_payload_not_bstr),
                                               key32, 32, NULL, 0, NULL));

    /* MAC0 huge external AAD > 1024 bytes */
    uint8_t valid_mac0_msg[256];
    size_t valid_mac0_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key32, 32, NULL,
                                                       0, sample_pt, 10, NULL, 0, valid_mac0_msg,
                                                       sizeof(valid_mac0_msg), &valid_mac0_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(valid_mac0_msg, valid_mac0_len, key32, 32,
                                               huge_payload, sizeof(huge_payload), NULL));

    /* syn_cose_key_encode & decode null checks */
    SYN_COSE_Key test_key;
    memset(&test_key, 0, sizeof(test_key));
    test_key.kty = SYN_COSE_KTY_SYMMETRIC;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_encode(NULL, huge_out, sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_encode(&test_key, NULL, sizeof(huge_out), &huge_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_encode(&test_key, huge_out, sizeof(huge_out), NULL));
    test_key.kty = (SYN_COSE_KeyType)99;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_encode(&test_key, huge_out, sizeof(huge_out), &huge_len));

    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(NULL, 10, &test_key));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(huge_out, 0, &test_key));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(huge_out, 10, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(not_bstr_prot_sign1,
                                                         sizeof(not_bstr_prot_sign1), &test_key));
}

void test_cose_encrypt0_aes_gcm_roundtrip_and_tamper(void)
{
    static const uint8_t key128[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    static const uint8_t key192[24] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                       0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    static const uint8_t key256[32] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
                                       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
    static const uint8_t iv[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                                   0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};
    static const uint8_t kid[] = "gcm-node-1";
    static const uint8_t plaintext[] =
        "Confidential AES-GCM Encrypted Industrial Telemetry Payload";
    static const uint8_t aad[] = "coap://valve/status";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;
    uint8_t dec_buf[128];
    size_t dec_len = 0;
    SYN_COSE_Encrypt0Message parsed;

    /* 1. Test AES-128-GCM (A128GCM, alg = 1) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_A128GCM, key128, iv, sizeof(iv), kid,
                                      sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key128, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_A128GCM, parsed.alg);

    /* 2. Test AES-192-GCM (A192GCM, alg = 2) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_A192GCM, key192, iv, sizeof(iv), kid,
                                      sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key192, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_A192GCM, parsed.alg);

    /* 3. Test AES-256-GCM (A256GCM, alg = 3) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_A256GCM, key256, iv, sizeof(iv), kid,
                                      sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key256, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_A256GCM, parsed.alg);

    /* 4. Tampering & verification failure tests */
    /* Wrong key */
    uint8_t bad_key[32];
    memcpy(bad_key, key256, 32);
    bad_key[0] ^= 0x01;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, bad_key, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));

    /* Corrupted ciphertext byte */
    cose_msg[cose_msg_len - 1] ^= 0x55;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key256, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));

    /* Empty plaintext with A256GCM */
    uint8_t empty_msg[256];
    size_t empty_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_A256GCM, key256, iv, sizeof(iv), NULL, 0, NULL,
                                      0, NULL, 0, empty_msg, sizeof(empty_msg), &empty_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_encrypt0_decrypt(empty_msg, empty_len, key256, NULL, 0, dec_buf,
                                                    sizeof(dec_buf), &dec_len, NULL));
    TEST_ASSERT_EQUAL(0, dec_len);
}

void test_cose_encrypt0_aes_ccm_roundtrip_and_tamper(void)
{
    static const uint8_t key128[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    static const uint8_t key256[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    static const uint8_t iv13[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                     0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
    static const uint8_t kid[] = "ccm-node-42";
    static const uint8_t plaintext[] = "Secure IEEE 802.15.4 / Thread Telemetry Payload";
    static const uint8_t aad[] = "coap://sensor/temperature";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;
    uint8_t dec_buf[128];
    size_t dec_len = 0;
    SYN_COSE_Encrypt0Message parsed;

    /* 1. AES-CCM-16-64-128 (alg = 10, 128-bit key, 64-bit tag, 13-byte nonce) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_AES_CCM_16_64_128, key128, iv13, sizeof(iv13),
                                      kid, sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key128, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_AES_CCM_16_64_128, parsed.alg);
    TEST_ASSERT_EQUAL(13, parsed.iv_len);
    TEST_ASSERT_EQUAL_MEMORY(iv13, parsed.iv, 13);

    /* 2. AES-CCM-16-64-256 (alg = 11, 256-bit key, 64-bit tag) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_AES_CCM_16_64_256, key256, iv13, sizeof(iv13),
                                      kid, sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key256, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_AES_CCM_16_64_256, parsed.alg);

    /* 3. AES-CCM-16-128-128 (alg = 30, 128-bit key, 128-bit tag) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_AES_CCM_16_128_128, key128, iv13, sizeof(iv13),
                                      kid, sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key128, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_AES_CCM_16_128_128, parsed.alg);

    /* 4. AES-CCM-16-128-256 (alg = 31, 256-bit key, 128-bit tag) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(
                                      SYN_COSE_ALGO_AES_CCM_16_128_256, key256, iv13, sizeof(iv13),
                                      kid, sizeof(kid) - 1, plaintext, sizeof(plaintext) - 1, aad,
                                      sizeof(aad) - 1, cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key256, aad,
                                                            sizeof(aad) - 1, dec_buf,
                                                            sizeof(dec_buf), &dec_len, &parsed));
    TEST_ASSERT_EQUAL(sizeof(plaintext) - 1, dec_len);
    TEST_ASSERT_EQUAL_MEMORY(plaintext, dec_buf, dec_len);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_AES_CCM_16_128_256, parsed.alg);

    /* 5. Tampering tests */
    /* Bad key */
    uint8_t bad_key[32];
    memcpy(bad_key, key256, 32);
    bad_key[0] ^= 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, bad_key, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));

    /* Bad tag */
    cose_msg[cose_msg_len - 1] ^= 0xAA;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(cose_msg, cose_msg_len, key256, aad,
                                                               sizeof(aad) - 1, dec_buf,
                                                               sizeof(dec_buf), &dec_len, NULL));
}

void test_cose_mac0_hmac_roundtrip_and_tamper(void)
{
    static const uint8_t key[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    static const uint8_t kid[] = "mac-node-01";
    static const uint8_t payload[] = "COSE_Mac0 Authenticated Telemetry Packet";
    static const uint8_t aad[] = "sensor://temperature/celsius";

    uint8_t cose_msg[512];
    size_t cose_msg_len = 0;
    SYN_COSE_Mac0Message parsed;

    /* 1. HMAC-256/256 (32-byte tag) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key, sizeof(key),
                                                       kid, sizeof(kid) - 1, payload,
                                                       sizeof(payload) - 1, aad, sizeof(aad) - 1,
                                                       cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_verify(cose_msg, cose_msg_len, key, sizeof(key),
                                                       aad, sizeof(aad) - 1, &parsed));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_HMAC_256_256, parsed.alg);
    TEST_ASSERT_EQUAL(sizeof(kid) - 1, parsed.kid_len);
    TEST_ASSERT_EQUAL_MEMORY(kid, parsed.kid, parsed.kid_len);
    TEST_ASSERT_EQUAL(sizeof(payload) - 1, parsed.payload_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, parsed.payload, parsed.payload_len);
    TEST_ASSERT_EQUAL(32, parsed.tag_len);

    /* 2. HMAC-256/64 (8-byte tag) */
    memset(&parsed, 0, sizeof(parsed));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_64, key, sizeof(key), NULL, 0,
                                               payload, sizeof(payload) - 1, NULL, 0, cose_msg,
                                               sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_GREATER_THAN(0, cose_msg_len);

    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_mac0_verify(cose_msg, cose_msg_len, key, sizeof(key), NULL, 0, &parsed));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_HMAC_256_64, parsed.alg);
    TEST_ASSERT_EQUAL(0, parsed.kid_len);
    TEST_ASSERT_EQUAL(8, parsed.tag_len);

    /* 3. Empty payload */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, key, sizeof(key), kid,
                                               sizeof(kid) - 1, NULL, 0, aad, sizeof(aad) - 1,
                                               cose_msg, sizeof(cose_msg), &cose_msg_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_verify(cose_msg, cose_msg_len, key, sizeof(key),
                                                       aad, sizeof(aad) - 1, &parsed));
    TEST_ASSERT_EQUAL(0, parsed.payload_len);

    /* 4. Tampering tests */
    /* Wrong key */
    uint8_t bad_key[32];
    memcpy(bad_key, key, 32);
    bad_key[0] ^= 0x55;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(cose_msg, cose_msg_len, bad_key, sizeof(bad_key),
                                               aad, sizeof(aad) - 1, NULL));

    /* Wrong AAD */
    static const uint8_t wrong_aad[] = "sensor://pressure/pascal";
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(cose_msg, cose_msg_len, key, sizeof(key),
                                                          wrong_aad, sizeof(wrong_aad) - 1, NULL));

    /* Tampered tag byte */
    cose_msg[cose_msg_len - 1] ^= 0xFF;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(cose_msg, cose_msg_len, key, sizeof(key),
                                                          aad, sizeof(aad) - 1, NULL));
}

void test_cose_key_encode_decode(void)
{
    /* 1. Symmetric Key */
    SYN_COSE_Key sym_key;
    memset(&sym_key, 0, sizeof(sym_key));
    sym_key.kty = SYN_COSE_KTY_SYMMETRIC;
    sym_key.alg = SYN_COSE_ALGO_CHACHA20_POLY1305;
    memcpy(sym_key.kid, "key-sym-01", 10);
    sym_key.kid_len = 10;
    memset(sym_key.priv_d, 0x42, 32);
    sym_key.priv_d_len = 32;

    uint8_t cbor[256];
    size_t cbor_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_encode(&sym_key, cbor, sizeof(cbor), &cbor_len));
    TEST_ASSERT_GREATER_THAN(0, cbor_len);

    SYN_COSE_Key decoded;
    memset(&decoded, 0, sizeof(decoded));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_decode(cbor, cbor_len, &decoded));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_KTY_SYMMETRIC, decoded.kty);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_CHACHA20_POLY1305, decoded.alg);
    TEST_ASSERT_EQUAL(10, decoded.kid_len);
    TEST_ASSERT_EQUAL_MEMORY("key-sym-01", decoded.kid, 10);
    TEST_ASSERT_EQUAL(32, decoded.priv_d_len);
    TEST_ASSERT_EQUAL_MEMORY(sym_key.priv_d, decoded.priv_d, 32);

    /* 2. OKP Ed25519 Key */
    SYN_COSE_Key okp_key;
    memset(&okp_key, 0, sizeof(okp_key));
    okp_key.kty = SYN_COSE_KTY_OKP;
    okp_key.crv = SYN_COSE_CRV_ED25519;
    okp_key.alg = SYN_COSE_ALGO_EDDSA;
    memcpy(okp_key.kid, "ed25519-id", 10);
    okp_key.kid_len = 10;
    memset(okp_key.pub_x, 0x33, 32);
    okp_key.pub_x_len = 32;
    memset(okp_key.priv_d, 0x77, 32);
    okp_key.priv_d_len = 32;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_encode(&okp_key, cbor, sizeof(cbor), &cbor_len));
    memset(&decoded, 0, sizeof(decoded));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_decode(cbor, cbor_len, &decoded));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_KTY_OKP, decoded.kty);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_CRV_ED25519, decoded.crv);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_EDDSA, decoded.alg);
    TEST_ASSERT_EQUAL(32, decoded.pub_x_len);
    TEST_ASSERT_EQUAL_MEMORY(okp_key.pub_x, decoded.pub_x, 32);
    TEST_ASSERT_EQUAL(32, decoded.priv_d_len);
    TEST_ASSERT_EQUAL_MEMORY(okp_key.priv_d, decoded.priv_d, 32);

    /* 3. EC2 P-256 Key (with private key priv_d) */
    SYN_COSE_Key ec2_key;
    memset(&ec2_key, 0, sizeof(ec2_key));
    ec2_key.kty = SYN_COSE_KTY_EC2;
    ec2_key.crv = SYN_COSE_CRV_P256;
    ec2_key.alg = SYN_COSE_ALGO_ES256;
    memset(ec2_key.pub_x, 0x11, 32);
    ec2_key.pub_x_len = 32;
    memset(ec2_key.pub_y, 0x22, 32);
    ec2_key.pub_y_len = 32;
    memset(ec2_key.priv_d, 0x99, 32);
    ec2_key.priv_d_len = 32;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_encode(&ec2_key, cbor, sizeof(cbor), &cbor_len));
    memset(&decoded, 0, sizeof(decoded));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_key_decode(cbor, cbor_len, &decoded));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_KTY_EC2, decoded.kty);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_CRV_P256, decoded.crv);
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_ES256, decoded.alg);
    TEST_ASSERT_EQUAL(32, decoded.pub_x_len);
    TEST_ASSERT_EQUAL_MEMORY(ec2_key.pub_x, decoded.pub_x, 32);
    TEST_ASSERT_EQUAL(32, decoded.pub_y_len);
    TEST_ASSERT_EQUAL_MEMORY(ec2_key.pub_y, decoded.pub_y, 32);
    TEST_ASSERT_EQUAL(32, decoded.priv_d_len);
    TEST_ASSERT_EQUAL_MEMORY(ec2_key.priv_d, decoded.priv_d, 32);

    /* 4. Encode with small buffer failure */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_encode(&ec2_key, cbor, 5, &cbor_len));

    /* 5. Decode with unknown map label (skips unknown) */
    uint8_t unknown_lbl_cbor[64];
    SYN_CborWriter kw;
    syn_cbor_writer_init(&kw, unknown_lbl_cbor, sizeof(unknown_lbl_cbor));
    syn_cbor_write_map_begin(&kw, 2);
    syn_cbor_write_int(&kw, SYN_COSE_KEY_KTY);
    syn_cbor_write_int(&kw, SYN_COSE_KTY_SYMMETRIC);
    syn_cbor_write_int(&kw, 999);
    syn_cbor_write_text_cstr(&kw, "unknown_field");
    memset(&decoded, 0, sizeof(decoded));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_key_decode(unknown_lbl_cbor, syn_cbor_writer_len(&kw), &decoded));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_KTY_SYMMETRIC, decoded.kty);
}

void test_cose_zero_copy_views_and_oversized_kid(void)
{
    uint8_t seed[32] = {0xAAU};
    uint8_t pub[32], sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(pub, sec, seed));
    static const uint8_t sym_key[32] = {0x55U};
    static const uint8_t iv[12] = {0x11U};
    static const uint8_t msg_body[] = "zero-copy-slice";

    uint8_t wire[256];
    size_t wire_len = 0U;

    /* 1. Sign1 zero-copy pointers point into wire[] */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, sec, pub, NULL, 0U,
                                                        msg_body, sizeof(msg_body) - 1U, NULL, 0U,
                                                        wire, sizeof(wire), &wire_len));
    SYN_COSE_Sign1Message s1;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_sign1_verify(wire, wire_len, pub, 32U, NULL, 0U, &s1));
    TEST_ASSERT_TRUE(s1.payload >= wire && (s1.payload + s1.payload_len) <= (wire + wire_len));
    TEST_ASSERT_TRUE(s1.signature >= wire &&
                     (s1.signature + s1.signature_len) <= (wire + wire_len));

    /* 2. Encrypt0 zero-copy ciphertext pointer points into wire[] */
    uint8_t pt_out[64];
    size_t pt_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, sym_key,
                                                           iv, sizeof(iv), NULL, 0U, msg_body,
                                                           sizeof(msg_body) - 1U, NULL, 0U, wire,
                                                           sizeof(wire), &wire_len));
    SYN_COSE_Encrypt0Message e0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(wire, wire_len, sym_key, NULL, 0U,
                                                            pt_out, sizeof(pt_out), &pt_len, &e0));
    TEST_ASSERT_TRUE(e0.ciphertext >= wire &&
                     (e0.ciphertext + e0.ciphertext_len) <= (wire + wire_len));

    /* 3. Mac0 zero-copy payload and tag pointers point into wire[] */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, sym_key, 32U,
                                                       NULL, 0U, msg_body, sizeof(msg_body) - 1U,
                                                       NULL, 0U, wire, sizeof(wire), &wire_len));
    SYN_COSE_Mac0Message m0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_mac0_verify(wire, wire_len, sym_key, 32U, NULL, 0U, &m0));
    TEST_ASSERT_TRUE(m0.payload >= wire && (m0.payload + m0.payload_len) <= (wire + wire_len));
    TEST_ASSERT_TRUE(m0.tag >= wire && (m0.tag + m0.tag_len) <= (wire + wire_len));

    /* 4. Oversized kid (> SYN_COSE_MAX_KID_LEN = 32B) rejected in Sign1, Encrypt0, and Mac0 */
    uint8_t big_kid[40];
    memset(big_kid, 0x4BU, sizeof(big_kid));

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, sec, pub, big_kid,
                                                sizeof(big_kid), msg_body, sizeof(msg_body) - 1U,
                                                NULL, 0U, wire, sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(wire, wire_len, pub, 32U, NULL, 0U, &s1));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, sym_key,
                                                           iv, sizeof(iv), big_kid, sizeof(big_kid),
                                                           msg_body, sizeof(msg_body) - 1U, NULL,
                                                           0U, wire, sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(wire, wire_len, sym_key, NULL, 0U, pt_out,
                                                    sizeof(pt_out), &pt_len, &e0));

    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_256, sym_key, 32U, big_kid,
                                               sizeof(big_kid), msg_body, sizeof(msg_body) - 1U,
                                               NULL, 0U, wire, sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(wire, wire_len, sym_key, 32U, NULL, 0U, &m0));
}

void test_r5_n01_cose_truncated_maps_and_sign_guards(void)
{
    uint8_t dummy_key[32] = {0x42U};
    uint8_t pt_out[32];
    size_t pt_len = 0U;
    SYN_COSE_Key key_out;

    /* 1. R5-N01 regression vectors (<1ms instant rejection of 4.29B map count) */
    static const uint8_t trunc_map_5b[] = {0xBAU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    static const uint8_t trunc_sign1_10b[] = {0x84U, 0x43U, 0xA1U, 0x01U, 0x27U,
                                              0xBAU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    static const uint8_t trunc_enc0_11b[] = {0x83U, 0x44U, 0xA1U, 0x01U, 0x18U, 0x18U,
                                             0xBAU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    static const uint8_t trunc_mac0_10b[] = {0x84U, 0x43U, 0xA1U, 0x01U, 0x05U,
                                             0xBAU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_decode(trunc_map_5b, sizeof(trunc_map_5b), &key_out));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(trunc_map_5b, sizeof(trunc_map_5b),
                                                           dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(trunc_sign1_10b, sizeof(trunc_sign1_10b),
                                                           dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_encrypt0_decrypt(trunc_enc0_11b, sizeof(trunc_enc0_11b), dummy_key,
                                             NULL, 0U, pt_out, sizeof(pt_out), &pt_len, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(trunc_mac0_10b, sizeof(trunc_mac0_10b),
                                                          dummy_key, 32U, NULL, 0U, NULL));

    /* 2. Protected header malformed maps (empty bstr, huge map, non-uint key, non-int alg) */
    static const uint8_t empty_prot_sign1[] = {0x84U, 0x40U, 0xA0U, 0x40U, 0x40U};
    static const uint8_t bad_prot_map_sign1[] = {0x84U, 0x45U, 0xBAU, 0xFFU, 0xFFU,
                                                 0xFFU, 0xFFU, 0xA0U, 0x40U, 0x40U};
    static const uint8_t text_key_prot_sign1[] = {0x84U, 0x44U, 0xA1U, 0x61U, 0x61U,
                                                  0x01U, 0xA0U, 0x40U, 0x40U};
    static const uint8_t text_alg_prot_sign1[] = {0x84U, 0x44U, 0xA1U, 0x01U, 0x61U,
                                                  0x61U, 0xA0U, 0x40U, 0x40U};
    static const uint8_t bad_prot_enc0[] = {0x83U, 0x44U, 0xA1U, 0x61U, 0x61U, 0x01U, 0xA0U, 0x40U};
    static const uint8_t bad_prot_mac0[] = {0x84U, 0x44U, 0xA1U, 0x61U, 0x61U,
                                            0x01U, 0xA0U, 0x40U, 0x40U};

    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(empty_prot_sign1, sizeof(empty_prot_sign1),
                                                dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(bad_prot_map_sign1, sizeof(bad_prot_map_sign1),
                                                dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(text_key_prot_sign1, sizeof(text_key_prot_sign1),
                                                dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(text_alg_prot_sign1, sizeof(text_alg_prot_sign1),
                                                dummy_key, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(bad_prot_enc0, sizeof(bad_prot_enc0),
                                                               dummy_key, NULL, 0U, pt_out,
                                                               sizeof(pt_out), &pt_len, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_mac0_verify(bad_prot_mac0, sizeof(bad_prot_mac0),
                                                          dummy_key, 32U, NULL, 0U, NULL));

    /* 3. Unprotected header non-uint key & oversized IV (>16B) */
    static const uint8_t text_key_unprot_sign1[] = {0x84U, 0x43U, 0xA1U, 0x01U, 0x27U, 0xA1U,
                                                    0x61U, 0x61U, 0x01U, 0x40U, 0x40U};
    static const uint8_t text_key_unprot_enc0[] = {0x83U, 0x44U, 0xA1U, 0x01U, 0x18U, 0x18U,
                                                   0xA1U, 0x61U, 0x61U, 0x01U, 0x40U};
    static const uint8_t big_iv_enc0[] = {
        0x83U, 0x44U, 0xA1U, 0x01U, 0x18U, 0x18U, 0xA1U, 0x05U, 0x54U, 0U, 0U, 0U, 0U, 0U, 0U,
        0U,    0U,    0U,    0U,    0U,    0U,    0U,    0U,    0U,    0U, 0U, 0U, 0U, 0U, 0x40U};

    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_verify(text_key_unprot_sign1,
                                                           sizeof(text_key_unprot_sign1), dummy_key,
                                                           32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(text_key_unprot_enc0,
                                                    sizeof(text_key_unprot_enc0), dummy_key, NULL,
                                                    0U, pt_out, sizeof(pt_out), &pt_len, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(big_iv_enc0, sizeof(big_iv_enc0),
                                                               dummy_key, NULL, 0U, pt_out,
                                                               sizeof(pt_out), &pt_len, NULL));

    /* 4. RNG failure and invalid ES256 private key in syn_cose_sign1_create */
    uint8_t out_buf[256];
    size_t out_len = 0U;
    uint8_t valid_priv[32] = {0};
    valid_priv[31] = 1U;
    uint8_t zero_priv[32] = {0};
    static const uint8_t msg[] = "rng-check";

    mock_random_skip = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_create(SYN_COSE_ALGO_ES256, valid_priv, NULL,
                                                           NULL, 0U, msg, sizeof(msg) - 1U, NULL,
                                                           0U, out_buf, sizeof(out_buf), &out_len));
    mock_random_skip = false;

    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_create(SYN_COSE_ALGO_ES256, zero_priv, NULL,
                                                           NULL, 0U, msg, sizeof(msg) - 1U, NULL,
                                                           0U, out_buf, sizeof(out_buf), &out_len));

    /* 5. COSE_Key decode error branches (non-int label, invalid bstr field, missing kty) */
    static const uint8_t key_non_int_lbl[] = {0xA1U, 0x61U, 0x61U, 0x01U};
    static const uint8_t key_bad_bstr[] = {0xA2U, 0x01U, 0x04U, 0x02U, 0x05U};
    static const uint8_t key_missing_kty[] = {0xA1U, 0x03U, 0x27U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_decode(key_non_int_lbl, sizeof(key_non_int_lbl), &key_out));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_decode(key_bad_bstr, sizeof(key_bad_bstr), &key_out));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_decode(key_missing_kty, sizeof(key_missing_kty), &key_out));

    /* 6. COSE_Mac0 with valid non-HMAC algorithm (alg=1 AES-GCM-128 -> tag_len 0 rejected) */
    static const uint8_t mac0_non_hmac_alg[] = {0x84U, 0x43U, 0xA1U, 0x01U,
                                                0x01U, 0xA0U, 0x40U, 0x40U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_mac0_verify(mac0_non_hmac_alg, sizeof(mac0_non_hmac_alg),
                                               dummy_key, 32U, NULL, 0U, NULL));
}

void test_pro04_cose_hardening_and_tags(void)
{
    uint8_t seed[32] = {0x21U};
    uint8_t ed_pub[32], ed_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(ed_pub, ed_sec, seed));

    static const uint8_t sym_key[32] = {0x31U, 0x32U, 0x33U, 0x34U};
    static const uint8_t iv12[12] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U};
    static const uint8_t iv13[13] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U};
    static const uint8_t msg_pt[] = "pro04-payload";

    uint8_t wire[256];
    uint8_t tagged[260];
    size_t wire_len = 0U;
    uint8_t pt_out[64];
    size_t pt_len = 0U;

    /* 1. EdDSA sign1_create rejects NULL public_key */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, ed_sec, NULL, NULL,
                                                           0U, msg_pt, sizeof(msg_pt) - 1U, NULL,
                                                           0U, wire, sizeof(wire), &wire_len));

    /* 2. encrypt0_decrypt guards NULL out_plaintext when pt_len > 0, allows NULL when pt_len == 0
     */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, sym_key,
                                                           iv12, sizeof(iv12), NULL, 0U, msg_pt,
                                                           sizeof(msg_pt) - 1U, NULL, 0U, wire,
                                                           sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(wire, wire_len, sym_key, NULL, 0U,
                                                               NULL, 64U, &pt_len, NULL));

    uint8_t empty_wire[128];
    size_t empty_wire_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(SYN_COSE_ALGO_CHACHA20_POLY1305, sym_key,
                                                           iv12, sizeof(iv12), NULL, 0U, NULL, 0U,
                                                           NULL, 0U, empty_wire, sizeof(empty_wire),
                                                           &empty_wire_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_decrypt(empty_wire, empty_wire_len, sym_key,
                                                            NULL, 0U, NULL, 0U, &pt_len, NULL));
    TEST_ASSERT_EQUAL_UINT(0U, pt_len);

    /* 3. CCM strict 13-byte IV check in create and decrypt (reject 12-byte IV on AES-CCM-16-*) */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_create(SYN_COSE_ALGO_AES_CCM_16_64_128,
                                                              sym_key, iv12, sizeof(iv12), NULL, 0U,
                                                              msg_pt, sizeof(msg_pt) - 1U, NULL, 0U,
                                                              wire, sizeof(wire), &wire_len));
    static const uint8_t ccm_iv12_cbor[] = {
        0x83U, 0x43U, 0xA1U, 0x01U, 0x0AU, 0xA1U, 0x05U, 0x4CU, 1U, 2U, 3U, 4U, 5U, 6U, 7U,
        8U,    9U,    10U,   11U,   12U,   0x48U, 0U,    0U,    0U, 0U, 0U, 0U, 0U, 0U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_encrypt0_decrypt(ccm_iv12_cbor, sizeof(ccm_iv12_cbor),
                                                               sym_key, NULL, 0U, pt_out,
                                                               sizeof(pt_out), &pt_len, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_encrypt0_create(SYN_COSE_ALGO_AES_CCM_16_64_128, sym_key,
                                                           iv13, sizeof(iv13), NULL, 0U, msg_pt,
                                                           sizeof(msg_pt) - 1U, NULL, 0U, wire,
                                                           sizeof(wire), &wire_len));

    /* 4. Optional CBOR Tags 16/17/18 + fail-closed parsed_out on tampered auth */
    /* Encrypt0: Tag 16 (0xD0) accepted, wrong Tag 18 (0xD2) rejected, tampered zeroes parsed_out */
    tagged[0] = 0xD0U;
    (void)memcpy(&tagged[1], wire, wire_len);
    SYN_COSE_Encrypt0Message parsed_e0;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_cose_encrypt0_decrypt(tagged, wire_len + 1U, sym_key, NULL, 0U,
                                                    pt_out, sizeof(pt_out), &pt_len, &parsed_e0));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_AES_CCM_16_64_128, parsed_e0.alg);
    tagged[0] = 0xD2U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(tagged, wire_len + 1U, sym_key, NULL, 0U,
                                                    pt_out, sizeof(pt_out), &pt_len, &parsed_e0));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_UNKNOWN, parsed_e0.alg);
    tagged[0] = 0xD0U;
    tagged[wire_len] ^= 0xFFU;
    parsed_e0.alg = SYN_COSE_ALGO_A128GCM;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_encrypt0_decrypt(tagged, wire_len + 1U, sym_key, NULL, 0U,
                                                    pt_out, sizeof(pt_out), &pt_len, &parsed_e0));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_UNKNOWN, parsed_e0.alg);
    TEST_ASSERT_NULL(parsed_e0.ciphertext);

    /* Sign1: Tag 18 (0xD2 and 0xD8,0x12) accepted, malformed/wrong tags rejected, fail-closed */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_sign1_create(SYN_COSE_ALGO_EDDSA, ed_sec, ed_pub, NULL,
                                                        0U, msg_pt, sizeof(msg_pt) - 1U, NULL, 0U,
                                                        wire, sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_HEX8(0x84U, wire[0]); /* create remains untagged */
    tagged[0] = 0xD2U;
    (void)memcpy(&tagged[1], wire, wire_len);
    SYN_COSE_Sign1Message parsed_s1;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_sign1_verify(tagged, wire_len + 1U, ed_pub, 32U, NULL, 0U, &parsed_s1));
    tagged[0] = 0xD8U;
    tagged[1] = 0x12U;
    (void)memcpy(&tagged[2], wire, wire_len);
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_sign1_verify(tagged, wire_len + 2U, ed_pub, 32U, NULL, 0U, &parsed_s1));
    static const uint8_t trunc_tag24[] = {0xD8U};
    static const uint8_t wide_tag[] = {0xD9U, 0x00U, 0x12U, 0x84U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_sign1_verify(trunc_tag24, 1U, ed_pub, 32U, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_sign1_verify(wide_tag, sizeof(wide_tag), ed_pub, 32U, NULL, 0U, NULL));
    tagged[2 + wire_len - 1U] ^= 0x01U;
    parsed_s1.alg = SYN_COSE_ALGO_EDDSA;
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_sign1_verify(tagged, wire_len + 2U, ed_pub, 32U, NULL, 0U, &parsed_s1));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_UNKNOWN, parsed_s1.alg);
    TEST_ASSERT_NULL(parsed_s1.payload);

    /* Mac0: Tag 17 (0xD1) accepted, tampered tag zeroes parsed_out */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_cose_mac0_create(SYN_COSE_ALGO_HMAC_256_64, sym_key, 32U,
                                                       NULL, 0U, msg_pt, sizeof(msg_pt) - 1U, NULL,
                                                       0U, wire, sizeof(wire), &wire_len));
    TEST_ASSERT_EQUAL_HEX8(0x84U, wire[0]);
    tagged[0] = 0xD1U;
    (void)memcpy(&tagged[1], wire, wire_len);
    SYN_COSE_Mac0Message parsed_m0;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_cose_mac0_verify(tagged, wire_len + 1U, sym_key, 32U, NULL, 0U, &parsed_m0));
    tagged[wire_len] ^= 0xFFU;
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_cose_mac0_verify(tagged, wire_len + 1U, sym_key, 32U, NULL, 0U, &parsed_m0));
    TEST_ASSERT_EQUAL_INT(SYN_COSE_ALGO_UNKNOWN, parsed_m0.alg);
    TEST_ASSERT_NULL(parsed_m0.payload);

    /* 5. syn_cose_key_encode length bounds checks & syn_cose_key_decode -1 type confusion */
    SYN_COSE_Key k;
    (void)memset(&k, 0, sizeof(k));
    k.kty = SYN_COSE_KTY_EC2;
    k.kid_len = 33U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_encode(&k, wire, sizeof(wire), &wire_len));
    k.kid_len = 0U;
    k.pub_x_len = 33U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_encode(&k, wire, sizeof(wire), &wire_len));
    k.pub_x_len = 0U;
    k.pub_y_len = 33U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_encode(&k, wire, sizeof(wire), &wire_len));
    k.pub_y_len = 0U;
    k.priv_d_len = 33U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_encode(&k, wire, sizeof(wire), &wire_len));

    /* Symmetric with int -1 (both {1:4,-1:6} and {-1:6,1:4}) & OKP/EC2 with bstr -1 + dup -1 */
    static const uint8_t sym_int_neg1[] = {0xA2U, 0x01U, 0x04U, 0x20U, 0x06U};
    static const uint8_t rev_sym_int_neg1[] = {0xA2U, 0x20U, 0x06U, 0x01U, 0x04U};
    static const uint8_t okp_bstr_neg1[] = {0xA2U, 0x01U, 0x01U, 0x20U, 0x42U, 0xAAU, 0xBBU};
    static const uint8_t ec2_bstr_neg1[] = {0xA2U, 0x20U, 0x42U, 0xAAU, 0xBBU, 0x01U, 0x02U};
    static const uint8_t dup_neg1[] = {0xA3U, 0x01U, 0x01U, 0x20U, 0x06U, 0x20U, 0x06U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(sym_int_neg1, sizeof(sym_int_neg1), &k));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_cose_key_decode(rev_sym_int_neg1, sizeof(rev_sym_int_neg1), &k));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(okp_bstr_neg1, sizeof(okp_bstr_neg1), &k));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(ec2_bstr_neg1, sizeof(ec2_bstr_neg1), &k));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_cose_key_decode(dup_neg1, sizeof(dup_neg1), &k));
}

void run_cose_tests(void)
{
    RUN_TEST(test_cose_sign1_eddsa_roundtrip_and_tamper);
    RUN_TEST(test_cose_sign1_es256_roundtrip_and_tamper);
    RUN_TEST(test_cose_encrypt0_chacha20poly1305_roundtrip_and_tamper);
    RUN_TEST(test_cose_encrypt0_aes_gcm_roundtrip_and_tamper);
    RUN_TEST(test_cose_encrypt0_aes_ccm_roundtrip_and_tamper);
    RUN_TEST(test_cose_mac0_hmac_roundtrip_and_tamper);
    RUN_TEST(test_cose_key_encode_decode);
    RUN_TEST(test_cose_null_and_boundary_checks);
    RUN_TEST(test_cose_zero_copy_views_and_oversized_kid);
    RUN_TEST(test_r5_n01_cose_truncated_maps_and_sign_guards);
    RUN_TEST(test_pro04_cose_hardening_and_tags);
}
