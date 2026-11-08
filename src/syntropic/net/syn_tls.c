/**
 * @file syn_tls.c
 * @brief Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter implementation.
 */

#include "syntropic/net/syn_tls.h"

#if SYN_TLS_ENABLED

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hmac.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/util/syn_random.h"

#include <string.h>

/** @cond INTERNAL */

/** @brief TLS record header size in bytes (5). */
#define TLS_RECORD_HEADER_LEN 5U
/** @brief TLS Handshake record content type (0x16). */
#define TLS_CONTENT_TYPE_HANDSHAKE 0x16U
/** @brief TLS Application Data record content type (0x17). */
#define TLS_CONTENT_TYPE_APPLICATION_DATA 0x17U

/** @brief TLS Handshake ClientHello message type (0x01). */
#define TLS_HANDSHAKE_CLIENT_HELLO 0x01U
/** @brief TLS Handshake ServerHello message type (0x02). */
#define TLS_HANDSHAKE_SERVER_HELLO 0x02U
/** @brief TLS Handshake CertificateVerify message type (0x0F). */
#define TLS_HANDSHAKE_CERTIFICATE_VERIFY 0x0FU
/** @brief TLS Handshake Finished message type (0x14). */
#define TLS_HANDSHAKE_FINISHED 0x14U
/** @brief Fixed 72-byte CertificateVerify wire message size (4B hdr + 2B alg + 2B len + 64B sig).
 */
#define TLS_CERT_VERIFY_LEN 72U

/**
 * @brief Internal send callback adapter for TLS transport binding.
 * @param data Data buffer.
 * @param len Data length.
 * @param ctx TLS context pointer.
 * @return true on successful send.
 */
static bool tls_transport_send_cb(const uint8_t *data, size_t len, void *ctx)
{
    SYN_TLS_Context *tls = (SYN_TLS_Context *)ctx;
    return syn_tls_send(tls, data, len);
}

/**
 * @brief Internal receive callback adapter for TLS transport binding.
 * @param data Data buffer.
 * @param max_len Max buffer capacity.
 * @param out_len [out] Received bytes.
 * @param ctx TLS context pointer.
 * @return true on successful receive.
 */
static bool tls_transport_recv_cb(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_TLS_Context *tls = (SYN_TLS_Context *)ctx;
    return syn_tls_recv(tls, data, max_len, out_len);
}

/**
 * @brief Check if buffered plaintext or underlying transport has pending data.
 * @param ctx TLS context pointer.
 * @return true if decrypted bytes are buffered or underlying transport is readable.
 */
static bool tls_transport_has_data_cb(const void *ctx)
{
    const SYN_TLS_Context *tls = (const SYN_TLS_Context *)ctx;
    if (tls == NULL) {
        return false;
    }
    if (tls->app_rx_head < tls->app_rx_tail) {
        return true;
    }
    return (tls->underlying_transport != NULL) && syn_transport_has_data(tls->underlying_transport);
}

/**
 * @brief Zero sensitive memory without compiler dead-store elimination.
 * @param v Buffer to wipe.
 * @param n Length in bytes.
 */
static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n--) {
        *p++ = 0;
    }
}

/**
 * @brief Map TLS 1.3 cipher suite enum to 16-bit RFC 8446 wire identifier.
 * @param suite Cipher suite enum value.
 * @return 16-bit TLS 1.3 cipher suite wire code.
 */
static uint16_t tls_cipher_wire_id(SYN_TLS_CipherSuite suite)
{
    static const uint16_t wire_ids[5] = {0x1303U, 0x1301U, 0x1302U, 0x1304U, 0x1305U};
    size_t idx = ((size_t)suite < 5U) ? (size_t)suite : 0U;
    return wire_ids[idx];
}

/**
 * @brief Reset running SHA-256 and SHA-384 handshake transcript hashes.
 * @param ctx TLS context pointer.
 */
static void tls_transcript_reset(SYN_TLS_Context *ctx)
{
    syn_sha256_init(&ctx->transcript_hash);
    syn_sha384_init(&ctx->transcript_sha384);
}

/**
 * @brief Feed handshake message bytes into running SHA-256 and SHA-384 transcript hashes.
 * @param ctx  TLS context pointer.
 * @param data Handshake message bytes (excluding 5-byte TLS record header).
 * @param len  Byte length.
 */
static void tls_transcript_update(SYN_TLS_Context *ctx, const uint8_t *data, size_t len)
{
    syn_sha256_update(&ctx->transcript_hash, data, len);
    syn_sha512_update(&ctx->transcript_sha384, data, len);
}

/**
 * @brief Derive TLS 1.3 traffic key or IV using HKDF-Expand-Label.
 * @param ctx     TLS context pointer.
 * @param secret  Traffic secret (32 or 48 bytes).
 * @param label   Label ("key" or "iv").
 * @param out     [out] Output buffer.
 * @param out_len Output length (16/32 for key, 12 for IV).
 */
static void derive_traffic_key_or_iv(const SYN_TLS_Context *ctx, const uint8_t *secret,
                                     const char *label, uint8_t *out, size_t out_len)
{
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_expand_label(secret, SYN_SHA384_DIGEST_SIZE, label, strlen(label), NULL, 0,
                                     out, out_len);
    } else {
        syn_hkdf_expand_label(secret, SYN_SHA256_DIGEST_SIZE, label, strlen(label), NULL, 0, out,
                              out_len);
    }
}

/**
 * @brief Compute handshake transcript digest for TLS 1.3 key schedule.
 * @param ctx               TLS context pointer.
 * @param is_sha384         true for SHA-384, false for SHA-256.
 * @param transcript_digest [out] Output digest buffer (up to 48 bytes).
 */
static void tls_compute_transcript_digest(const SYN_TLS_Context *ctx, bool is_sha384,
                                          uint8_t transcript_digest[SYN_TLS_SECRET_LEN])
{
    if (is_sha384) {
        SYN_SHA512 copy = ctx->transcript_sha384;
        syn_sha384_final(&copy, transcript_digest);
        syn_secure_zero(&copy, sizeof(copy));
    } else {
        SYN_SHA256 copy = ctx->transcript_hash;
        syn_sha256_final(&copy, transcript_digest);
        syn_secure_zero(&copy, sizeof(copy));
    }
}

/**
 * @brief Constant-time check that a 32-byte X25519 shared secret is non-zero (RFC 8446 §7.4.2).
 * @param b 32-byte buffer.
 * @return true if at least one byte is non-zero.
 */
static bool tls_is_nonzero_32(const uint8_t b[32])
{
    uint8_t acc = 0U;
    for (size_t i = 0U; i < 32U; i++) {
        acc |= b[i];
    }
    return acc != 0U;
}

