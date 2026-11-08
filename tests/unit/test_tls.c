/**
 * @file test_tls.c
 * @brief Unit tests for native zero-dependency TLS 1.3 engine and transport binding.
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/net/syn_tls.h"
#include "unity/unity.h"

#include <string.h>

typedef struct {
    uint8_t buf[4096];
    size_t len;
    bool fail_send;
} LoopbackTransport;

typedef struct {
    LoopbackTransport c2s;
    LoopbackTransport s2c;
} DuplexWire;

typedef struct {
    LoopbackTransport *tx;
    LoopbackTransport *rx;
} EndpointIo;

static uint8_t g_c_rx_buf[2560];
static uint8_t g_c_tx_buf[2560];
static uint8_t g_s_rx_buf[2560];
static uint8_t g_s_tx_buf[2560];

static bool ep_send(const uint8_t *data, size_t len, void *ctx)
{
    EndpointIo *io = (EndpointIo *)ctx;
    LoopbackTransport *lb = io->tx;
    if (lb->fail_send || lb->len + len > sizeof(lb->buf)) {
        return false;
    }
    memcpy(lb->buf + lb->len, data, len);
    lb->len += len;
    return true;
}

static bool ep_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    EndpointIo *io = (EndpointIo *)ctx;
    LoopbackTransport *lb = io->rx;
    if (lb->len == 0U) {
        return false;
    }
    size_t copy_len = (lb->len > max_len) ? max_len : lb->len;
    memcpy(data, lb->buf, copy_len);
    *out_len = copy_len;
    lb->len = 0U;
    return true;
}

static bool ep_has_data(const void *ctx)
{
    const EndpointIo *io = (const EndpointIo *)ctx;
    return (io != NULL && io->rx != NULL && io->rx->len > 0U);
}

static void duplex_init(DuplexWire *dw, EndpointIo *c_io, EndpointIo *s_io, SYN_Transport *c_tr,
                        SYN_Transport *s_tr, bool with_has_data)
{
    memset(dw, 0, sizeof(*dw));
    c_io->tx = &dw->c2s;
    c_io->rx = &dw->s2c;
    s_io->tx = &dw->s2c;
    s_io->rx = &dw->c2s;
    c_tr->send = ep_send;
    c_tr->recv = ep_recv;
    c_tr->has_data = with_has_data ? ep_has_data : NULL;
    c_tr->ctx = c_io;
    s_tr->send = ep_send;
    s_tr->recv = ep_recv;
    s_tr->has_data = with_has_data ? ep_has_data : NULL;
    s_tr->ctx = s_io;
}

static const uint8_t g_test_srv_seed[32] = {0x10U, 0x20U, 0x30U, 0x40U, 0x50U, 0x60U, 0x70U, 0x80U};
static uint8_t g_test_srv_pub[32];
static uint8_t g_test_srv_sec[32];
static bool g_test_keys_ready = false;

static void ensure_test_rpk_keys(void)
{
    if (!g_test_keys_ready) {
        TEST_ASSERT_TRUE(
            syn_ed25519_create_keypair(g_test_srv_pub, g_test_srv_sec, g_test_srv_seed));
        g_test_keys_ready = true;
    }
}

static void build_ed25519_cert_der(const uint8_t leaf_pub[32], const uint8_t ca_sec[32],
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

static size_t build_p256_cert_der(const uint8_t *leaf_pub, size_t leaf_pub_len,
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

static void pair_tls_handshake(SYN_TLS_Context *client, SYN_TLS_Context *server)
{
    /* Server poll before ClientHello arrives stays waiting cleanly */
    TEST_ASSERT_TRUE(syn_tls_handshake(server));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_UNINITIALIZED, server->state);

    /* Flight 1: Client sends ClientHello and waits for ServerHello */
    TEST_ASSERT_TRUE(syn_tls_handshake(client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_CLIENT_HELLO_SENT, client->state);

    /* Flight 2: Server processes ClientHello and sends ServerHello [+ CV] + Finished */
    TEST_ASSERT_TRUE(syn_tls_handshake(server));
    if (server->config.mode == SYN_TLS_AUTH_MODE_MTLS) {
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_FINISHED_SENT, server->state);
    } else {
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, server->state);
    }

    /* Client processes ServerHello [+ CV] + Finished (and sends Flight 3 in mTLS) */
    TEST_ASSERT_TRUE(syn_tls_handshake(client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, client->state);

    if (server->config.mode == SYN_TLS_AUTH_MODE_MTLS) {
        TEST_ASSERT_TRUE(syn_tls_handshake(server));
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, server->state);
    }
}

void test_tls_psk_mode_handshake_and_record_crypto(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .server_name = "device.local",
                            .psk_identity = (const uint8_t *)"device_01",
                            .psk_identity_len = 9,
                            .psk_secret = psk,
                            .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));

    pair_tls_handshake(&client, &server);
    TEST_ASSERT_TRUE(syn_tls_is_established(&client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, syn_tls_get_state(&client));

    /* Client -> Server encrypted record */
    static const char msg[] = "Hello Zero-Dependency TLS 1.3!";
    TEST_ASSERT_TRUE(syn_tls_send(&client, (const uint8_t *)msg, strlen(msg)));

    uint8_t rx_buf[128];
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_tls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx_buf, rx_len);

    /* Server -> Client encrypted record */
    static const char reply[] = "Server ACK over distinct traffic keys";
    TEST_ASSERT_TRUE(syn_tls_send(&server, (const uint8_t *)reply, strlen(reply)));
    TEST_ASSERT_TRUE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(reply), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(reply, rx_buf, rx_len);
}

void test_tls_transport_binding_interface(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_raw, s_raw;
    duplex_init(&dw, &c_io, &s_io, &c_raw, &s_raw, false);

    ensure_test_rpk_keys();
    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                            .peer_pubkey = g_test_srv_pub,
                            .peer_pubkey_len = sizeof(g_test_srv_pub)};
    SYN_TLS_Config s_cfg = {
        .mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY, .is_server = true, .client_privkey = g_test_srv_sec};

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_raw, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_raw, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&client, &server);

    SYN_Transport c_tls_tr, s_tls_tr;
    syn_tls_bind_transport(&client, &c_tls_tr);
    syn_tls_bind_transport(&server, &s_tls_tr);

    static const char payload[] = "Encapsulated via SYN_Transport abstraction";
    TEST_ASSERT_TRUE(syn_transport_send(&c_tls_tr, (const uint8_t *)payload, strlen(payload)));

    uint8_t rx_buf[128];
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_transport_recv(&s_tls_tr, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(payload), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, rx_buf, rx_len);
}

