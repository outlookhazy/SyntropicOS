/**
 * @file test_dtls.c
 * @brief Unit tests for DTLS 1.3 Datagram Security Protocol (RFC 9147).
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/net/syn_dtls.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

#include <string.h>

typedef struct {
    uint8_t buf[2560];
    size_t len;
    bool fail_send;
} DatagramWire;

typedef struct {
    DatagramWire c2s;
    DatagramWire s2c;
} DuplexDatagramWire;

typedef struct {
    DatagramWire *tx;
    DatagramWire *rx;
} DtlsEndpointIo;

static uint8_t g_dc_rx_buf[2560];
static uint8_t g_dc_tx_buf[2560];
static uint8_t g_ds_rx_buf[2560];
static uint8_t g_ds_tx_buf[2560];

static bool dgram_has_data(const void *ctx)
{
    const DtlsEndpointIo *io = (const DtlsEndpointIo *)ctx;
    return (io != NULL && io->rx != NULL && io->rx->len > 0U);
}

static bool dgram_send(const uint8_t *data, size_t len, void *ctx)
{
    DtlsEndpointIo *io = (DtlsEndpointIo *)ctx;
    DatagramWire *wire = io->tx;
    if (wire->fail_send || len > sizeof(wire->buf)) {
        return false;
    }
    memcpy(wire->buf, data, len);
    wire->len = len;
    return true;
}

static bool dgram_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    DtlsEndpointIo *io = (DtlsEndpointIo *)ctx;
    DatagramWire *wire = io->rx;
    if (wire->len == 0U) {
        return false;
    }
    size_t copy_len = (wire->len > max_len) ? max_len : wire->len;
    memcpy(data, wire->buf, copy_len);
    *out_len = copy_len;
    wire->len = 0U;
    return true;
}

static void dtls_duplex_init(DuplexDatagramWire *dw, DtlsEndpointIo *c_io, DtlsEndpointIo *s_io,
                             SYN_Transport *c_tr, SYN_Transport *s_tr, bool with_has_data)
{
    memset(dw, 0, sizeof(*dw));
    c_io->tx = &dw->c2s;
    c_io->rx = &dw->s2c;
    s_io->tx = &dw->s2c;
    s_io->rx = &dw->c2s;
    c_tr->send = dgram_send;
    c_tr->recv = dgram_recv;
    c_tr->has_data = with_has_data ? dgram_has_data : NULL;
    c_tr->ctx = c_io;
    s_tr->send = dgram_send;
    s_tr->recv = dgram_recv;
    s_tr->has_data = with_has_data ? dgram_has_data : NULL;
    s_tr->ctx = s_io;
}

static const uint8_t g_dtls_srv_seed[32] = {0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U, 0x28U};
static uint8_t g_dtls_srv_pub[32];
static uint8_t g_dtls_srv_sec[32];
static bool g_dtls_keys_ready = false;

static void ensure_dtls_rpk_keys(void)
{
    if (!g_dtls_keys_ready) {
        TEST_ASSERT_TRUE(
            syn_ed25519_create_keypair(g_dtls_srv_pub, g_dtls_srv_sec, g_dtls_srv_seed));
        g_dtls_keys_ready = true;
    }
}

static void build_dtls_ed25519_cert_der(const uint8_t leaf_pub[32], const uint8_t ca_sec[32],
                                        const uint8_t ca_pub[32], uint8_t out_der[184])
{
    static const uint8_t hdr[78] = {
        0x30, 0x81, 0xB5, 0x30, 0x69, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01,
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E,
        0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',
        'A',  0x30, 0x00, 0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04,
        0x03, 0x13, 0x0B, 'T',  'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',
        'r',  0x30, 0x2A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x21, 0x00};
    static const uint8_t sig_hdr[10] = {0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x41, 0x00};

    memcpy(out_der, hdr, sizeof(hdr));
    memcpy(out_der + 78U, leaf_pub, 32U);
    memcpy(out_der + 110U, sig_hdr, sizeof(sig_hdr));
    TEST_ASSERT_TRUE(syn_ed25519_sign(out_der + 3U, 107U, ca_sec, ca_pub, out_der + 120U));
}

static size_t build_dtls_p256_cert_der(const uint8_t *leaf_pub, size_t leaf_pub_len,
                                       const uint8_t ca_priv[32], uint8_t out_der[256])
{
    static const uint8_t tbs_prefix[66] = {
        0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86,
        0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03,
        0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A',  0x30, 0x00,
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r'};
    static const uint8_t spki_alg[21] = {0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48,
                                         0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
                                         0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};
    static const uint8_t sig_hdr[15] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE,
                                        0x3D, 0x04, 0x03, 0x02, 0x03, 0x41, 0x00};

    size_t spki_body = sizeof(spki_alg) + 3U + leaf_pub_len;
    size_t tbs_body = sizeof(tbs_prefix) + 2U + spki_body;
    size_t tbs_hdr = (tbs_body >= 128U) ? 3U : 2U;
    size_t tbs_total = tbs_hdr + tbs_body;
    size_t outer_body = tbs_total + sizeof(sig_hdr) + 64U;

    size_t pos = 0U;
    out_der[pos++] = 0x30U;
    out_der[pos++] = 0x81U;
    out_der[pos++] = (uint8_t)outer_body;

    size_t tbs_start = pos;
    out_der[pos++] = 0x30U;
    if (tbs_body >= 128U) {
        out_der[pos++] = 0x81U;
    }
    out_der[pos++] = (uint8_t)tbs_body;
    memcpy(out_der + pos, tbs_prefix, sizeof(tbs_prefix));
    pos += sizeof(tbs_prefix);
    out_der[pos++] = 0x30U;
    out_der[pos++] = (uint8_t)spki_body;
    memcpy(out_der + pos, spki_alg, sizeof(spki_alg));
    pos += sizeof(spki_alg);
    out_der[pos++] = 0x03U;
    out_der[pos++] = (uint8_t)(1U + leaf_pub_len);
    out_der[pos++] = 0x00U;
    memcpy(out_der + pos, leaf_pub, leaf_pub_len);
    pos += leaf_pub_len;

    uint8_t tbs_hash[32];
    syn_sha256(out_der + tbs_start, tbs_total, tbs_hash);
    memcpy(out_der + pos, sig_hdr, sizeof(sig_hdr));
    pos += sizeof(sig_hdr);
    TEST_ASSERT_TRUE(
        syn_p256_sign_ecdsa_deterministic(ca_priv, tbs_hash, out_der + pos, out_der + pos + 32U));
    return pos + 64U;
}

static void pair_dtls_handshake(SYN_DTLS_Context *client, SYN_DTLS_Context *server)
{
    /* Server poll before ClientHello arrives remains UNINITIALIZED */
    TEST_ASSERT_TRUE(syn_dtls_handshake(server));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_UNINITIALIZED, server->state);

    /* Flight 1: ClientHello */
    TEST_ASSERT_TRUE(syn_dtls_handshake(client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client->state);

    /* Flight 2: ServerHello [+ CV] + Finished */
    TEST_ASSERT_TRUE(syn_dtls_handshake(server));
    if (server->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        TEST_ASSERT_EQUAL(SYN_DTLS_STATE_FINISHED_SENT, server->state);
    } else {
        TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, server->state);
    }

    /* Client verifies ServerHello [+ CV] + Finished (and sends Flight 3 in mTLS) */
    TEST_ASSERT_TRUE(syn_dtls_handshake(client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, client->state);

    if (server->config.mode == SYN_DTLS_AUTH_MODE_MTLS) {
        TEST_ASSERT_TRUE(syn_dtls_handshake(server));
        TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, server->state);
    }
}

