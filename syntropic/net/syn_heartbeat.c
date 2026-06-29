#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HEARTBEAT) || SYN_USE_HEARTBEAT

/**
 * @file syn_heartbeat.c
 * @brief Heartbeat / keep-alive implementation.
 */

#include "syn_heartbeat.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Internal heartbeat handler ─────────────────────────────────────────── */

/**
 * @brief Router callback for incoming heartbeat packets.
 * @param pkt  Received packet.
 * @param ctx  Heartbeat context (SYN_Heartbeat *).
 */
static void hb_rx_handler(const SYN_Packet *pkt, void *ctx)
{
    SYN_Heartbeat *hb = (SYN_Heartbeat *)ctx;
    uint32_t now = syn_port_get_tick_ms();

    /* Find or auto-register this peer */
    for (uint8_t i = 0; i < hb->peer_capacity; i++) {
        SYN_HB_Peer *p = &hb->peers[i];
        if (p->used && p->node_id == pkt->src) {
            bool was_dead = !p->alive;
            p->last_seen = now;
            p->alive     = true;

            if (was_dead) {
                /* Peer came back! */
                if (hb->errlog != NULL) {
                    syn_errlog_record(hb->errlog, SYN_HB_ERR_PEER_FOUND,
                                       SYN_ERR_INFO, (uint32_t)pkt->src);
                }
                if (hb->on_peer_found != NULL) {
                    hb->on_peer_found(pkt->src, hb->cb_ctx);
                }
            }
            return;
        }
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_heartbeat_init(SYN_Heartbeat *hb, SYN_Router *router,
                           SYN_HB_Peer *peers, uint8_t peer_cap,
                           uint32_t interval_ms, uint32_t timeout_ms)
{
    SYN_ASSERT(hb != NULL);
    SYN_ASSERT(router != NULL);
    SYN_ASSERT(peers != NULL);

    memset(hb, 0, sizeof(*hb));
    hb->router        = router;
    hb->peers         = peers;
    hb->peer_capacity = peer_cap;
    hb->interval_ms   = interval_ms;
    hb->timeout_ms    = timeout_ms;
    hb->last_tx_tick  = syn_port_get_tick_ms();

    memset(peers, 0, sizeof(SYN_HB_Peer) * peer_cap);

    /* Register heartbeat handler on the router */
    syn_router_register(router, SYN_MSG_HEARTBEAT, hb_rx_handler, hb);
}

bool syn_heartbeat_add_peer(SYN_Heartbeat *hb, uint8_t node_id)
{
    SYN_ASSERT(hb != NULL);

    /* Check for duplicate */
    for (uint8_t i = 0; i < hb->peer_capacity; i++) {
        if (hb->peers[i].used && hb->peers[i].node_id == node_id) {
            return true;  /* already tracked */
        }
    }

    /* Find empty slot */
    for (uint8_t i = 0; i < hb->peer_capacity; i++) {
        if (!hb->peers[i].used) {
            hb->peers[i].node_id   = node_id;
            hb->peers[i].last_seen = syn_port_get_tick_ms();
            hb->peers[i].alive     = true;
            hb->peers[i].used      = true;
            hb->peer_count++;
            return true;
        }
    }

    return false;  /* table full */
}

void syn_heartbeat_update(SYN_Heartbeat *hb)
{
    SYN_ASSERT(hb != NULL);

    uint32_t now = syn_port_get_tick_ms();

    /* Send our heartbeat (broadcast) */
    if ((now - hb->last_tx_tick) >= hb->interval_ms) {
        syn_router_send(hb->router, 0xFF, SYN_MSG_HEARTBEAT,
                          NULL, 0, false);
        hb->last_tx_tick = now;
    }

    /* Check peer timeouts */
    for (uint8_t i = 0; i < hb->peer_capacity; i++) {
        SYN_HB_Peer *p = &hb->peers[i];
        if (!p->used) continue;

        uint32_t elapsed = now - p->last_seen;
        if (p->alive && elapsed >= hb->timeout_ms) {
            p->alive = false;

            if (hb->errlog != NULL) {
                syn_errlog_record(hb->errlog, SYN_HB_ERR_PEER_LOST,
                                   SYN_ERR_WARNING, (uint32_t)p->node_id);
            }
            if (hb->on_peer_lost != NULL) {
                hb->on_peer_lost(p->node_id, hb->cb_ctx);
            }
        }
    }
}

void syn_heartbeat_on_peer_lost(SYN_Heartbeat *hb,
                                   SYN_HB_Callback cb, void *ctx)
{
    SYN_ASSERT(hb != NULL);
    hb->on_peer_lost = cb;
    hb->cb_ctx       = ctx;
}

void syn_heartbeat_on_peer_found(SYN_Heartbeat *hb,
                                    SYN_HB_Callback cb, void *ctx)
{
    SYN_ASSERT(hb != NULL);
    hb->on_peer_found = cb;
    /* Note: shares cb_ctx with on_peer_lost */
    hb->cb_ctx = ctx;
}

bool syn_heartbeat_peer_alive(const SYN_Heartbeat *hb, uint8_t node_id)
{
    SYN_ASSERT(hb != NULL);

    for (uint8_t i = 0; i < hb->peer_capacity; i++) {
        if (hb->peers[i].used && hb->peers[i].node_id == node_id) {
            return hb->peers[i].alive;
        }
    }
    return false;
}

void syn_heartbeat_set_errlog(SYN_Heartbeat *hb, SYN_ErrLog *errlog)
{
    SYN_ASSERT(hb != NULL);
    hb->errlog = errlog;
}

#endif /* SYN_USE_HEARTBEAT */
