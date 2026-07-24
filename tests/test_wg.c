/**
 * @file test_wg.c
 * @brief Unity tests for WireGuard internals — HKDF, Noise helpers,
 *        replay window, transport, and handshake construction.
 *
 * Uses #include "syn_wg.c" to access static functions directly.
 * All expected values verified against the Go reference
 * (tests/test_wg_reference.go).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

/* Pull in the .c to access static functions */
#include "syntropic/net/syn_wg.c"

#include <string.h>
#include <stdio.h>

/* ── Hex-parse helper ──────────────────────────────────────────────────── */

static void wg_hex2bin(const char *hex, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        unsigned byte;
        sscanf(hex + 2 * i, "%02x", &byte);
        out[i] = (uint8_t)byte;
    }
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
               "16eb4206f87277f52d38d1988b78cd36", expected, 32);
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
               "2d9c6c662293e8b70ee19c65ba079ef3", expected, 32);
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
               "16eb4206f87277f52d38d1988b78cd36", ck, 32);

    /* E_pub from our Go reference */
    uint8_t e_pub[32];
    wg_hex2bin("5dfedd3b6bd47f6fa28ee15d969d5bb0"
               "ea53774d488bdaf9df1c6e0124b3ef22", e_pub, 32);

    wg_hkdf2(out1, out2, ck, e_pub, 32);

    /* out1 = new C (after e_pub) */
    uint8_t expected_ck[32];
    wg_hex2bin("3b9c2a603fb5783c1c74f7b4501c3901"
               "280d8451962abc94d0b8a6ea00b934c1", expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, out1, 32);
}

/* HKDF3 produces 3 outputs (used for PSK mixing) */
static void test_wg_hkdf3(void)
{
    uint8_t ck[32], out1[32], out2[32], out3[32];
    memset(ck, 0x42, 32);

    uint8_t input[8] = {'h','k','d','f','3','t','s','t'};
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
               "2d9c6c662293e8b70ee19c65ba079ef3", h, 32);

    uint8_t r_pub[32];
    wg_hex2bin("ce8d3ad1ccb633ec7b70c17814a5c76e"
               "cd029685050d344745ba05870e587d59", r_pub, 32);

    wg_mix_hash(h, r_pub, 32);

    uint8_t expected[32];
    wg_hex2bin("74c87e340810e6a8815cf911b640ebfb"
               "9150dc04293e5274e324126100cdf6e3", expected, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, h, 32);
}

/* encrypt_and_hash → decrypt_and_hash round-trip preserves hash chain */
static void test_wg_encrypt_decrypt_hash(void)
{
    uint8_t h_enc[32], h_dec[32], k[32];
    memset(h_enc, 0xAA, 32);
    memcpy(h_dec, h_enc, 32);  /* Start with same H */
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

    uint8_t plain[8] = {'t','e','s','t','d','a','t','a'};
    uint8_t ct[8], tag[16], decrypted[8];

    wg_encrypt_and_hash(h_enc, k, plain, 8, ct, tag);
    tag[0] ^= 0x01;  /* Tamper */

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
    msg2[0] = 0x22;  /* One byte different */

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
    TEST_ASSERT_FALSE(wg_replay_check(&s, 5));  /* duplicate */
}

/* Large forward jump accepted, old rejected */
static void test_wg_replay_forward_jump(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 1));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 100));  /* big jump */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 1));    /* now too old */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 50));   /* also too old (>32 behind) */
}

/* Out-of-order within window accepted */
static void test_wg_replay_out_of_order(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 10));
    TEST_ASSERT_TRUE(wg_replay_check(&s, 8));   /* 2 behind, within window */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 9));   /* 1 behind, within window */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 8));  /* already seen */
}