/** @brief SHA-256("") empty transcript hash per RFC 8446 §7.1. */
static const uint8_t TLS_EMPTY_SHA256[32] = {
    0xE3U, 0xB0U, 0xC4U, 0x42U, 0x98U, 0xFCU, 0x1CU, 0x14U, 0x9AU, 0xFBU, 0xF4U,
    0xC8U, 0x99U, 0x6FU, 0xB9U, 0x24U, 0x27U, 0xAEU, 0x41U, 0xE4U, 0x64U, 0x9BU,
    0x93U, 0x4CU, 0xA4U, 0x95U, 0x99U, 0x1BU, 0x78U, 0x52U, 0xB8U, 0x55U};

/** @brief SHA-384("") empty transcript hash per RFC 8446 §7.1. */
static const uint8_t TLS_EMPTY_SHA384[48] = {
    0x38U, 0xB0U, 0x60U, 0xA7U, 0x51U, 0xACU, 0x96U, 0x38U, 0x4CU, 0xD9U, 0x32U, 0x7EU,
    0xB1U, 0xB1U, 0xE3U, 0x6AU, 0x21U, 0xFDU, 0xB7U, 0x11U, 0x14U, 0xBEU, 0x07U, 0x43U,
    0x4CU, 0x0CU, 0xC7U, 0xBFU, 0x63U, 0xF6U, 0xE1U, 0xDAU, 0x27U, 0x4EU, 0xDEU, 0xBFU,
    0xE7U, 0x6FU, 0x65U, 0xFBU, 0xD5U, 0x1AU, 0xD2U, 0xF1U, 0x48U, 0x98U, 0xB9U, 0x5BU};

/**
 * @brief Derive TLS 1.3 handshake secrets and master secret after ServerHello (RFC 8446 §7.1).
 * @param ctx TLS context pointer.
 * @return true if X25519 shared secret is non-zero and handshake secrets were derived.
 */
static bool derive_tls13_hs_secrets(SYN_TLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_TLS_SECRET_LEN] = {0};
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    uint8_t buf1[SYN_TLS_SECRET_LEN];
    uint8_t buf2[SYN_TLS_SECRET_LEN];
    uint8_t ecdhe_shared[32];
    uint8_t transcript_digest[SYN_TLS_SECRET_LEN];

    memset(buf1, 0, sizeof(buf1));
    size_t copy_len = hash_len;

    if (ctx->config.mode == SYN_TLS_AUTH_MODE_PSK && ctx->config.psk_secret != NULL) {
        copy_len = ctx->config.psk_secret_len;
        if (copy_len > SYN_TLS_SECRET_LEN) {
            copy_len = SYN_TLS_SECRET_LEN;
        }
        memcpy(buf1, ctx->config.psk_secret, copy_len);
    }

    syn_x25519(ecdhe_shared, ctx->my_privkey, ctx->peer_pubkey);
    if (!tls_is_nonzero_32(ecdhe_shared)) {
        syn_secure_zero(buf1, sizeof(buf1));
        syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
        return false;
    }

    tls_compute_transcript_digest(ctx, is_sha384, transcript_digest);

    if (is_sha384) {
        syn_hkdf_sha384_extract(zero_salt, hash_len, buf1, copy_len, buf2);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "derived", 7, TLS_EMPTY_SHA384, hash_len, buf1,
                                     hash_len);
        syn_hkdf_sha384_extract(buf1, hash_len, ecdhe_shared, 32U, buf2);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "c hs traffic", 12, transcript_digest,
                                     hash_len, ctx->client_handshake_secret, hash_len);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "s hs traffic", 12, transcript_digest,
                                     hash_len, ctx->server_handshake_secret, hash_len);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "derived", 7, TLS_EMPTY_SHA384, hash_len, buf1,
                                     hash_len);
        syn_hkdf_sha384_extract(buf1, hash_len, zero_salt, hash_len, ctx->master_secret);
    } else {
        syn_hkdf_extract(zero_salt, hash_len, buf1, copy_len, buf2);
        syn_hkdf_expand_label(buf2, hash_len, "derived", 7, TLS_EMPTY_SHA256, hash_len, buf1,
                              hash_len);
        syn_hkdf_extract(buf1, hash_len, ecdhe_shared, 32U, buf2);
        syn_hkdf_expand_label(buf2, hash_len, "c hs traffic", 12, transcript_digest, hash_len,
                              ctx->client_handshake_secret, hash_len);
        syn_hkdf_expand_label(buf2, hash_len, "s hs traffic", 12, transcript_digest, hash_len,
                              ctx->server_handshake_secret, hash_len);
        syn_hkdf_expand_label(buf2, hash_len, "derived", 7, TLS_EMPTY_SHA256, hash_len, buf1,
                              hash_len);
        syn_hkdf_extract(buf1, hash_len, zero_salt, hash_len, ctx->master_secret);
    }

    syn_secure_zero(buf1, sizeof(buf1));
    syn_secure_zero(buf2, sizeof(buf2));
    syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
    syn_secure_zero(transcript_digest, sizeof(transcript_digest));
    return true;
}

/**
 * @brief Derive TLS 1.3 application traffic secrets, keys, and IVs after ServerFinished.
 * @param ctx TLS context pointer.
 */
static void derive_tls13_app_keys(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t transcript_digest[SYN_TLS_SECRET_LEN];

    tls_compute_transcript_digest(ctx, is_sha384, transcript_digest);
    if (is_sha384) {
        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12,
                                     transcript_digest, hash_len, ctx->client_app_secret, hash_len);
        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12,
                                     transcript_digest, hash_len, ctx->server_app_secret, hash_len);
    } else {
        syn_hkdf_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12, transcript_digest,
                              hash_len, ctx->client_app_secret, hash_len);
        syn_hkdf_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12, transcript_digest,
                              hash_len, ctx->server_app_secret, hash_len);
    }
    syn_secure_zero(transcript_digest, sizeof(transcript_digest));

    size_t key_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256 ||
                      ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256 ||
                      ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256)
                         ? 16U
                         : 32U;
    derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "key", ctx->client_app_key, key_len);
    derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "iv", ctx->client_app_iv, 12);
    derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "key", ctx->server_app_key, key_len);
    derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "iv", ctx->server_app_iv, 12);
}

/**
 * @brief Compute RFC 8446 §4.4.4 Finished verify_data MAC over running handshake transcript.
 * @param ctx       TLS context pointer.
 * @param hs_secret Base handshake traffic secret (server or client).
 * @param out_mac   [out] Verify data output buffer (32 or 48 bytes).
 */