void test_dtls_sliding_window_anti_replay(void)
{
    SYN_DTLS_ReplayWindow win;
    memset(&win, 0, sizeof(win));

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 0));
    syn_dtls_replay_update(&win, 0);
    TEST_ASSERT_TRUE(win.initialized);
    TEST_ASSERT_EQUAL_UINT64(0, win.max_seq);

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 1));
    syn_dtls_replay_update(&win, 1);
    TEST_ASSERT_EQUAL_UINT64(1, win.max_seq);

    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 0));
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 1));

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 5));
    syn_dtls_replay_update(&win, 5);
    TEST_ASSERT_EQUAL_UINT64(5, win.max_seq);

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 3));
    syn_dtls_replay_update(&win, 3);
    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 2));
    syn_dtls_replay_update(&win, 2);
    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 4));
    syn_dtls_replay_update(&win, 4);

    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 2));
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 3));
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 4));
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 5));

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 75));
    syn_dtls_replay_update(&win, 75);
    TEST_ASSERT_EQUAL_UINT64(75, win.max_seq);

    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 5));
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 10));

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 74));
    syn_dtls_replay_update(&win, 74);
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 74));

    TEST_ASSERT_TRUE(syn_dtls_replay_check(&win, 200));
    syn_dtls_replay_update(&win, 200);
    TEST_ASSERT_EQUAL_UINT64(200, win.max_seq);
    TEST_ASSERT_FALSE(syn_dtls_replay_check(&win, 75));
}

void test_dtls_psk_mode_all_cipher_suites(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;

    static const uint8_t psk[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

    uint8_t rx_buf[256];
    size_t rx_len = 0U;
    static const char sample_payload[] = "DTLS 1.3 CoAP Protected Telemetry Datagram";

    SYN_DTLS_CipherSuite suites[] = {
        SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256, SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
        SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384, SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256,
        SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256};

    for (size_t i = 0; i < sizeof(suites) / sizeof(suites[0]); i++) {
        dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
        SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                                 .cipher_suite = suites[i],
                                 .server_name = "sensor.dtls.local",
                                 .psk_identity = (const uint8_t *)"sensor-node-01",
                                 .psk_identity_len = 14,
                                 .psk_secret = psk,
                                 .psk_secret_len = sizeof(psk)};
        SYN_DTLS_Config s_cfg = c_cfg;
        s_cfg.is_server = true;

        SYN_DTLS_Context client, server;
        TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                       g_dc_tx_buf, sizeof(g_dc_tx_buf)));
        TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                       g_ds_tx_buf, sizeof(g_ds_tx_buf)));
        pair_dtls_handshake(&client, &server);

        TEST_ASSERT_TRUE(
            syn_dtls_send(&client, (const uint8_t *)sample_payload, strlen(sample_payload)));
        TEST_ASSERT_GREATER_THAN(0, dw.c2s.len);

        memset(rx_buf, 0, sizeof(rx_buf));
        TEST_ASSERT_TRUE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
        TEST_ASSERT_EQUAL(strlen(sample_payload), rx_len);
        TEST_ASSERT_EQUAL_MEMORY(sample_payload, rx_buf, rx_len);

        /* Server -> Client reverse direction */
        TEST_ASSERT_TRUE(
            syn_dtls_send(&server, (const uint8_t *)sample_payload, strlen(sample_payload)));
        memset(rx_buf, 0, sizeof(rx_buf));
        TEST_ASSERT_TRUE(syn_dtls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
        TEST_ASSERT_EQUAL(strlen(sample_payload), rx_len);
        TEST_ASSERT_EQUAL_MEMORY(sample_payload, rx_buf, rx_len);
    }
}

void test_dtls_seq_16bit_and_anti_replay_rejection(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x42};
    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                             .server_name = "test.local",
                             .psk_identity = (const uint8_t *)"psk",
                             .psk_identity_len = 3,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_DTLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_DTLS_Context client, server;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&client, &server);

    /* 1. 16-bit sequence number (seq > 255) */
    client.client_seq_num = 500ULL;
    static const char msg16[] = "16-bit sequence number test payload";
    TEST_ASSERT_TRUE(syn_dtls_send(&client, (const uint8_t *)msg16, strlen(msg16)));

    uint8_t saved_packet[512];
    size_t saved_len = dw.c2s.len;
    memcpy(saved_packet, dw.c2s.buf, saved_len);

    uint8_t rx_buf[128];
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg16), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg16, rx_buf, rx_len);

    /* 2. Replay the same packet -> rejected by anti-replay window */
    memcpy(dw.c2s.buf, saved_packet, saved_len);
    dw.c2s.len = saved_len;
    TEST_ASSERT_FALSE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));

    /* 3. Tampered MAC tag -> rejected */
    TEST_ASSERT_TRUE(syn_dtls_send(&client, (const uint8_t *)msg16, strlen(msg16)));
    dw.c2s.buf[dw.c2s.len - 1U] ^= 0xFFU;
    TEST_ASSERT_FALSE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));

    /* 4. Corrupted header fixed bit -> rejected */
    TEST_ASSERT_TRUE(syn_dtls_send(&client, (const uint8_t *)msg16, strlen(msg16)));
    dw.c2s.buf[0] &= (uint8_t)~SYN_DTLS_UNIFIED_FIXED_BIT;
    TEST_ASSERT_FALSE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
}

void test_dtls_transport_binding_and_task(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_raw, s_raw;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_raw, &s_raw, false);

    static const uint8_t psk[32] = {0x77};
    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_DTLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_DTLS_Context client, server;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_raw, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_raw, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));

    /* Drive handshake via syn_dtls_task protothread (yields PT_IDLE on CLIENT_HELLO_SENT) */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &client};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dtls_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    TEST_ASSERT_TRUE(syn_dtls_handshake(&server));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dtls_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, client.state);

    SYN_Transport c_dtls_tr, s_dtls_tr;
    syn_dtls_bind_transport(&client, &c_dtls_tr);
    syn_dtls_bind_transport(&server, &s_dtls_tr);

    static const char sample_msg[] = "Protected over SYN_Transport adapter";
    TEST_ASSERT_TRUE(
        syn_transport_send(&c_dtls_tr, (const uint8_t *)sample_msg, strlen(sample_msg)));

    uint8_t rx[128];
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_transport_recv(&s_dtls_tr, rx, sizeof(rx), &rx_len));
    TEST_ASSERT_EQUAL(strlen(sample_msg), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(sample_msg, rx, rx_len);
}