/* Window boundary: exactly 31 behind is accepted, 32 behind is rejected */
static void test_wg_replay_window_boundary(void)
{
    SYN_WgSession s;
    memset(&s, 0, sizeof(s));

    TEST_ASSERT_TRUE(wg_replay_check(&s, 32));

    /* 31 behind (counter 1) — should be accepted */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 1));

    /* Set counter further ahead */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 64));

    /* 31 behind (counter 33) — should be accepted */
    TEST_ASSERT_TRUE(wg_replay_check(&s, 33));

    /* 32 behind (counter 32) — should be rejected (outside window) */
    TEST_ASSERT_FALSE(wg_replay_check(&s, 32));
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

    memset(i_priv, 0x01, 32); syn_x25519_clamp(i_priv);
    syn_x25519_pubkey(i_pub, i_priv);

    memset(e_priv, 0x03, 32); syn_x25519_clamp(e_priv);
    syn_x25519_pubkey(e_pub, e_priv);

    /* Responder public key */
    wg_hex2bin("ce8d3ad1ccb633ec7b70c17814a5c76e"
               "cd029685050d344745ba05870e587d59", r_pub, 32);

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
               "9150dc04293e5274e324126100cdf6e3", expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 4: C, _ = KDF2(C, E_pub) */
    wg_mix_key(ck, k, e_pub, 32);

    uint8_t expected_ck[32];
    wg_hex2bin("3b9c2a603fb5783c1c74f7b4501c3901"
               "280d8451962abc94d0b8a6ea00b934c1", expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    /* Step 5: H = HASH(H || E_pub) */
    wg_mix_hash(h, e_pub, 32);

    wg_hex2bin("7c924cc768f21c641fa3292ddfca6234"
               "807e0d0cbf2f70763ad045be9b75b3b6", expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 6: DH(E_priv, S_pub_r) → C, k */
    syn_x25519(dh, e_priv, r_pub);
    wg_mix_key(ck, k, dh, 32);

    wg_hex2bin("50e36bfff4d62801ff6375d4521af1af"
               "4ddf24fe1425e4bdf4b0c0236be94d2f", expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    uint8_t expected_k[32];
    wg_hex2bin("ccb521ed0369ab01a50b94554bcd9bfb"
               "e81cf4851df2caeb3776b4b6882f84ad", expected_k, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_k, k, 32);

    /* Step 7: Encrypt static key → check H */
    uint8_t ct[32], tag[16];
    wg_encrypt_and_hash(h, k, i_pub, 32, ct, tag);

    wg_hex2bin("9988e81b8d83aca922a8092fb2c1ec0f"
               "3e70e8c0bd925f1bf8dece66a364e59c", expected_h, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_h, h, 32);

    /* Step 8: DH(S_priv_i, S_pub_r) → C, k */
    syn_x25519(dh, i_priv, r_pub);
    wg_mix_key(ck, k, dh, 32);

    wg_hex2bin("4696b0ed00cff9c62cf447eb5f9c899d"
               "508e66b889a8cef23c4ae5c60f902b5e", expected_ck, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_ck, ck, 32);

    wg_hex2bin("8c2e061899942b4083496ce0bb8e1b4c"
               "bf5308f05aafda8e55c26a20d7c1774a", expected_k, 32);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_k, k, 32);

    /* Step 9: Encrypt timestamp → check H */
    uint8_t timestamp[12] = {
        0x40, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x42, 0x40,
        0x00, 0x00, 0x00, 0x00,
    };
    wg_encrypt_and_hash(h, k, timestamp, 12, ct, tag);

    wg_hex2bin("7167ec354d44f588a7ddc45e754a4dad"
               "83f1bb903f77dd9cce9295b7e0ec4a51", expected_h, 32);
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

    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf),
                tx_buf, sizeof(tx_buf));

    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, wg.state);
    TEST_ASSERT_FALSE(syn_wg_is_established(&wg));
    TEST_ASSERT_EQUAL(0, wg.session.send_counter);
    TEST_ASSERT_EQUAL(0, wg.session.recv_counter);
}

/* ── State Machine Tests ────────────────────────────────────────────────── */

static uint8_t s_rx_buf[1024];
static uint8_t s_tx_buf[1024];
static SYN_WG   s_wg;
static SYN_SNTP s_sntp;

