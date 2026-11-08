/**
 * @file syn_transport.h
 * @brief Transport abstraction — pluggable send/receive (header-only).
 *
 * Defines a uniform interface for sending/receiving byte buffers over
 * any physical layer: UART, CAN, SPI, USB, etc. Higher-level modules
 * (router, heartbeat) consume transports without knowing the wire.
 *
 * Usage:
 * @code
 *   // Option A: UART transport via COBS framing
 *   static SYN_Transport tr;
 *   syn_transport_from_uart(&tr, &uart, &cobs_dec, cobs_buf, sizeof(cobs_buf));
 *
 *   // Option B: Custom transport
 *   static SYN_Transport tr = {
 *       .send = my_send_fn,
 *       .recv = my_recv_fn,
 *       .ctx  = &my_device,
 *   };
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_TRANSPORT_H
#define SYN_TRANSPORT_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Transport interface ────────────────────────────────────────────────── */

/** @brief Abstract transport interface (send/receive function pointers). */
typedef struct {
    /**
     * Send a packet.
     * @param data  Packet data.
     * @param len   Packet length.
     * @param ctx   Transport context.
     * @return true if sent successfully.
     */
    bool (*send)(const uint8_t *data, size_t len, void *ctx);

    /**
     * Receive a packet (non-blocking).
     * @param data     Buffer for received data.
     * @param max_len  Buffer capacity.
     * @param out_len  [out] Actual received length.
     * @param ctx      Transport context.
     * @return true if a complete packet was received.
     */
    bool (*recv)(uint8_t *data, size_t max_len, size_t *out_len, void *ctx);

    /**
     * Check if unread data is ready (optional).
     * @param ctx Transport context.
     * @return true if data is available to receive.
     */
    bool (*has_data)(const void *ctx);

    void *ctx; /**< Transport-specific context */
} SYN_Transport;

/* ── Convenience: send/receive via transport ────────────────────────────── */

/**
 * @brief Send a packet via transport.
 * @param t     Transport instance.
 * @param data  Packet data.
 * @param len   Packet length.
 * @return true if sent successfully.
 */
static inline bool syn_transport_send(SYN_Transport *t, const uint8_t *data, size_t len)
{
    if (t == NULL || t->send == NULL)
        return false;
    return t->send(data, len, t->ctx);
}

/**
 * @brief Receive a packet via transport (non-blocking).
 * @param t        Transport instance.
 * @param data     Buffer for received data.
 * @param max_len  Buffer capacity.
 * @param out_len  [out] Actual received length.
 * @return true if a complete packet was received.
 */
static inline bool syn_transport_recv(SYN_Transport *t, uint8_t *data, size_t max_len,
                                      size_t *out_len)
{
    if (t == NULL || t->recv == NULL)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    return t->recv(data, max_len, out_len, t->ctx);
}

/**
 * @brief Check if transport has unread data available (non-blocking).
 * @param t Transport instance.
 * @return true if data is ready or readiness check is unhandled.
 */
static inline bool syn_transport_has_data(const SYN_Transport *t)
{
    if (t == NULL)
        return false;
    if (t->has_data != NULL)
        return t->has_data(t->ctx);
    return true; /* Default if no hook: assume data may be available */
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_H */
