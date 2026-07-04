/**
 * @file syn_sntp.h
 * @brief SNTP client — lightweight time synchronization over UDP.
 *
 * Implements a Simple NTP (RFC 4330) client that queries a single
 * NTP server and maintains a local epoch clock offset. The client
 * runs as a cooperative protothread task, periodically re-syncing.
 *
 * After the first successful sync, syn_sntp_get_epoch_s() returns
 * a real UTC epoch timestamp derived from the last NTP response
 * plus local tick elapsed since then.
 *
 * Used by the WireGuard module for TAI64N handshake timestamps.
 *
 * @par Usage
 * @code
 *   static SYN_SNTP sntp;
 *   static SYN_SockAddr ntp_server = { .ip = {216,239,35,0}, .port = 123 };
 *
 *   syn_sntp_init(&sntp, &ntp_server, 3600);  // re-sync every hour
 *
 *   // Register as a scheduler task:
 *   syn_task_create(&tasks[0], "sntp", syn_sntp_task, 2, &sntp);
 *
 *   // Later, read the time:
 *   if (syn_sntp_is_synced(&sntp)) {
 *       uint32_t now = syn_sntp_get_epoch_s(&sntp);
 *   }
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_SNTP_H
#define SYN_SNTP_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SNTP) || SYN_USE_SNTP

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "../util/syn_backoff.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

/** NTP packet size (both request and response). */
#define SYN_SNTP_PACKET_SIZE    48

/** NTP epoch offset: seconds between 1900-01-01 and 1970-01-01. */
#define SYN_SNTP_EPOCH_OFFSET   2208988800UL

/** Default receive timeout for NTP response (ms). */
#ifndef SYN_SNTP_TIMEOUT_MS
#define SYN_SNTP_TIMEOUT_MS     3000
#endif

/** Maximum retry attempts per sync cycle. */
#ifndef SYN_SNTP_MAX_RETRIES
#define SYN_SNTP_MAX_RETRIES    3
#endif

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief SNTP client context — caller-owned.
 */
typedef struct {
    SYN_SockAddr server;            /**< NTP server address (IP + port)     */
    SYN_Socket   udp_sock;          /**< UDP socket handle                  */

    uint32_t     epoch_s;           /**< Last synced UTC epoch (seconds)    */
    uint32_t     epoch_frac;        /**< Fractional seconds (NTP format)    */
    uint32_t     sync_tick_ms;      /**< Local tick at time of last sync    */
    uint32_t     sync_interval_s;   /**< Re-sync interval in seconds        */

    bool         synced;            /**< true after first successful sync   */
    uint32_t     recv_deadline;     /**< Tick deadline for non-blocking recv */
    SYN_Backoff  backoff;           /**< Retry backoff state                */
} SYN_SNTP;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the SNTP client.
 *
 * @param sntp            Client context.
 * @param server          NTP server address (typically port 123).
 * @param sync_interval_s Seconds between re-syncs (e.g. 3600 for hourly).
 */
void syn_sntp_init(SYN_SNTP *sntp, const SYN_SockAddr *server,
                   uint32_t sync_interval_s);

/**
 * @brief Perform a single blocking NTP query.
 *
 * Opens a UDP socket, sends a request, waits for the response, and
 * updates the internal epoch. Closes the socket when done.
 *
 * @param sntp  Client context.
 * @return SYN_OK on success, SYN_TIMEOUT or SYN_ERROR on failure.
 */
SYN_Status syn_sntp_query(SYN_SNTP *sntp);

/**
 * @brief Cooperative protothread task for periodic NTP sync.
 *
 * Syncs on startup, then re-syncs every sync_interval_s seconds.
 * Pass the SYN_SNTP context via task->user_data.
 *
 * @param pt   Protothread.
 * @param task Task descriptor.
 * @return PT status (PT_WAITING, PT_YIELDED, or PT_ENDED).
 */
SYN_PT_Status syn_sntp_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Get current UTC epoch in seconds.
 *
 * Computed as: last_sync_epoch + (current_tick - sync_tick) / 1000.
 * Returns 0 if not yet synced.
 *
 * @param sntp  Client context.
 * @return Current epoch seconds, or 0 if unsynced.
 */
uint32_t syn_sntp_get_epoch_s(const SYN_SNTP *sntp);

/**
 * @brief Get current nanosecond component of the timestamp.
 *
 * Derived from the tick counter sub-second remainder + NTP fractional
 * seconds from the last sync. Approximate — not sub-ms accurate.
 *
 * @param sntp  Client context.
 * @return Nanoseconds (0–999999999), or 0 if unsynced.
 */
uint32_t syn_sntp_get_epoch_ns(const SYN_SNTP *sntp);

/**
 * @brief Check if the client has successfully synced at least once.
 *
 * @param sntp  Client context.
 * @return true if synced.
 */
static inline bool syn_sntp_is_synced(const SYN_SNTP *sntp)
{
    return sntp->synced;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SNTP */

#endif /* SYN_SNTP_H */
