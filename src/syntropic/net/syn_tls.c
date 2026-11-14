/**
 * @file syn_tls.c
 * @brief Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter implementation.
 */

#include "syntropic/net/syn_tls.h"

#if SYN_TLS_ENABLED

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hmac.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/sched/syn_sched.h"
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
    return syn_x25519_is_nonzero(b);
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
 * @brief Cipher-suite-aware HKDF-Extract (SHA-256 or SHA-384).
 * @param ctx      TLS context pointer.
 * @param salt     Salt bytes.
 * @param salt_len Salt byte length.
 * @param ikm      Input keying material.
 * @param ikm_len  IKM byte length.
 * @param prk      [out] Pseudorandom key output buffer.
 */
static void tls_hkdf_extract_cs(const SYN_TLS_Context *ctx, const uint8_t *salt, size_t salt_len,
                                const uint8_t *ikm, size_t ikm_len, uint8_t *prk)
{
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_extract(salt, salt_len, ikm, ikm_len, prk);
    } else {
        syn_hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    }
}

/**
 * @brief Cipher-suite-aware HKDF-Expand-Label (SHA-256 or SHA-384).
 * @param ctx         TLS context pointer.
 * @param secret      Base secret (32 or 48 bytes).
 * @param label       TLS 1.3 label string without "tls13 " prefix.
 * @param label_len   Label byte length.
 * @param context     Transcript hash context bytes.
 * @param context_len Context byte length.
 * @param out         [out] Derived key material output buffer.
 * @param out_len     Desired output byte length.
 */
static void tls_hkdf_expand_cs(const SYN_TLS_Context *ctx, const uint8_t *secret, const char *label,
                               size_t label_len, const uint8_t *context, size_t context_len,
                               uint8_t *out, size_t out_len)
{
    if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_expand_label(secret, SYN_SHA384_DIGEST_SIZE, label, label_len, context,
                                     context_len, out, out_len);
    } else {
        syn_hkdf_expand_label(secret, SYN_SHA256_DIGEST_SIZE, label, label_len, context,
                              context_len, out, out_len);
    }
}

/**
 * @brief Validate ECDHE shared secret and snapshot ClientHello..ServerHello transcript digest.
 * @param ctx TLS context pointer.
 * @return true if ECDHE shared secret is non-zero.
 */
static bool tls_hs_secrets_prep(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    if (!tls_is_nonzero_32(ctx->hs_work.ecdhe_secret)) {
        syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
        return false;
    }
    tls_compute_transcript_digest(ctx, is_sha384, ctx->hs_work.ks_tr);
    return true;
}

/**
 * @brief Extract TLS 1.3 Early Secret into hs_work.ks_prk.
 * @param ctx TLS context pointer.
 */
static void tls_hs_extract_early(SYN_TLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_TLS_SECRET_LEN] = {0};
    size_t hash_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    uint8_t ikm[SYN_TLS_SECRET_LEN];
    memset(ikm, 0, sizeof(ikm));
    size_t ikm_len = hash_len;
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_PSK) {
        ikm_len = (ctx->config.psk_secret_len > SYN_TLS_SECRET_LEN) ? SYN_TLS_SECRET_LEN
                                                                    : ctx->config.psk_secret_len;
        memcpy(ikm, ctx->config.psk_secret, ikm_len);
    }
    tls_hkdf_extract_cs(ctx, zero_salt, hash_len, ikm, ikm_len, ctx->hs_work.ks_prk);
    syn_secure_zero(ikm, sizeof(ikm));
}

/**
 * @brief Expand "derived" salt from hs_work.ks_prk over empty transcript hash into hs_work.ks_salt.
 * @param ctx TLS context pointer.
 */
static void tls_hs_expand_derived(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    const uint8_t *empty_h = is_sha384 ? TLS_EMPTY_SHA384 : TLS_EMPTY_SHA256;
    tls_hkdf_expand_cs(ctx, ctx->hs_work.ks_prk, "derived", 7U, empty_h, hash_len,
                       ctx->hs_work.ks_salt, hash_len);
}

/**
 * @brief Extract Handshake Secret from ks_salt and ecdhe_secret into hs_work.ks_prk.
 * @param ctx TLS context pointer.
 */
static void tls_hs_extract_hs(SYN_TLS_Context *ctx)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    tls_hkdf_extract_cs(ctx, ctx->hs_work.ks_salt, hash_len, ctx->hs_work.ecdhe_secret, 32U,
                        ctx->hs_work.ks_prk);
    syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
}

/**
 * @brief Expand a handshake traffic secret ("c hs traffic" or "s hs traffic") from ks_prk + ks_tr.
 * @param ctx   TLS context pointer.
 * @param label 12-byte label ("c hs traffic" or "s hs traffic").
 * @param out   [out] Destination secret buffer.
 */
static void tls_hs_expand_traffic(SYN_TLS_Context *ctx, const char *label, uint8_t *out)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    tls_hkdf_expand_cs(ctx, ctx->hs_work.ks_prk, label, 12U, ctx->hs_work.ks_tr, hash_len, out,
                       hash_len);
}

/**
 * @brief Extract master_secret from ks_salt and scrub HKDF staging buffers.
 * @param ctx TLS context pointer.
 */
static void tls_hs_extract_master(SYN_TLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_TLS_SECRET_LEN] = {0};
    size_t hash_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    tls_hkdf_extract_cs(ctx, ctx->hs_work.ks_salt, hash_len, zero_salt, hash_len,
                        ctx->master_secret);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
    syn_secure_zero(ctx->hs_work.ks_tr, sizeof(ctx->hs_work.ks_tr));
}

/**
 * @brief Cooperative child protothread deriving TLS 1.3 handshake secrets (1 HKDF call per slice).
 * @param pt  Sub-protothread control block (&ctx->sub_pt).
 * @param ctx TLS context pointer (sets hs_work.ok true on success).
 * @return PT status.
 */
