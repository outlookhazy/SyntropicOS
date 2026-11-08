/**
 * @file syn_lintp.h
 * @brief ISO 17987-2 (LIN Transport Layer & Network Layer Services / UDSonLIN).
 *
 * Provides non-blocking, zero-allocation ISO 17987-2 LIN Transport Protocol
 * segmentation and reassembly engine for multi-byte LIN payload transmission
 * using Single Frame (SF), First Frame (FF), and Consecutive Frame (CF).
 *
 * Operates without Flow Control (FC) frames per ISO 17987-2 & ISO 14229-7.
 * @ingroup syn_protocol
 */

#ifndef SYN_LINTP_H
#define SYN_LINTP_H

#include "../common/syn_defs.h"

#include <stdint.h>

/** @cond */
#if !defined(_SSIZE_T_DEFINED_) && !defined(_SSIZE_T_DECLARED) && !defined(__ssize_t_defined) && \
    !defined(_SSIZE_T_) && !defined(_SSIZE_T) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#define _SSIZE_T_DEFINED_
#define _SSIZE_T_DECLARED
#define __ssize_t_defined
#define _SSIZE_T_
#define _SSIZE_T
#define _SSIZE_T_DEFINED
#endif
/** @endcond */

#if !defined(SYN_USE_LINTP) || SYN_USE_LINTP

