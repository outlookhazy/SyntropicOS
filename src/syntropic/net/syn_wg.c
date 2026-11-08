#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG

/**
 * @file syn_wg.c
 * @brief WireGuard client — Noise_IKpsk2 handshake + transport.
 *
 * Implements the WireGuard protocol per the whitepaper:
 *   https://www.wireguard.com/papers/wireguard.pdf
 *
 * Noise protocol pattern: IKpsk2
 *   Initiator (us) knows responder's static public key.
 *   PSK mixed in at handshake step 2.
 */

#include "../crypto/syn_blake2s.h"
#include "../crypto/syn_chacha20poly1305.h"
#include "../crypto/syn_x25519.h"
#include "../port/syn_port_socket.h"
#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_metrics.h"
#include "../util/syn_random.h"
#include "syn_wg.h"

#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 *  Construction constants (from WireGuard spec)
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @name Internal constants
 * @{
 */
static const uint8_t WG_CONSTRUCTION[] =
    "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s"; /**< Noise construction string */
static const uint8_t WG_IDENTIFIER[] =
    "WireGuard v1 zx2c4 Jason@zx2c4.com";                       /**< Protocol identifier string */
static const uint8_t WG_LABEL_MAC1[] = "mac1----";              /**< MAC1 label                */
static const uint8_t WG_LABEL_COOKIE[] SYN_UNUSED = "cookie--"; /**< Cookie label              */
/** @} */

/* ═══════════════════════════════════════════════════════════════════════════
 *  Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Store 32-bit little-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/**
 * @brief Load 32-bit little-endian word.
 * @param p Source bytes.
 * @return 32-bit value.
 */
static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/**
 * @brief Store 64-bit little-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store64_le(uint8_t *p, uint64_t v)
{
    store32_le(p, (uint32_t)(v));
    store32_le(p + 4, (uint32_t)(v >> 32));
}

/**
 * @brief Load 64-bit little-endian word.
 * @param p Source bytes.
 * @return 64-bit value.
 */
static inline uint64_t load64_le(const uint8_t *p)
{
    return (uint64_t)load32_le(p) | ((uint64_t)load32_le(p + 4) << 32);
}

/**
 * @brief Store 32-bit big-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store32_be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

/**
 * @brief Store 64-bit big-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store64_be(uint8_t *p, uint64_t v)
{
    store32_be(p, (uint32_t)(v >> 32));
    store32_be(p + 4, (uint32_t)(v));
}

/**
 * @brief Zero sensitive memory without dead-store elimination.
 * @param v Target buffer.
 * @param n Byte count.
 */
static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n > 0U) {
        *p++ = 0U;
        n--;
    }
}

/**
 * @brief Constant-time byte-slice equality check.
 * @param a First buffer.
 * @param b Second buffer.
 * @param n Byte count.
 * @return true if all n bytes match, false otherwise.
 */
static bool syn_ct_memeq(const void *a, const void *b, size_t n)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t diff = 0U;
    for (size_t i = 0U; i < n; i++) {
        diff |= (uint8_t)(pa[i] ^ pb[i]);
    }
    return diff == 0U;
}

/* sender index — now using syn_random */
#if SYN_USE_METRICS
SYN_METRIC_DECLARE(wg_handshakes, "wg_handshakes", "Total WireGuard handshakes",
                   SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_tx_bytes, "wg_tx_bytes", "Total bytes sent over WG", SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_rx_bytes, "wg_rx_bytes", "Total bytes received over WG",
                   SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(wg_errors, "wg_errors", "Total WG processing errors", SYN_METRIC_TYPE_COUNTER);
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 *  HKDF (HMAC-BLAKE2s based, per WireGuard spec)
 * ═══════════════════════════════════════════════════════════════════════════ */

/** HKDF-Extract + Expand producing 2 outputs.
 * @param out1,out2 Output buffers (32 bytes each).
 * @param ck        Chaining key (32 bytes).
 * @param input     Input data.
 * @param input_len Length of input data.
 */
static void wg_hkdf2(uint8_t out1[32], uint8_t out2[32], const uint8_t ck[32], const uint8_t *input,
                     size_t input_len)
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

    syn_secure_zero(prk, sizeof(prk));
    syn_secure_zero(tmp, sizeof(tmp));
}

