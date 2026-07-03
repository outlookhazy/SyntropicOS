/**
 * @file syn_stream.h
 * @brief Cooperative byte stream — protothread-aware ringbuf wrapper.
 *
 * Composes SYN_RingBuf with readability awareness for cooperative tasks.
 * Three readability modes:
 *   - **Default**: readable when any bytes are available.
 *   - **Threshold**: readable when N+ bytes are available (frame protocols).
 *   - **Delimiter**: readable when a delimiter byte is in the buffer (line protocols).
 *
 * Producer side is ISR-safe (single-producer / single-consumer).
 * Consumer side integrates with protothreads via PT_STREAM_WAIT.
 *
 * @par Usage — Line-buffered UART reader
 * @code
 *   static uint8_t backing[128];
 *   static SYN_Stream uart_rx;
 *   syn_stream_init(&uart_rx, backing, sizeof(backing));
 *   syn_stream_set_delimiter(&uart_rx, '\n');
 *
 *   // ISR fills the stream:
 *   syn_stream_put(&uart_rx, byte);
 *
 *   // Protothread consumes complete lines:
 *   SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task) {
 *       static uint8_t line[80];
 *       static size_t  n;
 *       PT_BEGIN(pt);
 *       for (;;) {
 *           PT_STREAM_WAIT(pt, &uart_rx);
 *           n = syn_stream_read_line(&uart_rx, line, sizeof(line));
 *           process_line(line, n);
 *       }
 *       PT_END(pt);
 *   }
 * @endcode
 *
 * @par Usage — Threshold-based frame reader
 * @code
 *   syn_stream_set_threshold(&adc_stream, 64);  // wait for 64 bytes
 *   PT_STREAM_WAIT(pt, &adc_stream);
 *   n = syn_stream_read(&adc_stream, frame, 64);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_STREAM_H
#define SYN_STREAM_H

#include "../util/syn_ringbuf.h"
#include "../pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Stream instance ────────────────────────────────────────────────────── */

/**
 * @brief Cooperative byte stream — ringbuf with readability awareness.
 *
 * All fields are considered private. Use the API functions below.
 */
typedef struct {
    SYN_RingBuf  rb;           /**< Underlying ring buffer (composition)   */
    size_t       threshold;    /**< Readable when count >= this (0 = any)  */
    uint8_t      delimiter;    /**< Delimiter byte for line mode           */
    bool         delim_en;     /**< true if delimiter mode is active       */
} SYN_Stream;

/* ── Init / config ──────────────────────────────────────────────────────── */

/**
 * @brief Initialize a stream with a caller-owned backing buffer.
 *
 * @param s     Stream instance.
 * @param buf   Backing array (caller-owned, must outlive the stream).
 * @param size  Size of backing array. Usable capacity is size - 1.
 */
void syn_stream_init(SYN_Stream *s, uint8_t *buf, size_t size);

/**
 * @brief Set the byte threshold for readability.
 *
 * When set to N > 0, syn_stream_readable() returns true only when
 * at least N bytes are in the buffer. Set to 0 to revert to default
 * (readable when any bytes are available).
 *
 * @param s  Stream instance.
 * @param n  Byte threshold (0 = any).
 */
void syn_stream_set_threshold(SYN_Stream *s, size_t n);

/**
 * @brief Enable delimiter mode.
 *
 * When enabled, syn_stream_readable() returns true only when the
 * delimiter byte is present in the buffer. Pair with
 * syn_stream_read_line() to consume one delimited chunk at a time.
 *
 * Delimiter mode takes precedence over threshold mode.
 *
 * @param s      Stream instance.
 * @param delim  Delimiter byte (e.g. '\\n').
 */
void syn_stream_set_delimiter(SYN_Stream *s, uint8_t delim);

/**
 * @brief Disable delimiter mode, reverting to default or threshold mode.
 * @param s  Stream instance.
 */
void syn_stream_clear_delimiter(SYN_Stream *s);

/* ── Producer (ISR-safe, never blocks) ──────────────────────────────────── */

/**
 * @brief Write bytes into the stream.
 *
 * ISR-safe (single-producer). Writes up to @p len bytes. Returns the
 * number actually written (may be less if the buffer fills up).
 *
 * @param s     Stream instance.
 * @param data  Source data.
 * @param len   Number of bytes to write.
 * @return Number of bytes actually written.
 */
size_t syn_stream_write(SYN_Stream *s, const uint8_t *data, size_t len);

/**
 * @brief Put a single byte into the stream.
 *
 * ISR-safe convenience wrapper around syn_stream_write().
 *
 * @param s     Stream instance.
 * @param byte  Byte to store.
 * @return true if the byte was stored, false if the buffer is full.
 */
bool syn_stream_put(SYN_Stream *s, uint8_t byte);

/* ── Consumer ───────────────────────────────────────────────────────────── */

/**
 * @brief Check if the stream is readable (mode-aware).
 *
 * Behavior depends on configured mode:
 *   - **Delimiter mode**: true if the delimiter byte is in the buffer.
 *   - **Threshold mode**: true if count >= threshold.
 *   - **Default**: true if any bytes are available.
 *
 * Delimiter mode takes precedence over threshold mode.
 *
 * @param s  Stream instance.
 * @return true if the stream has data meeting the readability criteria.
 */
bool syn_stream_readable(const SYN_Stream *s);

/**
 * @brief Read up to @p max_len bytes from the stream.
 *
 * Non-blocking — returns 0 if the buffer is empty. Does not respect
 * threshold or delimiter; reads whatever is available.
 *
 * @param s        Stream instance.
 * @param buf      Destination buffer.
 * @param max_len  Maximum bytes to read.
 * @return Number of bytes actually read.
 */
size_t syn_stream_read(SYN_Stream *s, uint8_t *buf, size_t max_len);

/**
 * @brief Read up to and including the next delimiter byte.
 *
 * Reads bytes from the stream into @p buf, stopping after the first
 * delimiter byte (which is included in the output). If no delimiter
 * is found in the buffer, returns 0 and reads nothing.
 *
 * Pair with PT_STREAM_WAIT in delimiter mode to wait for a complete line:
 * @code
 *   PT_STREAM_WAIT(pt, &stream);
 *   n = syn_stream_read_line(&stream, buf, sizeof(buf));
 * @endcode
 *
 * @param s        Stream instance.
 * @param buf      Destination buffer.
 * @param max_len  Maximum bytes to read (including delimiter).
 * @return Number of bytes read (0 if no delimiter found or empty).
 */
size_t syn_stream_read_line(SYN_Stream *s, uint8_t *buf, size_t max_len);

/**
 * @brief Return the number of bytes currently in the stream.
 * @param s  Stream instance.
 * @return Byte count.
 */
size_t syn_stream_count(const SYN_Stream *s);

/**
 * @brief Return the number of free bytes available for writing.
 * @param s  Stream instance.
 * @return Free byte count.
 */
size_t syn_stream_free(const SYN_Stream *s);

/**
 * @brief Discard all data in the stream.
 * @param s  Stream instance.
 */
void syn_stream_flush(SYN_Stream *s);

/* ── Protothread integration ────────────────────────────────────────────── */

/**
 * @brief Wait until the stream is readable (mode-aware).
 *
 * Yields the protothread until syn_stream_readable() returns true.
 *
 * @param pt      Protothread context.
 * @param stream  Pointer to SYN_Stream.
 */
#define PT_STREAM_WAIT(pt, stream)  \
    PT_WAIT_UNTIL(pt, syn_stream_readable(stream))

#ifdef __cplusplus
}
#endif

#endif /* SYN_STREAM_H */