void test_dtls_null_and_bounds_checks(void)
{
    uint8_t buf[256];
    size_t out_len = 0U;

    TEST_ASSERT_FALSE(syn_dtls_replay_check(NULL, 0));
    syn_dtls_replay_update(NULL, 0);

    TEST_ASSERT_FALSE(syn_dtls_init(NULL, NULL, NULL, NULL, 0, NULL, 0));
    TEST_ASSERT_FALSE(syn_dtls_handshake(NULL));
    TEST_ASSERT_FALSE(syn_dtls_send(NULL, buf, 10));
    TEST_ASSERT_FALSE(syn_dtls_recv(NULL, buf, sizeof(buf), &out_len));

    syn_dtls_bind_transport(NULL, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dtls_task(NULL, NULL));
    SYN_PT pt_dummy;
    PT_INIT(&pt_dummy);
    SYN_Task null_task = {.user_data = NULL};
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dtls_task(&pt_dummy, &null_task));

    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x01};
    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_DTLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_DTLS_Context client, server;
    uint8_t c_tx_small[256], c_rx_small[256];
    uint8_t s_tx_small[256], s_rx_small[256];
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, c_rx_small, sizeof(c_rx_small),
                                   c_tx_small, sizeof(c_tx_small)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, s_rx_small, sizeof(s_rx_small),
                                   s_tx_small, sizeof(s_tx_small)));

    /* Auto-handshake on send when ServerHello is not yet available returns false */
    TEST_ASSERT_FALSE(syn_dtls_send(&client, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);
    TEST_ASSERT_TRUE(syn_dtls_handshake(&server));
    /* Now ServerHello is staged -> auto-completes handshake and 0-len send returns true */
    TEST_ASSERT_TRUE(syn_dtls_send(&client, NULL, 0U));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, client.state);
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client)); /* Already ESTABLISHED */

    /* Buffer too small to send */
    uint8_t msg[] = "auto-handshake test";
    client.tx_buf_size = 10U;
    TEST_ASSERT_FALSE(syn_dtls_send(&client, msg, sizeof(msg)));
    client.tx_buf_size = 25U;
    TEST_ASSERT_FALSE(syn_dtls_send(&client, msg, sizeof(msg)));
    client.tx_buf_size = sizeof(c_tx_small);

    /* Auto-handshake on recv when unestablished */
    SYN_DTLS_Context c2, s2;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&c2, &c_cfg, &c_tr, c_rx_small, sizeof(c_rx_small), c_tx_small,
                                   sizeof(c_tx_small)));
    TEST_ASSERT_TRUE(syn_dtls_init(&s2, &s_cfg, &s_tr, s_rx_small, sizeof(s_rx_small), s_tx_small,
                                   sizeof(s_tx_small)));
    uint8_t recv_out[256];
    TEST_ASSERT_FALSE(syn_dtls_recv(&c2, recv_out, sizeof(recv_out), &out_len));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&s2));
    TEST_ASSERT_FALSE(syn_dtls_recv(&c2, recv_out, sizeof(recv_out), &out_len));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, c2.state);

    /* Short rx packet (< 4 bytes) */
    dw.s2c.len = 2U;
    dw.s2c.buf[0] = 0x20U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Invalid fixed bit in header */
    dw.s2c.len = 20U;
    dw.s2c.buf[0] = 0x00U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Truncated header check */
    dw.s2c.len = 5U;
    dw.s2c.buf[0] = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT;
    dw.s2c.buf[1] = 0x01U;
    dw.s2c.buf[2] = 0x00U;
    dw.s2c.buf[3] = 0x20U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Transport recv failure */
    dw.s2c.len = 0U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Handshake failure inside send, recv, and task */
    SYN_DTLS_Context fail_dtls;
    SYN_DTLS_Config bad_rpk = {.mode = SYN_DTLS_AUTH_MODE_RAW_PUBKEY};
    TEST_ASSERT_TRUE(syn_dtls_init(&fail_dtls, &bad_rpk, &c_tr, c_rx_small, sizeof(c_rx_small),
                                   c_tx_small, sizeof(c_tx_small)));
    TEST_ASSERT_FALSE(syn_dtls_send(&fail_dtls, msg, sizeof(msg)));
    fail_dtls.state = SYN_DTLS_STATE_UNINITIALIZED;
    TEST_ASSERT_FALSE(syn_dtls_recv(&fail_dtls, recv_out, sizeof(recv_out), &out_len));
    fail_dtls.state = SYN_DTLS_STATE_UNINITIALIZED;
    SYN_PT pt_fail;
    PT_INIT(&pt_fail);
    SYN_Task task_fail = {.user_data = &fail_dtls};
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dtls_task(&pt_fail, &task_fail));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, fail_dtls.state);

    /* Task waits via PT_WAIT_UNTIL while ESTABLISHED and exits on state change */
    SYN_PT pt_loop;
    PT_INIT(&pt_loop);
    SYN_Task ok_task = {.user_data = &client, .state = SYN_TASK_READY};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dtls_task(&pt_loop, &ok_task));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dtls_task(&pt_loop, &ok_task));
    client.state = SYN_DTLS_STATE_ERROR;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_dtls_task(&pt_loop, &ok_task));
    client.state = SYN_DTLS_STATE_ESTABLISHED;

    /* Large payload (> 2048B) send check */
    static uint8_t large_msg[2049];
    memset(large_msg, 0x55, sizeof(large_msg));
    client.tx_buf = g_dc_tx_buf;
    client.tx_buf_size = sizeof(g_dc_tx_buf);
    TEST_ASSERT_FALSE(syn_dtls_send(&client, large_msg, sizeof(large_msg)));
    client.tx_buf = c_tx_small;
    client.tx_buf_size = sizeof(c_tx_small);

    /* Receive datagram without length bit (has_len = false) encrypted with server_app_key! */
    uint64_t seq_nolen = server.client_seq_num++;
    uint8_t hdr_nolen[2];
    hdr_nolen[0] =
        SYN_DTLS_UNIFIED_FIXED_BIT | ((uint8_t)server.epoch & SYN_DTLS_UNIFIED_EPOCH_MASK);
    hdr_nolen[1] = (uint8_t)(seq_nolen & 0xFFU);

    uint8_t pt_nolen[32];
    static const char nolen_text[] = "no-length-bit";
    size_t nolen_text_len = strlen(nolen_text);
    memcpy(pt_nolen, nolen_text, nolen_text_len);
    pt_nolen[nolen_text_len] = 0x17U;
    pt_nolen[nolen_text_len + 1U] = 0x00U; /* Optional trailing zero padding */

    uint8_t nonce[12];
    memcpy(nonce, server.server_app_iv, 12U);
    uint64_t full_seq = ((uint64_t)server.epoch << 48U) | (seq_nolen & 0xFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((full_seq >> (i * 8)) & 0xFFU);
    }

    dw.s2c.buf[0] = hdr_nolen[0];
    dw.s2c.buf[1] = hdr_nolen[1];
    SYN_AES_GCM_Context gcm;
    syn_aes_gcm_init(&gcm, server.server_app_key, 16U);
    syn_aes_gcm_encrypt(&gcm, nonce, 12U, hdr_nolen, 2U, pt_nolen, nolen_text_len + 2U,
                        &dw.s2c.buf[2], &dw.s2c.buf[2U + nolen_text_len + 2U]);
    dw.s2c.len = 2U + (nolen_text_len + 2U) + 16U;

    TEST_ASSERT_TRUE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));
    TEST_ASSERT_EQUAL(nolen_text_len, out_len);
    TEST_ASSERT_EQUAL_MEMORY(nolen_text, recv_out, out_len);

    /* Datagram with wrong inner content type (0x16) and all-zero inner plaintext */
    uint64_t seq_wrong_ct = server.client_seq_num++;
    hdr_nolen[1] = (uint8_t)(seq_wrong_ct & 0xFFU);
    memcpy(nonce, server.server_app_iv, 12U);
    full_seq = ((uint64_t)server.epoch << 48U) | (seq_wrong_ct & 0xFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) {
        nonce[11 - i] ^= (uint8_t)((full_seq >> (i * 8)) & 0xFFU);
    }
    dw.s2c.buf[0] = hdr_nolen[0];
    dw.s2c.buf[1] = hdr_nolen[1];
    pt_nolen[nolen_text_len] = 0x16U;
    syn_aes_gcm_encrypt(&gcm, nonce, 12U, hdr_nolen, 2U, pt_nolen, nolen_text_len + 1U,
                        &dw.s2c.buf[2], &dw.s2c.buf[2U + nolen_text_len + 1U]);
    dw.s2c.len = 2U + (nolen_text_len + 1U) + 16U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* All-zero inner plaintext (scan == 0) */
    uint8_t zero_inner[2] = {0x00U, 0x00U};
    dw.s2c.buf[0] = hdr_nolen[0];
    dw.s2c.buf[1] = hdr_nolen[1];
    syn_aes_gcm_encrypt(&gcm, nonce, 12U, hdr_nolen, 2U, zero_inner, 2U, &dw.s2c.buf[2],
                        &dw.s2c.buf[4]);
    dw.s2c.len = 2U + 2U + 16U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Length field exceeding rx_len and ct_tag_len < tag_sz + 1 */
    dw.s2c.len = 30U;
    dw.s2c.buf[0] = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT;
    dw.s2c.buf[1] = 0x05U;
    dw.s2c.buf[2] = 0x01U;
    dw.s2c.buf[3] = 0x00U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    dw.s2c.len = 30U;
    dw.s2c.buf[0] = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT;
    dw.s2c.buf[1] = 0x05U;
    dw.s2c.buf[2] = 0x00U;
    dw.s2c.buf[3] = 0x05U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));

    /* Oversized datagram (> 2048B + 1B) rejected */
    client.rx_buf = g_dc_rx_buf;
    client.rx_buf_size = sizeof(g_dc_rx_buf);
    size_t over_ct_tag = (SYN_DTLS_RECORD_MAX_PAYLOAD + 2U) + 16U;
    dw.s2c.len = 4U + over_ct_tag;
    memset(dw.s2c.buf, 0xAA, dw.s2c.len);
    dw.s2c.buf[0] = SYN_DTLS_UNIFIED_FIXED_BIT | SYN_DTLS_UNIFIED_LEN_BIT;
    dw.s2c.buf[1] = 0xFEU;
    dw.s2c.buf[2] = (uint8_t)((over_ct_tag >> 8U) & 0xFFU);
    dw.s2c.buf[3] = (uint8_t)(over_ct_tag & 0xFFU);
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, recv_out, sizeof(recv_out), &out_len));
}