/** HKDF-Extract + Expand producing 3 outputs.
 * @param out1,out2,out3 Output buffers (32 bytes each).
 * @param ck             Chaining key (32 bytes).
 * @param input          Input data.
 * @param input_len      Length of input data.
 */
static void wg_hkdf3(uint8_t out1[32], uint8_t out2[32], uint8_t out3[32], const uint8_t ck[32],
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

    syn_secure_zero(prk, sizeof(prk));
    syn_secure_zero(tmp, sizeof(tmp));
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Noise handshake helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @brief Mix hash: H = BLAKE2s(H || data).
 * @param h    Running hash (32 bytes).
 * @param data Input data.
 * @param len  Length of input data.
 */
static void wg_mix_hash(uint8_t h[32], const void *data, size_t len)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32);
    syn_blake2s_update(&ctx, h, 32);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, h);
}

/** @brief Mix key: (CK, k) = HKDF(CK, input).
 * @param ck    Running chaining key (32 bytes).
 * @param k     Output key (32 bytes).
 * @param input Input data.
 * @param len   Length of input data.
 */
static void wg_mix_key(uint8_t ck[32], uint8_t k[32], const uint8_t *input, size_t len)
{
    wg_hkdf2(ck, k, ck, input, len);
}

/** @brief Encrypt-and-hash: encrypt plaintext, mix ciphertext+tag into hash.
 * @param h         Running hash (32 bytes).
 * @param k         Encryption key (32 bytes).
 * @param plain     Plaintext input.
 * @param plain_len Plaintext length.
 * @param ct        Ciphertext output.
 * @param tag       MAC tag output (16 bytes).
 */
static void wg_encrypt_and_hash(uint8_t h[32], const uint8_t k[32], const uint8_t *plain,
                                size_t plain_len, uint8_t *ct, uint8_t tag[16])
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

/** @brief Decrypt-and-hash: verify + decrypt, mix ciphertext+tag into hash.
 * @param h      Running hash (32 bytes).
 * @param k      Decryption key (32 bytes).
 * @param ct     Ciphertext input.
 * @param ct_len Ciphertext length.
 * @param tag    MAC tag input (16 bytes).
 * @param plain  Plaintext output.
 * @return true if decryption and verification succeeded.
 */
static bool wg_decrypt_and_hash(uint8_t h[32], const uint8_t k[32], const uint8_t *ct,
                                size_t ct_len, const uint8_t tag[16], uint8_t *plain)
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

/** @brief Write a 12-byte TAI64N timestamp from NTP time.
 * @param out  Output buffer (12 bytes).
 * @param sntp NTP time source.
 */
static void wg_tai64n(uint8_t out[12], const SYN_SNTP *sntp)
{
    uint32_t epoch_s = syn_sntp_get_epoch_s(sntp);
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

/** @brief Compute mac1 = keyed BLAKE2s(HASH("mac1----" || peer_pub), msg).
 * @param mac      Output MAC (16 bytes).
 * @param peer_pub Peer's public key.
 * @param msg      Message data.
 * @param msg_len  Length of message data.
 */
static void wg_mac1(uint8_t mac[16], const uint8_t peer_pub[32], const uint8_t *msg, size_t msg_len)
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
    syn_secure_zero(key, sizeof(key));
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Handshake: Initiation (we are the initiator)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Compute X25519 DH, mix into Noise chaining key, and verify non-zero in constant time.
 * @param ck   [in,out] Running 32-byte Noise chaining key.
 * @param k    [out] Derived 32-byte AEAD key.
 * @param priv 32-byte local private scalar.
 * @param pub  32-byte peer public point.
 * @return 0U if the 32-byte shared secret is all-zero (RFC 7748 Section 6.1), 1U otherwise.
 */
static uint8_t wg_dh_mix_key(uint8_t ck[32], uint8_t k[32], const uint8_t priv[32],
                             const uint8_t pub[32])
{
    static const uint8_t zero32[32] = {0U};
    uint8_t dh[32];
    syn_x25519(dh, priv, pub);
    uint8_t valid = syn_ct_memeq(dh, zero32, 32U) ? 0U : 1U;
    wg_mix_key(ck, k, dh, 32U);
    syn_secure_zero(dh, sizeof(dh));
    return valid;
}

/**
 * @brief Initialize Noise_IKpsk2 chaining key and handshake hash with responder static public key.
 * @param ck       [out] 32-byte initial chaining key.
 * @param h        [out] 32-byte initial handshake hash.
 * @param peer_pub 32-byte responder static public key.
 */
static void wg_init_noise_state(uint8_t ck[32], uint8_t h[32], const uint8_t peer_pub[32])
{
    SYN_BLAKE2s tmp;
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1U, ck, 32U);
    syn_blake2s_init(&tmp, 32U);
    syn_blake2s_update(&tmp, ck, 32U);
    syn_blake2s_update(&tmp, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1U);
    syn_blake2s_final(&tmp, h);
    wg_mix_hash(h, peer_pub, 32U);
}

