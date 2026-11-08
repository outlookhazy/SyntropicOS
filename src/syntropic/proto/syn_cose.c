/**
 * @file syn_cose.c
 * @brief Zero-Heap CBOR Object Signing and Encryption (COSE - RFC 9052) implementation.
 */

#include "syntropic/proto/syn_cose.h"

#if SYN_COSE_ENABLED

#include "syntropic/util/syn_random.h"

#include <string.h>

/** @cond INTERNAL */

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

/* Parse protected CBOR header map and extract COSE algorithm label (1) */
static bool cose_parse_protected_alg(const uint8_t *buf, size_t len, SYN_COSE_Algorithm *alg_out)
{
    *alg_out = SYN_COSE_ALGO_UNKNOWN;
    if (len == 0U) {
        return true;
    }

    SYN_CborReader pr;
    syn_cbor_reader_init(&pr, buf, len);
    size_t pairs = syn_cbor_read_map_begin(&pr);
    if (!syn_cbor_reader_ok(&pr)) {
        return false;
    }

    for (size_t i = 0U; i < pairs && syn_cbor_reader_ok(&pr); i++) {
        uint64_t label = syn_cbor_read_uint(&pr);
        if (!syn_cbor_reader_ok(&pr)) {
            return false;
        }
        if (label == SYN_COSE_HEADER_ALG) {
            *alg_out = (SYN_COSE_Algorithm)syn_cbor_read_int(&pr);
        } else {
            syn_cbor_skip(&pr);
        }
    }

    return syn_cbor_reader_ok(&pr);
}

/* Parse unprotected header map for Sign1 / Mac0 (extracting optional kid) */
static bool cose_parse_unprot_kid(SYN_CborReader *r, uint8_t kid[SYN_COSE_MAX_KID_LEN],
                                  size_t *kid_len)
{
    *kid_len = 0U;
    size_t pairs = syn_cbor_read_map_begin(r);
    if (!syn_cbor_reader_ok(r)) {
        return false;
    }

    for (size_t i = 0U; i < pairs && syn_cbor_reader_ok(r); i++) {
        uint64_t label = syn_cbor_read_uint(r);
        if (!syn_cbor_reader_ok(r)) {
            return false;
        }
        if (label == SYN_COSE_HEADER_KID) {
            *kid_len = syn_cbor_read_bytes(r, kid, SYN_COSE_MAX_KID_LEN);
            if (*kid_len > SYN_COSE_MAX_KID_LEN) {
                return false;
            }
        } else {
            syn_cbor_skip(r);
        }
    }

    return syn_cbor_reader_ok(r);
}

/* Parse unprotected header map for Encrypt0 (extracting IV and optional kid) */
static bool cose_parse_unprot_enc0(SYN_CborReader *r, uint8_t iv[SYN_COSE_MAX_IV_LEN],
                                   size_t *iv_len, uint8_t kid[SYN_COSE_MAX_KID_LEN],
                                   size_t *kid_len)
{
    *iv_len = 0U;
    *kid_len = 0U;
    size_t pairs = syn_cbor_read_map_begin(r);
    if (!syn_cbor_reader_ok(r)) {
        return false;
    }

    for (size_t i = 0U; i < pairs && syn_cbor_reader_ok(r); i++) {
        uint64_t label = syn_cbor_read_uint(r);
        if (!syn_cbor_reader_ok(r)) {
            return false;
        }
        if (label == SYN_COSE_HEADER_IV) {
            *iv_len = syn_cbor_read_bytes(r, iv, SYN_COSE_MAX_IV_LEN);
            if (*iv_len > SYN_COSE_MAX_IV_LEN) {
                return false;
            }
        } else if (label == SYN_COSE_HEADER_KID) {
            *kid_len = syn_cbor_read_bytes(r, kid, SYN_COSE_MAX_KID_LEN);
            if (*kid_len > SYN_COSE_MAX_KID_LEN) {
                return false;
            }
        } else {
            syn_cbor_skip(r);
        }
    }

    return syn_cbor_reader_ok(r);
}