static SYN_PT_Status tls_derive_hs_secrets_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    ctx->hs_work.ok = false;
    if (!tls_hs_secrets_prep(ctx)) {
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    tls_hs_extract_early(ctx);
    PT_CONTINUE(pt);

    tls_hs_expand_derived(ctx);
    PT_CONTINUE(pt);

    tls_hs_extract_hs(ctx);
    PT_CONTINUE(pt);

    tls_hs_expand_traffic(ctx, "c hs traffic", ctx->client_handshake_secret);
    PT_CONTINUE(pt);

    tls_hs_expand_traffic(ctx, "s hs traffic", ctx->server_handshake_secret);
    PT_CONTINUE(pt);

    tls_hs_expand_derived(ctx);
    PT_CONTINUE(pt);

    tls_hs_extract_master(ctx);
    ctx->hs_work.ok = true;

    PT_END(pt);
}

/**
 * @brief Wipe ephemeral X25519 private key and intermediate handshake secrets once established.
 * @param ctx TLS context pointer.
 */
static void tls_scrub_handshake_secrets(SYN_TLS_Context *ctx)
{
    syn_secure_zero(ctx->my_privkey, sizeof(ctx->my_privkey));
    syn_secure_zero(ctx->client_handshake_secret, sizeof(ctx->client_handshake_secret));
    syn_secure_zero(ctx->server_handshake_secret, sizeof(ctx->server_handshake_secret));
    syn_secure_zero(ctx->master_secret, sizeof(ctx->master_secret));
    syn_secure_zero(&ctx->hs_work, sizeof(ctx->hs_work));
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
}

/**
 * @brief Return AEAD key byte length (16 or 32) for the configured TLS 1.3 cipher suite.
 * @param ctx TLS context pointer.
 * @return AEAD key size in bytes.
 */
static size_t tls_aead_key_len(const SYN_TLS_Context *ctx)
{
    return (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256 ||
            ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256 ||
            ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256)
               ? 16U
               : 32U;
}

/**
 * @brief Scrub handshake secrets and cooperatively initialize cached app AES-GCM contexts.
 * @param pt  Sub-protothread control block (&ctx->sub_pt).
 * @param ctx TLS context transitioning to SYN_TLS_STATE_ESTABLISHED.
 * @return PT status.
 */
static SYN_PT_Status tls_finalize_established_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    tls_scrub_handshake_secrets(ctx);
    if ((ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256) ||
        (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)) {
        const uint8_t *tx_key = ctx->config.is_server ? ctx->server_app_key : ctx->client_app_key;
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.app_gcm.tx, tx_key, tls_aead_key_len(ctx));
        PT_CONTINUE(pt);
        const uint8_t *rx_key = ctx->config.is_server ? ctx->client_app_key : ctx->server_app_key;
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.app_gcm.rx, rx_key, tls_aead_key_len(ctx));
    }
    ctx->state = SYN_TLS_STATE_ESTABLISHED;

    PT_END(pt);
}

/**
 * @brief Scrub transient handshake state and transition context to SYN_TLS_STATE_ERROR.
 * @param ctx TLS context pointer.
 * @return Always false.
 */
static bool tls_fail_handshake(SYN_TLS_Context *ctx)
{
    PT_INIT(&ctx->hs_pt);
    PT_INIT(&ctx->flight_pt);
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
    syn_secure_zero(ctx->hs_work.ks_tr, sizeof(ctx->hs_work.ks_tr));
    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    syn_secure_zero(ctx->client_app_secret, sizeof(ctx->client_app_secret));
    syn_secure_zero(ctx->server_app_secret, sizeof(ctx->server_app_secret));
    syn_secure_zero(ctx->client_app_key, sizeof(ctx->client_app_key));
    syn_secure_zero(ctx->client_app_iv, sizeof(ctx->client_app_iv));
    syn_secure_zero(ctx->server_app_key, sizeof(ctx->server_app_key));
    syn_secure_zero(ctx->server_app_iv, sizeof(ctx->server_app_iv));
    ctx->state = SYN_TLS_STATE_ERROR;
    return false;
}

/**
 * @brief Expand an application traffic secret ("c ap traffic" or "s ap traffic") from ks_tr.
 * @param ctx   TLS context pointer.
 * @param label 12-byte label ("c ap traffic" or "s ap traffic").
 * @param out   [out] Destination secret buffer.
 */
static void tls_app_expand_secret(SYN_TLS_Context *ctx, const char *label, uint8_t *out)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    tls_hkdf_expand_cs(ctx, ctx->master_secret, label, 12U, ctx->hs_work.ks_tr, hash_len, out,
                       hash_len);
}

/**
 * @brief Cooperative child protothread deriving TLS 1.3 application traffic keys (1 HKDF/slice).
 * @param pt  Sub-protothread control block (&ctx->sub_pt).
 * @param ctx TLS context pointer.
 * @return PT status.
 */
static SYN_PT_Status tls_derive_app_keys_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    tls_compute_transcript_digest(
        ctx, ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384,
        ctx->hs_work.ks_tr);
    PT_CONTINUE(pt);

    tls_app_expand_secret(ctx, "c ap traffic", ctx->client_app_secret);
    PT_CONTINUE(pt);

    tls_app_expand_secret(ctx, "s ap traffic", ctx->server_app_secret);
    syn_secure_zero(ctx->hs_work.ks_tr, sizeof(ctx->hs_work.ks_tr));
    PT_CONTINUE(pt);

    derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "key", ctx->client_app_key,
                             tls_aead_key_len(ctx));
    PT_CONTINUE(pt);

    derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "iv", ctx->client_app_iv, 12U);
    PT_CONTINUE(pt);

    derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "key", ctx->server_app_key,
                             tls_aead_key_len(ctx));
    PT_CONTINUE(pt);

    derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "iv", ctx->server_app_iv, 12U);
    PT_CONTINUE(pt);

    PT_END(pt);
}

/**
 * @brief Cooperative child protothread computing RFC 8446 §4.4.4 Finished MAC into hs_work.ks_salt.
 * @param pt        Sub-protothread control block (&ctx->sub_pt).
 * @param ctx       TLS context pointer (output MAC staged in ctx->hs_work.ks_salt).
 * @param hs_secret Base handshake traffic secret (server or client).
 * @return PT status.
 */
