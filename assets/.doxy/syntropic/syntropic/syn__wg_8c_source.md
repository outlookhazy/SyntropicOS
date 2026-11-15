

# File syn\_wg.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.c**](syn__wg_8c.md)

[Go to the documentation of this file](syn__wg_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG


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

static const uint8_t WG_CONSTRUCTION[] =
    "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s"; 
static const uint8_t WG_IDENTIFIER[] =
    "WireGuard v1 zx2c4 Jason@zx2c4.com";                       
static const uint8_t WG_LABEL_MAC1[] = "mac1----";              
static const uint8_t WG_LABEL_COOKIE[] SYN_UNUSED = "cookie--"; 

/* ═══════════════════════════════════════════════════════════════════════════
 *  Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static inline void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline void store64_le(uint8_t *p, uint64_t v)
{
    store32_le(p, (uint32_t)(v));
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
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

static inline void store64_be(uint8_t *p, uint64_t v)
{
    store32_be(p, (uint32_t)(v >> 32));
    store32_be(p + 4, (uint32_t)(v));
}

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n > 0U) {
        *p++ = 0U;
        n--;
    }
}

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

static void wg_hkdf2(uint8_t out1[32], uint8_t out2[32], const uint8_t ck[32], const uint8_t *input,
                     size_t input_len)
{
    uint8_t prk[32];
    uint8_t tmp[33];

    /* Extract */
    syn_hmac_blake2s(ck, 32U, input, input_len, prk);

    /* Expand: T1 = HMAC(PRK, 0x01) */
    tmp[0] = 0x01U;
    syn_hmac_blake2s(prk, 32U, tmp, 1U, out1);

    /* Expand: T2 = HMAC(PRK, T1 || 0x02) */
    memcpy(tmp, out1, 32U);
    tmp[32] = 0x02U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, out2);

    syn_secure_zero(prk, sizeof(prk));
    syn_secure_zero(tmp, sizeof(tmp));
}

static void wg_hkdf3_step1(uint8_t prk[32], uint8_t out1[32], const uint8_t ck[32],
                           const uint8_t *input, size_t input_len)
{
    uint8_t one = 0x01U;
    syn_hmac_blake2s(ck, 32U, input, input_len, prk);
    syn_hmac_blake2s(prk, 32U, &one, 1U, out1);
}

static void wg_hkdf3_step2(uint8_t out2[32], uint8_t out3[32], uint8_t prk[32],
                           const uint8_t out1[32])
{
    uint8_t tmp[33];
    memcpy(tmp, out1, 32U);
    tmp[32] = 0x02U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, out2);
    memcpy(tmp, out2, 32U);
    tmp[32] = 0x03U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, out3);
    syn_secure_zero(prk, 32U);
    syn_secure_zero(tmp, sizeof(tmp));
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

static void wg_mix_key(uint8_t ck[32], uint8_t k[32], const uint8_t *input, size_t len)
{
    wg_hkdf2(ck, k, ck, input, len);
}

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

static uint8_t wg_dh_finish_mix_key(uint8_t ck[32], uint8_t k[32], uint8_t dh[32])
{
    uint8_t valid = syn_x25519_is_nonzero(dh) ? 1U : 0U;
    wg_mix_key(ck, k, dh, 32U);
    syn_secure_zero(dh, 32U);
    return valid;
}

static void wg_hs_work_wipe(SYN_WgHsWork *w)
{
    syn_secure_zero(w, sizeof(*w));
}

static void wg_clear_handshake_state(SYN_WG *wg)
{
    wg->hs_valid = false;
    wg->hs_sender_index = 0U;
    syn_secure_zero(wg->hs_ephemeral_priv, sizeof(wg->hs_ephemeral_priv));
    syn_secure_zero(wg->hs_chaining_key, sizeof(wg->hs_chaining_key));
    syn_secure_zero(wg->hs_hash, sizeof(wg->hs_hash));
    wg_hs_work_wipe(&wg->hs_work);
}

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

static void wg_init_start(SYN_WG *wg, bool is_rekey)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_hs_work_wipe(w);
    PT_INIT(&w->hs_pt);
    w->active = true;
    w->is_resp = false;
    w->is_rekey = is_rekey;
}