/* Resolve key length, tag length, and CCM flag for COSE_Encrypt0 algorithms */
static bool cose_enc0_alg_params(SYN_COSE_Algorithm alg, size_t *key_len, size_t *tag_len,
                                 bool *is_ccm)
{
    *tag_len = 16U;
    *is_ccm = false;

    if (alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        *key_len = 32U;
        return true;
    }
    if (alg == SYN_COSE_ALGO_A128GCM || alg == SYN_COSE_ALGO_AES_CCM_16_64_128 ||
        alg == SYN_COSE_ALGO_AES_CCM_16_128_128) {
        *key_len = 16U;
        *is_ccm = (alg != SYN_COSE_ALGO_A128GCM);
        if (alg == SYN_COSE_ALGO_AES_CCM_16_64_128) {
            *tag_len = 8U;
        }
        return true;
    }
    if (alg == SYN_COSE_ALGO_A192GCM) {
        *key_len = 24U;
        return true;
    }
    if (alg == SYN_COSE_ALGO_A256GCM || alg == SYN_COSE_ALGO_AES_CCM_16_64_256 ||
        alg == SYN_COSE_ALGO_AES_CCM_16_128_256) {
        *key_len = 32U;
        *is_ccm = (alg != SYN_COSE_ALGO_A256GCM);
        if (alg == SYN_COSE_ALGO_AES_CCM_16_64_256) {
            *tag_len = 8U;
        }
        return true;
    }
    return false;
}

/* Execute authenticated encryption for COSE_Encrypt0 */
static SYN_Status cose_aead_encrypt(SYN_COSE_Algorithm alg, size_t key_len, size_t tag_len,
                                    bool is_ccm, const uint8_t *key, const uint8_t *iv,
                                    size_t iv_len, const uint8_t *aad, size_t aad_len,
                                    const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                                    uint8_t tag[16])
{
    if (alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        if (pt_len > 0U && pt != NULL) {
            syn_aead_encrypt(key, iv, aad, aad_len, pt, pt_len, ct_out, tag);
        } else {
            syn_aead_encrypt(key, iv, aad, aad_len, NULL, 0U, NULL, tag);
        }
        return SYN_OK;
    }
    if (is_ccm) {
        SYN_AES_Context aes_ctx;
        SYN_Status st = syn_aes_init(&aes_ctx, key, key_len);
        if (st == SYN_OK) {
            st = syn_aes_ccm_encrypt(&aes_ctx, iv, iv_len, aad, aad_len, pt, pt_len, ct_out, tag,
                                     tag_len);
        }
        syn_secure_zero(&aes_ctx, sizeof(aes_ctx));
        return (st == SYN_OK) ? SYN_OK : SYN_ERROR;
    }
    SYN_AES_GCM_Context gcm_ctx;
    SYN_Status st = syn_aes_gcm_init(&gcm_ctx, key, key_len);
    if (st == SYN_OK) {
        st = syn_aes_gcm_encrypt(&gcm_ctx, iv, iv_len, aad, aad_len, pt, pt_len, ct_out, tag);
    }
    syn_secure_zero(&gcm_ctx, sizeof(gcm_ctx));
    return (st == SYN_OK) ? SYN_OK : SYN_ERROR;
}

/* Execute authenticated decryption for COSE_Encrypt0 */
static SYN_Status cose_aead_decrypt(SYN_COSE_Algorithm alg, size_t key_len, size_t tag_len,
                                    bool is_ccm, const uint8_t *key, const uint8_t *iv,
                                    size_t iv_len, const uint8_t *aad, size_t aad_len,
                                    const uint8_t *ct, size_t pt_len, const uint8_t *tag,
                                    uint8_t *pt_out)
{
    if (alg == SYN_COSE_ALGO_CHACHA20_POLY1305) {
        return syn_aead_decrypt(key, iv, aad, aad_len, ct, pt_len, tag, pt_out) ? SYN_OK
                                                                                : SYN_ERROR;
    }
    if (is_ccm) {
        SYN_AES_Context aes_ctx;
        SYN_Status st = syn_aes_init(&aes_ctx, key, key_len);
        if (st == SYN_OK) {
            st = syn_aes_ccm_decrypt(&aes_ctx, iv, iv_len, aad, aad_len, ct, pt_len, tag, tag_len,
                                     pt_out);
        }
        syn_secure_zero(&aes_ctx, sizeof(aes_ctx));
        return (st == SYN_OK) ? SYN_OK : SYN_ERROR;
    }
    SYN_AES_GCM_Context gcm_ctx;
    SYN_Status st = syn_aes_gcm_init(&gcm_ctx, key, key_len);
    if (st == SYN_OK) {
        st = syn_aes_gcm_decrypt(&gcm_ctx, iv, iv_len, aad, aad_len, ct, pt_len, pt_out, tag);
    }
    syn_secure_zero(&gcm_ctx, sizeof(gcm_ctx));
    return (st == SYN_OK) ? SYN_OK : SYN_ERROR;
}

