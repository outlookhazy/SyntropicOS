/**
 * @file test_wg_integration.c
 * @brief End-to-end WireGuard Noise_IKpsk2 tunnel integration test and container responder.
 */

#ifdef WG_DAEMON_MAIN

#include "syntropic/crypto/syn_blake2s.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_x25519.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const uint8_t WG_CONSTRUCTION[] = "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s";
static const uint8_t WG_IDENTIFIER[] = "WireGuard v1 zx2c4 Jason@zx2c4.com";
static const uint8_t WG_LABEL_MAC1[] = "mac1----";

typedef struct {
    uint8_t srv_priv[32];
    uint8_t srv_pub[32];
    uint8_t cli_pub[32];
    uint8_t recv_key[32];
    uint8_t send_key[32];
    uint32_t srv_index;
    uint32_t peer_index;
    uint64_t send_counter;
    uint32_t hs_seq;
    bool established;
} WgDaemonState;

static void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8U);
    p[2] = (uint8_t)(v >> 16U);
    p[3] = (uint8_t)(v >> 24U);
}

static uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8U) | ((uint32_t)p[2] << 16U) |
           ((uint32_t)p[3] << 24U);
}

static void store64_le(uint8_t *p, uint64_t v)
{
    store32_le(p, (uint32_t)v);
    store32_le(p + 4U, (uint32_t)(v >> 32U));
}

static uint64_t load64_le(const uint8_t *p)
{
    return (uint64_t)load32_le(p) | ((uint64_t)load32_le(p + 4U) << 32U);
}

static uint16_t inet_csum16(const uint8_t *buf, size_t len)
{
    uint32_t sum = 0U;
    size_t i = 0U;
    while (i + 1U < len) {
        uint16_t word = (uint16_t)(((uint16_t)buf[i] << 8U) | (uint16_t)buf[i + 1U]);
        sum += word;
        i += 2U;
    }
    if (i < len) {
        sum += (uint32_t)((uint16_t)buf[i] << 8U);
    }
    while ((sum >> 16U) != 0U) {
        sum = (sum & 0xFFFFU) + (sum >> 16U);
    }
    return (uint16_t)(~sum & 0xFFFFU);
}

static void wg_mix_hash(uint8_t h[32], const void *data, size_t len)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32U);
    syn_blake2s_update(&ctx, h, 32U);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, h);
}

static void wg_hkdf2(uint8_t *t1, uint8_t *t2, const uint8_t *ck, const void *in, size_t n)
{
    uint8_t prk[32];
    uint8_t tmp[33];
    syn_hmac_blake2s(ck, 32U, in, n, prk);
    tmp[0] = 0x01U;
    syn_hmac_blake2s(prk, 32U, tmp, 1U, t1);
    memcpy(tmp, t1, 32U);
    tmp[32] = 0x02U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, t2);
}

static void wg_hkdf3(uint8_t *t1, uint8_t *t2, uint8_t *t3, const uint8_t *ck, const uint8_t *q)
{
    uint8_t prk[32];
    uint8_t tmp[33];
    syn_hmac_blake2s(ck, 32U, q, 32U, prk);
    tmp[0] = 0x01U;
    syn_hmac_blake2s(prk, 32U, tmp, 1U, t1);
    memcpy(tmp, t1, 32U);
    tmp[32] = 0x02U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, t2);
    memcpy(tmp, t2, 32U);
    tmp[32] = 0x03U;
    syn_hmac_blake2s(prk, 32U, tmp, 33U, t3);
}

static void wg_mac1(uint8_t mac[16], const uint8_t peer_pub[32], const uint8_t *msg, size_t len)
{
    uint8_t key[32];
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32U);
    syn_blake2s_update(&ctx, WG_LABEL_MAC1, sizeof(WG_LABEL_MAC1) - 1U);
    syn_blake2s_update(&ctx, peer_pub, 32U);
    syn_blake2s_final(&ctx, key);
    syn_blake2s_mac(key, 32U, msg, len, mac, 16U);
}