#ifdef __cplusplus
extern "C" {
#endif

/* ── LIN Diagnostic Addressing Constants ─────────────────────────────────── */

#define SYN_LIN_ID_MASTER_REQ 0x3C /**< Master Request LIN Frame ID */
#define SYN_LIN_ID_SLAVE_RESP 0x3D /**< Slave Response LIN Frame ID */

#define SYN_LINTP_NAD_FUNCTIONAL 0x7E /**< Functional Diagnostic Address */
#define SYN_LINTP_NAD_BROADCAST 0x7F  /**< Broadcast Diagnostic Address */

/* ── LIN TP PCI Frame Types ─────────────────────────────────────────────── */

#define SYN_LINTP_PCI_SF 0x00 /**< Single Frame PCI type (0x0X) */
#define SYN_LINTP_PCI_FF 0x01 /**< First Frame PCI type (0x1X) */
#define SYN_LINTP_PCI_CF 0x02 /**< Consecutive Frame PCI type (0x2X) */

/* ── LIN TP Link State Machine Definitions ───────────────────────────────── */

/**
 * @brief LIN TP Link State Machine State Enumeration.
 */
typedef enum {
    SYN_LINTP_STATE_IDLE = 0,
    SYN_LINTP_STATE_TRANSMITTING_FF,
    SYN_LINTP_STATE_TRANSMITTING_CF,
    SYN_LINTP_STATE_RECEIVING_CF
} SYN_LINTP_State;

/**
 * @brief LIN TP Link Context structure.
 */
typedef struct {
    uint8_t nad;          /**< Configured target/local Node Address (1..0x7D) */
    uint8_t padding_byte; /**< Unused byte fill value (default 0xFF) */

    /* RX State */
    SYN_LINTP_State rx_state; /**< RX state machine state */
    uint8_t *rx_buf;          /**< Pointer to user RX buffer */
    size_t rx_buf_size;       /**< Capacity of user RX buffer */
    size_t rx_total_len;      /**< Expected message payload length */
    size_t rx_offset;         /**< Bytes received so far */
    uint8_t rx_sn;            /**< Next expected sequence number (1..15, mod 16) */
    uint8_t rx_nad;           /**< NAD of active received message */
    bool rx_completed;        /**< True when a complete message is ready to consume */

    /* TX State */
    SYN_LINTP_State tx_state; /**< TX state machine state */
    uint8_t *tx_buf;          /**< Pointer to user TX payload buffer */
    size_t tx_size;           /**< Capacity of user TX buffer */
    size_t tx_total_len;      /**< Total payload size to transmit */
    size_t tx_offset;         /**< Payload bytes transmitted so far */
    uint8_t tx_sn;            /**< Sequence number for next CF (1..15, mod 16) */
    uint8_t tx_nad;           /**< Target NAD for active transmission */
    bool tx_frame_pending;    /**< True when a 8-byte frame is queued for TX */
    uint8_t tx_frame[8];      /**< Queued 8-byte LIN frame payload */

    /* Timers & Timeouts */
    uint32_t timer_n_as_ms; /**< Maximum time for frame TX completion (default 1000ms) */
    uint32_t timer_n_cr_ms; /**< Maximum time between consecutive RX frames (default 1000ms) */
    uint32_t rx_timer_ms;   /**< Elapsed time since last RX frame */
    uint32_t tx_timer_ms;   /**< Elapsed time since last frame queued */
} SYN_LINTP_Link;

/* ── Function Contracts ─────────────────────────────────────────────────── */

/**
 * @brief Initialize LIN TP Link context.
 *
 * @param link Pointer to LIN TP link structure.
 * @param nad Node Address (NAD) for this device (0x01..0x7D).
 * @param rx_buf Pointer to buffer for storing received payloads.
 * @param rx_size Size of RX buffer.
 * @param tx_buf Pointer to buffer reserved for TX (optional context pointer).
 * @param tx_size Size of TX buffer.
 */
void syn_lintp_init(SYN_LINTP_Link *link, uint8_t nad, uint8_t *rx_buf, size_t rx_size,
                    uint8_t *tx_buf, size_t tx_size);

/**
 * @brief Configure LIN TP timeouts in milliseconds.
 *
 * @param link Pointer to LIN TP link context.
 * @param n_as_ms Max frame TX time (default 1000ms).
 * @param n_cr_ms Max time between consecutive RX frames (default 1000ms).
 */
void syn_lintp_set_timeouts(SYN_LINTP_Link *link, uint32_t n_as_ms, uint32_t n_cr_ms);

/**
 * @brief Configure padding byte value for unused frame bytes.
 *
 * @param link Pointer to LIN TP link context.
 * @param pad_byte Fill byte value (default 0xFF).
 */
void syn_lintp_set_padding(SYN_LINTP_Link *link, uint8_t pad_byte);

/**
 * @brief Queue a message for multi-frame or single-frame LIN TP transmission.
 *
 * @param link Pointer to LIN TP link context.
 * @param nad Destination NAD.
 * @param payload Pointer to payload bytes to send.
 * @param len Payload length in bytes (1..4095).
 * @return SYN_OK on success, or error status code.
 */
SYN_Status syn_lintp_send(SYN_LINTP_Link *link, uint8_t nad, const uint8_t *payload, size_t len);

/**
 * @brief Retrieve next queued 8-byte LIN diagnostic payload to transmit on bus (0x3C / 0x3D).
 *
 * @param link Pointer to LIN TP link context.
 * @param out_frame Array of 8 bytes to receive frame data.
 * @return true if a frame was ready and copied, false otherwise.
 */
bool syn_lintp_get_tx_frame(SYN_LINTP_Link *link, uint8_t out_frame[8]);

/**
 * @brief Process an incoming 8-byte LIN diagnostic frame received from bus (0x3C / 0x3D).
 *
 * @param link Pointer to LIN TP link context.
 * @param frame Pointer to 8-byte received LIN frame payload.
 */
void syn_lintp_process_rx_frame(SYN_LINTP_Link *link, const uint8_t frame[8]);

/**
 * @brief Read a fully reassembled message payload from the LIN TP receiver.
 *
 * @param link Pointer to LIN TP link context.
 * @param out_buf Buffer to receive assembled payload.
 * @param max_len Maximum bytes to copy.
 * @return Number of payload bytes read, 0 if no complete message ready, or negative error code.
 */
ssize_t syn_lintp_receive(SYN_LINTP_Link *link, uint8_t *out_buf, size_t max_len);

/**
 * @brief Step LIN TP timers by dt_ms.
 *
 * @param link Pointer to LIN TP link context.
 * @param dt_ms Milliseconds elapsed since last step call.
 */
void syn_lintp_step(SYN_LINTP_Link *link, uint32_t dt_ms);

/**
 * @brief Check if LIN TP TX engine is idle and ready for new message.
 *
 * @param link Pointer to LIN TP link context.
 * @return true if idle, false if actively transmitting a message.
 */
bool syn_lintp_is_tx_idle(const SYN_LINTP_Link *link);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LINTP */
#endif /* SYN_LINTP_H */
