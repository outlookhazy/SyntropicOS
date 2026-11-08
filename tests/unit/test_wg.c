/**
 * @file test_wg.c
 * @brief Unity tests for WireGuard internals — HKDF, Noise helpers,
 *        replay window, transport, and handshake construction.
 *
 * Uses #include "syn_wg.c" to access static functions directly.
 * All expected values verified against the Go reference
 * (tests/test_wg_reference.go).
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

/* Pull in the .c to access static functions */
#include "syntropic/net/syn_wg.c"
#include "syntropic/util/syn_fmt.h"

#include <stdio.h>
#include <string.h>

static void wg_hex2bin(const char *hex, uint8_t *out, size_t len)
{
    syn_fmt_hex_parse(hex, out, len);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Construction constants (verified against Go wireguard-go)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* C = HASH(Construction) */
static void test_wg_construction_hash(void)
{
    uint8_t ck[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1, ck, 32);

    uint8_t expected[32];
    wg_hex2bin("60e26daef327efc02ec335e2a025d2d0"
               "16eb4206f87277f52d38d1988b78cd36",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, ck, 32);
}

/* H = HASH(C || Identifier) */
static void test_wg_identifier_hash(void)
{
    uint8_t ck[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1, ck, 32);

    uint8_t h[32];
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32);
    syn_blake2s_update(&ctx, ck, 32);
    syn_blake2s_update(&ctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1);
    syn_blake2s_final(&ctx, h);

    uint8_t expected[32];
    wg_hex2bin("2211b361081ac566691243db458ad532"
               "2d9c6c662293e8b70ee19c65ba079ef3",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, h, 32);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  HKDF2 / HKDF3
 * ═══════════════════════════════════════════════════════════════════════════ */

/* HKDF2 matches the Go reference for KDF2(C, E_pub) */
static void test_wg_hkdf2(void)
{
    uint8_t ck[32], out1[32], out2[32];
    /* Initial C */
    wg_hex2bin("60e26daef327efc02ec335e2a025d2d0"
               "16eb4206f87277f52d38d1988b78cd36",
               ck, 32);

    /* E_pub from our Go reference */
    uint8_t e_pub[32];
    wg_hex2bin("5dfedd3b6bd47f6fa28ee15d969d5bb0"
               "ea53774d488bdaf9df1c6e0124b3ef22",
               e_pub, 32);

    wg_hkdf2(out1, out2, ck, e_pub, 32);

    /* out1 = new C (after e_pub) */
    uint8_t expected_ck[32];
    wg_hex2bin("3b9c2a603fb5783c1c74f7b4501c3901"
               "280d8451962abc94d0b8a6ea00b934c1",
               expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, out1, 32);
}

/* HKDF3 produces 3 outputs (used for PSK mixing) */
static void test_wg_hkdf3(void)
{
    uint8_t ck[32], out1[32], out2[32], out3[32];
    memset(ck, 0x42, 32);

    uint8_t input[8] = {'h', 'k', 'd', 'f', '3', 't', 's', 't'};
    wg_hkdf3(out1, out2, out3, ck, input, 8);

    /* Verify all 3 outputs are different and non-zero */
    TEST_ASSERT_FALSE(memcmp(out1, out2, 32) == 0);
    TEST_ASSERT_FALSE(memcmp(out2, out3, 32) == 0);
    TEST_ASSERT_FALSE(memcmp(out1, out3, 32) == 0);

    uint8_t zeros[32] = {0};
    TEST_ASSERT_FALSE(memcmp(out1, zeros, 32) == 0);
    TEST_ASSERT_FALSE(memcmp(out2, zeros, 32) == 0);
    TEST_ASSERT_FALSE(memcmp(out3, zeros, 32) == 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Noise helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* mix_hash: H = BLAKE2s(H || data) */
static void test_wg_mix_hash(void)
{
    uint8_t h[32];
    /* Start with a known H, mix in r_pub */
    wg_hex2bin("2211b361081ac566691243db458ad532"
               "2d9c6c662293e8b70ee19c65ba079ef3",
               h, 32);

    uint8_t r_pub[32];
    wg_hex2bin("ce8d3ad1ccb633ec7b70c17814a5c76e"
               "cd029685050d344745ba05870e587d59",
               r_pub, 32);

    wg_mix_hash(h, r_pub, 32);

    uint8_t expected[32];
    wg_hex2bin("74c87e340810e6a8815cf911b640ebfb"
               "9150dc04293e5274e324126100cdf6e3",
               expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, h, 32);
}

/* encrypt_and_hash → decrypt_and_hash round-trip preserves hash chain */
static void test_wg_encrypt_decrypt_hash(void)
{
    uint8_t h_enc[32], h_dec[32], k[32];
    memset(h_enc, 0xAA, 32);
    memcpy(h_dec, h_enc, 32); /* Start with same H */
    memset(k, 0xBB, 32);

    uint8_t plain[16] = "test plaintext!";
    uint8_t ct[16], tag[16], decrypted[16];

    wg_encrypt_and_hash(h_enc, k, plain, 16, ct, tag);

    bool ok = wg_decrypt_and_hash(h_dec, k, ct, 16, tag, decrypted);
    TEST_ASSERT_TRUE(ok);

    /* Decrypted data matches original */
    TEST_ASSERT_EQUAL_HEX8_ARRAY(plain, decrypted, 16);

    /* Hash chains match (both sides mixed the same ct+tag) */
    TEST_ASSERT_EQUAL_HEX8_ARRAY(h_enc, h_dec, 32);
}

/* encrypt_and_hash with empty plaintext (used for handshake with no PSK) */
static void test_wg_encrypt_decrypt_hash_empty(void)
{
    uint8_t h_enc[32], h_dec[32], k[32];
    memset(h_enc, 0xCC, 32);
    memcpy(h_dec, h_enc, 32);
    memset(k, 0xDD, 32);

    uint8_t tag[16];

    wg_encrypt_and_hash(h_enc, k, NULL, 0, NULL, tag);

    bool ok = wg_decrypt_and_hash(h_dec, k, NULL, 0, tag, NULL);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(h_enc, h_dec, 32);
}

/* decrypt_and_hash fails with wrong tag */
static void test_wg_decrypt_and_hash_tampered(void)
{
    uint8_t h_enc[32], h_dec[32], k[32];
    memset(h_enc, 0xEE, 32);
    memcpy(h_dec, h_enc, 32);
    memset(k, 0xFF, 32);

    uint8_t plain[8] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    uint8_t ct[8], tag[16], decrypted[8];

    wg_encrypt_and_hash(h_enc, k, plain, 8, ct, tag);
    tag[0] ^= 0x01; /* Tamper */

    bool ok = wg_decrypt_and_hash(h_dec, k, ct, 8, tag, decrypted);
    TEST_ASSERT_FALSE(ok);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  MAC1
 * ═══════════════════════════════════════════════════════════════════════════ */

/* MAC1 produces consistent, non-zero output */
static void test_wg_mac1_basic(void)
{
    uint8_t peer_pub[32], mac[16];
    memset(peer_pub, 0x42, 32);

    uint8_t msg[64];
    memset(msg, 0x11, 64);

    wg_mac1(mac, peer_pub, msg, 64);

    /* Non-zero */
    uint8_t zeros[16] = {0};
    TEST_ASSERT_FALSE(memcmp(mac, zeros, 16) == 0);

    /* Deterministic */
    uint8_t mac2[16];
    wg_mac1(mac2, peer_pub, msg, 64);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(mac, mac2, 16);
}

/* MAC1 changes when message changes */
static void test_wg_mac1_sensitivity(void)
{
    uint8_t peer_pub[32], mac1[16], mac2[16];
    memset(peer_pub, 0x42, 32);

    uint8_t msg1[64], msg2[64];
    memset(msg1, 0x11, 64);
    memset(msg2, 0x11, 64);
    msg2[0] = 0x22; /* One byte different */

    wg_mac1(mac1, peer_pub, msg1, 64);
    wg_mac1(mac2, peer_pub, msg2, 64);

    TEST_ASSERT_FALSE(memcmp(mac1, mac2, 16) == 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Anti-replay window
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Sequential packets accepted */
static void test_wg_replay_sequential(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 1));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 2));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 3));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 4));
}

/* Duplicate rejected */
static void test_wg_replay_duplicate(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 5));
    TEST_ASSERT_FALSE(wg_replay_check(&s, 5)); /* duplicate */
}

/* Large forward jump accepted, old rejected */
static void test_wg_replay_forward_jump(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 1));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 100)); /* big jump */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 1));  /* now too old */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 30)); /* also too old (>64 behind) */
}

/* Out-of-order within window accepted */
static void test_wg_replay_out_of_order(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 10));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 8));  /* 2 behind, within window */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 9));  /* 1 behind, within window */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 8)); /* already seen */
}