static SYN_PT_Status tls_compute_finished_pt(SYN_PT *pt, SYN_TLS_Context *ctx,
                                             const uint8_t *hs_secret)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    PT_BEGIN(pt);

    tls_compute_transcript_digest(ctx, is_sha384, ctx->hs_work.ks_tr);
    PT_CONTINUE(pt);

    tls_hkdf_expand_cs(ctx, hs_secret, "finished", 8U, NULL, 0U, ctx->hs_work.ks_prk, hash_len);
    PT_CONTINUE(pt);

    tls_hkdf_extract_cs(ctx, ctx->hs_work.ks_prk, hash_len, ctx->hs_work.ks_tr, hash_len,
                        ctx->hs_work.ks_salt);
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
    syn_secure_zero(ctx->hs_work.ks_tr, sizeof(ctx->hs_work.ks_tr));

    PT_END(pt);
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
    if ((ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256) ||
        (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)) {
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.app_gcm.tx, nonce, 12U, aad,
                                  TLS_RECORD_HEADER_LEN, in, in_len, out_ct, out_tag);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag, 16U);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  in, in_len, out_ct, out_tag, 8U);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else {
        syn_aead_encrypt(key, nonce, aad, TLS_RECORD_HEADER_LEN, in, in_len, out_ct, out_tag);
    }
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
    if ((ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256) ||
        (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384)) {
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.app_gcm.rx, nonce, 12U, aad,
                                  TLS_RECORD_HEADER_LEN, ct, ct_len, out_pt, tag) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, tag, 16U, out_pt) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else if (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, TLS_RECORD_HEADER_LEN,
                                  ct, ct_len, tag, 8U, out_pt) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else {
        ok = syn_aead_decrypt(key, nonce, aad, TLS_RECORD_HEADER_LEN, ct, ct_len, tag, out_pt);
    }
    return ok;
}

/**
 * @brief Validate local certificate, private key, and RPK length constraints.
 * @param ctx TLS context pointer.
 * @return true if local credentials satisfy configured authentication mode.
 */
static bool tls_verify_local_auth(const SYN_TLS_Context *ctx)
{
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_PSK) {
        return (ctx->config.psk_secret != NULL) && (ctx->config.psk_secret_len > 0U);
    }
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
 * @brief Format CertificateVerify header/payload and initialize cooperative P-256 or Ed25519 sign.
 * @param ctx          TLS context pointer with hs_work.cv_algo populated.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer (8B header written).
 * @return true if signing context initialized successfully.
 */
static bool tls_init_cv_sign(SYN_TLS_Context *ctx, bool is_server_cv,
                             uint8_t out_cv[TLS_CERT_VERIFY_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t tr_digest[SYN_TLS_SECRET_LEN];
    tls_compute_transcript_digest(ctx, is_sha384, tr_digest);
    size_t cv_len = tls_format_cv_input(is_server_cv, tr_digest, hash_len, ctx->hs_work.cv_msg);
    syn_secure_zero(tr_digest, sizeof(tr_digest));

    out_cv[0] = TLS_HANDSHAKE_CERTIFICATE_VERIFY;
    out_cv[1] = 0x00U;
    out_cv[2] = 0x00U;
    out_cv[3] = 68U;
    out_cv[6] = 0x00U;
    out_cv[7] = 64U;

    if (ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) {
        uint8_t digest[32];
        out_cv[4] = 0x04U;
        out_cv[5] = 0x03U;
        syn_sha256(ctx->hs_work.cv_msg, cv_len, digest);
        bool ok = syn_p256_sign_deterministic_init(&ctx->crypto_scratch.p256,
                                                   ctx->config.client_privkey, digest);
        syn_secure_zero(digest, sizeof(digest));
        return ok;
    }

    out_cv[4] = 0x08U;
    out_cv[5] = 0x07U;
    (void)syn_ed25519_sign_init(&ctx->crypto_scratch.ed25519, ctx->hs_work.cv_msg, cv_len,
                                ctx->config.client_privkey, ctx->hs_work.pub_cache);
    return true;
}

/**
 * @brief Parse local X.509 leaf certificate in an isolated stack frame without union aliasing.
 * @param ctx TLS context pointer.
 * @return true if certificate parsed with an Ed25519 or ECDSA-P256 subject public key.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static bool
tls_parse_local_cert(SYN_TLS_Context *ctx)
{
    SYN_X509_Cert own;
    if (!syn_x509_parse(ctx->config.client_cert_der, ctx->config.client_cert_len, &own) ||
        ((own.pubkey_algo != SYN_X509_ALGO_ED25519) &&
         (own.pubkey_algo != SYN_X509_ALGO_ECDSA_P256))) {
        syn_secure_zero(&own, sizeof(own));
        return false;
    }
    ctx->hs_work.cv_algo = own.pubkey_algo;
    if (own.pubkey_algo == SYN_X509_ALGO_ED25519) {
        memcpy(ctx->hs_work.pub_cache, own.pubkey, 32U);
    }
    syn_secure_zero(&own, sizeof(own));
    return true;
}

/**
 * @brief Cooperative wrapper for Ed25519 RPK public-key derivation.
 * @param ctx TLS context pointer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_ed25519_pubkey_pt(SYN_TLS_Context *ctx)
{
    return syn_ed25519_publickey_pt(&ctx->crypto_pt, &ctx->crypto_scratch.ed25519,
                                    ctx->hs_work.pub_cache);
}

/**
 * @brief Cooperative wrapper for P-256 or Ed25519 CertificateVerify signing.
 * @param ctx    TLS context pointer.
 * @param out_cv 72-byte CertificateVerify wire buffer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_cv_sign_crypto_pt(SYN_TLS_Context *ctx, uint8_t *out_cv)
{
    if (ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) {
        return syn_p256_sign_pt(&ctx->crypto_pt, &ctx->crypto_scratch.p256, out_cv + 8U,
                                out_cv + 40U, &ctx->hs_work.ok);
    }
    SYN_PT_Status st =
        syn_ed25519_sign_pt(&ctx->crypto_pt, &ctx->crypto_scratch.ed25519, out_cv + 8U);
    if (st == PT_ENDED) {
        ctx->hs_work.ok = true;
    }
    return st;
}

/**
 * @brief Child protothread binding local credential and signing CertificateVerify.
 * @param pt           Protothread control block (&ctx->sub_pt).
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @return PT status.
 */
static SYN_PT_Status tls_build_local_cv_pt(SYN_PT *pt, SYN_TLS_Context *ctx, bool is_server_cv)
{
    uint8_t *out_cv = ctx->tx_buf + (is_server_cv ? 85U : 5U);

    PT_BEGIN(pt);

    ctx->hs_work.ok = false;
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_RAW_PUBKEY) {
        ctx->hs_work.cv_algo = SYN_X509_ALGO_ED25519;
        (void)syn_ed25519_publickey_init(&ctx->crypto_scratch.ed25519, ctx->config.client_privkey);
        PT_CONTINUE(pt);
        PT_SPAWN(pt, &ctx->crypto_pt, tls_ed25519_pubkey_pt(ctx));
        PT_CONTINUE(pt);
        tls_transcript_update(ctx, ctx->hs_work.pub_cache, 32U);
        PT_CONTINUE(pt);
        (void)tls_init_cv_sign(ctx, is_server_cv, out_cv);
    } else {
        if (!tls_parse_local_cert(ctx)) {
            (void)tls_fail_handshake(ctx);
            PT_EXIT(pt);
        }
        tls_transcript_update(ctx, ctx->config.client_cert_der, ctx->config.client_cert_len);
        PT_CONTINUE(pt);
        if (!tls_init_cv_sign(ctx, is_server_cv, out_cv)) {
            (void)tls_fail_handshake(ctx);
            PT_EXIT(pt);
        }
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->crypto_pt, tls_cv_sign_crypto_pt(ctx, out_cv));
    PT_CONTINUE(pt);
    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    if (ctx->hs_work.ok) {
        tls_transcript_update(ctx, out_cv, TLS_CERT_VERIFY_LEN);
    }
    PT_CONTINUE(pt);

    PT_END(pt);
}

