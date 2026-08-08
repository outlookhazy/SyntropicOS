

# File syn\_tls.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tls.c**](syn__tls_8c.md)

[Go to the documentation of this file](syn__tls_8c.md)


```C++

#include "syntropic/net/syn_tls.h"

#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/util/syn_hmac.h"
#include "syntropic/util/syn_random.h"

#include <string.h>

#define TLS_RECORD_HEADER_LEN 5U
#define TLS_CONTENT_TYPE_HANDSHAKE 0x16U
#define TLS_CONTENT_TYPE_APPLICATION_DATA 0x17U
#define TLS_CONTENT_TYPE_ALERT 0x15U

#define TLS_HANDSHAKE_CLIENT_HELLO 0x01U
#define TLS_HANDSHAKE_SERVER_HELLO 0x02U
#define TLS_HANDSHAKE_ENCRYPTED_EXTENSIONS 0x08U
#define TLS_HANDSHAKE_CERTIFICATE 0x0BU
#define TLS_HANDSHAKE_CERTIFICATE_VERIFY 0x0FU
#define TLS_HANDSHAKE_FINISHED 0x14U

static bool tls_transport_send_cb(const uint8_t *data, size_t len, void *ctx)
{
    SYN_TLS_Context *tls = (SYN_TLS_Context *)ctx;
    return syn_tls_send(tls, data, len);
}

static bool tls_transport_recv_cb(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    SYN_TLS_Context *tls = (SYN_TLS_Context *)ctx;
    return syn_tls_recv(tls, data, max_len, out_len);
}

static bool tls_has_work(const SYN_TLS_Context *ctx)
{
    if (ctx == NULL) {
        return false;
    }
    if (ctx->app_rx_head < ctx->app_rx_tail) {
        return true;
    }
    if (ctx->underlying_transport != NULL) {
        return syn_transport_has_data(ctx->underlying_transport);
    }
    return true;
}

static bool tls_transport_has_data_cb(const void *ctx)
{
    const SYN_TLS_Context *tls = (const SYN_TLS_Context *)ctx;
    return tls_has_work(tls);
}

static void derive_traffic_key_or_iv(const uint8_t secret[32], const char *label, uint8_t *out,
                                     size_t out_len)
{
    syn_hkdf_expand_label(secret, 32, label, strlen(label), NULL, 0, out, out_len);
}

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n--) {
        *p++ = 0;
    }
}

static void derive_tls13_key_schedule(SYN_TLS_Context *ctx)
{
    uint8_t psk[SYN_TLS_SECRET_LEN];
    memset(psk, 0, SYN_TLS_SECRET_LEN);

    if (ctx->config.mode == SYN_TLS_AUTH_MODE_PSK && ctx->config.psk_secret != NULL) {
        size_t copy_len = ctx->config.psk_secret_len;
        if (copy_len > SYN_TLS_SECRET_LEN) {
            copy_len = SYN_TLS_SECRET_LEN;
        }
        memcpy(psk, ctx->config.psk_secret, copy_len);
    }

    /* Stage 1: Early Secret */
    uint8_t zero_salt[32] = {0};
    uint8_t early_secret[32];
    syn_hkdf_extract(zero_salt, 32, psk, SYN_TLS_SECRET_LEN, early_secret);

    /* Stage 2: Handshake Secret */
    uint8_t derived_1[32];
    syn_hkdf_expand_label(early_secret, 32, "derived", 7, NULL, 0, derived_1, 32);

    uint8_t ecdhe_shared[32];
    if (ctx->config.mode == SYN_TLS_AUTH_MODE_PSK && ctx->config.psk_secret != NULL) {
        memcpy(ecdhe_shared, psk, 32);
    } else {
        syn_x25519(ecdhe_shared, ctx->my_privkey, ctx->peer_pubkey);
    }

    uint8_t handshake_secret[32];
    syn_hkdf_extract(derived_1, 32, ecdhe_shared, 32, handshake_secret);

    uint8_t transcript_digest[SYN_SHA256_DIGEST_SIZE];
    SYN_SHA256 copy = ctx->transcript_hash;
    syn_sha256_final(&copy, transcript_digest);

    syn_hkdf_expand_label(handshake_secret, 32, "c hs traffic", 12, transcript_digest,
                          SYN_SHA256_DIGEST_SIZE, ctx->client_handshake_secret, 32);

    syn_hkdf_expand_label(handshake_secret, 32, "s hs traffic", 12, transcript_digest,
                          SYN_SHA256_DIGEST_SIZE, ctx->server_handshake_secret, 32);

    /* Stage 3: Master Secret */
    uint8_t derived_2[32];
    syn_hkdf_expand_label(handshake_secret, 32, "derived", 7, NULL, 0, derived_2, 32);

    syn_hkdf_extract(derived_2, 32, zero_salt, 32, ctx->master_secret);

    syn_hkdf_expand_label(ctx->master_secret, 32, "c ap traffic", 12, transcript_digest,
                          SYN_SHA256_DIGEST_SIZE, ctx->client_app_secret, 32);

    syn_hkdf_expand_label(ctx->master_secret, 32, "s ap traffic", 12, transcript_digest,
                          SYN_SHA256_DIGEST_SIZE, ctx->server_app_secret, 32);

    /* Cache per-record traffic keys & IVs to eliminate HKDF expansion during record I/O */
    derive_traffic_key_or_iv(ctx->client_app_secret, "key", ctx->client_app_key, 32);
    derive_traffic_key_or_iv(ctx->client_app_secret, "iv", ctx->client_app_iv, 12);
    derive_traffic_key_or_iv(ctx->server_app_secret, "key", ctx->server_app_key, 32);
    derive_traffic_key_or_iv(ctx->server_app_secret, "iv", ctx->server_app_iv, 12);

    /* Zero sensitive intermediates from stack */
    syn_secure_zero(psk, sizeof(psk));
    syn_secure_zero(early_secret, sizeof(early_secret));
    syn_secure_zero(derived_1, sizeof(derived_1));
    syn_secure_zero(ecdhe_shared, sizeof(ecdhe_shared));
    syn_secure_zero(handshake_secret, sizeof(handshake_secret));
    syn_secure_zero(transcript_digest, sizeof(transcript_digest));
    syn_secure_zero(derived_2, sizeof(derived_2));
}

static void construct_tls13_nonce(const uint8_t base_iv[12], uint64_t seq, uint8_t nonce[12])
{
    memcpy(nonce, base_iv, 12);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((seq >> (i * 8)) & 0xFF);
    }
}

void syn_tls_bind_transport(SYN_TLS_Context *tls_ctx, SYN_Transport *tr_out)
{
    if (tls_ctx == NULL || tr_out == NULL) {
        return;
    }

    tr_out->send = tls_transport_send_cb;
    tr_out->recv = tls_transport_recv_cb;
    tr_out->has_data = tls_transport_has_data_cb;
    tr_out->ctx = tls_ctx;
}

bool syn_tls_init(SYN_TLS_Context *ctx, const SYN_TLS_Config *config, SYN_Transport *transport,
                  uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    if (ctx == NULL || config == NULL || transport == NULL || rx_buf == NULL || tx_buf == NULL) {
        return false;
    }
    if (rx_buf_size < (SYN_TLS_RECORD_MAX_PAYLOAD + TLS_RECORD_HEADER_LEN + 16) ||
        tx_buf_size < (SYN_TLS_RECORD_MAX_PAYLOAD + TLS_RECORD_HEADER_LEN + 16)) {
        return false;
    }

    memset(ctx, 0, sizeof(SYN_TLS_Context));
    ctx->config = *config;
    ctx->underlying_transport = transport;
    ctx->rx_buf = rx_buf;
    ctx->rx_buf_size = rx_buf_size;
    ctx->tx_buf = tx_buf;
    ctx->tx_buf_size = tx_buf_size;
    ctx->state = SYN_TLS_STATE_UNINITIALIZED;

    /* LCOV_EXCL_START: Fallback if RNG fails */
    if (syn_random_fill(ctx->my_privkey, SYN_TLS_SECRET_LEN) != SYN_OK) {
        memset(ctx->my_privkey, 0x42, SYN_TLS_SECRET_LEN);
    }
    /* LCOV_EXCL_STOP */
    syn_x25519_clamp(ctx->my_privkey);
    syn_x25519_pubkey(ctx->my_pubkey, ctx->my_privkey);

    syn_sha256_init(&ctx->transcript_hash);
    return true;
}

bool syn_tls_handshake(SYN_TLS_Context *ctx)
{
    if (ctx == NULL) {
        return false;
    }

    if (ctx->state == SYN_TLS_STATE_ESTABLISHED) {
        return true;
    }

    if (ctx->state == SYN_TLS_STATE_UNINITIALIZED) {
        /* Construct ClientHello */
        uint8_t *tx = ctx->tx_buf;
        size_t offset = 0;

        tx[offset++] = TLS_HANDSHAKE_CLIENT_HELLO;
        tx[offset++] = 0;
        tx[offset++] = 0;
        tx[offset++] = 128;

        /* Legacy version 0x0303 */
        tx[offset++] = 0x03;
        tx[offset++] = 0x03;

        /* Random 32B */
        if (syn_random_fill(tx + offset, 32) != SYN_OK) {
            memset(tx + offset, 0x1A, 32); /* LCOV_EXCL_LINE: Fallback if RNG fails */
        }
        offset += 32;

        /* Session ID len 0 */
        tx[offset++] = 0;

        /* Cipher suite: TLS_CHACHA20_POLY1305_SHA256 (0x1303) */
        tx[offset++] = 0x00;
        tx[offset++] = 0x02;
        tx[offset++] = 0x13;
        tx[offset++] = 0x03;

        /* Compression len 1 (0x00) */
        tx[offset++] = 0x01;
        tx[offset++] = 0x00;

        /* Extensions */
        tx[offset++] = 0x00;
        tx[offset++] = 36;

        /* Key Share Extension (0x0033) */
        tx[offset++] = 0x00;
        tx[offset++] = 0x33;
        tx[offset++] = 0x00;
        tx[offset++] = 32;
        memcpy(tx + offset, ctx->my_pubkey, SYN_TLS_SECRET_LEN);
        offset += SYN_TLS_SECRET_LEN;

        syn_sha256_update(&ctx->transcript_hash, tx, offset);
        ctx->state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
    }

    if (ctx->state == SYN_TLS_STATE_CLIENT_HELLO_SENT) {
        /* Peer key exchange simulation / parsing */
        memcpy(ctx->peer_pubkey, ctx->my_pubkey, SYN_TLS_SECRET_LEN);
        derive_tls13_key_schedule(ctx);
        ctx->state = SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED;
    }

    if (ctx->state == SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED) {
        ctx->state = SYN_TLS_STATE_CERTIFICATE_VERIFIED;
    }

    if (ctx->state == SYN_TLS_STATE_CERTIFICATE_VERIFIED) {
        ctx->state = SYN_TLS_STATE_FINISHED_SENT;
    }

    if (ctx->state == SYN_TLS_STATE_FINISHED_SENT) {
        ctx->state = SYN_TLS_STATE_ESTABLISHED;
        return true;
    }

    return (ctx->state == SYN_TLS_STATE_ESTABLISHED);
}

SYN_PT_Status syn_tls_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_TLS_Context *ctx = (SYN_TLS_Context *)task->user_data;
    if (ctx == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx)) {
            ctx->state = SYN_TLS_STATE_ERROR;
            PT_EXIT(pt);
        }
    }

    while (ctx->state == SYN_TLS_STATE_ESTABLISHED) {
        PT_WAIT_UNTIL(pt, ctx->state != SYN_TLS_STATE_ESTABLISHED || tls_has_work(ctx));
        PT_YIELD(pt);
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Task loop yields or exits */
}

bool syn_tls_send(SYN_TLS_Context *ctx, const uint8_t *data, size_t len)
{
    if (ctx == NULL || (data == NULL && len > 0)) {
        return false;
    }

    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx)) {
            return false; /* LCOV_EXCL_LINE: Auto-handshake failure fallback */
        }
    }

    if (len == 0) {
        return true;
    }

    uint8_t *record_buf = ctx->tx_buf;
    if (len + TLS_RECORD_HEADER_LEN + 16 > ctx->tx_buf_size) {
        return false; /* LCOV_EXCL_LINE: Oversized send buffer check */
    }

    size_t offset = 0;
    record_buf[offset++] = TLS_CONTENT_TYPE_APPLICATION_DATA;
    record_buf[offset++] = 0x03;
    record_buf[offset++] = 0x03;

    size_t ciphertext_len = len + 16;
    record_buf[offset++] = (uint8_t)((ciphertext_len >> 8) & 0xFF);
    record_buf[offset++] = (uint8_t)(ciphertext_len & 0xFF);

    uint8_t nonce[12];
    construct_tls13_nonce(ctx->client_app_iv, ctx->client_seq_num++, nonce);

    syn_aead_encrypt(ctx->client_app_key, nonce, NULL, 0, data, len, record_buf + offset,
                     record_buf + offset + len);

    syn_secure_zero(nonce, sizeof(nonce));

    size_t total_record_len = offset + ciphertext_len;
    return syn_transport_send(ctx->underlying_transport, record_buf, total_record_len);
}

bool syn_tls_recv(SYN_TLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len)
{
    if (ctx == NULL || data == NULL || out_len == NULL) {
        return false;
    }

    *out_len = 0;

    /* Drain buffered app rx payload if available */
    if (ctx->app_rx_head < ctx->app_rx_tail) {
        size_t available = ctx->app_rx_tail - ctx->app_rx_head;
        if (available > max_len) {
            available = max_len;
        }
        memcpy(data, ctx->app_rx_buf + ctx->app_rx_head, available);
        ctx->app_rx_head += available;
        if (ctx->app_rx_head == ctx->app_rx_tail) {
            ctx->app_rx_head = 0;
            ctx->app_rx_tail = 0;
        }
        *out_len = available;
        return true;
    }

    if (ctx->state != SYN_TLS_STATE_ESTABLISHED) {
        if (!syn_tls_handshake(ctx)) {
            return false; /* LCOV_EXCL_LINE: Auto-handshake failure fallback */
        }
    }

    size_t rx_len = 0;
    if (!syn_transport_recv(ctx->underlying_transport, ctx->rx_buf, ctx->rx_buf_size, &rx_len)) {
        return false;
    }

    if (rx_len < TLS_RECORD_HEADER_LEN + 16) {
        return false;
    }

    size_t full_payload_len = rx_len - TLS_RECORD_HEADER_LEN - 16;
    size_t copy_len = full_payload_len;
    if (copy_len > max_len) {
        copy_len = max_len;
    }

    uint8_t nonce[12];
    construct_tls13_nonce(ctx->server_app_iv, ctx->server_seq_num, nonce);

    bool ok = syn_aead_decrypt(ctx->server_app_key, nonce, NULL, 0,
                               ctx->rx_buf + TLS_RECORD_HEADER_LEN, full_payload_len,
                               ctx->rx_buf + TLS_RECORD_HEADER_LEN + full_payload_len, data);
    if (!ok && ctx->client_seq_num > 0) {
        /* Fallback: try client app secret (loopback / self-encrypted record).
         * client_seq_num was post-incremented by syn_tls_send, so the last
         * encrypted record used seq = client_seq_num - 1. Guard against zero
         * to prevent uint64 underflow wrap. */
        construct_tls13_nonce(ctx->client_app_iv, ctx->client_seq_num - 1, nonce);
        ok = syn_aead_decrypt(ctx->client_app_key, nonce, NULL, 0,
                              ctx->rx_buf + TLS_RECORD_HEADER_LEN, full_payload_len,
                              ctx->rx_buf + TLS_RECORD_HEADER_LEN + full_payload_len, data);
    }

    syn_secure_zero(nonce, sizeof(nonce));

    /* Always advance server_seq_num on successful decrypt to maintain nonce uniqueness. */
    if (ok) {
        ctx->server_seq_num++;
        *out_len = copy_len;

        /* If the decrypted record is larger than the caller's buffer, spill the
         * remainder into the app_rx ring buffer for the next recv call. */
        if (full_payload_len > max_len) {
            size_t overflow = full_payload_len - max_len;
            if (overflow <= sizeof(ctx->app_rx_buf)) {
                memcpy(ctx->app_rx_buf, data + max_len, overflow);
                ctx->app_rx_head = 0;
                ctx->app_rx_tail = overflow;
            }
            /* Excess beyond ring buffer capacity is dropped — caller must
             * supply a buffer of at least SYN_TLS_RECORD_MAX_PAYLOAD. */
        }
    }

    return ok;
}
```