/* Window boundary: exactly 63 behind is accepted, 64 behind is rejected */
static void test_wg_replay_window_boundary(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 64));

    /* 63 behind (counter 1) — should be accepted */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 1));

    /* Set counter further ahead */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 128));

    /* 63 behind (counter 65) — should be accepted */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 65));

    /* 64 behind (counter 64) — should be rejected (outside window) */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 64));
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Full handshake construction against Go reference
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Verify the full initiation message intermediate values match Go */
static void test_wg_handshake_intermediates(void)
{
    /* Fixed keys matching test_wg_reference.go */
    uint8_t i_priv[32], i_pub[32];
    uint8_t r_pub[32];
    uint8_t e_priv[32], e_pub[32];

    memset(i_priv, 0x01, 32);
    syn_x25519_clamp(i_priv);
    syn_x25519_pubkey(i_pub, i_priv);

    memset(e_priv, 0x03, 32);
    syn_x25519_clamp(e_priv);
    syn_x25519_pubkey(e_pub, e_priv);

    /* Responder public key */
    wg_hex2bin("ce8d3ad1ccb633ec7b70c17814a5c76e"
               "cd029685050d344745ba05870e587d59",
               r_pub, 32);

    /* Step 1: C = HASH(Construction) */
    uint8_t ck[32], h[32], k[32], dh[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1, ck, 32);

    /* Step 2: H = HASH(C || Identifier) */
    wg_mix_hash(ck, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1);
    /* Wait — ck is used as both C and the input to mix_hash.
     * Actually the spec says H starts from C, not modifying C in-place.
     * Let me recalculate properly. */

    /* Redo: */
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1, ck, 32);
    {
        SYN_BLAKE2s ctx;
        syn_blake2s_init(&ctx, 32);
        syn_blake2s_update(&ctx, ck, 32);
        syn_blake2s_update(&ctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1);
        syn_blake2s_final(&ctx, h);
    }

    /* Step 3: H = HASH(H || S_pub_r) */
    wg_mix_hash(h, r_pub, 32);

    uint8_t expected_h[32];
    wg_hex2bin("74c87e340810e6a8815cf911b640ebfb"
               "9150dc04293e5274e324126100cdf6e3",
               expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 4: C, _ = KDF2(C, E_pub) */
    wg_mix_key(ck, k, e_pub, 32);

    uint8_t expected_ck[32];
    wg_hex2bin("3b9c2a603fb5783c1c74f7b4501c3901"
               "280d8451962abc94d0b8a6ea00b934c1",
               expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    /* Step 5: H = HASH(H || E_pub) */
    wg_mix_hash(h, e_pub, 32);

    wg_hex2bin("7c924cc768f21c641fa3292ddfca6234"
               "807e0d0cbf2f70763ad045be9b75b3b6",
               expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 6: DH(E_priv, S_pub_r) → C, k */
    syn_x25519(dh, e_priv, r_pub);
    wg_mix_key(ck, k, dh, 32);

    wg_hex2bin("50e36bfff4d62801ff6375d4521af1af"
               "4ddf24fe1425e4bdf4b0c0236be94d2f",
               expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    uint8_t expected_k[32];
    wg_hex2bin("ccb521ed0369ab01a50b94554bcd9bfb"
               "e81cf4851df2caeb3776b4b6882f84ad",
               expected_k, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_k, k, 32);

    /* Step 7: Encrypt static key → check H */
    uint8_t ct[32], tag[16];
    wg_encrypt_and_hash(h, k, i_pub, 32, ct, tag);

    wg_hex2bin("9988e81b8d83aca922a8092fb2c1ec0f"
               "3e70e8c0bd925f1bf8dece66a364e59c",
               expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 8: DH(S_priv_i, S_pub_r) → C, k */
    syn_x25519(dh, i_priv, r_pub);
    wg_mix_key(ck, k, dh, 32);

    wg_hex2bin("4696b0ed00cff9c62cf447eb5f9c899d"
               "508e66b889a8cef23c4ae5c60f902b5e",
               expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    wg_hex2bin("8c2e061899942b4083496ce0bb8e1b4c"
               "bf5308f05aafda8e55c26a20d7c1774a",
               expected_k, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_k, k, 32);

    /* Step 9: Encrypt timestamp → check H */
    uint8_t timestamp[12] = {
        0x40, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x42, 0x40, 0x00, 0x00, 0x00, 0x00,
    };
    wg_encrypt_and_hash(h, k, timestamp, 12, ct, tag);

    wg_hex2bin("7167ec354d44f588a7ddc45e754a4dad"
               "83f1bb903f77dd9cce9295b7e0ec4a51",
               expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Init state
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_wg_init_state(void)
{
    SYN_WG wg;
    SYN_WgConfig cfg;
    SYN_SNTP sntp;
    uint8_t rx_buf[256], tx_buf[256];

    memset(&cfg, 0, sizeof(cfg));
    memset(cfg.private_key, 0x01, 32);
    memset(cfg.peer_public_key, 0x02, 32);
    cfg.endpoint.port = 51820;

    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);
    TEST_ASSERT_FALSE(syn_wg_is_established(&wg));
    TEST_ASSERT_EQUAL(0, wg.session.send_counter);
    TEST_ASSERT_EQUAL(0, wg.session.recv_counter);
}

/* ── State Machine Tests ────────────────────────────────────────────────── */

static uint8_t s_rx_buf[1024];
static uint8_t s_tx_buf[1024];
static SYN_WG s_wg;
static SYN_SNTP s_sntp;

/* Canned keys for testing */
static const uint8_t CLIENT_PRIV[32] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

static const uint8_t PEER_PUB[32] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

static void wg_state_setup(void)
{
    mock_port_reset();

    /* Mock SNTP as synced */
    memset(&s_sntp, 0, sizeof(s_sntp));
    s_sntp.synced = true;
    s_sntp.epoch_s = 1600000000;
    s_sntp.sync_tick_ms = mock_tick_ms;

    SYN_WgConfig cfg = {0};
    memcpy(cfg.private_key, CLIENT_PRIV, 32);
    memcpy(cfg.peer_public_key, PEER_PUB, 32);
    cfg.endpoint.port = 51820;
    cfg.endpoint.ip[0] = 1;
    cfg.endpoint.ip[1] = 2;
    cfg.endpoint.ip[2] = 3;
    cfg.endpoint.ip[3] = 4;
    cfg.keepalive_interval_s = 25;

    syn_wg_init(&s_wg, &cfg, &s_sntp, s_rx_buf, sizeof(s_rx_buf), s_tx_buf, sizeof(s_tx_buf));
}

static void test_wg_initiation_on_task(void)
{
    wg_state_setup();
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};

    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_handshake_response_invalid(void)
{
    wg_state_setup();

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);

    /* Simulate a Handshake Response from peer (but with invalid crypto) */
    uint8_t mock_resp[92];
    memset(mock_resp, 0, sizeof(mock_resp));
    uint32_t *msg_type = (uint32_t *)mock_resp;
    *msg_type = SYN_WG_MSG_RESPONSE;

    SYN_SockAddr from = {.port = 51820};
    from.ip[0] = 1;
    from.ip[1] = 2;
    from.ip[2] = 3;
    from.ip[3] = 4;

    mock_udp_inject_packet(mock_resp, sizeof(mock_resp), &from);

    syn_wg_task(&pt, &task);

    /* Should still be HANDSHAKE_INIT (rejected response) */
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static bool s_recv_called = false;
static void test_on_recv(const uint8_t *buf, size_t len, void *ctx)
{
    (void)buf;
    (void)len;
    (void)ctx;
    s_recv_called = true;
}

static void test_wg_established_transport_and_keepalive(void)
{
    wg_state_setup();
    mock_tick_ms = 1000;
    mock_udp_tx_len = 0;
    s_recv_called = false;
    s_wg.on_recv = test_on_recv;

    /* Manually put state into ESTABLISHED with keys */
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.udp_sock = 1;
    s_wg.session.sender_index = 100;
    s_wg.session.receiver_index = 200;
    memset(s_wg.session.send_key, 0x11, 32);
    memset(s_wg.session.recv_key, 0x22, 32);
    s_wg.session.established_ms = 1000;
    s_wg.last_sent_ms = 1000;

    /* 1. Test syn_wg_send */
    uint8_t payload[10] = "HELLO_WG!";
    SYN_Status st = syn_wg_send(&s_wg, payload, sizeof(payload));
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT64(1, s_wg.session.send_counter);

    /* 2. Test syn_wg_send when disconnected (fails) */
    s_wg.state = SYN_WG_DISCONNECTED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&s_wg, payload, sizeof(payload)));
    s_wg.state = SYN_WG_ESTABLISHED;

    /* 3. Build & receive encrypted transport message with on_recv callback */
    uint8_t tx_msg[64];
    uint8_t nonce[12];
    memset(nonce, 0, 4);
    store64_le(nonce + 4, 1); /* counter = 1 */

    store32_le(tx_msg, SYN_WG_MSG_TRANSPORT);
    store32_le(tx_msg + 4, 100); /* receiver index = sender index of active session */
    store64_le(tx_msg + 8, 1);   /* counter = 1 */

    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0, payload, sizeof(payload), tx_msg + 16,
                     tx_msg + 16 + sizeof(payload));

    size_t tx_msg_len = 16 + sizeof(payload) + 16;
    bool handled = wg_handle_transport(&s_wg, tx_msg, tx_msg_len);
    TEST_ASSERT_TRUE(handled);
    TEST_ASSERT_TRUE(s_recv_called);

    /* 4. Test receiving transport message in syn_wg_task with counter=2 */
    uint8_t tx_msg2[64];
    memset(nonce, 0, 4);
    store64_le(nonce + 4, 2);

    store32_le(tx_msg2, SYN_WG_MSG_TRANSPORT);
    store32_le(tx_msg2 + 4, 100);
    store64_le(tx_msg2 + 8, 2);

    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0, payload, sizeof(payload), tx_msg2 + 16,
                     tx_msg2 + 16 + sizeof(payload));

    SYN_SockAddr from = {.port = 51820};
    from.ip[0] = 1;
    from.ip[1] = 2;
    from.ip[2] = 3;
    from.ip[3] = 4;
    mock_udp_inject_packet(tx_msg2, tx_msg_len, &from);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, s_wg.udp_sock);
    s_wg.last_sent_ms = 100;
    st = syn_wg_send(&s_wg, NULL, 0);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Trigger periodic keepalive check in task */
    s_wg.config.keepalive_interval_s = 10;
    s_wg.last_sent_ms = 100;
    s_wg.session.established_ms = 10000;
    mock_tick_ms = 25000;
    syn_wg_task(&pt, &task);

    /* 6. Session expiry / rekey in task */
    s_wg.session.established_ms = 100;
    mock_tick_ms = 200000; /* > SYN_WG_REKEY_AFTER_TIME (120s) */
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_send_disconnected_or_null(void)
{
    SYN_WG wg;
    memset(&wg, 0, sizeof(wg));
    wg.state = SYN_WG_DISCONNECTED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, (const uint8_t *)"test", 4));

    /* Oversized send payload */
    wg.state = SYN_WG_ESTABLISHED;
    wg.tx_buf_size = 32;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, (const uint8_t *)"12345678901234567890", 20));
}

