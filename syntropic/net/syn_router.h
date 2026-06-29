/**
 * @file syn_router.h
 * @brief Packet router — addressed message dispatch.
 *
 * Routes packets between nodes over any SYN_Transport. Each node has
 * an ID (0-254, 0xFF = broadcast). Messages have a type + payload.
 * Handlers are registered per message type.
 *
 * Supports optional reliable delivery (ACK/retry) per-send.
 *
 * Usage:
 * @code
 *   static SYN_RouterHandler handlers[8];
 *   static SYN_Router router;
 *
 *   syn_router_init(&router, 0x01, &transport,
 *                     handlers, SYN_ARRAY_SIZE(handlers));
 *
 *   syn_router_register(&router, MSG_SENSOR_DATA, on_sensor, NULL);
 *
 *   // Send
 *   uint8_t payload[] = {0x42, 0x00};
 *   syn_router_send(&router, 0x02, MSG_SENSOR_DATA,
 *                     payload, sizeof(payload), false);
 *
 *   // In main loop:
 *   syn_router_poll(&router);
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_ROUTER_H
#define SYN_ROUTER_H

#include "../common/syn_defs.h"
#include "syn_transport.h"
#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Packet format ──────────────────────────────────────────────────────── */

#ifndef SYN_ROUTER_MAX_PAYLOAD
/** @brief Maximum router payload bytes. */
#define SYN_ROUTER_MAX_PAYLOAD  64
#endif

/** Wire format overhead: src(1) + dst(1) + type(1) + seq(1) + flags(1) + len(1) = 6 bytes */
#define SYN_ROUTER_HEADER_SIZE  6

/** @brief Router packet — wire format header + payload buffer. */
typedef struct {
    uint8_t  src;           /**< Source node ID                          */
    uint8_t  dst;           /**< Destination (0xFF = broadcast)          */
    uint8_t  type;          /**< Message type (app-defined)              */
    uint8_t  seq;           /**< Sequence number                         */
    uint8_t  flags;         /**< Packet flags                            */
    uint8_t  len;           /**< Payload length                          */
    uint8_t  payload[SYN_ROUTER_MAX_PAYLOAD]; /**< Payload data          */
} SYN_Packet;

/* Packet flags */
#define SYN_PKT_FLAG_ACK_REQ  0x01  /**< Sender wants an ACK           */
#define SYN_PKT_FLAG_IS_ACK   0x02  /**< This packet is an ACK         */

/* ── Built-in message types ─────────────────────────────────────────────── */

#define SYN_MSG_HEARTBEAT  0x00  /**< Keepalive ping                    */
#define SYN_MSG_ACK        0x01  /**< Acknowledgment                    */
#define SYN_MSG_DISCOVER   0x02  /**< Node discovery                    */

/* ── Handler registration ───────────────────────────────────────────────── */

/**
 * @brief Packet handler callback.
 * @param pkt  Received packet.
 * @param ctx  User context.
 */
typedef void (*SYN_RouterHandlerFn)(const SYN_Packet *pkt, void *ctx);

/** @brief Handler registration entry — message type + callback. */
typedef struct {
    uint8_t              type;     /**< Message type to match             */
    SYN_RouterHandlerFn handler;  /**< Handler function                  */
    void                *ctx;      /**< Handler context                   */
} SYN_RouterHandler;

/* ── Pending ACK tracking ───────────────────────────────────────────────── */

#ifndef SYN_ROUTER_MAX_PENDING
/** @brief Maximum pending ACK entries. */
#define SYN_ROUTER_MAX_PENDING  4
#endif

/** @brief Pending ACK entry for reliable delivery tracking. */
typedef struct {
    uint8_t   dst;         /**< Destination node ID                      */
    uint8_t   seq;         /**< Sequence number                          */
    uint8_t   type;        /**< Message type                             */
    uint8_t   retries;     /**< Retransmission count                     */
    uint32_t  sent_tick;   /**< Tick when last sent                      */
    uint8_t   payload[SYN_ROUTER_MAX_PAYLOAD]; /**< Payload copy         */
    uint8_t   len;         /**< Payload length                           */
    bool      active;      /**< Slot in use                              */
} SYN_PendingAck;

/* ── Router instance ────────────────────────────────────────────────────── */

/** @brief Router instance — node ID, transport, handler table, ACK tracking. */
typedef struct {
    uint8_t            node_id;        /**< Our node ID                   */
    SYN_Transport    *transport;      /**< Underlying transport          */

    /* Handler table */
    SYN_RouterHandler *handlers;      /**< Caller-owned handler array    */
    uint8_t             handler_count; /**< Registered handler count      */
    uint8_t             handler_cap;   /**< Array capacity                */

    /* Sequence counter */
    uint8_t             tx_seq;        /**< Auto-incrementing TX seq      */

    /* Reliable delivery */
    SYN_PendingAck    *pending;       /**< Caller-owned pending ACK array */
    uint8_t             pending_cap;   /**< Array capacity                */
    uint16_t            ack_timeout_ms; /**< ACK timeout in ms            */
    uint8_t             max_retries;   /**< Max retransmissions           */

    /* Stats */
    uint32_t            tx_count;      /**< Total packets sent            */
    uint32_t            rx_count;      /**< Total packets received        */
    uint32_t            drop_count;    /**< Unhandled / bad packets       */
} SYN_Router;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize packet router.
 *
 * @param r          Router instance.
 * @param node_id    This node's ID (0-254).
 * @param transport  Transport to send/receive over.
 * @param handlers   Caller-owned handler array.
 * @param handler_cap Array capacity.
 */
void syn_router_init(SYN_Router *r, uint8_t node_id,
                       SYN_Transport *transport,
                       SYN_RouterHandler *handlers, uint8_t handler_cap);

/**
 * @brief Enable reliable delivery (ACK/retry).
 *
 * @param r          Router.
 * @param pending    Caller-owned pending ACK array.
 * @param cap        Array capacity.
 * @param timeout_ms ACK timeout per attempt.
 * @param max_retries Max retransmissions before giving up.
 */
void syn_router_enable_ack(SYN_Router *r, SYN_PendingAck *pending,
                              uint8_t cap, uint16_t timeout_ms,
                              uint8_t max_retries);

/**
 * @brief Register a handler for a message type.
 *
 * @param r        Router.
 * @param type     Message type to handle.
 * @param handler  Handler function.
 * @param ctx      User context.
 * @return true if registered, false if handler table is full.
 */
bool syn_router_register(SYN_Router *r, uint8_t type,
                            SYN_RouterHandlerFn handler, void *ctx);

/**
 * @brief Send a message.
 *
 * @param r       Router.
 * @param dst     Destination node ID (0xFF = broadcast).
 * @param type    Message type.
 * @param data    Payload.
 * @param len     Payload length.
 * @param reliable If true and ACK is enabled, wait for ACK with retry.
 * @return true if sent (or queued for retry).
 */
bool syn_router_send(SYN_Router *r, uint8_t dst, uint8_t type,
                        const uint8_t *data, uint8_t len, bool reliable);

/**
 * @brief Poll for incoming packets and dispatch handlers.
 *
 * Also checks for ACK timeouts and retransmits if needed.
 * Call from your main loop.
 *
 * @param r  Router.
 */
void syn_router_poll(SYN_Router *r);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ROUTER_H */