void test_tls_task_protothread(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, true);

    static const uint8_t pt_psk[32] = {0x31U, 0x32U, 0x33U, 0x34U};
    SYN_TLS_Config c_cfg = {
        .mode = SYN_TLS_AUTH_MODE_PSK, .psk_secret = pt_psk, .psk_secret_len = sizeof(pt_psk)};
    SYN_TLS_Config s_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .is_server = true,
                            .psk_secret = pt_psk,
                            .psk_secret_len = sizeof(pt_psk)};

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));

    SYN_Task task = {.user_data = &client};
    SYN_PT pt;
    PT_INIT(&pt);

    /* Step 1: Client sends ClientHello and yields PT_IDLE via PT_PASS while awaiting ServerHello */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_tls_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_CLIENT_HELLO_SENT, client.state);

    /* Server answers ClientHello */
    TEST_ASSERT_TRUE(syn_tls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, server.state);

    /* Step 2: Client completes handshake and waits via PT_WAIT_UNTIL while ESTABLISHED */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_tls_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, client.state);

    /* Spurious resume while still ESTABLISHED yields PT_IDLE without deadlocking BLOCKED */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_tls_task(&pt, &task));

    /* Transitioning away from ESTABLISHED completes with PT_ENDED */
    client.state = SYN_TLS_STATE_ERROR;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_tls_task(&pt, &task));
}

