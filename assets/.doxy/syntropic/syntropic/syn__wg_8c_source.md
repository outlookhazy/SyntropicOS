

# File syn\_wg.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.c**](syn__wg_8c.md)

[Go to the documentation of this file](syn__wg_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG

#include "syn_wg.h"
#include "../port/syn_port_system.h"
#include "../port/syn_port_socket.h"
#include "../util/syn_random.h"
#include "../util/syn_assert.h"
#include "../crypto/syn_blake2s.h"
#include "../crypto/syn_chacha20poly1305.h"
#include "../crypto/syn_x25519.h"
#include "../util/syn_metrics.h"
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 *  Construction constants (from WireGuard spec)
 * ═══════════════════════════════════════════════════════════════════════════ */

static const uint8_t WG_CONSTRUCTION[] = "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s"; 
static const uint8_t WG_IDENTIFIER[]   = "WireGuard v1 zx2c4 Jason@zx2c4.com";    
static const uint8_t WG_LABEL_MAC1[]   = "mac1----";                              
static const uint8_t WG_LABEL_COOKIE[] SYN_UNUSED = "cookie--";                    
/* ═══════════════════════════════════════════════════════════════════════════
 *  Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static inline void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >>  8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] <<  8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static inline void store64_le(uint8_t *p, uint64_t v)
{
    store32_le(p,     (uint32_t)(v));
    store32_le(p + 4, (uint32_t)(v >> 32));
}

static inline uint64_t load64_le(const uint8_t *p)
{
    return (uint64_t)load32_le(p) | ((uint64_t)load32_le(p + 4) << 32);
}

static inline void store32_be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >>  8);
    p[3] = (uint8_t)(v);
}

static inline void store64_be(uint8_t *p, uint64_t v)
{
    store32_be(p,     (uint32_t)(v >> 32));
    store32_be(p + 4, (uint32_t)(v));
}

/* sender index — now using syn_random */
#if SYN_USE_METRICS
SYN_METRIC_DECLARE(wg_handshakes, "wg_handshakes", "Total WireGuard handshakes", SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_tx_bytes,   "wg_tx_bytes",   "Total bytes sent over WG",    SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_rx_bytes,   "wg_rx_bytes",   "Total bytes received over WG", SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_errors,     "wg_errors",     "Total WG processing errors",  SYN_METRIC_TYPE_COUNTER);
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 *  HKDF (HMAC-BLAKE2s based, per WireGuard spec)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_hkdf2(uint8_t out1[32], uint8_t out2[32],
                     const uint8_t ck[32],
                     const uint8_t *input, size_t input_len)
{
    uint8_t prk[32], tmp[33];

    /* Extract */
    syn_hmac_blake2s(ck, 32, input, input_len, prk);

    /* Expand: T1 = HMAC(PRK, 0x01) */
    tmp[0] = 0x01;
    syn_hmac_blake2s(prk, 32, tmp, 1, out1);

    /* Expand: T2 = HMAC(PRK, T1 || 0x02) */
    memcpy(tmp, out1, 32);
    tmp[32] = 0x02;
    syn_hmac_blake2s(prk, 32, tmp, 33, out2);
}

static void wg_hkdf3(uint8_t out1[32], uint8_t out2[32], uint8_t out3[32],
                     const uint8_t ck[32],
                     const uint8_t *input, size_t input_len)
{
    uint8_t prk[32], tmp[33];

    syn_hmac_blake2s(ck, 32, input, input_len, prk);

    tmp[0] = 0x01;
    syn_hmac_blake2s(prk, 32, tmp, 1, out1);

    memcpy(tmp, out1, 32);
    tmp[32] = 0x02;
    syn_hmac_blake2s(prk, 32, tmp, 33, out2);

    memcpy(tmp, out2, 32);
    tmp[32] = 0x03;
    syn_hmac_blake2s(prk, 32, tmp, 33, out3);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Noise handshake helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_mix_hash(uint8_t h[32], const void *data, size_t len)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32);
    syn_blake2s_update(&ctx, h, 32);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, h);
}

static void wg_mix_key(uint8_t ck[32], uint8_t k[32],
                       const uint8_t *input, size_t len)
{
    wg_hkdf2(ck, k, ck, input, len);
}