static bool wg_dec_hash(uint8_t *h, const uint8_t *k, const uint8_t *ct, size_t n, uint8_t *out)
{
    uint8_t nonce[12] = {0};
    const uint8_t *tag = ct + n;
    if (!syn_aead_decrypt(k, nonce, h, 32U, ct, n, tag, out)) {
        return false;
    }
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32U);
    syn_blake2s_update(&ctx, h, 32U);
    syn_blake2s_update(&ctx, ct, n);
    syn_blake2s_update(&ctx, tag, 16U);
    syn_blake2s_final(&ctx, h);
    return true;
}

static void wg_encrypt_empty_and_hash(uint8_t h[32], const uint8_t k[32], uint8_t tag[16])
{
    uint8_t nonce[12] = {0};
    uint8_t dummy = 0U;
    syn_aead_encrypt(k, nonce, h, 32U, &dummy, 0U, &dummy, tag);
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, 32U);
    syn_blake2s_update(&ctx, h, 32U);
    syn_blake2s_update(&ctx, tag, 16U);
    syn_blake2s_final(&ctx, h);
}

static bool handle_initiation(WgDaemonState *st, const uint8_t *msg, size_t len, uint8_t resp[92])
{
    if (len != 148U || load32_le(msg) != 1U) {
        return false;
    }
    uint8_t expected_mac[16];
    wg_mac1(expected_mac, st->srv_pub, msg, 116U);
    if (memcmp(expected_mac, msg + 116U, 16U) != 0) {
        return false;
    }

    uint8_t ck[32];
    uint8_t h[32];
    uint8_t k[32];
    uint8_t dh[32];
    syn_blake2s(WG_CONSTRUCTION, sizeof(WG_CONSTRUCTION) - 1U, ck, 32U);
    SYN_BLAKE2s bctx;
    syn_blake2s_init(&bctx, 32U);
    syn_blake2s_update(&bctx, ck, 32U);
    syn_blake2s_update(&bctx, WG_IDENTIFIER, sizeof(WG_IDENTIFIER) - 1U);
    syn_blake2s_final(&bctx, h);
    wg_mix_hash(h, st->srv_pub, 32U);

    const uint8_t *e_pub_i = msg + 8U;
    wg_hkdf2(ck, k, ck, e_pub_i, 32U);
    wg_mix_hash(h, e_pub_i, 32U);

    syn_x25519(dh, st->srv_priv, e_pub_i);
    wg_hkdf2(ck, k, ck, dh, 32U);
    uint8_t dec_static[32];
    if (!wg_dec_hash(h, k, msg + 40U, 32U, dec_static)) {
        return false;
    }
    if (memcmp(dec_static, st->cli_pub, 32U) != 0) {
        return false;
    }

    syn_x25519(dh, st->srv_priv, st->cli_pub);
    wg_hkdf2(ck, k, ck, dh, 32U);
    uint8_t dec_ts[12];
    if (!wg_dec_hash(h, k, msg + 88U, 12U, dec_ts)) {
        return false;
    }

    uint8_t e_priv_r[32];
    uint8_t e_pub_r[32];
    memset(e_priv_r, 0x07, 32U);
    e_priv_r[1] = (uint8_t)(++st->hs_seq);
    syn_x25519_clamp(e_priv_r);
    syn_x25519_pubkey(e_pub_r, e_priv_r);

    st->srv_index = 0x53594E00U + st->hs_seq;
    st->peer_index = load32_le(msg + 4U);
    memset(resp, 0, 92U);
    store32_le(resp, 2U);
    store32_le(resp + 4U, st->srv_index);
    store32_le(resp + 8U, st->peer_index);
    memcpy(resp + 12U, e_pub_r, 32U);

    wg_hkdf2(ck, k, ck, e_pub_r, 32U);
    wg_mix_hash(h, e_pub_r, 32U);
    syn_x25519(dh, e_priv_r, e_pub_i);
    wg_hkdf2(ck, k, ck, dh, 32U);
    syn_x25519(dh, e_priv_r, st->cli_pub);
    wg_hkdf2(ck, k, ck, dh, 32U);

    uint8_t zeros[32] = {0};
    uint8_t tau[32];
    wg_hkdf3(ck, tau, k, ck, zeros);
    wg_mix_hash(h, tau, 32U);
    wg_encrypt_empty_and_hash(h, k, resp + 44U);
    wg_mac1(resp + 60U, st->cli_pub, resp, 60U);

    wg_hkdf2(st->recv_key, st->send_key, ck, zeros, 0U);
    st->send_counter = 0U;
    st->established = true;
    return true;
}

