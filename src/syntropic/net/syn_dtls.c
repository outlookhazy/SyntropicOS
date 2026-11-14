/**
 * @file syn_dtls.c
 * @brief Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147).
 * @ingroup syn_net
 */

#include "syn_dtls.h"

#if SYN_DTLS_ENABLED

#include "syntropic/crypto/syn_asn1.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_random.h"

#include <string.h>

/** @cond INTERNAL */
#define DTLS_CONTENT_TYPE_HANDSHAKE 0x16U
#define DTLS_CONTENT_TYPE_APP_DATA 0x17U
#define DTLS_HS_CLIENT_HELLO 0x01U
#define DTLS_HS_SERVER_HELLO 0x02U
#define DTLS_HS_CERT_VERIFY 0x0FU
#define DTLS_HS_CLIENT_FINISHED 0x14U
#define DTLS_EPOCH0_HDR_LEN 13U
#define DTLS_CERT_VERIFY_LEN 72U
/** @endcond */

/* ── Anti-Replay Sliding Window (RFC 9147 Section 4.5.2) ─────────────────── */

bool syn_dtls_replay_check(const SYN_DTLS_ReplayWindow *win, uint64_t seq)
{
    if (win == NULL) {
        return false;
    }

    if (!win->initialized) {
        return true;
    }

    if (seq > win->max_seq) {
        return true;
    }

    uint64_t diff = win->max_seq - seq;
    if (diff >= SYN_DTLS_REPLAY_WINDOW_SIZE) {
        return false;
    }

    if ((win->bitmap & (1ULL << diff)) != 0ULL) {
        return false;
    }

    return true;
}

void syn_dtls_replay_update(SYN_DTLS_ReplayWindow *win, uint64_t seq)
{
    if (win == NULL) {
        return;
    }

    if (!win->initialized) {
        win->max_seq = seq;
        win->bitmap = 1ULL;
        win->initialized = true;
        return;
    }

    if (seq > win->max_seq) {
        uint64_t shift = seq - win->max_seq;
        if (shift >= SYN_DTLS_REPLAY_WINDOW_SIZE) {
            win->bitmap = 0ULL;
        } else {
            win->bitmap <<= shift;
        }
        win->bitmap |= 1ULL;
        win->max_seq = seq;
    } else {
        uint64_t diff = win->max_seq - seq;
        if (diff < SYN_DTLS_REPLAY_WINDOW_SIZE) {
            win->bitmap |= (1ULL << diff);
        }
    }
}

/** @cond INTERNAL */

/* ── Helper Functions ────────────────────────────────────────────────────── */

/**
 * @brief Return AEAD authentication tag size for active DTLS 1.3 cipher suite.
 * @param ctx DTLS context pointer.
 * @return Tag size in bytes (8 or 16).
 */
static inline size_t dtls_tag_len(const SYN_DTLS_Context *ctx)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        return 8U;
    }
    return 16U;
}

/**
 * @brief Map DTLS 1.3 cipher suite enum to 16-bit wire identifier.
 * @param suite Cipher suite enum value.
 * @return 16-bit TLS/DTLS 1.3 cipher suite code.
 */
static uint16_t dtls_cipher_wire_id(SYN_DTLS_CipherSuite suite)
{
    static const uint16_t wire_ids[5] = {0x1303U, 0x1301U, 0x1302U, 0x1304U, 0x1305U};
    size_t idx = ((size_t)suite < 5U) ? (size_t)suite : 0U;
    return wire_ids[idx];
}

/**
 * @brief Zero sensitive memory without compiler dead-store elimination.
 * @param v Buffer to wipe.
 * @param n Length in bytes.
 */
static inline void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n--) {
        *p++ = 0U;
    }
}

static bool dtls_fail_handshake(SYN_DTLS_Context *ctx);

/**
 * @brief Constant-time check that a 32-byte X25519 shared secret is non-zero.
 * @param b 32-byte shared point.
 * @return true if at least one byte is non-zero.
 */
static bool dtls_is_nonzero_32(const uint8_t b[32])
{
    return syn_x25519_is_nonzero(b);
}

/**
 * @brief Construct 12-byte DTLS 1.3 per-record nonce (RFC 9147 Section 4.2.2).
 * @param base_iv 12-byte base traffic IV.
 * @param epoch   2-bit/16-bit DTLS epoch.
 * @param seq     48-bit reconstructed packet sequence number.
 * @param nonce   [out] 12-byte output nonce.
 */
static void construct_dtls13_nonce(const uint8_t base_iv[12], SYN_DTLS_Epoch epoch, uint64_t seq,
                                   uint8_t nonce[12])
{
    memcpy(nonce, base_iv, 12);
    uint64_t full_seq = ((uint64_t)epoch << 48U) | (seq & 0xFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((full_seq >> (i * 8)) & 0xFF);
    }
}

/**
 * @brief Derive DTLS 1.3 per-direction traffic key or IV via HKDF-Expand-Label.
 * @param ctx     DTLS context pointer.
 * @param secret  Traffic secret (32 or 48 bytes).
 * @param label   HKDF label ("key" or "iv").
 * @param out     [out] Output buffer.
 * @param out_len Output length in bytes.
 */
static void dtls_derive_traffic_key_or_iv(const SYN_DTLS_Context *ctx, const uint8_t *secret,
                                          const char *label, uint8_t *out, size_t out_len)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_expand_label(secret, SYN_SHA384_DIGEST_SIZE, label, strlen(label), NULL, 0,
                                     out, out_len);
    } else {
        syn_hkdf_expand_label(secret, SYN_SHA256_DIGEST_SIZE, label, strlen(label), NULL, 0, out,
                              out_len);
    }
}

/** @brief SHA-256("") empty transcript hash per RFC 8446 §7.1 / RFC 9147. */
static const uint8_t DTLS_EMPTY_SHA256[32] = {
    0xE3U, 0xB0U, 0xC4U, 0x42U, 0x98U, 0xFCU, 0x1CU, 0x14U, 0x9AU, 0xFBU, 0xF4U,
    0xC8U, 0x99U, 0x6FU, 0xB9U, 0x24U, 0x27U, 0xAEU, 0x41U, 0xE4U, 0x64U, 0x9BU,
    0x93U, 0x4CU, 0xA4U, 0x95U, 0x99U, 0x1BU, 0x78U, 0x52U, 0xB8U, 0x55U};

/** @brief SHA-384("") empty transcript hash per RFC 8446 §7.1 / RFC 9147. */
static const uint8_t DTLS_EMPTY_SHA384[48] = {
    0x38U, 0xB0U, 0x60U, 0xA7U, 0x51U, 0xACU, 0x96U, 0x38U, 0x4CU, 0xD9U, 0x32U, 0x7EU,
    0xB1U, 0xB1U, 0xE3U, 0x6AU, 0x21U, 0xFDU, 0xB7U, 0x11U, 0x14U, 0xBEU, 0x07U, 0x43U,
    0x4CU, 0x0CU, 0xC7U, 0xBFU, 0x63U, 0xF6U, 0xE1U, 0xDAU, 0x27U, 0x4EU, 0xDEU, 0xBFU,
    0xE7U, 0x6FU, 0x65U, 0xFBU, 0xD5U, 0x1AU, 0xD2U, 0xF1U, 0x48U, 0x98U, 0xB9U, 0x5BU};

/**
 * @brief Compute initial handshake transcript digest over ClientHello and ServerHello fields.
 * @param ctx               DTLS context pointer.
 * @param is_sha384         true for SHA-384, false for SHA-256.
 * @param transcript_digest [out] Output digest buffer.
 */
static void dtls_compute_transcript(SYN_DTLS_Context *ctx, bool is_sha384,
                                    uint8_t transcript_digest[SYN_DTLS_SECRET_LEN])
{
    const uint8_t *c_pub = ctx->config.is_server ? ctx->peer_ecdhe_pub : ctx->ecdhe_pub;
    const uint8_t *s_pub = ctx->config.is_server ? ctx->ecdhe_pub : ctx->peer_ecdhe_pub;
    uint16_t suite_id = dtls_cipher_wire_id(ctx->config.cipher_suite);
    uint8_t ch_hdr[3] = {DTLS_HS_CLIENT_HELLO, (uint8_t)(suite_id >> 8U),
                         (uint8_t)(suite_id & 0xFFU)};
    uint8_t sh_hdr[3] = {DTLS_HS_SERVER_HELLO, (uint8_t)(suite_id >> 8U),
                         (uint8_t)(suite_id & 0xFFU)};

    if (is_sha384) {
        syn_sha384_init(&ctx->crypto_scratch.sha512);
        syn_sha512_update(&ctx->crypto_scratch.sha512, ch_hdr, sizeof(ch_hdr));
        syn_sha512_update(&ctx->crypto_scratch.sha512, ctx->client_random, 32U);
        syn_sha512_update(&ctx->crypto_scratch.sha512, c_pub, 32U);
        syn_sha512_update(&ctx->crypto_scratch.sha512, sh_hdr, sizeof(sh_hdr));
        syn_sha512_update(&ctx->crypto_scratch.sha512, ctx->server_random, 32U);
        syn_sha512_update(&ctx->crypto_scratch.sha512, s_pub, 32U);
        syn_sha384_final(&ctx->crypto_scratch.sha512, transcript_digest);
    } else {
        syn_sha256_init(&ctx->crypto_scratch.sha256);
        syn_sha256_update(&ctx->crypto_scratch.sha256, ch_hdr, sizeof(ch_hdr));
        syn_sha256_update(&ctx->crypto_scratch.sha256, ctx->client_random, 32U);
        syn_sha256_update(&ctx->crypto_scratch.sha256, c_pub, 32U);
        syn_sha256_update(&ctx->crypto_scratch.sha256, sh_hdr, sizeof(sh_hdr));
        syn_sha256_update(&ctx->crypto_scratch.sha256, ctx->server_random, 32U);
        syn_sha256_update(&ctx->crypto_scratch.sha256, s_pub, 32U);
        syn_sha256_final(&ctx->crypto_scratch.sha256, transcript_digest);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
}

/**
 * @brief Chain additional handshake data into a running DTLS transcript digest.
 * @param ctx       DTLS context pointer (uses crypto_scratch).
 * @param tr_digest [in,out] Running transcript digest buffer.
 * @param data      Handshake bytes to fold into transcript.
 * @param len       Byte count of data.
 */
static void dtls_chain_transcript(SYN_DTLS_Context *ctx, uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                  const uint8_t *data, size_t len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    if (is_sha384) {
        syn_sha384_init(&ctx->crypto_scratch.sha512);
        syn_sha512_update(&ctx->crypto_scratch.sha512, tr_digest, SYN_SHA384_DIGEST_SIZE);
        syn_sha512_update(&ctx->crypto_scratch.sha512, data, len);
        syn_sha384_final(&ctx->crypto_scratch.sha512, tr_digest);
    } else {
        syn_sha256_init(&ctx->crypto_scratch.sha256);
        syn_sha256_update(&ctx->crypto_scratch.sha256, tr_digest, SYN_SHA256_DIGEST_SIZE);
        syn_sha256_update(&ctx->crypto_scratch.sha256, data, len);
        syn_sha256_final(&ctx->crypto_scratch.sha256, tr_digest);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
}

/**
 * @brief Cipher-suite-aware HKDF-Extract (SHA-256 or SHA-384) for DTLS 1.3.
 * @param ctx      DTLS context pointer.
 * @param salt     Salt bytes.
 * @param salt_len Salt byte length.
 * @param ikm      Input keying material.
 * @param ikm_len  IKM byte length.
 * @param prk      [out] Pseudorandom key output buffer.
 */
static void dtls_hkdf_extract_cs(const SYN_DTLS_Context *ctx, const uint8_t *salt, size_t salt_len,
                                 const uint8_t *ikm, size_t ikm_len, uint8_t *prk)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_extract(salt, salt_len, ikm, ikm_len, prk);
    } else {
        syn_hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    }
}