void test_tls_null_and_bounds_checks(void)
{
    uint8_t rx_small[64];
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    SYN_TLS_Config config = {.mode = SYN_TLS_AUTH_MODE_PSK};
    SYN_TLS_Context tls;

    TEST_ASSERT_FALSE(syn_tls_init(NULL, NULL, NULL, NULL, 0, NULL, 0));
    TEST_ASSERT_FALSE(syn_tls_init(&tls, &config, &c_tr, rx_small, sizeof(rx_small), g_c_tx_buf,
                                   sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_init(&tls, &config, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), rx_small,
                                   sizeof(rx_small)));
    TEST_ASSERT_FALSE(syn_tls_handshake(NULL));
    TEST_ASSERT_FALSE(syn_tls_send(NULL, NULL, 0));
    TEST_ASSERT_FALSE(syn_tls_recv(NULL, NULL, 0, NULL));

    syn_tls_bind_transport(NULL, NULL);

    /* TLS-3: PSK mode with NULL or 0-length psk_secret MUST fail closed */
    SYN_TLS_Config null_psk_cfg = {
        .mode = SYN_TLS_AUTH_MODE_PSK, .psk_secret = NULL, .psk_secret_len = 16U};
    TEST_ASSERT_TRUE(syn_tls_init(&tls, &null_psk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, tls.state);

    uint8_t long_psk[64];
    memset(long_psk, 0xAA, sizeof(long_psk));
    SYN_TLS_Config zero_len_psk_cfg = {
        .mode = SYN_TLS_AUTH_MODE_PSK, .psk_secret = long_psk, .psk_secret_len = 0U};
    TEST_ASSERT_TRUE(syn_tls_init(&tls, &zero_len_psk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, tls.state);
    SYN_TLS_Config psk_cfg = {
        .mode = SYN_TLS_AUTH_MODE_PSK, .psk_secret = long_psk, .psk_secret_len = sizeof(long_psk)};
    SYN_TLS_Config srv_psk_cfg = psk_cfg;
    srv_psk_cfg.is_server = true;

    SYN_TLS_Context psk_tls, srv_tls;
    TEST_ASSERT_TRUE(syn_tls_init(&psk_tls, &psk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&srv_tls, &srv_psk_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&psk_tls, &srv_tls);
    TEST_ASSERT_TRUE(syn_tls_handshake(&psk_tls)); /* Repeat call when established */

    /* Zero length send & oversized send */
    TEST_ASSERT_TRUE(syn_tls_send(&psk_tls, NULL, 0));
    TEST_ASSERT_FALSE(syn_tls_send(&psk_tls, NULL, 10000));

    /* Recv errors */
    uint8_t rx_buf[64];
    size_t rx_len = 0U;
    TEST_ASSERT_FALSE(syn_tls_recv(NULL, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_FALSE(syn_tls_recv(&psk_tls, NULL, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_FALSE(syn_tls_recv(&psk_tls, rx_buf, sizeof(rx_buf), NULL));
    TEST_ASSERT_FALSE(syn_tls_recv(&psk_tls, rx_buf, sizeof(rx_buf), &rx_len)); /* Empty wire */

    /* Decrypt record spill into app_rx_buf test (Server -> Client) */
    TEST_ASSERT_TRUE(syn_tls_send(&srv_tls, (const uint8_t *)"SPILL_TEST_DATA", 15U));
    TEST_ASSERT_TRUE(syn_tls_recv(&psk_tls, rx_buf, 5U, &rx_len));
    TEST_ASSERT_EQUAL(5U, rx_len);
    TEST_ASSERT_EQUAL_MEMORY("SPILL", rx_buf, 5U);
    TEST_ASSERT_TRUE(syn_tls_recv(&psk_tls, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(10U, rx_len);
    TEST_ASSERT_EQUAL_MEMORY("_TEST_DATA", rx_buf, 10U);

    /* App data ring buffer drain test */
    SYN_TLS_Context ring_tls;
    memset(&ring_tls, 0, sizeof(ring_tls));
    ring_tls.state = SYN_TLS_STATE_ESTABLISHED;
    ring_tls.app_rx_buf[0] = 'X';
    ring_tls.app_rx_buf[1] = 'Y';
    ring_tls.app_rx_head = 0U;
    ring_tls.app_rx_tail = 2U;
    size_t ring_rx_len = 0U;
    TEST_ASSERT_TRUE(syn_tls_recv(&ring_tls, rx_buf, 1U, &ring_rx_len));
    TEST_ASSERT_EQUAL(1U, ring_rx_len);
    TEST_ASSERT_EQUAL('X', rx_buf[0]);
    TEST_ASSERT_TRUE(syn_tls_recv(&ring_tls, rx_buf, sizeof(rx_buf), &ring_rx_len));
    TEST_ASSERT_EQUAL(1U, ring_rx_len);
    TEST_ASSERT_EQUAL('Y', rx_buf[0]);

    /* Test tls_tr has_data hook: NULL when underlying transport lacks has_data */
    SYN_Transport bound_tr;
    syn_tls_bind_transport(&psk_tls, &bound_tr);
    TEST_ASSERT_NULL(bound_tr.has_data);
    TEST_ASSERT_TRUE(syn_transport_has_data(&bound_tr));

    /* When underlying transport provides has_data, bound_tr wires tls_transport_has_data_cb */
    SYN_Transport c_tr_hd, s_tr_hd;
    duplex_init(&dw, &c_io, &s_io, &c_tr_hd, &s_tr_hd, true);
    SYN_TLS_Context hd_tls, hd_srv;
    TEST_ASSERT_TRUE(syn_tls_init(&hd_tls, &psk_cfg, &c_tr_hd, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&hd_srv, &srv_psk_cfg, &s_tr_hd, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&hd_tls, &hd_srv);

    SYN_Transport bound_hd_tr;
    syn_tls_bind_transport(&hd_tls, &bound_hd_tr);
    TEST_ASSERT_NOT_NULL(bound_hd_tr.has_data);
    dw.s2c.len = 0U;
    TEST_ASSERT_FALSE(syn_transport_has_data(&bound_hd_tr));
    hd_tls.app_rx_head = 0U;
    hd_tls.app_rx_tail = 5U;
    TEST_ASSERT_TRUE(syn_transport_has_data(&bound_hd_tr));
    hd_tls.app_rx_tail = 0U;
    dw.s2c.len = 4U;
    TEST_ASSERT_TRUE(syn_transport_has_data(&bound_hd_tr));
    dw.s2c.len = 0U;

    /* Oversized send buffer checks & subtraction-form wrap guard */
    uint8_t dummy_byte = 0x42U;
    TEST_ASSERT_FALSE(syn_tls_send(&psk_tls, &dummy_byte, psk_tls.tx_buf_size));
    TEST_ASSERT_FALSE(syn_tls_send(&psk_tls, &dummy_byte, SIZE_MAX));
    psk_tls.tx_buf_size = 4U;
    TEST_ASSERT_FALSE(syn_tls_send(&psk_tls, &dummy_byte, 1U));
    psk_tls.tx_buf_size = sizeof(g_c_tx_buf);

    /* Auto-handshake on send/recv when ServerHello is already staged, and incomplete handshake */
    SYN_TLS_Context auto_c, auto_s;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&auto_c, &psk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&auto_s, &srv_psk_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    /* Send before ServerHello arrives sends ClientHello and returns false (not yet established) */
    TEST_ASSERT_FALSE(syn_tls_send(&auto_c, (const uint8_t *)"wait", 4U));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_CLIENT_HELLO_SENT, auto_c.state);
    /* Server replies with ServerHello + Finished; now auto_c send auto-completes handshake! */
    TEST_ASSERT_TRUE(syn_tls_handshake(&auto_s));
    TEST_ASSERT_TRUE(syn_tls_send(&auto_c, (const uint8_t *)"sent", 4U));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, auto_c.state);

    /* Same for auto-handshake inside syn_tls_recv */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&auto_c, &psk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&auto_s, &srv_psk_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_recv(&auto_c, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_CLIENT_HELLO_SENT, auto_c.state);
    TEST_ASSERT_TRUE(syn_tls_handshake(&auto_s));
    /* Auto-completes handshake on recv, then wire is empty so recv returns false */
    TEST_ASSERT_FALSE(syn_tls_recv(&auto_c, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, auto_c.state);

    /* Short corrupt wire record transitions to fatal SYN_TLS_STATE_ERROR */
    dw.s2c.len = 5U;
    memset(dw.s2c.buf, 0x17, 5U);
    TEST_ASSERT_FALSE(syn_tls_recv(&auto_c, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, auto_c.state);
    TEST_ASSERT_FALSE(syn_tls_send(&auto_c, &dummy_byte, 1U));
    TEST_ASSERT_FALSE(syn_tls_recv(&auto_c, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_FALSE(syn_tls_handshake(&auto_c));

    /* Task error handling & null checks */
    SYN_Task err_task = {.user_data = &auto_c};
    SYN_PT pt_err;
    PT_INIT(&pt_err);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_tls_task(&pt_err, &err_task));

    SYN_Task null_task = {.user_data = NULL};
    SYN_PT pt_null;
    PT_INIT(&pt_null);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_tls_task(NULL, &null_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_tls_task(&pt_null, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_tls_task(&pt_null, &null_task));

    TEST_ASSERT_FALSE(syn_tls_is_established(NULL));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_UNINITIALIZED, syn_tls_get_state(NULL));

    SYN_Transport null_ctx_tr = {.has_data = bound_hd_tr.has_data, .ctx = NULL};
    TEST_ASSERT_FALSE(syn_transport_has_data(&null_ctx_tr));

    SYN_TLS_Context no_tr_tls;
    memset(&no_tr_tls, 0, sizeof(no_tr_tls));
    SYN_Transport no_tr_bound;
    syn_tls_bind_transport(&no_tr_tls, &no_tr_bound);
    TEST_ASSERT_NULL(no_tr_bound.has_data);
    TEST_ASSERT_TRUE(syn_transport_has_data(&no_tr_bound));
}

void test_tls_recv_small_caller_buffer_does_not_overflow(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
                                    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};

    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .server_name = "device.local",
                            .psk_identity = (const uint8_t *)"device_01",
                            .psk_identity_len = 9,
                            .psk_secret = psk,
                            .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&client, &server);

    uint8_t large_payload[64];
    for (size_t i = 0; i < sizeof(large_payload); i++) {
        large_payload[i] = (uint8_t)(i + 0x20U);
    }
    TEST_ASSERT_TRUE(syn_tls_send(&server, large_payload, sizeof(large_payload)));

    struct {
        uint8_t pre_canary[16];
        uint8_t target[16];
        uint8_t post_canary[16];
    } memory_layout;

    memset(memory_layout.pre_canary, 0xAA, sizeof(memory_layout.pre_canary));
    memset(memory_layout.target, 0x00, sizeof(memory_layout.target));
    memset(memory_layout.post_canary, 0x55, sizeof(memory_layout.post_canary));

    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(
        syn_tls_recv(&client, memory_layout.target, sizeof(memory_layout.target), &rx_len));
    TEST_ASSERT_EQUAL(16U, rx_len);
    TEST_ASSERT_EQUAL_MEMORY(large_payload, memory_layout.target, 16U);

    uint8_t expected_pre[16], expected_post[16];
    memset(expected_pre, 0xAA, sizeof(expected_pre));
    memset(expected_post, 0x55, sizeof(expected_post));
    TEST_ASSERT_EQUAL_MEMORY(expected_pre, memory_layout.pre_canary, 16U);
    TEST_ASSERT_EQUAL_MEMORY(expected_post, memory_layout.post_canary, 16U);

    uint8_t remainder[64];
    size_t rem_len = 0U;
    TEST_ASSERT_TRUE(syn_tls_recv(&client, remainder, sizeof(remainder), &rem_len));
    TEST_ASSERT_EQUAL(48U, rem_len);
    TEST_ASSERT_EQUAL_MEMORY(large_payload + 16U, remainder, 48U);
}

void test_tls_x509_mode_handshake(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    uint8_t ca_seed[32] = {0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    uint8_t ca_pub[32], ca_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(ca_pub, ca_sec, ca_seed));

    uint8_t srv_seed[32] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    uint8_t srv_pub[32], srv_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(srv_pub, srv_sec, srv_seed));

    uint8_t cli_seed[32] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18};
    uint8_t cli_pub[32], cli_sec[32];
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(cli_pub, cli_sec, cli_seed));

    uint8_t live_cert_der[184];
    build_ed25519_cert_der(srv_pub, ca_sec, ca_pub, live_cert_der);

    uint8_t cli_cert_der[184];
    build_ed25519_cert_der(cli_pub, ca_sec, ca_pub, cli_cert_der);

    SYN_X509_Cert valid_root_ca;
    memset(&valid_root_ca, 0, sizeof(valid_root_ca));
    memcpy(valid_root_ca.pubkey, ca_pub, 32U);
    valid_root_ca.pubkey_len = 32U;
    valid_root_ca.pubkey_algo = SYN_X509_ALGO_ED25519;
    valid_root_ca.is_ca = true;
    strcpy(valid_root_ca.subject_cn, "Test CA");

    /* 1. Client X509_SERVER fails closed when root_ca or peer_cert_der is missing */
    SYN_TLS_Config missing_ca_cfg = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                                     .server_name = "Test Server",
                                     .root_ca = NULL,
                                     .peer_cert_der = live_cert_der,
                                     .peer_cert_len = sizeof(live_cert_der)};
    SYN_TLS_Context bad_tls;
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &missing_ca_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, bad_tls.state);

    /* 2. Truncated peer_cert_der */
    SYN_TLS_Config trunc_cfg = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                                .server_name = "Test Server",
                                .root_ca = &valid_root_ca,
                                .peer_cert_der = live_cert_der,
                                .peer_cert_len = 10U};
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &trunc_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, bad_tls.state);

    /* 3. CN mismatch */
    SYN_TLS_Config mismatch_cfg = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                                   .server_name = "mismatch.server.com",
                                   .root_ca = &valid_root_ca,
                                   .peer_cert_der = live_cert_der,
                                   .peer_cert_len = sizeof(live_cert_der)};
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &mismatch_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, bad_tls.state);

    /* 4. Server X509_SERVER requires local cert + privkey, while client does NOT! */
    SYN_TLS_Config srv_no_cert = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER, .is_server = true};
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &srv_no_cert, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, bad_tls.state);

    SYN_TLS_Config srv_no_key = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                                 .is_server = true,
                                 .client_cert_der = live_cert_der,
                                 .client_cert_len = sizeof(live_cert_der)};
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &srv_no_key, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, bad_tls.state);

    /* 5. Valid X509_SERVER pair (client has NO client_cert_der; verifies server CertificateVerify!)
     */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config c_x509 = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .server_name = "Test Server",
                             .root_ca = &valid_root_ca,
                             .peer_cert_der = live_cert_der,
                             .peer_cert_len = sizeof(live_cert_der)};
    SYN_TLS_Config s_x509 = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .is_server = true,
                             .client_cert_der = live_cert_der,
                             .client_cert_len = sizeof(live_cert_der),
                             .client_privkey = srv_sec};
    SYN_TLS_Context c_ctx, s_ctx;
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &c_x509, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &s_x509, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c_ctx, &s_ctx);

    /* 6. MTLS requires client_cert_der, client_privkey, AND peer_cert_der on both sides */
    SYN_TLS_Config mtls_missing_local = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                                         .server_name = "Test Server",
                                         .root_ca = &valid_root_ca,
                                         .peer_cert_der = live_cert_der,
                                         .peer_cert_len = sizeof(live_cert_der)};
    TEST_ASSERT_TRUE(syn_tls_init(&bad_tls, &mtls_missing_local, &c_tr, g_c_rx_buf,
                                  sizeof(g_c_rx_buf), g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&bad_tls));

    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config c_mtls = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                             .server_name = "Test Server",
                             .root_ca = &valid_root_ca,
                             .client_cert_der = cli_cert_der,
                             .client_cert_len = sizeof(cli_cert_der),
                             .client_privkey = cli_sec,
                             .peer_cert_der = live_cert_der,
                             .peer_cert_len = sizeof(live_cert_der)};
    SYN_TLS_Config s_mtls = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                             .is_server = true,
                             .root_ca = &valid_root_ca,
                             .client_cert_der = live_cert_der,
                             .client_cert_len = sizeof(live_cert_der),
                             .client_privkey = srv_sec,
                             .peer_cert_der = cli_cert_der,
                             .peer_cert_len = sizeof(cli_cert_der)};
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &c_mtls, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &s_mtls, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c_ctx, &s_ctx);

    /* 7. ECDSA-P256 X509_SERVER (65B uncompressed + 64B raw pubkey) & MITM CV rejection */
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

    SYN_X509_Cert p256_root_ca;
    memset(&p256_root_ca, 0, sizeof(p256_root_ca));
    memcpy(p256_root_ca.pubkey, p256_ca_pub65, 65U);
    p256_root_ca.pubkey_len = 65U;
    p256_root_ca.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    p256_root_ca.is_ca = true;
    strcpy(p256_root_ca.subject_cn, "Test CA");

    uint8_t p256_cert65[256], p256_cert64[256], p256_cert_badlen[256];
    size_t p256_cert65_len = build_p256_cert_der(p256_srv_pub65, 65U, p256_ca_priv, p256_cert65);
    size_t p256_cert64_len = build_p256_cert_der(p256_srv_pub64, 64U, p256_ca_priv, p256_cert64);
    size_t p256_badlen = build_p256_cert_der(p256_srv_pub64, 32U, p256_ca_priv, p256_cert_badlen);

    for (int variant = 0; variant < 2; variant++) {
        const uint8_t *c_der = (variant == 0) ? p256_cert65 : p256_cert64;
        size_t c_len = (variant == 0) ? p256_cert65_len : p256_cert64_len;
        duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
        SYN_TLS_Config cp = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .server_name = "Test Server",
                             .root_ca = &p256_root_ca,
                             .peer_cert_der = c_der,
                             .peer_cert_len = c_len};
        SYN_TLS_Config sp = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .is_server = true,
                             .client_cert_der = c_der,
                             .client_cert_len = c_len,
                             .client_privkey = p256_srv_priv};
        TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &cp, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                      g_c_tx_buf, sizeof(g_c_tx_buf)));
        TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &sp, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                      g_s_tx_buf, sizeof(g_s_tx_buf)));
        pair_tls_handshake(&c_ctx, &s_ctx);
    }

    /* Invalid P-256 SPKI pubkey length (32B != 64B/65B) & zero P-256 signing scalar rejected */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config cp_bad = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .server_name = "Test Server",
                             .root_ca = &p256_root_ca,
                             .peer_cert_der = p256_cert_badlen,
                             .peer_cert_len = p256_badlen};
    SYN_TLS_Config sp_bad = {.mode = SYN_TLS_AUTH_MODE_X509_SERVER,
                             .is_server = true,
                             .client_cert_der = p256_cert_badlen,
                             .client_cert_len = p256_badlen,
                             .client_privkey = p256_srv_priv};
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &cp_bad, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &sp_bad, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_TRUE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_FALSE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, c_ctx.state);

    uint8_t zero_scalar[32] = {0};
    sp_bad.client_privkey = zero_scalar;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &cp_bad, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &sp_bad, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_ctx.state);

    /* Unknown SPKI algo on local cert rejected in server X509 & client mTLS Flight 3 */
    uint8_t unk_spki_cert[184];
    memcpy(unk_spki_cert, live_cert_der, sizeof(live_cert_der));
    unk_spki_cert[74] = 0x71U; /* Unknown SPKI OID */
    TEST_ASSERT_TRUE(
        syn_ed25519_sign(unk_spki_cert + 3U, 107U, ca_sec, ca_pub, unk_spki_cert + 120U));

    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config sp_unk = s_x509;
    sp_unk.client_cert_der = unk_spki_cert;
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &c_x509, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &sp_unk, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_ctx));

    /* Client mTLS Flight 3 failures & server mTLS Flight 3 verification failures */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config c_mtls_unk = c_mtls;
    c_mtls_unk.client_cert_der = unk_spki_cert;
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &c_mtls_unk, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &s_mtls, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_TRUE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_FALSE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, c_ctx.state);

    /* Valid mTLS Flight 2 + Flight 3 saved -> test CV header/scheme/sig & ClientFinished tamper */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&c_ctx, &c_mtls, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_ctx, &s_mtls, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));
    TEST_ASSERT_TRUE(syn_tls_handshake(&s_ctx));

    uint8_t saved_f2[256];
    size_t saved_f2_len = dw.s2c.len;
    memcpy(saved_f2, dw.s2c.buf, saved_f2_len);

    /* Tamper server CV header, sig length, scheme, and MITM signature */
    size_t cv_offsets[4] = {85U, 92U, 89U, 93U};
    for (size_t k = 0U; k < 4U; k++) {
        c_ctx.state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
        syn_sha256_init(&c_ctx.transcript_hash);
        syn_sha384_init(&c_ctx.transcript_sha384);
        syn_sha256_update(&c_ctx.transcript_hash, g_c_tx_buf + 5U, 83U);
        syn_sha512_update(&c_ctx.transcript_sha384, g_c_tx_buf + 5U, 83U);
        memcpy(dw.s2c.buf, saved_f2, saved_f2_len);
        dw.s2c.buf[cv_offsets[k]] ^= 0xFFU;
        dw.s2c.len = saved_f2_len;
        TEST_ASSERT_FALSE(syn_tls_handshake(&c_ctx));
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, c_ctx.state);
    }

    /* Restore valid Flight 2 -> produce valid Flight 3, then tamper Flight 3 on server */
    c_ctx.state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
    syn_sha256_init(&c_ctx.transcript_hash);
    syn_sha384_init(&c_ctx.transcript_sha384);
    syn_sha256_update(&c_ctx.transcript_hash, g_c_tx_buf + 5U, 83U);
    syn_sha512_update(&c_ctx.transcript_sha384, g_c_tx_buf + 5U, 83U);
    memcpy(dw.s2c.buf, saved_f2, saved_f2_len);
    dw.s2c.len = saved_f2_len;
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_ctx));

    uint8_t saved_f3[160];
    size_t saved_f3_len = dw.c2s.len;
    memcpy(saved_f3, dw.c2s.buf, saved_f3_len);
    SYN_SHA256 srv_tr256 = s_ctx.transcript_hash;
    SYN_SHA512 srv_tr384 = s_ctx.transcript_sha384;

    /* Truncated Flight 3 */
    dw.c2s.len = 10U;
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_ctx.state);

    /* Forged client CV signature in Flight 3 */
    s_ctx.state = SYN_TLS_STATE_FINISHED_SENT;
    s_ctx.transcript_hash = srv_tr256;
    s_ctx.transcript_sha384 = srv_tr384;
    memcpy(dw.c2s.buf, saved_f3, saved_f3_len);
    dw.c2s.buf[15] ^= 0xFFU;
    dw.c2s.len = saved_f3_len;
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_ctx.state);

    /* Tampered ClientFinished MAC in Flight 3 */
    s_ctx.state = SYN_TLS_STATE_FINISHED_SENT;
    s_ctx.transcript_hash = srv_tr256;
    s_ctx.transcript_sha384 = srv_tr384;
    memcpy(dw.c2s.buf, saved_f3, saved_f3_len);
    dw.c2s.buf[saved_f3_len - 1U] ^= 0xFFU;
    dw.c2s.len = saved_f3_len;
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_ctx));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_ctx.state);
}

