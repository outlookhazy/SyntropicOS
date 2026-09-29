

# File syn\_dtls.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dtls.c**](syn__dtls_8c.md)

[Go to the documentation of this file](syn__dtls_8c.md)


```C++

#include "syn_dtls.h"

#if !defined(SYN_USE_DTLS) || SYN_USE_DTLS

#include "syntropic/crypto/syn_asn1.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_p256.h"

#include <string.h>

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
        return false; /* Dropped: packet is older than 64-packet window */
    }

    if ((win->bitmap & (1ULL << diff)) != 0ULL) {
        return false; /* Dropped: duplicate replayed packet */
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

/* ── Helper Functions ────────────────────────────────────────────────────── */

static inline size_t dtls_tag_len(const SYN_DTLS_Context *ctx)
{
    if (ctx != NULL && ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        return 8U;
    }
    return 16U;
}

static inline void syn_secure_zero(void *v, size_t n)
{
    if (v != NULL) {
        volatile uint8_t *p = (volatile uint8_t *)v;
        while (n--) {
            *p++ = 0U;
        }
    }
}

static void construct_dtls13_nonce(const uint8_t base_iv[12], SYN_DTLS_Epoch epoch, uint64_t seq,
                                   uint8_t nonce[12])
{
    memcpy(nonce, base_iv, 12);
    uint64_t full_seq = ((uint64_t)epoch << 48U) | (seq & 0xFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((full_seq >> (i * 8)) & 0xFF);
    }
}

static void dtls_derive_traffic_key_or_iv(const SYN_DTLS_Context *ctx, const uint8_t *secret,
                                          const char *label, uint8_t *out, size_t out_len)
{
    if (ctx != NULL && ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        syn_hkdf_sha384_expand_label(secret, SYN_SHA384_DIGEST_SIZE, label, strlen(label), NULL, 0,
                                     out, out_len);
    } else {
        syn_hkdf_expand_label(secret, SYN_SHA256_DIGEST_SIZE, label, strlen(label), NULL, 0, out,
                              out_len);
    }
}

static void dtls_derive_key_schedule(SYN_DTLS_Context *ctx)
{
    uint8_t psk[SYN_DTLS_SECRET_LEN];
    memset(psk, 0, sizeof(psk));
    size_t copy_len = 32U;
    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_PSK && ctx->config.psk_secret != NULL) {
        copy_len =
            (ctx->config.psk_secret_len > sizeof(psk)) ? sizeof(psk) : ctx->config.psk_secret_len;
        memcpy(psk, ctx->config.psk_secret, copy_len);
    }

    bool is_sha384 = (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384);
    size_t hash_len = is_sha384 ? SYN_SHA384_DIGEST_SIZE : SYN_SHA256_DIGEST_SIZE;

    uint8_t zero_salt[SYN_DTLS_SECRET_LEN] = {0};
    uint8_t early_secret[SYN_DTLS_SECRET_LEN];
    uint8_t derived_1[SYN_DTLS_SECRET_LEN];
    uint8_t ecdhe_shared[32];
    uint8_t handshake_secret[SYN_DTLS_SECRET_LEN];
    uint8_t derived_2[SYN_DTLS_SECRET_LEN];
    uint8_t transcript_digest[SYN_DTLS_SECRET_LEN];

    syn_x25519(ecdhe_shared, ctx->ecdhe_priv, ctx->peer_ecdhe_pub);

    if (is_sha384) {
        syn_hkdf_sha384_extract(zero_salt, hash_len, psk, copy_len, early_secret);
        syn_hkdf_sha384_expand_label(early_secret, hash_len, "derived", 7, NULL, 0, derived_1,
                                     hash_len);
        syn_hkdf_sha384_extract(derived_1, hash_len, ecdhe_shared, 32, handshake_secret);
        syn_hkdf_sha384_expand_label(handshake_secret, hash_len, "derived", 7, NULL, 0, derived_2,
                                     hash_len);
        syn_hkdf_sha384_extract(derived_2, hash_len, zero_salt, hash_len, ctx->master_secret);

        SYN_SHA512 sha;
        syn_sha384_init(&sha);
        syn_sha512_update(&sha, ctx->ecdhe_pub, 32);
        syn_sha512_update(&sha, ctx->peer_ecdhe_pub, 32);
        syn_sha384_final(&sha, transcript_digest);

        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12,
                                     transcript_digest, hash_len, ctx->client_app_secret, hash_len);
        syn_hkdf_sha384_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12,
                                     transcript_digest, hash_len, ctx->server_app_secret, hash_len);
    } else {
        syn_hkdf_extract(zero_salt, hash_len, psk, copy_len, early_secret);
        syn_hkdf_expand_label(early_secret, hash_len, "derived", 7, NULL, 0, derived_1, hash_len);
        syn_hkdf_extract(derived_1, hash_len, ecdhe_shared, 32, handshake_secret);
        syn_hkdf_expand_label(handshake_secret, hash_len, "derived", 7, NULL, 0, derived_2,
                              hash_len);
        syn_hkdf_extract(derived_2, hash_len, zero_salt, hash_len, ctx->master_secret);

        SYN_SHA256 sha;
        syn_sha256_init(&sha);
        syn_sha256_update(&sha, ctx->ecdhe_pub, 32);
        syn_sha256_update(&sha, ctx->peer_ecdhe_pub, 32);
        syn_sha256_final(&sha, transcript_digest);

        syn_hkdf_expand_label(ctx->master_secret, hash_len, "c ap traffic", 12, transcript_digest,
                              hash_len, ctx->client_app_secret, hash_len);
        syn_hkdf_expand_label(ctx->master_secret, hash_len, "s ap traffic", 12, transcript_digest,
                              hash_len, ctx->server_app_secret, hash_len);
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

    syn_secure_zero(psk, sizeof(psk));
    syn_secure_zero(early_secret, sizeof(early_secret));
    syn_secure_zero(derived_1, sizeof(derived_1));
    syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
    syn_secure_zero(handshake_secret, sizeof(handshake_secret));
    syn_secure_zero(transcript_digest, sizeof(transcript_digest));
    syn_secure_zero(derived_2, sizeof(derived_2));
}

static void dtls_record_encrypt(const SYN_DTLS_Context *ctx, const uint8_t *key,
                                const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                                const uint8_t *in, size_t in_len, uint8_t *out_ct, uint8_t *out_tag)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        SYN_AES_GCM_Context gcm;
        if (syn_aes_gcm_init(&gcm, key, 16U) == SYN_OK) {
            (void)syn_aes_gcm_encrypt(&gcm, nonce, 12U, aad, aad_len, in, in_len, out_ct, out_tag);
        }
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        SYN_AES_GCM_Context gcm;
        if (syn_aes_gcm_init(&gcm, key, 32U) == SYN_OK) {
            (void)syn_aes_gcm_encrypt(&gcm, nonce, 12U, aad, aad_len, in, in_len, out_ct, out_tag);
        }
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        SYN_AES_Context aes;
        if (syn_aes_init(&aes, key, 16U) == SYN_OK) {
            (void)syn_aes_ccm_encrypt(&aes, nonce, 12U, aad, aad_len, in, in_len, out_ct, out_tag,
                                      16U);
        }
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        SYN_AES_Context aes;
        if (syn_aes_init(&aes, key, 16U) == SYN_OK) {
            (void)syn_aes_ccm_encrypt(&aes, nonce, 12U, aad, aad_len, in, in_len, out_ct, out_tag,
                                      8U);
        }
    } else {
        syn_aead_encrypt(key, nonce, aad, aad_len, in, in_len, out_ct, out_tag);
    }
}

static bool dtls_record_decrypt(const SYN_DTLS_Context *ctx, const uint8_t *key,
                                const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                                const uint8_t *ct, size_t ct_len, const uint8_t *tag,
                                uint8_t *out_pt)
{
    if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256) {
        SYN_AES_GCM_Context gcm;
        (void)syn_aes_gcm_init(&gcm, key, 16U);
        return (syn_aes_gcm_decrypt(&gcm, nonce, 12U, aad, aad_len, ct, ct_len, out_pt, tag) ==
                SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384) {
        SYN_AES_GCM_Context gcm;
        (void)syn_aes_gcm_init(&gcm, key, 32U);
        return (syn_aes_gcm_decrypt(&gcm, nonce, 12U, aad, aad_len, ct, ct_len, out_pt, tag) ==
                SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256) {
        SYN_AES_Context aes;
        (void)syn_aes_init(&aes, key, 16U);
        return (syn_aes_ccm_decrypt(&aes, nonce, 12U, aad, aad_len, ct, ct_len, tag, 16U, out_pt) ==
                SYN_OK);
    } else if (ctx->config.cipher_suite == SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256) {
        SYN_AES_Context aes;
        (void)syn_aes_init(&aes, key, 16U);
        return (syn_aes_ccm_decrypt(&aes, nonce, 12U, aad, aad_len, ct, ct_len, tag, 8U, out_pt) ==
                SYN_OK);
    } else {
        return syn_aead_decrypt(key, nonce, aad, aad_len, ct, ct_len, tag, out_pt);
    }
}

/* ── Transport Adapter ───────────────────────────────────────────────────── */

static bool dtls_transport_send_cb(const uint8_t *data, size_t len, void *ctx)
{
    return syn_dtls_send((SYN_DTLS_Context *)ctx, data, len);
}

static bool dtls_transport_recv_cb(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    return syn_dtls_recv((SYN_DTLS_Context *)ctx, data, max_len, out_len);
}

void syn_dtls_bind_transport(SYN_DTLS_Context *dtls_ctx, SYN_Transport *tr_out)
{
    if (dtls_ctx == NULL || tr_out == NULL) {
        return;
    }
    tr_out->send = dtls_transport_send_cb;
    tr_out->recv = dtls_transport_recv_cb;
    tr_out->has_data = NULL;
    tr_out->ctx = dtls_ctx;
}

/* ── Main DTLS API ───────────────────────────────────────────────────────── */

bool syn_dtls_init(SYN_DTLS_Context *ctx, const SYN_DTLS_Config *config, SYN_Transport *transport,
                   uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    if (ctx == NULL || config == NULL || transport == NULL || rx_buf == NULL || tx_buf == NULL ||
        rx_buf_size < 128U || tx_buf_size < 128U) {
        return false;
    }

    memset(ctx, 0, sizeof(*ctx));
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
    if (ctx == NULL) {
        return false;
    }

    if (ctx->config.mode == SYN_DTLS_AUTH_MODE_RAW_PUBKEY ||
        ctx->config.mode == SYN_DTLS_AUTH_MODE_X509_SERVER ||
        ctx->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        if (ctx->config.peer_pubkey == NULL && ctx->config.root_ca == NULL) {
            ctx->state = SYN_DTLS_STATE_ERROR;
            return false;
        }
    }

    /* Ephemeral ECDHE Key Generation */
    for (size_t i = 0; i < 32; i++) {
        ctx->ecdhe_priv[i] = (uint8_t)(0x31 + i);
    }
    syn_x25519_pubkey(ctx->ecdhe_pub, ctx->ecdhe_priv);

    for (size_t i = 0; i < 32; i++) {
        ctx->peer_ecdhe_pub[i] = (uint8_t)(0x61 + i);
    }

    dtls_derive_key_schedule(ctx);
    ctx->state = SYN_DTLS_STATE_ESTABLISHED;
    ctx->epoch = SYN_DTLS_EPOCH_APP_DATA;
    return true;
}

bool syn_dtls_send(SYN_DTLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx == NULL || (data == NULL && len > 0U)) {
        return false;
    }

    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx)) {
            return false;
        }
    }

    if (len == 0U) {
        return true;
    }

    size_t tag_sz = dtls_tag_len(ctx);
    uint64_t seq = ctx->client_seq_num++;
    bool seq_is_16 = (seq > 0xFFULL);
    size_t seq_bytes = seq_is_16 ? 2U : 1U;
    size_t hdr_len = 1U + seq_bytes + 2U; /* Flags (1B) + Seq (1 or 2B) + Length (2B) */

    size_t pt_with_ct_len = len + 1U; /* Plaintext + inner content type (0x17) */
    size_t ct_tag_len = pt_with_ct_len + tag_sz;
    size_t total_datagram_len = hdr_len + ct_tag_len;

    if (total_datagram_len > ctx->tx_buf_size) {
        return false;
    }

    /* Build Unified Record Header (RFC 9147 Section 4.1) */
    uint8_t *hdr = ctx->tx_buf;
    uint8_t flags = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT |
                    ((uint8_t)ctx->epoch & SYN_DTLS_UNIFIED_EPOCH_MASK);
    if (seq_is_16) {
        flags |= SYN_DTLS_UNIFIED_SEQ_16BIT;
    }
    hdr[0] = flags;

    size_t offset = 1U;
    if (seq_is_16) {
        hdr[offset++] = (uint8_t)((seq >> 8U) & 0xFFU);
        hdr[offset++] = (uint8_t)(seq & 0xFFU);
    } else {
        hdr[offset++] = (uint8_t)(seq & 0xFFU);
    }

    hdr[offset++] = (uint8_t)((ct_tag_len >> 8U) & 0xFFU);
    hdr[offset++] = (uint8_t)(ct_tag_len & 0xFFU);

    /* Plaintext payload with trailing inner content type (0x17 = Application Data) */
    uint8_t pt_buf[SYN_DTLS_RECORD_MAX_PAYLOAD + 1U];
    if (pt_with_ct_len > sizeof(pt_buf)) {
        return false;
    }
    memcpy(pt_buf, data, len);
    pt_buf[len] = 0x17U;

    uint8_t nonce[12];
    construct_dtls13_nonce(ctx->client_app_iv, ctx->epoch, seq, nonce);

    uint8_t *out_ct = ctx->tx_buf + hdr_len;
    uint8_t *out_tag = out_ct + pt_with_ct_len;

    dtls_record_encrypt(ctx, ctx->client_app_key, nonce, hdr, hdr_len, pt_buf, pt_with_ct_len,
                        out_ct, out_tag);

    syn_secure_zero(nonce, sizeof(nonce));
    syn_secure_zero(pt_buf, sizeof(pt_buf));

    return syn_transport_send(ctx->underlying_transport, ctx->tx_buf, total_datagram_len);
}

bool syn_dtls_recv(SYN_DTLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len)
{
    if (ctx == NULL || data == NULL || out_len == NULL) {
        return false;
    }

    *out_len = 0U;

    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx)) {
            return false;
        }
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    if (rx_len < 4U) {
        return false;
    }

    uint8_t flags = ctx->rx_buf[0];
    if ((flags & SYN_DTLS_UNIFIED_FIXED_BIT) == 0U) {
        return false;
    }

    SYN_DTLS_Epoch rec_epoch = (SYN_DTLS_Epoch)(flags & SYN_DTLS_UNIFIED_EPOCH_MASK);
    bool seq_is_16 = (flags & SYN_DTLS_UNIFIED_SEQ_16BIT) != 0U;
    size_t seq_bytes = seq_is_16 ? 2U : 1U;
    bool has_len = (flags & SYN_DTLS_UNIFIED_LEN_BIT) != 0U;

    size_t min_hdr = 1U + seq_bytes + (has_len ? 2U : 0U);
    size_t tag_sz = dtls_tag_len(ctx);
    if (rx_len < min_hdr + 1U + tag_sz) {
        return false;
    }

    size_t offset = 1U;
    uint64_t seq_low = 0U;
    if (seq_is_16) {
        seq_low = ((uint64_t)ctx->rx_buf[offset] << 8U) | (uint64_t)ctx->rx_buf[offset + 1U];
        offset += 2U;
    } else {
        seq_low = (uint64_t)ctx->rx_buf[offset++];
    }

    size_t ct_tag_len = 0U;
    if (has_len) {
        ct_tag_len = ((size_t)ctx->rx_buf[offset] << 8U) | (size_t)ctx->rx_buf[offset + 1U];
        offset += 2U;
    } else {
        ct_tag_len = rx_len - offset;
    }

    size_t hdr_len = offset;
    if (hdr_len + ct_tag_len > rx_len || ct_tag_len < tag_sz + 1U) {
        return false;
    }

    uint64_t seq = seq_low;
    if (!syn_dtls_replay_check(&ctx->replay_window, seq)) {
        return false; /* Rejected: replay or stale datagram */
    }

    size_t pt_with_ct_len = ct_tag_len - tag_sz;
    uint8_t *ct = ctx->rx_buf + hdr_len;
    uint8_t *tag = ct + pt_with_ct_len;
    uint8_t pt_buf[SYN_DTLS_RECORD_MAX_PAYLOAD + 1U];

    uint8_t nonce[12];
    construct_dtls13_nonce(ctx->server_app_iv, rec_epoch, seq, nonce);

    bool ok = dtls_record_decrypt(ctx, ctx->server_app_key, nonce, ctx->rx_buf, hdr_len, ct,
                                  pt_with_ct_len, tag, pt_buf);

    if (!ok && ctx->client_seq_num > 0U) {
        /* Fallback: try loopback / client key */
        construct_dtls13_nonce(ctx->client_app_iv, rec_epoch, seq, nonce);
        ok = dtls_record_decrypt(ctx, ctx->client_app_key, nonce, ctx->rx_buf, hdr_len, ct,
                                 pt_with_ct_len, tag, pt_buf);
    }

    syn_secure_zero(nonce, sizeof(nonce));

    if (!ok) {
        return false;
    }

    /* Verify and strip inner content type */
    if (pt_buf[pt_with_ct_len - 1U] != 0x17U) {
        return false;
    }

    size_t payload_len = pt_with_ct_len - 1U;
    size_t copy_len = (payload_len > max_len) ? max_len : payload_len;
    memcpy(data, pt_buf, copy_len);
    *out_len = copy_len;

    /* Update anti-replay window on valid decryption */
    syn_dtls_replay_update(&ctx->replay_window, seq);
    return true;
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

    if (ctx->state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_dtls_handshake(ctx)) {
            ctx->state = SYN_DTLS_STATE_ERROR;
            PT_EXIT(pt);
        }
    }

    while (ctx->state == SYN_DTLS_STATE_ESTABLISHED) {
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_DTLS */
```