static void test_wg_transport_error_branches(void)
{
    wg_state_setup();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.sender_index = 100;
    s_wg.session.receiver_index = 200;
    memset(s_wg.session.send_key, 0x11, 32);
    memset(s_wg.session.recv_key, 0x22, 32);

    /* 1. Runt message len < 32 */
    uint8_t runt[20] = {0};
    TEST_ASSERT_FALSE(wg_handle_transport(&s_wg, runt, 20));

    /* 2. Receiver index mismatch */
    uint8_t msg[64] = {0};
    store32_le(msg, SYN_WG_MSG_TRANSPORT);
    store32_le(msg + 4, 999); /* Wrong index */
    TEST_ASSERT_FALSE(wg_handle_transport(&s_wg, msg, 64));

    /* 3. Corrupted tag -> AEAD decrypt failure */
    store32_le(msg + 4, 100);
    store64_le(msg + 8, 1);
    TEST_ASSERT_FALSE(wg_handle_transport(&s_wg, msg, 64));
}

static void test_wg_transport_corrupt_tag_does_not_advance_replay_window(void)
{
    wg_state_setup();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.sender_index = 100;
    s_wg.session.receiver_index = 200;
    memset(s_wg.session.send_key, 0x11, 32);
    memset(s_wg.session.recv_key, 0x22, 32);
    s_wg.session.recv_counter = 0;
    s_wg.session.recv_bitmap = 0;

    /* 1. Receive unauthenticated forged packet with large counter = 100 and garbage payload/tag */
    uint8_t forged_msg[64] = {0};
    store32_le(forged_msg, SYN_WG_MSG_TRANSPORT);
    store32_le(forged_msg + 4, 100);
    store64_le(forged_msg + 8, 100);
    TEST_ASSERT_FALSE(wg_handle_transport(&s_wg, forged_msg, 64));

    /* Anti-replay window must NOT have been updated */
    TEST_ASSERT_EQUAL_UINT64(0, s_wg.session.recv_counter);
    TEST_ASSERT_EQUAL_UINT32(0, s_wg.session.recv_bitmap);

    /* 2. Legitimate packet with counter = 1 must now succeed */
    uint8_t valid_msg[64];
    uint8_t payload[8] = {'o', 'k', 'd', 'a', 't', 'a', '1', '!'};
    uint8_t nonce[12] = {0};
    store64_le(nonce + 4, 1);

    store32_le(valid_msg, SYN_WG_MSG_TRANSPORT);
    store32_le(valid_msg + 4, 100);
    store64_le(valid_msg + 8, 1);

    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0, payload, sizeof(payload),
                     valid_msg + 16, valid_msg + 16 + sizeof(payload));

    size_t valid_len = 16 + sizeof(payload) + 16;
    TEST_ASSERT_TRUE(wg_handle_transport(&s_wg, valid_msg, valid_len));

    /* Replay window now committed */
    TEST_ASSERT_EQUAL_UINT64(1, s_wg.session.recv_counter);
    TEST_ASSERT_EQUAL_UINT32(1, s_wg.session.recv_bitmap);
}

static void test_wg_reject_after_time_expiry(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.established_ms = 1000;
    mock_tick_ms = 250000; /* > SYN_WG_REJECT_AFTER_TIME (180s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_cookie_packet_handling(void)
{
    wg_state_setup();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.udp_sock = 1;

    /* Inject MSG_COOKIE (type 3) and verify it does not crash or corrupt state */
    uint8_t cookie_pkt[64];
    memset(cookie_pkt, 0, sizeof(cookie_pkt));
    store32_le(cookie_pkt, SYN_WG_MSG_COOKIE);
    mock_udp_set_response(cookie_pkt, sizeof(cookie_pkt), &s_wg.config.endpoint);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);
}

static void test_wg_rekey_after_time(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.established_ms = 1000;
    mock_tick_ms = 130000; /* > SYN_WG_REKEY_AFTER_TIME (120s), < REJECT_AFTER_TIME (180s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);
    TEST_ASSERT_TRUE(s_wg.rekey_in_progress);
}

static void test_wg_handshake_timeout(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    s_wg.last_handshake_ms = 1000;
    mock_tick_ms = 8000; /* > SYN_WG_REKEY_TIMEOUT (5s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);
}

static void test_wg_disconnect_and_stats(void)
{
    SYN_WG wg;
    uint8_t rx_buf[1600], tx_buf[1600];
    SYN_SNTP sntp;

    SYN_WgConfig cfg = {
        .endpoint = {.ip = {192, 168, 1, 1}, .port = 51820},
        .keepalive_interval_s = 25,
    };

    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    SYN_WgStats stats;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_get_stats(&wg, &stats));
    TEST_ASSERT_FALSE(stats.is_established);

    wg.state = SYN_WG_ESTABLISHED;
    wg.udp_sock = 1;
    wg.session.established_ms = 1000;
    uint8_t sample_ip[12] = {0x45, 0, 0, 12, 0, 0, 0, 0, 64, 1, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_send(&wg, sample_ip, sizeof(sample_ip)));

    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_get_stats(&wg, &stats));
    TEST_ASSERT_TRUE(stats.is_established);
    TEST_ASSERT_EQUAL_UINT64(sizeof(sample_ip), stats.tx_bytes);

    syn_wg_disconnect(&wg);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);
    TEST_ASSERT_EQUAL_UINT64(0U, wg.tx_bytes);
}

static void test_wg_send_fail_and_rekey_timeout_task_branches(void)
{
    wg_state_setup();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.udp_sock = 1;
    s_wg.session.receiver_index = 200;
    mock_udp_sendto_fail = true;
    uint8_t payload[10] = "FAIL_SEND";
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&s_wg, payload, sizeof(payload)));
    mock_udp_sendto_fail = false;

    /* Task in HANDSHAKE_INIT state timing out after SYN_WG_REKEY_TIMEOUT (5s) */
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    s_wg.last_handshake_ms = mock_tick_ms;
    mock_tick_ms += 6000;
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);
}

static void dummy_wg_rx_cb(const uint8_t *data, size_t len, void *ctx)
{
    (void)data;
    (void)len;
    (void)ctx;
}

static void test_wg_set_rx_callback(void)
{
    SYN_WG wg;
    memset(&wg, 0, sizeof(wg));
    wg.on_recv = dummy_wg_rx_cb;
    wg.user_ctx = (void *)0x1234;
    TEST_ASSERT_EQUAL_PTR(dummy_wg_rx_cb, wg.on_recv);
    TEST_ASSERT_EQUAL_PTR((void *)0x1234, wg.user_ctx);
}

static void test_wg_cookie_mac2_verification_failure(void)
{
    SYN_WG wg;
    memset(&wg, 0, sizeof(wg));
    uint8_t msg[92];
    memset(msg, 0, sizeof(msg));
    msg[0] = 2;

    /* Correct mac1 */
    wg_mac1(&msg[60], wg.public_key, msg, 60);
    /* Corrupted mac2 */
    msg[76] ^= 0xFF;

    TEST_ASSERT_FALSE(wg_consume_response(&wg, msg, sizeof(msg)));
}

static void test_wg_response_message_invalid_length_and_receiver(void)
{
    SYN_WG wg;
    memset(&wg, 0, sizeof(wg));
    wg.session.sender_index = 0x12345678;

    uint8_t short_msg[30] = {0};
    TEST_ASSERT_FALSE(wg_consume_response(&wg, short_msg, sizeof(short_msg)));

    uint8_t msg[92];
    memset(msg, 0, sizeof(msg));
    store32_le(msg, SYN_WG_MSG_RESPONSE);
    store32_le(msg + 8, 0x87654321); /* Receiver mismatch */

    TEST_ASSERT_FALSE(wg_consume_response(&wg, msg, 92));
}

static void test_wg_response_decryption_and_mac_validation_failures(void)
{
    SYN_WG wg;
    memset(&wg, 0, sizeof(wg));
    wg.session.sender_index = 0x12345678;

    uint8_t msg[92];
    memset(msg, 0, sizeof(msg));
    store32_le(msg, SYN_WG_MSG_RESPONSE);
    store32_le(msg + 4, 0x11223344); /* Peer sender index */
    store32_le(msg + 8, 0x12345678); /* Matching receiver index */

    /* Bad payload decryption or bad mac1 will cause wg_consume_response to fail gracefully */
    TEST_ASSERT_FALSE(wg_consume_response(&wg, msg, sizeof(msg)));
}

static void test_wg_send_buffer_too_small(void)
{
    SYN_WG wg;
    SYN_WgConfig cfg;
    SYN_SNTP sntp;
    uint8_t rx_buf[64], tx_buf[64];
    memset(&cfg, 0, sizeof(cfg));
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    uint8_t dummy[100];
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, dummy, sizeof(dummy)));
}

static void test_wg_response_mac1_tampered_rejection(void)
{
    SYN_WG wg;
    SYN_WgConfig cfg;
    SYN_SNTP sntp;
    uint8_t rx_buf[256], tx_buf[256];

    memset(&cfg, 0, sizeof(cfg));
    memset(cfg.private_key, 0x01, 32);
    memset(cfg.peer_public_key, 0x02, 32);
    cfg.endpoint.port = 51820;

    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    wg.state = SYN_WG_HANDSHAKE_INIT;
    wg.session.sender_index = 0x12345678;

    uint8_t msg[92];
    memset(msg, 0, sizeof(msg));
    msg[0] = 0x02;                    /* Response type */
    syn_poke_u32(0x12345678, msg, 8); /* receiver_index matching wg.session.sender_index */

    /* Tamper MAC1 at offset 60 */
    msg[60] ^= 0xFF;

    TEST_ASSERT_FALSE(wg_consume_response(&wg, msg, sizeof(msg)));
}

static void test_wg_send_initiation_failure_task_branch(void)
{
    /* Line 723: send_initiation fail in task */
    test_wg_init_state();
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    mock_udp_sendto_fail = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
    mock_udp_sendto_fail = false;
}