static void tls_compute_finished(const SYN_TLS_Context *ctx, const uint8_t *hs_secret,
                                 uint8_t out_mac[SYN_TLS_SECRET_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t fin_key[SYN_TLS_SECRET_LEN];
    uint8_t tr_digest[SYN_TLS_SECRET_LEN];

    tls_compute_transcript_digest(ctx, is_sha384, tr_digest);
    if (is_sha384) {
        syn_hkdf_sha384_expand_label(hs_secret, hash_len, "finished", 8U, NULL, 0U, fin_key,
                                     hash_len);
        syn_hkdf_sha384_extract(fin_key, hash_len, tr_digest, hash_len, out_mac);
    } else {
        syn_hkdf_expand_label(hs_secret, hash_len, "finished", 8U, NULL, 0U, fin_key, hash_len);
        syn_hkdf_extract(fin_key, hash_len, tr_digest, hash_len, out_mac);
    }

    syn_secure_zero(fin_key, sizeof(fin_key));
    syn_secure_zero(tr_digest, sizeof(tr_digest));
}

/**
 * @brief Get tag length in bytes for active TLS 1.3 cipher suite.
 * @param ctx TLS context.
 * @return size_t Authentication tag size (8 or 16 bytes).
 */
static inline size_t tls_tag_len(const SYN_TLS_Context *ctx)
{
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        return 8U;
    }
    return 16U;
}

/**
 * @brief Construct non-blocking TLS 1.3 AEAD nonce (IV XOR sequence counter).
 * @param base_iv Base IV (12 bytes).
 * @param seq Sequence counter (64-bit).
 * @param nonce [out] Output 12-byte nonce.
 */
static void construct_tls13_nonce(const uint8_t base_iv[12], uint64_t seq, uint8_t nonce[12])
{
    memcpy(nonce, base_iv, 12);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((seq >> (i * 8)) & 0xFF);
    }
}

/**
 * @brief Encrypt a single TLS 1.3 inner record payload with 5-byte header AAD.
 * @param ctx     TLS context pointer.
 * @param key     AEAD traffic key.
 * @param nonce   12-byte record nonce.
 * @param aad     5-byte TLSCiphertext record header.
 * @param in      Plaintext input buffer.
 * @param in_len  Plaintext length in bytes.
 * @param out_ct  [out] Ciphertext output buffer.
 * @param out_tag [out] Authentication tag output buffer.
 */
static void tls_record_encrypt(SYN_TLS_Context *ctx, const uint8_t *key, const uint8_t nonce[12],
                               const uint8_t aad[TLS_RECORD_HEADER_LEN], const uint8_t *in,
                               size_t in_len, uint8_t *out_ct, uint8_t *out_tag)
{
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 16U);
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag, 16U);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag, 8U);
    } else {
        syn_aead_encrypt(key, nonce, aad, TLS_RECORD_HEADER_LEN, in, in_len, out_ct, out_tag);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
}

/**
 * @brief Decrypt and authenticate a TLS 1.3 record payload with 5-byte header AAD.
 * @param ctx    TLS context pointer.
 * @param key    AEAD traffic key.
 * @param nonce  12-byte record nonce.
 * @param aad    5-byte TLSCiphertext record header.
 * @param ct     Ciphertext input buffer.
 * @param ct_len Ciphertext length in bytes.
 * @param tag    Authentication tag buffer.
 * @param out_pt [out] Plaintext output buffer.
 * @return true if tag authentication succeeded.
 */
static bool tls_record_decrypt(SYN_TLS_Context *ctx, const uint8_t *key, const uint8_t nonce[12],
                               const uint8_t aad[TLS_RECORD_HEADER_LEN], const uint8_t *ct,
                               size_t ct_len, const uint8_t *tag, uint8_t *out_pt)
{
    bool ok = false;
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 16U);
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, out_pt, tag) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, out_pt, tag) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, tag, 16U, out_pt) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, tag, 8U, out_pt) == SYN_OK);
    } else {
        ok = syn_aead_decrypt(key, nonce, aad, TLS_RECORD_HEADER_LEN, ct, ct_len, tag, out_pt);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    return ok;
}

/**
 * @brief Validate local certificate, private key, and RPK length constraints.
 * @param ctx TLS context pointer.
 * @return true if local credentials satisfy configured authentication mode.
 */
static bool tls_verify_local_auth(const SYN_TLS_Context *ctx)
{
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_RAW_PUBKEY) {
        if (ctx->config.is_server) {
            return ctx->config.client_privkey != NULL;
        }
        return (ctx->config.peer_pubkey != NULL) && (ctx->config.peer_pubkey_len == 32U);
    }
    if ((ctx->config.mode == SYN_TLS_AUTH_MODE_X509_SERVER && ctx->config.is_server) ||
        ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) {
        return (ctx->config.client_cert_der != NULL) && (ctx->config.client_cert_len > 0U) &&
               (ctx->config.client_privkey != NULL);
    }
    return true;
}

/**
 * @brief Format RFC 8446 §4.4.3 CertificateVerify signing input over transcript digest.
 * @param is_server_cv true for server context string, false for client context string.
 * @param tr_digest    Transcript hash bytes.
 * @param hash_len     Transcript hash length (32 or 48).
 * @param out          [out] Output buffer (up to 146 bytes).
 * @return Total formatted byte length (98 + hash_len).
 */
static size_t tls_format_cv_input(bool is_server_cv, const uint8_t *tr_digest, size_t hash_len,
                                  uint8_t out[146])
{
    static const char srv_ctx[] = "TLS 1.3, server CertificateVerify";
    static const char cli_ctx[] = "TLS 1.3, client CertificateVerify";
    memset(out, 0x20, 64U);
    memcpy(out + 64U, is_server_cv ? srv_ctx : cli_ctx, 34U);
    memcpy(out + 98U, tr_digest, hash_len);
    return 98U + hash_len;
}

/**
 * @brief Build and sign a 72-byte RFC 8446 §4.4.3 CertificateVerify handshake message.
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param algo         Local signing key algorithm (Ed25519 or ECDSA-P256).
 * @param out_cv       [out] 72-byte CertificateVerify message buffer.
 * @return true if signature generation succeeded.
 */