/**
 * @brief Cipher-suite-aware HKDF-Expand-Label (SHA-256 or SHA-384) for DTLS 1.3.
 * @param ctx         DTLS context pointer.
 * @param secret      Base secret (32 or 48 bytes).
 * @param label       TLS 1.3 label string without "tls13 " prefix.
 * @param label_len   Label byte length.
 * @param context     Transcript hash context bytes.
 * @param context_len Context byte length.
 * @param out         [out] Derived key material output buffer.
 * @param out_len     Desired output byte length.
 */
static void dtls_hkdf_expand_cs(const SYN_DTLS_Context *ctx, const uint8_t *secret,
                                const char *label, size_t label_len, const uint8_t *context,
                                size_t context_len, uint8_t *out, size_t out_len)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_expand_label(secret, SYN_SHA384_DIGEST_SIZE, label, label_len, context,
                                     context_len, out, out_len);
    } else {
        syn_hkdf_expand_label(secret, SYN_SHA256_DIGEST_SIZE, label, label_len, context,
                              context_len, out, out_len);
    }
}

/**
 * @brief Validate ECDHE shared secret and compute ClientHello..ServerHello transcript digest.
 * @param ctx DTLS context pointer.
 * @return true if ECDHE shared secret is non-zero.
 */
static bool dtls_hs_secrets_prep(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    if (!dtls_is_nonzero_32(ctx->hs_work.ecdhe_secret)) {
        syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
        return false;
    }
    dtls_compute_transcript(ctx, is_sha384, ctx->transcript_digest);
    return true;
}

/**
 * @brief Extract DTLS 1.3 Early Secret into hs_work.ks_prk.
 * @param ctx DTLS context pointer.
 */
static void dtls_hs_extract_early(SYN_DTLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_DTLS_SECRET_LEN] = {0};
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    uint8_t ikm[SYN_DTLS_SECRET_LEN];
    memset(ikm, 0, sizeof(ikm));
    size_t ikm_len = hash_len;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_PSK) {
        ikm_len =
            (ctx->config.psk_secret_len > sizeof(ikm)) ? sizeof(ikm) : ctx->config.psk_secret_len;
        memcpy(ikm, ctx->config.psk_secret, ikm_len);
    }
    dtls_hkdf_extract_cs(ctx, zero_salt, hash_len, ikm, ikm_len, ctx->hs_work.ks_prk);
    syn_secure_zero(ikm, sizeof(ikm));
}

/**
 * @brief Expand "derived" salt from hs_work.ks_prk over empty transcript hash into hs_work.ks_salt.
 * @param ctx DTLS context pointer.
 */
static void dtls_hs_expand_derived(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    const uint8_t *empty_h = is_sha384 ? DTLS_EMPTY_SHA384 : DTLS_EMPTY_SHA256;
    dtls_hkdf_expand_cs(ctx, ctx->hs_work.ks_prk, "derived", 7U, empty_h, hash_len,
                        ctx->hs_work.ks_salt, hash_len);
}

/**
 * @brief Extract DTLS Handshake Secret from ks_salt and ecdhe_secret into hs_work.ks_prk.
 * @param ctx DTLS context pointer.
 */
static void dtls_hs_extract_hs(SYN_DTLS_Context *ctx)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    dtls_hkdf_extract_cs(ctx, ctx->hs_work.ks_salt, hash_len, ctx->hs_work.ecdhe_secret, 32U,
                         ctx->hs_work.ks_prk);
    syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
}

/**
 * @brief Expand a DTLS handshake traffic secret ("c hs traffic" or "s hs traffic").
 * @param ctx   DTLS context pointer.
 * @param label 12-byte label ("c hs traffic" or "s hs traffic").
 * @param out   [out] Destination secret buffer.
 */
static void dtls_hs_expand_traffic(SYN_DTLS_Context *ctx, const char *label, uint8_t *out)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    dtls_hkdf_expand_cs(ctx, ctx->hs_work.ks_prk, label, 12U, ctx->transcript_digest, hash_len, out,
                        hash_len);
}

/**
 * @brief Extract master_secret from ks_salt and scrub HKDF staging buffers.
 * @param ctx DTLS context pointer.
 */
static void dtls_hs_extract_master(SYN_DTLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_DTLS_SECRET_LEN] = {0};
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    dtls_hkdf_extract_cs(ctx, ctx->hs_work.ks_salt, hash_len, zero_salt, hash_len,
                         ctx->master_secret);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
}

/**
 * @brief Cooperative child protothread deriving DTLS 1.3 handshake secrets (1 HKDF per slice).
 * @param pt  Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx DTLS context pointer (`hs_work.crypto_ok` set true on success).
 * @return Protothread status.
 */
static SYN_PT_Status dtls_derive_hs_secrets_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    PT_BEGIN(pt);

    ctx->hs_work.crypto_ok = false;
    if (!dtls_hs_secrets_prep(ctx)) {
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    dtls_hs_extract_early(ctx);
    PT_CONTINUE(pt);

    dtls_hs_expand_derived(ctx);
    PT_CONTINUE(pt);

    dtls_hs_extract_hs(ctx);
    PT_CONTINUE(pt);

    dtls_hs_expand_traffic(ctx, "c hs traffic", ctx->client_handshake_secret);
    PT_CONTINUE(pt);

    dtls_hs_expand_traffic(ctx, "s hs traffic", ctx->server_handshake_secret);
    PT_CONTINUE(pt);

    dtls_hs_expand_derived(ctx);
    PT_CONTINUE(pt);

    dtls_hs_extract_master(ctx);
    ctx->hs_work.crypto_ok = true;

    PT_END(pt);
}

/**
 * @brief Wipe ephemeral X25519 private key and intermediate handshake secrets once established.
 * @param ctx DTLS context pointer.
 */
static void dtls_scrub_handshake_secrets(SYN_DTLS_Context *ctx)
{
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);
    syn_secure_zero(ctx->ecdhe_priv, sizeof(ctx->ecdhe_priv));
    syn_secure_zero(ctx->client_handshake_secret, sizeof(ctx->client_handshake_secret));
    syn_secure_zero(ctx->server_handshake_secret, sizeof(ctx->server_handshake_secret));
    syn_secure_zero(ctx->master_secret, sizeof(ctx->master_secret));
    syn_secure_zero(&ctx->hs_work, sizeof(ctx->hs_work));
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
}

/**
 * @brief Scrub transient handshake state and transition context to SYN_DTLS_STATE_ERROR.
 * @param ctx DTLS context pointer.
 * @return Always false.
 */
static bool dtls_fail_handshake(SYN_DTLS_Context *ctx)
{
    PT_INIT(&ctx->hs_pt);
    PT_INIT(&ctx->flight_pt);
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);
    syn_secure_zero(ctx->client_handshake_secret, sizeof(ctx->client_handshake_secret));
    syn_secure_zero(ctx->server_handshake_secret, sizeof(ctx->server_handshake_secret));
    syn_secure_zero(ctx->master_secret, sizeof(ctx->master_secret));
    syn_secure_zero(ctx->client_app_secret, sizeof(ctx->client_app_secret));
    syn_secure_zero(ctx->server_app_secret, sizeof(ctx->server_app_secret));
    syn_secure_zero(ctx->client_app_key, sizeof(ctx->client_app_key));
    syn_secure_zero(ctx->client_app_iv, sizeof(ctx->client_app_iv));
    syn_secure_zero(ctx->server_app_key, sizeof(ctx->server_app_key));
    syn_secure_zero(ctx->server_app_iv, sizeof(ctx->server_app_iv));
    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    syn_secure_zero(ctx->hs_work.tr_cand, sizeof(ctx->hs_work.tr_cand));
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
    syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    ctx->hs_work.crypto_ok = false;
    ctx->state = SYN_DTLS_STATE_ERROR;
    return false;
}

/**
 * @brief Scrub unauthenticated ServerHello candidate state and return to CLIENT_HELLO_SENT.
 * @param ctx DTLS client context after a forged/corrupted UDP ServerHello flight.
 */