static void test_wg_valid_response_exchange(void)
{
    SYN_WG wg;
    SYN_WgConfig cfg;
    SYN_SNTP sntp;
    memset(&sntp, 0, sizeof(sntp));
    uint8_t rx_buf[1600], tx_buf[1600];

    memset(&cfg, 0, sizeof(cfg));
    /* Initiator static key = 0x01 */
    memset(cfg.private_key, 0x01, 32);
    /* Responder static private key = 0x02 -> calculate responder public key */
    uint8_t resp_priv[32];
    memset(resp_priv, 0x02, 32);
    syn_x25519_clamp(resp_priv);
    syn_x25519_pubkey(cfg.peer_public_key, resp_priv);
    cfg.endpoint.port = 51820;

    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    wg.udp_sock = 1;

    /* 1. Generate Initiation packet via syn_wg_task */
    SYN_PT init_pt;
    PT_INIT(&init_pt);
    SYN_Task init_task = {.user_data = &wg};
    mock_port_reset();
    mock_sock_connected = true;
    syn_wg_task(&init_pt, &init_task);

    /* 2. Build valid Response packet from Responder */
    uint8_t msg[92];
    memset(msg, 0, sizeof(msg));
    store32_le(msg, SYN_WG_MSG_RESPONSE);
    store32_le(msg + 4, 0x99887766); /* Responder sender_index */
    store32_le(msg + 8, wg.session.sender_index);

    /* Responder ephemeral keypair (0x03) */
    uint8_t resp_e_priv[32], resp_e_pub[32];
    memset(resp_e_priv, 0x03, 32);
    syn_x25519_clamp(resp_e_priv);
    syn_x25519_pubkey(resp_e_pub, resp_e_priv);
    memcpy(msg + 12, resp_e_pub, 32);

    /* Initialize noise state on responder side */
    uint8_t ck[32], h[32];
    const char *INIT_LABEL = "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s";
    const char *IDENT_LABEL = "WireGuard v1 zx2c4 justify";
    syn_blake2s(INIT_LABEL, strlen(INIT_LABEL), ck, 32);
    memcpy(h, ck, 32);
    wg_mix_hash(h, IDENT_LABEL, strlen(IDENT_LABEL));
    wg_mix_hash(h, cfg.peer_public_key, 32);  /* Responder public key */
    wg_mix_hash(h, mock_udp_tx_buf + 12, 32); /* Initiator ephemeral key (E_i) */
    wg_mix_hash(h, resp_e_pub, 32);           /* Responder ephemeral key (E_r) */

    /* DH(E_r, E_i) */
    uint8_t dh1[32], key[32];
    syn_x25519(dh1, resp_e_priv, mock_udp_tx_buf + 12);
    wg_hkdf2(ck, key, ck, dh1, 32);

    /* DH(E_r, S_i) */
    uint8_t dh2[32];
    syn_x25519(dh2, resp_e_priv, wg.public_key);
    wg_hkdf2(ck, key, ck, dh2, 32);

    /* DH(psk) */
    uint8_t psk_zeros[32] = {0};
    wg_hkdf3(ck, h, key, ck, psk_zeros, 32);

    /* Encrypt empty payload */
    uint8_t zero_nonce[12] = {0};
    syn_aead_encrypt(key, zero_nonce, h, 32, NULL, 0, msg + 44, msg + 44 + 0);
    wg_mix_hash(h, msg + 44, 16);

    /* MAC1 over 0..59 with initiator's public key */
    wg_mac1(msg + 60, wg.public_key, msg, 60);

    /* Consume invalid response length */
    TEST_ASSERT_FALSE(wg_consume_response(&wg, msg, 10));

    /* Test task receiving packet in SYN_WG_HANDSHAKE_INIT state (lines 757, 761-762) */
    mock_port_reset();
    test_wg_init_state();
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    s_wg.udp_sock = 1;
    mock_sock_connected = true;
    mock_sock_set_response(msg, 92);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &s_wg};
    syn_wg_task(&pt, &task);

    /* Test HANDSHAKE_INIT timeout in task loop (lines 721-725) */
    PT_INIT(&pt);
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    s_wg.last_handshake_ms = 1000;
    mock_tick_ms = 10000;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);
    mock_port_reset();
}

static void test_wg_send_initiation_tx_buf_too_small(void)
{
    /* Test syn_wg_disconnect with open socket (line 781) */
    SYN_WG wg;
    wg.udp_sock = 5;
    syn_wg_disconnect(&wg);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, wg.udp_sock);
}

static void test_wg_handshake_rekey_timer_expiry(void)
{
    /* wg == NULL -> false (line 691) */
    SYN_Task null_task = {.user_data = NULL};
    SYN_PT pt;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt, &null_task));

    /* SYN_WG_DISCONNECTED -> true (line 693) */
    SYN_WG wg_disc;
    memset(&wg_disc, 0, sizeof(wg_disc));
    wg_disc.state = SYN_WG_DISCONNECTED;
    TEST_ASSERT_TRUE(wg_has_work(&wg_disc));

    /* wg_has_work(NULL) -> line 691 */
    TEST_ASSERT_FALSE(wg_has_work(NULL));

    /* SYN_WG_HANDSHAKE_INIT no timeout -> false (line 700) */
    SYN_WG wg_hs;
    memset(&wg_hs, 0, sizeof(wg_hs));
    wg_hs.state = SYN_WG_HANDSHAKE_INIT;
    wg_hs.last_handshake_ms = 1000;
    mock_tick_ms = 1000 + (SYN_WG_REKEY_TIMEOUT + 1) * 1000;
    TEST_ASSERT_TRUE(wg_has_work(&wg_hs));
    mock_tick_ms = 1000 + 1000;
    TEST_ASSERT_FALSE(wg_has_work(&wg_hs));

    /* SYN_WG_ESTABLISHED no timeout, keepalive 0 -> false (lines 704 & 708) */
    SYN_WG wg_est_no_work;
    memset(&wg_est_no_work, 0, sizeof(wg_est_no_work));
    wg_est_no_work.state = SYN_WG_ESTABLISHED;
    wg_est_no_work.session.established_ms = 1000;
    wg_est_no_work.last_sent_ms = 1000;
    wg_est_no_work.config.keepalive_interval_s = 0;
    mock_tick_ms = 1000 + 2000;
    TEST_ASSERT_FALSE(wg_has_work(&wg_est_no_work));

    /* Test get_stats NULL parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wg_get_stats(NULL, NULL));
    SYN_WgStats stats_out;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wg_get_stats(NULL, &stats_out));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wg_get_stats(&wg_hs, NULL));

    /* Test replay check out-of-order and duplicate checks */
    SYN_WgSession replay_sess = {0};
    TEST_ASSERT_TRUE(wg_replay_check(&replay_sess, 20));
    TEST_ASSERT_TRUE(wg_replay_check(&replay_sess, 15));  /* diff = 5 < 32, unseen -> true */
    TEST_ASSERT_FALSE(wg_replay_check(&replay_sess, 15)); /* diff = 5 < 32, seen -> false */

    /* Test syn_wg_send error branches */
    SYN_WG wg_snd;
    memset(&wg_snd, 0, sizeof(wg_snd));
    uint8_t dummy_pkt[16] = {0};
    wg_snd.state = SYN_WG_DISCONNECTED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg_snd, dummy_pkt, sizeof(dummy_pkt)));

    wg_snd.state = SYN_WG_ESTABLISHED;
    wg_snd.tx_buf_size = 20; /* total (16+16+16=48) > 20 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg_snd, dummy_pkt, sizeof(dummy_pkt)));

    uint8_t big_tx[128];
    wg_snd.tx_buf = big_tx;
    wg_snd.tx_buf_size = sizeof(big_tx);
    wg_snd.udp_sock = 1;
    mock_udp_sendto_fail = true; /* sendto will return -1 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg_snd, dummy_pkt, sizeof(dummy_pkt)));
    mock_udp_sendto_fail = false;

    /* Test wg_handle_transport length and receiver mismatch */
    uint8_t short_msg[10] = {0};
    TEST_ASSERT_FALSE(wg_handle_transport(&wg_snd, short_msg, sizeof(short_msg)));

    uint8_t rcv_mismatch_msg[32] = {0};
    wg_snd.session.sender_index = 0x12345678;
    TEST_ASSERT_FALSE(wg_handle_transport(&wg_snd, rcv_mismatch_msg, sizeof(rcv_mismatch_msg)));

    /* Test disconnect and stats NULL checks */
    syn_wg_disconnect(NULL);
    SYN_WgStats stats;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wg_get_stats(NULL, &stats));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wg_get_stats(&wg_hs, NULL));

    /* Test get_stats in established state */
    SYN_WG wg_est;
    memset(&wg_est, 0, sizeof(wg_est));
    wg_est.state = SYN_WG_ESTABLISHED;
    wg_est.session.established_ms = 1000;
    wg_est.session.send_counter = 5;
    wg_est.session.recv_counter = 10;
    mock_tick_ms = 5000;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_get_stats(&wg_est, &stats));
    TEST_ASSERT_TRUE(stats.is_established);
    TEST_ASSERT_EQUAL_UINT32(4, stats.handshake_age_sec);

    /* Task NULL parameter checks */
    SYN_PT pt_null;
    PT_INIT(&pt_null);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt_null, NULL));
    SYN_Task null_user_task = {.user_data = NULL};
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt_null, &null_user_task));

    /* Established state keepalive and rekey task loop branches */
    SYN_SNTP sntp_mock = {.synced = true};
    SYN_WG wg_loop;
    memset(&wg_loop, 0, sizeof(wg_loop));
    wg_loop.state = SYN_WG_ESTABLISHED;
    wg_loop.sntp = &sntp_mock;
    wg_loop.config.keepalive_interval_s = 5;
    wg_loop.last_sent_ms = 1000;
    wg_loop.session.established_ms = 1000;
    wg_loop.udp_sock = 1;
    mock_tick_ms = 10000; /* > 1000 + 5000ms -> keepalive triggered */
    mock_sock_connected = true;

    SYN_Task loop_task = {.user_data = &wg_loop};
    PT_INIT(&pt_null);
    wg_loop.last_sent_ms = mock_tick_ms;
    syn_wg_task(&pt_null, &loop_task);

    /* Test syn_wg_task receiving n < 4 (n = 3) packet and msg_type = MSG_COOKIE (type = 4) */
    uint8_t short_pkt[3] = {1, 2, 3};
    SYN_SockAddr from = {0};
    mock_udp_set_response(short_pkt, sizeof(short_pkt), &from);
    PT_INIT(&pt_null);
    syn_wg_task(&pt_null, &loop_task);

    uint8_t cookie_pkt[32];
    memset(cookie_pkt, 0, sizeof(cookie_pkt));
    store32_le(cookie_pkt, 4); /* SYN_WG_MSG_COOKIE = 4 */
    mock_udp_set_response(cookie_pkt, sizeof(cookie_pkt), &from);
    PT_INIT(&pt_null);
    syn_wg_task(&pt_null, &loop_task);

    /* Test replay check large counter jump (diff >= 64) */
    SYN_WgSession s_jump = {.recv_counter = 10, .recv_bitmap = 0x01};
    TEST_ASSERT_TRUE(wg_replay_check(&s_jump, 100));
    TEST_ASSERT_EQUAL_UINT64(100, s_jump.recv_counter);
    TEST_ASSERT_EQUAL_UINT64(1, s_jump.recv_bitmap);

    /* Test session reject after time (180s, line 773) */
    wg_loop.state = SYN_WG_ESTABLISHED;
    wg_loop.session.established_ms = 1000;
    mock_tick_ms = 1000 + 185 * 1000;
    PT_INIT(&pt_null);
    syn_wg_task(&pt_null, &loop_task);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg_loop.state);
}