static bool tls_sign_cert_verify(const SYN_TLS_Context *ctx, bool is_server_cv, SYN_X509_Algo algo,
                                 uint8_t out_cv[TLS_CERT_VERIFY_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t tr_digest[SYN_TLS_SECRET_LEN];
    uint8_t cv_msg[146];

    tls_compute_transcript_digest(ctx, is_sha384, tr_digest);
    size_t cv_len = tls_format_cv_input(is_server_cv, tr_digest, hash_len, cv_msg);

    out_cv[0] = TLS_HANDSHAKE_CERTIFICATE_VERIFY;
    out_cv[1] = 0x00U;
    out_cv[2] = 0x00U;
    out_cv[3] = 68U;
    out_cv[6] = 0x00U;
    out_cv[7] = 64U;

    bool ok = false;
    if (algo == SYN_X509_ALGO_ECDSA_P256) {
        uint8_t digest[32];
        out_cv[4] = 0x04U;
        out_cv[5] = 0x03U;
        syn_sha256(cv_msg, cv_len, digest);
        ok = syn_p256_sign_ecdsa_deterministic(ctx->config.client_privkey, digest, out_cv + 8U,
                                               out_cv + 40U);
        syn_secure_zero(digest, sizeof(digest));
    } else {
        out_cv[4] = 0x08U;
        out_cv[5] = 0x07U;
        ok = syn_ed25519_sign(cv_msg, cv_len, ctx->config.client_privkey, NULL, out_cv + 8U);
    }

    syn_secure_zero(tr_digest, sizeof(tr_digest));
    syn_secure_zero(cv_msg, sizeof(cv_msg));
    return ok;
}

/**
 * @brief Verify a 64-byte ECDSA-P256 signature over SHA-256(cv_msg).
 * @param pubkey     Peer P-256 public key (65-byte uncompressed 0x04||X||Y or 64-byte X||Y).
 * @param pubkey_len Public key byte length.
 * @param cv_msg     Formatted CertificateVerify signing content.
 * @param cv_len     Byte length of cv_msg.
 * @param sig64      64-byte raw (r || s) signature.
 * @return true if ECDSA signature is valid.
 */
static bool tls_verify_p256_cv(const uint8_t *pubkey, size_t pubkey_len, const uint8_t *cv_msg,
                               size_t cv_len, const uint8_t sig64[64])
{
    const uint8_t *px = pubkey;
    if (pubkey_len == 65U && pubkey[0] == 0x04U) {
        px = pubkey + 1U;
    } else if (pubkey_len != 64U) {
        return false;
    }
    uint8_t digest[32];
    syn_sha256(cv_msg, cv_len, digest);
    return syn_p256_verify_ecdsa(digest, sig64, sig64 + 32U, px, px + 32U);
}

/**
 * @brief Verify an inbound 72-byte RFC 8446 §4.4.3 CertificateVerify handshake message.
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param algo         Expected peer public key algorithm.
 * @param pubkey       Authenticated peer public key bytes.
 * @param pubkey_len   Public key byte length.
 * @param in_cv        72-byte wire CertificateVerify message.
 * @return true if header, SignatureScheme, and cryptographic signature are valid.
 */
static bool tls_verify_cert_verify(const SYN_TLS_Context *ctx, bool is_server_cv,
                                   SYN_X509_Algo algo, const uint8_t *pubkey, size_t pubkey_len,
                                   const uint8_t in_cv[TLS_CERT_VERIFY_LEN])
{
    static const uint8_t cv_hdr[4] = {TLS_HANDSHAKE_CERTIFICATE_VERIFY, 0x00U, 0x00U, 68U};
    if (memcmp(in_cv, cv_hdr, 4U) != 0 || in_cv[6] != 0x00U || in_cv[7] != 64U) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t tr_digest[SYN_TLS_SECRET_LEN];
    uint8_t cv_msg[146];

    tls_compute_transcript_digest(ctx, is_sha384, tr_digest);
    size_t cv_len = tls_format_cv_input(is_server_cv, tr_digest, hash_len, cv_msg);
    uint16_t scheme = ((uint16_t)in_cv[4] << 8U) | (uint16_t)in_cv[5];

    bool ok = false;
    if (algo == SYN_X509_ALGO_ED25519 && scheme == 0x0807U && pubkey_len == 32U) {
        ok = syn_ed25519_verify(in_cv + 8U, cv_msg, cv_len, pubkey);
    } else if (algo == SYN_X509_ALGO_ECDSA_P256 && scheme == 0x0403U) {
        ok = tls_verify_p256_cv(pubkey, pubkey_len, cv_msg, cv_len, in_cv + 8U);
    }

    syn_secure_zero(tr_digest, sizeof(tr_digest));
    syn_secure_zero(cv_msg, sizeof(cv_msg));
    return ok;
}

/**
 * @brief Bind local certificate or raw public key into transcript and sign CertificateVerify.
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer.
 * @return true if local credential parsed and signature succeeded.
 */
static bool tls_build_local_cv(SYN_TLS_Context *ctx, bool is_server_cv,
                               uint8_t out_cv[TLS_CERT_VERIFY_LEN])
{
    SYN_X509_Algo algo = SYN_X509_ALGO_ED25519;
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_RAW_PUBKEY) {
        uint8_t pub[32];
        (void)syn_ed25519_publickey(ctx->config.client_privkey, pub);
        tls_transcript_update(ctx, pub, 32U);
    } else {
        SYN_X509_Cert *own = &ctx->crypto_scratch.cert;
        if (!syn_x509_parse(ctx->config.client_cert_der, ctx->config.client_cert_len, own) ||
            (own->pubkey_algo != SYN_X509_ALGO_ED25519 &&
             own->pubkey_algo != SYN_X509_ALGO_ECDSA_P256)) {
            syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
            return false;
        }
        algo = own->pubkey_algo;
        syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
        tls_transcript_update(ctx, ctx->config.client_cert_der, ctx->config.client_cert_len);
    }
    if (!tls_sign_cert_verify(ctx, is_server_cv, algo, out_cv)) {
        return false;
    }
    tls_transcript_update(ctx, out_cv, TLS_CERT_VERIFY_LEN);
    return true;
}

/**
 * @brief Bind peer certificate or raw public key into transcript and verify CertificateVerify.
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param in_cv        72-byte inbound CertificateVerify message.
 * @return true if peer CertificateVerify signature matches authenticated peer key.
 */
static bool tls_check_peer_cv(SYN_TLS_Context *ctx, bool is_server_cv,
                              const uint8_t in_cv[TLS_CERT_VERIFY_LEN])
{
    bool ok = false;
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_RAW_PUBKEY) {
        tls_transcript_update(ctx, ctx->config.peer_pubkey, 32U);
        ok = tls_verify_cert_verify(ctx, is_server_cv, SYN_X509_ALGO_ED25519,
                                    ctx->config.peer_pubkey, 32U, in_cv);
    } else {
        SYN_X509_Cert *peer = &ctx->crypto_scratch.cert;
        (void)syn_x509_parse(ctx->config.peer_cert_der, ctx->config.peer_cert_len, peer);
        tls_transcript_update(ctx, ctx->config.peer_cert_der, ctx->config.peer_cert_len);
        ok = tls_verify_cert_verify(ctx, is_server_cv, peer->pubkey_algo, peer->pubkey,
                                    peer->pubkey_len, in_cv);
        syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    }
    if (ok) {
        tls_transcript_update(ctx, in_cv, TLS_CERT_VERIFY_LEN);
    }
    return ok;
}

