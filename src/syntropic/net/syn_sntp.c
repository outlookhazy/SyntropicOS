#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SNTP) || SYN_USE_SNTP

/**
 * @file syn_sntp.c
 * @brief SNTP client implementation — RFC 4330 subset.
 */

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_sntp.h"

#include <stdio.h>
#include <string.h>

/* ── Internal Helpers ───────────────────────────────────────────────────── */

static SYN_Status sntp_send_request(const SYN_SNTP *sntp, SYN_Socket sock);
static SYN_Status sntp_parse_packet(SYN_SNTP *sntp, const uint8_t *pkt);
static uint32_t sntp_sync_wait_ms(uint32_t sync_interval_s);

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sntp_init(SYN_SNTP *sntp, const SYN_SockAddr *server, uint32_t sync_interval_s)
{
    SYN_ASSERT(sntp != NULL);
    SYN_ASSERT(server != NULL);

    if (sync_interval_s == 0U) {
        sync_interval_s = 1U;
    } else if (sync_interval_s > 2147483U) {
        sync_interval_s = 2147483U;
    }

    memset(sntp, 0, sizeof(*sntp));
    sntp->server = *server;
    sntp->sync_interval_s = sync_interval_s;
    sntp->synced = false;

    /* Initialize backoff: 1s base, 60s max, factor 2, max_retries */
    syn_backoff_init(&sntp->backoff, 1000, 60000, 2, SYN_SNTP_MAX_RETRIES);
    /* Initialize EMA drift filter: alpha=64 (~0.25 smoothing factor for network jitter rejection)
     */
    syn_filter_ema_init(&sntp->drift_filter, 64);
    sntp->udp_sock = SYN_SOCKET_INVALID;
}

/**
 * @brief Verify that the UDP datagram sender matches the configured server IP/port when non-zero.
 * @param sntp SNTP client context.
 * @param from Sender socket address reported by recvfrom.
 * @return true if server IP/port are wildcard or match sender address.
 */
static bool sntp_sender_valid(const SYN_SNTP *sntp, const SYN_SockAddr *from)
{
    bool port_ok = (sntp->server.port == 0U) || (from->port == sntp->server.port);
    uint8_t configured = (uint8_t)(sntp->server.ip[0] | sntp->server.ip[1] | sntp->server.ip[2] |
                                   sntp->server.ip[3]);
    if (configured == 0U) {
        return port_ok;
    }
    return port_ok && (memcmp(from->ip, sntp->server.ip, 4U) == 0);
}

/**
 * @brief Validate SNTP response header fields (LI, Mode, and Stratum per RFC 4330).
 * @param pkt Raw 48-byte NTP packet.
 * @return true if synchronized server/broadcast header is valid, false otherwise.
 */
static bool sntp_header_valid(const uint8_t *pkt)
{
    uint8_t li = (pkt[0] >> 6U) & 0x03U;
    uint8_t mode = pkt[0] & 0x07U;
    uint8_t stratum = pkt[1];
    if (li == 3U || (mode != 4U && mode != 5U)) {
        return false;
    }
    return (stratum >= 1U) && (stratum <= 15U);
}

/**
 * @brief Parse an SNTP response packet and extract the transmit timestamp.
 * @param sntp  SNTP client instance.
 * @param pkt   Raw UDP payload (at least SYN_SNTP_PACKET_SIZE bytes).
 * @return SYN_OK on success, SYN_ERROR on invalid response.
 */