/* Read bounded byte string field in COSE_Key map */
static bool cose_key_read_bounded(SYN_CborReader *r, uint8_t *dst, size_t cap, size_t *out_len)
{
    size_t n = syn_cbor_read_bytes(r, dst, cap);
    if (!syn_cbor_reader_ok(r) || n > cap) {
        return false;
    }
    *out_len = n;
    return true;
}

/* Decode one label/value entry of a COSE_Key map */
static bool cose_key_decode_entry(SYN_CborReader *r, int64_t label, SYN_COSE_Key *key_out)
{
    switch (label) {
    case SYN_COSE_KEY_KTY:
        key_out->kty = (SYN_COSE_KeyType)syn_cbor_read_int(r);
        return syn_cbor_reader_ok(r);
    case SYN_COSE_KEY_KID:
        return cose_key_read_bounded(r, key_out->kid, sizeof(key_out->kid), &key_out->kid_len);
    case SYN_COSE_KEY_ALG:
        key_out->alg = (SYN_COSE_Algorithm)syn_cbor_read_int(r);
        return syn_cbor_reader_ok(r);
    case SYN_COSE_KEY_CRV:
        if (key_out->kty == SYN_COSE_KTY_SYMMETRIC || syn_cbor_peek_type(r) == SYN_CBOR_BYTES) {
            return cose_key_read_bounded(r, key_out->priv_d, sizeof(key_out->priv_d),
                                         &key_out->priv_d_len);
        }
        key_out->crv = (SYN_COSE_Curve)syn_cbor_read_int(r);
        return syn_cbor_reader_ok(r);
    case SYN_COSE_KEY_X:
        return cose_key_read_bounded(r, key_out->pub_x, sizeof(key_out->pub_x),
                                     &key_out->pub_x_len);
    case SYN_COSE_KEY_Y:
        return cose_key_read_bounded(r, key_out->pub_y, sizeof(key_out->pub_y),
                                     &key_out->pub_y_len);
    case SYN_COSE_KEY_D:
        return cose_key_read_bounded(r, key_out->priv_d, sizeof(key_out->priv_d),
                                     &key_out->priv_d_len);
    default:
        syn_cbor_skip(r);
        return syn_cbor_reader_ok(r);
    }
}

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

/**
 * @brief Build serialized MAC_structure per RFC 9052 Section 6.3.
 * @param body_protected Protected header byte buffer.
 * @param body_protected_len Length of protected header buffer.
 * @param external_aad External associated data buffer.
 * @param external_aad_len Length of external AAD buffer.
 * @param payload Payload data buffer.
 * @param payload_len Length of payload data buffer.
 * @param out_buf Output buffer for serialized CBOR structure.
 * @param out_buf_size Capacity of output buffer.
 * @param out_len Pointer to store output byte count.
 * @return true on successful encoding, false otherwise.
 */
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

static SYN_Status cose_verify_sign1_sig(SYN_COSE_Algorithm alg, const uint8_t *public_key,
                                        size_t public_key_len, const uint8_t *sig_ptr,
                                        const uint8_t *sig_structure, size_t sig_struct_len)
{
    if (alg == SYN_COSE_ALGO_EDDSA) {
        if (public_key_len != SYN_ED25519_PUBLIC_KEY_SIZE ||
            !syn_ed25519_verify(sig_ptr, sig_structure, sig_struct_len, public_key)) {
            return SYN_ERROR;
        }
        return SYN_OK;
    }

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
    return syn_p256_verify_ecdsa(hash, sig_ptr, sig_ptr + 32, px, py) ? SYN_OK : SYN_ERROR;
}