/**
 * @brief Verify peer X.509 certificate chain and local credentials for configured auth mode.
 * @param ctx TLS context pointer.
 * @return true if credential validation passes or is not required.
 */
static bool tls_verify_peer_certificate(SYN_TLS_Context *ctx)
{
    if (!tls_verify_local_auth(ctx)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    bool need_peer_x509 =
        (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) ||
        (ctx->config.mode == SYN_TLS_AUTH_MODE_X509_SERVER && !ctx->config.is_server);
    if (!need_peer_x509) {
        return true;
    }

    if (ctx->config.root_ca == NULL || ctx->config.peer_cert_der == NULL ||
        ctx->config.peer_cert_len == 0U) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    SYN_X509_Cert *peer_cert = &ctx->crypto_scratch.cert;
    const char *expected_cn = ctx->config.is_server ? NULL : ctx->config.server_name;
    if (!syn_x509_parse(ctx->config.peer_cert_der, ctx->config.peer_cert_len, peer_cert) ||
        !syn_x509_validate_chain(peer_cert, ctx->config.root_ca, expected_cn)) {
        syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    return true;
}

/**
 * @brief Build and transmit an 88-byte TLS 1.3 ClientHello record over underlying_transport.
 * @param ctx TLS client context.
 * @return true if transmitted and state advanced to CLIENT_HELLO_SENT.
 */
static bool tls_send_client_hello(SYN_TLS_Context *ctx)
{
    uint8_t *tx = ctx->tx_buf;
    uint16_t suite_id = tls_cipher_wire_id(ctx->config.cipher_suite);

    tx[0] = TLS_CONTENT_TYPE_HANDSHAKE;
    tx[1] = 0x03U;
    tx[2] = 0x03U;
    tx[3] = 0x00U;
    tx[4] = 83U;

    tx[5] = TLS_HANDSHAKE_CLIENT_HELLO;
    tx[6] = 0x00U;
    tx[7] = 0x00U;
    tx[8] = 79U;
    tx[9] = 0x03U;
    tx[10] = 0x03U;

    if (syn_random_fill(tx + 11U, 32U) != SYN_OK) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    tx[43] = 0x00U;
    tx[44] = 0x00U;
    tx[45] = 0x02U;
    tx[46] = (uint8_t)(suite_id >> 8U);
    tx[47] = (uint8_t)(suite_id & 0xFFU);
    tx[48] = 0x01U;
    tx[49] = 0x00U;
    tx[50] = 0x00U;
    tx[51] = 36U;
    tx[52] = 0x00U;
    tx[53] = 0x33U;
    tx[54] = 0x00U;
    tx[55] = 32U;
    memcpy(tx + 56U, ctx->my_pubkey, 32U);

    tls_transcript_reset(ctx);
    tls_transcript_update(ctx, tx + 5U, 83U);

    if (!syn_transport_send(ctx->underlying_transport, tx, 88U)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }
    ctx->state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
    return true;
}

/**
 * @brief Parse and validate an inbound 88-byte ClientHello record on the server.
 * @param ctx    TLS server context.
 * @param rx_len Bytes read from underlying_transport.
 * @return true if valid ClientHello parsed into peer_pubkey and transcript.
 */
static bool tls_parse_client_hello(SYN_TLS_Context *ctx, size_t rx_len)
{
    static const uint8_t ch_hdr[11] = {TLS_CONTENT_TYPE_HANDSHAKE,
                                       0x03U,
                                       0x03U,
                                       0x00U,
                                       83U,
                                       TLS_HANDSHAKE_CLIENT_HELLO,
                                       0x00U,
                                       0x00U,
                                       79U,
                                       0x03U,
                                       0x03U};
    static const uint8_t mid_hdr[4] = {0x00U, 0x00U, 0x02U};
    static const uint8_t ks_hdr[8] = {0x01U, 0x00U, 0x00U, 36U, 0x00U, 0x33U, 0x00U, 32U};
    const uint8_t *rx = ctx->rx_buf;
    uint16_t exp_suite = tls_cipher_wire_id(ctx->config.cipher_suite);

    if (rx_len != 88U || memcmp(rx, ch_hdr, sizeof(ch_hdr)) != 0 ||
        memcmp(rx + 43U, mid_hdr, 3U) != 0 ||
        (((uint16_t)rx[46] << 8U) | (uint16_t)rx[47]) != exp_suite ||
        memcmp(rx + 48U, ks_hdr, sizeof(ks_hdr)) != 0) {
        return false;
    }

    memcpy(ctx->peer_pubkey, rx + 56U, 32U);
    tls_transcript_reset(ctx);
    tls_transcript_update(ctx, rx + 5U, 83U);
    return true;
}

/**
 * @brief Build and send ServerHello [+ CertificateVerify] + ServerFinished record.
 * @param ctx TLS server context.
 * @return true if derived, signed, and sent.
 */
static bool tls_send_server_flight(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    bool has_cv = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK);
    size_t cv_len = has_cv ? TLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 80U + cv_len + 4U + hash_len;
    uint16_t suite_id = tls_cipher_wire_id(ctx->config.cipher_suite);
    uint8_t *tx = ctx->tx_buf;

    tx[0] = TLS_CONTENT_TYPE_HANDSHAKE;
    tx[1] = 0x03U;
    tx[2] = 0x03U;
    tx[3] = 0x00U;
    tx[4] = (uint8_t)body_len;

    tx[5] = TLS_HANDSHAKE_SERVER_HELLO;
    tx[6] = 0x00U;
    tx[7] = 0x00U;
    tx[8] = 76U;
    tx[9] = 0x03U;
    tx[10] = 0x03U;

    if (syn_random_fill(tx + 11U, 32U) != SYN_OK) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    tx[43] = 0x00U;
    tx[44] = (uint8_t)(suite_id >> 8U);
    tx[45] = (uint8_t)(suite_id & 0xFFU);
    tx[46] = 0x00U;
    tx[47] = 0x00U;
    tx[48] = 36U;
    tx[49] = 0x00U;
    tx[50] = 0x33U;
    tx[51] = 0x00U;
    tx[52] = 32U;
    memcpy(tx + 53U, ctx->my_pubkey, 32U);

    tls_transcript_update(ctx, tx + 5U, 80U);
    if (!derive_tls13_hs_secrets(ctx) || (has_cv && !tls_build_local_cv(ctx, true, tx + 85U))) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    uint8_t *fin = tx + 85U + cv_len;
    fin[0] = TLS_HANDSHAKE_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    tls_compute_finished(ctx, ctx->server_handshake_secret, fin + 4U);
    tls_transcript_update(ctx, fin, 4U + hash_len);
    derive_tls13_app_keys(ctx);

    if (!syn_transport_send(ctx->underlying_transport, tx, 5U + body_len)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    ctx->state = (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) ? SYN_TLS_STATE_FINISHED_SENT
                                                              : SYN_TLS_STATE_ESTABLISHED;
    return true;
}

/**
 * @brief Validate ServerHello + Finished wire headers before cryptographic verification.
 * @param rx        Received server flight record.
 * @param rx_len    Received byte count.
 * @param body_len  Expected record payload length.
 * @param exp_suite Expected 16-bit cipher suite wire identifier.
 * @param fin_off   Offset of Finished handshake header in rx.
 * @param hash_len  Finished MAC length (32 or 48).
 * @return true if all framing headers match.
 */
static bool tls_check_server_flight_hdr(const uint8_t *rx, size_t rx_len, size_t body_len,
                                        uint16_t exp_suite, size_t fin_off, size_t hash_len)
{
    uint8_t sh_hdr[11] = {TLS_CONTENT_TYPE_HANDSHAKE,
                          0x03U,
                          0x03U,
                          0x00U,
                          (uint8_t)body_len,
                          TLS_HANDSHAKE_SERVER_HELLO,
                          0x00U,
                          0x00U,
                          76U,
                          0x03U,
                          0x03U};
    static const uint8_t ks_hdr[7] = {0x00U, 0x00U, 36U, 0x00U, 0x33U, 0x00U, 32U};
    uint8_t fin_hdr[4] = {TLS_HANDSHAKE_FINISHED, 0x00U, 0x00U, (uint8_t)hash_len};

    return (rx_len == 5U + body_len) && (memcmp(rx, sh_hdr, sizeof(sh_hdr)) == 0) &&
           (rx[43] == 0x00U) && ((((uint16_t)rx[44] << 8U) | (uint16_t)rx[45]) == exp_suite) &&
           (memcmp(rx + 46U, ks_hdr, sizeof(ks_hdr)) == 0) &&
           (memcmp(rx + fin_off, fin_hdr, sizeof(fin_hdr)) == 0);
}

/**
 * @brief Constant-time comparison of Finished verify_data MAC.
 * @param expected Expected verify_data bytes.
 * @param actual   Received verify_data bytes.
 * @param hash_len MAC length in bytes (32 or 48).
 * @return true if MACs match identically.
 */
static bool tls_verify_mac_ct(const uint8_t *expected, const uint8_t *actual, size_t hash_len)
{
    uint8_t diff = 0U;
    for (size_t i = 0U; i < hash_len; i++) {
        diff |= (uint8_t)(expected[i] ^ actual[i]);
    }
    return diff == 0U;
}

/**
 * @brief Build and send mTLS Flight 3 (Client CertificateVerify + Client Finished).
 * @param ctx TLS client context.
 * @return true if signed and transmitted.
 */
static bool tls_send_client_mtls_flight(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = TLS_CERT_VERIFY_LEN + 4U + hash_len;
    uint8_t *tx = ctx->tx_buf;

    tx[0] = TLS_CONTENT_TYPE_HANDSHAKE;
    tx[1] = 0x03U;
    tx[2] = 0x03U;
    tx[3] = 0x00U;
    tx[4] = (uint8_t)body_len;

    if (!tls_build_local_cv(ctx, false, tx + 5U)) {
        return false;
    }

    uint8_t *fin = tx + 5U + TLS_CERT_VERIFY_LEN;
    fin[0] = TLS_HANDSHAKE_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    tls_compute_finished(ctx, ctx->client_handshake_secret, fin + 4U);

    return syn_transport_send(ctx->underlying_transport, tx, 5U + body_len);
}

/**
 * @brief Parse and verify mTLS Flight 3 (Client CertificateVerify + Client Finished) on server.
 * @param ctx    TLS server context in SYN_TLS_STATE_FINISHED_SENT.
 * @param rx_len Bytes received from underlying_transport.
 * @return true if Client CertificateVerify and Client Finished are authentic.
 */
static bool tls_process_client_mtls_flight(SYN_TLS_Context *ctx, size_t rx_len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = TLS_CERT_VERIFY_LEN + 4U + hash_len;
    const uint8_t *rx = ctx->rx_buf;

    uint8_t rec_hdr[5] = {TLS_CONTENT_TYPE_HANDSHAKE, 0x03U, 0x03U, 0x00U, (uint8_t)body_len};
    uint8_t fin_hdr[4] = {TLS_HANDSHAKE_FINISHED, 0x00U, 0x00U, (uint8_t)hash_len};

    if (rx_len != 5U + body_len || memcmp(rx, rec_hdr, 5U) != 0 ||
        memcmp(rx + 5U + TLS_CERT_VERIFY_LEN, fin_hdr, 4U) != 0) {
        return false;
    }
    if (!tls_check_peer_cv(ctx, false, rx + 5U)) {
        return false;
    }

    uint8_t expected_mac[SYN_TLS_SECRET_LEN];
    tls_compute_finished(ctx, ctx->client_handshake_secret, expected_mac);
    bool ok = tls_verify_mac_ct(expected_mac, rx + 5U + TLS_CERT_VERIFY_LEN + 4U, hash_len);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    return ok;
}

/**
 * @brief Parse ServerHello [+ CertificateVerify] + ServerFinished on client and finish handshake.
 * @param ctx    TLS client context.
 * @param rx_len Bytes received from underlying_transport.
 * @return true if server flight verified (and mTLS client flight sent when configured).
 */
static bool tls_process_server_flight(SYN_TLS_Context *ctx, size_t rx_len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    bool has_cv = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK);
    size_t cv_len = has_cv ? TLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 80U + cv_len + 4U + hash_len;
    size_t fin_off = 85U + cv_len;
    uint16_t exp_suite = tls_cipher_wire_id(ctx->config.cipher_suite);
    const uint8_t *rx = ctx->rx_buf;

    if (!tls_check_server_flight_hdr(rx, rx_len, body_len, exp_suite, fin_off, hash_len)) {
        return false;
    }

    memcpy(ctx->peer_pubkey, rx + 53U, 32U);
    tls_transcript_update(ctx, rx + 5U, 80U);
    if (!derive_tls13_hs_secrets(ctx) || (has_cv && !tls_check_peer_cv(ctx, true, rx + 85U))) {
        return false;
    }

    uint8_t expected_mac[SYN_TLS_SECRET_LEN];
    tls_compute_finished(ctx, ctx->server_handshake_secret, expected_mac);
    bool ok = tls_verify_mac_ct(expected_mac, rx + fin_off + 4U, hash_len);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    if (!ok) {
        return false;
    }

    tls_transcript_update(ctx, rx + fin_off, 4U + hash_len);
    derive_tls13_app_keys(ctx);

    if (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS && !tls_send_client_mtls_flight(ctx)) {
        return false;
    }
    return true;
}

/**
 * @brief Drain buffered decrypted plaintext from the internal ring buffer.
 * @param ctx     TLS context pointer.
 * @param data    Destination caller buffer.
 * @param max_len Destination capacity.
 * @return Number of bytes copied (> 0 if drained, 0 if empty).
 */
static size_t tls_drain_app_rx(SYN_TLS_Context *ctx, uint8_t *data, size_t max_len)
{
    if (ctx->app_rx_head >= ctx->app_rx_tail) {
        return 0U;
    }
    size_t available = ctx->app_rx_tail - ctx->app_rx_head;
    if (available > max_len) {
        available = max_len;
    }
    memcpy(data, ctx->app_rx_buf + ctx->app_rx_head, available);
    ctx->app_rx_head += available;
    if (ctx->app_rx_head == ctx->app_rx_tail) {
        ctx->app_rx_head = 0U;
        ctx->app_rx_tail = 0U;
    }
    return available;
}

/**
 * @brief Validate a 5-byte TLSCiphertext header and compute inner plaintext+type length.
 * @param rx_buf             Wire record buffer.
 * @param rx_len             Total bytes received from transport.
 * @param tag_sz             AEAD authentication tag size in bytes.
 * @param pt_with_ct_len_out [out] Validated inner plaintext + ContentType length.
 * @return true if header type, version, and bounds are valid.
 */
static bool tls_parse_record_header(const uint8_t *rx_buf, size_t rx_len, size_t tag_sz,
                                    size_t *pt_with_ct_len_out)
{
    if (rx_len < TLS_RECORD_HEADER_LEN + tag_sz + 1U) {
        return false;
    }
    if (rx_buf[0] != TLS_CONTENT_TYPE_APPLICATION_DATA || rx_buf[1] != 0x03U ||
        rx_buf[2] != 0x03U) {
        return false;
    }

    size_t ciphertext_len = ((size_t)rx_buf[3] << 8U) | (size_t)rx_buf[4];
    if (ciphertext_len < tag_sz + 1U || ciphertext_len > rx_len - TLS_RECORD_HEADER_LEN) {
        return false;
    }

    size_t pt_with_ct_len = ciphertext_len - tag_sz;
    if (pt_with_ct_len > SYN_TLS_RECORD_MAX_PAYLOAD + 1U) {
        return false;
    }

    *pt_with_ct_len_out = pt_with_ct_len;
    return true;
}

/**
 * @brief Advance server-side TLS 1.3 handshake state machine by one non-blocking step.
 * @param ctx TLS server context.
 * @return true if progressing or completed, false on fatal error.
 */
static bool tls_server_handshake_step(SYN_TLS_Context *ctx)
{
    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return true;
    }

    if (ctx->state == SYN_TLS_STATE_FINISHED_SENT) {
        if (!tls_process_client_mtls_flight(ctx, rx_len)) {
            ctx->state = SYN_TLS_STATE_ERROR;
            return false;
        }
        ctx->state = SYN_TLS_STATE_ESTABLISHED;
        return true;
    }

    if (!tls_parse_client_hello(ctx, rx_len)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }
    return tls_send_server_flight(ctx);
}