static bool wg_init_prepare(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t rnd[36];
    if ((wg->tx_buf_size < SYN_WG_INITIATION_SIZE) ||
        (syn_random_fill(rnd, sizeof(rnd)) != SYN_OK)) {
        return false;
    }
    memcpy(w->eph_priv, rnd, 32U);
    syn_x25519_clamp(w->eph_priv);
    w->expected_idx = load32_le(rnd + 32U);
    syn_secure_zero(rnd, sizeof(rnd));
    return true;
}

static void wg_init_noise_and_eph(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_init_noise_state(w->ck, w->h, wg->config.peer_public_key);
    (void)syn_x25519_pubkey_init(&w->x25519, w->eph_priv);
}

static void wg_init_mix_eph_key(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    wg_mix_key(w->ck, k, w->eph_pub, 32U);
    syn_secure_zero(k, sizeof(k));
}

static void wg_init_start_dh_es(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_mix_hash(w->h, w->eph_pub, 32U);
    (void)syn_x25519_init(&w->x25519, w->eph_priv, wg->config.peer_public_key);
}

static void wg_init_mix_es(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    w->dh_ok = wg_dh_finish_mix_key(w->ck, k, w->tmp_key);
    memcpy(w->tmp_key, k, 32U);
    syn_secure_zero(k, sizeof(k));
}

static void wg_init_enc_static(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_encrypt_and_hash(w->h, w->tmp_key, wg->public_key, 32U, w->enc_static, w->enc_static + 32U);
    syn_secure_zero(w->tmp_key, 32U);
    (void)syn_x25519_init(&w->x25519, wg->config.private_key, wg->config.peer_public_key);
}

static void wg_init_mix_ss(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    w->dh_ok &= wg_dh_finish_mix_key(w->ck, k, w->tmp_key);
    memcpy(w->tmp_key, k, 32U);
    syn_secure_zero(k, sizeof(k));
}

static void wg_init_enc_ts(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t timestamp[12];
    wg_tai64n(timestamp, wg->sntp);
    wg_encrypt_and_hash(w->h, w->tmp_key, timestamp, 12U, w->enc_ts, w->enc_ts + 12U);
    syn_secure_zero(w->tmp_key, 32U);
    syn_secure_zero(timestamp, sizeof(timestamp));
}

static bool wg_init_mac_and_send(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t *msg = wg->tx_buf;
    uint32_t init_sender_idx = w->expected_idx;

    store32_le(msg + 0U, SYN_WG_MSG_INITIATION);
    store32_le(msg + 4U, init_sender_idx);
    memcpy(msg + 8U, w->eph_pub, 32U);
    memcpy(msg + 40U, w->enc_static, 48U);
    memcpy(msg + 88U, w->enc_ts, 28U);
    wg_mac1(msg + 116U, wg->config.peer_public_key, msg, 116U);
    memset(msg + 132U, 0, 16U);

    int sent = -1;
    if (w->dh_ok != 0U) {
        sent = syn_port_udp_sendto(wg->udp_sock, msg, SYN_WG_INITIATION_SIZE, &wg->config.endpoint);
    }
    bool sent_ok = (sent >= 0) && ((size_t)sent == SYN_WG_INITIATION_SIZE);
    if (sent_ok) {
        memcpy(wg->hs_ephemeral_priv, w->eph_priv, 32U);
        memcpy(wg->hs_chaining_key, w->ck, 32U);
        memcpy(wg->hs_hash, w->h, 32U);
        wg->hs_sender_index = init_sender_idx;
        wg->hs_valid = true;
        if (wg->state != SYN_WG_ESTABLISHED) {
            wg->session.sender_index = init_sender_idx;
        }
        wg->last_sent_ms = syn_port_get_tick_ms();
        wg->last_handshake_ms = wg->last_sent_ms;
    } else {
        syn_secure_zero(msg, SYN_WG_INITIATION_SIZE);
    }
    return sent_ok;
}