static size_t handle_transport(WgDaemonState *st, const uint8_t *msg, size_t len, uint8_t *out)
{
    if (!st->established || len < 32U || load32_le(msg) != 4U) {
        return 0U;
    }
    if (load32_le(msg + 4U) != st->srv_index) {
        return 0U;
    }
    uint64_t rx_cnt = load64_le(msg + 8U);
    uint8_t nonce[12] = {0};
    store64_le(nonce + 4U, rx_cnt);

    size_t ct_len = len - 32U;
    uint8_t plain[1500];
    if (ct_len == 0U || ct_len > sizeof(plain)) {
        return 0U;
    }
    const uint8_t *ct = msg + 16U;
    if (!syn_aead_decrypt(st->recv_key, nonce, NULL, 0U, ct, ct_len, ct + ct_len, plain)) {
        return 0U;
    }
    if (ct_len >= 28U && plain[0] == 0x45U && plain[9] == 1U && plain[20] == 8U) {
        uint8_t src_ip[4];
        memcpy(src_ip, plain + 12U, 4U);
        memcpy(plain + 12U, plain + 16U, 4U);
        memcpy(plain + 16U, src_ip, 4U);
        plain[10] = 0U;
        plain[11] = 0U;
        uint16_t ip_csum = inet_csum16(plain, 20U);
        plain[10] = (uint8_t)(ip_csum >> 8U);
        plain[11] = (uint8_t)(ip_csum & 0xFFU);

        plain[20] = 0U;
        plain[22] = 0U;
        plain[23] = 0U;
        uint16_t icmp_csum = inet_csum16(plain + 20U, ct_len - 20U);
        plain[22] = (uint8_t)(icmp_csum >> 8U);
        plain[23] = (uint8_t)(icmp_csum & 0xFFU);
    }

    store32_le(out, 4U);
    store32_le(out + 4U, st->peer_index);
    store64_le(out + 8U, st->send_counter);
    memset(nonce, 0, 4U);
    store64_le(nonce + 4U, st->send_counter);
    st->send_counter++;
    uint8_t *out_ct = out + 16U;
    syn_aead_encrypt(st->send_key, nonce, NULL, 0U, plain, ct_len, out_ct, out_ct + ct_len);
    return ct_len + 32U;
}

int main(void)
{
    WgDaemonState st;
    memset(&st, 0, sizeof(st));

    uint8_t cli_priv[32];
    memset(cli_priv, 0x01, 32U);
    syn_x25519_clamp(cli_priv);
    syn_x25519_pubkey(st.cli_pub, cli_priv);

    memset(st.srv_priv, 0x02, 32U);
    syn_x25519_clamp(st.srv_priv);
    syn_x25519_pubkey(st.srv_pub, st.srv_priv);

    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        return 1;
    }
    int opt = 1;
    (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(51820U);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 1;
    }

    printf("[WireGuard Daemon] Userspace Noise_IKpsk2 responder listening on 51820/udp\n");
    (void)fflush(stdout);

    uint8_t rx_buf[1600];
    uint8_t tx_buf[1600];
    for (;;) {
        struct sockaddr_in peer;
        socklen_t peer_len = sizeof(peer);
        ssize_t n = recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (struct sockaddr *)&peer, &peer_len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if ((size_t)n == 148U && handle_initiation(&st, rx_buf, 148U, tx_buf)) {
            (void)sendto(fd, tx_buf, 92U, 0, (struct sockaddr *)&peer, peer_len);
        } else if ((size_t)n >= 32U) {
            size_t out_len = handle_transport(&st, rx_buf, (size_t)n, tx_buf);
            if (out_len > 0U) {
                (void)sendto(fd, tx_buf, out_len, 0, (struct sockaddr *)&peer, peer_len);
            }
        }
    }

    close(fd);
    return 0;
}

