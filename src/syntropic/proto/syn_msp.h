/**
 * @file syn_msp.h
 * @brief Zero-Heap MSP v1 (MultiWii Serial Protocol v1) Encoder & Decoder.
 *
 * MSP Specifications:
 * - Frame Format: `$M` + [Direction] + [Size] + [Command] + [Payload...] + [Checksum]
 * - Direction: `<` (Inbound Request), `>` (Outbound Response), `!` (Error Response).
 * - Checksum: XOR sum over `Size ^ Command ^ Payload[0] ^ ... ^ Payload[N-1]`.
 * - Common Commands:
 *   - 101: `MSP_STATUS` (Cycle time, I2C errors, sensors active, flight mode flags).
 *   - 102: `MSP_RAW_IMU` (Gyro X/Y/Z, Accel X/Y/Z, Mag X/Y/Z).
 *   - 108: `MSP_ATTITUDE` (Roll 10x deg, Pitch 10x deg, Yaw deg).
 *   - 109: `MSP_ALTITUDE` (Estimated altitude in cm & variometer rate).
 *   - 130: `MSP_BATTERY_STATE` (Cell count, capacity mAh, voltage 0.1V, current 0.01A).
 */

#ifndef SYN_MSP_H
#define SYN_MSP_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MSP_MAX_PAYLOAD 64U /**< Maximum MSP v1 payload byte length */
#define SYN_MSP_HEADER_CHAR '$' /**< MSP preamble start character '$' */
#define SYN_MSP_V1_CHAR 'M'     /**< MSP v1 protocol identifier 'M' */
#define SYN_MSP_REQ_CHAR '<'    /**< MSP direction character for request '<' */
#define SYN_MSP_RESP_CHAR '>'   /**< MSP direction character for response '>' */
#define SYN_MSP_ERR_CHAR '!'    /**< MSP direction character for error '!' */

/** Common MSP Command IDs. */
typedef enum {
    SYN_MSP_API_VERSION = 1U,
    SYN_MSP_FC_VARIANT = 2U,
    SYN_MSP_STATUS = 101U,
    SYN_MSP_RAW_IMU = 102U,
    SYN_MSP_ATTITUDE = 108U,
    SYN_MSP_ALTITUDE = 109U,
    SYN_MSP_BATTERY_STATE = 130U
} SYN_MSP_Cmd;

/** Parsed MSP Frame. */
typedef struct {
    uint8_t dir_char;                     /**< '<', '>', or '!'. */
    uint8_t cmd;                          /**< Command ID (0..255). */
    uint8_t payload_len;                  /**< Payload byte length. */
    uint8_t payload[SYN_MSP_MAX_PAYLOAD]; /**< Payload buffer. */
} SYN_MSP_Frame;

/** MSP Streaming Parser Instance. */
typedef struct {
    uint8_t state;                        /**< Parser state machine step */
    uint8_t dir_char;                     /**< Received direction character */
    uint8_t cmd;                          /**< Received command ID */
    uint8_t payload_len;                  /**< Expected payload length */
    uint8_t payload_idx;                  /**< Payload write index */
    uint8_t payload[SYN_MSP_MAX_PAYLOAD]; /**< Payload assembly buffer */
    uint8_t checksum;                     /**< Running XOR checksum accumulator */
    SYN_MSP_Frame last_frame;             /**< Last decoded frame */
    uint32_t frames_received;             /**< Total valid frames decoded */
    uint32_t checksum_errors;             /**< Count of checksum mismatch errors */
} SYN_MSP_Parser;

/**
 * @brief Initialize MSP parser instance.
 *
 * @param parser Pointer to parser struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_msp_init(SYN_MSP_Parser *parser);

/**
 * @brief Process incoming byte from serial stream.
 *
 * @param parser Pointer to parser struct.
 * @param byte   Incoming serial byte.
 * @param frame  Pointer to destination frame (populated on complete valid frame).
 * @return SYN_OK on complete frame, SYN_BUSY if byte accepted but frame incomplete, SYN_ERROR on
 * checksum failure.
 */
SYN_Status syn_msp_parse_byte(SYN_MSP_Parser *parser, uint8_t byte, SYN_MSP_Frame *frame);

/**
 * @brief Encode an MSP response frame into raw serial bytes buffer.
 *
 * @param cmd      Command ID.
 * @param payload  Pointer to payload buffer (or NULL if 0 length).
 * @param len      Payload length in bytes.
 * @param buf_out  Output buffer (must hold at least `len + 6` bytes).
 * @param buf_size Capacity of buf_out in bytes.
 * @param out_len  Pointer to receive total generated frame length in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_msp_encode_response(uint8_t cmd, const uint8_t *payload, uint8_t len,
                                   uint8_t *buf_out, size_t buf_size, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MSP_H */
