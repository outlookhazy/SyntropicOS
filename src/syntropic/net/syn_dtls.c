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

/**
 * @brief Constant-time check that a 32-byte X25519 shared secret is non-zero.
 * @param b 32-byte shared point.
 * @return true if at least one byte is non-zero.
 */
static bool dtls_is_nonzero_32(const uint8_t b[32])
{
    uint8_t acc = 0U;
    for (size_t i = 0U; i < 32U; i++) {
        acc |= b[i];
    }
    return acc != 0U;
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
 * @brief Derive DTLS 1.3 handshake secrets and master secret after ServerHello.
 * @param ctx DTLS context pointer.
 * @return true if X25519 shared secret is non-zero and handshake secrets were derived.
 */
static bool dtls_derive_hs_secrets(SYN_DTLS_Context *ctx)
{
    static const uint8_t zero_salt[SYN_DTLS_SECRET_LEN] = {0};
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    uint8_t buf1[SYN_DTLS_SECRET_LEN];
    uint8_t buf2[SYN_DTLS_SECRET_LEN];
    uint8_t ecdhe_shared[32];

    memset(buf1, 0, sizeof(buf1));
    size_t copy_len = hash_len;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_PSK) {
        copy_len =
            (ctx->config.psk_secret_len > sizeof(buf1)) ? sizeof(buf1) : ctx->config.psk_secret_len;
        memcpy(buf1, ctx->config.psk_secret, copy_len);
    }

    syn_x25519(ecdhe_shared, ctx->ecdhe_priv, ctx->peer_ecdhe_pub);
    if (!dtls_is_nonzero_32(ecdhe_shared)) {
        syn_secure_zero(buf1, sizeof(buf1));
        syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
        return false;
    }

    dtls_compute_transcript(ctx, is_sha384, ctx->transcript_digest);

    if (is_sha384) {
        syn_hkdf_sha384_extract(zero_salt, hash_len, buf1, copy_len, buf2);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "derived", 7, DTLS_EMPTY_SHA384, hash_len,
                                     buf1, hash_len);
        syn_hkdf_sha384_extract(buf1, hash_len, ecdhe_shared, 32U, buf2);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "c hs traffic", 12, ctx->transcript_digest,
                                     hash_len, ctx->client_handshake_secret, hash_len);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "s hs traffic", 12, ctx->transcript_digest,
                                     hash_len, ctx->server_handshake_secret, hash_len);
        syn_hkdf_sha384_expand_label(buf2, hash_len, "derived", 7, DTLS_EMPTY_SHA384, hash_len,
                                     buf1, hash_len);
        syn_hkdf_sha384_extract(buf1, hash_len, zero_salt, hash_len, ctx->master_secret);
    } else {
        syn_hkdf_extract(zero_salt, hash_len, buf1, copy_len, buf2);
        syn_hkdf_expand_label(buf2, hash_len, "derived", 7, DTLS_EMPTY_SHA256, hash_len, buf1,
                              hash_len);
        syn_hkdf_extract(buf1, hash_len, ecdhe_shared, 32U, buf2);
        syn_hkdf_expand_label(buf2, hash_len, "c hs traffic", 12, ctx->transcript_digest, hash_len,
                              ctx->client_handshake_secret, hash_len);
        syn_hkdf_expand_label(buf2, hash_len, "s hs traffic", 12, ctx->transcript_digest, hash_len,
                              ctx->server_handshake_secret, hash_len);
        syn_hkdf_expand_label(buf2, hash_len, "derived", 7, DTLS_EMPTY_SHA256, hash_len, buf1,
                              hash_len);
        syn_hkdf_extract(buf1, hash_len, zero_salt, hash_len, ctx->master_secret);
    }

    syn_secure_zero(buf1, sizeof(buf1));
    syn_secure_zero(buf2, sizeof(buf2));
    syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
    return true;
}