static SYN_Status sntp_parse_packet(SYN_SNTP *sntp, const uint8_t *pkt)
{
    if (!sntp_header_valid(pkt)) {
        return SYN_ERROR;
    }

    /* Extract transmit timestamp (bytes 40–47, NTP epoch big-endian) */
    uint32_t ntp_s = syn_peek_u32(pkt, 40);
    uint32_t ntp_frac = syn_peek_u32(pkt, 44);

    if (ntp_s < SYN_SNTP_EPOCH_OFFSET) {
        return SYN_ERROR;
    }

    uint32_t new_epoch = ntp_s - SYN_SNTP_EPOCH_OFFSET;
    uint32_t new_tick_ms = syn_port_get_tick_ms();

    /* Calculate clock drift in PPM if we have a monotonic forward sync baseline */
    if (sntp->synced && sntp->prev_sync_epoch != 0U && new_epoch > sntp->prev_sync_epoch) {
        uint32_t ntp_elapsed_s = new_epoch - sntp->prev_sync_epoch;
        uint32_t tick_elapsed_ms = new_tick_ms - sntp->prev_sync_tick_ms;

        if (ntp_elapsed_s >= 5U) {
            int64_t expected_ms = (int64_t)ntp_elapsed_s * 1000LL;
            int64_t diff_ms = (int64_t)tick_elapsed_ms - expected_ms;
            int64_t ppm64 = (diff_ms * 1000000LL) / expected_ms;
            /* Clamp to int16_t range before cast to avoid silent wrap */
            if (ppm64 > 32767) {
                ppm64 = 32767;
            }
            if (ppm64 < -32767) {
                ppm64 = -32767;
            }
            int16_t raw_ppm = (int16_t)ppm64;

            /* Apply SyntropicOS Exponential Moving Average Filter (SYN_FilterEMA) */
            sntp->drift_ppm = (int32_t)syn_filter_ema_update(&sntp->drift_filter, raw_ppm);
        }
    }

    sntp->prev_sync_epoch = new_epoch;
    sntp->prev_sync_tick_ms = new_tick_ms;
    sntp->epoch_s = new_epoch;
    sntp->epoch_frac = ntp_frac;
    sntp->sync_tick_ms = new_tick_ms;
    sntp->synced = true;
    sntp->cycle_ok = true;

    return SYN_OK;
}