void test_dtls_round4_seq_wrap_cid_epoch_spill_and_auth(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, true);

    uint8_t out_buf[128];
    size_t out_len = 0U;

    /* 1. 48-bit sequence wraparound across 65535 -> 65536 */
    ensure_dtls_rpk_keys();
    SYN_DTLS_Config c_rpk = {.mode = SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                             .peer_pubkey = g_dtls_srv_pub,
                             .peer_pubkey_len = sizeof(g_dtls_srv_pub)};
    SYN_DTLS_Config s_rpk = {.mode = SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                             .is_server = true,
                             .client_privkey = g_dtls_srv_sec};

    SYN_DTLS_Config s_rpk_nokey = {.mode = SYN_DTLS_AUTH_MODE_RAW_PUBKEY, .is_server = true};
    SYN_DTLS_Context s_nokey;
    TEST_ASSERT_TRUE(syn_dtls_init(&s_nokey, &s_rpk_nokey, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&s_nokey));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, s_nokey.state);

    SYN_DTLS_Context client, server;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_rpk, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_rpk, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&client, &server);

    server.client_seq_num = 65534ULL;
    client.replay_window.initialized = true;
    client.replay_window.max_seq = 65533ULL;
    client.replay_window.bitmap = 1ULL;

    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"seq-65534", 9U));
    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));
    TEST_ASSERT_EQUAL_UINT64(65534ULL, client.replay_window.max_seq);

    uint8_t pkt_65535[256], pkt_65536[256];
    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"seq-65535", 9U));
    size_t len_65535 = dw.s2c.len;
    memcpy(pkt_65535, dw.s2c.buf, len_65535);

    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"seq-65536", 9U));
    size_t len_65536 = dw.s2c.len;
    memcpy(pkt_65536, dw.s2c.buf, len_65536);

    memcpy(dw.s2c.buf, pkt_65536, len_65536);
    dw.s2c.len = len_65536;
    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));
    TEST_ASSERT_EQUAL_MEMORY("seq-65536", out_buf, 9U);
    TEST_ASSERT_EQUAL_UINT64(65536ULL, client.replay_window.max_seq);

    memcpy(dw.s2c.buf, pkt_65535, len_65535);
    dw.s2c.len = len_65535;
    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));
    TEST_ASSERT_EQUAL_MEMORY("seq-65535", out_buf, 9U);
    TEST_ASSERT_EQUAL_UINT64(65536ULL, client.replay_window.max_seq);

    memcpy(dw.s2c.buf, pkt_65536, len_65536);
    dw.s2c.len = len_65536;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));

    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"seq-65537", 9U));
    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));
    TEST_ASSERT_EQUAL_UINT64(65537ULL, client.replay_window.max_seq);

    /* 2. CID bit and epoch mismatch rejection */
    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"cid-test", 8U));
    dw.s2c.buf[0] |= SYN_DTLS_UNIFIED_CID_BIT;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));

    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)"epoch-test", 10U));
    dw.s2c.buf[0] = (uint8_t)((dw.s2c.buf[0] & ~SYN_DTLS_UNIFIED_EPOCH_MASK) |
                              (uint8_t)SYN_DTLS_EPOCH_HANDSHAKE);
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));

    /* 3. Single-datagram boundary preservation (no cross-datagram spilling) & has_data */
    SYN_Transport dtls_tr;
    syn_dtls_bind_transport(&client, &dtls_tr);
    TEST_ASSERT_NOT_NULL(dtls_tr.has_data);
    dw.s2c.len = 0U;
    TEST_ASSERT_FALSE(syn_transport_has_data(&dtls_tr));

    static const char dgram_msg[] = "DTLS_SINGLE_DGRAM_PAYLOAD";
    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)dgram_msg, strlen(dgram_msg)));
    TEST_ASSERT_TRUE(syn_transport_has_data(&dtls_tr));

    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, 0U, &out_len));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)out_len);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)dw.s2c.len);
    TEST_ASSERT_FALSE(syn_transport_has_data(&dtls_tr));

    TEST_ASSERT_TRUE(syn_dtls_send(&server, (const uint8_t *)dgram_msg, strlen(dgram_msg)));
    TEST_ASSERT_TRUE(syn_dtls_recv(&client, out_buf, 11U, &out_len));
    TEST_ASSERT_EQUAL_UINT32(11U, (uint32_t)out_len);
    TEST_ASSERT_EQUAL_MEMORY("DTLS_SINGLE", out_buf, 11U);
    /* Remainder of truncated UDP datagram is NOT buffered for a subsequent recv */
    TEST_ASSERT_FALSE(syn_transport_has_data(&dtls_tr));
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, out_buf, sizeof(out_buf), &out_len));

    SYN_Transport null_ctx_tr = {.has_data = dtls_tr.has_data, .ctx = NULL};
    TEST_ASSERT_FALSE(syn_transport_has_data(&null_ctx_tr));

    /* Oversized PSK (> 48B) */
    uint8_t long_psk[64];
    memset(long_psk, 0xABU, sizeof(long_psk));
    SYN_DTLS_Config lp_c = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                            .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256,
                            .psk_secret = long_psk,
                            .psk_secret_len = sizeof(long_psk)};
    SYN_DTLS_Config lp_s = lp_c;
    lp_s.is_server = true;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &lp_c, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &lp_s, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&client, &server);

    /* 4. RPK invalid length & X.509 / mTLS certificate validation + CertificateVerify */
    SYN_DTLS_Config bad_rpk_cfg = {.mode = SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
                                   .peer_pubkey = g_dtls_srv_pub,
                                   .peer_pubkey_len = 16U};
    SYN_DTLS_Context bad_rpk;
    TEST_ASSERT_TRUE(syn_dtls_init(&bad_rpk, &bad_rpk_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&bad_rpk));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, bad_rpk.state);

    uint8_t ca_seed[32] = {0x11U, 0x22U, 0x33U, 0x44U};
    uint8_t ca_pub[32], ca_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(ca_pub, ca_sec, ca_seed));

    uint8_t srv_seed[32] = {0x51U, 0x52U, 0x53U, 0x54U};
    uint8_t srv_pub[32], srv_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(srv_pub, srv_sec, srv_seed));

    uint8_t cli_seed[32] = {0x61U, 0x62U, 0x63U, 0x64U};
    uint8_t cli_pub[32], cli_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(cli_pub, cli_sec, cli_seed));

    uint8_t cert_der[184];
    build_dtls_ed25519_cert_der(srv_pub, ca_sec, ca_pub, cert_der);

    uint8_t cli_cert_der[184];
    build_dtls_ed25519_cert_der(cli_pub, ca_sec, ca_pub, cli_cert_der);

    SYN_X509_Cert root_ca;
    memset(&root_ca, 0, sizeof(root_ca));
    memcpy(root_ca.pubkey, ca_pub, 32U);
    root_ca.pubkey_len = 32U;
    root_ca.pubkey_algo = SYN_X509_ALGO_ED25519;
    root_ca.is_ca = true;
    strcpy(root_ca.subject_cn, "Test CA");

    SYN_DTLS_Config x509_cfg = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                                .server_name = "Test Server",
                                .root_ca = NULL,
                                .peer_cert_der = cert_der,
                                .peer_cert_len = sizeof(cert_der)};
    SYN_DTLS_Context x509_ctx;
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &x509_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    x509_cfg.root_ca = &root_ca;
    x509_cfg.peer_cert_len = 12U;
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &x509_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    x509_cfg.peer_cert_len = sizeof(cert_der);
    x509_cfg.server_name = "Wrong Server";
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &x509_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    /* Server X509_SERVER missing local cert or key & MTLS missing client_cert_der */
    SYN_DTLS_Config srv_no_cert = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER, .is_server = true};
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &srv_no_cert, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    SYN_DTLS_Config srv_no_key = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                                  .is_server = true,
                                  .client_cert_der = cert_der,
                                  .client_cert_len = sizeof(cert_der)};
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &srv_no_key, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    SYN_DTLS_Config mtls_no_local = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                                     .server_name = "Test Server",
                                     .root_ca = &root_ca,
                                     .peer_cert_der = cert_der,
                                     .peer_cert_len = sizeof(cert_der)};
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &mtls_no_local, &c_tr, g_dc_rx_buf,
                                   sizeof(g_dc_rx_buf), g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));

    /* Valid X509_SERVER pair & valid MTLS pair */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_DTLS_Config c_x509 = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                              .server_name = "Test Server",
                              .root_ca = &root_ca,
                              .peer_cert_der = cert_der,
                              .peer_cert_len = sizeof(cert_der)};
    SYN_DTLS_Config s_x509 = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                              .is_server = true,
                              .client_cert_der = cert_der,
                              .client_cert_len = sizeof(cert_der),
                              .client_privkey = srv_sec};
    SYN_DTLS_Context x509_srv;
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &c_x509, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &s_x509, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&x509_ctx, &x509_srv);

    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_DTLS_Config c_mtls = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                              .server_name = "Test Server",
                              .root_ca = &root_ca,
                              .client_cert_der = cli_cert_der,
                              .client_cert_len = sizeof(cli_cert_der),
                              .client_privkey = cli_sec,
                              .peer_cert_der = cert_der,
                              .peer_cert_len = sizeof(cert_der)};
    SYN_DTLS_Config s_mtls = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                              .is_server = true,
                              .root_ca = &root_ca,
                              .client_cert_der = cert_der,
                              .client_cert_len = sizeof(cert_der),
                              .client_privkey = srv_sec,
                              .peer_cert_der = cli_cert_der,
                              .peer_cert_len = sizeof(cli_cert_der)};
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &c_mtls, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &s_mtls, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&x509_ctx, &x509_srv);

    /* ECDSA-P256 X509_SERVER (65B + 64B + 32B bad len + zero scalar) & SHA-384 chain_transcript */
    static const uint8_t p256_ca_priv[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                             0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                             0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                             0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t p256_srv_priv[32] = {0x7D, 0x40, 0xE0, 0x7E, 0x74, 0xF4, 0x9A, 0x71,
                                              0x48, 0xC3, 0x32, 0x10, 0xCA, 0x19, 0xBD, 0x9B,
                                              0x9D, 0x04, 0x50, 0x77, 0x92, 0xEC, 0xF7, 0x3E,
                                              0x87, 0x7E, 0x10, 0x64, 0x79, 0x80, 0x0C, 0x85};
    uint8_t p256_ca_pub65[65], p256_srv_pub65[65], p256_srv_pub64[64];
    p256_ca_pub65[0] = 0x04U;
    p256_srv_pub65[0] = 0x04U;
    TEST_ASSERT_TRUE(syn_p256_base_mul(p256_ca_priv, p256_ca_pub65 + 1U, p256_ca_pub65 + 33U));
    TEST_ASSERT_TRUE(syn_p256_base_mul(p256_srv_priv, p256_srv_pub65 + 1U, p256_srv_pub65 + 33U));
    memcpy(p256_srv_pub64, p256_srv_pub65 + 1U, 64U);

    SYN_X509_Cert p256_ca;
    memset(&p256_ca, 0, sizeof(p256_ca));
    memcpy(p256_ca.pubkey, p256_ca_pub65, 65U);
    p256_ca.pubkey_len = 65U;
    p256_ca.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    p256_ca.is_ca = true;
    strcpy(p256_ca.subject_cn, "Test CA");

    uint8_t p256_c65[256], p256_c64[256], p256_c32[256];
    size_t p256_c65_len = build_dtls_p256_cert_der(p256_srv_pub65, 65U, p256_ca_priv, p256_c65);
    size_t p256_c64_len = build_dtls_p256_cert_der(p256_srv_pub64, 64U, p256_ca_priv, p256_c64);
    size_t p256_c32_len = build_dtls_p256_cert_der(p256_srv_pub64, 32U, p256_ca_priv, p256_c32);

    for (int v = 0; v < 2; v++) {
        const uint8_t *cd = (v == 0) ? p256_c65 : p256_c64;
        size_t cl = (v == 0) ? p256_c65_len : p256_c64_len;
        dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
        SYN_DTLS_Config cp = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                              .cipher_suite = (v == 0) ? SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384
                                                       : SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                              .server_name = "Test Server",
                              .root_ca = &p256_ca,
                              .peer_cert_der = cd,
                              .peer_cert_len = cl};
        SYN_DTLS_Config sp = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                              .cipher_suite = cp.cipher_suite,
                              .is_server = true,
                              .client_cert_der = cd,
                              .client_cert_len = cl,
                              .client_privkey = p256_srv_priv};
        TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &cp, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                       g_dc_tx_buf, sizeof(g_dc_tx_buf)));
        TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &sp, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                       g_ds_tx_buf, sizeof(g_ds_tx_buf)));
        pair_dtls_handshake(&x509_ctx, &x509_srv);
    }

    /* Invalid P-256 pubkey len (32B) silently dropped on client; zero P-256 scalar fails server */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_DTLS_Config cp32 = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                            .server_name = "Test Server",
                            .root_ca = &p256_ca,
                            .peer_cert_der = p256_c32,
                            .peer_cert_len = p256_c32_len};
    SYN_DTLS_Config sp32 = {.mode = SYN_DTLS_AUTH_MODE_X509_SERVER,
                            .is_server = true,
                            .client_cert_der = p256_c32,
                            .client_cert_len = p256_c32_len,
                            .client_privkey = p256_srv_priv};
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &cp32, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &sp32, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, x509_ctx.state);

    uint8_t zero_scalar[32] = {0};
    sp32.client_privkey = zero_scalar;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &cp32, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &sp32, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, x509_srv.state);

    /* Server Flight 2 small tx_buf_size (< 184B) & unknown local SPKI algo */
    uint8_t unk_cert[184];
    memcpy(unk_cert, cert_der, sizeof(cert_der));
    unk_cert[74] = 0x71U;
    TEST_ASSERT_TRUE(syn_ed25519_sign(unk_cert + 3U, 107U, ca_sec, ca_pub, unk_cert + 120U));

    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    /* DTLS-1: Non-PSK modes require >= 184B (SHA-256) or >= 200B (SHA-384) at syn_dtls_init */
    TEST_ASSERT_FALSE(syn_dtls_init(&x509_ctx, &c_x509, &c_tr, g_dc_rx_buf, 183U, g_dc_tx_buf,
                                    sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_init(&x509_srv, &s_x509, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                    g_ds_tx_buf, 183U));
    SYN_DTLS_Config c_x509_384 = c_x509;
    c_x509_384.cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384;
    TEST_ASSERT_FALSE(syn_dtls_init(&x509_ctx, &c_x509_384, &c_tr, g_dc_rx_buf, 199U, g_dc_tx_buf,
                                    sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_init(&x509_ctx, &c_x509_384, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                    g_dc_tx_buf, 199U));

    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &c_x509, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &s_x509, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    x509_srv.tx_buf_size = 140U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, x509_srv.state);

    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_DTLS_Config sp_unk = s_x509;
    sp_unk.client_cert_der = unk_cert;
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &c_x509, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &sp_unk, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, x509_srv.state);

    /* mTLS Flight 2 & Flight 3 tamper/discard + client Flight 3 local errors */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_DTLS_Config cm_unk = c_mtls;
    cm_unk.client_cert_der = unk_cert;
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &cm_unk, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &s_mtls, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, x509_ctx.state);

    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_ctx, &c_mtls, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&x509_srv, &s_mtls, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));

    uint8_t saved_f2[256];
    size_t saved_f2_len = dw.s2c.len;
    memcpy(saved_f2, dw.s2c.buf, saved_f2_len);

    /* Forged server CV header, sig len, scheme, and sig are silently dropped on client */
    size_t cv_offs[4] = {80U, 87U, 84U, 88U};
    for (size_t k = 0U; k < 4U; k++) {
        memcpy(dw.s2c.buf, saved_f2, saved_f2_len);
        dw.s2c.buf[cv_offs[k]] ^= 0xFFU;
        dw.s2c.len = saved_f2_len;
        TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
        TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, x509_ctx.state);
    }

    /* Client Flight 3 send failure fails closed */
    memcpy(dw.s2c.buf, saved_f2, saved_f2_len);
    dw.s2c.len = saved_f2_len;
    dw.c2s.fail_send = true;
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, x509_ctx.state);
    dw.c2s.fail_send = false;

    /* Valid Flight 2 -> Flight 3, then inject forged UDP packets into FINISHED_SENT server */
    x509_ctx.state = SYN_DTLS_STATE_CLIENT_HELLO_SENT;
    memcpy(dw.s2c.buf, saved_f2, saved_f2_len);
    dw.s2c.len = saved_f2_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, x509_ctx.state);

    uint8_t saved_f3[160];
    size_t saved_f3_len = dw.c2s.len;
    memcpy(saved_f3, dw.c2s.buf, saved_f3_len);

    /* Short packet, forged client CV, and tampered ClientFinished MAC are silently dropped! */
    dw.c2s.len = 12U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_FINISHED_SENT, x509_srv.state);

    memcpy(dw.c2s.buf, saved_f3, saved_f3_len);
    dw.c2s.buf[25] ^= 0xFFU;
    dw.c2s.len = saved_f3_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_FINISHED_SENT, x509_srv.state);

    memcpy(dw.c2s.buf, saved_f3, saved_f3_len);
    dw.c2s.buf[saved_f3_len - 1U] ^= 0xFFU;
    dw.c2s.len = saved_f3_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_FINISHED_SENT, x509_srv.state);

    /* Authentic Flight 3 still succeeds because server transcript was not poisoned! */
    memcpy(dw.c2s.buf, saved_f3, saved_f3_len);
    dw.c2s.len = saved_f3_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&x509_srv));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, x509_srv.state);

    /* Fail closed on CSPRNG failure in init and client handshake */
    x509_ctx.state = SYN_DTLS_STATE_UNINITIALIZED;
    mock_random_skip = true;
    TEST_ASSERT_FALSE(syn_dtls_handshake(&x509_ctx));
    TEST_ASSERT_FALSE(syn_dtls_init(&x509_ctx, &c_mtls, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                    g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    mock_random_skip = false;
}

void test_r5_n05_dtls_handshake_and_reflection_rejection(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x33, 0x44, 0x55, 0x66};
    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                             .cipher_suite = (SYN_DTLS_CipherSuite)99, /* default wire_id branch */
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_DTLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_DTLS_Context client, server;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&client, &server);

    /* 1. CRITICAL: Replaying client's own encrypted datagram back into client recv MUST fail! */
    static const char reflect_msg[] = "DTLS client self-reflection test";
    TEST_ASSERT_TRUE(syn_dtls_send(&client, (const uint8_t *)reflect_msg, strlen(reflect_msg)));
    memcpy(dw.s2c.buf, dw.c2s.buf, dw.c2s.len);
    dw.s2c.len = dw.c2s.len;
    dw.c2s.len = 0U;

    uint8_t rx_buf[64];
    size_t rx_len = 0U;
    TEST_ASSERT_FALSE(syn_dtls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));

    /* 2. Handshake failure paths: send fail, bad ClientHello, server RNG fail, zero X25519, bad
     * ServerHello, tampered Finished MAC */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    dw.c2s.fail_send = true;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, client.state);

    /* Malformed ClientHello on UDP server is silently dropped per RFC 9147 §4.5.1 */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    dw.c2s.len = 16U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_UNINITIALIZED, server.state);

    /* Valid ClientHello -> test server RNG fail, zero X25519 key_share, and server send fail */
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client));
    uint8_t saved_ch[80];
    memcpy(saved_ch, dw.c2s.buf, 80U);

    mock_random_skip = true;
    TEST_ASSERT_FALSE(syn_dtls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, server.state);
    mock_random_skip = false;

    /* All-zero X25519 ClientHello key_share is silently dropped without poisoning server state */
    server.state = SYN_DTLS_STATE_UNINITIALIZED;
    memcpy(dw.c2s.buf, saved_ch, 80U);
    memset(dw.c2s.buf + 48U, 0, 32U);
    dw.c2s.len = 80U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_UNINITIALIZED, server.state);

    /* Server transport send fail */
    memcpy(dw.c2s.buf, saved_ch, 80U);
    dw.c2s.len = 80U;
    dw.s2c.fail_send = true;
    TEST_ASSERT_FALSE(syn_dtls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, server.state);
    dw.s2c.fail_send = false;

    /* Valid ServerHello -> forged UDP packets dropped cleanly, then valid flight succeeds! */
    server.state = SYN_DTLS_STATE_UNINITIALIZED;
    memcpy(dw.c2s.buf, saved_ch, 80U);
    dw.c2s.len = 80U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&server));

    uint8_t saved_sh[140];
    size_t saved_sh_len = dw.s2c.len;
    memcpy(saved_sh, dw.s2c.buf, saved_sh_len);

    dw.s2c.buf[saved_sh_len - 1U] ^= 0xFFU;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    memcpy(dw.s2c.buf, saved_sh, saved_sh_len);
    memset(dw.s2c.buf + 48U, 0, 32U);
    dw.s2c.len = saved_sh_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    dw.s2c.len = 16U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    memcpy(dw.s2c.buf, saved_sh, saved_sh_len);
    dw.s2c.len = saved_sh_len;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, client.state);
}