/**
 * @brief Wipe ephemeral X25519 private key and intermediate handshake secrets once established.
 * @param ctx DTLS context pointer.
 */
static void dtls_scrub_handshake_secrets(SYN_DTLS_Context *ctx)
{
    syn_secure_zero(ctx->ecdhe_priv, sizeof(ctx->ecdhe_priv));
    syn_secure_zero(ctx->client_handshake_secret, sizeof(ctx->client_handshake_secret));
    syn_secure_zero(ctx->server_handshake_secret, sizeof(ctx->server_handshake_secret));
    syn_secure_zero(ctx->master_secret, sizeof(ctx->master_secret));
}

/**
 * @brief Derive DTLS 1.3 application traffic secrets, keys, and IVs after ServerFinished.
 * @param ctx DTLS context pointer.
 */
static void dtls_derive_app_keys(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    if (is_sha384) {
        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12,
                                     ctx->transcript_digest, hash_len, ctx->client_app_secret,
                                     hash_len);
        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12,
                                     ctx->transcript_digest, hash_len, ctx->server_app_secret,
                                     hash_len);
    } else {
        syn_hkdf_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12,
                              ctx->transcript_digest, hash_len, ctx->client_app_secret, hash_len);
        syn_hkdf_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12,
                              ctx->transcript_digest, hash_len, ctx->server_app_secret, hash_len);
    }

    size_t key_len = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256 ||
                      ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256 ||
                      ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256)
                         ? 16U
                         : 32U;

    dtls_derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "key", ctx->client_app_key, key_len);
    dtls_derive_traffic_key_or_iv(ctx, ctx->client_app_secret, "iv", ctx->client_app_iv, 12);
    dtls_derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "key", ctx->server_app_key, key_len);
    dtls_derive_traffic_key_or_iv(ctx, ctx->server_app_secret, "iv", ctx->server_app_iv, 12);
}

/**
 * @brief Compute DTLS 1.3 Finished verify_data MAC over a transcript digest.
 * @param ctx       DTLS context pointer.
 * @param hs_secret Base handshake secret (server or client).
 * @param tr_digest Transcript digest bytes.
 * @param out_mac   [out] Verify data output buffer (32 or 48 bytes).
 */
static void dtls_compute_finished(const SYN_DTLS_Context *ctx, const uint8_t *hs_secret,
                                  const uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                  uint8_t out_mac[SYN_DTLS_SECRET_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t fin_key[SYN_DTLS_SECRET_LEN];

    if (is_sha384) {
        syn_hkdf_sha384_expand_label(hs_secret, hash_len, "finished", 8U, NULL, 0U, fin_key,
                                     hash_len);
        syn_hkdf_sha384_extract(fin_key, hash_len, tr_digest, hash_len, out_mac);
    } else {
        syn_hkdf_expand_label(hs_secret, hash_len, "finished", 8U, NULL, 0U, fin_key, hash_len);
        syn_hkdf_extract(fin_key, hash_len, tr_digest, hash_len, out_mac);
    }

    syn_secure_zero(fin_key, sizeof(fin_key));
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
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        (void)syn_aes_gcm_encrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag, 16U);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        (void)syn_aes_ccm_encrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, in, in_len,
                                  out_ct, out_tag, 8U);
    } else {
        syn_aead_encrypt(key, nonce, aad, aad_len, in, in_len, out_ct, out_tag);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
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
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        (void)syn_aes_gcm_init(&ctx->crypto_scratch.gcm, key, 32U);
        ok = (syn_aes_gcm_decrypt(&ctx->crypto_scratch.gcm, nonce, 12U, aad, aad_len, ct, ct_len,
                                  out_pt, tag) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, ct, ct_len,
                                  tag, 16U, out_pt) == SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        (void)syn_aes_init(&ctx->crypto_scratch.aes, key, 16U);
        ok = (syn_aes_ccm_decrypt(&ctx->crypto_scratch.aes, nonce, 12U, aad, aad_len, ct, ct_len,
                                  tag, 8U, out_pt) == SYN_OK);
    } else {
        ok = syn_aead_decrypt(key, nonce, aad, aad_len, ct, ct_len, tag, out_pt);
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
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
 * @brief Build and sign a 72-byte DTLS 1.3 CertificateVerify message.
 * @param ctx          DTLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param algo         Local signing key algorithm (Ed25519 or ECDSA-P256).
 * @param tr_digest    Transcript digest bytes.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer.
 * @return true if signature succeeded.
 */
static bool dtls_sign_cert_verify(const SYN_DTLS_Context *ctx, bool is_server_cv,
                                  SYN_X509_Algo algo, const uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                  uint8_t out_cv[DTLS_CERT_VERIFY_LEN])
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t cv_msg[146];
    size_t cv_len = dtls_format_cv_input(is_server_cv, tr_digest, hash_len, cv_msg);

    out_cv[0] = DTLS_HS_CERT_VERIFY;
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

    syn_secure_zero(cv_msg, sizeof(cv_msg));
    return ok;
}