static bool cose_parse_enc0_prot_and_unprot(SYN_CborReader *r, uint8_t *protected_buf,
                                            size_t *protected_len, SYN_COSE_Algorithm *parsed_alg,
                                            size_t *key_len, size_t *tag_len, bool *is_ccm,
                                            uint8_t *parsed_iv, size_t *parsed_iv_len,
                                            uint8_t *parsed_kid, size_t *parsed_kid_len)
{
    *protected_len = syn_cbor_read_bytes(r, protected_buf, SYN_COSE_MAX_PROTECTED_LEN);
    if (!syn_cbor_reader_ok(r) || *protected_len > SYN_COSE_MAX_PROTECTED_LEN ||
        !cose_parse_protected_alg(protected_buf, *protected_len, parsed_alg) ||
        !cose_enc0_alg_params(*parsed_alg, key_len, tag_len, is_ccm) ||
        !cose_parse_unprot_enc0(r, parsed_iv, parsed_iv_len, parsed_kid, parsed_kid_len)) {
        return false;
    }
    return (*is_ccm || *parsed_iv_len == 12U);
}

static size_t cose_mac0_tag_len(SYN_COSE_Algorithm alg)
{
    if (alg == SYN_COSE_ALGO_HMAC_256_256) {
        return 32U;
    }
    if (alg == SYN_COSE_ALGO_HMAC_256_64) {
        return 8U;
    }
    return 0U;
}

static bool cose_parse_prot_and_kid(SYN_CborReader *r, uint8_t *protected_buf,
                                    size_t *protected_len, SYN_COSE_Algorithm *parsed_alg,
                                    uint8_t *parsed_kid, size_t *parsed_kid_len)
{
    *protected_len = syn_cbor_read_bytes(r, protected_buf, SYN_COSE_MAX_PROTECTED_LEN);
    if (!syn_cbor_reader_ok(r) || *protected_len > SYN_COSE_MAX_PROTECTED_LEN ||
        !cose_parse_protected_alg(protected_buf, *protected_len, parsed_alg) ||
        !cose_parse_unprot_kid(r, parsed_kid, parsed_kid_len)) {
        return false;
    }
    return true;
}

static SYN_Status cose_mac0_verify_tag(const uint8_t *key, size_t key_len,
                                       const uint8_t *mac_struct, size_t mac_struct_len,
                                       const uint8_t *tag_ptr, size_t exp_tag_len)
{
    uint8_t expected_tag[32];
    syn_hmac_sha256(key, key_len, mac_struct, mac_struct_len, expected_tag);

    uint8_t diff = 0U;
    for (size_t i = 0U; i < exp_tag_len; i++) {
        diff |= (uint8_t)(tag_ptr[i] ^ expected_tag[i]);
    }
    syn_secure_zero(expected_tag, sizeof(expected_tag));
    return (diff == 0U) ? SYN_OK : SYN_ERROR;
}

