#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ROUTER) || SYN_USE_ROUTER

/**
 * @file syn_router.c
 * @brief Packet router implementation.
 */

#include "syn_router.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Serialization ──────────────────────────────────────────────────────── */

/**
 * @brief Serialize a packet into a wire-format buffer.
 * @param pkt  Packet to serialize.
 * @param buf  [out] Output buffer (header + payload).
 * @return Total bytes written.
 */
static size_t serialize_packet(const SYN_Packet *pkt, uint8_t *buf)
{
    buf[0] = pkt->src;
    buf[1] = pkt->dst;
    buf[2] = pkt->type;
    buf[3] = pkt->seq;
    buf[4] = pkt->flags;
    buf[5] = pkt->len;
    if (pkt->len > 0) {
        memcpy(&buf[SYN_ROUTER_HEADER_SIZE], pkt->payload, pkt->len);
    }
    return SYN_ROUTER_HEADER_SIZE + pkt->len;
}

/**
 * @brief Deserialize a wire-format buffer into a packet.
 * @param buf  Input buffer.
 * @param len  Buffer length.
 * @param pkt  [out] Parsed packet.
 * @return true on success.
 */
static bool deserialize_packet(const uint8_t *buf, size_t len, SYN_Packet *pkt)
{
    if (len < SYN_ROUTER_HEADER_SIZE) return false;

    pkt->src   = buf[0];
    pkt->dst   = buf[1];
    pkt->type  = buf[2];
    pkt->seq   = buf[3];
    pkt->flags = buf[4];
    pkt->len   = buf[5];

    if (pkt->len > SYN_ROUTER_MAX_PAYLOAD) return false;
    if (len < (size_t)SYN_ROUTER_HEADER_SIZE + pkt->len) return false;

    if (pkt->len > 0) {
        memcpy(pkt->payload, &buf[SYN_ROUTER_HEADER_SIZE], pkt->len);
    }
    return true;
}

/* ── ACK helpers ────────────────────────────────────────────────────────── */

/**
 * @brief Send an ACK packet to the source.
 * @param r    Router instance.
 * @param dst  Destination node to acknowledge.
 * @param seq  Sequence number being acknowledged.
 */
static void send_ack(SYN_Router *r, uint8_t dst, uint8_t seq)
{
    SYN_Packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.src   = r->node_id;
    ack.dst   = dst;
    ack.type  = SYN_MSG_ACK;
    ack.seq   = seq;
    ack.flags = SYN_PKT_FLAG_IS_ACK;
    ack.len   = 0;

    uint8_t buf[SYN_ROUTER_HEADER_SIZE];
    size_t n = serialize_packet(&ack, buf);
    syn_transport_send(r->transport, buf, n);
}

/**
 * @brief Handle an incoming ACK by clearing the pending entry.
 * @param r    Router instance.
 * @param src  Source node ID.
 * @param seq  Sequence number.
 */
static void handle_ack(SYN_Router *r, uint8_t src, uint8_t seq)
{
    if (r->pending == NULL) return;

    for (uint8_t i = 0; i < r->pending_cap; i++) {
        SYN_PendingAck *p = &r->pending[i];
        if (p->active && p->dst == src && p->seq == seq) {
            p->active = false;
            return;
        }
    }
}

/**
 * @brief Queue a pending ACK entry for reliable delivery.
 * @param r     Router instance.
 * @param dst   Destination node ID.
 * @param type  Message type.
 * @param seq   Sequence number.
 * @param data  Payload data.
 * @param len   Payload length.
 * @return true if queued successfully.
 */
static bool queue_pending(SYN_Router *r, uint8_t dst, uint8_t type,
                           uint8_t seq, const uint8_t *data, uint8_t len)
{
    if (r->pending == NULL) return false;

    for (uint8_t i = 0; i < r->pending_cap; i++) {
        SYN_PendingAck *p = &r->pending[i];
        if (!p->active) {
            p->dst       = dst;
            p->seq       = seq;
            p->type      = type;
            p->retries   = 0;
            p->sent_tick = syn_port_get_tick_ms();
            p->len       = len;
            if (len > 0) memcpy(p->payload, data, len);
            p->active = true;
            return true;
        }
    }
    return false;  /* pending table full */
}

/**
 * @brief Retry pending packets that have timed out.
 * @param r  Router instance.
 */