static void dtls_rollback_forged_sh(SYN_DTLS_Context *ctx)
{
    PT_INIT(&ctx->flight_pt);
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);
    syn_secure_zero(ctx->server_random, sizeof(ctx->server_random));
    syn_secure_zero(ctx->peer_ecdhe_pub, sizeof(ctx->peer_ecdhe_pub));
    syn_secure_zero(ctx->client_handshake_secret, sizeof(ctx->client_handshake_secret));
    syn_secure_zero(ctx->server_handshake_secret, sizeof(ctx->server_handshake_secret));
    syn_secure_zero(ctx->master_secret, sizeof(ctx->master_secret));
    syn_secure_zero(ctx->transcript_digest, sizeof(ctx->transcript_digest));
    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));
    syn_secure_zero(ctx->hs_work.ecdhe_secret, sizeof(ctx->hs_work.ecdhe_secret));
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    ctx->hs_work.crypto_ok = false;
    ctx->state = SYN_DTLS_STATE_CLIENT_HELLO_SENT;
}

/**
 * @brief Return AEAD key byte length (16 or 32) for the configured DTLS 1.3 cipher suite.
 * @param ctx DTLS context pointer.
 * @return AEAD key size in bytes.
 */
static size_t dtls_aead_key_len(const SYN_DTLS_Context *ctx)
{
    return (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256 ||
            ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256 ||
            ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256)
               ? 16U
               : 32U;
}

/**
 * @brief Expand a DTLS application traffic secret ("c ap traffic" or "s ap traffic").
 * @param ctx   DTLS context pointer.
 * @param label 12-byte label ("c ap traffic" or "s ap traffic").
 * @param out   [out] Destination secret buffer.
 */
static void dtls_app_expand_secret(SYN_DTLS_Context *ctx, const char *label, uint8_t *out)
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;
    dtls_hkdf_expand_cs(ctx, ctx->master_secret, label, 12U, ctx->transcript_digest, hash_len, out,
                        hash_len);
}

/**
 * @brief Cooperative child protothread deriving DTLS 1.3 application traffic keys (1 HKDF/slice).
 * @param pt  Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx DTLS context pointer.
 * @return Protothread status.
 */
static SYN_PT_Status dtls_derive_app_keys_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    PT_BEGIN(pt);

    dtls_app_expand_secret(ctx, "c ap traffic", ctx->client_app_secret);
    PT_CONTINUE(pt);

    dtls_app_expand_secret(ctx, "s ap traffic", ctx->server_app_secret);
    PT_CONTINUE(pt);

    dtls_derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "key", ctx->client_app_key,
                                  dtls_aead_key_len(ctx));
    PT_CONTINUE(pt);

    dtls_derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "iv", ctx->client_app_iv, 12U);
    PT_CONTINUE(pt);

    dtls_derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "key", ctx->server_app_key,
                                  dtls_aead_key_len(ctx));
    PT_CONTINUE(pt);

    dtls_derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "iv", ctx->server_app_iv, 12U);
    PT_CONTINUE(pt);

    PT_END(pt);
}

/**
 * @brief Cooperative child protothread computing DTLS 1.3 Finished MAC into hs_work.ks_salt.
 * @param pt        Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx       DTLS context pointer (output MAC staged in `ctx->hs_work.ks_salt`).
 * @param hs_secret Base handshake secret (server or client).
 * @param tr_digest Transcript digest bytes.
 * @return Protothread status.
 */
static SYN_PT_Status dtls_compute_finished_pt(SYN_PT *pt, SYN_DTLS_Context *ctx,
                                              const uint8_t *hs_secret,
                                              const uint8_t tr_digest[SYN_DTLS_SECRET_LEN])
{
    size_t hash_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384)
                          ? SYN_SHA384_DIGEST_SIZE
                          : SYN_SHA256_DIGEST_SIZE;

    PT_BEGIN(pt);

    dtls_hkdf_expand_cs(ctx, hs_secret, "finished", 8U, NULL, 0U, ctx->hs_work.ks_prk, hash_len);
    PT_CONTINUE(pt);

    dtls_hkdf_extract_cs(ctx, ctx->hs_work.ks_prk, hash_len, tr_digest, hash_len,
                         ctx->hs_work.ks_salt);
    syn_secure_zero(ctx->hs_work.ks_prk, sizeof(ctx->hs_work.ks_prk));

    PT_END(pt);
}

/**
 * @brief Encrypt DTLS 1.3 inner plaintext with unified header AAD.
 * @param ctx     DTLS context pointer.
 * @param key     AEAD traffic key.
 * @param nonce   12-byte record nonce.
 * @param aad     Unified header AAD buffer.
 * @param aad_len Unified header AAD length.
 * @param in      Inner plaintext buffer.
 * @param in_len  Inner plaintext length.
 * @param out_ct  [out] Ciphertext output buffer (may alias in).
 * @param out_tag [out] Authentication tag output buffer.
 */
static void dtls_record_encrypt(SYN_DTLS_Context *ctx, const uint8_t *key, const uint8_t nonce[12],
                                const uint8_t *aad, size_t aad_len, const uint8_t *in,
                                size_t in_len, uint8_t *out_ct, uint8_t *out_tag)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 16U);
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag);
        syn_secure_zero(&ctx->crypto_scratch.gcm, sizeof(ctx->crypto_scratch.gcm));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag);
        syn_secure_zero(&ctx->crypto_scratch.gcm, sizeof(ctx->crypto_scratch.gcm));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag, 16U);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag, 8U);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else {
        syn_aead_encrypt(key, nonce, aad, aad_len, in, in_len, out_ct, out_tag);
    }
}

/**
 * @brief Decrypt and authenticate DTLS 1.3 record payload with unified header AAD.
 * @param ctx    DTLS context pointer.
 * @param key    AEAD traffic key.
 * @param nonce  12-byte record nonce.
 * @param aad    Unified header AAD buffer.
 * @param aad_len Unified header AAD length.
 * @param ct     Ciphertext input buffer.
 * @param ct_len Ciphertext length in bytes.
 * @param tag    Authentication tag buffer.
 * @param out_pt [out] Plaintext output buffer (may alias ct).
 * @return true if authentication succeeded.
 */
static bool dtls_record_decrypt(SYN_DTLS_Context *ctx, const uint8_t *key, const uint8_t nonce[12],
                                const uint8_t *aad, size_t aad_len, const uint8_t *ct,
                                size_t ct_len, const uint8_t *tag, uint8_t *out_pt)
{
    bool ok = false;
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 16U);
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, ct, ct_len,
                                  out_pt, tag) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.gcm, sizeof(ctx->crypto_scratch.gcm));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, ct, ct_len,
                                  out_pt, tag) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.gcm, sizeof(ctx->crypto_scratch.gcm));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, ct, ct_len,
                                  tag, 16U, out_pt) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, ct, ct_len,
                                  tag, 8U, out_pt) == SYN_OK);
        syn_secure_zero(&ctx->crypto_scratch.aes, sizeof(ctx->crypto_scratch.aes));
    } else {
        ok = syn_aead_decrypt(key, nonce, aad, aad_len, ct, ct_len, tag, out_pt);
    }
    return ok;
}

/* ── Transport Adapter ───────────────────────────────────────────────────── */

/**
 * @brief Send callback adapter for DTLS transport binding.
 * @param data Payload buffer.
 * @param len  Payload length.
 * @param ctx  DTLS context pointer.
 * @return true on successful send.
 */
static bool dtls_transport_send_cb(const uint8_t *data, size_t len, void *ctx)
{
    return syn_dtls_send((SYN_DTLS_Context *)ctx, data, len);
}

/**
 * @brief Receive callback adapter for DTLS transport binding.
 * @param data    Output buffer.
 * @param max_len Output buffer capacity.
 * @param out_len [out] Received payload length.
 * @param ctx     DTLS context pointer.
 * @return true on successful receive.
 */
static bool dtls_transport_recv_cb(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    return syn_dtls_recv((SYN_DTLS_Context *)ctx, data, max_len, out_len);
}

/**
 * @brief Check if underlying datagram transport is readable.
 * @param ctx DTLS context pointer.
 * @return true if wire data is available.
 */
static bool dtls_transport_has_data_cb(const void *ctx)
{
    const SYN_DTLS_Context *dtls = (const SYN_DTLS_Context *)ctx;
    if (dtls == NULL) {
        return false;
    }
    return (dtls->underlying_transport != NULL) &&
           syn_transport_has_data(dtls->underlying_transport);
}

/**
 * @brief Reconstruct 48-bit sequence number from 8/16-bit wire LSBs (RFC 9147 Section 4.2.2).
 *
 * Selects the 48-bit integer congruent to wire_seq modulo 2^8 or 2^16 closest to
 * max_seq + 1 so sequence wraparound at 256 and 65536 advances without lockout.
 *
 * @param win       Sliding replay window state.
 * @param wire_seq  Truncated 8-bit or 16-bit wire sequence number.
 * @param seq_is_16 true for 16-bit wire sequence, false for 8-bit.
 * @return Reconstructed 48-bit sequence number.
 */
static uint64_t dtls_reconstruct_seq(const SYN_DTLS_ReplayWindow *win, uint64_t wire_seq,
                                     bool seq_is_16)
{
    uint64_t expected = win->initialized ? (win->max_seq + 1ULL) : 0ULL;
    uint64_t mod = seq_is_16 ? 65536ULL : 256ULL;
    uint64_t mask = mod - 1ULL;
    uint64_t half = mod >> 1U;
    uint64_t candidate = (expected & ~mask) | (wire_seq & mask);

    if (candidate < expected && (expected - candidate) > half) {
        candidate += mod;
    } else if (candidate > expected && (candidate - expected) >= half && candidate >= mod) {
        candidate -= mod;
    }
    return candidate & 0xFFFFFFFFFFFFULL;
}

/**
 * @brief Parse and bounds-check DTLS 1.3 unified record header (RFC 9147 Section 4.1).
 * @param ctx                DTLS context pointer.
 * @param rx_len             Received datagram length.
 * @param rec_epoch          [out] Decoded epoch low bits.
 * @param seq_out            [out] Reconstructed 48-bit sequence number.
 * @param hdr_len_out        [out] Header length in bytes.
 * @param pt_with_ct_len_out [out] Inner plaintext + content-type length in bytes.
 * @return true if unified header and record lengths are valid.
 */