/** @endcond */

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
    uint8_t sig_structure[SYN_COSE_MAX_STRUCT_LEN];
    size_t sig_struct_len = 0;
    if (!build_sig_structure(protected_raw, protected_raw_len, external_aad, external_aad_len,
                             payload, payload_len, sig_structure, sizeof(sig_structure),
                             &sig_struct_len)) {
        return SYN_ERROR;
    }

    /* 3. Compute signature */
    uint8_t signature[SYN_COSE_MAX_SIG_LEN];
    size_t sig_len = 64U;
    bool sign_ok = false;

    if (alg == SYN_COSE_ALGO_EDDSA) {
        sign_ok =
            syn_ed25519_sign(sig_structure, sig_struct_len, secret_key, public_key, signature);
    } else {
        /* ES256: Hash Sig_structure with SHA-256 then sign with P-256 ECDSA */
        uint8_t hash[SYN_SHA256_DIGEST_SIZE];
        uint8_t nonce_k[32];
        syn_sha256(sig_structure, sig_struct_len, hash);

        if (syn_random_fill(nonce_k, sizeof(nonce_k)) == SYN_OK) {
            sign_ok = syn_p256_sign_ecdsa(secret_key, nonce_k, hash, signature, signature + 32);
        }
        syn_secure_zero(nonce_k, sizeof(nonce_k));
    }

    if (!sign_ok) {
        return SYN_ERROR;
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

    /* 1 & 2. Read protected and unprotected headers */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = 0U;
    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;
    if (!cose_parse_prot_and_kid(&r, protected_buf, &protected_len, &parsed_alg, parsed_kid,
                                 &parsed_kid_len) ||
        (parsed_alg != SYN_COSE_ALGO_EDDSA && parsed_alg != SYN_COSE_ALGO_ES256)) {
        return SYN_ERROR;
    }

    /* 3. Read payload (zero-copy view into msg) */
    size_t payload_len = syn_cbor_read_bytes(&r, NULL, 0U);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }
    const uint8_t *payload_ptr = &r.buf[r.pos - payload_len];

    /* 4. Read signature (zero-copy view into msg) */
    size_t sig_len = syn_cbor_read_bytes(&r, NULL, 0U);
    if (!syn_cbor_reader_ok(&r) || sig_len != 64U) {
        return SYN_ERROR;
    }
    const uint8_t *sig_ptr = &r.buf[r.pos - sig_len];

    /* Populate optional output struct */
    if (parsed_out != NULL) {
        parsed_out->alg = parsed_alg;
        (void)memcpy(parsed_out->kid, parsed_kid, parsed_kid_len);
        parsed_out->kid_len = parsed_kid_len;
        parsed_out->payload = payload_ptr;
        parsed_out->payload_len = payload_len;
        parsed_out->signature = sig_ptr;
        parsed_out->signature_len = sig_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 5. Reconstruct Sig_structure */
    uint8_t sig_structure[SYN_COSE_MAX_STRUCT_LEN];
    size_t sig_struct_len = 0;
    if (!build_sig_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             payload_ptr, payload_len, sig_structure, sizeof(sig_structure),
                             &sig_struct_len)) {
        return SYN_ERROR;
    }

    /* 6. Verify signature */
    return cose_verify_sign1_sig(parsed_alg, public_key, public_key_len, sig_ptr, sig_structure,
                                 sig_struct_len);
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

    size_t key_len = 0U;
    size_t tag_len = 16U;
    bool is_ccm = false;
    if (!cose_enc0_alg_params(alg, &key_len, &tag_len, &is_ccm)) {
        return SYN_ERROR;
    }
    if (!is_ccm && iv_len != 12U) {
        return SYN_ERROR;
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
    if (plaintext_len > SYN_COSE_MAX_ENCRYPT0_PAYLOAD) {
        return SYN_ERROR;
    }
    uint8_t enc_structure[SYN_COSE_MAX_ENC_STRUCT_LEN];
    size_t enc_struct_len = 0;
    if (!build_enc_structure(protected_raw, protected_raw_len, external_aad, external_aad_len,
                             enc_structure, sizeof(enc_structure), &enc_struct_len)) {
        return SYN_ERROR;
    }

    /* 3. Encode COSE_Encrypt0 header + bstr prefix directly into out_buf */
    size_t total_ct_len = plaintext_len + tag_len;
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

    size_t bstr_hdr_pos = syn_cbor_writer_len(&w);
    syn_cbor_write_uint(&w, (uint64_t)total_ct_len);
    size_t ct_pos = syn_cbor_writer_len(&w);
    if (!syn_cbor_writer_ok(&w) || total_ct_len > (out_buf_size - ct_pos)) {
        return SYN_ERROR;
    }
    out_buf[bstr_hdr_pos] |= 0x40U; /* CBOR major type 2 (byte string) */

    /* 4. Encrypt plaintext || tag directly into out_buf */
    uint8_t tag[16];
    if (cose_aead_encrypt(alg, key_len, tag_len, is_ccm, key, iv, iv_len, enc_structure,
                          enc_struct_len, plaintext, plaintext_len, out_buf + ct_pos,
                          tag) != SYN_OK) {
        return SYN_ERROR;
    }
    (void)memcpy(out_buf + ct_pos + plaintext_len, tag, tag_len);

    *out_len = ct_pos + total_ct_len;
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

    /* 1 & 2. Read protected + unprotected headers */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = 0U;
    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    size_t key_len = 0U;
    size_t tag_len = 16U;
    bool is_ccm = false;
    uint8_t parsed_iv[SYN_COSE_MAX_IV_LEN] = {0};
    size_t parsed_iv_len = 0;
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;
    if (!cose_parse_enc0_prot_and_unprot(&r, protected_buf, &protected_len, &parsed_alg, &key_len,
                                         &tag_len, &is_ccm, parsed_iv, &parsed_iv_len, parsed_kid,
                                         &parsed_kid_len)) {
        return SYN_ERROR;
    }

    /* 3. Read ciphertext + tag (zero-copy view into msg) */
    size_t ct_tag_len = syn_cbor_read_bytes(&r, NULL, 0U);
    if (!syn_cbor_reader_ok(&r) || ct_tag_len < tag_len) {
        return SYN_ERROR;
    }
    const uint8_t *ct_tag_ptr = &r.buf[r.pos - ct_tag_len];

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
        parsed_out->ciphertext = ct_tag_ptr;
        parsed_out->ciphertext_len = ct_tag_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 4. Reconstruct Enc_structure */
    uint8_t enc_structure[SYN_COSE_MAX_ENC_STRUCT_LEN];
    size_t enc_struct_len = 0;
    if (!build_enc_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             enc_structure, sizeof(enc_structure), &enc_struct_len)) {
        return SYN_ERROR;
    }

    /* 5. Decrypt and verify tag */
    const uint8_t *tag = ct_tag_ptr + pt_len;
    if (cose_aead_decrypt(parsed_alg, key_len, tag_len, is_ccm, key, parsed_iv, parsed_iv_len,
                          enc_structure, enc_struct_len, ct_tag_ptr, pt_len, tag,
                          out_plaintext) != SYN_OK) {
        return SYN_ERROR;
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
    uint8_t mac_structure[SYN_COSE_MAX_STRUCT_LEN];
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

    /* 0 & 1. Protected and unprotected headers */
    uint8_t protected_buf[SYN_COSE_MAX_PROTECTED_LEN];
    size_t protected_len = 0U;
    SYN_COSE_Algorithm parsed_alg = SYN_COSE_ALGO_UNKNOWN;
    uint8_t parsed_kid[SYN_COSE_MAX_KID_LEN] = {0};
    size_t parsed_kid_len = 0;
    size_t exp_tag_len = 0U;
    if (!cose_parse_prot_and_kid(&r, protected_buf, &protected_len, &parsed_alg, parsed_kid,
                                 &parsed_kid_len)) {
        return SYN_ERROR;
    }
    exp_tag_len = cose_mac0_tag_len(parsed_alg);
    if (exp_tag_len == 0U) {
        return SYN_ERROR;
    }

    /* 2. Payload (zero-copy view into msg) */
    size_t payload_len = syn_cbor_read_bytes(&r, NULL, 0U);
    if (!syn_cbor_reader_ok(&r)) {
        return SYN_ERROR;
    }
    const uint8_t *payload_ptr = &r.buf[r.pos - payload_len];

    /* 3. Tag (zero-copy view into msg) */
    size_t tag_len = syn_cbor_read_bytes(&r, NULL, 0U);
    if (!syn_cbor_reader_ok(&r) || tag_len != exp_tag_len) {
        return SYN_ERROR;
    }
    const uint8_t *tag_ptr = &r.buf[r.pos - tag_len];

    if (parsed_out != NULL) {
        parsed_out->alg = parsed_alg;
        (void)memcpy(parsed_out->kid, parsed_kid, parsed_kid_len);
        parsed_out->kid_len = parsed_kid_len;
        parsed_out->payload = payload_ptr;
        parsed_out->payload_len = payload_len;
        parsed_out->tag = tag_ptr;
        parsed_out->tag_len = tag_len;
        (void)memcpy(parsed_out->protected_hdr, protected_buf, protected_len);
        parsed_out->protected_hdr_len = protected_len;
    }

    /* 4. Reconstruct MAC_structure */
    uint8_t mac_structure[SYN_COSE_MAX_STRUCT_LEN];
    size_t mac_struct_len = 0;
    if (!build_mac_structure(protected_buf, protected_len, external_aad, external_aad_len,
                             payload_ptr, payload_len, mac_structure, sizeof(mac_structure),
                             &mac_struct_len)) {
        return SYN_ERROR;
    }

    /* 5. Compute HMAC and constant-time compare */
    return cose_mac0_verify_tag(key, key_len, mac_structure, mac_struct_len, tag_ptr, exp_tag_len);
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

    for (size_t i = 0U; i < map_sz && syn_cbor_reader_ok(&r); i++) {
        int64_t label = syn_cbor_read_int(&r);
        if (!syn_cbor_reader_ok(&r) || !cose_key_decode_entry(&r, label, key_out)) {
            syn_secure_zero(key_out, sizeof(*key_out));
            return SYN_ERROR;
        }
    }

    if (key_out->kty == SYN_COSE_KTY_RESERVED) {
        syn_secure_zero(key_out, sizeof(*key_out));
        return SYN_ERROR;
    }
    return SYN_OK;
}

#endif /* SYN_COSE_ENABLED */