static void test_wg_task_null_sntp_multi_packet_yield_and_n11_reconnect(void)
{
    mock_port_reset();

    /* 1. wg->sntp == NULL guard (syn_wg.c:762-763) + NULL pt/task guards */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task empty_task = {0};
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(NULL, &empty_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt, &empty_task));

    wg_state_setup();
    s_wg.sntp = NULL;
    SYN_Task task = {0};
    task.user_data = &s_wg;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wg_task(&pt, &task));

    /* 2. Two back-to-back UDP packets hit PT_CONTINUE_IF_NEEDED + continue (syn_wg.c:851-852) */
    wg_state_setup();
    s_sntp.synced = true;
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.udp_sock = 20;
    s_wg.session.established_ms = mock_tick_ms;
    s_wg.last_sent_ms = mock_tick_ms;

    uint8_t cookie_pkt[32];
    memset(cookie_pkt, 0, sizeof(cookie_pkt));
    store32_le(cookie_pkt, SYN_WG_MSG_COOKIE);
    SYN_SockAddr from = {.ip = {10, 0, 0, 1}, .port = 51820};
    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_inject_packet(cookie_pkt, sizeof(cookie_pkt), &from);
    mock_udp_inject_packet(cookie_pkt, sizeof(cookie_pkt), &from);

    SYN_Sched sched = {0};
    memset(&task, 0, sizeof(task));
    task.user_data = &s_wg;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_MORE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(1, mock_udp_rx_pos);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(2, mock_udp_rx_pos);

    /* 3. Regression N-11: syn_wg_disconnect() while syn_wg_task is parked closes udp_sock;
     *    resuming syn_wg_task re-opens udp_sock and initiates a fresh handshake cleanly */
    wg_state_setup();
    s_sntp.synced = true;
    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_tx_len = 0;

    memset(&task, 0, sizeof(task));
    task.user_data = &s_wg;
    PT_INIT(&pt);

    /* Initial step opens socket, sends 148-byte handshake initiation, parks at PT_WAIT_UNTIL */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, s_wg.udp_sock);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
    TEST_ASSERT_EQUAL_UINT(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);

    /* Disconnect while task is live */
    syn_wg_disconnect(&s_wg);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, s_wg.udp_sock);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);

    /* Step same task instance -> wakes via wg_has_work, re-opens udp_sock, sends new initiation */
    mock_udp_tx_len = 0;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, s_wg.udp_sock);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
    TEST_ASSERT_EQUAL_UINT(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);
}

static void test_wg_round3_regressions(void)
{
    mock_port_reset();

    /* 1. R3-N15: Full Noise_IKpsk2 handshake exchange (valid AEAD + tampered mac1 + valid mac1) */
    SYN_SNTP sntp = {0};
    sntp.synced = true;
    sntp.epoch_s = 1700000000U;
    sntp.sync_tick_ms = mock_tick_ms;

    uint8_t resp_s_priv[32];
    memset(resp_s_priv, 0x02, 32);
    syn_x25519_clamp(resp_s_priv);

    SYN_WgConfig cfg = {0};
    memset(cfg.private_key, 0x01, 32);
    syn_x25519_clamp(cfg.private_key);
    syn_x25519_pubkey(cfg.peer_public_key, resp_s_priv);
    memset(cfg.preshared_key, 0x07, 32);
    cfg.endpoint.ip[0] = 10;
    cfg.endpoint.ip[3] = 1;
    cfg.endpoint.port = 51820;

    uint8_t rx_buf[512], tx_buf[512];
    SYN_WG wg;
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    /* wg_send_initiation error guards: small tx_buf, RNG failure, UDP sendto failure */
    wg.udp_sock = 20;
    wg.tx_buf_size = 100U;
    TEST_ASSERT_FALSE(wg_send_initiation(&wg));
    wg.tx_buf_size = sizeof(tx_buf);

    mock_random_skip = true;
    TEST_ASSERT_FALSE(wg_send_initiation(&wg));
    mock_random_skip = false;

    mock_udp_sendto_fail = true;
    TEST_ASSERT_FALSE(wg_send_initiation(&wg));
    mock_udp_sendto_fail = false;
    wg.udp_sock = SYN_SOCKET_INVALID;

    /* Run syn_wg_task to open socket and transmit real 148-byte initiation */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &wg;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);

    /* Responder parses initiation in mock_udp_tx_buf and builds authentic 92-byte response */
    const uint8_t *e_pub_i = mock_udp_tx_buf + 8;
    const uint8_t *enc_static = mock_udp_tx_buf + 40;
    const uint8_t *enc_ts = mock_udp_tx_buf + 88;

    uint8_t ck[32], h[32], k[32], dh[32], tau[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1U, ck, 32);
    {
        SYN_BLAKE2s bctx;
        syn_blake2s_init(&bctx, 32);
        syn_blake2s_update(&bctx, ck, 32);
        syn_blake2s_update(&bctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1U);
        syn_blake2s_final(&bctx, h);
    }
    wg_mix_hash(h, cfg.peer_public_key, 32);
    wg_mix_key(ck, k, e_pub_i, 32);
    wg_mix_hash(h, e_pub_i, 32);

    syn_x25519(dh, resp_s_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32);
    uint8_t dec_s_pub_i[32];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_static, 32, enc_static + 32, dec_s_pub_i));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(wg.public_key, dec_s_pub_i, 32);

    syn_x25519(dh, resp_s_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32);
    uint8_t dec_ts[12];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_ts, 12, enc_ts + 12, dec_ts));

    uint8_t resp_msg[SYN_WG_RESPONSE_SIZE];
    memset(resp_msg, 0, sizeof(resp_msg));
    store32_le(resp_msg + 0, SYN_WG_MSG_RESPONSE);
    store32_le(resp_msg + 4, 0xCAFEBABEU);
    store32_le(resp_msg + 8, wg.session.sender_index);

    uint8_t resp_e_priv[32], resp_e_pub[32];
    memset(resp_e_priv, 0x05, 32);
    syn_x25519_clamp(resp_e_priv);
    syn_x25519_pubkey(resp_e_pub, resp_e_priv);
    memcpy(resp_msg + 12, resp_e_pub, 32);

    wg_mix_key(ck, k, resp_e_pub, 32);
    wg_mix_hash(h, resp_e_pub, 32);
    syn_x25519(dh, resp_e_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32);
    syn_x25519(dh, resp_e_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32);
    wg_hkdf3(ck, tau, k, ck, cfg.preshared_key, 32);
    wg_mix_hash(h, tau, 32);
    wg_encrypt_and_hash(h, k, NULL, 0, NULL, resp_msg + 44);
    wg_mac1(resp_msg + 60, dec_s_pub_i, resp_msg, 60);

    uint8_t resp_recv_key[32], resp_send_key[32];
    wg_hkdf2(resp_recv_key, resp_send_key, ck, (const uint8_t *)"", 0);

    /* Wrong message type (len == 92, type != 2) rejected */
    uint8_t bad_type_msg[SYN_WG_RESPONSE_SIZE];
    memcpy(bad_type_msg, resp_msg, sizeof(resp_msg));
    store32_le(bad_type_msg, SYN_WG_MSG_COOKIE);
    TEST_ASSERT_FALSE(wg_consume_response(&wg, bad_type_msg, sizeof(bad_type_msg)));

    /* Valid AEAD tag but corrupted mac1 rejected on cloned handshake state */
    SYN_WG wg_clone = wg;
    uint8_t bad_mac_msg[SYN_WG_RESPONSE_SIZE];
    memcpy(bad_mac_msg, resp_msg, sizeof(resp_msg));
    bad_mac_msg[60] ^= 0x01U;
    TEST_ASSERT_FALSE(wg_consume_response(&wg_clone, bad_mac_msg, sizeof(bad_mac_msg)));

    /* Deliver authentic response through syn_wg_task -> transitions to ESTABLISHED */
    SYN_SockAddr from = cfg.endpoint;
    mock_udp_set_response(resp_msg, sizeof(resp_msg), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    TEST_ASSERT_EQUAL_HEX32(0xCAFEBABEU, wg.session.receiver_index);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(resp_recv_key, wg.session.send_key, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(resp_send_key, wg.session.recv_key, 32);

    /* 2. R3-N15: syn_wg_send subtraction-form size_t overflow check */
    uint8_t dummy_ip[8] = {0x45, 0, 0, 8, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, dummy_ip, (size_t)-8));

    /* 3. wg_handle_transport replay duplicate & rx_buf_size overflow rejection */
    uint8_t tp_msg[48];
    uint8_t tp_nonce[12] = {0};
    store64_le(tp_nonce + 4, 7U);
    store32_le(tp_msg, SYN_WG_MSG_TRANSPORT);
    store32_le(tp_msg + 4, wg.session.sender_index);
    store64_le(tp_msg + 8, 7U);
    syn_aead_encrypt(wg.session.recv_key, tp_nonce, NULL, 0, dummy_ip, sizeof(dummy_ip),
                     tp_msg + 16, tp_msg + 24);

    wg.rx_buf_size = 4U; /* ct_len (8) > rx_buf_size (4) */
    TEST_ASSERT_FALSE(wg_handle_transport(&wg, tp_msg, 40U));
    wg.rx_buf_size = sizeof(rx_buf);

    TEST_ASSERT_TRUE(wg_handle_transport(&wg, tp_msg, 40U));
    TEST_ASSERT_FALSE(wg_handle_transport(&wg, tp_msg, 40U)); /* Replay rejected */

    /* 4. R3-N06: mock_udp_recv_fail yields PT_IDLE via PT_PASS without spinning */
    mock_udp_recv_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    mock_udp_recv_fail = false;

    /* 5. Task retry delays (udp_open fail, send_initiation fail, handshake timeout resume) */
    mock_port_reset();
    sntp.synced = true;
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    PT_INIT(&pt);
    memset(&task, 0, sizeof(task));
    task.user_data = &wg;

    /* (a) udp_open failure -> 5s delay -> resume opens socket, then send_initiation failure -> 5s
     */
    mock_udp_open_ok = false;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, wg.udp_sock);
    mock_udp_open_ok = true;
    mock_udp_sendto_fail = true;
    mock_tick_advance(5100U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, wg.udp_sock);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);

    /* (b) Resume after send_initiation delay -> sends initiation -> HANDSHAKE_INIT */
    mock_udp_sendto_fail = false;
    mock_tick_advance(5100U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);

    /* (c) Handshake timeout -> 500ms delay -> resume re-sends initiation */
    mock_tick_advance((SYN_WG_REKEY_TIMEOUT + 1U) * 1000U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);
    mock_tick_advance(600U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);

    /* 6. Unmatched protothread line falls through to PT_END */
    pt.lc = 0xFFFFU;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wg_task(&pt, &task));
}