/**
 * @brief Verify a 64-byte ECDSA-P256 signature over SHA-256(cv_msg).
 * @param pubkey     Peer P-256 public key (65-byte 0x04||X||Y or 64-byte X||Y).
 * @param pubkey_len Public key byte length.
 * @param cv_msg     Formatted CertificateVerify signing content.
 * @param cv_len     Byte length of cv_msg.
 * @param sig64      64-byte raw (r || s) signature.
 * @return true if ECDSA signature is valid.
 */
static bool dtls_verify_p256_cv(const uint8_t *pubkey, size_t pubkey_len, const uint8_t *cv_msg,
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
 * @brief Verify a 72-byte inbound DTLS 1.3 CertificateVerify message.
 * @param ctx          DTLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param algo         Expected peer public key algorithm.
 * @param pubkey       Authenticated peer public key bytes.
 * @param pubkey_len   Public key byte length.
 * @param tr_digest    Transcript digest bytes.
 * @param in_cv        72-byte wire CertificateVerify message.
 * @return true if header, SignatureScheme, and cryptographic signature are valid.
 */
static bool dtls_verify_cert_verify(const SYN_DTLS_Context *ctx, bool is_server_cv,
                                    SYN_X509_Algo algo, const uint8_t *pubkey, size_t pubkey_len,
                                    const uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                                    const uint8_t in_cv[DTLS_CERT_VERIFY_LEN])
{
    static const uint8_t cv_hdr[4] = {DTLS_HS_CERT_VERIFY, 0x00U, 0x00U, 68U};
    if (memcmp(in_cv, cv_hdr, 4U) != 0 || in_cv[6] != 0x00U || in_cv[7] != 64U) {
        return false;
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    uint8_t cv_msg[146];
    size_t cv_len = dtls_format_cv_input(is_server_cv, tr_digest, hash_len, cv_msg);
    uint16_t scheme = ((uint16_t)in_cv[4] << 8U) | (uint16_t)in_cv[5];

    bool ok = false;
    if (algo == SYN_X509_ALGO_ED25519 && scheme == 0x0807U && pubkey_len == 32U) {
        ok = syn_ed25519_verify(in_cv + 8U, cv_msg, cv_len, pubkey);
    } else if (algo == SYN_X509_ALGO_ECDSA_P256 && scheme == 0x0403U) {
        ok = dtls_verify_p256_cv(pubkey, pubkey_len, cv_msg, cv_len, in_cv + 8U);
    }

    syn_secure_zero(cv_msg, sizeof(cv_msg));
    return ok;
}

/**
 * @brief Bind local certificate or RPK into transcript and sign CertificateVerify.
 * @param ctx          DTLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param out_cv       [out] 72-byte CertificateVerify message buffer.
 * @return true if local credential parsed and signature succeeded.
 */
static bool dtls_build_local_cv(SYN_DTLS_Context *ctx, bool is_server_cv,
                                uint8_t out_cv[DTLS_CERT_VERIFY_LEN])
{
    SYN_X509_Algo algo = SYN_X509_ALGO_ED25519;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY) {
        uint8_t pub[32];
        (void)syn_ed25519_publickey(ctx->config.client_privkey, pub);
        dtls_chain_transcript(ctx, ctx->transcript_digest, pub, 32U);
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
        dtls_chain_transcript(ctx, ctx->transcript_digest, ctx->config.client_cert_der,
                              ctx->config.client_cert_len);
    }
    if (!dtls_sign_cert_verify(ctx, is_server_cv, algo, ctx->transcript_digest, out_cv)) {
        return false;
    }
    dtls_chain_transcript(ctx, ctx->transcript_digest, out_cv, DTLS_CERT_VERIFY_LEN);
    return true;
}

/**
 * @brief Bind peer certificate or RPK into a transcript copy and verify CertificateVerify.
 * @param ctx          DTLS context pointer.
 * @param is_server_cv true for server CertificateVerify, false for client.
 * @param tr_digest    [in,out] Transcript digest buffer.
 * @param in_cv        72-byte inbound CertificateVerify message.
 * @return true if peer CertificateVerify signature matches authenticated peer key.
 */
static bool dtls_check_peer_cv(SYN_DTLS_Context *ctx, bool is_server_cv,
                               uint8_t tr_digest[SYN_DTLS_SECRET_LEN],
                               const uint8_t in_cv[DTLS_CERT_VERIFY_LEN])
{
    bool ok = false;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY) {
        dtls_chain_transcript(ctx, tr_digest, ctx->config.peer_pubkey, 32U);
        ok = dtls_verify_cert_verify(ctx, is_server_cv, SYN_X509_ALGO_ED25519,
                                     ctx->config.peer_pubkey, 32U, tr_digest, in_cv);
    } else {
        uint8_t peer_pub[SYN_X509_PUBKEY_MAX_LEN];
        SYN_X509_Cert *peer = &ctx->crypto_scratch.cert;
        (void)syn_x509_parse(ctx->config.peer_cert_der, ctx->config.peer_cert_len, peer);
        SYN_X509_Algo peer_algo = peer->pubkey_algo;
        size_t peer_pub_len = peer->pubkey_len;
        memcpy(peer_pub, peer->pubkey, peer_pub_len);
        syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));

        dtls_chain_transcript(ctx, tr_digest, ctx->config.peer_cert_der, ctx->config.peer_cert_len);
        ok = dtls_verify_cert_verify(ctx, is_server_cv, peer_algo, peer_pub, peer_pub_len,
                                     tr_digest, in_cv);
    }
    if (ok) {
        dtls_chain_transcript(ctx, tr_digest, in_cv, DTLS_CERT_VERIFY_LEN);
    }
    return ok;
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
 * @brief Verify peer credentials according to configured DTLS 1.3 authentication mode.
 * @param ctx DTLS context pointer.
 * @return true if peer credentials pass validation.
 */