/** @endcond */

void syn_tls_bind_transport(SYN_TLS_Context *tls_ctx, SYN_Transport *tr_out)
{
    if (tls_ctx == NULL || tr_out == NULL) {
        return;
    }

    tr_out->send = tls_transport_send_cb;
    tr_out->recv = tls_transport_recv_cb;
    tr_out->has_data =
        (tls_ctx->underlying_transport != NULL && tls_ctx->underlying_transport->has_data != NULL)
            ? tls_transport_has_data_cb
            : NULL;
    tr_out->ctx = tls_ctx;
}

bool syn_tls_init(SYN_TLS_Context *ctx, const SYN_TLS_Config *config, SYN_Transport *transport,
                  uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    if (ctx == NULL || config == NULL || transport == NULL || rx_buf == NULL || tx_buf == NULL) {
        return false;
    }
    if (rx_buf_size < (SYN_TLS_RECORD_MAX_PAYLOAD + TLS_RECORD_HEADER_LEN + 16U) ||
        tx_buf_size < (SYN_TLS_RECORD_MAX_PAYLOAD + TLS_RECORD_HEADER_LEN + 16U)) {
        return false;
    }

    memset(ctx, 0, sizeof(SYN_TLS_Context));
    if (syn_random_fill(ctx->my_privkey, 32U) != SYN_OK) {
        return false;
    }

    ctx->config = *config;
    ctx->underlying_transport = transport;
    ctx->rx_buf = rx_buf;
    ctx->rx_buf_size = rx_buf_size;
    ctx->tx_buf = tx_buf;
    ctx->tx_buf_size = tx_buf_size;
    ctx->state = SYN_TLS_STATE_UNINITIALIZED;

    syn_x25519_clamp(ctx->my_privkey);
    syn_x25519_pubkey(ctx->my_pubkey, ctx->my_privkey);

    tls_transcript_reset(ctx);
    return true;
}