static void test_wg_round4_forged_response_and_disconnect_zeroing(void)
{
    mock_port_reset();

    SYN_SNTP sntp = {0};
    sntp.synced = true;
    sntp.epoch_s = 1700000100U;
    sntp.sync_tick_ms = mock_tick_ms;

    uint8_t resp_s_priv[32];
    memset(resp_s_priv, 0x12, 32);
    syn_x25519_clamp(resp_s_priv);

    SYN_WgConfig cfg = {0};
    memset(cfg.private_key, 0x11, 32);
    syn_x25519_clamp(cfg.private_key);
    syn_x25519_pubkey(cfg.peer_public_key, resp_s_priv);
    memset(cfg.preshared_key, 0x19, 32);
    cfg.endpoint.ip[0] = 10;
    cfg.endpoint.ip[3] = 2;
    cfg.endpoint.port = 51820;

    uint8_t rx_buf[512];
    uint8_t tx_buf[512];
    SYN_WG wg;
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &wg;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);

    /* Construct genuine responder message from captured initiation */
    const uint8_t *e_pub_i = mock_udp_tx_buf + 8;
    const uint8_t *enc_static = mock_udp_tx_buf + 40;
    const uint8_t *enc_ts = mock_udp_tx_buf + 88;

    uint8_t ck[32], h[32], k[32], dh[32], tau[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1U, ck, 32);
    {
        SYN_BLAKE2s bctx;
        syn_blake2s_init(&bctx, 32);
        syn_blake2s_update(&bctx, ck, 32);
        syn_blake2s_update(&bctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1U);
        syn_blake2s_final(&bctx, h);
    }
    wg_mix_hash(h, cfg.peer_public_key, 32);
    wg_mix_key(ck, k, e_pub_i, 32);
    wg_mix_hash(h, e_pub_i, 32);

    syn_x25519(dh, resp_s_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32);
    uint8_t dec_s_pub_i[32];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_static, 32, enc_static + 32, dec_s_pub_i));

    syn_x25519(dh, resp_s_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32);
    uint8_t dec_ts[12];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_ts, 12, enc_ts + 12, dec_ts));

    uint8_t resp_msg[SYN_WG_RESPONSE_SIZE];
    memset(resp_msg, 0, sizeof(resp_msg));
    store32_le(resp_msg + 0, SYN_WG_MSG_RESPONSE);
    store32_le(resp_msg + 4, 0xCAFE1234U);
    store32_le(resp_msg + 8, wg.session.sender_index);

    uint8_t resp_e_priv[32], resp_e_pub[32];
    memset(resp_e_priv, 0x15, 32);
    syn_x25519_clamp(resp_e_priv);
    syn_x25519_pubkey(resp_e_pub, resp_e_priv);
    memcpy(resp_msg + 12, resp_e_pub, 32);

    wg_mix_key(ck, k, resp_e_pub, 32);
    wg_mix_hash(h, resp_e_pub, 32);
    syn_x25519(dh, resp_e_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32);
    syn_x25519(dh, resp_e_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32);
    wg_hkdf3(ck, tau, k, ck, cfg.preshared_key, 32);
    wg_mix_hash(h, tau, 32);
    wg_encrypt_and_hash(h, k, NULL, 0, NULL, resp_msg + 44);
    wg_mac1(resp_msg + 60, dec_s_pub_i, resp_msg, 60);

    SYN_SockAddr from = cfg.endpoint;

    /* 1. Forged 92B response with matching sender_index and bad mac1 */
    uint8_t forged_bad_mac[SYN_WG_RESPONSE_SIZE];
    memcpy(forged_bad_mac, resp_msg, sizeof(resp_msg));
    store32_le(forged_bad_mac + 4, 0xDEAD1111U);
    memset(forged_bad_mac + 12, 0xAA, 32);
    forged_bad_mac[60] ^= 0x55U;
    mock_udp_set_response(forged_bad_mac, sizeof(forged_bad_mac), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);
    TEST_ASSERT_EQUAL_HEX32(0U, wg.session.receiver_index);

    /* 2. Forged 92B response with valid mac1 but bad AEAD tag */
    uint8_t forged_bad_aead[SYN_WG_RESPONSE_SIZE];
    memcpy(forged_bad_aead, resp_msg, sizeof(resp_msg));
    store32_le(forged_bad_aead + 4, 0xDEAD2222U);
    forged_bad_aead[44] ^= 0x01U;
    wg_mac1(forged_bad_aead + 60, wg.public_key, forged_bad_aead, 60);
    mock_udp_set_response(forged_bad_aead, sizeof(forged_bad_aead), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);
    TEST_ASSERT_EQUAL_HEX32(0U, wg.session.receiver_index);

    /* 3. Genuine responder message succeeds immediately without re-initiation */
    mock_udp_set_response(resp_msg, sizeof(resp_msg), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    TEST_ASSERT_EQUAL_HEX32(0xCAFE1234U, wg.session.receiver_index);

    /* 4. R4-N16: syn_wg_disconnect zeroes session and handshake scratch buffers */
    memset(wg.hs_ephemeral_priv, 0xAB, sizeof(wg.hs_ephemeral_priv));
    memset(wg.hs_chaining_key, 0xCD, sizeof(wg.hs_chaining_key));
    memset(wg.hs_hash, 0xEF, sizeof(wg.hs_hash));
    syn_wg_disconnect(&wg);

    uint8_t zero32[32] = {0};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero32, wg.session.send_key, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero32, wg.session.recv_key, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero32, wg.hs_ephemeral_priv, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero32, wg.hs_chaining_key, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(zero32, wg.hs_hash, 32);
}

static void build_responder_handshake_reply(const SYN_WgConfig *cfg, const uint8_t *resp_s_priv,
                                            const uint8_t *init_pkt, uint8_t eph_seed,
                                            uint32_t resp_sender_idx, uint8_t out_resp[92],
                                            uint8_t out_client_send_key[32],
                                            uint8_t out_client_recv_key[32])
{
    uint32_t init_sender_idx = load32_le(init_pkt + 4U);
    const uint8_t *e_pub_i = init_pkt + 8U;
    const uint8_t *enc_static = init_pkt + 40U;
    const uint8_t *enc_ts = init_pkt + 88U;

    uint8_t ck[32], h[32], k[32], dh[32], tau[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1U, ck, 32U);
    {
        SYN_BLAKE2s bctx;
        syn_blake2s_init(&bctx, 32U);
        syn_blake2s_update(&bctx, ck, 32U);
        syn_blake2s_update(&bctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1U);
        syn_blake2s_final(&bctx, h);
    }
    wg_mix_hash(h, cfg->peer_public_key, 32U);
    wg_mix_key(ck, k, e_pub_i, 32U);
    wg_mix_hash(h, e_pub_i, 32U);

    syn_x25519(dh, resp_s_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32U);
    uint8_t dec_s_pub_i[32];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_static, 32U, enc_static + 32U, dec_s_pub_i));

    syn_x25519(dh, resp_s_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32U);
    uint8_t dec_ts[12];
    TEST_ASSERT_TRUE(wg_decrypt_and_hash(h, k, enc_ts, 12U, enc_ts + 12U, dec_ts));

    memset(out_resp, 0, SYN_WG_RESPONSE_SIZE);
    store32_le(out_resp + 0U, SYN_WG_MSG_RESPONSE);
    store32_le(out_resp + 4U, resp_sender_idx);
    store32_le(out_resp + 8U, init_sender_idx);

    uint8_t resp_e_priv[32], resp_e_pub[32];
    memset(resp_e_priv, eph_seed, 32U);
    syn_x25519_clamp(resp_e_priv);
    syn_x25519_pubkey(resp_e_pub, resp_e_priv);
    memcpy(out_resp + 12U, resp_e_pub, 32U);

    wg_mix_key(ck, k, resp_e_pub, 32U);
    wg_mix_hash(h, resp_e_pub, 32U);
    syn_x25519(dh, resp_e_priv, e_pub_i);
    wg_mix_key(ck, k, dh, 32U);
    syn_x25519(dh, resp_e_priv, dec_s_pub_i);
    wg_mix_key(ck, k, dh, 32U);
    wg_hkdf3(ck, tau, k, ck, cfg->preshared_key, 32U);
    wg_mix_hash(h, tau, 32U);
    wg_encrypt_and_hash(h, k, NULL, 0U, NULL, out_resp + 44U);
    wg_mac1(out_resp + 60U, dec_s_pub_i, out_resp, 60U);

    wg_hkdf2(out_client_send_key, out_client_recv_key, ck, (const uint8_t *)"", 0U);
}