void test_tls_aes_gcm_cipher_suites(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;

    static const uint8_t psk[32] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                                    0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
                                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t rx_buf[128];
    size_t rx_len = 0U;
    static const char msg128[] = "Encrypted with TLS_AES_128_GCM_SHA256!";
    static const char msg256[] = "Encrypted with TLS_AES_256_GCM_SHA384!";

    /* 1. AES-128-GCM */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config cfg128 = {.mode = SYN_TLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config scfg128 = cfg128;
    scfg128.is_server = true;
    SYN_TLS_Context c128, s128;
    TEST_ASSERT_TRUE(syn_tls_init(&c128, &cfg128, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                  sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s128, &scfg128, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c128, &s128);
    TEST_ASSERT_TRUE(syn_tls_send(&c128, (const uint8_t *)msg128, strlen(msg128)));
    TEST_ASSERT_TRUE(syn_tls_recv(&s128, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg128), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg128, rx_buf, rx_len);

    /* 2. AES-256-GCM */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config cfg256 = {.mode = SYN_TLS_AUTH_MODE_PSK,
                             .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                             .psk_secret = psk,
                             .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config scfg256 = cfg256;
    scfg256.is_server = true;
    SYN_TLS_Context c256, s256;
    TEST_ASSERT_TRUE(syn_tls_init(&c256, &cfg256, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                  sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s256, &scfg256, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c256, &s256);
    TEST_ASSERT_TRUE(syn_tls_send(&c256, (const uint8_t *)msg256, strlen(msg256)));
    TEST_ASSERT_TRUE(syn_tls_recv(&s256, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg256), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg256, rx_buf, rx_len);
}

void test_tls_aes_ccm_cipher_suites(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;

    static const uint8_t psk[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t rx_buf[128];
    size_t rx_len = 0U;
    static const char msg_ccm16[] = "TLS_AES_128_CCM_SHA256 Encrypted Payload";
    static const char msg_ccm8[] = "TLS_AES_128_CCM_8_SHA256 Short-Tag Payload";

    /* 1. CCM-16 */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config cfg16 = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256,
                            .psk_secret = psk,
                            .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config scfg16 = cfg16;
    scfg16.is_server = true;
    SYN_TLS_Context c16, s16;
    TEST_ASSERT_TRUE(syn_tls_init(&c16, &cfg16, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                  sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s16, &scfg16, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf), g_s_tx_buf,
                                  sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c16, &s16);
    TEST_ASSERT_TRUE(syn_tls_send(&c16, (const uint8_t *)msg_ccm16, strlen(msg_ccm16)));
    TEST_ASSERT_TRUE(syn_tls_recv(&s16, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg_ccm16), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg_ccm16, rx_buf, rx_len);

    /* 2. CCM-8 */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Config cfg8 = {.mode = SYN_TLS_AUTH_MODE_PSK,
                           .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256,
                           .psk_secret = psk,
                           .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config scfg8 = cfg8;
    scfg8.is_server = true;
    SYN_TLS_Context c8, s8;
    TEST_ASSERT_TRUE(syn_tls_init(&c8, &cfg8, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                  sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s8, &scfg8, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf), g_s_tx_buf,
                                  sizeof(g_s_tx_buf)));
    pair_tls_handshake(&c8, &s8);
    TEST_ASSERT_TRUE(syn_tls_send(&c8, (const uint8_t *)msg_ccm8, strlen(msg_ccm8)));
    TEST_ASSERT_TRUE(syn_tls_recv(&s8, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg_ccm8), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg_ccm8, rx_buf, rx_len);

    /* Tampering test -> fatal SYN_TLS_STATE_ERROR */
    TEST_ASSERT_TRUE(syn_tls_send(&c8, (const uint8_t *)msg_ccm8, strlen(msg_ccm8)));
    dw.c2s.buf[dw.c2s.len - 1U] ^= 0xFFU;
    TEST_ASSERT_FALSE(syn_tls_recv(&s8, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s8.state);

    /* CSPRNG failure MUST fail closed across init, ClientHello, and ServerHello */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    SYN_TLS_Context c_fb, s_fb;
    mock_random_skip = true;
    TEST_ASSERT_FALSE(syn_tls_init(&c_fb, &cfg8, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                   sizeof(g_c_tx_buf)));
    mock_random_skip = false;

    TEST_ASSERT_TRUE(syn_tls_init(&c_fb, &cfg8, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                  sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_fb, &scfg8, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf), g_s_tx_buf,
                                  sizeof(g_s_tx_buf)));
    mock_random_skip = true;
    TEST_ASSERT_FALSE(syn_tls_handshake(&c_fb));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, c_fb.state);
    mock_random_skip = false;

    c_fb.state = SYN_TLS_STATE_UNINITIALIZED;
    TEST_ASSERT_TRUE(syn_tls_handshake(&c_fb));
    mock_random_skip = true;
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_fb));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_fb.state);
    mock_random_skip = false;
}

void test_tls_round4_aad_framing_and_bounds(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    uint8_t rx_buf[128];
    size_t rx_len = 0U;

    /* 1. ClientHello 5B record header + 83B handshake body framing & SHA-384 RPK */
    ensure_test_rpk_keys();
    SYN_TLS_Config cfg_rpk384 = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                                 .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                                 .server_name = "rpk384.local",
                                 .peer_pubkey = g_test_srv_pub,
                                 .peer_pubkey_len = sizeof(g_test_srv_pub)};
    SYN_TLS_Config scfg_rpk384 = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                                  .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                                  .is_server = true,
                                  .client_privkey = g_test_srv_sec};

    SYN_TLS_Config scfg_rpk_nokey = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY, .is_server = true};
    SYN_TLS_Context s_nokey;
    TEST_ASSERT_TRUE(syn_tls_init(&s_nokey, &scfg_rpk_nokey, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&s_nokey));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s_nokey.state);

    SYN_TLS_Context c384, s384;
    memset(g_c_tx_buf, 0xCC, sizeof(g_c_tx_buf));
    TEST_ASSERT_TRUE(syn_tls_init(&c384, &cfg_rpk384, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&s384, &scfg_rpk384, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));

    TEST_ASSERT_TRUE(syn_tls_handshake(&c384));
    TEST_ASSERT_EQUAL_UINT8(0x16U, g_c_tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(83U, g_c_tx_buf[4]);
    TEST_ASSERT_EQUAL_UINT8(0x01U, g_c_tx_buf[5]);
    TEST_ASSERT_EQUAL_UINT8(79U, g_c_tx_buf[8]);
    TEST_ASSERT_EQUAL_MEMORY(c384.my_pubkey, &g_c_tx_buf[56], 32U);
    TEST_ASSERT_EQUAL_UINT8(0xCCU, g_c_tx_buf[88]);

    TEST_ASSERT_TRUE(syn_tls_handshake(&s384));
    TEST_ASSERT_TRUE(syn_tls_handshake(&c384));

    /* 2. Oversized syn_tls_send (> 2048) rejected, exact 2048 accepted */
    static uint8_t big_payload[2049];
    memset(big_payload, 0x5AU, sizeof(big_payload));
    TEST_ASSERT_FALSE(syn_tls_send(&c384, big_payload, 2049U));
    TEST_ASSERT_TRUE(syn_tls_send(&c384, big_payload, 2048U));
    TEST_ASSERT_TRUE(syn_tls_recv(&s384, rx_buf, sizeof(rx_buf), &rx_len));
    size_t total_drained = rx_len;
    while (total_drained < 2048U) {
        TEST_ASSERT_TRUE(syn_tls_recv(&s384, rx_buf, sizeof(rx_buf), &rx_len));
        total_drained += rx_len;
    }
    TEST_ASSERT_EQUAL_UINT32(2048U, (uint32_t)total_drained);

    c384.tx_buf_size = 30U;
    TEST_ASSERT_FALSE(syn_tls_send(&c384, big_payload, 16U));
    c384.tx_buf_size = sizeof(g_c_tx_buf);

    /* 3. 5-byte TLSCiphertext header AAD tamper across all 5 suites */
    static const char aad_msg[] = "TLS 1.3 Header AAD Integrity Check";
    SYN_TLS_CipherSuite all_suites[] = {
        SYN_TLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256, SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256,
        SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384, SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256,
        SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256};

    for (size_t i = 0U; i < sizeof(all_suites) / sizeof(all_suites[0]); i++) {
        SYN_TLS_Config cfg = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                              .cipher_suite = all_suites[i],
                              .peer_pubkey = g_test_srv_pub,
                              .peer_pubkey_len = sizeof(g_test_srv_pub)};
        SYN_TLS_Config scfg = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                               .cipher_suite = all_suites[i],
                               .is_server = true,
                               .client_privkey = g_test_srv_sec};
        SYN_TLS_Context cc, ss;
        duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
        TEST_ASSERT_TRUE(syn_tls_init(&cc, &cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                      sizeof(g_c_tx_buf)));
        TEST_ASSERT_TRUE(syn_tls_init(&ss, &scfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf), g_s_tx_buf,
                                      sizeof(g_s_tx_buf)));
        pair_tls_handshake(&cc, &ss);

        TEST_ASSERT_TRUE(syn_tls_send(&cc, (const uint8_t *)aad_msg, strlen(aad_msg)));
        dw.c2s.buf[dw.c2s.len - 1U] ^= 0x01U;
        TEST_ASSERT_FALSE(syn_tls_recv(&ss, rx_buf, sizeof(rx_buf), &rx_len));
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, ss.state);

        /* Re-establish for AAD header length tamper test */
        duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
        TEST_ASSERT_TRUE(syn_tls_init(&cc, &cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf), g_c_tx_buf,
                                      sizeof(g_c_tx_buf)));
        TEST_ASSERT_TRUE(syn_tls_init(&ss, &scfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf), g_s_tx_buf,
                                      sizeof(g_s_tx_buf)));
        pair_tls_handshake(&cc, &ss);
        TEST_ASSERT_TRUE(syn_tls_send(&cc, (const uint8_t *)aad_msg, strlen(aad_msg)));
        dw.c2s.buf[dw.c2s.len] = 0x00U;
        dw.c2s.len += 1U;
        dw.c2s.buf[4] = (uint8_t)(dw.c2s.buf[4] + 1U);
        TEST_ASSERT_FALSE(syn_tls_recv(&ss, rx_buf, sizeof(rx_buf), &rx_len));
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, ss.state);
    }

    /* 4. Header parser rejection cases (resetting state between fatal errors) */
    uint8_t bad_hdrs[5][5] = {
        {0x16U, 0x03U, 0x03U, 0x00U, 20U},  /* Wrong content type */
        {0x17U, 0x03U, 0x01U, 0x00U, 20U},  /* Wrong minor version */
        {0x17U, 0x02U, 0x03U, 0x00U, 20U},  /* Wrong major version */
        {0x17U, 0x03U, 0x03U, 0x00U, 8U},   /* ciphertext_len < tag_sz + 1 */
        {0x17U, 0x03U, 0x03U, 0x01U, 0x00U} /* ciphertext_len = 256 > rx_len - 5 */
    };
    for (size_t i = 0U; i < 5U; i++) {
        s384.state = SYN_TLS_STATE_ESTABLISHED;
        dw.c2s.len = 32U;
        memset(dw.c2s.buf, 0U, dw.c2s.len);
        memcpy(dw.c2s.buf, bad_hdrs[i], 5U);
        TEST_ASSERT_FALSE(syn_tls_recv(&s384, rx_buf, sizeof(rx_buf), &rx_len));
        TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s384.state);
    }

    /* Oversized RX record payload (> 2048B + 1B inner ContentType) rejected */
    s384.state = SYN_TLS_STATE_ESTABLISHED;
    size_t over_ct_len = 2050U + 16U;
    dw.c2s.len = 5U + over_ct_len;
    memset(dw.c2s.buf, 0xAAU, dw.c2s.len);
    dw.c2s.buf[0] = 0x17U;
    dw.c2s.buf[1] = 0x03U;
    dw.c2s.buf[2] = 0x03U;
    dw.c2s.buf[3] = (uint8_t)((over_ct_len >> 8U) & 0xFFU);
    dw.c2s.buf[4] = (uint8_t)(over_ct_len & 0xFFU);
    TEST_ASSERT_FALSE(syn_tls_recv(&s384, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, s384.state);
}

