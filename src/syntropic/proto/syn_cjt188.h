/**
 * @file syn_cjt188.h
 * @brief CJ/T 188-2004 / CJ/T 188-2018 Smart Metering Protocol.
 *
 * Implements non-blocking, zero-malloc CJ/T 188 protocol framing, data encoding,
 * valve control commands, 8-bit checksum verification, and streaming UART decoding
 * for cold water, hot water, heat, and gas meters.
 */

#ifndef SYN_CJT188_H
#define SYN_CJT188_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Meter Types (T) ────────────────────────────────────────────────────── */

#define SYN_CJT188_METER_COLD_WATER 0x10U      /**< Cold water meter           */
#define SYN_CJT188_METER_HOT_WATER 0x11U       /**< Domestic hot water meter    */
#define SYN_CJT188_METER_DRINK_WATER 0x12U     /**< Direct drinking water meter */
#define SYN_CJT188_METER_RECLAIMED_WATER 0x13U /**< Reclaimed water meter       */
#define SYN_CJT188_METER_HEAT 0x20U            /**< Heat / caloric meter (heat) */
#define SYN_CJT188_METER_COOLING 0x21U         /**< Heat meter (cooling)        */
#define SYN_CJT188_METER_GAS 0x30U             /**< Gas meter                   */
#define SYN_CJT188_METER_POWER 0x40U           /**< Power meter                 */

/* ── Control Codes (C) ───────────────────────────────────────────────────── */

#define SYN_CJT188_CTRL_READ_DATA 0x01U       /**< Read meter data request     */
#define SYN_CJT188_CTRL_READ_DATA_RESP 0x81U  /**< Read meter data response    */
#define SYN_CJT188_CTRL_WRITE_DATA 0x04U      /**< Write data / control req    */
#define SYN_CJT188_CTRL_WRITE_DATA_RESP 0x84U /**< Write data / control resp   */
#define SYN_CJT188_CTRL_WRITE_ADDR 0x15U      /**< Write meter address request */
#define SYN_CJT188_CTRL_WRITE_ADDR_RESP 0x95U /**< Write meter address resp    */

/* ── Data Identifiers (DI) ──────────────────────────────────────────────── */

#define SYN_CJT188_DI_READ_METER_DATA 0x901FU /**< Read meter current data    */
#define SYN_CJT188_DI_READ_HIST_DATA 0xD120U  /**< Read historical data       */
#define SYN_CJT188_DI_VALVE_CONTROL 0xA017U   /**< Valve control command      */

/* ── Valve Control Function Codes ───────────────────────────────────────── */

#define SYN_CJT188_VALVE_OPEN 0x55U  /**< Open valve command          */
#define SYN_CJT188_VALVE_CLOSE 0x99U /**< Close valve command         */

/* ── Framing Delimiters ─────────────────────────────────────────────────── */

#define SYN_CJT188_PREAMBLE_BYTE 0xFEU /**< Leading preamble byte       */
#define SYN_CJT188_START_BYTE 0x68U    /**< Frame start delimiter       */
#define SYN_CJT188_END_BYTE 0x16U      /**< Frame end delimiter         */
#define SYN_CJT188_MIN_FRAME_SIZE 13U  /**< Min frame without preamble  */
#define SYN_CJT188_MAX_FRAME_SIZE 128U /**< Max supported frame length  */

/* ── Data Structures ────────────────────────────────────────────────────── */

/**
 * @brief Decoded CJ/T 188 Protocol Frame.
 */
typedef struct {
    uint8_t meter_type;     /**< Meter type (0x10 cold water, etc.)      */
    uint8_t meter_id[5];    /**< 5-byte BCD meter ID (reverse order)     */
    uint8_t vendor_id[2];   /**< 2-byte manufacturer code                */
    uint8_t ctrl;           /**< Control code (0x01, 0x81, 0x04, etc.)   */
    uint8_t len;            /**< Data field length                       */
    uint16_t data_id;       /**< 16-bit Data ID (e.g. 0x901F)            */
    uint8_t seq;            /**< Sequence number                         */
    const uint8_t *payload; /**< Pointer to data payload inside frame   */
    size_t payload_len;     /**< Length of payload (len - 3)             */
} SYN_CJT188_Frame;