/**
 * @brief Initialize cooperative ECDSA-P256 verification over SHA-256(hs_work.cv_msg).
 * @param ctx    TLS context pointer with hs_work.pub_cache and hs_work.cv_msg populated.
 * @param cv_len Byte length of hs_work.cv_msg.
 * @param sig64  64-byte raw (r || s) signature.
 * @return true if public key and signature scalars passed initial checks.
 */
static bool tls_init_p256_cv_verify(SYN_TLS_Context *ctx, size_t cv_len, const uint8_t sig64[64])
{
    const uint8_t *pubkey = ctx->hs_work.pub_cache;
    size_t pubkey_len = ctx->hs_work.pub_cache_len;
    const uint8_t *px = pubkey;
    if ((pubkey_len == 65U) && (pubkey[0] == 0x04U)) {
        px = pubkey + 1U;
    } else if (pubkey_len != 64U) {
        return false;
    }
    uint8_t digest[32];
    syn_sha256(ctx->hs_work.cv_msg, cv_len, digest);
    bool ok =
        syn_p256_verify_init(&ctx->crypto_scratch.p256, digest, sig64, sig64 + 32U, px, px + 32U);
    syn_secure_zero(digest, sizeof(digest));
    return ok;
}

/**
 * @brief Bind peer raw public key or X.509 DER certificate into the handshake transcript.
 * @param ctx TLS context pointer.
 */
static void tls_bind_peer_cv_cred(SYN_TLS_Context *ctx)
{
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_RAW_PUBKEY) {
        ctx->hs_work.cv_algo = SYN_X509_ALGO_ED25519;
        memcpy(ctx->hs_work.pub_cache, ctx->config.peer_pubkey, 32U);
        ctx->hs_work.pub_cache_len = 32U;
        tls_transcript_update(ctx, ctx->config.peer_pubkey, 32U);
    } else {
        ctx->hs_work.cv_algo = ctx->hs_work.peer_algo;
        memcpy(ctx->hs_work.pub_cache, ctx->hs_work.peer_pub, ctx->hs_work.peer_pub_len);
        ctx->hs_work.pub_cache_len = ctx->hs_work.peer_pub_len;
        tls_transcript_update(ctx, ctx->config.peer_cert_der, ctx->config.peer_cert_len);
    }
}

/**
 * @brief Validate CertificateVerify framing and initialize signature verification context.
 * @param ctx          TLS context pointer with peer credential already bound into transcript.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param in_cv        72-byte wire CertificateVerify message.
 * @return true if header, SignatureScheme, and initial curve checks succeeded.
 */
static bool tls_init_peer_cv_verify(SYN_TLS_Context *ctx, bool is_server_cv,
                                    const uint8_t in_cv[TLS_CERT_VERIFY_LEN])
{
    static const uint8_t cv_hdr[4] = {TLS_HANDSHAKE_CERTIFICATE_VERIFY, 0x00U, 0x00U, 68U};
    if ((memcmp(in_cv, cv_hdr, 4U) != 0) || (in_cv[6] != 0x00U) || (in_cv[7] != 64U)) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t tr_digest[SYN_TLS_SECRET_LEN];
    tls_compute_transcript_digest(ctx, is_sha384, tr_digest);
    size_t cv_len = tls_format_cv_input(is_server_cv, tr_digest, hash_len, ctx->hs_work.cv_msg);
    syn_secure_zero(tr_digest, sizeof(tr_digest));

    uint16_t scheme = ((uint16_t)in_cv[4] << 8U) | (uint16_t)in_cv[5];
    if ((ctx->hs_work.cv_algo == SYN_X509_ALGO_ED25519) && (scheme == 0x0807U) &&
        (ctx->hs_work.pub_cache_len == 32U)) {
        return syn_ed25519_verify_init(&ctx->crypto_scratch.ed25519, in_cv + 8U,
                                       ctx->hs_work.cv_msg, cv_len, ctx->hs_work.pub_cache);
    }
    if ((ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) && (scheme == 0x0403U)) {
        return tls_init_p256_cv_verify(ctx, cv_len, in_cv + 8U);
    }
    return false;
}

