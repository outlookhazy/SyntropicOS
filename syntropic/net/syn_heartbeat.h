/**
 * @file syn_heartbeat.h
 * @brief Heartbeat / keep-alive monitor.
 *
 * Sends periodic keepalive messages and tracks peer liveness.
 * Integrates with the packet router for transport and can log
 * peer-lost events to errlog.
 *
 * Usage:
 * @code
 *   static SYN_HB_Peer peers[4];
 *   static SYN_Heartbeat hb;
 *
 *   syn_heartbeat_init(&hb, &router, peers, 4, 1000, 3000);
 *   syn_heartbeat_add_peer(&hb, 0x02);
 *   syn_heartbeat_add_peer(&hb, 0x03);
 *   syn_heartbeat_on_peer_lost(&hb, my_lost_cb, NULL);
 *
 *   // In main loop:
 *   syn_heartbeat_update(&hb);
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_HEARTBEAT_H
#define SYN_HEARTBEAT_H

#include "../common/syn_defs.h"
#include "syn_router.h"
#include "../system/syn_errlog.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Error codes ────────────────────────────────────────────────────────── */

#define SYN_HB_ERR_PEER_LOST   0x0500  /**< Peer heartbeat timeout.   */
#define SYN_HB_ERR_PEER_FOUND  0x0501  /**< Peer reappeared.          */

/* ── Peer entry ─────────────────────────────────────────────────────────── */

/** @brief Tracked peer entry. */
typedef struct {
    uint8_t   node_id;      /**< Peer's node ID                          */
    uint32_t  last_seen;    /**< Tick of last heartbeat received         */
    bool      alive;        /**< Is peer currently alive?                */
    bool      used;         /**< Is this slot in use?                    */
} SYN_HB_Peer;

/* ── Callbacks ──────────────────────────────────────────────────────────── */

/**
 * @brief Heartbeat event callback.
 * @param node_id  Node that triggered the event.
 * @param ctx      User context.
 */
typedef void (*SYN_HB_Callback)(uint8_t node_id, void *ctx);

/* ── Heartbeat instance ─────────────────────────────────────────────────── */

/** @brief Heartbeat monitor — send keepalives, track peer liveness. */
typedef struct {
    SYN_Router    *router;          /**< Packet router                   */
    SYN_HB_Peer  *peers;           /**< Caller-owned peer array         */
    uint8_t         peer_capacity;   /**< Peer array capacity             */
    uint8_t         peer_count;      /**< Number of tracked peers         */

    uint32_t        interval_ms;     /**< How often to send our heartbeat */
    uint32_t        timeout_ms;      /**< Peer considered dead after this */
    uint32_t        last_tx_tick;    /**< When we last sent               */

    SYN_HB_Callback on_peer_lost;    /**< Called when peer goes dead      */
    SYN_HB_Callback on_peer_found;   /**< Called when peer reappears      */
    void            *cb_ctx;         /**< Context for callbacks           */

    SYN_ErrLog    *errlog;          /**< Optional error logging          */
} SYN_Heartbeat;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize heartbeat system.
 *
 * Automatically registers a handler for SYN_MSG_HEARTBEAT on the router.
 *
 * @param hb          Heartbeat instance.
 * @param router      Packet router.
 * @param peers       Caller-owned peer array.
 * @param peer_cap    Array capacity.
 * @param interval_ms How often to send our heartbeat.
 * @param timeout_ms  How long before a peer is "dead".
 */
void syn_heartbeat_init(SYN_Heartbeat *hb, SYN_Router *router,
                           SYN_HB_Peer *peers, uint8_t peer_cap,
                           uint32_t interval_ms, uint32_t timeout_ms);

/**
 * @brief Add a peer to monitor.
 *
 * @param hb       Heartbeat instance.
 * @param node_id  Peer node ID.
 * @return true if added, false if peer table is full.
 */
bool syn_heartbeat_add_peer(SYN_Heartbeat *hb, uint8_t node_id);

/**
 * @brief Update — send heartbeat + check peers.
 *
 * Call from your main loop.
 *
 * @param hb  Heartbeat instance.
 */
void syn_heartbeat_update(SYN_Heartbeat *hb);

/**
 * @brief Register peer-lost callback.
 * @param hb   Heartbeat instance.
 * @param cb   Callback function.
 * @param ctx  User context.
 */
void syn_heartbeat_on_peer_lost(SYN_Heartbeat *hb,
                                   SYN_HB_Callback cb, void *ctx);

/**
 * @brief Register peer-found callback.
 * @param hb   Heartbeat instance.
 * @param cb   Callback function.
 * @param ctx  User context.
 */
void syn_heartbeat_on_peer_found(SYN_Heartbeat *hb,
                                    SYN_HB_Callback cb, void *ctx);

/**
 * @brief Check if a specific peer is alive.
 * @param hb       Heartbeat instance.
 * @param node_id  Peer node ID.
 * @return true if the peer is alive.
 */
bool syn_heartbeat_peer_alive(const SYN_Heartbeat *hb, uint8_t node_id);

/**
 * @brief Attach error log for peer-lost/found events.
 * @param hb      Heartbeat instance.
 * @param errlog  Error log instance.
 */
void syn_heartbeat_set_errlog(SYN_Heartbeat *hb, SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HEARTBEAT_H */