static bool dtls_parse_unified_header(const SYN_DTLS_Context *ctx, size_t rx_len,
                                      SYN_DTLS_Epoch *rec_epoch, uint64_t *seq_out,
                                      size_t *hdr_len_out, size_t *pt_with_ct_len_out)
{
    if (rx_len < 4U) {
        return false;
    }

    uint8_t flags = ctx->rx_buf[0];
    if ((flags & SYN_DTLS_UNIFIED_FIXED_BIT) == 0U || (flags & SYN_DTLS_UNIFIED_CID_BIT) != 0U) {
        return false;
    }

    *rec_epoch = (SYN_DTLS_Epoch)(flags & SYN_DTLS_UNIFIED_EPOCH_MASK);
    bool seq_is_16 = (flags & SYN_DTLS_UNIFIED_SEQ_16BIT) != 0U;
    size_t seq_bytes = seq_is_16 ? 2U : 1U;
    bool has_len = (flags & SYN_DTLS_UNIFIED_LEN_BIT) != 0U;

    size_t min_hdr = 1U + seq_bytes + (has_len ? 2U : 0U);
    size_t tag_sz = dtls_tag_len(ctx);
    if (rx_len < min_hdr + 1U + tag_sz) {
        return false;
    }

    size_t offset = 1U;
    uint64_t wire_seq = 0U;
    if (seq_is_16) {
        wire_seq = ((uint64_t)ctx->rx_buf[offset] << 8U) | (uint64_t)ctx->rx_buf[offset + 1U];
        offset += 2U;
    } else {
        wire_seq = (uint64_t)ctx->rx_buf[offset++];
    }

    size_t ct_tag_len = 0U;
    if (has_len) {
        ct_tag_len = ((size_t)ctx->rx_buf[offset] << 8U) | (size_t)ctx->rx_buf[offset + 1U];
        offset += 2U;
    } else {
        ct_tag_len = rx_len - offset;
    }

    if (ct_tag_len > rx_len - offset || ct_tag_len < tag_sz + 1U) {
        return false;
    }

    size_t pt_with_ct_len = ct_tag_len - tag_sz;
    if (pt_with_ct_len > SYN_DTLS_RECORD_MAX_PAYLOAD + 1U) {
        return false;
    }

    *seq_out = dtls_reconstruct_seq(&ctx->replay_window, wire_seq, seq_is_16);
    *hdr_len_out = offset;
    *pt_with_ct_len_out = pt_with_ct_len;
    return true;
}

/**
 * @brief Validate local certificate, private key, and RPK length constraints for DTLS.
 * @param ctx DTLS context pointer.
 * @return true if local credentials satisfy configured authentication mode.
 */
static bool dtls_verify_local_auth(const SYN_DTLS_Context *ctx)
{
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_PSK) {
        return (ctx->config.psk_secret != NULL) && (ctx->config.psk_secret_len > 0U);
    }
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY) {
        if (ctx->config.is_server) {
            return ctx->config.client_privkey != NULL;
        }
        return (ctx->config.peer_pubkey != NULL) && (ctx->config.peer_pubkey_len == 32U);
    }
    if ((ctx->config.mode == SYN_DTLS_AUTH_MODE_X509_SERVER && ctx->config.is_server) ||
        ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        return (ctx->config.client_cert_der != NULL) && (ctx->config.client_cert_len > 0U) &&
               (ctx->config.client_privkey != NULL);
    }
    return true;
}

/**
 * @brief Format RFC 8446 §4.4.3 / RFC 9147 §5.3 CertificateVerify signing content.
 * @param is_server_cv true for server context string, false for client.
 * @param tr_digest    Transcript hash bytes.
 * @param hash_len     Transcript hash length (32 or 48).
 * @param out          [out] Formatted buffer (up to 146 bytes).
 * @return Total formatted byte length (98 + hash_len).
 */
