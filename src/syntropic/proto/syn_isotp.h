/**
 * @file syn_isotp.h
 * @brief ISO 15765-2 (DoCAN / ISO-TP) Multi-Frame CAN Transport Protocol.
 *
 * Implements non-blocking, zero-malloc ISO 15765-2 segmentation & reassembly
 * for multi-byte CAN payload transmission with Single Frame (SF),
 * First Frame (FF), Consecutive Frame (CF), and Flow Control (FC) support.
 *
 * Support for CAN FD (64-byte payload frames & ISO 15765-2:2016 extended length)
 * is opt-in via #define SYN_USE_CAN_FD 1.
 */

#ifndef SYN_ISOTP_H
#define SYN_ISOTP_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#if defined(__AVR__) && !defined(_SSIZE_T_DEFINED_)
typedef int ssize_t;
#define _SSIZE_T_DEFINED_
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief ISO-TP Protocol Constants */
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
#define SYN_ISOTP_MAX_PAYLOAD             0xFFFFFFFFU /**< 32-bit extended length (ISO 15765-2:2016) */
#else
#define SYN_ISOTP_MAX_PAYLOAD             4095U       /**< Standard 12-bit max payload (Classic CAN) */
#endif

#define SYN_ISOTP_PCI_SF                  0x00U /**< Single Frame              */
#define SYN_ISOTP_PCI_FF                  0x10U /**< First Frame               */
#define SYN_ISOTP_PCI_CF                  0x20U /**< Consecutive Frame         */
#define SYN_ISOTP_PCI_FC                  0x30U /**< Flow Control Frame        */

/** @brief Flow Control Status (FC) */
#define SYN_ISOTP_FC_CTS                  0x00U /**< Continue To Send          */
#define SYN_ISOTP_FC_WAIT                 0x01U /**< Wait                      */
#define SYN_ISOTP_FC_OVERFLOW             0x02U /**< Buffer Overflow           */

/** @brief ISO-TP Tx States */
typedef enum {
    SYN_ISOTP_TX_IDLE = 0,
    SYN_ISOTP_TX_SEND_SF,
    SYN_ISOTP_TX_SEND_FF,
    SYN_ISOTP_TX_WAIT_FC,
    SYN_ISOTP_TX_SEND_CF,
} SYN_ISOTP_TxState;

/** @brief ISO-TP Rx States */
typedef enum {
    SYN_ISOTP_RX_IDLE = 0,
    SYN_ISOTP_RX_WAIT_CF,
    SYN_ISOTP_RX_COMPLETE,
} SYN_ISOTP_RxState;

/** @brief ISO 15765-2 Link Handle */
typedef struct {
    /* Addressing & Mode */
    uint32_t            rx_id;          /**< Expected CAN Rx ID         */
    uint32_t            tx_id;          /**< Transmit CAN Tx ID         */
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    bool                is_fd;          /**< true = CAN FD mode (64B)   */
#endif

    /* Tx Channel */
    uint8_t            *tx_buf;         /**< Tx payload buffer          */
    size_t              tx_buf_size;    /**< Tx capacity                */
    size_t              tx_len;         /**< Total payload length to tx */
    size_t              tx_offset;      /**< Sent byte counter          */
    uint8_t             tx_seq;         /**< Sequence counter (0..15)   */
    SYN_ISOTP_TxState   tx_state;       /**< Transmit state             */
    uint8_t             tx_bs;          /**< Active block size limit    */
    uint8_t             tx_bs_count;    /**< Sent block frame counter   */
    uint8_t             tx_st_min;      /**< STmin from receiver        */
    uint32_t            tx_st_timer_us; /**< STmin timer (microseconds) */

    /* Rx Channel */
    uint8_t            *rx_buf;         /**< Rx assembly buffer         */
    size_t              rx_buf_size;    /**< Rx capacity                */
    size_t              rx_len;         /**< Assembled byte count       */
    size_t              rx_expected;    /**< Total expected msg length  */
    uint8_t             rx_seq;         /**< Next expected sequence     */
    SYN_ISOTP_RxState   rx_state;       /**< Receive state              */
    bool                rx_fc_pending;  /**< Pending Flow Control frame */
    uint8_t             rx_fc_status;   /**< Flow Control status to tx  */
} SYN_ISOTP_Link;

/**
 * @brief Initialize ISO-TP Link in Classic CAN mode (8-byte frames).
 * @param link         Link handle.
 * @param rx_id        Incoming CAN identifier.
 * @param tx_id        Outgoing CAN identifier.
 * @param rx_buf       Receive assembly buffer.
 * @param rx_buf_size  Receive buffer capacity.
 * @param tx_buf       Transmit buffer.
 * @param tx_buf_size  Transmit buffer capacity.
 */
void syn_isotp_init(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                    uint8_t *rx_buf, size_t rx_buf_size,
                    uint8_t *tx_buf, size_t tx_buf_size);

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
/**
 * @brief Initialize ISO-TP Link with explicit CAN or CAN FD mode.
 * @param link         Link handle.
 * @param rx_id        Incoming CAN identifier.
 * @param tx_id        Outgoing CAN identifier.
 * @param rx_buf       Receive assembly buffer.
 * @param rx_buf_size  Receive buffer capacity.
 * @param tx_buf       Transmit buffer.
 * @param tx_buf_size  Transmit buffer capacity.
 * @param is_fd        true = CAN FD mode (up to 64-byte payload frames).
 */
void syn_isotp_init_fd(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                       uint8_t *rx_buf, size_t rx_buf_size,
                       uint8_t *tx_buf, size_t tx_buf_size, bool is_fd);
#endif

/**
 * @brief Initiate transmission of a multi-byte payload.
 * @param link         Link handle.
 * @param payload      Payload buffer.
 * @param payload_len  Length of payload (1 to max buffer capacity).
 * @return SYN_OK on success, error code on busy/invalid params.
 */
SYN_Status syn_isotp_send(SYN_ISOTP_Link *link, const uint8_t *payload, size_t payload_len);

/**
 * @brief Retrieve next pending outgoing CAN frame from ISO-TP engine.
 * @param link   Link handle.
 * @param frame  Output CAN frame buffer.
 * @return true if a frame was generated, false otherwise.
 */
bool syn_isotp_get_tx_frame(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame);

/**
 * @brief Process an incoming CAN frame into the ISO-TP engine.
 * @param link   Link handle.
 * @param frame  Incoming CAN frame.
 */
void syn_isotp_process_rx_frame(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame);

/**
 * @brief Read completed assembled message from ISO-TP receiver.
 * @param link     Link handle.
 * @param out_buf  Output buffer.
 * @param max_len  Capacity of out_buf.
 * @return Number of bytes copied, or -1 if no message ready.
 */
ssize_t syn_isotp_receive(SYN_ISOTP_Link *link, uint8_t *out_buf, size_t max_len);

/**
 * @brief Step ISO-TP timers (milliseconds).
 * @param link   Link handle.
 * @param dt_ms  Elapsed time in milliseconds.
 */
void syn_isotp_step(SYN_ISOTP_Link *link, uint32_t dt_ms);

/**
 * @brief Step ISO-TP timers (microseconds for fine STmin pacing).
 * @param link   Link handle.
 * @param dt_us  Elapsed time in microseconds.
 */
void syn_isotp_step_us(SYN_ISOTP_Link *link, uint32_t dt_us);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ISOTP_H */