static SYN_PT_Status wg_send_initiation_pt(SYN_PT *pt, SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;

    PT_BEGIN(pt);

    if (!wg_init_prepare(wg)) {
        wg_hs_work_wipe(w);
        w->ok = false;
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    wg_init_noise_and_eph(wg);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &w->crypto_pt, syn_x25519_pt(&w->crypto_pt, &w->x25519, w->eph_pub));
    PT_CONTINUE(pt);

    wg_init_mix_eph_key(wg);
    PT_CONTINUE(pt);

    wg_init_start_dh_es(wg);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &w->crypto_pt, syn_x25519_pt(&w->crypto_pt, &w->x25519, w->tmp_key));
    PT_CONTINUE(pt);

    wg_init_mix_es(wg);
    PT_CONTINUE(pt);

    wg_init_enc_static(wg);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &w->crypto_pt, syn_x25519_pt(&w->crypto_pt, &w->x25519, w->tmp_key));
    PT_CONTINUE(pt);

    wg_init_mix_ss(wg);
    PT_CONTINUE(pt);

    wg_init_enc_ts(wg);
    PT_CONTINUE(pt);

    {
        bool rekey = w->is_rekey;
        bool sent_ok = wg_init_mac_and_send(wg);
        wg_hs_work_wipe(w);
        if (!sent_ok) {
            w->ok = false;
            PT_EXIT(pt);
        }
        if (!rekey) {
            wg->state = SYN_WG_HANDSHAKE_INIT;
        }
        w->ok = true;
    }

    PT_END(pt);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Handshake: Consume Response
 * ═══════════════════════════════════════════════════════════════════════════ */

static void wg_send_keepalive(SYN_WG *wg)
{
    /* A keepalive is just a transport message with zero-length payload */
    (void)syn_wg_send(wg, NULL, 0U);
}

static bool wg_verify_response_mac1(const SYN_WG *wg, const uint8_t *msg)
{
    uint8_t expected_mac[16];
    wg_mac1(expected_mac, wg->public_key, msg, 60U);
    bool ok = syn_ct_memeq(expected_mac, msg + 60U, 16U);
    syn_secure_zero(expected_mac, sizeof(expected_mac));
    return ok;
}

static bool wg_resp_start(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    if ((len != SYN_WG_RESPONSE_SIZE) || (load32_le(msg) != SYN_WG_MSG_RESPONSE) || !wg->hs_valid ||
        wg->hs_work.active) {
        return false;
    }
    if ((load32_le(msg + 8U) != wg->hs_sender_index) || !wg_verify_response_mac1(wg, msg)) {
        return false;
    }

    SYN_WgHsWork *w = &wg->hs_work;
    wg_hs_work_wipe(w);
    memcpy(w->h, wg->hs_hash, 32U);
    memcpy(w->ck, wg->hs_chaining_key, 32U);
    memcpy(w->eph_pub, msg + 12U, 32U);
    memcpy(w->enc_static, msg + 44U, 16U);
    w->expected_idx = wg->hs_sender_index;
    w->peer_idx = load32_le(msg + 4U);

    PT_INIT(&w->hs_pt);
    w->active = true;
    w->is_resp = true;
    return true;
}

static void wg_resp_mix_eph_key(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    wg_mix_key(w->ck, k, w->eph_pub, 32U);
    syn_secure_zero(k, sizeof(k));
}

static void wg_resp_start_dh_ee(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_mix_hash(w->h, w->eph_pub, 32U);
    (void)syn_x25519_init(&w->x25519, wg->hs_ephemeral_priv, w->eph_pub);
}

static void wg_resp_after_dh_ee(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    w->dh_ok = wg_dh_finish_mix_key(w->ck, k, w->tmp_key);
    syn_secure_zero(k, sizeof(k));
    (void)syn_x25519_init(&w->x25519, wg->config.private_key, w->eph_pub);
}

static void wg_resp_mix_se(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t k[32];
    w->dh_ok &= wg_dh_finish_mix_key(w->ck, k, w->tmp_key);
    syn_secure_zero(k, sizeof(k));
}

static void wg_resp_psk_step1(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_hkdf3_step1(w->eph_pub, w->ck, w->ck, wg->config.preshared_key, 32U);
}

static void wg_resp_psk_step2(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    uint8_t tau[32];
    wg_hkdf3_step2(tau, w->tmp_key, w->eph_pub, w->ck);
    wg_mix_hash(w->h, tau, 32U);
    syn_secure_zero(tau, sizeof(tau));
}

static bool wg_resp_verify_tag(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    bool ok =
        (w->dh_ok != 0U) && wg_decrypt_and_hash(w->h, w->tmp_key, NULL, 0U, w->enc_static, NULL);
    if (!ok) {
        wg_hs_work_wipe(w);
    }
    return ok;
}

