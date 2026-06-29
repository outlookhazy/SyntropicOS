/**
 * @file syn_cobs.h
 * @brief COBS (Consistent Overhead Byte Stuffing) packet framing.
 *
 * Turns an arbitrary byte stream into framed packets separated by 0x00
 * delimiters, with zero bytes inside replaced by run-length codes.
 * Overhead is exactly 1 byte per 254 data bytes.
 *
 * Use this over UART, RS-485, or any byte-oriented link where you need
 * reliable packet boundaries.
 *
 * @par Usage
 * @code
 *   // Encode a packet
 *   uint8_t encoded[256];
 *   size_t enc_len = syn_cobs_encode(data, data_len, encoded);
 *
 *   // Decode a packet
 *   uint8_t decoded[256];
 *   size_t dec_len = syn_cobs_decode(encoded, enc_len, decoded);
 *
 *   // Streaming decoder (byte-at-a-time from UART RX)
 *   SYN_COBS_Decoder dec;
 *   syn_cobs_decoder_init(&dec, rx_buf, sizeof(rx_buf), on_packet, NULL);
 *   syn_cobs_decoder_feed(&dec, byte);
 * @endcode
 * @ingroup syn_protocol
 */

#ifndef SYN_COBS_H
#define SYN_COBS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── One-shot encode / decode ───────────────────────────────────────────── */

/**
 * @brief Encode data using COBS.
 *
 * @param src      Input data.
 * @param src_len  Input length.
 * @param dst      Output buffer (must be at least src_len + src_len/254 + 1).
 * @return Number of bytes written to dst (excluding the trailing 0x00 delimiter).
 */
size_t syn_cobs_encode(const void *src, size_t src_len, void *dst);

/**
 * @brief Decode a COBS-encoded packet.
 *
 * @param src      COBS-encoded data (without the 0x00 delimiter).
 * @param src_len  Encoded length.
 * @param dst      Output buffer (must be at least src_len).
 * @return Number of decoded bytes, or 0 on error.
 */
size_t syn_cobs_decode(const void *src, size_t src_len, void *dst);

/* ── Streaming decoder ──────────────────────────────────────────────────── */

struct SYN_COBS_Decoder;

/**
 * @brief Callback invoked when a complete packet is decoded.
 *
 * @param data     Decoded packet data.
 * @param len      Decoded packet length.
 * @param ctx      User context.
 */
typedef void (*SYN_COBS_PacketCallback)(const uint8_t *data, size_t len,
                                         void *ctx);

/** @brief Streaming COBS frame decoder. */
typedef struct SYN_COBS_Decoder {
    uint8_t                 *buf;       /**< Receive buffer                */
    size_t                   buf_size;  /**< Buffer capacity               */
    size_t                   idx;       /**< Current write position        */
    SYN_COBS_PacketCallback callback;  /**< Callback on complete packet   */
    void                    *ctx;      /**< User context for callback     */
} SYN_COBS_Decoder;

/**
 * @brief Initialize streaming COBS decoder.
 *
 * @param dec       Decoder instance.
 * @param buf       Receive buffer for accumulating encoded bytes.
 * @param buf_size  Buffer capacity.
 * @param callback  Called when a complete packet is decoded.
 * @param ctx       User context for callback.
 */
void syn_cobs_decoder_init(SYN_COBS_Decoder *dec,
                            uint8_t *buf, size_t buf_size,
                            SYN_COBS_PacketCallback callback,
                            void *ctx);

/**
 * @brief Feed a byte to the streaming decoder.
 *
 * When a 0x00 delimiter is received, the accumulated bytes are
 * decoded and the callback is invoked with the decoded packet.
 *
 * @param dec   Decoder.
 * @param byte  Received byte.
 */
void syn_cobs_decoder_feed(SYN_COBS_Decoder *dec, uint8_t byte);

/**
 * @brief Reset the streaming decoder state.
 * @param dec  Decoder.
 */
void syn_cobs_decoder_reset(SYN_COBS_Decoder *dec);

#ifdef __cplusplus
}
#endif

#endif /* SYN_COBS_H */