/**
 * @brief Cooperative wrapper for P-256 or Ed25519 CertificateVerify verification.
 * @param ctx TLS context pointer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_cv_verify_crypto_pt(SYN_TLS_Context *ctx)
{
    if (ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) {
        return syn_p256_verify_pt(&ctx->crypto_pt, &ctx->crypto_scratch.p256, &ctx->hs_work.ok);
    }
    return syn_ed25519_verify_pt(&ctx->crypto_pt, &ctx->crypto_scratch.ed25519, &ctx->hs_work.ok);
}

/**
 * @brief Child protothread verifying inbound peer CertificateVerify message.
 * @param pt           Protothread control block (&ctx->sub_pt).
 * @param ctx          TLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @return PT status.
 */
static SYN_PT_Status tls_check_peer_cv_pt(SYN_PT *pt, SYN_TLS_Context *ctx, bool is_server_cv)
{
    const uint8_t *in_cv = ctx->rx_buf + (is_server_cv ? 85U : 5U);

    PT_BEGIN(pt);

    ctx->hs_work.ok = false;
    tls_bind_peer_cv_cred(ctx);
    PT_CONTINUE(pt);

    if (!tls_init_peer_cv_verify(ctx, is_server_cv, in_cv)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->crypto_pt, tls_cv_verify_crypto_pt(ctx));
    PT_CONTINUE(pt);
    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    if (!ctx->hs_work.ok) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    tls_transcript_update(ctx, in_cv, TLS_CERT_VERIFY_LEN);
    PT_CONTINUE(pt);

    PT_END(pt);
}

/**
 * @brief Parse peer X.509 certificate in an isolated stack frame and initialize chain validation.
 * @param ctx TLS context pointer.
 * @return true if certificate parsed and chain verification context initialized.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static bool
tls_init_peer_x509(SYN_TLS_Context *ctx)
{
    if ((ctx->config.root_ca == NULL) || (ctx->config.peer_cert_der == NULL) ||
        (ctx->config.peer_cert_len == 0U)) {
        return false;
    }

    SYN_X509_Cert peer_cert;
    const char *expected_cn = ctx->config.is_server ? NULL : ctx->config.server_name;
    if (!syn_x509_parse(ctx->config.peer_cert_der, ctx->config.peer_cert_len, &peer_cert) ||
        !syn_x509_validate_chain_init(&ctx->crypto_scratch.x509, &peer_cert, ctx->config.root_ca,
                                      expected_cn)) {
        syn_secure_zero(&peer_cert, sizeof(peer_cert));
        return false;
    }
    ctx->hs_work.peer_algo = peer_cert.pubkey_algo;
    ctx->hs_work.peer_pub_len = (uint8_t)peer_cert.pubkey_len;
    memcpy(ctx->hs_work.peer_pub, peer_cert.pubkey, peer_cert.pubkey_len);
    syn_secure_zero(&peer_cert, sizeof(peer_cert));
    return true;
}

/**
 * @brief Cooperative wrapper for X.509 certificate chain signature verification.
 * @param ctx TLS context pointer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_x509_verify_pt(SYN_TLS_Context *ctx)
{
    return syn_x509_verify_pt(&ctx->crypto_pt, &ctx->crypto_scratch.x509, &ctx->hs_work.ok);
}

/**
 * @brief Child protothread validating local credentials and peer X.509 certificate chain.
 * @param pt  Protothread control block (&ctx->sub_pt).
 * @param ctx TLS context pointer.
 * @return PT status.
 */
static SYN_PT_Status tls_peer_cert_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    if (!ctx->hs_work.peer_auth_verified) {
        if (!tls_verify_local_auth(ctx)) {
            (void)tls_fail_handshake(ctx);
            PT_EXIT(pt);
        }

        bool need_peer_x509 =
            (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) ||
            ((ctx->config.mode == SYN_TLS_AUTH_MODE_X509_SERVER) && !ctx->config.is_server);
        if (need_peer_x509) {
            if (!tls_init_peer_x509(ctx)) {
                (void)tls_fail_handshake(ctx);
                PT_EXIT(pt);
            }
            ctx->hs_work.ok = false;
            PT_CONTINUE(pt);
            PT_SPAWN(pt, &ctx->crypto_pt, tls_x509_verify_pt(ctx));
            if (!ctx->hs_work.ok) {
                (void)tls_fail_handshake(ctx);
                PT_EXIT(pt);
            }
            ctx->hs_work.peer_auth_verified = true;
            PT_CONTINUE(pt);
        } else {
            ctx->hs_work.peer_auth_verified = true;
        }
    }

    PT_END(pt);
}

/**
 * @brief Cooperative wrapper for local X25519 ephemeral public-key derivation.
 * @param ctx TLS context pointer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_x25519_keygen_pt(SYN_TLS_Context *ctx)
{
    return syn_x25519_pt(&ctx->crypto_pt, &ctx->crypto_scratch.x25519, ctx->my_pubkey);
}

/**
 * @brief Cooperative wrapper for X25519 ECDHE shared-secret computation.
 * @param ctx TLS context pointer.
 * @return Child protothread status.
 */
static SYN_PT_Status tls_x25519_ecdhe_pt(SYN_TLS_Context *ctx)
{
    return syn_x25519_pt(&ctx->crypto_pt, &ctx->crypto_scratch.x25519, ctx->hs_work.ecdhe_secret);
}

/**
 * @brief Child protothread deriving local X25519 ephemeral public key cooperatively.
 * @param pt  Protothread control block (&ctx->sub_pt).
 * @param ctx TLS context pointer.
 * @return PT status.
 */
