#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SNTP) || SYN_USE_SNTP

/**
 * @file syn_sntp.c
 * @brief SNTP client implementation — RFC 4330 subset.
 */

#include "syn_sntp.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"
#include <string.h>
#include <stdio.h>

/* ── Internal Helpers ───────────────────────────────────────────────────── */

static SYN_Status sntp_send_request(const SYN_SNTP *sntp, SYN_Socket sock);
static SYN_Status sntp_parse_packet(SYN_SNTP *sntp, const uint8_t *pkt, size_t len);

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Read a big-endian uint32 from a byte buffer.
 * @param p Source bytes.
 * @return 32-bit value.
 */
static inline uint32_t load32_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24)
         | ((uint32_t)p[1] << 16)
         | ((uint32_t)p[2] <<  8)
         | ((uint32_t)p[3]);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sntp_init(SYN_SNTP *sntp, const SYN_SockAddr *server,
                   uint32_t sync_interval_s)
{
    SYN_ASSERT(sntp != NULL);
    SYN_ASSERT(server != NULL);

    memset(sntp, 0, sizeof(*sntp));
    sntp->server          = *server;
    sntp->sync_interval_s = sync_interval_s;
    sntp->synced          = false;

    /* Initialize backoff: 1s base, 60s max, factor 2, max_retries */
    syn_backoff_init(&sntp->backoff, 1000, 60000, 2, SYN_SNTP_MAX_RETRIES);
    /* Initialize EMA drift filter: alpha=64 (~0.25 smoothing factor for network jitter rejection) */
    syn_filter_ema_init(&sntp->drift_filter, 64);
    sntp->udp_sock        = SYN_SOCKET_INVALID;
}

/**
 * @brief Parse an SNTP response packet and extract the transmit timestamp.
 * @param sntp  SNTP client instance.
 * @param pkt   Raw UDP payload.
 * @param len   Payload length in bytes.
 * @return SYN_OK on success, SYN_BUSY if packet too short, SYN_ERROR on invalid response.
 */
static SYN_Status sntp_parse_packet(SYN_SNTP *sntp, const uint8_t *pkt, size_t len)
{
    if (len < SYN_SNTP_PACKET_SIZE) return SYN_BUSY;

    /* Validate mode (4=server, 5=broadcast) and stratum != 0 */
    uint8_t mode = pkt[0] & 0x07;
    if (mode != 4 && mode != 5) return SYN_ERROR;
    if (pkt[1] == 0) return SYN_ERROR;  /* kiss-of-death */

    /* Extract transmit timestamp (bytes 40–47, NTP epoch big-endian) */
    uint32_t ntp_s    = load32_be(pkt + 40);
    uint32_t ntp_frac = load32_be(pkt + 44);

    if (ntp_s < SYN_SNTP_EPOCH_OFFSET) return SYN_ERROR;

    uint32_t new_epoch   = ntp_s - SYN_SNTP_EPOCH_OFFSET;
    uint32_t new_tick_ms = syn_port_get_tick_ms();

    /* Calculate clock drift in PPM if we have a previous sync baseline */
    if (sntp->synced && sntp->prev_sync_epoch != 0) {
        uint32_t ntp_elapsed_s   = new_epoch - sntp->prev_sync_epoch;
        uint32_t tick_elapsed_ms = new_tick_ms - sntp->prev_sync_tick_ms;

        if (ntp_elapsed_s >= 5) {
            int64_t expected_ms = (int64_t)ntp_elapsed_s * 1000LL;
            int64_t diff_ms     = (int64_t)tick_elapsed_ms - expected_ms;
            int16_t raw_ppm     = (int16_t)((diff_ms * 1000000LL) / expected_ms);
            
            /* Apply SyntropicOS Exponential Moving Average Filter (SYN_FilterEMA) */
            sntp->drift_ppm     = (int32_t)syn_filter_ema_update(&sntp->drift_filter, raw_ppm);
        }
    }

    sntp->prev_sync_epoch   = new_epoch;
    sntp->prev_sync_tick_ms = new_tick_ms;
    sntp->epoch_s           = new_epoch;
    sntp->epoch_frac        = ntp_frac;
    sntp->sync_tick_ms      = new_tick_ms;
    sntp->synced            = true;

    return SYN_OK;
}

SYN_Status syn_sntp_query(SYN_SNTP *sntp)
{
    SYN_ASSERT(sntp != NULL);

    uint8_t pkt[SYN_SNTP_PACKET_SIZE];
    SYN_SockAddr from;

    SYN_Socket sock = syn_port_udp_open(0);
    if (sock == SYN_SOCKET_INVALID) return SYN_ERROR;

    if (sntp_send_request(sntp, sock) != SYN_OK) {
        syn_port_sock_close(sock);
        return SYN_ERROR;
    }

    int n = syn_port_udp_recvfrom(sock, pkt, sizeof(pkt), &from, SYN_SNTP_TIMEOUT_MS);
    syn_port_sock_close(sock);

    if (n < (int)SYN_SNTP_PACKET_SIZE) return (n <= 0) ? SYN_TIMEOUT : SYN_ERROR;

    return sntp_parse_packet(sntp, pkt, (size_t)n);
}

uint32_t syn_sntp_get_epoch_s(const SYN_SNTP *sntp)
{
    if (!sntp->synced) return 0;

    uint32_t elapsed_ms = syn_port_get_tick_ms() - sntp->sync_tick_ms;
    return sntp->epoch_s + (elapsed_ms / 1000u);
}

uint32_t syn_sntp_get_epoch_ns(const SYN_SNTP *sntp)
{
    if (!sntp->synced) return 0;

    uint32_t elapsed_ms = syn_port_get_tick_ms() - sntp->sync_tick_ms;
    uint32_t sub_s_ms   = elapsed_ms % 1000u;

    return sub_s_ms * 1000000u;  /* ms → ns */
}

int32_t syn_sntp_get_drift_ppm(const SYN_SNTP *sntp)
{
    if (!sntp) return 0;
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
    SYN_SockAddr from;

    int n = syn_port_udp_recvfrom(sock, pkt, sizeof(pkt), &from, 0);
    if (n < (int)SYN_SNTP_PACKET_SIZE) {
        return (n <= 0) ? SYN_BUSY : SYN_ERROR;
    }

    return sntp_parse_packet(sntp, pkt, (size_t)n);
}

/* ── Protothread task ───────────────────────────────────────────────────── */

SYN_PT_Status syn_sntp_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_SNTP *sntp = (SYN_SNTP *)task->user_data;
    SYN_ASSERT(sntp != NULL);

    PT_BEGIN(pt);

    for (;;) {
        syn_backoff_reset(&sntp->backoff);

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
            PT_WAIT_UNTIL(pt,
                sntp_try_recv(sntp, sntp->udp_sock) != SYN_BUSY ||
                (int32_t)(syn_port_get_tick_ms() - sntp->recv_deadline) >= 0);

            syn_port_sock_close(sntp->udp_sock);
            sntp->udp_sock = SYN_SOCKET_INVALID;

            if (sntp->synced) {
                break; /* Success! */
            }

            /* Failure or timeout — backoff and try again */
            PT_TASK_DELAY_MS(pt, task, syn_backoff_next_ms(&sntp->backoff));
        }

        /* Wait for next sync interval */
        PT_TASK_DELAY_MS(pt, task, sntp->sync_interval_s * 1000);
    }

    PT_END(pt);
}

#endif /* SYN_USE_SNTP */