static size_t dtls_format_cv_input(bool is_server_cv, const uint8_t *tr_digest, size_t hash_len,
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
 * @param ctx          DTLS context pointer with hs_work.cv_algo populated.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer (8B header written).
 * @return true if signing context initialized successfully.
 */
static bool dtls_init_cv_sign(SYN_DTLS_Context *ctx, bool is_server_cv,
                              uint8_t out_cv[DTLS_CERT_VERIFY_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len =
        dtls_format_cv_input(is_server_cv, ctx->transcript_digest, hash_len, ctx->hs_work.cv_msg);

    out_cv[0] = DTLS_HS_CERT_VERIFY;
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
 * @param ctx DTLS context pointer.
 * @return true if certificate parsed with an Ed25519 or ECDSA-P256 subject public key.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static bool
dtls_parse_local_cert(SYN_DTLS_Context *ctx)
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
 * @brief Cooperative wrapper for P-256 or Ed25519 CertificateVerify signing.
 * @param ctx    DTLS context pointer.
 * @param out_cv 72-byte CertificateVerify wire buffer.
 * @return Child protothread status.
 */
static SYN_PT_Status dtls_cv_sign_crypto_pt(SYN_DTLS_Context *ctx, uint8_t *out_cv)
{
    if (ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) {
        return syn_p256_sign_pt(&ctx->crypto_pt, &ctx->crypto_scratch.p256, out_cv + 8U,
                                out_cv + 40U, &ctx->hs_work.crypto_ok);
    }
    SYN_PT_Status st =
        syn_ed25519_sign_pt(&ctx->crypto_pt, &ctx->crypto_scratch.ed25519, out_cv + 8U);
    if (st == PT_ENDED) {
        ctx->hs_work.crypto_ok = true;
    }
    return st;
}

/**
 * @brief Cooperatively bind local credential into transcript and sign CertificateVerify.
 * @param pt           Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx          DTLS context pointer (`hs_work.crypto_ok` set true on success).
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer.
 * @return Protothread status (`PT_MORE`, `PT_EXITED`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_build_local_cv_pt(SYN_PT *pt, SYN_DTLS_Context *ctx, bool is_server_cv,
                                            uint8_t out_cv[DTLS_CERT_VERIFY_LEN])
{
    SYN_PT *cpt = &ctx->crypto_pt;
    SYN_Ed25519_Ctx *ed = &ctx->crypto_scratch.ed25519;
    bool *ok = &ctx->hs_work.crypto_ok;

    PT_BEGIN(pt);

    *ok = false;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY) {
        ctx->hs_work.cv_algo = SYN_X509_ALGO_ED25519;
        (void)syn_ed25519_publickey_init(ed, ctx->config.client_privkey);
        PT_CONTINUE(pt);
        PT_SPAWN(pt, cpt, syn_ed25519_publickey_pt(cpt, ed, ctx->hs_work.pub_cache));
        PT_CONTINUE(pt);
        dtls_chain_transcript(ctx, ctx->transcript_digest, ctx->hs_work.pub_cache, 32U);
        (void)dtls_init_cv_sign(ctx, is_server_cv, out_cv);
        PT_CONTINUE(pt);
    } else {
        if (!dtls_parse_local_cert(ctx)) {
            PT_EXIT(pt);
        }
        dtls_chain_transcript(ctx, ctx->transcript_digest, ctx->config.client_cert_der,
                              ctx->config.client_cert_len);
        PT_CONTINUE(pt);
        if (!dtls_init_cv_sign(ctx, is_server_cv, out_cv)) {
            PT_EXIT(pt);
        }
        PT_CONTINUE(pt);
    }

    PT_SPAWN(pt, cpt, dtls_cv_sign_crypto_pt(ctx, out_cv));
    PT_CONTINUE(pt);

    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    if (!(*ok)) {
        (void)dtls_fail_handshake(ctx);
        PT_EXIT(pt);
    }
    dtls_chain_transcript(ctx, ctx->transcript_digest, out_cv, DTLS_CERT_VERIFY_LEN);

    PT_END(pt);
}

/**
 * @brief Initialize cooperative ECDSA-P256 verification over SHA-256(hs_work.cv_msg).
 * @param ctx    DTLS context pointer with hs_work.pub_cache and hs_work.cv_msg populated.
 * @param cv_len Byte length of hs_work.cv_msg.
 * @param sig64  64-byte raw (r || s) signature.
 * @return true if public key and signature scalars passed initial checks.
 */
static bool dtls_init_p256_cv_verify(SYN_DTLS_Context *ctx, size_t cv_len, const uint8_t sig64[64])
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
 * @brief Bind peer raw public key or X.509 DER certificate into the DTLS transcript digest.
 * @param ctx       DTLS context pointer.
 * @param tr_digest [in,out] Transcript digest buffer.
 */
static void dtls_bind_peer_cv_cred(SYN_DTLS_Context *ctx, uint8_t tr_digest[SYN_DTLS_SECRET_LEN])
{
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY) {
        ctx->hs_work.cv_algo = SYN_X509_ALGO_ED25519;
        memcpy(ctx->hs_work.pub_cache, ctx->config.peer_pubkey, 32U);
        ctx->hs_work.pub_cache_len = 32U;
        dtls_chain_transcript(ctx, tr_digest, ctx->config.peer_pubkey, 32U);
    } else {
        ctx->hs_work.cv_algo = ctx->hs_work.peer_algo;
        ctx->hs_work.pub_cache_len = ctx->hs_work.peer_pub_len;
        memcpy(ctx->hs_work.pub_cache, ctx->hs_work.peer_pub, ctx->hs_work.peer_pub_len);
        dtls_chain_transcript(ctx, tr_digest, ctx->config.peer_cert_der, ctx->config.peer_cert_len);
    }
}

/**
 * @brief Validate CertificateVerify framing and initialize signature verification context.
 * @param ctx          DTLS context pointer with peer credential already bound into tr_digest.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param tr_digest    Transcript digest buffer.
 * @param in_cv        72-byte wire CertificateVerify message.
 * @return true if header, SignatureScheme, and initial curve checks succeeded.
 */
static bool dtls_init_peer_cv_verify(SYN_DTLS_Context *ctx, bool is_server_cv,
                                     const uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                     const uint8_t in_cv[DTLS_CERT_VERIFY_LEN])
{
    static const uint8_t cv_hdr[4] = {DTLS_HS_CERT_VERIFY, 0x00U, 0x00U, 68U};
    if ((memcmp(in_cv, cv_hdr, 4U) != 0) || (in_cv[6] != 0x00U) || (in_cv[7] != 64U)) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = dtls_format_cv_input(is_server_cv, tr_digest, hash_len, ctx->hs_work.cv_msg);
    uint16_t scheme = ((uint16_t)in_cv[4] << 8U) | (uint16_t)in_cv[5];

    if ((ctx->hs_work.cv_algo == SYN_X509_ALGO_ED25519) && (scheme == 0x0807U) &&
        (ctx->hs_work.pub_cache_len == 32U)) {
        return syn_ed25519_verify_init(&ctx->crypto_scratch.ed25519, in_cv + 8U,
                                       ctx->hs_work.cv_msg, cv_len, ctx->hs_work.pub_cache);
    }
    if ((ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) && (scheme == 0x0403U)) {
        return dtls_init_p256_cv_verify(ctx, cv_len, in_cv + 8U);
    }
    return false;
}

/**
 * @brief Cooperatively bind peer credential into transcript and verify CertificateVerify.
 * @param pt           Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx          DTLS context pointer (`hs_work.crypto_ok` set true when authentic).
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param tr_digest    [in,out] Transcript digest buffer.
 * @param in_cv        72-byte inbound CertificateVerify message.
 * @return Protothread status (`PT_MORE`, `PT_EXITED`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_check_peer_cv_pt(SYN_PT *pt, SYN_DTLS_Context *ctx, bool is_server_cv,
                                           uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                           const uint8_t in_cv[DTLS_CERT_VERIFY_LEN])
{
    SYN_PT *cpt = &ctx->crypto_pt;
    bool *ok = &ctx->hs_work.crypto_ok;

    PT_BEGIN(pt);

    *ok = false;
    dtls_bind_peer_cv_cred(ctx, tr_digest);
    PT_CONTINUE(pt);

    if (!dtls_init_peer_cv_verify(ctx, is_server_cv, tr_digest, in_cv)) {
        syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
        syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    if (ctx->hs_work.cv_algo == SYN_X509_ALGO_ECDSA_P256) {
        PT_SPAWN(pt, cpt, syn_p256_verify_pt(cpt, &ctx->crypto_scratch.p256, ok));
    } else {
        PT_SPAWN(pt, cpt, syn_ed25519_verify_pt(cpt, &ctx->crypto_scratch.ed25519, ok));
    }
    PT_CONTINUE(pt);

    syn_secure_zero(ctx->hs_work.cv_msg, sizeof(ctx->hs_work.cv_msg));
    syn_secure_zero(ctx->hs_work.pub_cache, sizeof(ctx->hs_work.pub_cache));
    if (!(*ok)) {
        PT_EXIT(pt);
    }
    dtls_chain_transcript(ctx, tr_digest, in_cv, DTLS_CERT_VERIFY_LEN);

    PT_END(pt);
}

/**
 * @brief Constant-time comparison of DTLS Finished verify_data MAC.
 * @param expected Expected verify_data bytes.
 * @param actual   Received verify_data bytes.
 * @param hash_len MAC length in bytes (32 or 48).
 * @return true if MACs match identically.
 */
static bool dtls_verify_mac_ct(const uint8_t *expected, const uint8_t *actual, size_t hash_len)
{
    uint8_t diff = 0U;
    for (size_t i = 0U; i < hash_len; i++) {
        diff |= (uint8_t)(expected[i] ^ actual[i]);
    }
    return diff == 0U;
}

/**
 * @brief Parse peer X.509 certificate in an isolated stack frame and initialize chain validation.
 * @param ctx DTLS context pointer.
 * @return true if certificate parsed and chain verification context initialized.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static bool
dtls_init_peer_x509(SYN_DTLS_Context *ctx)
{
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
 * @brief Return true if configured DTLS endpoint must verify a peer X.509 certificate chain.
 * @param ctx DTLS context pointer.
 * @return true for mTLS endpoints and X.509 clients, false otherwise.
 */
static bool dtls_needs_peer_x509(const SYN_DTLS_Context *ctx)
{
    return (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) ||
           ((ctx->config.mode == SYN_DTLS_AUTH_MODE_X509_SERVER) && !ctx->config.is_server);
}

/**
 * @brief Validate local credentials and cooperatively verify peer X.509 certificate chain.
 * @param pt  Sub-protothread control block (`&ctx->sub_pt`).
 * @param ctx DTLS context pointer (`hs_work.peer_auth_verified` set true on success).
 * @return Protothread status (`PT_MORE`, `PT_EXITED`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_peer_auth_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *cpt = &ctx->crypto_pt;
    bool *ok = &ctx->hs_work.crypto_ok;

    PT_BEGIN(pt);

    if (!ctx->hs_work.peer_auth_verified) {
        if (!dtls_verify_local_auth(ctx)) {
            (void)dtls_fail_handshake(ctx);
            PT_EXIT(pt);
        }
        if (dtls_needs_peer_x509(ctx)) {
            if (!dtls_init_peer_x509(ctx)) {
                (void)dtls_fail_handshake(ctx);
                PT_EXIT(pt);
            }
            *ok = false;
            PT_CONTINUE(pt);
            PT_SPAWN(pt, cpt, syn_x509_verify_pt(cpt, &ctx->crypto_scratch.x509, ok));
            if (!(*ok)) {
                (void)dtls_fail_handshake(ctx);
                PT_EXIT(pt);
            }
            PT_CONTINUE(pt);
        }
        ctx->hs_work.peer_auth_verified = true;
    }

    PT_END(pt);
}

/**
 * @brief Write 13-byte DTLS Epoch-0 plaintext handshake record header.
 * @param tx       Destination packet buffer.
 * @param body_len Handshake fragment length in bytes.
 */
static void dtls_write_epoch0_hdr(uint8_t *tx, size_t body_len)
{
    memset(tx, 0, DTLS_EPOCH0_HDR_LEN);
    tx[0] = DTLS_CONTENT_TYPE_HANDSHAKE;
    tx[1] = 0xFEU;
    tx[2] = 0xFDU;
    tx[11] = (uint8_t)((body_len >> 8U) & 0xFFU);
    tx[12] = (uint8_t)(body_len & 0xFFU);
}

/**
 * @brief Generate client ephemeral X25519 private key + random and initialize pubkey derivation.
 * @param ctx DTLS client context.
 * @return true on success, false on CSPRNG failure.
 */
static bool dtls_client_init_ecdhe(SYN_DTLS_Context *ctx)
{
    uint8_t rnd[64];
    if (syn_random_fill(rnd, sizeof(rnd)) != SYN_OK) {
        return dtls_fail_handshake(ctx);
    }
    memcpy(ctx->ecdhe_priv, rnd, 32U);
    memcpy(ctx->client_random, rnd + 32U, 32U);
    syn_secure_zero(rnd, sizeof(rnd));
    syn_x25519_clamp(ctx->ecdhe_priv);
    (void)syn_x25519_pubkey_init(&ctx->crypto_scratch.x25519, ctx->ecdhe_priv);
    return true;
}

/**
 * @brief Format and transmit 80-byte DTLS ClientHello datagram.
 * @param ctx DTLS client context with `ecdhe_pub` computed.
 * @return true if transmitted, false on transport error.
 */
static bool dtls_client_send_ch_record(SYN_DTLS_Context *ctx)
{
    uint8_t *tx = ctx->tx_buf;
    uint16_t suite_id = dtls_cipher_wire_id(ctx->config.cipher_suite);
    dtls_write_epoch0_hdr(tx, 67U);
    tx[13] = DTLS_HS_CLIENT_HELLO;
    tx[14] = (uint8_t)(suite_id >> 8U);
    tx[15] = (uint8_t)(suite_id & 0xFFU);
    memcpy(tx + 16U, ctx->client_random, 32U);
    memcpy(tx + 48U, ctx->ecdhe_pub, 32U);

    if (!syn_transport_send(ctx->underlying_transport, tx, 80U)) {
        return dtls_fail_handshake(ctx);
    }
    ctx->state = SYN_DTLS_STATE_CLIENT_HELLO_SENT;
    return true;
}

/**
 * @brief Client Stage 0 protothread: verify peer X.509, compute X25519 pubkey, send ClientHello.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS client context in `SYN_DTLS_STATE_UNINITIALIZED`.
 * @return Protothread status.
 */
static SYN_PT_Status dtls_client_stage0_send_ch_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;
    SYN_PT *cpt = &ctx->crypto_pt;
    SYN_X25519_Ctx *x25519 = &ctx->crypto_scratch.x25519;

    PT_BEGIN(pt);

    PT_SPAWN(pt, spt, dtls_peer_auth_pt(spt, ctx));
    if (ctx->state == SYN_DTLS_STATE_ERROR) {
        PT_EXIT(pt);
    }

    if (!dtls_client_init_ecdhe(ctx)) {
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, cpt, syn_x25519_pt(cpt, x25519, ctx->ecdhe_pub));
    PT_CONTINUE(pt);

    if (!dtls_client_send_ch_record(ctx)) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

/**
 * @brief Write ServerHello record header and payload into tx_buf prior to optional CV signing.
 * @param ctx DTLS server context in `SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED`.
 * @return true if `tx_buf_size` is sufficient and ServerHello header was written.
 */
static bool dtls_server_prep_flight2(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) ? DTLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 67U + cv_len + hash_len;
    if (ctx->tx_buf_size < (DTLS_EPOCH0_HDR_LEN + body_len)) {
        return dtls_fail_handshake(ctx);
    }

    uint16_t exp_suite = dtls_cipher_wire_id(ctx->config.cipher_suite);
    uint8_t *tx = ctx->tx_buf;
    dtls_write_epoch0_hdr(tx, body_len);
    tx[13] = DTLS_HS_SERVER_HELLO;
    tx[14] = (uint8_t)(exp_suite >> 8U);
    tx[15] = (uint8_t)(exp_suite & 0xFFU);
    memcpy(tx + 16U, ctx->server_random, 32U);
    memcpy(tx + 48U, ctx->ecdhe_pub, 32U);
    return true;
}

/**
 * @brief Append staged ServerFinished MAC (`hs_work.ks_salt`) to Flight 2 buffer and transmit.
 * @param ctx DTLS server context.
 * @return true if Flight 2 datagram transmitted, false on transport failure.
 */
static bool dtls_server_finish_and_send_flight2(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) ? DTLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 67U + cv_len + hash_len;
    uint8_t *fin = ctx->tx_buf + 80U + cv_len;

    memcpy(fin, ctx->hs_work.ks_salt, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    dtls_chain_transcript(ctx, ctx->transcript_digest, fin, hash_len);
    if (!syn_transport_send(ctx->underlying_transport, ctx->tx_buf,
                            DTLS_EPOCH0_HDR_LEN + body_len)) {
        return dtls_fail_handshake(ctx);
    }
    return true;
}

/**
 * @brief Server Flight 2 protothread: sign optional CV, send ServerHello flight, derive app keys.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS server context in `SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED`.
 * @return Protothread status.
 */
static SYN_PT_Status dtls_server_send_flight2_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;

    PT_BEGIN(pt);

    if (!dtls_server_prep_flight2(ctx)) {
        PT_EXIT(pt);
    }

    if (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) {
        PT_SPAWN(pt, spt, dtls_build_local_cv_pt(spt, ctx, true, ctx->tx_buf + 80U));
        if (!ctx->hs_work.crypto_ok) {
            (void)dtls_fail_handshake(ctx);
            PT_EXIT(pt);
        }
    }

    PT_SPAWN(
        pt, spt,
        dtls_compute_finished_pt(spt, ctx, ctx->server_handshake_secret, ctx->transcript_digest));
    PT_CONTINUE(pt);

    if (!dtls_server_finish_and_send_flight2(ctx)) {
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, spt, dtls_derive_app_keys_pt(spt, ctx));
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        ctx->state = SYN_DTLS_STATE_FINISHED_SENT;
    } else {
        dtls_scrub_handshake_secrets(ctx);
        ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    }
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;

    PT_END(pt);
}

