/**
 * @file syn_ymodem.h
 * @brief YMODEM / XMODEM-1K Serial File Transfer Receiver Protocol.
 * @ingroup syn_proto
 *
 * Implements a lightweight, zero-allocation YMODEM (and XMODEM-1K compatible)
 * batch file transfer receiver state machine for microcontroller bootloaders.
 * Supports Block 0 metadata (filename, file size parsing), 1024-byte (STX)
 * and 128-byte (SOH) blocks, CRC-16/XMODEM error checking, and cancellation.
 *
 * @par Usage
 * @code
 *   SYN_YMODEM_Receiver rx;
 *   syn_ymodem_receiver_init(&rx, my_putchar, my_getchar, my_event_cb, user_ctx);
 *   SYN_YMODEM_Status status = syn_ymodem_receive(&rx);
 * @endcode
 */

#ifndef SYN_YMODEM_H
#define SYN_YMODEM_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name YMODEM Control Characters */
/**@{*/
#define SYN_YMODEM_SOH 0x01U /**< Start of 128-byte data block */
#define SYN_YMODEM_STX 0x02U /**< Start of 1024-byte data block */
#define SYN_YMODEM_EOT 0x04U /**< End of transmission */
#define SYN_YMODEM_ACK 0x06U /**< Acknowledge */
#define SYN_YMODEM_NAK 0x15U /**< Negative acknowledge */
#define SYN_YMODEM_CAN 0x18U /**< Cancel transmission */
#define SYN_YMODEM_CRC 0x43U /**< ASCII 'C' to request CRC-16 mode */
/**@}*/

/** @name Configuration Defaults */
/**@{*/
#ifndef SYN_YMODEM_MAX_FILENAME
#define SYN_YMODEM_MAX_FILENAME 64U /**< Maximum filename string buffer size */
#endif

#ifndef SYN_YMODEM_MAX_BLOCK_SIZE
#define SYN_YMODEM_MAX_BLOCK_SIZE 1024U /**< Maximum packet block size (1024 B STX) */
#endif

#ifndef SYN_YMODEM_MAX_RETRIES
#define SYN_YMODEM_MAX_RETRIES 10U /**< Maximum packet NAK retry attempts */
#endif
/**@}*/

/** @brief YMODEM Session Status Codes */
typedef enum {
    SYN_YMODEM_OK = 0,            /**< Transfer completed successfully */
    SYN_YMODEM_ERR_TIMEOUT = -1,  /**< Receive timeout exceeded */
    SYN_YMODEM_ERR_CRC = -2,      /**< Unrecoverable CRC error / max retries exceeded */
    SYN_YMODEM_ERR_CANCEL = -3,   /**< Transfer cancelled by sender or receiver */
    SYN_YMODEM_ERR_SEQUENCE = -4, /**< Out of order sequence number */
    SYN_YMODEM_ERR_OVERFLOW = -5, /**< Packet buffer overflow */
    SYN_YMODEM_ERR_PARAM = -6     /**< Invalid parameter or callback failure */
} SYN_YMODEM_Status;

/** @brief YMODEM Event Types passed to callback */
typedef enum {
    SYN_YMODEM_EVENT_FILE_START = 0, /**< Block 0 received; file header parsed */
    SYN_YMODEM_EVENT_DATA = 1,       /**< Data block received */
    SYN_YMODEM_EVENT_FILE_END = 2,   /**< EOT received; file completed */
    SYN_YMODEM_EVENT_SESSION_END = 3 /**< Empty Block 0 received; session ended */
} SYN_YMODEM_Event;

/**
 * @brief Write a byte to the serial link (UART TX).
 * @param byte Data byte to send.
 * @param ctx  User context.
 */
typedef void (*SYN_YMODEM_PutChar)(uint8_t byte, void *ctx);

/**
 * @brief Read a byte from the serial link (UART RX).
 * @param timeout_ms Read timeout in milliseconds.
 * @param ctx        User context.
 * @return Byte value (0-255) on success, or negative value (<0) on timeout/error.
 */
typedef int (*SYN_YMODEM_GetChar)(uint32_t timeout_ms, void *ctx);

/**
 * @brief Application callback for YMODEM transfer events.
 * @param event Event type.
 * @param data  Payload buffer (filename for FILE_START, block payload for DATA).
 * @param len   Payload size in bytes.
 * @param ctx   User context.
 * @return 0 on success, non-zero to cancel transfer.
 */
typedef int (*SYN_YMODEM_EventCallback)(SYN_YMODEM_Event event, const uint8_t *data, size_t len,
                                        void *ctx);

/** @brief YMODEM Receiver Context */
typedef struct {
    SYN_YMODEM_PutChar putchar_fn;                   /**< Serial TX function */
    SYN_YMODEM_GetChar getchar_fn;                   /**< Serial RX function */
    SYN_YMODEM_EventCallback event_fn;               /**< Event callback */
    void *ctx;                                       /**< User context pointer */
    uint8_t pkt_buf[SYN_YMODEM_MAX_BLOCK_SIZE + 5U]; /**< Packet RX buffer */
    uint8_t expected_seq;                            /**< Next expected block sequence number */
    char filename[SYN_YMODEM_MAX_FILENAME];          /**< Parsed filename from Block 0 */
    uint32_t filesize;                               /**< Parsed file size from Block 0 */
    uint32_t bytes_received;                         /**< Accumulated payload bytes received */
    bool has_file_size;                              /**< True if Block 0 specified file size */
} SYN_YMODEM_Receiver;

/**
 * @brief Initialize a YMODEM receiver instance.
 *
 * @param rx          Receiver context.
 * @param putchar_fn  Serial TX function.
 * @param getchar_fn  Serial RX function with timeout.
 * @param event_fn    Event callback handler.
 * @param ctx         User context pointer.
 */
void syn_ymodem_receiver_init(SYN_YMODEM_Receiver *rx, SYN_YMODEM_PutChar putchar_fn,
                              SYN_YMODEM_GetChar getchar_fn, SYN_YMODEM_EventCallback event_fn,
                              void *ctx);

/**
 * @brief Execute a YMODEM receive session.
 *
 * Runs the YMODEM handshake, processes Block 0 header, accepts data packets,
 * verifies CRCs, and invokes event callbacks until all files in the batch
 * are transferred or an unrecoverable error occurs.
 *
 * @param rx  Receiver context.
 * @return SYN_YMODEM_OK on successful session completion, or error code (<0).
 */
SYN_YMODEM_Status syn_ymodem_receive(SYN_YMODEM_Receiver *rx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_YMODEM_H */