static void test_tier_a_dtls_psk_seq_and_secret_scrub(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    /* DTLS-3: NULL or 0-length PSK must fail closed */
    static const uint8_t psk[32] = {0x5A, 0x6B, 0x7C, 0x8D};
    static const uint8_t zeros[SYN_DTLS_SECRET_LEN] = {0};
    SYN_DTLS_Context client, server;

    SYN_DTLS_Config null_psk = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                                .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                                .psk_secret = NULL,
                                .psk_secret_len = 16U};
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &null_psk, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, client.state);

    SYN_DTLS_Config zero_len_psk = null_psk;
    zero_len_psk.psk_secret = psk;
    zero_len_psk.psk_secret_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &zero_len_psk, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_FALSE(syn_dtls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, client.state);

    /* Valid handshake -> DTLS-5 secret scrub + DTLS-2 TX backpressure & 48-bit seq limit */
    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_DTLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_cfg, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_cfg, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));
    pair_dtls_handshake(&client, &server);

    TEST_ASSERT_EQUAL_MEMORY(zeros, client.ecdhe_priv, sizeof(client.ecdhe_priv));
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.client_handshake_secret, SYN_DTLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.server_handshake_secret, SYN_DTLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.master_secret, SYN_DTLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, server.ecdhe_priv, sizeof(server.ecdhe_priv));
    TEST_ASSERT_EQUAL_MEMORY(zeros, server.master_secret, SYN_DTLS_SECRET_LEN);

    /* 200 failed sends (> 128 half-window) must NOT burn sequence numbers */
    static const char msg[] = "DTLS backpressure resilience";
    dw.c2s.fail_send = true;
    for (size_t i = 0U; i < 200U; i++) {
        TEST_ASSERT_FALSE(syn_dtls_send(&client, (const uint8_t *)msg, strlen(msg)));
    }
    TEST_ASSERT_EQUAL_UINT64(0ULL, client.client_seq_num);

    dw.c2s.fail_send = false;
    TEST_ASSERT_TRUE(syn_dtls_send(&client, (const uint8_t *)msg, strlen(msg)));
    TEST_ASSERT_EQUAL_UINT64(1ULL, client.client_seq_num);

    uint8_t rx_buf[64] = {0};
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx_buf, rx_len);

    /* RFC 9147 §4.5.3: 48-bit sequence exhaustion (> 0xFFFFFFFFFFFFULL) fails closed */
    client.client_seq_num = 0x1000000000000ULL;
    TEST_ASSERT_FALSE(syn_dtls_send(&client, (const uint8_t *)msg, strlen(msg)));
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ERROR, client.state);
}

