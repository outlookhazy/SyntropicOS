/**
 * @file syn_mbus.h
 * @brief M-Bus (Meter-Bus, EN 13757-2 / EN 13757-3) Protocol Engine.
 *
 * Implements M-Bus master and slave frame formatting, checksum validation,
 * one-shot encoding/decoding, and streaming byte-by-byte frame reception.
 * Supports all 4 M-Bus EN 13757-2 frame types:
 *   - Single Character ACK (0xE5)
 *   - Short Frame (0x10)
 *   - Control Frame (0x68)
 *   - Long Frame (0x68)
 *
 * @ingroup syn_protocol
 */

#ifndef SYN_MBUS_H
#define SYN_MBUS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MBUS) || SYN_USE_MBUS

#ifdef __cplusplus
extern "C" {
#endif

/* ── M-Bus Delimiters & Constants ────────────────────────────────────────── */

#define SYN_MBUS_START_SHORT 0x10u   /**< Start delimiter for Short frame */
#define SYN_MBUS_START_CONTROL 0x68u /**< Start delimiter for Control frame */
#define SYN_MBUS_START_LONG 0x68u    /**< Start delimiter for Long frame */
#define SYN_MBUS_STOP 0x16u          /**< Stop delimiter for frames */
#define SYN_MBUS_ACK_BYTE 0xE5u      /**< Single character ACK byte */

/* ── Standard M-Bus Control (C) Field Codes ─────────────────────────────── */

#define SYN_MBUS_C_SND_NKE 0x40u     /**< Master -> Slave: Link reset (SND_NKE) */
#define SYN_MBUS_C_SND_UD 0x53u      /**< Master -> Slave: Send user data (FCB=0) */
#define SYN_MBUS_C_SND_UD_FCB 0x73u  /**< Master -> Slave: Send user data (FCB=1) */
#define SYN_MBUS_C_REQ_UD2 0x5Bu     /**< Master -> Slave: Request data class 2 (FCB=0) */
#define SYN_MBUS_C_REQ_UD2_FCB 0x7Bu /**< Master -> Slave: Request data class 2 (FCB=1) */
#define SYN_MBUS_C_REQ_UD1 0x5Au     /**< Master -> Slave: Request data class 1 (FCB=0) */
#define SYN_MBUS_C_RSP_UD 0x08u      /**< Slave -> Master: Response user data */

/* ── Standard M-Bus Control Information (CI) Field Codes ───────────────── */

#define SYN_MBUS_CI_RSP_DATA_LSB 0x72u /**< Response data, 12-byte header (LSB first) */
#define SYN_MBUS_CI_RSP_DATA_MSB 0x73u /**< Response data, 12-byte header (MSB first) */
#define SYN_MBUS_CI_SND_UD_LSB 0x51u   /**< Send data, 12-byte header */
#define SYN_MBUS_CI_SELECT_SLAVE 0x52u /**< Select slave (secondary address) */

/* ── M-Bus Special Addresses ────────────────────────────────────────────── */

#define SYN_MBUS_ADDR_BROADCAST_REPLY 0xFEu /**< Broadcast with reply */
#define SYN_MBUS_ADDR_BROADCAST_NONE 0xFFu  /**< Broadcast without reply */

/* ── Limits ─────────────────────────────────────────────────────────────── */

#define SYN_MBUS_MAX_PAYLOAD 252u /**< Max user payload size in bytes */
#define SYN_MBUS_MAX_FRAME_LEN \
    261u /**< Max total raw frame size (SOF+L+L+SOF+C+A+CI+Data+CHK+EOF) */

/* ── M-Bus Frame Types ─────────────────────────────────────────────────── */

/** M-Bus Frame Format Types Enum. */
typedef enum {
    SYN_MBUS_FRAME_TYPE_UNKNOWN = 0, /**< Unrecognized frame type */
    SYN_MBUS_FRAME_TYPE_SINGLE_ACK,  /**< Single byte 0xE5 ACK */
    SYN_MBUS_FRAME_TYPE_SHORT,       /**< Short frame (5 bytes) */
    SYN_MBUS_FRAME_TYPE_CONTROL,     /**< Control frame (9 bytes) */
    SYN_MBUS_FRAME_TYPE_LONG,        /**< Long frame (N >= 9 bytes) */
} SYN_MBUS_FrameType;

/* ── M-Bus Decoded Frame Structure ──────────────────────────────────────── */

/** @brief M-Bus decoded frame structure. */
typedef struct {
    SYN_MBUS_FrameType type;               /**< Frame format classification */
    uint8_t c_field;                       /**< Control field byte */
    uint8_t a_field;                       /**< Address field byte (0-250, 0xFE, 0xFF) */
    uint8_t ci_field;                      /**< Control Information field (Control & Long frames) */
    uint8_t payload[SYN_MBUS_MAX_PAYLOAD]; /**< Payload data buffer */
    uint8_t payload_len;                   /**< Length of payload data */
    uint8_t checksum;                      /**< Received checksum byte */
    bool checksum_valid;                   /**< True if calculated checksum matches received */
} SYN_MBUS_Frame;

/* ── Streaming Decoder Callback ─────────────────────────────────────────── */

/**
 * @brief Callback invoked when a complete, valid M-Bus frame is received.
 *
 * @param frame Pointer to decoded frame structure.
 * @param ctx   User context.
 */
typedef void (*SYN_MBUS_FrameCallback)(const SYN_MBUS_Frame *frame, void *ctx);

/** @brief M-Bus streaming state machine decoder. */
typedef struct {
    uint8_t rx_buf[SYN_MBUS_MAX_FRAME_LEN]; /**< Internal frame receive buffer */
    size_t rx_idx;                          /**< Buffer write index */
    size_t expected_len;                    /**< Expected total frame length */
    uint8_t state;                          /**< Decoder state machine state */
    SYN_MBUS_FrameCallback callback;        /**< Frame completion callback */
    void *ctx;                              /**< Callback user context */
} SYN_MBUS_Decoder;

/* ── API Function Declarations ───────────────────────────────────────────── */

/**
 * @brief Calculate M-Bus 8-bit checksum (modulo-256 sum over specified buffer).
 *
 * @param data Pointer to buffer.
 * @param len  Length of data in bytes.
 * @return Calculated checksum byte.
 */
uint8_t syn_mbus_calc_checksum(const uint8_t *data, size_t len);

/**
 * @brief Encode a Single Character ACK frame (0xE5).
 *
 * @param buf      Output buffer.
 * @param cap      Buffer capacity (must be >= 1).
 * @param out_len  Receives total bytes written (1).
 * @return SYN_OK on success, or error status.
 */
SYN_Status syn_mbus_encode_ack(uint8_t *buf, size_t cap, size_t *out_len);

/**
 * @brief Encode a Short Frame (5 bytes: 0x10 | C | A | Checksum | 0x16).
 *
 * @param c_field  Control field byte.
 * @param a_field  Address field byte.
 * @param buf      Output buffer.
 * @param cap      Buffer capacity (must be >= 5).
 * @param out_len  Receives total bytes written (5).
 * @return SYN_OK on success, or error status.
 */
SYN_Status syn_mbus_encode_short(uint8_t c_field, uint8_t a_field, uint8_t *buf, size_t cap,
                                 size_t *out_len);

/**
 * @brief Encode a Control Frame (9 bytes: 0x68 | 0x03 | 0x03 | 0x68 | C | A | CI | Checksum |
 * 0x16).
 *
 * @param c_field  Control field byte.
 * @param a_field  Address field byte.
 * @param ci_field Control Information field byte.
 * @param buf      Output buffer.
 * @param cap      Buffer capacity (must be >= 9).
 * @param out_len  Receives total bytes written (9).
 * @return SYN_OK on success, or error status.
 */
SYN_Status syn_mbus_encode_control(uint8_t c_field, uint8_t a_field, uint8_t ci_field, uint8_t *buf,
                                   size_t cap, size_t *out_len);

/**
 * @brief Encode a Long Frame (0x68 | L | L | 0x68 | C | A | CI | Payload... | Checksum | 0x16).
 *
 * @param c_field     Control field byte.
 * @param a_field     Address field byte.
 * @param ci_field    Control Information field byte.
 * @param payload     Pointer to payload data buffer.
 * @param payload_len Payload length (max 252 bytes).
 * @param buf         Output buffer.
 * @param cap         Buffer capacity (must be >= payload_len + 9).
 * @param out_len     Receives total bytes written.
 * @return SYN_OK on success, or error status.
 */
SYN_Status syn_mbus_encode_long(uint8_t c_field, uint8_t a_field, uint8_t ci_field,
                                const uint8_t *payload, uint8_t payload_len, uint8_t *buf,
                                size_t cap, size_t *out_len);

/**
 * @brief Decode a raw M-Bus byte buffer into a frame structure.
 *
 * @param buf Input byte buffer containing a complete M-Bus frame.
 * @param len Buffer length in bytes.
 * @param frame Destination frame structure.
 * @return SYN_OK on success, SYN_ERROR on checksum or framing failure, or SYN_INVALID_PARAM.
 */
SYN_Status syn_mbus_decode_frame(const uint8_t *buf, size_t len, SYN_MBUS_Frame *frame);

/**
 * @brief Initialize an M-Bus streaming state machine decoder.
 *
 * @param dec      Decoder instance.
 * @param callback Callback called upon successful frame completion.
 * @param ctx      User context passed to callback.
 */
void syn_mbus_decoder_init(SYN_MBUS_Decoder *dec, SYN_MBUS_FrameCallback callback, void *ctx);

/**
 * @brief Reset the streaming decoder state.
 *
 * @param dec Decoder instance.
 */
void syn_mbus_decoder_reset(SYN_MBUS_Decoder *dec);

/**
 * @brief Feed a single byte to the streaming M-Bus decoder.
 *
 * @param dec Decoder instance.
 * @param byte Byte received from UART/RS232/transceiver.
 */
void syn_mbus_decoder_feed(SYN_MBUS_Decoder *dec, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MBUS */
#endif /* SYN_MBUS_H */