/**
 * @brief Parsed CJ/T 188 Meter Response Data.
 */
typedef struct {
    uint32_t current_flow_bcd; /**< Current cumulative flow (8-digit BCD) */
    uint8_t unit;              /**< Unit code (e.g., 0x2C for m³)         */
    uint32_t month_flow_bcd;   /**< Month usage flow (8-digit BCD)        */
    uint16_t status;           /**< 2-byte meter status flags             */
} SYN_CJT188_MeterData;

/**
 * @brief Streaming Decoder State Machine for CJ/T 188.
 */
typedef struct {
    uint8_t buf[SYN_CJT188_MAX_FRAME_SIZE]; /**< Frame assembly buffer */
    size_t index;                           /**< Current byte index in buffer */
    size_t expected_len;                    /**< Total expected frame byte length */
    bool in_frame;                          /**< True if currently receiving active frame */
} SYN_CJT188_Decoder;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Calculate CJ/T 188 8-bit checksum.
 *
 * @param buf  Pointer to frame starting at 0x68.
 * @param len  Length of frame from 0x68 up to payload end (excluding CS and 0x16).
 * @return 8-bit sum modulo 256.
 */
uint8_t syn_cjt188_checksum(const uint8_t *buf, size_t len);

/**
 * @brief Encode a CJ/T 188 Read Meter Data Request (`0x01`).
 *
 * @param out_buf     Buffer to write encoded frame into.
 * @param buf_size    Capacity of out_buf (at least 20 bytes for 4x FE + frame).
 * @param meter_type  Meter type (e.g. SYN_CJT188_METER_COLD_WATER).
 * @param meter_id    5-byte BCD meter ID array.
 * @param vendor_id   2-byte vendor code array.
 * @param data_id     Data ID (e.g. SYN_CJT188_DI_READ_METER_DATA).
 * @param seq         Sequence number.
 * @return Total bytes written to out_buf, or 0 if buffer too small.
 */
size_t syn_cjt188_encode_read_req(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                  const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                  uint16_t data_id, uint8_t seq);

/**
 * @brief Encode a CJ/T 188 Valve Control Request (`0x04`).
 *
 * @param out_buf     Buffer to write encoded frame into.
 * @param buf_size    Capacity of out_buf (at least 21 bytes).
 * @param meter_type  Meter type.
 * @param meter_id    5-byte BCD meter ID array.
 * @param vendor_id   2-byte vendor code array.
 * @param open_valve  true for OPEN (0x55), false for CLOSE (0x99).
 * @param seq         Sequence number.
 * @return Total bytes written, or 0 if buffer too small.
 */
size_t syn_cjt188_encode_valve_ctrl(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                    const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                    bool open_valve, uint8_t seq);

/**
 * @brief Parse a raw CJ/T 188 binary frame.
 *
 * @param buf        Pointer to frame buffer (may begin with optional preamble `0xFE`).
 * @param len        Length of raw buffer.
 * @param out_frame  Pointer to output frame structure.
 * @return true if valid frame parsed, false otherwise.
 */
bool syn_cjt188_parse_frame(const uint8_t *buf, size_t len, SYN_CJT188_Frame *out_frame);

/**
 * @brief Initialize a CJ/T 188 streaming decoder instance.
 * @param decoder Pointer to decoder instance.
 */
void syn_cjt188_decoder_init(SYN_CJT188_Decoder *decoder);

/**
 * @brief Feed a single byte into the streaming decoder.
 *
 * @param decoder    Decoder instance.
 * @param byte       Received byte from UART/RS-485.
 * @param out_frame  Output frame structure populated when a complete frame arrives.
 * @return true if a complete valid frame was decoded, false otherwise.
 */
bool syn_cjt188_decoder_feed(SYN_CJT188_Decoder *decoder, uint8_t byte,
                             SYN_CJT188_Frame *out_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CJT188_H */