static SYN_PT_Status tls_keygen_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    if (!ctx->hs_work.my_pubkey_ready) {
        (void)syn_x25519_pubkey_init(&ctx->crypto_scratch.x25519, ctx->my_privkey);
        PT_SPAWN(pt, &ctx->crypto_pt, tls_x25519_keygen_pt(ctx));
        ctx->hs_work.my_pubkey_ready = true;
        PT_CONTINUE(pt);
    }

    PT_END(pt);
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
        return tls_fail_handshake(ctx);
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
        return tls_fail_handshake(ctx);
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
 * @brief Format ServerHello record header and body into tx_buf and update transcript.
 * @param ctx TLS server context.
 * @return true if ServerHello random filled and formatted into tx_buf.
 */
static bool tls_format_server_hello(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) ? TLS_CERT_VERIFY_LEN : 0U;
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
    return true;
}

/**
 * @brief Poll underlying transport for an inbound record into ctx->rx_buf.
 * @param ctx    TLS context pointer.
 * @param rx_len [out] Bytes received.
 * @return true if a record was read into ctx->rx_buf.
 */
static inline bool tls_poll_rx(const SYN_TLS_Context *ctx, size_t *rx_len)
{
    return syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, rx_len);
}

/**
 * @brief Append staged ServerFinished MAC (`hs_work.ks_salt`) to tx_buf and transmit Flight 2.
 * @param ctx TLS server context with ServerHello (and optional CV) formatted in tx_buf.
 * @return true if Server Flight 2 record transmitted.
 */
static bool tls_send_server_flight2(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) ? TLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 80U + cv_len + 4U + hash_len;
    uint8_t *fin = ctx->tx_buf + 85U + cv_len;

    fin[0] = TLS_HANDSHAKE_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    memcpy(fin + 4U, ctx->hs_work.ks_salt, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    tls_transcript_update(ctx, fin, 4U + hash_len);
    return syn_transport_send(ctx->underlying_transport, ctx->tx_buf, 5U + body_len);
}

/**
 * @brief Server Flight 1 PT: verify peer cert, derive keygen, parse ClientHello, run ECDHE.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS server context in SYN_TLS_STATE_UNINITIALIZED.
 * @return PT status.
 */
static SYN_PT_Status tls_server_flight1_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    size_t rx_len = 0U;

    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_peer_cert_pt(&ctx->sub_pt, ctx));
    if (ctx->state == SYN_TLS_STATE_ERROR) {
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt, tls_keygen_pt(&ctx->sub_pt, ctx));

    while (!tls_poll_rx(ctx, &rx_len)) {
        PT_PASS(pt);
    }
    if (!tls_parse_client_hello(ctx, rx_len) || !tls_format_server_hello(ctx)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    (void)syn_x25519_init(&ctx->crypto_scratch.x25519, ctx->my_privkey, ctx->peer_pubkey);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->crypto_pt, tls_x25519_ecdhe_pt(ctx));
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_derive_hs_secrets_pt(&ctx->sub_pt, ctx));
    if (!ctx->hs_work.ok) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    ctx->state = SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED;

    PT_END(pt);
}

/**
 * @brief Server Flight 2 PT: sign CertificateVerify, transmit flight, and derive app keys.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS server context in SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED.
 * @return PT status.
 */
static SYN_PT_Status tls_server_flight2_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    if (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) {
        PT_SPAWN(pt, &ctx->sub_pt, tls_build_local_cv_pt(&ctx->sub_pt, ctx, true));
        if (!ctx->hs_work.ok) {
            (void)tls_fail_handshake(ctx);
            PT_EXIT(pt);
        }
    }

    PT_SPAWN(pt, &ctx->sub_pt,
             tls_compute_finished_pt(&ctx->sub_pt, ctx, ctx->server_handshake_secret));
    PT_CONTINUE(pt);

    if (!tls_send_server_flight2(ctx)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_derive_app_keys_pt(&ctx->sub_pt, ctx));
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) {
        ctx->state = SYN_TLS_STATE_FINISHED_SENT;
    } else {
        PT_SPAWN(pt, &ctx->sub_pt, tls_finalize_established_pt(&ctx->sub_pt, ctx));
    }

    PT_END(pt);
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
 * @brief Validate framing headers on inbound mTLS Client Flight 3 record.
 * @param ctx    TLS server context.
 * @param rx_len Received record byte count.
 * @return true if record header and Finished handshake header match expected lengths.
 */
static bool tls_check_client_mtls_hdr(const SYN_TLS_Context *ctx, size_t rx_len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = TLS_CERT_VERIFY_LEN + 4U + hash_len;
    const uint8_t *rx = ctx->rx_buf;
    uint8_t rec_hdr[5] = {TLS_CONTENT_TYPE_HANDSHAKE, 0x03U, 0x03U, 0x00U, (uint8_t)body_len};
    uint8_t fin_hdr[4] = {TLS_HANDSHAKE_FINISHED, 0x00U, 0x00U, (uint8_t)hash_len};

    return (rx_len == (5U + body_len)) && (memcmp(rx, rec_hdr, 5U) == 0) &&
           (memcmp(rx + 5U + TLS_CERT_VERIFY_LEN, fin_hdr, 4U) == 0);
}

/**
 * @brief Verify staged mTLS ClientFinished MAC (`hs_work.ks_salt`) in constant time.
 * @param ctx TLS server context with client CV bound into transcript.
 * @return true if ClientFinished MAC matches expected verify_data.
 */
static bool tls_verify_client_finished(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    bool ok = tls_verify_mac_ct(ctx->hs_work.ks_salt, ctx->rx_buf + 5U + TLS_CERT_VERIFY_LEN + 4U,
                                hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    return ok;
}

/**
 * @brief Server Flight 3 (mTLS) PT: receive Flight 3, verify Client CV and ClientFinished.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS server context in SYN_TLS_STATE_FINISHED_SENT.
 * @return PT status.
 */
static SYN_PT_Status tls_server_mtls_flight3_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    size_t rx_len = 0U;

    PT_BEGIN(pt);

    while (!tls_poll_rx(ctx, &rx_len)) {
        PT_PASS(pt);
    }
    if (!tls_check_client_mtls_hdr(ctx, rx_len)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt, tls_check_peer_cv_pt(&ctx->sub_pt, ctx, false));
    if (ctx->state == SYN_TLS_STATE_ERROR) {
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt,
             tls_compute_finished_pt(&ctx->sub_pt, ctx, ctx->client_handshake_secret));
    PT_CONTINUE(pt);
    if (!tls_verify_client_finished(ctx)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt, tls_finalize_established_pt(&ctx->sub_pt, ctx));

    PT_END(pt);
}