static void check_retries(SYN_Router *r)
{
    if (r->pending == NULL) return;

    uint32_t now = syn_port_get_tick_ms();

    for (uint8_t i = 0; i < r->pending_cap; i++) {
        SYN_PendingAck *p = &r->pending[i];
        if (!p->active) continue;

        uint32_t elapsed = now - p->sent_tick;
        if (elapsed < r->ack_timeout_ms) continue;

        if (p->retries >= r->max_retries) {
            p->active = false;
            r->drop_count++;
            continue;
        }

        /* Retransmit */
        SYN_Packet pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.src   = r->node_id;
        pkt.dst   = p->dst;
        pkt.type  = p->type;
        pkt.seq   = p->seq;
        pkt.flags = SYN_PKT_FLAG_ACK_REQ;
        pkt.len   = p->len;
        if (p->len > 0) memcpy(pkt.payload, p->payload, p->len);

        uint8_t buf[SYN_ROUTER_HEADER_SIZE + SYN_ROUTER_MAX_PAYLOAD];
        size_t n = serialize_packet(&pkt, buf);
        syn_transport_send(r->transport, buf, n);

        p->retries++;
        p->sent_tick = now;
    }
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void syn_router_init(SYN_Router *r, uint8_t node_id,
                       SYN_Transport *transport,
                       SYN_RouterHandler *handlers, uint8_t handler_cap)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(transport != NULL);
    SYN_ASSERT(handlers != NULL);

    memset(r, 0, sizeof(*r));
    r->node_id     = node_id;
    r->transport   = transport;
    r->handlers    = handlers;
    r->handler_cap = handler_cap;

    memset(handlers, 0, sizeof(SYN_RouterHandler) * handler_cap);
}

void syn_router_enable_ack(SYN_Router *r, SYN_PendingAck *pending,
                              uint8_t cap, uint16_t timeout_ms,
                              uint8_t max_retries)
{
    SYN_ASSERT(r != NULL);
    r->pending        = pending;
    r->pending_cap    = cap;
    r->ack_timeout_ms = timeout_ms;
    r->max_retries    = max_retries;

    if (pending != NULL) {
        memset(pending, 0, sizeof(SYN_PendingAck) * cap);
    }
}

bool syn_router_register(SYN_Router *r, uint8_t type,
                            SYN_RouterHandlerFn handler, void *ctx)
{
    SYN_ASSERT(r != NULL);

    if (r->handler_count >= r->handler_cap) return false;

    r->handlers[r->handler_count].type    = type;
    r->handlers[r->handler_count].handler = handler;
    r->handlers[r->handler_count].ctx     = ctx;
    r->handler_count++;
    return true;
}

bool syn_router_send(SYN_Router *r, uint8_t dst, uint8_t type,
                        const uint8_t *data, uint8_t len, bool reliable)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(len <= SYN_ROUTER_MAX_PAYLOAD);

    SYN_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.src   = r->node_id;
    pkt.dst   = dst;
    pkt.type  = type;
    pkt.seq   = r->tx_seq++;
    pkt.flags = reliable ? SYN_PKT_FLAG_ACK_REQ : 0;
    pkt.len   = len;
    if (len > 0 && data != NULL) {
        memcpy(pkt.payload, data, len);
    }

    uint8_t buf[SYN_ROUTER_HEADER_SIZE + SYN_ROUTER_MAX_PAYLOAD];
    size_t n = serialize_packet(&pkt, buf);

    bool ok = syn_transport_send(r->transport, buf, n);
    if (ok) {
        r->tx_count++;
        if (reliable && r->pending != NULL) {
            queue_pending(r, dst, type, pkt.seq, data, len);
        }
    }
    return ok;
}

void syn_router_poll(SYN_Router *r)
{
    SYN_ASSERT(r != NULL);

    /* Check for ACK timeouts */
    check_retries(r);

    /* Receive packets */
    uint8_t buf[SYN_ROUTER_HEADER_SIZE + SYN_ROUTER_MAX_PAYLOAD];
    size_t  len = 0;

    while (syn_transport_recv(r->transport, buf, sizeof(buf), &len)) {
        SYN_Packet pkt;
        if (!deserialize_packet(buf, len, &pkt)) {
            r->drop_count++;
            continue;
        }

        /* Not for us? */
        if (pkt.dst != r->node_id && pkt.dst != 0xFF) {
            continue;
        }

        r->rx_count++;

        /* Handle ACK response */
        if (pkt.flags & SYN_PKT_FLAG_IS_ACK) {
            handle_ack(r, pkt.src, pkt.seq);
            continue;
        }

        /* Send ACK if requested */
        if (pkt.flags & SYN_PKT_FLAG_ACK_REQ) {
            send_ack(r, pkt.src, pkt.seq);
        }

        /* Dispatch to handler */
        bool handled = false;
        for (uint8_t i = 0; i < r->handler_count; i++) {
            if (r->handlers[i].type == pkt.type && r->handlers[i].handler) {
                r->handlers[i].handler(&pkt, r->handlers[i].ctx);
                handled = true;
                break;
            }
        }

        if (!handled) {
            r->drop_count++;
        }
    }
}

#endif /* SYN_USE_ROUTER */