#else

#include "mock_port.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/net/syn_sntp.h"
#include "syntropic/net/syn_wg.h"
#include "unity/unity.h"

#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool wg_packet_received = false;
static size_t wg_rx_len = 0U;
static uint8_t wg_rx_pkt[128];

static bool integration_skip_allowed(void)
{
    const char *v = getenv("SYN_ALLOW_SKIP_INTEGRATION");
    return (v != NULL && strcmp(v, "1") == 0);
}

static void fail_or_skip(const char *msg)
{
    if (integration_skip_allowed()) {
        TEST_IGNORE_MESSAGE(msg);
    } else {
        TEST_FAIL_MESSAGE(msg);
    }
}

static uint16_t inet_csum16(const uint8_t *buf, size_t len)
{
    uint32_t sum = 0U;
    size_t i = 0U;
    while (i + 1U < len) {
        uint16_t word = (uint16_t)(((uint16_t)buf[i] << 8U) | (uint16_t)buf[i + 1U]);
        sum += word;
        i += 2U;
    }
    if (i < len) {
        sum += (uint32_t)((uint16_t)buf[i] << 8U);
    }
    while ((sum >> 16U) != 0U) {
        sum = (sum & 0xFFFFU) + (sum >> 16U);
    }
    return (uint16_t)(~sum & 0xFFFFU);
}

static bool resolve_ipv4(const char *host, uint8_t out_ip[4])
{
    struct hostent *he = gethostbyname(host);
    if (he == NULL || he->h_addr_list[0] == NULL) {
        return false;
    }
    memcpy(out_ip, he->h_addr_list[0], 4U);
    return true;
}

static void build_icmp_echo_request(uint8_t pkt[28])
{
    memset(pkt, 0, 28U);
    pkt[0] = 0x45U;
    pkt[3] = 28U;
    pkt[4] = 0x12U;
    pkt[5] = 0x34U;
    pkt[8] = 64U;
    pkt[9] = 1U;
    pkt[12] = 10U;
    pkt[13] = 0U;
    pkt[14] = 0U;
    pkt[15] = 2U;
    pkt[16] = 10U;
    pkt[17] = 0U;
    pkt[18] = 0U;
    pkt[19] = 1U;
    uint16_t ip_csum = inet_csum16(pkt, 20U);
    pkt[10] = (uint8_t)(ip_csum >> 8U);
    pkt[11] = (uint8_t)(ip_csum & 0xFFU);

    pkt[20] = 8U;
    pkt[24] = 0x00U;
    pkt[25] = 0x01U;
    pkt[26] = 0x00U;
    pkt[27] = 0x01U;
    uint16_t icmp_csum = inet_csum16(pkt + 20U, 8U);
    pkt[22] = (uint8_t)(icmp_csum >> 8U);
    pkt[23] = (uint8_t)(icmp_csum & 0xFFU);
}

static void on_wg_recv(const uint8_t *ip_packet, size_t len, void *ctx)
{
    (void)ctx;
    wg_rx_len = len;
    size_t copy_len = (len < sizeof(wg_rx_pkt)) ? len : sizeof(wg_rx_pkt);
    memcpy(wg_rx_pkt, ip_packet, copy_len);
    wg_packet_received = true;
}

