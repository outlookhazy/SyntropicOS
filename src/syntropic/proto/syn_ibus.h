/**
 * @file syn_ibus.h
 * @brief Zero-Heap IBUS (FlySky) 14-Channel Receiver Decoder.
 *
 * IBUS Protocol Specifications:
 * - Baud rate: 115,200 bps, 8N1.
 * - Frame length: 32 bytes.
 * - Header: 0x20 0x40
 * - Channels: 14 16-bit little-endian channels (bytes 2..29, raw 1000..2000 us).
 * - Checksum: 16-bit sum checksum (`0xFFFF - sum(bytes[0..29])`).
 */

#ifndef SYN_IBUS_H
#define SYN_IBUS_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_IBUS_FRAME_SIZE 32U   /**< IBUS frame size in bytes (32) */
#define SYN_IBUS_NUM_CHANNELS 14U /**< Number of RC channels in IBUS frame (14) */
#define SYN_IBUS_HEADER1 0x20U    /**< First header byte of IBUS frame (0x20) */
#define SYN_IBUS_HEADER2 0x40U    /**< Second header byte of IBUS frame (0x40) */

/** Parsed IBUS Frame. */
typedef struct {
    uint16_t channels[SYN_IBUS_NUM_CHANNELS]; /**< 14 16-bit channel values in microseconds
                                                 (1000..2000 us). */
} SYN_IBUS_Frame;

/** IBUS Streaming Parser Instance. */
typedef struct {
    uint8_t buf[SYN_IBUS_FRAME_SIZE]; /**< Frame assembly buffer */
    uint8_t idx;                      /**< Current write index */
    uint32_t frames_received;         /**< Total valid IBUS frames decoded */
    uint32_t checksum_errors;         /**< Count of checksum errors */
    SYN_IBUS_Frame last_frame;        /**< Last successfully decoded IBUS frame */
} SYN_IBUS_Parser;

/**
 * @brief Initialize IBUS parser.
 *
 * @param parser Pointer to parser struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_ibus_init(SYN_IBUS_Parser *parser);

/**
 * @brief Process single byte from serial RX stream.
 *
 * @param parser Pointer to parser struct.
 * @param byte   Incoming byte.
 * @param frame  Pointer to output frame struct (populated when a valid 32-byte frame is decoded).
 * @return SYN_OK on complete frame, SYN_BUSY if byte accepted but incomplete, SYN_ERROR on bad
 * header/checksum.
 */
SYN_Status syn_ibus_parse_byte(SYN_IBUS_Parser *parser, uint8_t byte, SYN_IBUS_Frame *frame);

/**
 * @brief Calculate IBUS 16-bit checksum.
 *
 * @param buf Pointer to 30 data bytes.
 * @return 16-bit expected checksum (`0xFFFF - sum(bytes)`).
 */
uint16_t syn_ibus_calc_checksum(const uint8_t buf[30]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IBUS_H */