/**
 * @brief Client Flight 1 PT: verify peer cert, derive local X25519 pubkey, send ClientHello.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS client context in SYN_TLS_STATE_UNINITIALIZED.
 * @return PT status.
 */
static SYN_PT_Status tls_client_flight1_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_peer_cert_pt(&ctx->sub_pt, ctx));
    if (ctx->state == SYN_TLS_STATE_ERROR) {
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt, tls_keygen_pt(&ctx->sub_pt, ctx));

    if (!tls_send_client_hello(ctx)) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

/**
 * @brief Validate inbound ServerHello flight headers and record peer ephemeral X25519 key.
 * @param ctx    TLS client context.
 * @param rx_len Bytes received into ctx->rx_buf.
 * @return true if ServerHello framing is valid and transcript updated.
 */
static bool tls_accept_server_hello(SYN_TLS_Context *ctx, size_t rx_len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) ? TLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 80U + cv_len + 4U + hash_len;
    size_t fin_off = 85U + cv_len;
    uint16_t exp_suite = tls_cipher_wire_id(ctx->config.cipher_suite);
    const uint8_t *rx = ctx->rx_buf;

    if (!tls_check_server_flight_hdr(rx, rx_len, body_len, exp_suite, fin_off, hash_len)) {
        return false;
    }
    ctx->rx_record_len = rx_len;
    memcpy(ctx->peer_pubkey, rx + 53U, 32U);
    tls_transcript_update(ctx, rx + 5U, 80U);
    return true;
}

/**
 * @brief Client ServerHello PT: receive ServerHello flight, run X25519 ECDHE, derive HS keys.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS client context in SYN_TLS_STATE_CLIENT_HELLO_SENT.
 * @return PT status.
 */
static SYN_PT_Status tls_client_sh_ecdhe_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    size_t rx_len = 0U;

    PT_BEGIN(pt);

    while (!tls_poll_rx(ctx, &rx_len)) {
        PT_PASS(pt);
    }
    if (!tls_accept_server_hello(ctx, rx_len)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    (void)syn_x25519_init(&ctx->crypto_scratch.x25519, ctx->my_privkey, ctx->peer_pubkey);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->crypto_pt, tls_x25519_ecdhe_pt(ctx));
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_derive_hs_secrets_pt(&ctx->sub_pt, ctx));
    if (!ctx->hs_work.ok) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    ctx->state = SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED;

    PT_END(pt);
}

/**
 * @brief Client CertificateVerify PT: verify server CertificateVerify signature when non-PSK.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS client context in SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED.
 * @return PT status.
 */
static SYN_PT_Status tls_client_verify_cv_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    if (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) {
        PT_SPAWN(pt, &ctx->sub_pt, tls_check_peer_cv_pt(&ctx->sub_pt, ctx, true));
        if (ctx->state == SYN_TLS_STATE_ERROR) {
            PT_EXIT(pt);
        }
    }
    ctx->state = SYN_TLS_STATE_CERTIFICATE_VERIFIED;

    PT_END(pt);
}

/**
 * @brief Verify staged ServerFinished MAC (`hs_work.ks_salt`) and append Finished to transcript.
 * @param ctx TLS client context in SYN_TLS_STATE_CERTIFICATE_VERIFIED.
 * @return true if ServerFinished MAC is authentic.
 */
static bool tls_verify_server_finished(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_TLS_AUTH_MODE_PSK) ? TLS_CERT_VERIFY_LEN : 0U;
    size_t fin_off = 85U + cv_len;
    const uint8_t *rx = ctx->rx_buf;

    bool ok = tls_verify_mac_ct(ctx->hs_work.ks_salt, rx + fin_off + 4U, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    if (!ok) {
        return false;
    }
    tls_transcript_update(ctx, rx + fin_off, 4U + hash_len);
    return true;
}

/**
 * @brief Client Finished PT: verify ServerFinished MAC and derive application traffic keys.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS client context in SYN_TLS_STATE_CERTIFICATE_VERIFIED.
 * @return PT status.
 */
static SYN_PT_Status tls_client_verify_fin_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt,
             tls_compute_finished_pt(&ctx->sub_pt, ctx, ctx->server_handshake_secret));
    PT_CONTINUE(pt);

    if (!tls_verify_server_finished(ctx)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_derive_app_keys_pt(&ctx->sub_pt, ctx));
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_MTLS) {
        ctx->state = SYN_TLS_STATE_SERVER_HELLO_RECEIVED;
    } else {
        PT_SPAWN(pt, &ctx->sub_pt, tls_finalize_established_pt(&ctx->sub_pt, ctx));
    }

    PT_END(pt);
}

/**
 * @brief Format 5B record header + staged ClientFinished MAC (`hs_work.ks_salt`) and send Flight 3.
 * @param ctx TLS client context with signed 72B CertificateVerify at tx_buf + 5.
 * @return true if Flight 3 record transmitted.
 */