SYN_Status syn_sntp_query(SYN_SNTP *sntp)
{
    SYN_ASSERT(sntp != NULL);

    uint8_t pkt[SYN_SNTP_PACKET_SIZE];
    SYN_SockAddr from = {{0U, 0U, 0U, 0U}, 0U};

    SYN_Socket sock = syn_port_udp_open(0);
    if (sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    if (sntp_send_request(sntp, sock) != SYN_OK) {
        syn_port_sock_close(sock);
        return SYN_ERROR;
    }

    int n = syn_port_udp_recvfrom(sock, pkt, sizeof(pkt), &from, SYN_SNTP_TIMEOUT_MS);
    syn_port_sock_close(sock);

    if (n < (int)SYN_SNTP_PACKET_SIZE) {
        return (n <= 0) ? SYN_TIMEOUT : SYN_ERROR;
    }
    if (!sntp_sender_valid(sntp, &from)) {
        return SYN_ERROR;
    }

    return sntp_parse_packet(sntp, pkt);
}

/**
 * @brief Compute current UTC epoch seconds and nanoseconds including NTP fractional carry.
 * @param sntp   Synced SNTP client context (non-NULL).
 * @param out_s  [out] UTC epoch seconds.
 * @param out_ns [out] Sub-second nanoseconds (0..999999999).
 */
static void sntp_compute_time(const SYN_SNTP *sntp, uint32_t *out_s, uint32_t *out_ns)
{
    uint32_t elapsed_ms = syn_port_get_tick_ms() - sntp->sync_tick_ms;
    uint32_t sub_s_ns = (elapsed_ms % 1000U) * 1000000U;
    uint32_t frac_ns = (uint32_t)(((uint64_t)sntp->epoch_frac * 1000000000ULL) >> 32U);
    uint32_t total_ns = frac_ns + sub_s_ns;
    uint32_t carry_s = 0U;

    if (total_ns >= 1000000000U) {
        carry_s = 1U;
        total_ns -= 1000000000U;
    }

    *out_s = sntp->epoch_s + (elapsed_ms / 1000U) + carry_s;
    *out_ns = total_ns;
}

uint32_t syn_sntp_get_epoch_s(const SYN_SNTP *sntp)
{
    if (sntp == NULL || !sntp->synced) {
        return 0U;
    }

    uint32_t sec = 0U;
    uint32_t ns = 0U;
    sntp_compute_time(sntp, &sec, &ns);
    return sec;
}

uint32_t syn_sntp_get_epoch_ns(const SYN_SNTP *sntp)
{
    if (sntp == NULL || !sntp->synced) {
        return 0U;
    }

    uint32_t sec = 0U;
    uint32_t ns = 0U;
    sntp_compute_time(sntp, &sec, &ns);
    return ns;
}

int32_t syn_sntp_get_drift_ppm(const SYN_SNTP *sntp)
{
    if (!sntp)
        return 0;
    return sntp->drift_ppm;
}

/* ── Non-blocking helpers (for protothread task) ───────────────────────── */

/**
 * @brief Build and send an SNTP request on an already-open socket.
 *
 * @param sntp  SNTP context (for server address).
 * @param sock  Open UDP socket.
 * @return SYN_OK if sent, SYN_ERROR on failure.
 */
static SYN_Status sntp_send_request(const SYN_SNTP *sntp, SYN_Socket sock)
{
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0x23; /* LI=0, VN=4, Mode=3 (client) */

    int n = syn_port_udp_sendto(sock, pkt, sizeof(pkt), &sntp->server);
    return (n == SYN_SNTP_PACKET_SIZE) ? SYN_OK : SYN_ERROR;
}

/**
 * @brief Non-blocking receive: try to read an NTP response.
 *
 * Calls recvfrom with timeout_ms = 0 (non-blocking). If a valid
 * response is available, parses it and sets sntp->synced = true.
 *
 * @param sntp  SNTP context (updated on success).
 * @param sock  Open UDP socket.
 * @return SYN_OK if synced, SYN_BUSY if no data yet, SYN_ERROR on bad response.
 */
static SYN_Status sntp_try_recv(SYN_SNTP *sntp, SYN_Socket sock)
{
    uint8_t pkt[SYN_SNTP_PACKET_SIZE];
    SYN_SockAddr from = {{0U, 0U, 0U, 0U}, 0U};

    int n = syn_port_udp_recvfrom(sock, pkt, sizeof(pkt), &from, 0);
    if (n <= 0 || !sntp_sender_valid(sntp, &from)) {
        return SYN_BUSY;
    }
    if (n < (int)SYN_SNTP_PACKET_SIZE) {
        return SYN_ERROR;
    }

    return sntp_parse_packet(sntp, pkt);
}

/**
 * @brief Clamp sync interval seconds into valid millisecond delay without overflow.
 * @param sync_interval_s Sync interval in seconds.
 * @return Clamped wait duration in milliseconds.
 */
static uint32_t sntp_sync_wait_ms(uint32_t sync_interval_s)
{
    if (sync_interval_s > 2147483U) {
        return 2147483000U;
    }
    return ((sync_interval_s == 0U) ? 1U : sync_interval_s) * 1000U;
}

/* ── Protothread task ───────────────────────────────────────────────────── */

SYN_PT_Status syn_sntp_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_SNTP *sntp = (SYN_SNTP *)task->user_data;

    PT_BEGIN(pt);

    if (sntp->udp_sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(sntp->udp_sock);
        sntp->udp_sock = SYN_SOCKET_INVALID;
    }

    for (;;) {
        syn_backoff_reset(&sntp->backoff);
        sntp->cycle_ok = false;

        while (sntp->backoff.attempts < SYN_SNTP_MAX_RETRIES) {
            /* Phase 1: Open socket */
            sntp->udp_sock = syn_port_udp_open(0);
            if (sntp->udp_sock == SYN_SOCKET_INVALID) {
                PT_TASK_DELAY_MS(pt, task, syn_backoff_next_ms(&sntp->backoff));
                continue;
            }

            /* Phase 2: Send request */
            if (sntp_send_request(sntp, sntp->udp_sock) != SYN_OK) {
                syn_port_sock_close(sntp->udp_sock);
                sntp->udp_sock = SYN_SOCKET_INVALID;
                PT_TASK_DELAY_MS(pt, task, syn_backoff_next_ms(&sntp->backoff));
                continue;
            }

            /* Phase 3: Non-blocking poll with deadline */
            sntp->recv_deadline = syn_port_get_tick_ms() + SYN_SNTP_TIMEOUT_MS;
            PT_WAIT_UNTIL(pt, sntp_try_recv(sntp, sntp->udp_sock) != SYN_BUSY ||
                                  (int32_t)(syn_port_get_tick_ms() - sntp->recv_deadline) >= 0);

            syn_port_sock_close(sntp->udp_sock);
            sntp->udp_sock = SYN_SOCKET_INVALID;

            if (sntp->cycle_ok) {
                break; /* Success! */
            }

            /* Failure or timeout — backoff and try again */
            PT_TASK_DELAY_MS(pt, task, syn_backoff_next_ms(&sntp->backoff));
        }

        /* Wait for next sync interval */
        PT_TASK_DELAY_MS(pt, task, sntp_sync_wait_ms(sntp->sync_interval_s));
    }

    PT_END(pt);
}

#endif /* SYN_USE_SNTP */