static bool dtls_verify_peer_auth(SYN_DTLS_Context *ctx)
{
    if (!dtls_verify_local_auth(ctx)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    bool need_peer_x509 =
        (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) ||
        (ctx->config.mode == SYN_DTLS_AUTH_MODE_X509_SERVER && !ctx->config.is_server);
    if (!need_peer_x509) {
        return true;
    }

    if (ctx->config.root_ca == NULL || ctx->config.peer_cert_der == NULL ||
        ctx->config.peer_cert_len == 0U) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    SYN_X509_Cert *peer_cert = &ctx->crypto_scratch.cert;
    const char *expected_cn = ctx->config.is_server ? NULL : ctx->config.server_name;
    if (!syn_x509_parse(ctx->config.peer_cert_der, ctx->config.peer_cert_len, peer_cert) ||
        !syn_x509_validate_chain(peer_cert, ctx->config.root_ca, expected_cn)) {
        syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }
    syn_secure_zero(&ctx->crypto_scratch, sizeof(ctx->crypto_scratch));
    return true;
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
 * @brief Build and send an 80-byte DTLS 1.3 ClientHello datagram.
 * @param ctx DTLS client context.
 * @return true if ClientHello datagram was sent.
 */
static bool dtls_send_client_hello(SYN_DTLS_Context *ctx)
{
    if (syn_random_fill(ctx->ecdhe_priv, 32U) != SYN_OK ||
        syn_random_fill(ctx->client_random, 32U) != SYN_OK) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }
    syn_x25519_clamp(ctx->ecdhe_priv);
    syn_x25519_pubkey(ctx->ecdhe_pub, ctx->ecdhe_priv);

    uint8_t *tx = ctx->tx_buf;
    uint16_t suite_id = dtls_cipher_wire_id(ctx->config.cipher_suite);
    dtls_write_epoch0_hdr(tx, 67U);
    tx[13] = DTLS_HS_CLIENT_HELLO;
    tx[14] = (uint8_t)(suite_id >> 8U);
    tx[15] = (uint8_t)(suite_id & 0xFFU);
    memcpy(tx + 16U, ctx->client_random, 32U);
    memcpy(tx + 48U, ctx->ecdhe_pub, 32U);

    if (!syn_transport_send(ctx->underlying_transport, tx, 80U)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }
    ctx->state = SYN_DTLS_STATE_CLIENT_HELLO_SENT;
    return true;
}

/**
 * @brief Build and transmit server Flight 2 (ServerHello [+ CertificateVerify] + Finished).
 * @param ctx       DTLS server context.
 * @param exp_suite Negotiated 16-bit cipher suite wire identifier.
 * @return true if Flight 2 signed and transmitted.
 */
static bool dtls_server_send_flight2(SYN_DTLS_Context *ctx, uint16_t exp_suite)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    bool has_cv = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK);
    size_t cv_len = has_cv ? DTLS_CERT_VERIFY_LEN : 0U;
    size_t body_len = 67U + cv_len + hash_len;
    uint8_t *tx = ctx->tx_buf;

    if (ctx->tx_buf_size < DTLS_EPOCH0_HDR_LEN + body_len) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    dtls_write_epoch0_hdr(tx, body_len);
    tx[13] = DTLS_HS_SERVER_HELLO;
    tx[14] = (uint8_t)(exp_suite >> 8U);
    tx[15] = (uint8_t)(exp_suite & 0xFFU);
    memcpy(tx + 16U, ctx->server_random, 32U);
    memcpy(tx + 48U, ctx->ecdhe_pub, 32U);

    if (has_cv && !dtls_build_local_cv(ctx, true, tx + 80U)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    uint8_t *fin = tx + 80U + cv_len;
    dtls_compute_finished(ctx, ctx->server_handshake_secret, ctx->transcript_digest, fin);
    dtls_chain_transcript(ctx, ctx->transcript_digest, fin, hash_len);
    dtls_derive_app_keys(ctx);

    if (!syn_transport_send(ctx->underlying_transport, tx, DTLS_EPOCH0_HDR_LEN + body_len)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        ctx->state = SYN_DTLS_STATE_FINISHED_SENT;
    } else {
        dtls_scrub_handshake_secrets(ctx);
        ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    }
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;
    return true;
}

