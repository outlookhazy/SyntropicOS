/**
 * @file syn_mavlink.h
 * @brief Zero-Heap MAVLink v2 Protocol Encoder & Streaming Decoder.
 *
 * MAVLink v2 Frame Specification:
 * Header: [0xFD] + [Payload Len 1B] + [Incompat Flags 1B] + [Compat Flags 1B] +
 *         [Seq 1B] + [Sys ID 1B] + [Comp ID 1B] + [Msg ID 3B (Little-Endian)]
 * Payload: [Payload N B]
 * Checksum: [16-bit CRC-X25 over (Header_without_STX + Payload + CRC_Extra)]
 */

#ifndef SYN_MAVLINK_H
#define SYN_MAVLINK_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MAVLINK_STX_V2 0xFD          /**< MAVLink v2 Start of Frame marker byte (0xFD) */
#define SYN_MAVLINK_MAX_PAYLOAD_LEN 255U /**< Maximum MAVLink payload length in bytes */
#define SYN_MAVLINK_HEADER_LEN 10U       /**< MAVLink v2 header length in bytes */

/** MAVLink Message IDs. */
typedef enum {
    SYN_MAVLINK_MSG_HEARTBEAT = 0U,
    SYN_MAVLINK_MSG_SYS_STATUS = 1U,
    SYN_MAVLINK_MSG_ATTITUDE = 30U,
    SYN_MAVLINK_MSG_GLOBAL_POSITION_INT = 33U,
    SYN_MAVLINK_MSG_VFR_HUD = 74U
} SYN_MAVLINK_MsgId;

/** Parsed MAVLink v2 Frame. */
typedef struct {
    uint8_t payload_len;                          /**< Payload length in bytes */
    uint8_t incompat_flags;                       /**< Incompatibility flags byte */
    uint8_t compat_flags;                         /**< Compatibility flags byte */
    uint8_t seq;                                  /**< Sequence number */
    uint8_t sys_id;                               /**< Sender system ID */
    uint8_t comp_id;                              /**< Sender component ID */
    uint32_t msg_id;                              /**< 24-bit Message ID */
    uint8_t payload[SYN_MAVLINK_MAX_PAYLOAD_LEN]; /**< Frame payload bytes */
} SYN_MAVLINK_Frame;

/** MAVLink v2 Streaming Parser Instance. */
typedef struct {
    uint8_t state;                                /**< Internal parser state machine step */
    uint8_t payload_len;                          /**< Expected payload length */
    uint8_t incompat_flags;                       /**< Received incompatibility flags */
    uint8_t compat_flags;                         /**< Received compatibility flags */
    uint8_t seq;                                  /**< Received sequence number */
    uint8_t sys_id;                               /**< Received system ID */
    uint8_t comp_id;                              /**< Received component ID */
    uint32_t msg_id;                              /**< Decoded message ID */
    uint8_t payload_idx;                          /**< Current payload write index */
    uint8_t payload[SYN_MAVLINK_MAX_PAYLOAD_LEN]; /**< Internal payload buffer */
    uint16_t crc;                                 /**< Running CRC accumulator */
    SYN_MAVLINK_Frame last_frame;                 /**< Last successfully parsed frame */
    uint32_t packets_received;                    /**< Total valid packets parsed */
    uint32_t crc_errors;                          /**< Count of CRC mismatch errors */
} SYN_MAVLINK_Parser;

/**
 * @brief Initialize MAVLink v2 parser instance.
 *
 * @param parser Pointer to parser struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_mavlink_init(SYN_MAVLINK_Parser *parser);

/**
 * @brief Process incoming byte from serial stream.
 *
 * @param parser Pointer to parser struct.
 * @param byte   Incoming byte.
 * @param frame  Destination frame struct (populated on valid complete packet).
 * @return SYN_OK on complete valid frame, SYN_BUSY if processing, SYN_ERROR on CRC mismatch.
 */
SYN_Status syn_mavlink_parse_byte(SYN_MAVLINK_Parser *parser, uint8_t byte,
                                  SYN_MAVLINK_Frame *frame);

/**
 * @brief Encode a MAVLink v2 packet into raw byte buffer.
 *
 * @param sys_id     System ID (1..255).
 * @param comp_id    Component ID (1..255).
 * @param seq        Packet sequence number.
 * @param msg_id     Message ID.
 * @param crc_extra  CRC Extra byte for the specific message ID.
 * @param payload    Pointer to payload data buffer (or NULL if 0 len).
 * @param payload_len Payload byte length.
 * @param buf_out    Output byte buffer (must hold at least `payload_len + 12` bytes).
 * @param out_len    Pointer to receive final encoded frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_mavlink_encode_msg(uint8_t sys_id, uint8_t comp_id, uint8_t seq, uint32_t msg_id,
                                  uint8_t crc_extra, const uint8_t *payload, uint8_t payload_len,
                                  uint8_t *buf_out, size_t *out_len);

/**
 * @brief Accumulate single byte into MAVLink X25 CRC.
 *
 * @param byte Input byte to accumulate.
 * @param crc  Initial CRC accumulator value.
 * @return Updated 16-bit CRC value.
 */
uint16_t syn_mavlink_crc_accumulate(uint8_t byte, uint16_t crc);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MAVLINK_H */