bool syn_tls_handshake(SYN_TLS_Context *ctx)
{
    if (ctx == NULL || ctx->state == SYN_TLS_STATE_ERROR) {
        return false;
    }
    if (ctx->state == SYN_TLS_STATE_ESTABLISHED) {
        return true;
    }
    if (!tls_verify_peer_certificate(ctx)) {
        return false;
    }

    if (ctx->config.is_server) {
        return tls_server_handshake_step(ctx);
    }

    if (ctx->state == SYN_TLS_STATE_UNINITIALIZED && !tls_send_client_hello(ctx)) {
        return false;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return true;
    }
    if (!tls_process_server_flight(ctx, rx_len)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    ctx->state = SYN_TLS_STATE_ESTABLISHED;
    return true;
}

SYN_PT_Status syn_tls_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_TLS_Context *ctx = (SYN_TLS_Context *)task->user_data;

    PT_BEGIN(pt);

    while (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx)) {
            ctx->state = SYN_TLS_STATE_ERROR;
            PT_EXIT(pt);
        }
        if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
            PT_PASS(pt);
        }
    }

    PT_WAIT_UNTIL(pt, ctx->state != SYN_TLS_STATE_ESTABLISHED);

    PT_END(pt);
}

bool syn_tls_send(SYN_TLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx == NULL || ctx->state == SYN_TLS_STATE_ERROR || (data == NULL && len > 0U)) {
        return false;
    }

    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx) || ctx->state != SYN_TLS_STATE_ESTABLISHED) {
            return false;
        }
    }

    if (len == 0U) {
        return true;
    }

    if (len > SYN_TLS_RECORD_MAX_PAYLOAD) {
        return false;
    }

    uint8_t *record_buf = ctx->tx_buf;
    size_t tag_sz = tls_tag_len(ctx);
    size_t overhead = TLS_RECORD_HEADER_LEN + 1U + tag_sz;
    if (ctx->tx_buf_size < overhead || len > ctx->tx_buf_size - overhead) {
        return false;
    }

    size_t pt_with_ct_len = len + 1U;
    size_t ciphertext_len = pt_with_ct_len + tag_sz;

    record_buf[0] = TLS_CONTENT_TYPE_APPLICATION_DATA;
    record_buf[1] = 0x03U;
    record_buf[2] = 0x03U;
    record_buf[3] = (uint8_t)((ciphertext_len >> 8U) & 0xFFU);
    record_buf[4] = (uint8_t)(ciphertext_len & 0xFFU);

    uint8_t *out_ct = record_buf + TLS_RECORD_HEADER_LEN;
    memmove(out_ct, data, len);
    out_ct[len] = TLS_CONTENT_TYPE_APPLICATION_DATA;

    const uint8_t *tx_key = ctx->config.is_server ? ctx->server_app_key : ctx->client_app_key;
    const uint8_t *tx_iv = ctx->config.is_server ? ctx->server_app_iv : ctx->client_app_iv;

    uint8_t nonce[12];
    construct_tls13_nonce(tx_iv, ctx->client_seq_num++, nonce);

    tls_record_encrypt(ctx, tx_key, nonce, record_buf, out_ct, pt_with_ct_len, out_ct,
                       out_ct + pt_with_ct_len);

    syn_secure_zero(nonce, sizeof(nonce));

    size_t total_record_len = TLS_RECORD_HEADER_LEN + ciphertext_len;
    return syn_transport_send(ctx->underlying_transport, record_buf, total_record_len);
}