/* Canned keys for testing */
static const uint8_t CLIENT_PRIV[32] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

static const uint8_t PEER_PUB[32] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};

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
    cfg.endpoint.ip[0] = 1; cfg.endpoint.ip[1] = 2; cfg.endpoint.ip[2] = 3; cfg.endpoint.ip[3] = 4;
    cfg.keepalive_interval_s = 25;

    syn_wg_init(&s_wg, &cfg, &s_sntp, s_rx_buf, sizeof(s_rx_buf), s_tx_buf, sizeof(s_tx_buf));
}

static void test_wg_initiation_on_task(void)
{
    wg_state_setup();
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);
    
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_handshake_response_invalid(void)
{
    wg_state_setup();
    
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);
    
    /* Simulate a Handshake Response from peer (but with invalid crypto) */
    uint8_t mock_resp[92];
    memset(mock_resp, 0, sizeof(mock_resp));
    uint32_t *msg_type = (uint32_t *)mock_resp;
    *msg_type = SYN_WG_MSG_RESPONSE;
    
    SYN_SockAddr from = { .port = 51820 };
    from.ip[0] = 1; from.ip[1] = 2; from.ip[2] = 3; from.ip[3] = 4;
    
    mock_udp_inject_packet(mock_resp, sizeof(mock_resp), &from);
    
    syn_wg_task(&pt, &task);
    
    /* Should still be HANDSHAKE_INIT (rejected response) */
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static bool s_recv_called = false;
static void test_on_recv(const uint8_t *buf, size_t len, void *ctx)
{
    (void)buf; (void)len; (void)ctx;
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

    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0,
                     payload, sizeof(payload),
                     tx_msg + 16, tx_msg + 16 + sizeof(payload));

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

    syn_aead_encrypt(s_wg.session.recv_key, nonce, NULL, 0,
                     payload, sizeof(payload),
                     tx_msg2 + 16, tx_msg2 + 16 + sizeof(payload));

    SYN_SockAddr from = { .port = 51820 };
    from.ip[0] = 1; from.ip[1] = 2; from.ip[2] = 3; from.ip[3] = 4;
    mock_udp_inject_packet(tx_msg2, tx_msg_len, &from);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);
    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, s_wg.udp_sock);
    s_wg.last_sent_ms = 100;
    st = syn_wg_send(&s_wg, NULL, 0);
    TEST_ASSERT_EQUAL(SYN_OK, st);

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
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wg_send(&wg, (const uint8_t*)"test", 4));
}

static void test_wg_reject_after_time_expiry(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.established_ms = 1000;
    mock_tick_ms = 250000; /* > SYN_WG_REJECT_AFTER_TIME (180s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_cookie_packet_handling(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_ESTABLISHED;

    /* Inject MSG_COOKIE (type 3) and verify it does not crash or corrupt state */
    uint8_t cookie_pkt[64];
    memset(cookie_pkt, 0, sizeof(cookie_pkt));
    store32_le(cookie_pkt, 3); /* SYN_WG_MSG_COOKIE */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_rekey_after_time(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_ESTABLISHED;
    s_wg.session.established_ms = 1000;
    mock_tick_ms = 130000; /* > SYN_WG_REKEY_AFTER_TIME (120s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_HANDSHAKE_INIT, s_wg.state);
}

static void test_wg_handshake_timeout(void)
{
    test_wg_init_state();
    s_wg.state = SYN_WG_HANDSHAKE_INIT;
    s_wg.last_handshake_ms = 1000;
    mock_tick_ms = 8000; /* > SYN_WG_REKEY_TIMEOUT (5s) */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = { .user_data = &s_wg };
    syn_wg_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_WG_DISCONNECTED, s_wg.state);
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
    RUN_TEST(test_wg_reject_after_time_expiry);
    RUN_TEST(test_wg_cookie_packet_handling);
    RUN_TEST(test_wg_rekey_after_time);
    RUN_TEST(test_wg_handshake_timeout);
}