void test_r5_n05_tls_handshake_reflection_and_fatal_error(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x91, 0x92, 0x93, 0x94};
    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                            .psk_secret = psk,
                            .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&client, &server);

    /* 1. CRITICAL: Self-key reflection attack MUST be rejected and transition to fatal ERROR! */
    static const char reflect_msg[] = "Reflected client record";
    TEST_ASSERT_TRUE(syn_tls_send(&client, (const uint8_t *)reflect_msg, strlen(reflect_msg)));
    memcpy(dw.s2c.buf, dw.c2s.buf, dw.c2s.len);
    dw.s2c.len = dw.c2s.len;
    dw.c2s.len = 0U;

    uint8_t rx_buf[64];
    size_t rx_len = 0U;
    TEST_ASSERT_FALSE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);
    /* Fatal error rejects subsequent send/recv */
    TEST_ASSERT_FALSE(syn_tls_send(&client, (const uint8_t *)"after", 5U));
    TEST_ASSERT_FALSE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));

    /* 2. Inner ContentType 0x17 + optional 0x00 padding, wrong inner type, and all-zero inner */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&client, &server);

    /* Craft padded TLS 1.3 record from server ("HI" + 0x17 + 0x00 + 0x00) */
    uint8_t padded_pt[5] = {'H', 'I', 0x17U, 0x00U, 0x00U};
    uint8_t nonce[12];
    memcpy(nonce, server.server_app_iv, 12U);
    uint8_t aad[5] = {0x17U, 0x03U, 0x03U, 0x00U, 21U};
    memcpy(dw.s2c.buf, aad, 5U);
    SYN_AES_GCM_Context gcm;
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_init(&gcm, server.server_app_key, 16U));
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm, nonce, 12U, aad, 5U, padded_pt, 5U,
                                                  dw.s2c.buf + 5U, dw.s2c.buf + 10U));
    dw.s2c.len = 26U;
    TEST_ASSERT_TRUE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(2U, rx_len);
    TEST_ASSERT_EQUAL_MEMORY("HI", rx_buf, 2U);

    /* Craft empty application payload (0-byte payload + 0x17) -> copy_len == 0 */
    uint8_t empty_pt[1] = {0x17U};
    memcpy(nonce, server.server_app_iv, 12U);
    nonce[11] ^= 0x01U; /* seq 1 */
    uint8_t empty_aad[5] = {0x17U, 0x03U, 0x03U, 0x00U, 17U};
    memcpy(dw.s2c.buf, empty_aad, 5U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm, nonce, 12U, empty_aad, 5U, empty_pt, 1U,
                                                  dw.s2c.buf + 5U, dw.s2c.buf + 6U));
    dw.s2c.len = 22U;
    TEST_ASSERT_TRUE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(0U, rx_len);

    /* Craft wrong inner ContentType (0x16) -> rejected & fatal ERROR */
    uint8_t bad_ct_pt[2] = {'X', 0x16U};
    memcpy(nonce, server.server_app_iv, 12U);
    nonce[11] ^= 0x02U; /* seq 2 */
    uint8_t bad_aad[5] = {0x17U, 0x03U, 0x03U, 0x00U, 18U};
    memcpy(dw.s2c.buf, bad_aad, 5U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm, nonce, 12U, bad_aad, 5U, bad_ct_pt, 2U,
                                                  dw.s2c.buf + 5U, dw.s2c.buf + 7U));
    dw.s2c.len = 23U;
    TEST_ASSERT_FALSE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);

    /* Craft all-zero inner plaintext (0x00, 0x00 -> scan == 0) -> rejected */
    client.state = SYN_TLS_STATE_ESTABLISHED;
    uint8_t zero_pt[2] = {0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_aes_gcm_encrypt(&gcm, nonce, 12U, bad_aad, 5U, zero_pt, 2U,
                                                  dw.s2c.buf + 5U, dw.s2c.buf + 7U));
    dw.s2c.len = 23U;
    TEST_ASSERT_FALSE(syn_tls_recv(&client, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);

    /* 3. Handshake tamper, all-zero X25519 key_share, transport send fail, & RPK/enum guards */
    uint8_t short_rpk[16] = {0};
    SYN_TLS_Config bad_rpk = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                              .cipher_suite = (SYN_TLS_CipherSuite)99,
                              .peer_pubkey = short_rpk,
                              .peer_pubkey_len = 16U};
    TEST_ASSERT_TRUE(syn_tls_init(&client, &bad_rpk, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&client));

    uint8_t ok_rpk[32] = {0x01U};
    SYN_TLS_Config ok_rpk_cfg = {.mode = SYN_TLS_AUTH_MODE_RAW_PUBKEY,
                                 .cipher_suite = (SYN_TLS_CipherSuite)99,
                                 .peer_pubkey = ok_rpk,
                                 .peer_pubkey_len = 32U};
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    dw.c2s.fail_send = true;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &ok_rpk_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_FALSE(syn_tls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);

    /* Malformed ClientHello rejected on server */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    dw.c2s.len = 20U;
    TEST_ASSERT_FALSE(syn_tls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, server.state);

    /* All-zero X25519 ClientHello key_share rejected on server */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&client));
    memset(dw.c2s.buf + 56U, 0, 32U);
    TEST_ASSERT_FALSE(syn_tls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, server.state);

    /* Server transport send failure on ServerHello */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&client));
    dw.s2c.fail_send = true;
    TEST_ASSERT_FALSE(syn_tls_handshake(&server));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, server.state);

    /* Malformed ServerHello, all-zero X25519 ServerHello key_share, and tampered Finished MAC */
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_handshake(&client));
    TEST_ASSERT_TRUE(syn_tls_handshake(&server));

    uint8_t saved_sh[160];
    size_t saved_sh_len = dw.s2c.len;
    memcpy(saved_sh, dw.s2c.buf, saved_sh_len);

    /* Tampered Finished MAC */
    dw.s2c.buf[saved_sh_len - 1U] ^= 0xFFU;
    TEST_ASSERT_FALSE(syn_tls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);

    /* All-zero ServerHello key_share */
    client.state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
    memcpy(dw.s2c.buf, saved_sh, saved_sh_len);
    memset(dw.s2c.buf + 53U, 0, 32U);
    dw.s2c.len = saved_sh_len;
    TEST_ASSERT_FALSE(syn_tls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);

    /* Truncated ServerHello */
    client.state = SYN_TLS_STATE_CLIENT_HELLO_SENT;
    dw.s2c.len = 12U;
    TEST_ASSERT_FALSE(syn_tls_handshake(&client));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);
}