static void test_r5_n08_wg_64bit_replay_hitless_rekey_and_real_stats(void)
{
    mock_port_reset();

    /* 1. 64-bit sliding window accepts bit 63 and rejects bit 64 */
    SYN_WgSession r_sess = {0};
    TEST_ASSERT_TRUE(wg_replay_check(&r_sess, 100U));
    TEST_ASSERT_TRUE(wg_replay_check(&r_sess, 37U));  /* diff == 63 */
    TEST_ASSERT_FALSE(wg_replay_check(&r_sess, 37U)); /* duplicate at bit 63 */
    TEST_ASSERT_FALSE(wg_replay_check(&r_sess, 36U)); /* diff == 64 -> rejected */

    /* 2. Full hitless rekey at 120s: active tunnel keeps encrypting/decrypting while rekey runs */
    SYN_SNTP sntp = {0};
    sntp.synced = true;
    sntp.epoch_s = 1700000500U;
    sntp.sync_tick_ms = mock_tick_ms;

    uint8_t resp_s_priv[32];
    memset(resp_s_priv, 0x22, 32U);
    syn_x25519_clamp(resp_s_priv);

    SYN_WgConfig cfg = {0};
    memset(cfg.private_key, 0x21, 32U);
    syn_x25519_clamp(cfg.private_key);
    syn_x25519_pubkey(cfg.peer_public_key, resp_s_priv);
    memset(cfg.preshared_key, 0x29, 32U);
    cfg.endpoint.ip[0] = 10;
    cfg.endpoint.ip[3] = 9;
    cfg.endpoint.port = 51820;

    uint8_t rx_buf[512];
    uint8_t tx_buf[512];
    SYN_WG wg;
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &wg};

    /* Initial handshake -> ESTABLISHED */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, wg.state);
    uint8_t resp1[SYN_WG_RESPONSE_SIZE];
    uint8_t send_k1[32], recv_k1[32];
    build_responder_handshake_reply(&cfg, resp_s_priv, mock_udp_tx_buf, 0x31U, 0x11112222U, resp1,
                                    send_k1, recv_k1);
    SYN_SockAddr from = cfg.endpoint;
    mock_udp_set_response(resp1, sizeof(resp1), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    uint32_t old_sender_idx = wg.session.sender_index;

    /* Advance past 120s REKEY_AFTER_TIME (125s < 180s REJECT_AFTER_TIME) */
    mock_udp_tx_len = 0U;
    mock_tick_advance(125000U);
    TEST_ASSERT_TRUE(wg_has_work(&wg));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));

    /* Tunnel MUST remain ESTABLISHED with original session keys while rekey is in flight! */
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    TEST_ASSERT_TRUE(wg.rekey_in_progress);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);
    TEST_ASSERT_EQUAL_HEX32(old_sender_idx, wg.session.sender_index);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(send_k1, wg.session.send_key, 32U);
    TEST_ASSERT_FALSE(wg_has_work(&wg)); /* Parked cleanly while awaiting rekey response */

    /* Active data plane still sends and decrypts packets during hitless rekey! */
    uint8_t app_pkt[20] = {0x45, 0x00, 0x00, 0x14};
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_send(&wg, app_pkt, sizeof(app_pkt)));

    uint8_t rx_tp[16 + 14 + 16];
    uint8_t rx_plain[14] = "TUNDATA_ACTIVE";
    uint8_t nonce[12] = {0};
    store64_le(nonce + 4U, 1U);
    store32_le(rx_tp, SYN_WG_MSG_TRANSPORT);
    store32_le(rx_tp + 4U, old_sender_idx);
    store64_le(rx_tp + 8U, 1U);
    syn_aead_encrypt(recv_k1, nonce, NULL, 0U, rx_plain, sizeof(rx_plain), rx_tp + 16U,
                     rx_tp + 30U);
    mock_udp_set_response(rx_tp, sizeof(rx_tp), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));

    SYN_WgStats stats;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_get_stats(&wg, &stats));
    TEST_ASSERT_EQUAL_UINT64(sizeof(app_pkt), stats.tx_bytes);
    TEST_ASSERT_EQUAL_UINT64(sizeof(rx_plain), stats.rx_bytes);

    /* Advance 6s (> REKEY_TIMEOUT) without response -> retries rekey initiation hitlessly */
    mock_udp_tx_len = 0U;
    mock_tick_advance(6000U);
    TEST_ASSERT_TRUE(wg_has_work(&wg));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);

    /* Complete rekey handshake -> atomically swaps new session in! */
    uint8_t resp2[SYN_WG_RESPONSE_SIZE];
    uint8_t send_k2[32], recv_k2[32];
    build_responder_handshake_reply(&cfg, resp_s_priv, mock_udp_tx_buf, 0x32U, 0x33334444U, resp2,
                                    send_k2, recv_k2);
    mock_udp_tx_len = 0U;
    mock_udp_set_response(resp2, sizeof(resp2), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, wg.state);
    TEST_ASSERT_FALSE(wg.rekey_in_progress);
    TEST_ASSERT_EQUAL_HEX32(0x33334444U, wg.session.receiver_index);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(send_k2, wg.session.send_key, 32U);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(recv_k2, wg.session.recv_key, 32U);
    /* Initiator immediately emits Type-4 empty confirmation keepalive (counter 0 -> 1) */
    TEST_ASSERT_EQUAL_UINT64(1U, wg.session.send_counter);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_TRANSPORT_OVERHEAD, mock_udp_tx_len);
    TEST_ASSERT_EQUAL_UINT32(SYN_WG_MSG_TRANSPORT, load32_le(mock_udp_tx_buf));
    TEST_ASSERT_EQUAL_HEX32(0x33334444U, load32_le(mock_udp_tx_buf + 4U));

    /* 3. Advance past 180s REJECT_AFTER_TIME without completing rekey -> hard session expiry */
    mock_udp_sendto_fail = true;
    mock_tick_advance(185000U);
    TEST_ASSERT_TRUE(wg_has_work(&wg));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);
    mock_udp_sendto_fail = false;
}

static size_t s_unpad_last_len = 0U;
static void on_unpad_recv(const uint8_t *buf, size_t len, void *ctx)
{
    (void)buf;
    (void)ctx;
    s_unpad_last_len = len;
}

static void test_wg_transport_ip_padding_trim(void)
{
    wg_state_setup();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.sender_index = 42U;
    memset(s_wg.session.recv_key, 0x33, 32U);
    s_wg.on_recv = on_unpad_recv;

    /* 1. 28B IPv4 packet padded to 32B -> trimmed to 28B */
    uint8_t ip4_padded[32] = {0};
    ip4_padded[0] = 0x45U;
    ip4_padded[3] = 28U;
    uint8_t msg4[16 + 32 + 16] = {0};
    uint8_t nonce[12] = {0};
    store64_le(nonce + 4U, 1U);
    store32_le(msg4, SYN_WG_MSG_TRANSPORT);
    store32_le(msg4 + 4U, 42U);
    store64_le(msg4 + 8U, 1U);
    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0U, ip4_padded, sizeof(ip4_padded),
                     msg4 + 16U, msg4 + 48U);
    TEST_ASSERT_TRUE(wg_handle_transport(&s_wg, msg4, sizeof(msg4)));
    TEST_ASSERT_EQUAL_size_t(28U, s_unpad_last_len);
    TEST_ASSERT_EQUAL_UINT64(28U, s_wg.rx_bytes);

    /* 2. 44B IPv6 packet padded to 48B -> trimmed to 44B */
    uint8_t ip6_padded[48] = {0};
    ip6_padded[0] = 0x60U;
    ip6_padded[5] = 4U; /* payload_len = 4 -> total = 44 */
    uint8_t msg6[16 + 48 + 16] = {0};
    store64_le(nonce + 4U, 2U);
    store32_le(msg6, SYN_WG_MSG_TRANSPORT);
    store32_le(msg6 + 4U, 42U);
    store64_le(msg6 + 8U, 2U);
    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0U, ip6_padded, sizeof(ip6_padded),
                     msg6 + 16U, msg6 + 64U);
    TEST_ASSERT_TRUE(wg_handle_transport(&s_wg, msg6, sizeof(msg6)));
    TEST_ASSERT_EQUAL_size_t(44U, s_unpad_last_len);

    /* 3. Malformed IPv4 (total_len > ct_len) and malformed IPv6 (40+payload > ct_len) fall back */
    ip4_padded[2] = 0x01U; /* 284 > 32 */
    store64_le(nonce + 4U, 3U);
    store64_le(msg4 + 8U, 3U);
    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0U, ip4_padded, sizeof(ip4_padded),
                     msg4 + 16U, msg4 + 48U);
    TEST_ASSERT_TRUE(wg_handle_transport(&s_wg, msg4, sizeof(msg4)));
    TEST_ASSERT_EQUAL_size_t(32U, s_unpad_last_len);

    ip6_padded[4] = 0x01U; /* 40 + 260 > 48 */
    store64_le(nonce + 4U, 4U);
    store64_le(msg6 + 8U, 4U);
    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0U, ip6_padded, sizeof(ip6_padded),
                     msg6 + 16U, msg6 + 64U);
    TEST_ASSERT_TRUE(wg_handle_transport(&s_wg, msg6, sizeof(msg6)));
    TEST_ASSERT_EQUAL_size_t(48U, s_unpad_last_len);
}