/**
 * @brief Poll for an 80-byte ClientHello datagram and initialize server ephemeral X25519 pubkey.
 * @param ctx DTLS server context in `SYN_DTLS_STATE_UNINITIALIZED`.
 * @return true if valid ClientHello arrived and X25519 pubkey derivation was initialized.
 */
static bool dtls_server_poll_ch(SYN_DTLS_Context *ctx)
{
    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    static const uint8_t ch_hdr[14] = {DTLS_CONTENT_TYPE_HANDSHAKE,
                                       0xFEU,
                                       0xFDU,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       0x00U,
                                       67U,
                                       DTLS_HS_CLIENT_HELLO};
    const uint8_t *rx = ctx->rx_buf;
    uint16_t exp_suite = dtls_cipher_wire_id(ctx->config.cipher_suite);

    if ((rx_len != 80U) || (memcmp(rx, ch_hdr, sizeof(ch_hdr)) != 0) ||
        ((((uint16_t)rx[14] << 8U) | (uint16_t)rx[15]) != exp_suite)) {
        return false;
    }

    uint8_t rnd[64];
    if (syn_random_fill(rnd, sizeof(rnd)) != SYN_OK) {
        return dtls_fail_handshake(ctx);
    }
    memcpy(ctx->client_random, rx + 16U, 32U);
    memcpy(ctx->peer_ecdhe_pub, rx + 48U, 32U);
    memcpy(ctx->ecdhe_priv, rnd, 32U);
    memcpy(ctx->server_random, rnd + 32U, 32U);
    syn_secure_zero(rnd, sizeof(rnd));
    syn_x25519_clamp(ctx->ecdhe_priv);
    (void)syn_x25519_pubkey_init(&ctx->crypto_scratch.x25519, ctx->ecdhe_priv);
    return true;
}

/**
 * @brief Server Stage 1 protothread: verify peer X.509, await ClientHello, run 2x X25519 + HKDF.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS server context in `SYN_DTLS_STATE_UNINITIALIZED`.
 * @return Protothread status (PT_IDLE awaiting RX, PT_EXITED, PT_MORE, or PT_ENDED).
 */
static SYN_PT_Status dtls_server_stage1_recv_ch_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;
    SYN_PT *cpt = &ctx->crypto_pt;
    SYN_X25519_Ctx *x25519 = &ctx->crypto_scratch.x25519;

    PT_BEGIN(pt);

    PT_SPAWN(pt, spt, dtls_peer_auth_pt(spt, ctx));
    if (ctx->state == SYN_DTLS_STATE_ERROR) {
        PT_EXIT(pt);
    }

    if (!dtls_server_poll_ch(ctx)) {
        if (ctx->state == SYN_DTLS_STATE_ERROR) {
            PT_EXIT(pt);
        }
        PT_RESTART(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, cpt, syn_x25519_pt(cpt, x25519, ctx->ecdhe_pub));
    PT_CONTINUE(pt);

    (void)syn_x25519_init(x25519, ctx->ecdhe_priv, ctx->peer_ecdhe_pub);
    PT_SPAWN(pt, cpt, syn_x25519_pt(cpt, x25519, ctx->hs_work.ecdhe_secret));
    PT_CONTINUE(pt);

    PT_SPAWN(pt, spt, dtls_derive_hs_secrets_pt(spt, ctx));
    if (!ctx->hs_work.crypto_ok) {
        PT_RESTART(pt);
    }
    ctx->state = SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED;

    PT_END(pt);
}

/**
 * @brief Append staged ClientFinished MAC (`hs_work.ks_salt`) and transmit DTLS Flight 3.
 * @param ctx DTLS client context.
 * @return true if Flight 3 datagram transmitted, false on transport failure.
 */
static bool dtls_finish_and_send_client_mtls(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = DTLS_CERT_VERIFY_LEN + 4U + hash_len;
    uint8_t *tx = ctx->tx_buf;

    dtls_write_epoch0_hdr(tx, body_len);
    uint8_t *fin = tx + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN;
    fin[0] = DTLS_HS_CLIENT_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    memcpy(fin + 4U, ctx->hs_work.ks_salt, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    if (!syn_transport_send(ctx->underlying_transport, tx, DTLS_EPOCH0_HDR_LEN + body_len)) {
        return dtls_fail_handshake(ctx);
    }
    return true;
}

/**
 * @brief Poll transport for a structurally valid mTLS Flight 3 datagram and snapshot transcript.
 * @param ctx DTLS server context in `SYN_DTLS_STATE_FINISHED_SENT`.
 * @return true if a candidate Flight 3 datagram arrived in `rx_buf`.
 */
static bool dtls_server_poll_mtls_flight3(SYN_DTLS_Context *ctx)
{
    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = DTLS_CERT_VERIFY_LEN + 4U + hash_len;
    uint8_t exp_hdr[13] = {DTLS_CONTENT_TYPE_HANDSHAKE,
                           0xFEU,
                           0xFDU,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           0x00U,
                           (uint8_t)body_len};
    uint8_t fin_hdr[4] = {DTLS_HS_CLIENT_FINISHED, 0x00U, 0x00U, (uint8_t)hash_len};
    const uint8_t *rx = ctx->rx_buf;

    if ((rx_len != (DTLS_EPOCH0_HDR_LEN + body_len)) ||
        (memcmp(rx, exp_hdr, sizeof(exp_hdr)) != 0) ||
        (memcmp(rx + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN, fin_hdr, 4U) != 0)) {
        return false;
    }

    memcpy(ctx->hs_work.tr_cand, ctx->transcript_digest, SYN_DTLS_SECRET_LEN);
    return true;
}

/**
 * @brief Verify staged ClientFinished MAC (`hs_work.ks_salt`) and establish session on match.
 * @param ctx DTLS server context in `SYN_DTLS_STATE_FINISHED_SENT`.
 * @return true if ClientFinished MAC matched and session reached `SYN_DTLS_STATE_ESTABLISHED`.
 */
static bool dtls_server_verify_client_fin(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    const uint8_t *mac = ctx->rx_buf + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN + 4U;

    bool ok = dtls_verify_mac_ct(ctx->hs_work.ks_salt, mac, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    if (!ok) {
        return false;
    }

    memcpy(ctx->transcript_digest, ctx->hs_work.tr_cand, SYN_DTLS_SECRET_LEN);
    dtls_scrub_handshake_secrets(ctx);
    ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    return true;
}

/**
 * @brief Server Stage 3 (mTLS) protothread: verify client Flight 3; silently drop forged UDP.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS server context in `SYN_DTLS_STATE_FINISHED_SENT`.
 * @return Protothread status (`PT_IDLE` while awaiting/dropping UDP, `PT_MORE`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_server_stage3_recv_mtls_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;
    const uint8_t *rx_cv = ctx->rx_buf + DTLS_EPOCH0_HDR_LEN;

    PT_BEGIN(pt);

    if (!dtls_server_poll_mtls_flight3(ctx)) {
        PT_RESTART(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, spt, dtls_check_peer_cv_pt(spt, ctx, false, ctx->hs_work.tr_cand, rx_cv));
    if (!ctx->hs_work.crypto_ok) {
        PT_RESTART(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(
        pt, spt,
        dtls_compute_finished_pt(spt, ctx, ctx->client_handshake_secret, ctx->hs_work.tr_cand));
    PT_CONTINUE(pt);
    if (!dtls_server_verify_client_fin(ctx)) {
        PT_RESTART(pt);
    }

    PT_END(pt);
}

/**
 * @brief Poll transport for a valid ServerHello flight and initialize X25519 ECDHE derivation.
 * @param ctx DTLS client context in `SYN_DTLS_STATE_CLIENT_HELLO_SENT`.
 * @return true if a structurally valid ServerHello arrived and X25519 was initialized.
 */
static bool dtls_client_poll_sh(SYN_DTLS_Context *ctx)
{
    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) ? DTLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 67U + cv_len + hash_len;
    uint8_t sh_hdr[14] = {DTLS_CONTENT_TYPE_HANDSHAKE,
                          0xFEU,
                          0xFDU,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          0x00U,
                          (uint8_t)body_len,
                          DTLS_HS_SERVER_HELLO};
    const uint8_t *rx = ctx->rx_buf;
    uint16_t exp_suite = dtls_cipher_wire_id(ctx->config.cipher_suite);

    if ((rx_len != (DTLS_EPOCH0_HDR_LEN + body_len)) || (memcmp(rx, sh_hdr, sizeof(sh_hdr)) != 0) ||
        ((((uint16_t)rx[14] << 8U) | (uint16_t)rx[15]) != exp_suite)) {
        return false;
    }

    memcpy(ctx->server_random, rx + 16U, 32U);
    memcpy(ctx->peer_ecdhe_pub, rx + 48U, 32U);
    (void)syn_x25519_init(&ctx->crypto_scratch.x25519, ctx->ecdhe_priv, ctx->peer_ecdhe_pub);
    return true;
}

/**
 * @brief Client Stage 2 protothread: await ServerHello, run X25519 ECDHE, derive handshake keys.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS client context in `SYN_DTLS_STATE_CLIENT_HELLO_SENT`.
 * @return Protothread status (`PT_IDLE` while awaiting/dropping UDP, `PT_MORE`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_client_stage2_recv_sh_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *cpt = &ctx->crypto_pt;
    SYN_X25519_Ctx *x25519 = &ctx->crypto_scratch.x25519;

    PT_BEGIN(pt);

    if (!dtls_client_poll_sh(ctx)) {
        PT_RESTART(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, cpt, syn_x25519_pt(cpt, x25519, ctx->hs_work.ecdhe_secret));
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, dtls_derive_hs_secrets_pt(&ctx->sub_pt, ctx));
    if (!ctx->hs_work.crypto_ok) {
        dtls_rollback_forged_sh(ctx);
        PT_RESTART(pt);
    }
    ctx->state = SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED;

    PT_END(pt);
}

/**
 * @brief Client Stage 3 protothread: verify Server CertificateVerify; rollback on forged UDP.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS client context in `SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED`.
 * @return Protothread status (`PT_IDLE` on forged rollback, `PT_MORE`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_client_stage3_verify_cv_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;
    const uint8_t *rx_cv = ctx->rx_buf + 80U;

    PT_BEGIN(pt);

    if (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) {
        PT_SPAWN(pt, spt, dtls_check_peer_cv_pt(spt, ctx, true, ctx->transcript_digest, rx_cv));
        if (!ctx->hs_work.crypto_ok) {
            dtls_rollback_forged_sh(ctx);
            PT_RESTART(pt);
        }
    }
    ctx->state = SYN_DTLS_STATE_CERTIFICATE_VERIFIED;

    PT_END(pt);
}

/**
 * @brief Verify staged ServerFinished MAC (`hs_work.ks_salt`) and chain into client transcript.
 * @param ctx DTLS client context in `SYN_DTLS_STATE_CERTIFICATE_VERIFIED`.
 * @return true if ServerFinished MAC matched, false if forged/corrupted.
 */
static bool dtls_client_verify_server_fin(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t cv_len = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK) ? DTLS_CERT_VERIFY_LEN : 0U;
    const uint8_t *fin = ctx->rx_buf + 80U + cv_len;

    bool ok = dtls_verify_mac_ct(ctx->hs_work.ks_salt, fin, hash_len);
    syn_secure_zero(ctx->hs_work.ks_salt, sizeof(ctx->hs_work.ks_salt));
    if (!ok) {
        return false;
    }

    dtls_chain_transcript(ctx, ctx->transcript_digest, fin, hash_len);
    return true;
}