/**
 * @brief Parse ClientHello datagram on server; silently drop malformed packets per RFC 9147 §4.5.1.
 * @param ctx    DTLS server context.
 * @param rx_len Received datagram length.
 * @return true if valid ClientHello processed or stray packet discarded; false on local error.
 */
static bool dtls_server_process_client_hello(SYN_DTLS_Context *ctx, size_t rx_len)
{
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

    if (rx_len != 80U || memcmp(rx, ch_hdr, sizeof(ch_hdr)) != 0 ||
        (((uint16_t)rx[14] << 8U) | (uint16_t)rx[15]) != exp_suite) {
        return true;
    }

    memcpy(ctx->client_random, rx + 16U, 32U);
    memcpy(ctx->peer_ecdhe_pub, rx + 48U, 32U);

    if (syn_random_fill(ctx->ecdhe_priv, 32U) != SYN_OK ||
        syn_random_fill(ctx->server_random, 32U) != SYN_OK) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }
    syn_x25519_clamp(ctx->ecdhe_priv);
    syn_x25519_pubkey(ctx->ecdhe_pub, ctx->ecdhe_priv);

    if (!dtls_derive_hs_secrets(ctx)) {
        return true;
    }

    return dtls_server_send_flight2(ctx, exp_suite);
}

/**
 * @brief Build and transmit mTLS Flight 3 (Client CertificateVerify + Client Finished).
 * @param ctx DTLS client context.
 * @return true if signed and transmitted, false on local error.
 */