static SYN_PT_Status dtls_peer_more_task(SYN_PT *pt, SYN_Task *task)
{
    uint32_t *runs = (uint32_t *)task->user_data;
    (*runs)++;
    PT_BEGIN(pt);
    for (;;) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_dtls_cooperative_staged_handshake_and_forged_recovery(void)
{
    DuplexDatagramWire dw;
    DtlsEndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    dtls_duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, true);

    uint8_t ca_seed[32] = {0x55, 0x66, 0x77, 0x88};
    uint8_t ca_pub[32], ca_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(ca_pub, ca_sec, ca_seed));

    uint8_t srv_seed[32] = {0x12, 0x34, 0x56, 0x78};
    uint8_t srv_pub[32], srv_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(srv_pub, srv_sec, srv_seed));

    uint8_t cli_seed[32] = {0xA1, 0xB2, 0xC3, 0xD4};
    uint8_t cli_pub[32], cli_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(cli_pub, cli_sec, cli_seed));

    uint8_t srv_cert_der[184], cli_cert_der[184];
    build_dtls_ed25519_cert_der(srv_pub, ca_sec, ca_pub, srv_cert_der);
    build_dtls_ed25519_cert_der(cli_pub, ca_sec, ca_pub, cli_cert_der);

    SYN_X509_Cert root_ca;
    memset(&root_ca, 0, sizeof(root_ca));
    memcpy(root_ca.pubkey, ca_pub, 32U);
    root_ca.pubkey_len = 32U;
    root_ca.pubkey_algo = SYN_X509_ALGO_ED25519;
    root_ca.is_ca = true;
    strcpy(root_ca.subject_cn, "Test CA");

    SYN_DTLS_Config c_mtls = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                              .server_name = "Test Server",
                              .root_ca = &root_ca,
                              .client_cert_der = cli_cert_der,
                              .client_cert_len = sizeof(cli_cert_der),
                              .client_privkey = cli_sec,
                              .peer_cert_der = srv_cert_der,
                              .peer_cert_len = sizeof(srv_cert_der)};
    SYN_DTLS_Config s_mtls = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                              .is_server = true,
                              .root_ca = &root_ca,
                              .client_cert_der = srv_cert_der,
                              .client_cert_len = sizeof(srv_cert_der),
                              .client_privkey = srv_sec,
                              .peer_cert_der = cli_cert_der,
                              .peer_cert_len = sizeof(cli_cert_der)};

    SYN_DTLS_Context client, server;
    TEST_ASSERT_TRUE(syn_dtls_init(&client, &c_mtls, &c_tr, g_dc_rx_buf, sizeof(g_dc_rx_buf),
                                   g_dc_tx_buf, sizeof(g_dc_tx_buf)));
    TEST_ASSERT_TRUE(syn_dtls_init(&server, &s_mtls, &s_tr, g_ds_rx_buf, sizeof(g_ds_rx_buf),
                                   g_ds_tx_buf, sizeof(g_ds_tx_buf)));

    uint32_t peer_runs = 0U;
    SYN_Task tasks[3];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "dtls_c", syn_dtls_task, SYN_PRIO_HIGH, &client);
    syn_task_create(&tasks[1], "dtls_s", syn_dtls_task, SYN_PRIO_HIGH, &server);
    syn_task_create(&tasks[2], "peer", dtls_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 3U);

    /* Turn 1 (dtls_c): Stage 1 verifies server X.509 cert + sends ClientHello -> yields PT_MORE */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);
    TEST_ASSERT_EQUAL_size_t(80U, dw.c2s.len);

    /* Turn 2 (dtls_s): Stage 1 verifies client X.509 cert + parses CH + derives HS keys -> PT_MORE
     */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED, server.state);

    /* Turn 3 (peer): peer runs */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, peer_runs);

    /* Turn 4 (dtls_c): ServerHello not sent yet -> yields PT_IDLE via PT_PASS */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    /* Turn 5 (dtls_s): Stage 2 signs Server CV + sends Flight 2 -> FINISHED_SENT, yields PT_MORE */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_FINISHED_SENT, server.state);
    TEST_ASSERT_TRUE(dw.s2c.len > 0U);

    uint8_t valid_f2[256];
    size_t valid_f2_len = dw.s2c.len;
    memcpy(valid_f2, dw.s2c.buf, valid_f2_len);

    /* Corrupt Server CV signature in Flight 2 to test mid-stage forged UDP recovery! */
    dw.s2c.buf[90] ^= 0xFFU;

    syn_sched_run(&sched); /* peer (2) */
    syn_sched_run(&sched); /* dtls_s polls empty wire -> PT_IDLE */

    mock_tick_advance(10U);
    syn_sched_run(&sched); /* peer (overdue) */
    syn_sched_run(&sched); /* dtls_c Stage 2 on forged packet -> HANDSHAKE_KEYS_DERIVED! */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED, client.state);
    syn_sched_run(&sched); /* dtls_s (overdue) -> PT_IDLE */
    syn_sched_run(&sched); /* peer */
    syn_sched_run(&sched); /* dtls_c Stage 3 detects forged CV -> resets to CLIENT_HELLO_SENT! */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CLIENT_HELLO_SENT, client.state);

    /* Now deliver authentic Server Flight 2 and step dtls_c through Stages 2 -> 3 -> 4 -> 5 */
    memcpy(dw.s2c.buf, valid_f2, valid_f2_len);
    dw.s2c.len = valid_f2_len;

    mock_tick_advance(10U);
    syn_sched_run(&sched); /* dtls_s (overdue, rr=1) -> PT_IDLE */
    syn_sched_run(&sched); /* peer (overdue, rr=2) -> PT_MORE */
    syn_sched_run(&sched); /* dtls_c Stage 2 (overdue, rr=0): recv SH + X25519 -> KEYS_DERIVED */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED, client.state);

    syn_sched_run(&sched); /* peer */
    syn_sched_run(&sched); /* dtls_c Stage 3: verify Server CV -> CERTIFICATE_VERIFIED */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_CERTIFICATE_VERIFIED, client.state);

    syn_sched_run(&sched); /* peer */
    syn_sched_run(&sched); /* dtls_c Stage 4: verify Server Finished -> SERVER_HELLO_RECEIVED */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_SERVER_HELLO_RECEIVED, client.state);

    syn_sched_run(&sched); /* peer */
    syn_sched_run(&sched); /* dtls_c Stage 5: sign Client CV + send Flight 3 -> ESTABLISHED */
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, client.state);
    TEST_ASSERT_TRUE(dw.c2s.len > 0U);

    mock_tick_advance(10U);
    while (server.state != SYN_DTLS_STATE_ESTABLISHED) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, server.state);
}

void run_dtls_tests(void)
{
    RUN_TEST(test_dtls_sliding_window_anti_replay);
    RUN_TEST(test_dtls_psk_mode_all_cipher_suites);
    RUN_TEST(test_dtls_seq_16bit_and_anti_replay_rejection);
    RUN_TEST(test_dtls_transport_binding_and_task);
    RUN_TEST(test_dtls_null_and_bounds_checks);
    RUN_TEST(test_dtls_round4_seq_wrap_cid_epoch_spill_and_auth);
    RUN_TEST(test_r5_n05_dtls_handshake_and_reflection_rejection);
    RUN_TEST(test_tier_a_dtls_psk_seq_and_secret_scrub);
    RUN_TEST(test_dtls_cooperative_staged_handshake_and_forged_recovery);
}