static bool sync_sntp_for_wg(SYN_SNTP *sntp)
{
    const char *ntp_host = getenv("SNTP_HOST");
    if (ntp_host == NULL) {
        ntp_host = "127.0.0.1";
    }
    SYN_SockAddr ntp_srv;
    memset(&ntp_srv, 0, sizeof(ntp_srv));
    ntp_srv.port = (strcmp(ntp_host, "127.0.0.1") == 0) ? 10123U : 123U;
    if (!resolve_ipv4(ntp_host, ntp_srv.ip)) {
        return false;
    }
    syn_sntp_init(sntp, &ntp_srv, 3600U);
    for (int i = 0; i < 5; i++) {
        if (syn_sntp_query(sntp) == SYN_OK) {
            return true;
        }
        usleep(200000);
    }
    return false;
}

void setUp(void)
{
    wg_packet_received = false;
    wg_rx_len = 0U;
    memset(wg_rx_pkt, 0, sizeof(wg_rx_pkt));
}

void tearDown(void)
{
}

void test_wireguard_vpn_integration(void)
{
    const char *host = getenv("WG_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    SYN_SNTP sntp;
    if (!sync_sntp_for_wg(&sntp)) {
        fail_or_skip("Chrony SNTP time source for WireGuard TAI64N not reachable");
        return;
    }

    SYN_WgConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.endpoint.port = 51820U;
    if (!resolve_ipv4(host, cfg.endpoint.ip)) {
        fail_or_skip("WireGuard container host resolution failed");
        return;
    }
    cfg.keepalive_interval_s = 25U;

    memset(cfg.private_key, 0x01, 32U);
    syn_x25519_clamp(cfg.private_key);
    uint8_t srv_priv[32];
    memset(srv_priv, 0x02, 32U);
    syn_x25519_clamp(srv_priv);
    syn_x25519_pubkey(cfg.peer_public_key, srv_priv);

    SYN_WG wg;
    uint8_t rx_buf[1600];
    uint8_t tx_buf[1600];
    syn_wg_init(&wg, &cfg, &sntp, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    wg.on_recv = on_wg_recv;
    TEST_ASSERT_EQUAL_INT(SYN_WG_DISCONNECTED, wg.state);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    memset(&task, 0, sizeof(task));
    task.user_data = &wg;

    for (int i = 0; i < 200 && !syn_wg_is_established(&wg); i++) {
        mock_tick_advance(100U);
        syn_wg_task(&pt, &task);
        usleep(15000);
    }
    if (!syn_wg_is_established(&wg)) {
        syn_wg_disconnect(&wg);
        fail_or_skip("WireGuard Noise_IKpsk2 handshake with server container timed out");
        return;
    }

    TEST_ASSERT_NOT_EQUAL(SYN_SOCKET_INVALID, wg.udp_sock);
    TEST_ASSERT_TRUE(syn_wg_is_established(&wg));

    uint8_t icmp_req[28];
    build_icmp_echo_request(icmp_req);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_wg_send(&wg, icmp_req, sizeof(icmp_req)));

    for (int i = 0; i < 200 && !wg_packet_received; i++) {
        mock_tick_advance(10U);
        syn_wg_task(&pt, &task);
        usleep(10000);
    }

    SYN_WgStats stats;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_wg_get_stats(&wg, &stats));
    TEST_ASSERT_TRUE(stats.is_established);
    TEST_ASSERT_TRUE(stats.tx_bytes > 0U);

    syn_wg_disconnect(&wg);
    TEST_ASSERT_EQUAL_INT(SYN_SOCKET_INVALID, wg.udp_sock);
    TEST_ASSERT_TRUE(wg_packet_received);
    TEST_ASSERT_EQUAL_UINT32(28U, (uint32_t)wg_rx_len);
    TEST_ASSERT_EQUAL_UINT8(0x45U, wg_rx_pkt[0]);
    TEST_ASSERT_EQUAL_UINT8(0U, wg_rx_pkt[20]);
    printf("[Integration Test] End-to-End WireGuard Noise_IKpsk2 Tunnel Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_wireguard_vpn_integration);
    return UNITY_END();
}

#endif