/**
 * @brief Commit decrypted record bytes to caller buffer and spill remainder to app_rx_buf.
 * @param ctx              TLS context.
 * @param decrypted_buf    Decrypted payload bytes.
 * @param full_payload_len Total decrypted payload length.
 * @param data             Caller output buffer.
 * @param max_len          Caller output buffer capacity.
 * @param out_len          [out] Bytes written to data.
 */
static void tls_commit_decrypted_record(SYN_TLS_Context *ctx, const uint8_t *decrypted_buf,
                                        size_t full_payload_len, uint8_t *data, size_t max_len,
                                        size_t *out_len)
{
    size_t copy_len = (full_payload_len > max_len) ? max_len : full_payload_len;
    ctx->server_seq_num++;
    *out_len = copy_len;
    if (copy_len > 0U) {
        memcpy(data, decrypted_buf, copy_len);
    }

    if (full_payload_len > max_len) {
        size_t overflow = full_payload_len - max_len;
        memcpy(ctx->app_rx_buf, decrypted_buf + max_len, overflow);
        ctx->app_rx_head = 0U;
        ctx->app_rx_tail = overflow;
    }
}

/**
 * @brief Strip optional trailing 0x00 padding and verify inner ContentType is 0x17.
 * @param pt_buf         Decrypted inner plaintext buffer.
 * @param pt_with_ct_len Total decrypted inner bytes.
 * @param app_len_out    [out] Application payload length excluding inner ContentType.
 * @return true if trailing non-zero ContentType byte is 0x17.
 */
static bool tls_strip_inner_content_type(const uint8_t *pt_buf, size_t pt_with_ct_len,
                                         size_t *app_len_out)
{
    size_t scan = pt_with_ct_len;
    while (scan > 0U && pt_buf[scan - 1U] == 0x00U) {
        scan--;
    }
    if (scan == 0U || pt_buf[scan - 1U] != TLS_CONTENT_TYPE_APPLICATION_DATA) {
        return false;
    }
    *app_len_out = scan - 1U;
    return true;
}

bool syn_tls_recv(SYN_TLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len)
{
    if (ctx == NULL || data == NULL || out_len == NULL) {
        return false;
    }

    *out_len = 0U;
    if (ctx->state == SYN_TLS_STATE_ERROR) {
        return false;
    }

    size_t drained = tls_drain_app_rx(ctx, data, max_len);
    if (drained > 0U) {
        *out_len = drained;
        return true;
    }

    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx) || ctx->state != SYN_TLS_STATE_ESTABLISHED) {
            return false;
        }
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    size_t tag_sz = tls_tag_len(ctx);
    size_t pt_with_ct_len = 0U;
    if (!tls_parse_record_header(ctx->rx_buf, rx_len, tag_sz, &pt_with_ct_len)) {
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    const uint8_t *rx_key = ctx->config.is_server ? ctx->client_app_key : ctx->server_app_key;
    const uint8_t *rx_iv = ctx->config.is_server ? ctx->client_app_iv : ctx->server_app_iv;

    uint8_t nonce[12];
    construct_tls13_nonce(rx_iv, ctx->server_seq_num, nonce);

    uint8_t *decrypted_buf = ctx->rx_buf + TLS_RECORD_HEADER_LEN;
    bool ok = tls_record_decrypt(ctx, rx_key, nonce, ctx->rx_buf, decrypted_buf, pt_with_ct_len,
                                 decrypted_buf + pt_with_ct_len, decrypted_buf);
    syn_secure_zero(nonce, sizeof(nonce));

    size_t app_len = 0U;
    if (!ok || !tls_strip_inner_content_type(decrypted_buf, pt_with_ct_len, &app_len)) {
        syn_secure_zero(decrypted_buf, pt_with_ct_len);
        ctx->state = SYN_TLS_STATE_ERROR;
        return false;
    }

    tls_commit_decrypted_record(ctx, decrypted_buf, app_len, data, max_len, out_len);
    return true;
}

#endif /* SYN_TLS_ENABLED */