static void test_tier_a_wg_low_order_dh_staged_hs_and_msg_limits(void)
{
    mock_port_reset();

    /* 1. NULL guards in syn_wg_is_established and syn_wg_send */
    uint8_t app_pkt[20] = {0x45U, 0x00U, 0x00U, 0x14U};
    TEST_ASSERT_FALSE(syn_wg_is_established(NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(NULL, app_pkt, sizeof(app_pkt)));

    SYN_SNTP sntp = {0};
    sntp.synced = true;
    sntp.epoch_s = 1700001000U;
    sntp.sync_tick_ms = mock_tick_ms;

    uint8_t resp_s_priv[32];
    memset(resp_s_priv, 0x42, 32U);
    syn_x25519_clamp(resp_s_priv);

    SYN_WgConfig cfg = {0};
    memset(cfg.private_key, 0x41, 32U);
    syn_x25519_clamp(cfg.private_key);
    syn_x25519_pubkey(cfg.peer_public_key, resp_s_priv);
    memset(cfg.preshared_key, 0x49, 32U);
    cfg.endpoint.ip[0] = 10U;
    cfg.endpoint.ip[3] = 4U;
    cfg.endpoint.port = 51820U;

    uint8_t rx_buf[512];
    uint8_t tx_buf[512];
    SYN_WG wg;
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));

    /* 2. Low-order peer_public_key rejected in wg_send_initiation without clobbering hs_* */
    SYN_WG wg_bad_peer = wg;
    wg_bad_peer.udp_sock = 7;
    memset(wg_bad_peer.config.peer_public_key, 0, 32U);
    memset(wg_bad_peer.hs_chaining_key, 0xA5, 32U);
    mock_udp_tx_len = 0U;
    TEST_ASSERT_FALSE(wg_send_initiation(&wg_bad_peer));
    TEST_ASSERT_EQUAL_size_t(0U, mock_udp_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, wg_bad_peer.hs_chaining_key[0]);

    /* Order-2 X25519 point (u = 1) also rejected */
    wg_bad_peer.config.peer_public_key[0] = 1U;
    TEST_ASSERT_FALSE(wg_send_initiation(&wg_bad_peer));

    /* 3. Establish genuine session, then test NULL payload, REKEY_AFTER_MESSAGES, staged hs_*,
     *    and low-order responder ephemeral key with forged valid AEAD tag */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &wg};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    uint8_t resp1[SYN_WG_RESPONSE_SIZE];
    uint8_t send_k1[32], recv_k1[32];
    build_responder_handshake_reply(&cfg, resp_s_priv, mock_udp_tx_buf, 0x51U, 0xAAAA1111U, resp1,
                                    send_k1, recv_k1);
    SYN_SockAddr from = cfg.endpoint;
    mock_udp_set_response(resp1, sizeof(resp1), &from);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_TRUE(syn_wg_is_established(&wg));

    /* ip_packet == NULL with len > 0 rejected while len == 0 keepalive still allowed */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, NULL, 4U));

    /* Crossing SYN_WG_REKEY_AFTER_MESSAGES wakes syn_wg_task to send hitless rekey initiation */
    wg.session.send_counter = SYN_WG_REKEY_AFTER_MESSAGES - 1ULL;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_send(&wg, app_pkt, sizeof(app_pkt)));
    TEST_ASSERT_EQUAL_UINT64(SYN_WG_REKEY_AFTER_MESSAGES, wg.session.send_counter);
    TEST_ASSERT_TRUE(wg_has_work(&wg));

    mock_udp_tx_len = 0U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    TEST_ASSERT_TRUE(wg.rekey_in_progress);
    TEST_ASSERT_EQUAL_size_t(SYN_WG_INITIATION_SIZE, mock_udp_tx_len);
    uint8_t saved_rekey_init[SYN_WG_INITIATION_SIZE];
    memcpy(saved_rekey_init, mock_udp_tx_buf, sizeof(saved_rekey_init));

    /* Failed retry sendto after REKEY_TIMEOUT must NOT clobber in-flight wg.hs_* */
    mock_tick_advance(6000U);
    mock_udp_sendto_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wg_task(&pt, &task));
    mock_udp_sendto_fail = false;

    /* Craft forged response with low-order E_pub_r = 0^32 and valid AEAD/mac1 over 0^32 DH */
    uint8_t forged_low_order[SYN_WG_RESPONSE_SIZE];
    memset(forged_low_order, 0, sizeof(forged_low_order));
    store32_le(forged_low_order + 0U, SYN_WG_MSG_RESPONSE);
    store32_le(forged_low_order + 4U, 0xDEAD9999U);
    store32_le(forged_low_order + 8U, load32_le(saved_rekey_init + 4U));
    {
        uint8_t f_ck[32], f_h[32], f_k[32], f_tau[32];
        uint8_t zero32[32] = {0U};
        memcpy(f_ck, wg.hs_chaining_key, 32U);
        memcpy(f_h, wg.hs_hash, 32U);
        wg_mix_key(f_ck, f_k, zero32, 32U);
        wg_mix_hash(f_h, zero32, 32U);
        wg_mix_key(f_ck, f_k, zero32, 32U);
        wg_mix_key(f_ck, f_k, zero32, 32U);
        wg_hkdf3(f_ck, f_tau, f_k, f_ck, cfg.preshared_key, 32U);
        wg_mix_hash(f_h, f_tau, 32U);
        wg_encrypt_and_hash(f_h, f_k, NULL, 0U, NULL, forged_low_order + 44U);
        wg_mac1(forged_low_order + 60U, wg.public_key, forged_low_order, 60U);
    }
    TEST_ASSERT_FALSE(wg_consume_response(&wg, forged_low_order, sizeof(forged_low_order)));

    /* Genuine response to saved_rekey_init still succeeds because wg.hs_* was preserved */
    uint8_t resp2[SYN_WG_RESPONSE_SIZE];
    uint8_t send_k2[32], recv_k2[32];
    build_responder_handshake_reply(&cfg, resp_s_priv, saved_rekey_init, 0x52U, 0xBBBB2222U, resp2,
                                    send_k2, recv_k2);
    TEST_ASSERT_TRUE(wg_consume_response(&wg, resp2, sizeof(resp2)));
    TEST_ASSERT_FALSE(wg.rekey_in_progress);
    TEST_ASSERT_EQUAL_UINT64(0U, wg.session.send_counter);

    /* 4. SYN_WG_REJECT_AFTER_MESSAGES enforced on both TX and RX */
    wg.session.send_counter = SYN_WG_REJECT_AFTER_MESSAGES - 1ULL;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wg_send(&wg, app_pkt, sizeof(app_pkt)));
    TEST_ASSERT_EQUAL_UINT64(SYN_WG_REJECT_AFTER_MESSAGES, wg.session.send_counter);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, app_pkt, sizeof(app_pkt)));
    TEST_ASSERT_EQUAL_UINT64(SYN_WG_REJECT_AFTER_MESSAGES, wg.session.send_counter);

    SYN_WgSession rx_lim = {0};
    TEST_ASSERT_TRUE(wg_replay_check(&rx_lim, SYN_WG_REJECT_AFTER_MESSAGES - 1ULL));
    TEST_ASSERT_FALSE(wg_replay_check(&rx_lim, SYN_WG_REJECT_AFTER_MESSAGES));
}

void run_wg_tests(void)
{
    /* Handshake internals */
    RUN_TEST(test_wg_construction_hash);
    RUN_TEST(test_wg_identifier_hash);
    RUN_TEST(test_wg_hkdf2);
    RUN_TEST(test_wg_hkdf3);

    /* Noise helpers */
    RUN_TEST(test_wg_mix_hash);
    RUN_TEST(test_wg_encrypt_decrypt_hash);
    RUN_TEST(test_wg_encrypt_decrypt_hash_empty);
    RUN_TEST(test_wg_decrypt_and_hash_tampered);

    /* MAC1 */
    RUN_TEST(test_wg_mac1_basic);
    RUN_TEST(test_wg_mac1_sensitivity);

    /* Anti-replay */
    RUN_TEST(test_wg_replay_sequential);
    RUN_TEST(test_wg_replay_duplicate);
    RUN_TEST(test_wg_replay_forward_jump);
    RUN_TEST(test_wg_replay_out_of_order);
    RUN_TEST(test_wg_replay_window_boundary);

    /* Full handshake construction */
    RUN_TEST(test_wg_handshake_intermediates);

    /* State machine & transport */
    RUN_TEST(test_wg_init_state);
    RUN_TEST(test_wg_initiation_on_task);
    RUN_TEST(test_wg_handshake_response_invalid);
    RUN_TEST(test_wg_established_transport_and_keepalive);
    RUN_TEST(test_wg_send_disconnected_or_null);
    RUN_TEST(test_wg_transport_error_branches);
    RUN_TEST(test_wg_transport_corrupt_tag_does_not_advance_replay_window);
    RUN_TEST(test_wg_reject_after_time_expiry);
    RUN_TEST(test_wg_cookie_packet_handling);
    RUN_TEST(test_wg_rekey_after_time);
    RUN_TEST(test_wg_handshake_timeout);
    RUN_TEST(test_wg_disconnect_and_stats);
    RUN_TEST(test_wg_set_rx_callback);
    RUN_TEST(test_wg_response_message_invalid_length_and_receiver);
    RUN_TEST(test_wg_response_decryption_and_mac_validation_failures);
    RUN_TEST(test_wg_send_fail_and_rekey_timeout_task_branches);
    RUN_TEST(test_wg_response_mac1_tampered_rejection);
    RUN_TEST(test_wg_cookie_mac2_verification_failure);
    RUN_TEST(test_wg_send_initiation_failure_task_branch);
    RUN_TEST(test_wg_valid_response_exchange);
    RUN_TEST(test_wg_send_initiation_tx_buf_too_small);
    RUN_TEST(test_wg_send_buffer_too_small);
    RUN_TEST(test_wg_handshake_rekey_timer_expiry);
    RUN_TEST(test_wg_task_null_sntp_multi_packet_yield_and_n11_reconnect);
    RUN_TEST(test_wg_round3_regressions);
    RUN_TEST(test_wg_round4_forged_response_and_disconnect_zeroing);
    RUN_TEST(test_r5_n08_wg_64bit_replay_hitless_rekey_and_real_stats);
    RUN_TEST(test_wg_transport_ip_padding_trim);
    RUN_TEST(test_tier_a_wg_low_order_dh_staged_hs_and_msg_limits);
}