/**
 * @brief Client Stage 4 protothread: verify ServerFinished MAC, then derive application keys.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS client context in `SYN_DTLS_STATE_CERTIFICATE_VERIFIED`.
 * @return Protothread status (`PT_IDLE` on forged rollback, `PT_MORE`, or `PT_ENDED`).
 */
static SYN_PT_Status dtls_client_stage4_verify_fin_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt,
             dtls_compute_finished_pt(&ctx->sub_pt, ctx, ctx->server_handshake_secret,
                                      ctx->transcript_digest));
    PT_CONTINUE(pt);

    if (!dtls_client_verify_server_fin(ctx)) {
        dtls_rollback_forged_sh(ctx);
        PT_RESTART(pt);
    }
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, dtls_derive_app_keys_pt(&ctx->sub_pt, ctx));
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        ctx->state = SYN_DTLS_STATE_SERVER_HELLO_RECEIVED;
    } else {
        dtls_scrub_handshake_secrets(ctx);
        ctx->state = SYN_DTLS_STATE_ESTABLISHED;
        ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;
    }

    PT_END(pt);
}

/**
 * @brief Client Stage 5 (mTLS) protothread: sign Client CertificateVerify and send Flight 3.
 * @param pt  Flight protothread control block (`&ctx->flight_pt`).
 * @param ctx DTLS client context in `SYN_DTLS_STATE_SERVER_HELLO_RECEIVED`.
 * @return Protothread status.
 */
static SYN_PT_Status dtls_client_stage5_send_mtls_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    SYN_PT *spt = &ctx->sub_pt;

    PT_BEGIN(pt);

    PT_SPAWN(pt, spt, dtls_build_local_cv_pt(spt, ctx, false, ctx->tx_buf + DTLS_EPOCH0_HDR_LEN));
    if (!ctx->hs_work.crypto_ok) {
        (void)dtls_fail_handshake(ctx);
        PT_EXIT(pt);
    }

    PT_SPAWN(
        pt, spt,
        dtls_compute_finished_pt(spt, ctx, ctx->client_handshake_secret, ctx->transcript_digest));
    PT_CONTINUE(pt);

    if (!dtls_finish_and_send_client_mtls(ctx)) {
        PT_EXIT(pt);
    }
    dtls_scrub_handshake_secrets(ctx);
    ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;

    PT_END(pt);
}

/**
 * @brief Format DTLS 1.3 unified record header into tx_buf.
 * @param hdr        Destination header buffer.
 * @param epoch      Active DTLS epoch.
 * @param seq        48-bit outgoing sequence number.
 * @param seq_is_16  true to encode 16-bit sequence LSBs, false for 8-bit.
 * @param ct_tag_len Ciphertext + tag length in bytes.
 */
static void dtls_write_unified_header(uint8_t *hdr, SYN_DTLS_Epoch epoch, uint64_t seq,
                                      bool seq_is_16, size_t ct_tag_len)
{
    uint8_t flags = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT |
                    ((uint8_t)epoch & SYN_DTLS_UNIFIED_EPOCH_MASK);
    size_t offset = 1U;
    if (seq_is_16) {
        flags |= SYN_DTLS_UNIFIED_SEQ_16BIT;
        hdr[offset++] = (uint8_t)((seq >> 8U) & 0xFFU);
        hdr[offset++] = (uint8_t)(seq & 0xFFU);
    } else {
        hdr[offset++] = (uint8_t)(seq & 0xFFU);
    }
    hdr[0] = flags;
    hdr[offset++] = (uint8_t)((ct_tag_len >> 8U) & 0xFFU);
    hdr[offset] = (uint8_t)(ct_tag_len & 0xFFU);
}

/**
 * @brief Copy single decrypted datagram payload to caller buffer without cross-datagram spilling.
 * @param ctx         DTLS context pointer.
 * @param pt          Decrypted payload buffer.
 * @param payload_len Decrypted payload length.
 * @param seq         Verified 48-bit packet sequence number.
 * @param data        Caller output buffer.
 * @param max_len     Caller output buffer capacity.
 * @param out_len     [out] Bytes copied to data.
 */
static void dtls_commit_decrypted_record(SYN_DTLS_Context *ctx, const uint8_t *pt,
                                         size_t payload_len, uint64_t seq, uint8_t *data,
                                         size_t max_len, size_t *out_len)
{
    size_t copy_len = (payload_len > max_len) ? max_len : payload_len;
    if (copy_len > 0U) {
        memcpy(data, pt, copy_len);
    }
    *out_len = copy_len;
    ctx->server_seq_num = seq + 1ULL;
    syn_dtls_replay_update(&ctx->replay_window, seq);
}

/**
 * @brief Strip optional trailing 0x00 padding and verify DTLS inner ContentType is 0x17.
 * @param pt_buf         Decrypted inner plaintext buffer.
 * @param pt_with_ct_len Total decrypted inner bytes.
 * @param app_len_out    [out] Application payload length excluding inner ContentType.
 * @return true if trailing non-zero ContentType byte is 0x17.
 */
static bool dtls_strip_inner_content_type(const uint8_t *pt_buf, size_t pt_with_ct_len,
                                          size_t *app_len_out)
{
    size_t scan = pt_with_ct_len;
    while (scan > 0U && pt_buf[scan - 1U] == 0x00U) {
        scan--;
    }
    if (scan == 0U || pt_buf[scan - 1U] != DTLS_CONTENT_TYPE_APP_DATA) {
        return false;
    }
    *app_len_out = scan - 1U;
    return true;
}

/**
 * @brief Dispatch active server handshake flight protothread based on `ctx->state`.
 * @param ctx DTLS server context.
 * @return Active flight protothread status.
 */
static SYN_PT_Status dtls_dispatch_server_flight_pt(SYN_DTLS_Context *ctx)
{
    if (ctx->state == SYN_DTLS_STATE_UNINITIALIZED) {
        return dtls_server_stage1_recv_ch_pt(&ctx->flight_pt, ctx);
    }
    if (ctx->state == SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED) {
        return dtls_server_send_flight2_pt(&ctx->flight_pt, ctx);
    }
    return dtls_server_stage3_recv_mtls_pt(&ctx->flight_pt, ctx);
}

/**
 * @brief Dispatch active client handshake flight protothread based on `ctx->state`.
 * @param ctx DTLS client context.
 * @return Active flight protothread status.
 */
static SYN_PT_Status dtls_dispatch_client_flight_pt(SYN_DTLS_Context *ctx)
{
    if (ctx->state == SYN_DTLS_STATE_UNINITIALIZED) {
        return dtls_client_stage0_send_ch_pt(&ctx->flight_pt, ctx);
    }
    if (ctx->state == SYN_DTLS_STATE_CLIENT_HELLO_SENT) {
        return dtls_client_stage2_recv_sh_pt(&ctx->flight_pt, ctx);
    }
    if (ctx->state == SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED) {
        return dtls_client_stage3_verify_cv_pt(&ctx->flight_pt, ctx);
    }
    if (ctx->state == SYN_DTLS_STATE_CERTIFICATE_VERIFIED) {
        return dtls_client_stage4_verify_fin_pt(&ctx->flight_pt, ctx);
    }
    return dtls_client_stage5_send_mtls_pt(&ctx->flight_pt, ctx);
}

