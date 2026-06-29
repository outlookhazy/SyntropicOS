/**
 * @file syn_can.h
 * @brief CAN bus driver abstraction.
 *
 * Port-based CAN driver with frame send/receive and callback dispatch.
 *
 * Usage:
 * @code
 *   static SYN_CAN can;
 *   syn_can_init(&can, 0, 500000);  // CAN0 at 500kbps
 *   syn_can_on_receive(&can, my_rx_handler, NULL);
 *
 *   SYN_CAN_Frame tx = { .id = 0x100, .dlc = 2 };
 *   tx.data[0] = 0x42; tx.data[1] = 0x00;
 *   syn_can_send(&can, &tx);
 *
 *   // In main loop:
 *   syn_can_poll(&can);  // dispatches received frames
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_CAN_H
#define SYN_CAN_H

#include "../common/syn_defs.h"
#include "../port/syn_port_can.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── CAN frame ──────────────────────────────────────────────────────────── */

/** @brief CAN bus frame — standard or extended. */
typedef struct {
    uint32_t id;            /**< 11-bit or 29-bit identifier             */
    uint8_t  data[8];       /**< Frame data                              */
    uint8_t  dlc;           /**< Data length code (0-8)                  */
    bool     extended;      /**< true = 29-bit ID                        */
    bool     rtr;           /**< Remote transmission request             */
} SYN_CAN_Frame;

/* ── Callback ───────────────────────────────────────────────────────────── */

/**
 * @brief CAN receive callback.
 * @param frame  Received frame.
 * @param ctx    User context.
 */
typedef void (*SYN_CAN_Callback)(const SYN_CAN_Frame *frame, void *ctx);

/* ── CAN instance ───────────────────────────────────────────────────────── */

/** @brief CAN bus instance — port, bitrate, callbacks, stats. */
typedef struct {
    uint8_t           port;        /**< CAN port number                   */
    uint32_t          bitrate;     /**< Configured bitrate                */
    SYN_CAN_Callback on_rx;       /**< Receive callback                  */
    void             *on_rx_ctx;   /**< Receive callback context          */
    uint32_t          tx_count;    /**< Frames transmitted                 */
    uint32_t          rx_count;    /**< Frames received                    */
    uint32_t          err_count;   /**< Transmission errors                */
} SYN_CAN;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize CAN driver.
 *
 * @param can      CAN instance.
 * @param port     CAN peripheral port number.
 * @param bitrate  Bitrate in bps (e.g. 500000).
 * @return SYN_OK on success.
 */
SYN_Status syn_can_init(SYN_CAN *can, uint8_t port, uint32_t bitrate);

/**
 * @brief Send a CAN frame.
 * @param can    CAN instance.
 * @param frame  Frame to send.
 * @return true if frame was queued successfully.
 */
bool syn_can_send(SYN_CAN *can, const SYN_CAN_Frame *frame);

/**
 * @brief Poll for received frames and dispatch callback.
 *
 * Call from your main loop.
 *
 * @param can  CAN instance.
 */
void syn_can_poll(SYN_CAN *can);

/**
 * @brief Register receive callback.
 * @param can  CAN instance.
 * @param cb   Callback function.
 * @param ctx  User context.
 */
void syn_can_on_receive(SYN_CAN *can, SYN_CAN_Callback cb, void *ctx);

/**
 * @brief Set hardware acceptance filter.
 * @param can   CAN instance.
 * @param id    Filter ID.
 * @param mask  Filter mask.
 */
void syn_can_set_filter(const SYN_CAN *can, uint32_t id, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CAN_H */