static bool dtls_send_client_mtls_flight(SYN_DTLS_Context *ctx)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    size_t body_len = DTLS_CERT_VERIFY_LEN + 4U + hash_len;
    uint8_t *tx = ctx->tx_buf;

    if (ctx->tx_buf_size < DTLS_EPOCH0_HDR_LEN + body_len ||
        !dtls_build_local_cv(ctx, false, tx + DTLS_EPOCH0_HDR_LEN)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }

    dtls_write_epoch0_hdr(tx, body_len);
    uint8_t *fin = tx + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN;
    fin[0] = DTLS_HS_CLIENT_FINISHED;
    fin[1] = 0x00U;
    fin[2] = 0x00U;
    fin[3] = (uint8_t)hash_len;
    dtls_compute_finished(ctx, ctx->client_handshake_secret, ctx->transcript_digest, fin + 4U);

    if (!syn_transport_send(ctx->underlying_transport, tx, DTLS_EPOCH0_HDR_LEN + body_len)) {
        ctx->state = SYN_DTLS_STATE_ERROR;
        return false;
    }
    return true;
}

/**
 * @brief Process mTLS Flight 3 on server; silently drop forged/stray UDP packets per RFC 9147.
 * @param ctx    DTLS server context in SYN_DTLS_STATE_FINISHED_SENT.
 * @param rx_len Received datagram length.
 * @return true (session transitions to ESTABLISHED only on authentic client flight).
 */
static bool dtls_server_process_client_mtls_flight(SYN_DTLS_Context *ctx, size_t rx_len)
{
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

    if (rx_len != DTLS_EPOCH0_HDR_LEN + body_len || memcmp(rx, exp_hdr, sizeof(exp_hdr)) != 0 ||
        memcmp(rx + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN, fin_hdr, 4U) != 0) {
        return true;
    }

    uint8_t tr[SYN_DTLS_SECRET_LEN];
    memcpy(tr, ctx->transcript_digest, sizeof(tr));
    if (!dtls_check_peer_cv(ctx, false, tr, rx + DTLS_EPOCH0_HDR_LEN)) {
        return true;
    }

    uint8_t expected_mac[SYN_DTLS_SECRET_LEN];
    dtls_compute_finished(ctx, ctx->client_handshake_secret, tr, expected_mac);
    bool ok = dtls_verify_mac_ct(expected_mac, rx + DTLS_EPOCH0_HDR_LEN + DTLS_CERT_VERIFY_LEN + 4U,
                                 hash_len);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    if (ok) {
        memcpy(ctx->transcript_digest, tr, sizeof(tr));
        dtls_scrub_handshake_secrets(ctx);
        ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    }
    return true;
}