/**
 * @brief Dispatch client or server flight protothread for current handshake state.
 * @param ctx DTLS context.
 * @return Active flight protothread status.
 */
static SYN_PT_Status dtls_dispatch_flight_pt(SYN_DTLS_Context *ctx)
{
    if (ctx->config.is_server) {
        return dtls_dispatch_server_flight_pt(ctx);
    }
    return dtls_dispatch_client_flight_pt(ctx);
}

/** @endcond */

void syn_dtls_bind_transport(SYN_DTLS_Context *dtls_ctx, SYN_Transport *tr_out)
{
    if (dtls_ctx == NULL || tr_out == NULL) {
        return;
    }
    tr_out->send = dtls_transport_send_cb;
    tr_out->recv = dtls_transport_recv_cb;
    tr_out->has_data =
        (dtls_ctx->underlying_transport != NULL && dtls_ctx->underlying_transport->has_data != NULL)
            ? dtls_transport_has_data_cb
            : NULL;
    tr_out->ctx = dtls_ctx;
}

/* ── Main DTLS API ───────────────────────────────────────────────────────── */

/**
 * @brief Compute minimum RX/TX handshake datagram buffer capacity for @p config.
 * @param config DTLS configuration pointer.
 * @return Minimum buffer size in bytes (140 PSK, 184 SHA-256 CV, 200 SHA-384 CV).
 */
static size_t dtls_min_buf_size(const SYN_DTLS_Config *config)
{
    if (config->mode == SYN_DTLS_AUTH_MODE_PSK) {
        return 140U;
    }
    return (config->cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) ? 200U : 184U;
}

bool syn_dtls_init(SYN_DTLS_Context *ctx, const SYN_DTLS_Config *config, SYN_Transport *transport,
                   uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    if (ctx == NULL || config == NULL || transport == NULL || rx_buf == NULL || tx_buf == NULL) {
        return false;
    }
    size_t min_buf = dtls_min_buf_size(config);
    if (rx_buf_size < min_buf || tx_buf_size < min_buf) {
        return false;
    }

    memset(ctx, 0, sizeof(*ctx));
    if (syn_random_fill(ctx->ecdhe_priv, 32U) != SYN_OK) {
        return false;
    }

    ctx->config = *config;
    ctx->underlying_transport = transport;
    ctx->rx_buf = rx_buf;
    ctx->rx_buf_size = rx_buf_size;
    ctx->tx_buf = tx_buf;
    ctx->tx_buf_size = tx_buf_size;
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;

    PT_INIT(&ctx->pt);
    PT_INIT(&ctx->hs_pt);
    PT_INIT(&ctx->flight_pt);
    PT_INIT(&ctx->sub_pt);
    PT_INIT(&ctx->crypto_pt);
    return true;
}

SYN_PT_Status syn_dtls_handshake_pt(SYN_PT *pt, SYN_DTLS_Context *ctx)
{
    if ((pt == NULL) || (ctx == NULL)) {
        return PT_EXITED;
    }
    if (ctx->state == SYN_DTLS_STATE_ERROR) {
        PT_INIT(pt);
        return PT_EXITED;
    }

    SYN_PT_Status st = PT_IDLE;

    PT_BEGIN(pt);

    while (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        st = dtls_dispatch_flight_pt(ctx);
        if (ctx->state == SYN_DTLS_STATE_ERROR) {
            PT_EXIT(pt);
        }
        if (st == PT_IDLE) {
            PT_PASS(pt);
        } else if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
            PT_CONTINUE(pt);
        }
    }

    PT_END(pt);
}

bool syn_dtls_handshake(SYN_DTLS_Context *ctx)
{
    if (ctx == NULL) {
        return false;
    }
    SYN_PT_Status st = syn_dtls_handshake_pt(&ctx->hs_pt, ctx);
    while (st == PT_MORE) {
        st = syn_dtls_handshake_pt(&ctx->hs_pt, ctx);
    }
    return st != PT_EXITED;
}

/** @cond INTERNAL */
/**
 * @brief Encrypt and transmit a single DTLS 1.3 application datagram, advancing seq only on send.
 * @param ctx  Established DTLS context.
 * @param data Plaintext payload buffer.
 * @param len  Plaintext byte count (> 0 and <= SYN_DTLS_RECORD_MAX_PAYLOAD).
 * @return true if datagram encrypted and accepted by underlying transport.
 */
static bool dtls_transmit_app_record(SYN_DTLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx->client_seq_num > 0xFFFFFFFFFFFFULL) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    size_t tag_sz = dtls_tag_len(ctx);
    uint64_t seq = ctx->client_seq_num;
    bool seq_is_16 = (seq > 0xFFULL);
    size_t seq_bytes = seq_is_16 ? 2U : 1U;
    size_t hdr_len = 1U + seq_bytes + 2U;
    size_t overhead = hdr_len + 1U + tag_sz;

    if (ctx->tx_buf_size < overhead || len > ctx->tx_buf_size - overhead) {
        return false;
    }

    size_t pt_with_ct_len = len + 1U;
    size_t ct_tag_len = pt_with_ct_len + tag_sz;
    size_t total_datagram_len = hdr_len + ct_tag_len;

    uint8_t *hdr = ctx->tx_buf;
    uint8_t *out_ct = ctx->tx_buf + hdr_len;
    uint8_t *out_tag = out_ct + pt_with_ct_len;
    memmove(out_ct, data, len);
    out_ct[len] = DTLS_CONTENT_TYPE_APP_DATA;
    dtls_write_unified_header(hdr, ctx->epoch, seq, seq_is_16, ct_tag_len);

    const uint8_t *tx_key = ctx->config.is_server ? ctx->server_app_key : ctx->client_app_key;
    const uint8_t *tx_iv = ctx->config.is_server ? ctx->server_app_iv : ctx->client_app_iv;

    uint8_t nonce[12];
    construct_dtls13_nonce(tx_iv, ctx->epoch, seq, nonce);
    dtls_record_encrypt(ctx, tx_key, nonce, hdr, hdr_len, out_ct, pt_with_ct_len, out_ct, out_tag);
    syn_secure_zero(nonce, sizeof(nonce));

    if (!syn_transport_send(ctx->underlying_transport, ctx->tx_buf, total_datagram_len)) {
        return false;
    }
    ctx->client_seq_num++;
    return true;
}
/** @endcond */

bool syn_dtls_send(SYN_DTLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx == NULL || ctx->state == SYN_DTLS_STATE_ERROR || (data == NULL && len > 0U)) {
        return false;
    }

    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx) || ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
            return false;
        }
    }

    if (len == 0U) {
        return true;
    }
    if (len > SYN_DTLS_RECORD_MAX_PAYLOAD) {
        return false;
    }

    return dtls_transmit_app_record(ctx, data, len);
}

/**
 * @brief Validate, decrypt, and commit an encrypted DTLS 1.3 application record.
 * @param ctx     DTLS session context.
 * @param rx_len  Total received datagram byte length.
 * @param data    Caller plaintext output buffer.
 * @param max_len Maximum caller buffer length.
 * @param out_len Output decrypted byte count.
 * @return true on authentic record decryption, false otherwise.
 */
static bool dtls_decrypt_rx_record(SYN_DTLS_Context *ctx, size_t rx_len, uint8_t *data,
                                   size_t max_len, size_t *out_len)
{
    SYN_DTLS_Epoch rec_epoch = SYN_DTLS_EPOCH_PLAINTEXT;
    uint64_t seq = 0U;
    size_t hdr_len = 0U;
    size_t pt_with_ct_len = 0U;
    if (!dtls_parse_unified_header(ctx, rx_len, &rec_epoch, &seq, &hdr_len, &pt_with_ct_len) ||
        rec_epoch != ctx->epoch || !syn_dtls_replay_check(&ctx->replay_window, seq)) {
        return false;
    }

    uint8_t *ct = ctx->rx_buf + hdr_len;
    uint8_t *tag = ct + pt_with_ct_len;

    const uint8_t *rx_key = ctx->config.is_server ? ctx->client_app_key : ctx->server_app_key;
    const uint8_t *rx_iv = ctx->config.is_server ? ctx->client_app_iv : ctx->server_app_iv;

    uint8_t nonce[12];
    construct_dtls13_nonce(rx_iv, rec_epoch, seq, nonce);

    bool ok =
        dtls_record_decrypt(ctx, rx_key, nonce, ctx->rx_buf, hdr_len, ct, pt_with_ct_len, tag, ct);
    syn_secure_zero(nonce, sizeof(nonce));

    size_t app_len = 0U;
    if (!ok || !dtls_strip_inner_content_type(ct, pt_with_ct_len, &app_len)) {
        syn_secure_zero(ct, pt_with_ct_len);
        return false;
    }

    dtls_commit_decrypted_record(ctx, ct, app_len, seq, data, max_len, out_len);
    return true;
}

bool syn_dtls_recv(SYN_DTLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len)
{
    if (ctx == NULL || data == NULL || out_len == NULL || ctx->state == SYN_DTLS_STATE_ERROR) {
        return false;
    }

    *out_len = 0U;

    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx) || ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
            return false;
        }
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    return dtls_decrypt_rx_record(ctx, rx_len, data, max_len, out_len);
}

SYN_PT_Status syn_dtls_task(SYN_PT *pt, SYN_Task *task)
{
    if (task == NULL || pt == NULL) {
        return PT_EXITED;
    }
    SYN_DTLS_Context *ctx = (SYN_DTLS_Context *)task->user_data;
    if (ctx == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    PT_SPAWN_IF_NEEDED(pt, task, &ctx->hs_pt, syn_dtls_handshake_pt(&ctx->hs_pt, ctx));
    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        PT_EXIT(pt);
    }

    PT_WAIT_UNTIL(pt, ctx->state != SYN_DTLS_STATE_ESTABLISHED);

    PT_END(pt);
}

#endif /* SYN_DTLS_ENABLED */