static void test_tier_a_tls_seq_backpressure_and_secret_scrub(void)
{
    DuplexWire dw;
    EndpointIo c_io, s_io;
    SYN_Transport c_tr, s_tr;
    duplex_init(&dw, &c_io, &s_io, &c_tr, &s_tr, false);

    static const uint8_t psk[32] = {0x12, 0x34, 0x56, 0x78};
    static const uint8_t zeros[SYN_TLS_SECRET_LEN] = {0};
    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256,
                            .psk_secret = psk,
                            .psk_secret_len = sizeof(psk)};
    SYN_TLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    SYN_TLS_Context client, server;
    TEST_ASSERT_TRUE(syn_tls_init(&client, &c_cfg, &c_tr, g_c_rx_buf, sizeof(g_c_rx_buf),
                                  g_c_tx_buf, sizeof(g_c_tx_buf)));
    TEST_ASSERT_TRUE(syn_tls_init(&server, &s_cfg, &s_tr, g_s_rx_buf, sizeof(g_s_rx_buf),
                                  g_s_tx_buf, sizeof(g_s_tx_buf)));
    pair_tls_handshake(&client, &server);

    /* TLS-4: Ephemeral private key and handshake/master secrets scrubbed upon ESTABLISHED */
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.my_privkey, sizeof(client.my_privkey));
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.client_handshake_secret, SYN_TLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.server_handshake_secret, SYN_TLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, client.master_secret, SYN_TLS_SECRET_LEN);
    TEST_ASSERT_EQUAL_MEMORY(zeros, server.my_privkey, sizeof(server.my_privkey));
    TEST_ASSERT_EQUAL_MEMORY(zeros, server.master_secret, SYN_TLS_SECRET_LEN);

    /* TLS-2: Non-blocking TX backpressure must not desynchronize AEAD sequence counter */
    static const char msg[] = "Backpressure-safe TLS 1.3 record";
    dw.c2s.fail_send = true;
    TEST_ASSERT_FALSE(syn_tls_send(&client, (const uint8_t *)msg, strlen(msg)));
    TEST_ASSERT_EQUAL_UINT64(0ULL, client.client_seq_num);
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, client.state);

    dw.c2s.fail_send = false;
    TEST_ASSERT_TRUE(syn_tls_send(&client, (const uint8_t *)msg, strlen(msg)));
    TEST_ASSERT_EQUAL_UINT64(1ULL, client.client_seq_num);

    uint8_t rx_buf[64] = {0};
    size_t rx_len = 0U;
    TEST_ASSERT_TRUE(syn_tls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(strlen(msg), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx_buf, rx_len);

    /* TLS-2: RFC 8446 §5.3 UINT64_MAX sequence exhaustion terminates connection */
    TEST_ASSERT_TRUE(syn_tls_send(&client, (const uint8_t *)msg, strlen(msg)));
    server.server_seq_num = UINT64_MAX;
    TEST_ASSERT_FALSE(syn_tls_recv(&server, rx_buf, sizeof(rx_buf), &rx_len));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, server.state);

    client.client_seq_num = UINT64_MAX;
    TEST_ASSERT_FALSE(syn_tls_send(&client, (const uint8_t *)msg, strlen(msg)));
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ERROR, client.state);
}

void run_tls_tests(void)
{
    RUN_TEST(test_tls_psk_mode_handshake_and_record_crypto);
    RUN_TEST(test_tls_transport_binding_interface);
    RUN_TEST(test_tls_task_protothread);
    RUN_TEST(test_tls_null_and_bounds_checks);
    RUN_TEST(test_tls_recv_small_caller_buffer_does_not_overflow);
    RUN_TEST(test_tls_x509_mode_handshake);
    RUN_TEST(test_tls_aes_gcm_cipher_suites);
    RUN_TEST(test_tls_aes_ccm_cipher_suites);
    RUN_TEST(test_tls_round4_aad_framing_and_bounds);
    RUN_TEST(test_r5_n05_tls_handshake_reflection_and_fatal_error);
    RUN_TEST(test_tier_a_tls_seq_backpressure_and_secret_scrub);
}