/** @brief Build and send a Noise_IKpsk2 handshake initiation message.
 * @param wg Client context.
 * @return true if message was built and sent.
 */
static bool wg_send_initiation(SYN_WG *wg)
{
    if (wg->tx_buf_size < SYN_WG_INITIATION_SIZE) {
        return false;
    }

    uint8_t eph_priv[32];
    if (syn_random_fill(eph_priv, 32U) != SYN_OK) {
        return false;
    }
    syn_x25519_clamp(eph_priv);

    uint8_t ck_tmp[32];
    uint8_t h_tmp[32];
    uint8_t eph_pub[32];
    uint8_t k[32];
    uint8_t timestamp[12];
    uint8_t *msg = wg->tx_buf;

    wg_init_noise_state(ck_tmp, h_tmp, wg->config.peer_public_key);
    syn_x25519_pubkey(eph_pub, eph_priv);

    uint32_t init_sender_idx = syn_random_u32();
    size_t pos = 0U;
    store32_le(msg + pos, SYN_WG_MSG_INITIATION);
    pos += 4U;
    store32_le(msg + pos, init_sender_idx);
    pos += 4U;

    memcpy(msg + pos, eph_pub, 32U);
    wg_mix_key(ck_tmp, k, eph_pub, 32U);
    wg_mix_hash(h_tmp, eph_pub, 32U);
    pos += 32U;

    /* DH #1 & DH #2: reject low-order peer static public key in constant time */
    uint8_t dh_ok = wg_dh_mix_key(ck_tmp, k, eph_priv, wg->config.peer_public_key);
    wg_encrypt_and_hash(h_tmp, k, wg->public_key, 32U, msg + pos, msg + pos + 32U);
    pos += 48U;

    dh_ok &= wg_dh_mix_key(ck_tmp, k, wg->config.private_key, wg->config.peer_public_key);
    wg_tai64n(timestamp, wg->sntp);
    wg_encrypt_and_hash(h_tmp, k, timestamp, 12U, msg + pos, msg + pos + 12U);
    pos += 28U;

    wg_mac1(msg + pos, wg->config.peer_public_key, msg, pos);
    pos += 16U;
    memset(msg + pos, 0, 16U);
    pos += 16U;

    SYN_ASSERT(pos == SYN_WG_INITIATION_SIZE);

    /* Commit handshake state only after UDP transmission succeeds */
    bool sent_ok = (dh_ok != 0U) &&
                   (syn_port_udp_sendto(wg->udp_sock, msg, pos, &wg->config.endpoint) == (int)pos);
    if (sent_ok) {
        memcpy(wg->hs_ephemeral_priv, eph_priv, 32U);
        memcpy(wg->hs_chaining_key, ck_tmp, 32U);
        memcpy(wg->hs_hash, h_tmp, 32U);
        wg->hs_sender_index = init_sender_idx;
        if (wg->state != SYN_WG_ESTABLISHED) {
            wg->session.sender_index = init_sender_idx;
        }
        wg->last_sent_ms = syn_port_get_tick_ms();
        wg->last_handshake_ms = wg->last_sent_ms;
    } else {
        syn_secure_zero(msg, pos);
    }

    syn_secure_zero(eph_priv, sizeof(eph_priv));
    syn_secure_zero(eph_pub, sizeof(eph_pub));
    syn_secure_zero(ck_tmp, sizeof(ck_tmp));
    syn_secure_zero(h_tmp, sizeof(h_tmp));
    syn_secure_zero(k, sizeof(k));
    syn_secure_zero(timestamp, sizeof(timestamp));
    return sent_ok;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Handshake: Consume Response
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @brief Verify mac1 on an incoming 92-byte handshake response before DH work.
 * @param wg  Client context.
 * @param msg Received 92-byte response message.
 * @return true if mac1 authenticates against our static public key.
 */
static bool wg_verify_response_mac1(const SYN_WG *wg, const uint8_t *msg)
{
    uint8_t expected_mac[16];
    wg_mac1(expected_mac, wg->public_key, msg, 60U);
    bool ok = syn_ct_memeq(expected_mac, msg + 60U, 16U);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    return ok;
}

/** @brief Parse and validate a handshake response, deriving session keys.
 * @param wg  Client context.
 * @param msg Received message data.
 * @param len Received message length.
 * @return true if response was valid and session established.
 */
static bool wg_consume_response(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    uint8_t h_tmp[32];
    uint8_t ck_tmp[32];
    uint8_t k[32];
    uint8_t buf[32];

    if ((len != SYN_WG_RESPONSE_SIZE) || (load32_le(msg) != SYN_WG_MSG_RESPONSE)) {
        return false;
    }

    /* Verify receiver_index and mac1 before any X25519 DH or handshake state mutation */
    uint32_t expected_idx = wg->session.sender_index;
    if ((wg->state == SYN_WG_ESTABLISHED) && wg->rekey_in_progress) {
        expected_idx = wg->hs_sender_index;
    }
    if ((load32_le(msg + 8U) != expected_idx) || !wg_verify_response_mac1(wg, msg)) {
        return false;
    }

    memcpy(h_tmp, wg->hs_hash, 32U);
    memcpy(ck_tmp, wg->hs_chaining_key, 32U);

    const uint8_t *re_pub = msg + 12U;
    wg_mix_key(ck_tmp, k, re_pub, 32U);
    wg_mix_hash(h_tmp, re_pub, 32U);

    /* DH #1 & DH #2: reject low-order responder ephemeral public key in constant time */
    uint8_t dh_ok = wg_dh_mix_key(ck_tmp, k, wg->hs_ephemeral_priv, re_pub);
    dh_ok &= wg_dh_mix_key(ck_tmp, k, wg->config.private_key, re_pub);

    wg_hkdf3(ck_tmp, buf, k, ck_tmp, wg->config.preshared_key, 32U);
    wg_mix_hash(h_tmp, buf, 32U);

    bool ok = (dh_ok != 0U) && wg_decrypt_and_hash(h_tmp, k, NULL, 0U, msg + 44U, NULL);
    if (ok) {
        wg->session.sender_index = expected_idx;
        wg->session.receiver_index = load32_le(msg + 4U);
        wg_hkdf2(wg->session.send_key, wg->session.recv_key, ck_tmp, (const uint8_t *)"", 0U);
        wg->session.send_counter = 0U;
        wg->session.recv_counter = 0U;
        wg->session.recv_bitmap = 0U;
        wg->session.established_ms = syn_port_get_tick_ms();
        wg->rekey_in_progress = false;
        wg->hs_sender_index = 0U;
        syn_secure_zero(wg->hs_ephemeral_priv, sizeof(wg->hs_ephemeral_priv));
        syn_secure_zero(wg->hs_chaining_key, sizeof(wg->hs_chaining_key));
        syn_secure_zero(wg->hs_hash, sizeof(wg->hs_hash));
    }

    syn_secure_zero(h_tmp, sizeof(h_tmp));
    syn_secure_zero(ck_tmp, sizeof(ck_tmp));
    syn_secure_zero(k, sizeof(k));
    syn_secure_zero(buf, sizeof(buf));
    return ok;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Transport: Encrypt + Send
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_wg_send(SYN_WG *wg, const uint8_t *ip_packet, size_t len)
{
    if ((wg == NULL) || ((ip_packet == NULL) && (len > 0U))) {
        return SYN_ERROR;
    }
    if ((wg->state != SYN_WG_ESTABLISHED) ||
        (wg->session.send_counter >= SYN_WG_REJECT_AFTER_MESSAGES)) {
        return SYN_ERROR;
    }

    if ((wg->tx_buf_size < SYN_WG_TRANSPORT_OVERHEAD) ||
        (len > (wg->tx_buf_size - SYN_WG_TRANSPORT_OVERHEAD))) {
        return SYN_ERROR;
    }
    size_t total = len + SYN_WG_TRANSPORT_OVERHEAD;

    uint8_t *msg = wg->tx_buf;
    uint8_t nonce[12];

    /* Header: type(4) + receiver_index(4) + counter(8) */
    store32_le(msg, SYN_WG_MSG_TRANSPORT);
    store32_le(msg + 4U, wg->session.receiver_index);
    store64_le(msg + 8U, wg->session.send_counter);

    /* Nonce: 4 zero bytes + 8-byte little-endian counter */
    memset(nonce, 0, 4U);
    store64_le(nonce + 4U, wg->session.send_counter);

    /* Encrypt payload */
    syn_aead_encrypt(wg->session.send_key, nonce, NULL, 0U, ip_packet, len, msg + 16U,
                     msg + 16U + len);

    wg->session.send_counter++;

    int sent = syn_port_udp_sendto(wg->udp_sock, msg, total, &wg->config.endpoint);
    if (sent != (int)total) {
        return SYN_ERROR;
    }
    wg->tx_bytes += (uint64_t)len;
    SYN_METRIC_ADD(wg_tx_bytes, len);
    wg->last_sent_ms = syn_port_get_tick_ms();
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Transport: Receive + Decrypt
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @brief Anti-replay check without state mutation.
 * @param s       Active session.
 * @param counter Received counter value.
 * @return true if counter is new and within window.
 */
static bool wg_replay_test(const SYN_WgSession *s, uint64_t counter)
{
    if (counter >= SYN_WG_REJECT_AFTER_MESSAGES) {
        return false;
    }
    if (counter > s->recv_counter) {
        return true;
    }

    uint64_t diff = s->recv_counter - counter;
    if (diff >= 64U) {
        return false; /* Too old */
    }

    uint64_t bit = 1ULL << (uint32_t)diff;
    if ((s->recv_bitmap & bit) != 0ULL) {
        return false; /* Already seen */
    }

    return true;
}

/** @brief Commit counter into anti-replay sliding window bitmap.
 * @param s       Active session.
 * @param counter Validated counter value.
 */
static void wg_replay_commit(SYN_WgSession *s, uint64_t counter)
{
    if (counter > s->recv_counter) {
        uint64_t diff = counter - s->recv_counter;
        if (diff < 64U) {
            s->recv_bitmap <<= (uint32_t)diff;
            s->recv_bitmap |= 1ULL;
        } else {
            s->recv_bitmap = 1ULL;
        }
        s->recv_counter = counter;
        return;
    }

    uint64_t diff = s->recv_counter - counter;
    if (diff < 64U) {
        s->recv_bitmap |= (1ULL << (uint32_t)diff);
    }
}

/** @brief Anti-replay check and commit in one step (convenience / test helper).
 * @param s       Active session.
 * @param counter Received counter value.
 * @return true if counter is new and within window.
 */
SYN_UNUSED static bool wg_replay_check(SYN_WgSession *s, uint64_t counter)
{
    if (!wg_replay_test(s, counter)) {
        return false;
    }
    wg_replay_commit(s, counter);
    return true;
}

/** @brief Strip WireGuard 16-byte transport padding using inner IPv4/IPv6 header length.
 * @param plain  Decrypted transport payload buffer.
 * @param ct_len Decrypted ciphertext byte length.
 * @return Actual IP packet length, or ct_len for non-IP raw payloads.
 */
static size_t wg_unpad_ip_len(const uint8_t *plain, size_t ct_len)
{
    if (ct_len >= 20U && (plain[0] >> 4U) == 4U) {
        size_t ihl_bytes = (size_t)(plain[0] & 0x0FU) * 4U;
        size_t ip4_len = ((size_t)plain[2] << 8U) | (size_t)plain[3];
        if (ihl_bytes >= 20U && ip4_len >= ihl_bytes && ip4_len <= ct_len) {
            return ip4_len;
        }
    } else if (ct_len >= 40U && (plain[0] >> 4U) == 6U) {
        size_t ip6_len = 40U + (((size_t)plain[4] << 8U) | (size_t)plain[5]);
        if (ip6_len <= ct_len) {
            return ip6_len;
        }
    }
    return ct_len;
}

/** @brief Decrypt and deliver an incoming WireGuard transport message.
 * @param wg  Client context.
 * @param msg Received message data.
 * @param len Received message length.
 * @return true if message was valid and delivered.
 */
static bool wg_handle_transport(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    if (len < 32)
        return false; /* Minimum: 16 header + 16 tag (empty) */

    /* Verify receiver index */
    uint32_t receiver = load32_le(msg + 4);
    if (receiver != wg->session.sender_index)
        return false;

    /* Extract counter */
    uint64_t counter = load64_le(msg + 8);

    /* Anti-replay pre-check (non-mutating) */
    if (!wg_replay_test(&wg->session, counter))
        return false;

    /* Build nonce */
    uint8_t nonce[12];
    memset(nonce, 0, 4);
    store64_le(nonce + 4, counter);

    /* Decrypt: payload starts at offset 16, tag is last 16 bytes */
    size_t ct_len = len - 32; /* Subtract header(16) + tag(16) */
    const uint8_t *ct = msg + 16;
    const uint8_t *tag = msg + 16 + ct_len;

    uint8_t *plain = wg->rx_buf; /* Reuse rx_buf for decrypted output */
    if (ct_len > wg->rx_buf_size)
        return false;

    if (!syn_aead_decrypt(wg->session.recv_key, nonce, NULL, 0, ct, ct_len, tag, plain)) {
        SYN_METRIC_INC(wg_errors);
        return false;
    }

    size_t pkt_len = wg_unpad_ip_len(plain, ct_len);

    /* Only commit anti-replay counter after AEAD authentication succeeds */
    wg_replay_commit(&wg->session, counter);
    wg->rx_bytes += (uint64_t)pkt_len;
    SYN_METRIC_ADD(wg_rx_bytes, pkt_len);

    wg->last_recv_ms = syn_port_get_tick_ms();

    /* Deliver to user (pkt_len == 0 means keepalive — no callback) */
    if (pkt_len > 0U && wg->on_recv != NULL) {
        wg->on_recv(plain, pkt_len, wg->user_ctx);
    }

    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Keepalive
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @brief Send an empty (keepalive) transport message.
 * @param wg Client context.
 */
static void wg_send_keepalive(SYN_WG *wg)
{
    /* A keepalive is just a transport message with zero-length payload */
    syn_wg_send(wg, NULL, 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Public API
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_wg_init(SYN_WG *wg, const SYN_WgConfig *config, SYN_SNTP *sntp, uint8_t *rx_buf,
                 size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size)
{
    SYN_ASSERT(wg != NULL);
    SYN_ASSERT(config != NULL);
    SYN_ASSERT(sntp != NULL);
    SYN_ASSERT(rx_buf != NULL);
    SYN_ASSERT(tx_buf != NULL);

    memset(wg, 0, sizeof(*wg));
    wg->config = *config;
    wg->sntp = sntp;
    wg->state = SYN_WG_DISCONNECTED;
    wg->udp_sock = SYN_SOCKET_INVALID;
    wg->rx_buf = rx_buf;
    wg->rx_buf_size = rx_buf_size;
    wg->tx_buf = tx_buf;
    wg->tx_buf_size = tx_buf_size;

    /* Derive our public key */
    syn_x25519_pubkey(wg->public_key, wg->config.private_key);

    SYN_METRIC_REGISTER(wg_handshakes);
    SYN_METRIC_REGISTER(wg_tx_bytes);
    SYN_METRIC_REGISTER(wg_rx_bytes);
    SYN_METRIC_REGISTER(wg_errors);
}

/**
 * @brief Check if WireGuard UDP socket is open and readable.
 * @param wg Client context.
 * @return true if socket has pending data or error.
 */
static bool wg_socket_readable(const SYN_WG *wg)
{
    return wg->udp_sock != SYN_SOCKET_INVALID && syn_port_udp_readable(wg->udp_sock);
}

/**
 * @brief Check if pending handshake initiation has exceeded rekey timeout.
 * @param wg Client context.
 * @return true if in SYN_WG_HANDSHAKE_INIT and timed out.
 */
static bool wg_handshake_timed_out(const SYN_WG *wg)
{
    return wg->state == SYN_WG_HANDSHAKE_INIT && (syn_port_get_tick_ms() - wg->last_handshake_ms) >
                                                     (uint32_t)SYN_WG_REKEY_TIMEOUT * 1000U;
}

/**
 * @brief Check whether hitless rekey initiation or rekey retry is due on an active session.
 * @param wg  Client context.
 * @param now Current millisecond tick.
 * @return true if a rekey initiation packet should be transmitted.
 */
static bool wg_rekey_due(const SYN_WG *wg, uint32_t now)
{
    uint32_t rekey_ms = (uint32_t)SYN_WG_REKEY_AFTER_TIME * 1000U;
    bool time_due = (now - wg->session.established_ms) > rekey_ms;
    bool msg_due = wg->session.send_counter >= SYN_WG_REKEY_AFTER_MESSAGES;
    if (!time_due && !msg_due) {
        return false;
    }
    if (!wg->rekey_in_progress) {
        return true;
    }
    return (now - wg->last_handshake_ms) > ((uint32_t)SYN_WG_REKEY_TIMEOUT * 1000U);
}

/**
 * @brief Check if WireGuard task has pending work to process.
 * @param wg Pointer to WireGuard state structure.
 * @return true if work pending, false otherwise.
 */
static bool wg_has_work(const SYN_WG *wg)
{
    if (wg == NULL)
        return false;
    if (wg->state == SYN_WG_DISCONNECTED || wg_socket_readable(wg) || wg_handshake_timed_out(wg))
        return true;

    if (wg->state == SYN_WG_ESTABLISHED) {
        uint32_t now = syn_port_get_tick_ms();
        if ((now - wg->session.established_ms) > (uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U ||
            wg_rekey_due(wg, now)) {
            return true;
        }
        if (wg->config.keepalive_interval_s > 0U &&
            (now - wg->last_sent_ms) > (uint32_t)wg->config.keepalive_interval_s * 1000U) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Validate task arguments for syn_wg_task.
 * @param pt   Protothread pointer.
 * @param task Task descriptor.
 * @return true if arguments and SNTP pointer are non-NULL.
 */
static bool wg_task_valid(const SYN_PT *pt, const SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return false;
    }
    const SYN_WG *wg = (const SYN_WG *)task->user_data;
    return wg->sntp != NULL;
}

/**
 * @brief Evaluate established-session rekey/reject expiry and keepalive timers.
 * @param wg Client context.
 * @return true if session expired and transitioned to DISCONNECTED, false otherwise.
 */
static bool wg_step_established_timers(SYN_WG *wg)
{
    if (wg->state != SYN_WG_ESTABLISHED) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    if ((now - wg->session.established_ms) > (uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U) {
        syn_secure_zero(&wg->session, sizeof(wg->session));
        wg->rekey_in_progress = false;
        wg->hs_sender_index = 0U;
        wg->state = SYN_WG_DISCONNECTED;
        return true;
    }

    if (wg_rekey_due(wg, now)) {
        wg->last_handshake_ms = now;
        wg->rekey_in_progress = true;
        (void)wg_send_initiation(wg);
    }

    if (wg->config.keepalive_interval_s > 0U &&
        (now - wg->last_sent_ms) > (uint32_t)wg->config.keepalive_interval_s * 1000U) {
        wg_send_keepalive(wg);
    }
    return false;
}

/**
 * @brief Dispatch a received WireGuard UDP datagram by message type.
 * @param wg  Client context.
 * @param len Datagram byte length (> 0).
 */
static void wg_process_datagram(SYN_WG *wg, size_t len)
{
    uint32_t msg_type = (len >= 4U) ? load32_le(wg->rx_buf) : 0U;
    bool accepting_resp = (wg->state == SYN_WG_HANDSHAKE_INIT) ||
                          (wg->state == SYN_WG_ESTABLISHED && wg->rekey_in_progress);

    if (msg_type == SYN_WG_MSG_RESPONSE && accepting_resp) {
        if (wg_consume_response(wg, wg->rx_buf, len)) {
            SYN_METRIC_INC(wg_handshakes);
            wg->state = SYN_WG_ESTABLISHED;
            wg->last_recv_ms = syn_port_get_tick_ms();
            wg_send_keepalive(wg);
        }
    } else if (msg_type == SYN_WG_MSG_TRANSPORT && wg->state == SYN_WG_ESTABLISHED) {
        (void)wg_handle_transport(wg, wg->rx_buf, len);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Protothread task
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_PT_Status syn_wg_task(SYN_PT *pt, SYN_Task *task)
{
    if (!wg_task_valid(pt, task)) {
        return PT_EXITED;
    }
    SYN_WG *wg = (SYN_WG *)task->user_data;

    PT_BEGIN(pt);

    /* ── Wait for NTP sync ──────────────────────────────────────────── */
    PT_WAIT_UNTIL(pt, syn_sntp_is_synced(wg->sntp));

    for (;;) {
        if (wg->udp_sock == SYN_SOCKET_INVALID) {
            wg->udp_sock = syn_port_udp_open(0);
            if (wg->udp_sock == SYN_SOCKET_INVALID) {
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
        }

        if (wg->state == SYN_WG_DISCONNECTED) {
            if (!wg_send_initiation(wg)) {
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
            wg->state = SYN_WG_HANDSHAKE_INIT;
        }

        if (wg_handshake_timed_out(wg)) {
            wg->state = SYN_WG_DISCONNECTED;
            PT_TASK_DELAY_MS(pt, task, 500);
            continue;
        }

        if (wg_step_established_timers(wg)) {
            continue;
        }

        SYN_SockAddr from;
        int n = syn_port_udp_recvfrom(wg->udp_sock, wg->rx_buf, wg->rx_buf_size, &from, 0);
        if (n > 0) {
            wg_process_datagram(wg, (size_t)n);
            if (wg_socket_readable(wg)) {
                PT_CONTINUE_IF_NEEDED(pt, task);
                continue;
            }
        } else if (wg_socket_readable(wg)) {
            PT_PASS(pt);
            continue;
        }

        PT_WAIT_UNTIL(pt, wg_has_work(wg));
    }

    PT_END(pt);
}

void syn_wg_disconnect(SYN_WG *wg)
{
    if (wg == NULL) {
        return;
    }

    if (wg->udp_sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(wg->udp_sock);
        wg->udp_sock = SYN_SOCKET_INVALID;
    }

    wg->state = SYN_WG_DISCONNECTED;
    wg->rekey_in_progress = false;
    wg->hs_sender_index = 0U;
    wg->tx_bytes = 0U;
    wg->rx_bytes = 0U;
    syn_secure_zero(&wg->session, sizeof(wg->session));
    syn_secure_zero(wg->hs_ephemeral_priv, sizeof(wg->hs_ephemeral_priv));
    syn_secure_zero(wg->hs_chaining_key, sizeof(wg->hs_chaining_key));
    syn_secure_zero(wg->hs_hash, sizeof(wg->hs_hash));
}

SYN_Status syn_wg_get_stats(const SYN_WG *wg, SYN_WgStats *stats)
{
    if (wg == NULL || stats == NULL)
        return SYN_INVALID_PARAM;

    memset(stats, 0, sizeof(*stats));
    stats->is_established = (wg->state == SYN_WG_ESTABLISHED);

    if (stats->is_established) {
        uint32_t now = syn_port_get_tick_ms();
        stats->handshake_age_sec = (now - wg->session.established_ms) / 1000;
        stats->tx_bytes = wg->tx_bytes;
        stats->rx_bytes = wg->rx_bytes;
    }

    return SYN_OK;
}

#endif /* SYN_USE_WG */