static bool tls_send_client_mtls_flight3(SYN_TLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = TLS_CERT_VERIFY_LEN + 4U + hash_len;
    uint8_t *tx = ctx->tx_buf;
    uint8_t *fin = tx + 5U + TLS_CERT_VERIFY_LEN;

    tx[0] = TLS_CONTENT_TYPE_HANDSHAKE;
    tx[1] = 0x03U;
    tx[2] = 0x03U;
    tx[3] = 0x00U;
    tx[4] = (uint8_t)body_len;
    fin[0] = TLS_HANDSHAKE_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    memcpy(fin + 4U, ctx->hs_work.ks_salt, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    return syn_transport_send(ctx->underlying_transport, tx, 5U + body_len);
}

/**
 * @brief Client Flight 3 (mTLS) PT: sign Client CertificateVerify and send ClientFinished.
 * @param pt  Protothread control block (&ctx->flight_pt).
 * @param ctx TLS client context in SYN_TLS_STATE_SERVER_HELLO_RECEIVED.
 * @return PT status.
 */
static SYN_PT_Status tls_client_mtls_flight3_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, tls_build_local_cv_pt(&ctx->sub_pt, ctx, false));
    if (!ctx->hs_work.ok) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt,
             tls_compute_finished_pt(&ctx->sub_pt, ctx, ctx->client_handshake_secret));
    PT_CONTINUE(pt);

    if (!tls_send_client_mtls_flight3(ctx)) {
        (void)tls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &ctx->sub_pt, tls_finalize_established_pt(&ctx->sub_pt, ctx));

    PT_END(pt);
}

/**
 * @brief Dispatch the active per-state client or server handshake flight protothread.
 * @param pt  Flight protothread control block (&ctx->flight_pt).
 * @param ctx TLS context pointer.
 * @return Child flight protothread status.
 */
static SYN_PT_Status tls_dispatch_flight_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    if (ctx->config.is_server) {
        if (ctx->state == SYN_TLS_STATE_UNINITIALIZED) {
            return tls_server_flight1_pt(pt, ctx);
        }
        if (ctx->state == SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED) {
            return tls_server_flight2_pt(pt, ctx);
        }
        return tls_server_mtls_flight3_pt(pt, ctx);
    }
    if (ctx->state == SYN_TLS_STATE_UNINITIALIZED) {
        return tls_client_flight1_pt(pt, ctx);
    }
    if (ctx->state == SYN_TLS_STATE_CLIENT_HELLO_SENT) {
        return tls_client_sh_ecdhe_pt(pt, ctx);
    }
    if (ctx->state == SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED) {
        return tls_client_verify_cv_pt(pt, ctx);
    }
    if (ctx->state == SYN_TLS_STATE_CERTIFICATE_VERIFIED) {
        return tls_client_verify_fin_pt(pt, ctx);
    }
    return tls_client_mtls_flight3_pt(pt, ctx);
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
    PT_INIT(&ctx->hs_pt);
    PT_INIT(&ctx->flight_pt);
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);

    syn_x25519_clamp(ctx->my_privkey);
    tls_transcript_reset(ctx);
    return true;
}

SYN_PT_Status syn_tls_handshake_pt(SYN_PT *pt, SYN_TLS_Context *ctx)
{
    if (pt == NULL || ctx == NULL || ctx->state == SYN_TLS_STATE_ERROR) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    while (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        PT_SPAWN(pt, &ctx->flight_pt, tls_dispatch_flight_pt(&ctx->flight_pt, ctx));
        if (ctx->state == SYN_TLS_STATE_ERROR) {
            PT_EXIT(pt);
        }
        if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
            PT_CONTINUE(pt);
        }
    }

    PT_END(pt);
}

bool syn_tls_handshake(SYN_TLS_Context *ctx)
{
    if (ctx == NULL) {
        return false;
    }
    SYN_PT_Status st;
    do {
        st = syn_tls_handshake_pt(&ctx->hs_pt, ctx);
    } while (st == PT_MORE);
    return st != PT_EXITED;
}

SYN_PT_Status syn_tls_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_TLS_Context *ctx = (SYN_TLS_Context *)task->user_data;

    PT_BEGIN(pt);

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->hs_pt, syn_tls_handshake_pt(&ctx->hs_pt, ctx));
    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, ctx->state != SYN_TLS_STATE_ESTABLISHED);

    PT_END(pt);
}

/** @cond INTERNAL */
/**
 * @brief Encrypt and transmit a single TLS 1.3 application record, advancing seq only on send.
 * @param ctx  Established TLS context.
 * @param data Plaintext payload buffer.
 * @param len  Plaintext byte count (> 0 and <= SYN_TLS_RECORD_MAX_PAYLOAD).
 * @return true if record encrypted and accepted by underlying transport.
 */
static bool tls_transmit_app_record(SYN_TLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx->client_seq_num == UINT64_MAX) {
        ctx->state = SYN_TLS_STATE_ERROR;
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
    construct_tls13_nonce(tx_iv, ctx->client_seq_num, nonce);
    tls_record_encrypt(ctx, tx_key, nonce, record_buf, out_ct, pt_with_ct_len, out_ct,
                       out_ct + pt_with_ct_len);
    syn_secure_zero(nonce, sizeof(nonce));

    if (!syn_transport_send(ctx->underlying_transport, record_buf,
                            TLS_RECORD_HEADER_LEN + ciphertext_len)) {
        return false;
    }
    ctx->client_seq_num++;
    return true;
}
/** @endcond */

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

    return tls_transmit_app_record(ctx, data, len);
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

/** @cond INTERNAL */
/**
 * @brief Validate, decrypt, and commit an inbound TLS 1.3 application record.
 * @param ctx     Established TLS context.
 * @param rx_len  Bytes received into ctx->rx_buf.
 * @param data    Caller destination buffer.
 * @param max_len Caller destination capacity.
 * @param out_len [out] Decrypted plaintext byte count.
 * @return true on authentic record decryption, false on fatal error.
 */
static bool tls_decrypt_rx_record(SYN_TLS_Context *ctx, size_t rx_len, uint8_t *data,
                                  size_t max_len, size_t *out_len)
{
    size_t tag_sz = tls_tag_len(ctx);
    size_t pt_with_ct_len = 0U;
    if (ctx->server_seq_num == UINT64_MAX ||
        !tls_parse_record_header(ctx->rx_buf, rx_len, tag_sz, &pt_with_ct_len)) {
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
/** @endcond */

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

    return tls_decrypt_rx_record(ctx, rx_len, data, max_len, out_len);
}

#endif /* SYN_TLS_ENABLED */
