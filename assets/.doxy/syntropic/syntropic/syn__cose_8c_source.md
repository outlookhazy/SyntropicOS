

# File syn\_cose.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cose.c**](syn__cose_8c.md)

[Go to the documentation of this file](syn__cose_8c.md)


```C++

#include "syntropic/proto/syn_cose.h"

#if !defined(SYN_USE_COSE) || SYN_USE_COSE

#include "syntropic/util/syn_random.h"

#include <string.h>

/* Build serialized Sig_structure per RFC 9052 Section 4.4 */
static bool build_sig_structure(const uint8_t *body_protected, size_t body_protected_len,
                                const uint8_t *external_aad, size_t external_aad_len,
                                const uint8_t *payload, size_t payload_len, uint8_t *out_buf,
                                size_t out_buf_size, size_t *out_len)
{
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    /* Sig_structure is an array of 4 elements */
    syn_cbor_write_array_begin(&w, 4);
    syn_cbor_write_text_cstr(&w, "Signature1");
    syn_cbor_write_bytes(&w, body_protected, body_protected_len);
    syn_cbor_write_bytes(&w, (external_aad != NULL) ? external_aad : (const uint8_t *)"",
                         external_aad_len);
    syn_cbor_write_bytes(&w, (payload != NULL) ? payload : (const uint8_t *)"", payload_len);

    if (!syn_cbor_writer_ok(&w)) {
        return false;
    }

    *out_len = syn_cbor_writer_len(&w);
    return true;
}

/* Build serialized Enc_structure per RFC 9052 Section 5.3 */
static bool build_enc_structure(const uint8_t *body_protected, size_t body_protected_len,
                                const uint8_t *external_aad, size_t external_aad_len,
                                uint8_t *out_buf, size_t out_buf_size, size_t *out_len)
{
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    /* Enc_structure is an array of 3 elements */
    syn_cbor_write_array_begin(&w, 3);
    syn_cbor_write_text_cstr(&w, "Encrypt0");
    syn_cbor_write_bytes(&w, body_protected, body_protected_len);
    syn_cbor_write_bytes(&w, (external_aad != NULL) ? external_aad : (const uint8_t *)"",
                         external_aad_len);

    if (!syn_cbor_writer_ok(&w)) {
        return false;
    }

    *out_len = syn_cbor_writer_len(&w);
    return true;
}

static bool build_mac_structure(const uint8_t *body_protected, size_t body_protected_len,
                                const uint8_t *external_aad, size_t external_aad_len,
                                const uint8_t *payload, size_t payload_len, uint8_t *out_buf,
                                size_t out_buf_size, size_t *out_len)
{
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    /* MAC_structure is an array of 4 elements */
    syn_cbor_write_array_begin(&w, 4);
    syn_cbor_write_text_cstr(&w, "MAC0");
    syn_cbor_write_bytes(&w, body_protected, body_protected_len);
    syn_cbor_write_bytes(&w, (external_aad != NULL) ? external_aad : (const uint8_t *)"",
                         external_aad_len);
    syn_cbor_write_bytes(&w, (payload != NULL) ? payload : (const uint8_t *)"", payload_len);

    if (!syn_cbor_writer_ok(&w)) {
        return false;
    }

    *out_len = syn_cbor_writer_len(&w);
    return true;
}

SYN_Status syn_cose_sign1_create(SYN_COSE_Algorithm alg, const uint8_t *secret_key,
                                 const uint8_t *public_key, const uint8_t *kid, size_t kid_len,
                                 const uint8_t *payload, size_t payload_len,
                                 const uint8_t *external_aad, size_t external_aad_len,
                                 uint8_t *out_buf, size_t out_buf_size, size_t *out_len)
{
    if (secret_key == NULL || out_buf == NULL || out_len == NULL ||
        (payload == NULL && payload_len > 0U)) {
        return SYN_ERROR;
    }

    if (alg != SYN_COSE_ALGO_EDDSA && alg != SYN_COSE_ALGO_ES256) {
        return SYN_ERROR;
    }

    /* 1. Build protected header: { 1: alg } */
    uint8_t protected_raw[16];
    SYN_CborWriter pw;
    syn_cbor_writer_init(&pw, protected_raw, sizeof(protected_raw));
    syn_cbor_write_map_begin(&pw, 1);
    syn_cbor_write_uint(&pw, SYN_COSE_HEADER_ALG);
    syn_cbor_write_int(&pw, (int64_t)alg);
    size_t protected_raw_len = syn_cbor_writer_len(&pw);

    /* 2. Build Sig_structure for signature generation */
    uint8_t sig_structure[1024];
    size_t sig_struct_len = 0;
    if (!build_sig_structure(protected_raw, protected_raw_len, external_aad, external_aad_len,
                             payload, payload_len, sig_structure, sizeof(sig_structure),
                             &sig_struct_len)) {
        return SYN_ERROR;
    }

    /* 3. Compute signature */
    uint8_t signature[SYN_COSE_MAX_SIG_LEN];
    size_t sig_len = 64U;

    if (alg == SYN_COSE_ALGO_EDDSA) {
        (void)syn_ed25519_sign(sig_structure, sig_struct_len, secret_key, public_key, signature);
    } else {
        /* ES256: Hash Sig_structure with SHA-256 then sign with P-256 ECDSA */
        uint8_t hash[SYN_SHA256_DIGEST_SIZE];
        syn_sha256(sig_structure, sig_struct_len, hash);

        uint8_t nonce_k[32];
        (void)syn_random_fill(nonce_k, sizeof(nonce_k));
        (void)syn_p256_sign_ecdsa(secret_key, nonce_k, hash, signature, signature + 32);
    }

    /* 4. Encode final COSE_Sign1 message */
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    syn_cbor_write_array_begin(&w, 4);
    syn_cbor_write_bytes(&w, protected_raw, protected_raw_len);

    /* Unprotected header map */
    if (kid != NULL && kid_len > 0U) {
        syn_cbor_write_map_begin(&w, 1);
        syn_cbor_write_uint(&w, SYN_COSE_HEADER_KID);
        syn_cbor_write_bytes(&w, kid, kid_len);
    } else {
        syn_cbor_write_map_begin(&w, 0);
    }

    /* Payload */
    syn_cbor_write_bytes(&w, (payload != NULL) ? payload : (const uint8_t *)"", payload_len);

    /* Signature */
    syn_cbor_write_bytes(&w, signature, sig_len);

    if (!syn_cbor_writer_ok(&w)) {
        return SYN_ERROR;
    }

    *out_len = syn_cbor_writer_len(&w);
    return SYN_OK;
}

SYN_Status syn_cose_sign1_verify(const uint8_t *msg, size_t msg_len, const uint8_t *public_key,
                                 size_t public_key_len, const uint8_t *external_aad,
                                 size_t external_aad_len, SYN_COSE_Sign1Message *parsed_out)
{
    if (msg == NULL || msg_len == 0U || public_key == NULL) {
        return SYN_ERROR;
    }

    SYN_CborReader r;
    syn_cbor_reader_init(&r, msg, msg_len);

    size_t array_items = syn_cbor_read_array_begin(&r);
    if (!syn_cbor_reader_ok(&r) || array_items != 4U) {
        return SYN_ERROR;
    }

    /* 1. Read protected header bstr */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = syn_cbor_read_bytes(&r, protected_buf, sizeof(protected_buf));
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    /* Parse protected header map */
    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    if (protected_len > 0U) {
        SYN_CborReader pr;
        syn_cbor_reader_init(&pr, protected_buf, protected_len);
        size_t pairs = syn_cbor_read_map_begin(&pr);
        for (size_t i = 0; i < pairs; i++) {
            uint64_t label = syn_cbor_read_uint(&pr);
            if (label == SYN_COSE_HEADER_ALG) {
                parsed_alg = (SYN_COSE_Algorithm)syn_cbor_read_int(&pr);
            } else {
                syn_cbor_skip(&pr);
            }
        }
    }

    if (parsed_alg != SYN_COSE_ALGO_EDDSA && parsed_alg != SYN_COSE_ALGO_ES256) {
        return SYN_ERROR;
    }

    /* 2. Read unprotected header map */
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;

    size_t unprot_pairs = syn_cbor_read_map_begin(&r);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }
    for (size_t i = 0; i < unprot_pairs; i++) {
        uint64_t label = syn_cbor_read_uint(&r);
        if (label == SYN_COSE_HEADER_KID) {
            parsed_kid_len = syn_cbor_read_bytes(&r, parsed_kid, sizeof(parsed_kid));
        } else {
            syn_cbor_skip(&r);
        }
    }

    /* 3. Read payload */
    uint8_t payload_buf[1024];
    size_t payload_len = syn_cbor_read_bytes(&r, payload_buf, sizeof(payload_buf));
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    /* 4. Read signature */
    uint8_t sig_buf[SYN_COSE_MAX_SIG_LEN];
    size_t sig_len = syn_cbor_read_bytes(&r, sig_buf, sizeof(sig_buf));
    if (!syn_cbor_reader_ok(&r) || sig_len != 64U) {
        return SYN_ERROR;
    }

    /* Populate optional output struct */
    if (parsed_out != NULL) {
        parsed_out->alg = parsed_alg;
        (void)memcpy(parsed_out->kid, parsed_kid, parsed_kid_len);
        parsed_out->kid_len = parsed_kid_len;
        parsed_out->payload = payload_buf;
        parsed_out->payload_len = payload_len;
        parsed_out->signature = sig_buf;
        parsed_out->signature_len = sig_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 5. Reconstruct Sig_structure */
    uint8_t sig_structure[1024];
    size_t sig_struct_len = 0;
    if (!build_sig_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             payload_buf, payload_len, sig_structure, sizeof(sig_structure),
                             &sig_struct_len)) {
        return SYN_ERROR;
    }

    /* 6. Verify signature */
    if (parsed_alg == SYN_COSE_ALGO_EDDSA) {
        if (public_key_len != SYN_ED25519_PUBLIC_KEY_SIZE) {
            return SYN_ERROR;
        }
        if (!syn_ed25519_verify(sig_buf, sig_structure, sig_struct_len, public_key)) {
            return SYN_ERROR;
        }
    } else {
        /* ES256 */
        const uint8_t *px = public_key;
        const uint8_t *py = public_key + 32;
        if (public_key_len == 65U && public_key[0] == 0x04U) {
            px = public_key + 1;
            py = public_key + 33;
        } else if (public_key_len != 64U) {
            return SYN_ERROR;
        }

        uint8_t hash[SYN_SHA256_DIGEST_SIZE];
        syn_sha256(sig_structure, sig_struct_len, hash);

        if (!syn_p256_verify_ecdsa(hash, sig_buf, sig_buf + 32, px, py)) {
            return SYN_ERROR;
        }
    }

    return SYN_OK;
}

SYN_Status syn_cose_encrypt0_create(SYN_COSE_Algorithm alg, const uint8_t *key, const uint8_t *iv,
                                    size_t iv_len, const uint8_t *kid, size_t kid_len,
                                    const uint8_t *plaintext, size_t plaintext_len,
                                    const uint8_t *external_aad, size_t external_aad_len,
                                    uint8_t *out_buf, size_t out_buf_size, size_t *out_len)
{
    if (key == NULL || iv == NULL || out_buf == NULL || out_len == NULL ||
        (plaintext == NULL && plaintext_len > 0U)) {
        return SYN_ERROR;
    }

    size_t key_len = 0;
    size_t tag_len = 16U;
    if (alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        key_len = 32U;
    } else if (alg == SYN_COSE_ALGO_A128GCM || alg == SYN_COSE_ALGO_AES_CCM_16_64_128 ||
               alg == SYN_COSE_ALGO_AES_CCM_16_128_128) {
        key_len = 16U;
        if (alg == SYN_COSE_ALGO_AES_CCM_16_64_128) {
            tag_len = 8U;
        }
    } else if (alg == SYN_COSE_ALGO_A192GCM) {
        key_len = 24U;
    } else if (alg == SYN_COSE_ALGO_A256GCM || alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
               alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        key_len = 32U;
        if (alg == SYN_COSE_ALGO_AES_CCM_16_64_256) {
            tag_len = 8U;
        }
    } else {
        return SYN_ERROR;
    }

    if (alg == SYN_COSE_ALGO_AES_CCM_16_64_128 || alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
        alg == SYN_COSE_ALGO_AES_CCM_16_128_128 || alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        if (iv_len < 7U || iv_len > 13U) {
            return SYN_ERROR;
        }
    } else {
        if (iv_len != 12U) {
            return SYN_ERROR;
        }
    }

    /* 1. Build protected header: { 1: alg } */
    uint8_t protected_raw[16];
    SYN_CborWriter pw;
    syn_cbor_writer_init(&pw, protected_raw, sizeof(protected_raw));
    syn_cbor_write_map_begin(&pw, 1);
    syn_cbor_write_uint(&pw, SYN_COSE_HEADER_ALG);
    syn_cbor_write_uint(&pw, (uint64_t)alg);
    size_t protected_raw_len = syn_cbor_writer_len(&pw);

    /* 2. Build Enc_structure for AAD */
    uint8_t enc_structure[512];
    size_t enc_struct_len = 0;
    if (!build_enc_structure(protected_raw, protected_raw_len, external_aad, external_aad_len,
                             enc_structure, sizeof(enc_structure), &enc_struct_len)) {
        return SYN_ERROR;
    }

    /* 3. Encrypt plaintext (Payload || Tag) */
    uint8_t ct_tag_buf[1024];
    if (plaintext_len + tag_len > sizeof(ct_tag_buf)) {
        return SYN_ERROR;
    }

    uint8_t tag[16];
    if (alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        if (plaintext_len > 0U && plaintext != NULL) {
            syn_aead_encrypt(key, iv, enc_structure, enc_struct_len, plaintext, plaintext_len,
                             ct_tag_buf, tag);
        } else {
            syn_aead_encrypt(key, iv, enc_structure, enc_struct_len, NULL, 0U, NULL, tag);
        }
    } else if (alg == SYN_COSE_ALGO_AES_CCM_16_64_128 || alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
               alg == SYN_COSE_ALGO_AES_CCM_16_128_128 || alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        SYN_AES_Context aes_ctx;
        (void)syn_aes_init(&aes_ctx, key, key_len);
        (void)syn_aes_ccm_encrypt(&aes_ctx, iv, iv_len, enc_structure, enc_struct_len, plaintext,
                                  plaintext_len, ct_tag_buf, tag, tag_len);
    } else {
        SYN_AES_GCM_Context gcm_ctx;
        (void)syn_aes_gcm_init(&gcm_ctx, key, key_len);
        (void)syn_aes_gcm_encrypt(&gcm_ctx, iv, iv_len, enc_structure, enc_struct_len, plaintext,
                                  plaintext_len, ct_tag_buf, tag);
    }
    (void)memcpy(ct_tag_buf + plaintext_len, tag, tag_len);
    size_t total_ct_len = plaintext_len + tag_len;

    /* 4. Encode final COSE_Encrypt0 message */
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    syn_cbor_write_array_begin(&w, 3);
    syn_cbor_write_bytes(&w, protected_raw, protected_raw_len);

    /* Unprotected header map: { 5: IV, [4: kid] } */
    size_t unprot_count = 1U + (size_t)(kid != NULL && kid_len > 0U);
    syn_cbor_write_map_begin(&w, unprot_count);
    syn_cbor_write_uint(&w, SYN_COSE_HEADER_IV);
    syn_cbor_write_bytes(&w, iv, iv_len);
    if (kid != NULL && kid_len > 0U) {
        syn_cbor_write_uint(&w, SYN_COSE_HEADER_KID);
        syn_cbor_write_bytes(&w, kid, kid_len);
    }

    /* Ciphertext + Tag */
    syn_cbor_write_bytes(&w, ct_tag_buf, total_ct_len);

    if (!syn_cbor_writer_ok(&w)) {
        return SYN_ERROR;
    }

    *out_len = syn_cbor_writer_len(&w);
    return SYN_OK;
}

SYN_Status syn_cose_encrypt0_decrypt(const uint8_t *msg, size_t msg_len, const uint8_t *key,
                                     const uint8_t *external_aad, size_t external_aad_len,
                                     uint8_t *out_plaintext, size_t out_plaintext_size,
                                     size_t *out_plaintext_len,
                                     SYN_COSE_Encrypt0Message *parsed_out)
{
    if (msg == NULL || msg_len == 0U || key == NULL || out_plaintext_len == NULL) {
        return SYN_ERROR;
    }

    SYN_CborReader r;
    syn_cbor_reader_init(&r, msg, msg_len);

    size_t array_items = syn_cbor_read_array_begin(&r);
    if (!syn_cbor_reader_ok(&r) || array_items != 3U) {
        return SYN_ERROR;
    }

    /* 1. Read protected header */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = syn_cbor_read_bytes(&r, protected_buf, sizeof(protected_buf));
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    if (protected_len > 0U) {
        SYN_CborReader pr;
        syn_cbor_reader_init(&pr, protected_buf, protected_len);
        size_t pairs = syn_cbor_read_map_begin(&pr);
        for (size_t i = 0; i < pairs; i++) {
            uint64_t label = syn_cbor_read_uint(&pr);
            if (label == SYN_COSE_HEADER_ALG) {
                parsed_alg = (SYN_COSE_Algorithm)syn_cbor_read_int(&pr);
            } else {
                syn_cbor_skip(&pr);
            }
        }
    }

    size_t key_len = 0;
    size_t tag_len = 16U;
    if (parsed_alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        key_len = 32U;
    } else if (parsed_alg == SYN_COSE_ALGO_A128GCM ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_128 ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_128) {
        key_len = 16U;
        if (parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_128) {
            tag_len = 8U;
        }
    } else if (parsed_alg == SYN_COSE_ALGO_A192GCM) {
        key_len = 24U;
    } else if (parsed_alg == SYN_COSE_ALGO_A256GCM ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        key_len = 32U;
        if (parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_256) {
            tag_len = 8U;
        }
    } else {
        return SYN_ERROR;
    }

    /* 2. Read unprotected header map */
    uint8_t parsed_iv[SYN_COSE_MAX_IV_LEN] = {0};
    size_t parsed_iv_len = 0;
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;

    size_t unprot_pairs = syn_cbor_read_map_begin(&r);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }
    for (size_t i = 0; i < unprot_pairs; i++) {
        uint64_t label = syn_cbor_read_uint(&r);
        if (label == SYN_COSE_HEADER_IV) {
            parsed_iv_len = syn_cbor_read_bytes(&r, parsed_iv, sizeof(parsed_iv));
        } else if (label == SYN_COSE_HEADER_KID) {
            parsed_kid_len = syn_cbor_read_bytes(&r, parsed_kid, sizeof(parsed_kid));
        } else {
            syn_cbor_skip(&r);
        }
    }

    if (parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_128 ||
        parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
        parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_128 ||
        parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        if (parsed_iv_len < 7U || parsed_iv_len > 13U) {
            return SYN_ERROR;
        }
    } else {
        if (parsed_iv_len != 12U) {
            return SYN_ERROR;
        }
    }

    /* 3. Read ciphertext + tag */
    uint8_t ct_tag_buf[1024];
    size_t ct_tag_len = syn_cbor_read_bytes(&r, ct_tag_buf, sizeof(ct_tag_buf));
    if (!syn_cbor_reader_ok(&r) || ct_tag_len < tag_len) {
        return SYN_ERROR;
    }

    size_t pt_len = ct_tag_len - tag_len;
    if (out_plaintext != NULL && pt_len > out_plaintext_size) {
        return SYN_ERROR;
    }

    /* Populate optional output descriptor */
    if (parsed_out != NULL) {
        parsed_out->alg = parsed_alg;
        (void)memcpy(parsed_out->iv, parsed_iv, parsed_iv_len);
        parsed_out->iv_len = parsed_iv_len;
        (void)memcpy(parsed_out->kid, parsed_kid, parsed_kid_len);
        parsed_out->kid_len = parsed_kid_len;
        parsed_out->ciphertext = ct_tag_buf;
        parsed_out->ciphertext_len = ct_tag_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 4. Reconstruct Enc_structure */
    uint8_t enc_structure[512];
    size_t enc_struct_len = 0;
    if (!build_enc_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             enc_structure, sizeof(enc_structure), &enc_struct_len)) {
        return SYN_ERROR;
    }

    /* 5. Decrypt and verify tag */
    const uint8_t *tag = ct_tag_buf + pt_len;
    if (parsed_alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        if (!syn_aead_decrypt(key, parsed_iv, enc_structure, enc_struct_len, ct_tag_buf, pt_len,
                              tag, out_plaintext)) {
            return SYN_ERROR;
        }
    } else if (parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_128 ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_128 ||
               parsed_alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        SYN_AES_Context aes_ctx;
        (void)syn_aes_init(&aes_ctx, key, key_len);
        if (syn_aes_ccm_decrypt(&aes_ctx, parsed_iv, parsed_iv_len, enc_structure, enc_struct_len,
                                ct_tag_buf, pt_len, tag, tag_len, out_plaintext) != SYN_OK) {
            return SYN_ERROR;
        }
    } else {
        SYN_AES_GCM_Context gcm_ctx;
        (void)syn_aes_gcm_init(&gcm_ctx, key, key_len);
        if (syn_aes_gcm_decrypt(&gcm_ctx, parsed_iv, parsed_iv_len, enc_structure, enc_struct_len,
                                ct_tag_buf, pt_len, out_plaintext, tag) != SYN_OK) {
            return SYN_ERROR;
        }
    }

    *out_plaintext_len = pt_len;
    return SYN_OK;
}

/* ── COSE_Mac0 API ───────────────────────────────────────────────────────── */

SYN_Status syn_cose_mac0_create(SYN_COSE_Algorithm alg, const uint8_t *key, size_t key_len,
                                const uint8_t *kid, size_t kid_len, const uint8_t *payload,
                                size_t payload_len, const uint8_t *external_aad,
                                size_t external_aad_len, uint8_t *out_buf, size_t out_buf_size,
                                size_t *out_len)
{
    if (key == NULL || key_len == 0U || out_buf == NULL || out_len == NULL ||
        (payload == NULL && payload_len > 0U)) {
        return SYN_ERROR;
    }

    size_t tag_len = 0U;
    if (alg == SYN_COSE_ALGO_HMAC_256_256) {
        tag_len = 32U;
    } else if (alg == SYN_COSE_ALGO_HMAC_256_64) {
        tag_len = 8U;
    } else {
        return SYN_ERROR;
    }

    /* 1. Build protected header: { 1: alg } */
    uint8_t protected_raw[16];
    SYN_CborWriter pw;
    syn_cbor_writer_init(&pw, protected_raw, sizeof(protected_raw));
    syn_cbor_write_map_begin(&pw, 1);
    syn_cbor_write_uint(&pw, SYN_COSE_HEADER_ALG);
    syn_cbor_write_int(&pw, (int64_t)alg);
    size_t protected_raw_len = syn_cbor_writer_len(&pw);

    /* 2. Build MAC_structure */
    uint8_t mac_structure[1024];
    size_t mac_struct_len = 0;
    if (!build_mac_structure(protected_raw, protected_raw_len, external_aad, external_aad_len,
                             payload, payload_len, mac_structure, sizeof(mac_structure),
                             &mac_struct_len)) {
        return SYN_ERROR;
    }

    /* 3. Compute HMAC */
    uint8_t full_tag[32];
    syn_hmac_sha256(key, key_len, mac_structure, mac_struct_len, full_tag);

    /* 4. Encode COSE_Mac0 message (Array of 4 elements) */
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);
    syn_cbor_write_array_begin(&w, 4);

    /* Element 0: protected header */
    syn_cbor_write_bytes(&w, protected_raw, protected_raw_len);

    /* Element 1: unprotected map { 4: kid } */
    if (kid != NULL && kid_len > 0U) {
        syn_cbor_write_map_begin(&w, 1);
        syn_cbor_write_uint(&w, SYN_COSE_HEADER_KID);
        syn_cbor_write_bytes(&w, kid, kid_len);
    } else {
        syn_cbor_write_map_begin(&w, 0);
    }

    /* Element 2: payload */
    syn_cbor_write_bytes(&w, (payload != NULL) ? payload : (const uint8_t *)"", payload_len);

    /* Element 3: tag */
    syn_cbor_write_bytes(&w, full_tag, tag_len);

    if (!syn_cbor_writer_ok(&w)) {
        return SYN_ERROR;
    }

    *out_len = syn_cbor_writer_len(&w);
    return SYN_OK;
}

SYN_Status syn_cose_mac0_verify(const uint8_t *msg, size_t msg_len, const uint8_t *key,
                                size_t key_len, const uint8_t *external_aad,
                                size_t external_aad_len, SYN_COSE_Mac0Message *parsed_out)
{
    if (msg == NULL || msg_len == 0U || key == NULL || key_len == 0U) {
        return SYN_ERROR;
    }

    SYN_CborReader r;
    syn_cbor_reader_init(&r, msg, msg_len);

    size_t array_items = syn_cbor_read_array_begin(&r);
    if (!syn_cbor_reader_ok(&r) || array_items != 4U) {
        return SYN_ERROR;
    }

    /* 0. Protected header */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = syn_cbor_read_bytes(&r, protected_buf, sizeof(protected_buf));
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    if (protected_len > 0U) {
        SYN_CborReader pr;
        syn_cbor_reader_init(&pr, protected_buf, protected_len);
        size_t pairs = syn_cbor_read_map_begin(&pr);
        for (size_t i = 0; i < pairs; i++) {
            uint64_t label = syn_cbor_read_uint(&pr);
            if (label == SYN_COSE_HEADER_ALG) {
                parsed_alg = (SYN_COSE_Algorithm)syn_cbor_read_int(&pr);
            } else {
                syn_cbor_skip(&pr);
            }
        }
    }

    size_t exp_tag_len = 0U;
    if (parsed_alg == SYN_COSE_ALGO_HMAC_256_256) {
        exp_tag_len = 32U;
    } else if (parsed_alg == SYN_COSE_ALGO_HMAC_256_64) {
        exp_tag_len = 8U;
    } else {
        return SYN_ERROR;
    }

    /* 1. Unprotected header */
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;
    size_t unprot_pairs = syn_cbor_read_map_begin(&r);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }
    for (size_t i = 0; i < unprot_pairs; i++) {
        uint64_t label = syn_cbor_read_uint(&r);
        if (label == SYN_COSE_HEADER_KID) {
            parsed_kid_len = syn_cbor_read_bytes(&r, parsed_kid, sizeof(parsed_kid));
        } else {
            syn_cbor_skip(&r);
        }
    }

    /* 2. Payload */
    uint8_t payload_buf[1024];
    size_t payload_len = syn_cbor_read_bytes(&r, payload_buf, sizeof(payload_buf));
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    /* 3. Tag */
    uint8_t tag_buf[SYN_COSE_MAX_TAG_LEN];
    size_t tag_len = syn_cbor_read_bytes(&r, tag_buf, sizeof(tag_buf));
    if (!syn_cbor_reader_ok(&r) || tag_len != exp_tag_len) {
        return SYN_ERROR;
    }

    if (parsed_out != NULL) {
        parsed_out->alg = parsed_alg;
        (void)memcpy(parsed_out->kid, parsed_kid, parsed_kid_len);
        parsed_out->kid_len = parsed_kid_len;
        parsed_out->payload = payload_buf;
        parsed_out->payload_len = payload_len;
        parsed_out->tag = tag_buf;
        parsed_out->tag_len = tag_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 4. Reconstruct MAC_structure */
    uint8_t mac_structure[1024];
    size_t mac_struct_len = 0;
    if (!build_mac_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             payload_buf, payload_len, mac_structure, sizeof(mac_structure),
                             &mac_struct_len)) {
        return SYN_ERROR;
    }

    /* 5. Compute HMAC and constant-time compare */
    uint8_t expected_tag[32];
    syn_hmac_sha256(key, key_len, mac_structure, mac_struct_len, expected_tag);

    uint8_t diff = 0U;
    for (size_t i = 0U; i < exp_tag_len; i++) {
        diff |= (uint8_t)(tag_buf[i] ^ expected_tag[i]);
    }

    return (diff == 0U) ? SYN_OK : SYN_ERROR;
}

/* ── COSE_Key API ────────────────────────────────────────────────────────── */

SYN_Status syn_cose_key_encode(const SYN_COSE_Key *key, uint8_t *out_buf, size_t out_buf_size,
                               size_t *out_len)
{
    if (key == NULL || out_buf == NULL || out_len == NULL) {
        return SYN_ERROR;
    }

    SYN_CborWriter w;
    syn_cbor_writer_init(&w, out_buf, out_buf_size);

    /* Count map entries */
    size_t count = 1; /* kty */
    if (key->kid_len > 0U) {
        count++;
    }
    if (key->alg != SYN_COSE_ALGO_UNKNOWN) {
        count++;
    }
    if (key->kty == SYN_COSE_KTY_SYMMETRIC) {
        if (key->priv_d_len > 0U) {
            count++; /* k = -1 */
        }
    } else if (key->kty == SYN_COSE_KTY_OKP) {
        if (key->crv != SYN_COSE_CRV_RESERVED) {
            count++; /* crv = -1 */
        }
        if (key->pub_x_len > 0U) {
            count++; /* x = -2 */
        }
        if (key->priv_d_len > 0U) {
            count++; /* d = -4 */
        }
    } else if (key->kty == SYN_COSE_KTY_EC2) {
        if (key->crv != SYN_COSE_CRV_RESERVED) {
            count++; /* crv = -1 */
        }
        if (key->pub_x_len > 0U) {
            count++; /* x = -2 */
        }
        if (key->pub_y_len > 0U) {
            count++; /* y = -3 */
        }
        if (key->priv_d_len > 0U) {
            count++; /* d = -4 */
        }
    } else {
        return SYN_ERROR;
    }

    syn_cbor_write_map_begin(&w, count);

    /* 1: kty */
    syn_cbor_write_int(&w, SYN_COSE_KEY_KTY);
    syn_cbor_write_int(&w, (int64_t)key->kty);

    /* 2: kid */
    if (key->kid_len > 0U) {
        syn_cbor_write_int(&w, SYN_COSE_KEY_KID);
        syn_cbor_write_bytes(&w, key->kid, key->kid_len);
    }

    /* 3: alg */
    if (key->alg != SYN_COSE_ALGO_UNKNOWN) {
        syn_cbor_write_int(&w, SYN_COSE_KEY_ALG);
        syn_cbor_write_int(&w, (int64_t)key->alg);
    }

    if (key->kty == SYN_COSE_KTY_SYMMETRIC) {
        if (key->priv_d_len > 0U) {
            syn_cbor_write_int(&w, SYN_COSE_KEY_K);
            syn_cbor_write_bytes(&w, key->priv_d, key->priv_d_len);
        }
    } else if (key->kty == SYN_COSE_KTY_OKP || key->kty == SYN_COSE_KTY_EC2) {
        if (key->crv != SYN_COSE_CRV_RESERVED) {
            syn_cbor_write_int(&w, SYN_COSE_KEY_CRV);
            syn_cbor_write_int(&w, (int64_t)key->crv);
        }
        if (key->pub_x_len > 0U) {
            syn_cbor_write_int(&w, SYN_COSE_KEY_X);
            syn_cbor_write_bytes(&w, key->pub_x, key->pub_x_len);
        }
        if (key->kty == SYN_COSE_KTY_EC2 && key->pub_y_len > 0U) {
            syn_cbor_write_int(&w, SYN_COSE_KEY_Y);
            syn_cbor_write_bytes(&w, key->pub_y, key->pub_y_len);
        }
        if (key->priv_d_len > 0U) {
            syn_cbor_write_int(&w, SYN_COSE_KEY_D);
            syn_cbor_write_bytes(&w, key->priv_d, key->priv_d_len);
        }
    }

    if (!syn_cbor_writer_ok(&w)) {
        return SYN_ERROR;
    }

    *out_len = syn_cbor_writer_len(&w);
    return SYN_OK;
}

SYN_Status syn_cose_key_decode(const uint8_t *buf, size_t buf_len, SYN_COSE_Key *key_out)
{
    if (buf == NULL || buf_len == 0U || key_out == NULL) {
        return SYN_ERROR;
    }

    memset(key_out, 0, sizeof(*key_out));

    SYN_CborReader r;
    syn_cbor_reader_init(&r, buf, buf_len);

    size_t map_sz = syn_cbor_read_map_begin(&r);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }

    for (size_t i = 0; i < map_sz; i++) {
        int64_t label = syn_cbor_read_int(&r);

        switch (label) {
        case SYN_COSE_KEY_KTY: {
            int64_t val = syn_cbor_read_int(&r);
            key_out->kty = (SYN_COSE_KeyType)val;
            break;
        }
        case SYN_COSE_KEY_KID:
            key_out->kid_len = syn_cbor_read_bytes(&r, key_out->kid, sizeof(key_out->kid));
            break;
        case SYN_COSE_KEY_ALG: {
            int64_t val = syn_cbor_read_int(&r);
            key_out->alg = (SYN_COSE_Algorithm)val;
            break;
        }
        case SYN_COSE_KEY_CRV: { /* -1: crv or k */
            if (key_out->kty == SYN_COSE_KTY_SYMMETRIC ||
                syn_cbor_peek_type(&r) == SYN_CBOR_BYTES) {
                key_out->priv_d_len =
                    syn_cbor_read_bytes(&r, key_out->priv_d, sizeof(key_out->priv_d));
            } else {
                int64_t val = syn_cbor_read_int(&r);
                key_out->crv = (SYN_COSE_Curve)val;
            }
            break;
        }
        case SYN_COSE_KEY_X: /* -2 */
            key_out->pub_x_len = syn_cbor_read_bytes(&r, key_out->pub_x, sizeof(key_out->pub_x));
            break;
        case SYN_COSE_KEY_Y: /* -3 */
            key_out->pub_y_len = syn_cbor_read_bytes(&r, key_out->pub_y, sizeof(key_out->pub_y));
            break;
        case SYN_COSE_KEY_D: /* -4 */
            key_out->priv_d_len = syn_cbor_read_bytes(&r, key_out->priv_d, sizeof(key_out->priv_d));
            break;
        default:
            syn_cbor_skip(&r);
            break;
        }
    }

    return (key_out->kty != SYN_COSE_KTY_RESERVED) ? SYN_OK : SYN_ERROR;
}

#endif /* SYN_USE_COSE */
```