static void wg_resp_derive_keys(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg_hkdf2(w->eph_priv, w->tmp_key, w->ck, (const uint8_t *)"", 0U);
}

static void wg_resp_commit_session(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    wg->session.sender_index = w->expected_idx;
    wg->session.receiver_index = w->peer_idx;
    memcpy(wg->session.send_key, w->eph_priv, 32U);
    memcpy(wg->session.recv_key, w->tmp_key, 32U);
    wg->session.send_counter = 0U;
    wg->session.recv_counter = 0U;
    wg->session.recv_bitmap = 0U;
    wg->session.established_ms = syn_port_get_tick_ms();
    wg->last_recv_ms = wg->session.established_ms;
    wg->rekey_in_progress = false;
    wg->state = SYN_WG_ESTABLISHED;
    SYN_METRIC_INC(wg_handshakes);
    wg_clear_handshake_state(wg);
    wg_send_keepalive(wg);
}

static SYN_PT_Status wg_consume_response_pt(SYN_PT *pt, SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;

    PT_BEGIN(pt);

    wg_resp_mix_eph_key(wg);
    PT_CONTINUE(pt);

    wg_resp_start_dh_ee(wg);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &w->crypto_pt, syn_x25519_pt(&w->crypto_pt, &w->x25519, w->tmp_key));
    PT_CONTINUE(pt);

    wg_resp_after_dh_ee(wg);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &w->crypto_pt, syn_x25519_pt(&w->crypto_pt, &w->x25519, w->tmp_key));
    PT_CONTINUE(pt);

    wg_resp_mix_se(wg);
    PT_CONTINUE(pt);

    wg_resp_psk_step1(wg);
    PT_CONTINUE(pt);

    wg_resp_psk_step2(wg);
    PT_CONTINUE(pt);

    if ((w->dh_ok == 0U) || !wg_resp_verify_tag(wg)) {
        w->ok = false;
        PT_EXIT(pt);
    }
    PT_CONTINUE(pt);

    wg_resp_derive_keys(wg);
    PT_CONTINUE(pt);

    wg_resp_commit_session(wg);
    w->ok = true;

    PT_END(pt);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Transport: Encrypt + Send
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_wg_send(SYN_WG *wg, const uint8_t *ip_packet, size_t len)
{
    if ((wg == NULL) || ((ip_packet == NULL) && (len > 0U)) || (len > SYN_WG_MTU)) {
        return SYN_ERROR;
    }
    uint32_t now = syn_port_get_tick_ms();
    if ((wg->state != SYN_WG_ESTABLISHED) ||
        (wg->session.send_counter >= SYN_WG_REJECT_AFTER_MESSAGES) ||
        ((now - wg->session.established_ms) > ((uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U))) {
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
    if ((sent < 0) || ((size_t)sent != total)) {
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

static bool wg_handle_transport(SYN_WG *wg, const uint8_t *msg, size_t len)
{
    uint32_t now = syn_port_get_tick_ms();
    if ((len < 32U) ||
        ((now - wg->session.established_ms) > ((uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U))) {
        return false; /* Minimum: 16 header + 16 tag (empty), within session lifetime */
    }

    /* Verify receiver index */
    uint32_t receiver = load32_le(msg + 4U);
    if (receiver != wg->session.sender_index) {
        return false;
    }

    /* Extract counter */
    uint64_t counter = load64_le(msg + 8U);

    /* Anti-replay pre-check (non-mutating) */
    if (!wg_replay_test(&wg->session, counter)) {
        return false;
    }

    /* Build nonce */
    uint8_t nonce[12];
    memset(nonce, 0, 4U);
    store64_le(nonce + 4U, counter);

    /* Decrypt: payload starts at offset 16, tag is last 16 bytes */
    size_t ct_len = len - 32U; /* Subtract header(16) + tag(16) */
    const uint8_t *ct = msg + 16U;
    const uint8_t *tag = msg + 16U + ct_len;

    uint8_t *plain = wg->rx_buf; /* Reuse rx_buf for decrypted output */
    if (ct_len > wg->rx_buf_size) {
        return false;
    }

    if (!syn_aead_decrypt(wg->session.recv_key, nonce, NULL, 0U, ct, ct_len, tag, plain)) {
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
    if ((pkt_len > 0U) && (wg->on_recv != NULL)) {
        wg->on_recv(plain, pkt_len, wg->user_ctx);
    }

    return true;
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

static bool wg_socket_readable(const SYN_WG *wg)
{
    return wg->udp_sock != SYN_SOCKET_INVALID && syn_port_udp_readable(wg->udp_sock);
}

static bool wg_handshake_timed_out(const SYN_WG *wg)
{
    return wg->state == SYN_WG_HANDSHAKE_INIT && (syn_port_get_tick_ms() - wg->last_handshake_ms) >
                                                     (uint32_t)SYN_WG_REKEY_TIMEOUT * 1000U;
}

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

static bool wg_has_work(const SYN_WG *wg)
{
    if (wg == NULL) {
        return false;
    }
    if (wg->hs_work.active || (wg->state == SYN_WG_DISCONNECTED) || wg_socket_readable(wg) ||
        wg_handshake_timed_out(wg)) {
        return true;
    }

    if (wg->state == SYN_WG_ESTABLISHED) {
        uint32_t now = syn_port_get_tick_ms();
        if (((now - wg->session.established_ms) > ((uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U)) ||
            wg_rekey_due(wg, now)) {
            return true;
        }
        if ((wg->config.keepalive_interval_s > 0U) &&
            ((now - wg->last_sent_ms) > ((uint32_t)wg->config.keepalive_interval_s * 1000U))) {
            return true;
        }
    }
    return false;
}

static bool wg_task_valid(const SYN_PT *pt, const SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return false;
    }
    const SYN_WG *wg = (const SYN_WG *)task->user_data;
    return wg->sntp != NULL;
}

static SYN_PT_Status wg_step_active_hs(SYN_WG *wg)
{
    SYN_WgHsWork *w = &wg->hs_work;
    SYN_PT_Status st;
    if (w->is_resp) {
        st = wg_consume_response_pt(&w->hs_pt, wg);
    } else {
        st = wg_send_initiation_pt(&w->hs_pt, wg);
    }
    if (st >= PT_EXITED) {
        w->active = false;
    }
    return st;
}

static bool wg_step_hs_or_init(SYN_WG *wg, bool *need_delay)
{
    *need_delay = false;
    if (!wg->hs_work.active) {
        if (wg->state != SYN_WG_DISCONNECTED) {
            return false;
        }
        wg_init_start(wg, false);
    }

    bool was_init_disc = (!wg->hs_work.is_resp) && (!wg->hs_work.is_rekey);
    SYN_PT_Status st = wg_step_active_hs(wg);
    if ((st >= PT_EXITED) && was_init_disc && !wg->hs_work.ok) {
        *need_delay = true;
    }
    return true;
}

static bool wg_step_established_timers(SYN_WG *wg)
{
    if (wg->state != SYN_WG_ESTABLISHED) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    if ((now - wg->session.established_ms) > ((uint32_t)SYN_WG_REJECT_AFTER_TIME * 1000U)) {
        syn_secure_zero(&wg->session, sizeof(wg->session));
        wg->rekey_in_progress = false;
        wg_clear_handshake_state(wg);
        wg->state = SYN_WG_DISCONNECTED;
        return true;
    }

    if (wg_rekey_due(wg, now)) {
        wg->last_handshake_ms = now;
        wg->rekey_in_progress = true;
        wg_init_start(wg, true);
        (void)wg_step_active_hs(wg);
        wg->rx_turn = 1U;
        return wg->hs_work.active;
    }

    if ((wg->config.keepalive_interval_s > 0U) &&
        ((now - wg->last_sent_ms) > ((uint32_t)wg->config.keepalive_interval_s * 1000U))) {
        wg_send_keepalive(wg);
    }
    return false;
}

static void wg_process_datagram(SYN_WG *wg, size_t len)
{
    uint32_t msg_type = (len >= 4U) ? load32_le(wg->rx_buf) : 0U;
    bool hs_waiting = (wg->state == SYN_WG_HANDSHAKE_INIT) ||
                      ((wg->state == SYN_WG_ESTABLISHED) && wg->rekey_in_progress);
    bool accepting_resp = (!wg->hs_work.active) && wg->hs_valid && hs_waiting;

    if ((msg_type == SYN_WG_MSG_RESPONSE) && accepting_resp) {
        (void)wg_resp_start(wg, wg->rx_buf, len);
    } else if ((msg_type == SYN_WG_MSG_TRANSPORT) && (wg->state == SYN_WG_ESTABLISHED)) {
        (void)wg_handle_transport(wg, wg->rx_buf, len);
    }
}

typedef enum {
    WG_ACT_WAIT = 0, 
    WG_ACT_MORE,     
    WG_ACT_PASS,     
    WG_ACT_DELAY     
} WG_StepAct;

static bool wg_rekey_rx_due(const SYN_WG *wg)
{
    return (wg->state == SYN_WG_ESTABLISHED) && wg->hs_work.active && (wg->rx_turn != 0U) &&
           wg_socket_readable(wg);
}

static WG_StepAct wg_step_recv(SYN_WG *wg)
{
    SYN_SockAddr from;
    int n = syn_port_udp_recvfrom(wg->udp_sock, wg->rx_buf, wg->rx_buf_size, &from, 0U);
    if (n > 0) {
        wg_process_datagram(wg, (size_t)n);
        if (wg->hs_work.active || wg_socket_readable(wg)) {
            return WG_ACT_MORE;
        }
        return WG_ACT_WAIT;
    }
    if (wg_socket_readable(wg)) {
        return WG_ACT_PASS;
    }
    return WG_ACT_WAIT;
}

static WG_StepAct wg_task_step_once(SYN_WG *wg, uint32_t *delay_ms)
{
    *delay_ms = 0U;
    if (wg->udp_sock == SYN_SOCKET_INVALID) {
        wg->udp_sock = syn_port_udp_open(0U);
        if (wg->udp_sock == SYN_SOCKET_INVALID) {
            *delay_ms = 5000U;
            return WG_ACT_DELAY;
        }
    }

    if (wg_rekey_rx_due(wg)) {
        wg->rx_turn = 0U;
        return wg_step_recv(wg);
    }

    bool need_delay = false;
    if (wg_step_hs_or_init(wg, &need_delay)) {
        wg->rx_turn = 1U;
        if (need_delay) {
            *delay_ms = 5000U;
            return WG_ACT_DELAY;
        }
        if (wg->hs_work.active || wg_socket_readable(wg)) {
            return WG_ACT_MORE;
        }
        return WG_ACT_WAIT;
    }

    if (wg_handshake_timed_out(wg)) {
        wg_clear_handshake_state(wg);
        wg->state = SYN_WG_DISCONNECTED;
        *delay_ms = 500U;
        return WG_ACT_DELAY;
    }

    if (wg_step_established_timers(wg)) {
        return WG_ACT_MORE;
    }

    return wg_step_recv(wg);
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
        uint32_t delay_ms = 0U;
        WG_StepAct act = wg_task_step_once(wg, &delay_ms);
        if (act == WG_ACT_DELAY) {
            PT_TASK_DELAY_MS(pt, task, delay_ms);
        } else if (act == WG_ACT_MORE) {
            PT_CONTINUE_IF_NEEDED(pt, task);
        } else if (act == WG_ACT_PASS) {
            PT_PASS(pt);
        } else {
            PT_WAIT_UNTIL(pt, wg_has_work(wg));
        }
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
    wg->rx_turn = 0U;
    wg->tx_bytes = 0U;
    wg->rx_bytes = 0U;
    syn_secure_zero(&wg->session, sizeof(wg->session));
    wg_clear_handshake_state(wg);
}

SYN_Status syn_wg_get_stats(const SYN_WG *wg, SYN_WgStats *stats)
{
    if ((wg == NULL) || (stats == NULL)) {
        return SYN_INVALID_PARAM;
    }

    memset(stats, 0, sizeof(*stats));
    stats->is_established = (wg->state == SYN_WG_ESTABLISHED);

    if (stats->is_established) {
        uint32_t now = syn_port_get_tick_ms();
        stats->handshake_age_sec = (now - wg->session.established_ms) / 1000U;
        stats->tx_bytes = wg->tx_bytes;
        stats->rx_bytes = wg->rx_bytes;
    }

    return SYN_OK;
}

#endif /* SYN_USE_WG */
```


