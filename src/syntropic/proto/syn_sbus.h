/**
 * @file syn_sbus.h
 * @brief Zero-Heap SBUS (Futaba/FrSky) 16-Channel Receiver Decoder.
 *
 * SBUS Protocol Specifications:
 * - Baud rate: 100,000 bps, 8 data bits, Even parity, 2 stop bits (8E2), Inverted logic.
 * - Frame length: 25 bytes.
 * - Header byte: 0x0F
 * - Channels: 16 11-bit channels (packed bitstream across bytes 1..22, raw range 172..1811,
 * 1000..2000 us).
 * - Flags (byte 23): Channel 17 (bit 0), Channel 18 (bit 1), Frame Loss (bit 2), Failsafe (bit 3).
 * - Footer byte: 0x00 (or 0x04 for endbyte flags).
 */

#ifndef SYN_SBUS_H
#define SYN_SBUS_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SBUS_NUM_CHANNELS 16 /**< Number of analog channels in SBUS frame (16) */
#define SYN_SBUS_FRAME_SIZE 25   /**< Total byte length of standard SBUS frame (25) */
#define SYN_SBUS_HEADER 0x0FU    /**< SBUS frame header marker byte (0x0F) */

/** Parsed SBUS Frame structure. */
typedef struct {
    uint16_t channels[SYN_SBUS_NUM_CHANNELS]; /**< 16 11-bit channel values (172..1811 raw). */
    bool ch17;                                /**< Digital Channel 17 flag. */
    bool ch18;                                /**< Digital Channel 18 flag. */
    bool frame_loss;                          /**< True if receiver detected frame loss. */
    bool failsafe;                            /**< True if receiver entered failsafe state. */
} SYN_SBUS_Frame;

/** SBUS Streaming Decoder State Machine Instance. */
typedef struct {
    uint8_t buf[SYN_SBUS_FRAME_SIZE]; /**< Frame assembly buffer */
    uint8_t idx;                      /**< Current byte index in buffer */
    uint32_t frames_received;         /**< Total valid SBUS frames decoded */
    uint32_t frame_loss_count;        /**< Count of frames with frame loss bit set */
    uint32_t failsafe_count;          /**< Count of frames with failsafe bit set */
    SYN_SBUS_Frame last_frame;        /**< Last successfully decoded SBUS frame */
} SYN_SBUS_Parser;

/**
 * @brief Initialize SBUS parser state machine.
 *
 * @param parser Pointer to parser struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_sbus_init(SYN_SBUS_Parser *parser);

/**
 * @brief Process single incoming byte from UART.
 *
 * @param parser Pointer to parser struct.
 * @param byte   Incoming serial byte.
 * @param frame  Pointer to output frame struct (populated when a complete valid 25-byte frame is
 * decoded).
 * @return SYN_OK if complete frame decoded, SYN_BUSY if byte accepted but frame incomplete,
 * SYN_ERROR on header mismatch.
 */
SYN_Status syn_sbus_parse_byte(SYN_SBUS_Parser *parser, uint8_t byte, SYN_SBUS_Frame *frame);

/**
 * @brief Decode raw 25-byte SBUS frame buffer directly.
 *
 * @param buf   Pointer to 25-byte buffer.
 * @param frame Pointer to destination frame struct.
 * @return SYN_OK on successful decode, SYN_INVALID_PARAM or SYN_ERROR on bad header/footer.
 */
SYN_Status syn_sbus_decode_buffer(const uint8_t buf[SYN_SBUS_FRAME_SIZE], SYN_SBUS_Frame *frame);

/**
 * @brief Convert raw 11-bit SBUS channel value to pulse width in microseconds (us).
 *
 * Standard mapping:
 * - 172 raw  -> 1000 us
 * - 992 raw  -> 1500 us
 * - 1811 raw -> 2000 us
 *
 * @param raw_val 11-bit raw SBUS channel value (172..1811).
 * @return Pulse width in microseconds (clamped 1000..2000 us).
 */
uint16_t syn_sbus_raw_to_us(uint16_t raw_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SBUS_H */