/**
 * @brief Parse ServerHello [+ CertificateVerify] + Finished on client; drop invalid UDP packets.
 * @param ctx    DTLS client context.
 * @param rx_len Received datagram length.
 * @return true if verified or stray packet discarded, false on local mTLS send/sign error.
 */
static bool dtls_client_process_server_hello(SYN_DTLS_Context *ctx, size_t rx_len)
{
    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;
    bool has_cv = (ctx->config.mode != SYN_DTLS_AUTH_MODE_PSK);
    size_t cv_len = has_cv ? DTLS_CERT_VERIFY_LEN : 0U;
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

    if (rx_len != DTLS_EPOCH0_HDR_LEN + body_len || memcmp(rx, sh_hdr, sizeof(sh_hdr)) != 0 ||
        (((uint16_t)rx[14] << 8U) | (uint16_t)rx[15]) != exp_suite) {
        return true;
    }

    memcpy(ctx->server_random, rx + 16U, 32U);
    memcpy(ctx->peer_ecdhe_pub, rx + 48U, 32U);

    if (!dtls_derive_hs_secrets(ctx) ||
        (has_cv && !dtls_check_peer_cv(ctx, true, ctx->transcript_digest, rx + 80U))) {
        return true;
    }

    const uint8_t *fin = rx + 80U + cv_len;
    uint8_t expected_mac[SYN_DTLS_SECRET_LEN];
    dtls_compute_finished(ctx, ctx->server_handshake_secret, ctx->transcript_digest, expected_mac);
    bool ok = dtls_verify_mac_ct(expected_mac, fin, hash_len);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    if (!ok) {
        return true;
    }

    dtls_chain_transcript(ctx, ctx->transcript_digest, fin, hash_len);
    dtls_derive_app_keys(ctx);

    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS && !dtls_send_client_mtls_flight(ctx)) {
        return false;
    }

    dtls_scrub_handshake_secrets(ctx);
    ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;
    return true;
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
 * @brief Advance server-side DTLS 1.3 handshake state machine by one non-blocking step.
 * @param ctx DTLS server context.
 * @return true if progressing or completed, false on fatal local error.
 */
static bool dtls_server_handshake_step(SYN_DTLS_Context *ctx)
{
    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return true;
    }
    if (ctx->state == SYN_DTLS_STATE_FINISHED_SENT) {
        return dtls_server_process_client_mtls_flight(ctx, rx_len);
    }
    return dtls_server_process_client_hello(ctx, rx_len);
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

bool syn_dtls_init(SYN_DTLS_Context *ctx, const SYN_DTLS_Config *config, SYN_Transport *transport,
                   uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    if (ctx == NULL || config == NULL || transport == NULL || rx_buf == NULL || tx_buf == NULL ||
        rx_buf_size < 140U || tx_buf_size < 140U) {
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
    return true;
}

bool syn_dtls_handshake(SYN_DTLS_Context *ctx)
{
    if (ctx == NULL || ctx->state == SYN_DTLS_STATE_ERROR) {
        return false;
    }
    if (ctx->state == SYN_DTLS_STATE_ESTABLISHED) {
        return true;
    }
    if (!dtls_verify_peer_auth(ctx)) {
        return false;
    }

    if (ctx->config.is_server) {
        return dtls_server_handshake_step(ctx);
    }

    if (ctx->state == SYN_DTLS_STATE_UNINITIALIZED && !dtls_send_client_hello(ctx)) {
        return false;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return true;
    }
    return dtls_client_process_server_hello(ctx, rx_len);
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

    while (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx)) {
            ctx->state = SYN_DTLS_STATE_ERROR;
            PT_EXIT(pt);
        }
        if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
            PT_PASS(pt);
        }
    }

    PT_WAIT_UNTIL(pt, ctx->state != SYN_DTLS_STATE_ESTABLISHED);

    PT_END(pt);
}

#endif /* SYN_DTLS_ENABLED */