static void wg_encrypt_and_hash(uint8_t h[32], const uint8_t k[32],
                                const uint8_t *plain, size_t plain_len,
                                uint8_t *ct, uint8_t tag[16])
{
    uint8_t nonce[12];
    memset(nonce, 0, 12);

    syn_aead_encrypt(k, nonce, h, 32, plain, plain_len, ct, tag);

    /* Mix ciphertext + tag into hash as ONE operation:
     * H = HASH(H || ct || tag)  — NOT two separate calls! */
    {
        SYN_BLAKE2s ctx;
        syn_blake2s_init(&ctx, 32);
        syn_blake2s_update(&ctx, h, 32);
        syn_blake2s_update(&ctx, ct, plain_len);
        syn_blake2s_update(&ctx, tag, 16);
        syn_blake2s_final(&ctx, h);
    }
}

static bool wg_decrypt_and_hash(uint8_t h[32], const uint8_t k[32],
                                const uint8_t *ct, size_t ct_len,
                                const uint8_t tag[16],
                                uint8_t *plain)
{
    uint8_t nonce[12];
    memset(nonce, 0, 12);

    if (!syn_aead_decrypt(k, nonce, h, 32, ct, ct_len, tag, plain)) {
        return false;
    }

    /* Mix ciphertext + tag into hash as ONE operation */
    {
        SYN_BLAKE2s ctx;
        syn_blake2s_init(&ctx, 32);
        syn_blake2s_update(&ctx, h, 32);
        syn_blake2s_update(&ctx, ct, ct_len);
        syn_blake2s_update(&ctx, tag, 16);
        syn_blake2s_final(&ctx, h);
    }

    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  TAI64N timestamp
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_tai64n(uint8_t out[12], const SYN_SNTP *sntp)
{
    uint32_t epoch_s  = syn_sntp_get_epoch_s(sntp);
    uint32_t epoch_ns = syn_sntp_get_epoch_ns(sntp);

    /* TAI64N: 8 bytes seconds (TAI epoch: add 2^62 + leap seconds offset),
     * 4 bytes nanoseconds. For WireGuard, the exact TAI offset doesn't
     * matter — only monotonicity. We use UTC + 2^62. */
    uint64_t tai_s = (uint64_t)epoch_s + 0x4000000000000000ULL;

    store64_be(out, tai_s);
    store32_be(out + 8, epoch_ns);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  MAC1 (for handshake messages)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_mac1(uint8_t mac[16],
                    const uint8_t peer_pub[32],
                    const uint8_t *msg, size_t msg_len)
{
    uint8_t key[32];
    SYN_BLAKE2s ctx;

    /* key = HASH("mac1----" || peer_public_key) */
    syn_blake2s_init(&ctx, 32);
    syn_blake2s_update(&ctx, WG_LABEL_MAC1, sizeof(WG_LABEL_MAC1) - 1);
    syn_blake2s_update(&ctx, peer_pub, 32);
    syn_blake2s_final(&ctx, key);

    /* mac1 = MAC(key, msg) — 16-byte keyed BLAKE2s */
    syn_blake2s_mac(key, 32, msg, msg_len, mac, 16);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Handshake: Initiation (we are the initiator)
 * ═══════════════════════════════════════════════════════════════════════════ */

static bool wg_send_initiation(SYN_WG *wg)
{
    uint8_t *msg = wg->tx_buf;
    uint8_t *h = wg->hs_hash;
    uint8_t *ck = wg->hs_chaining_key;
    uint8_t k[32];
    uint8_t dh_result[32];
    uint8_t timestamp[12];
    size_t pos;

    if (wg->tx_buf_size < SYN_WG_INITIATION_SIZE) return false;

    /* Initialize Noise:
     * C = HASH(CONSTRUCTION)
     * H = HASH(C || IDENTIFIER)
     * H = HASH(H || S_pub_responder)
     */
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1, ck, 32);
    {
        SYN_BLAKE2s tmp;
        syn_blake2s_init(&tmp, 32);
        syn_blake2s_update(&tmp, ck, 32);
        syn_blake2s_update(&tmp, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1);
        syn_blake2s_final(&tmp, h);
    }
    wg_mix_hash(h, wg->config.peer_public_key, 32);

    /* Generate ephemeral keypair */
    /* Now using secure syn_random_fill */
    if (syn_random_fill(wg->hs_ephemeral_priv, 32) != SYN_OK) {
        return false;
    }
    syn_x25519_clamp(wg->hs_ephemeral_priv);

    uint8_t ephemeral_pub[32];
    syn_x25519_pubkey(ephemeral_pub, wg->hs_ephemeral_priv);

    /* Message type + reserved */
    pos = 0;
    store32_le(msg + pos, SYN_WG_MSG_INITIATION); pos += 4;

    /* Sender index */
    wg->session.sender_index = syn_random_u32();
    store32_le(msg + pos, wg->session.sender_index); pos += 4;

    /* msg.ephemeral = E_pub */
    memcpy(msg + pos, ephemeral_pub, 32);

    /* C, k = KDF(C, E_pub) */
    wg_mix_key(ck, k, ephemeral_pub, 32);
    /* H = HASH(H || E_pub) */
    wg_mix_hash(h, ephemeral_pub, 32);
    pos += 32;

    /* msg.static = AEAD(k, 0, S_pub_initiator, H) */
    /* DH: C, k = KDF(C, DH(E_priv, S_pub_responder)) */
    syn_x25519(dh_result, wg->hs_ephemeral_priv, wg->config.peer_public_key);
    wg_mix_key(ck, k, dh_result, 32);
    wg_encrypt_and_hash(h, k, wg->public_key, 32, msg + pos, msg + pos + 32);
    pos += 48;  /* 32 ciphertext + 16 tag */

    /* msg.timestamp = AEAD(k, 0, timestamp, H) */
    /* DH: C, k = KDF(C, DH(S_priv, S_pub_responder)) */
    syn_x25519(dh_result, wg->config.private_key, wg->config.peer_public_key);
    wg_mix_key(ck, k, dh_result, 32);
    wg_tai64n(timestamp, wg->sntp);
    wg_encrypt_and_hash(h, k, timestamp, 12, msg + pos, msg + pos + 12);
    pos += 28;  /* 12 ciphertext + 16 tag */

    /* mac1 = MAC(HASH("mac1----" || S_pub_responder), msg[0..pos]) */
    wg_mac1(msg + pos, wg->config.peer_public_key, msg, pos);
    pos += 16;

    /* mac2 = zeros (no cookie) */
    memset(msg + pos, 0, 16);
    pos += 16;

    SYN_ASSERT(pos == SYN_WG_INITIATION_SIZE);

    /* Send */
    int sent = syn_port_udp_sendto(wg->udp_sock, msg, pos, &wg->config.endpoint);
    if (sent != (int)pos) return false;

    wg->last_sent_ms = syn_port_get_tick_ms();
    wg->last_handshake_ms = wg->last_sent_ms;

    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Handshake: Consume Response
 * ═══════════════════════════════════════════════════════════════════════════ */

static bool wg_consume_response(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    uint8_t *h = wg->hs_hash;
    uint8_t *ck = wg->hs_chaining_key;
    uint8_t k[32], dh_result[32];

    if (len != SYN_WG_RESPONSE_SIZE) return false;

    /* Verify message type */
    if (load32_le(msg) != SYN_WG_MSG_RESPONSE) return false;

    /* Verify receiver_index matches our sender_index */
    uint32_t receiver = load32_le(msg + 8);
    if (receiver != wg->session.sender_index) return false;

    /* Save peer's sender index */
    wg->session.receiver_index = load32_le(msg + 4);

    /* Responder's ephemeral public key */
    const uint8_t *re_pub = msg + 12;

    /* C, k = KDF(C, E_pub_responder) */
    wg_mix_key(ck, k, re_pub, 32);
    /* H = HASH(H || E_pub_responder) */
    wg_mix_hash(h, re_pub, 32);

    /* DH: C, k = KDF(C, DH(E_priv_initiator, E_pub_responder)) */
    syn_x25519(dh_result, wg->hs_ephemeral_priv, re_pub);
    wg_mix_key(ck, k, dh_result, 32);

    /* DH: C, k = KDF(C, DH(S_priv_initiator, E_pub_responder)) */
    syn_x25519(dh_result, wg->config.private_key, re_pub);
    wg_mix_key(ck, k, dh_result, 32);

    /* PSK: C, t, k = KDF3(C, Q) — Q is preshared key */
    {
        uint8_t tau[32];
        wg_hkdf3(ck, tau, k, ck, wg->config.preshared_key, 32);
        wg_mix_hash(h, tau, 32);
    }

    /* Decrypt empty payload: AEAD(k, 0, "", H) */
    /* msg.empty = 16 bytes at offset 44 (just the tag, no ciphertext) */
    if (!wg_decrypt_and_hash(h, k, NULL, 0, msg + 44, NULL)) {
        return false;
    }

    /* Verify mac1 */
    {
        uint8_t expected_mac[16];
        wg_mac1(expected_mac, wg->public_key, msg, 60);
        uint8_t diff = 0;
        unsigned i;
        for (i = 0; i < 16; i++) diff |= expected_mac[i] ^ msg[60 + i];
        if (diff != 0) return false;
    }

    /* Derive transport keys: (T_send, T_recv) = KDF2(C, "") */
    wg_hkdf2(wg->session.send_key, wg->session.recv_key,
             ck, (const uint8_t *)"", 0);

    /* Initialize transport state */
    wg->session.send_counter = 0;
    wg->session.recv_counter = 0;
    wg->session.recv_bitmap  = 0;
    wg->session.established_ms = syn_port_get_tick_ms();

    /* Clear handshake secrets */
    memset(wg->hs_ephemeral_priv, 0, 32);
    memset(wg->hs_chaining_key, 0, 32);
    memset(wg->hs_hash, 0, 32);

    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Transport: Encrypt + Send
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_wg_send(SYN_WG *wg, const uint8_t *ip_packet, size_t len)
{
    SYN_ASSERT(wg != NULL);

    if (wg->state != SYN_WG_ESTABLISHED) return SYN_ERROR;

    size_t total = 16 + len + 16;  /* header(16) + encrypted + tag(16) */
    if (total > wg->tx_buf_size) return SYN_ERROR;

    uint8_t *msg = wg->tx_buf;
    uint8_t nonce[12];

    /* Header: type(4) + receiver_index(4) + counter(8) */
    store32_le(msg, SYN_WG_MSG_TRANSPORT);
    store32_le(msg + 4, wg->session.receiver_index);
    store64_le(msg + 8, wg->session.send_counter);

    /* Nonce: 4 zero bytes + 8-byte little-endian counter */
    memset(nonce, 0, 4);
    store64_le(nonce + 4, wg->session.send_counter);

    /* Encrypt payload */
    syn_aead_encrypt(wg->session.send_key, nonce,
                     NULL, 0,
                     ip_packet, len,
                     msg + 16,
                     msg + 16 + len);

    wg->session.send_counter++;

    int sent = syn_port_udp_sendto(wg->udp_sock, msg, total,
                                   &wg->config.endpoint);
    if (sent != (int)total) return SYN_ERROR;
    SYN_METRIC_ADD(wg_tx_bytes, len);
    wg->last_sent_ms = syn_port_get_tick_ms();
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Transport: Receive + Decrypt
 * ═══════════════════════════════════════════════════════════════════════════ */

static bool wg_replay_check(SYN_WgSession *s, uint64_t counter)
{
    if (counter > s->recv_counter) {
        /* New highest — shift window */
        uint64_t diff = counter - s->recv_counter;
        if (diff < 32) {
            s->recv_bitmap <<= diff;
            s->recv_bitmap |= 1;
        } else {
            s->recv_bitmap = 1;
        }
        s->recv_counter = counter;
        return true;
    }

    uint64_t diff = s->recv_counter - counter;
    if (diff >= 32) return false;  /* Too old */

    uint32_t bit = 1u << (uint32_t)diff;
    if (s->recv_bitmap & bit) return false;  /* Already seen */

    s->recv_bitmap |= bit;
    return true;
}

static bool wg_handle_transport(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    if (len < 32) return false;  /* Minimum: 16 header + 16 tag (empty) */

    /* Verify receiver index */
    uint32_t receiver = load32_le(msg + 4);
    if (receiver != wg->session.sender_index) return false;

    /* Extract counter */
    uint64_t counter = load64_le(msg + 8);

    /* Anti-replay */
    if (!wg_replay_check(&wg->session, counter)) return false;

    /* Build nonce */
    uint8_t nonce[12];
    memset(nonce, 0, 4);
    store64_le(nonce + 4, counter);

    /* Decrypt: payload starts at offset 16, tag is last 16 bytes */
    size_t ct_len = len - 32;  /* Subtract header(16) + tag(16) */
    const uint8_t *ct  = msg + 16;
    const uint8_t *tag = msg + 16 + ct_len;

    uint8_t *plain = wg->rx_buf;  /* Reuse rx_buf for decrypted output */
    if (ct_len > wg->rx_buf_size) return false;

    if (!syn_aead_decrypt(wg->session.recv_key, nonce,
                          NULL, 0, ct, ct_len, tag, plain)) {
        SYN_METRIC_INC(wg_errors);
        return false;
    }

    wg->last_recv_ms = syn_port_get_tick_ms();

    /* Deliver to user (ct_len == 0 means keepalive — no callback) */
    if (ct_len > 0 && wg->on_recv != NULL) {
        wg->on_recv(plain, ct_len, wg->user_ctx);
    }

    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Keepalive
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_send_keepalive(SYN_WG *wg)
{
    /* A keepalive is just a transport message with zero-length payload */
    syn_wg_send(wg, NULL, 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Public API
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_wg_init(SYN_WG *wg, const SYN_WgConfig *config,
                 SYN_SNTP *sntp,
                 uint8_t *rx_buf, size_t rx_buf_size,
                 uint8_t *tx_buf, size_t tx_buf_size)
{
    SYN_ASSERT(wg != NULL);
    SYN_ASSERT(config != NULL);
    SYN_ASSERT(sntp != NULL);
    SYN_ASSERT(rx_buf != NULL);
    SYN_ASSERT(tx_buf != NULL);

    memset(wg, 0, sizeof(*wg));
    wg->config      = *config;
    wg->sntp        = sntp;
    wg->state       = SYN_WG_DISCONNECTED;
    wg->udp_sock    = SYN_SOCKET_INVALID;
    wg->rx_buf      = rx_buf;
    wg->rx_buf_size = rx_buf_size;
    wg->tx_buf      = tx_buf;
    wg->tx_buf_size = tx_buf_size;

    /* Derive our public key */
    syn_x25519_pubkey(wg->public_key, wg->config.private_key);

    SYN_METRIC_REGISTER(wg_handshakes);
    SYN_METRIC_REGISTER(wg_tx_bytes);
    SYN_METRIC_REGISTER(wg_rx_bytes);
    SYN_METRIC_REGISTER(wg_errors);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Protothread task
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_PT_Status syn_wg_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_WG *wg = (SYN_WG *)task->user_data;
    SYN_ASSERT(wg != NULL);

    PT_BEGIN(pt);

    /* ── Wait for NTP sync ──────────────────────────────────────────── */
    PT_WAIT_UNTIL(pt, syn_sntp_is_synced(wg->sntp));

    /* ── Open UDP socket ────────────────────────────────────────────── */
    wg->udp_sock = syn_port_udp_open(0);
    if (wg->udp_sock == SYN_SOCKET_INVALID) {
        PT_TASK_DELAY_MS(pt, task, 5000);
        PT_RESTART(pt);
    }

    for (;;) {
        uint32_t now = syn_port_get_tick_ms();

        /* ── State: DISCONNECTED — initiate handshake ───────────── */
        if (wg->state == SYN_WG_DISCONNECTED) {
            if (wg_send_initiation(wg)) {
                wg->state = SYN_WG_HANDSHAKE_INIT;
            } else {
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
        }

        /* ── State: HANDSHAKE_INIT — waiting for response ───────── */
        if (wg->state == SYN_WG_HANDSHAKE_INIT) {
            now = syn_port_get_tick_ms();
            if ((now - wg->last_handshake_ms) >
                (uint32_t)SYN_WG_REKEY_TIMEOUT * 1000) {
                /* Timeout — retry */
                wg->state = SYN_WG_DISCONNECTED;
                PT_TASK_DELAY_MS(pt, task, 500);
                continue;
            }
        }

        /* ── State: ESTABLISHED — check session expiry ──────────── */
        if (wg->state == SYN_WG_ESTABLISHED) {
            now = syn_port_get_tick_ms();

            /* Session expired? */
            if ((now - wg->session.established_ms) >
                (uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000) {
                wg->state = SYN_WG_DISCONNECTED;
                continue;
            }

            /* Need rekey? */
            if ((now - wg->session.established_ms) >
                (uint32_t)SYN_WG_REKEY_AFTER_TIME * 1000) {
                wg->state = SYN_WG_DISCONNECTED;
                continue;
            }

            /* Keepalive needed? */
            if (wg->config.keepalive_interval_s > 0 &&
                (now - wg->last_sent_ms) >
                (uint32_t)wg->config.keepalive_interval_s * 1000) {
                wg_send_keepalive(wg);
            }
        }

        /* ── Poll for incoming UDP packets ──────────────────────── */
        {
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(wg->udp_sock,
                                          wg->rx_buf, wg->rx_buf_size,
                                          &from, 0);
            if (n > 0) {
                uint32_t msg_type = (n >= 4) ? load32_le(wg->rx_buf) : 0;

                if (msg_type == SYN_WG_MSG_RESPONSE &&
                    wg->state == SYN_WG_HANDSHAKE_INIT) {
                    if (wg_consume_response(wg, wg->rx_buf, (size_t)n)) {
                        wg->state = SYN_WG_ESTABLISHED;
                        wg->last_recv_ms = syn_port_get_tick_ms();
                    }
                } else if (msg_type == SYN_WG_MSG_TRANSPORT &&
                           wg->state == SYN_WG_ESTABLISHED) {
                    wg_handle_transport(wg, wg->rx_buf, (size_t)n);
                }
                /* MSG_COOKIE and unknown types are silently dropped */
            }
        }

        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_WG */
```


