/**
 * @file syn_ringbuf.h
 * @brief Fixed-size, statically-allocated ring buffer.
 *
 * Designed for ISR-safe single-producer / single-consumer use.
 * The buffer is caller-owned — you provide the backing array and the
 * SYN_RingBuf struct; the library never allocates memory.
 *
 * Usage:
 * @code
 *   static uint8_t backing[64];
 *   static SYN_RingBuf rb;
 *   syn_ringbuf_init(&rb, backing, sizeof(backing));
 *
 *   syn_ringbuf_put(&rb, 0xAA);           // producer (e.g., ISR)
 *
 *   uint8_t byte;
 *   if (syn_ringbuf_get(&rb, &byte)) {    // consumer (e.g., main loop)
 *       // use byte
 *   }
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_RINGBUF_H
#define SYN_RINGBUF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ring buffer control structure.
 *
 * All fields are considered private. Use the API functions below.
 */
typedef struct {
    uint8_t *buf;       /**< Pointer to caller-provided backing array */
    size_t   size;      /**< Total size of backing array in bytes */
    volatile size_t head;  /**< Write index (producer advances this) */
    volatile size_t tail;  /**< Read index (consumer advances this) */
} SYN_RingBuf;

/**
 * @brief Initialize a ring buffer.
 *
 * @param rb    Pointer to the ring buffer control struct.
 * @param buf   Pointer to the caller-provided backing array.
 * @param size  Size of the backing array in bytes. Must be > 0.
 *              One byte is used as a sentinel, so usable capacity is size - 1.
 */
void syn_ringbuf_init(SYN_RingBuf *rb, uint8_t *buf, size_t size);

/**
 * @brief Reset the ring buffer to empty.
 * @param rb  Ring buffer.
 */
void syn_ringbuf_reset(SYN_RingBuf *rb);

/**
 * @brief Put a byte into the ring buffer.
 *
 * @param rb    Ring buffer.
 * @param byte  Byte to store.
 * @return true if the byte was stored, false if the buffer is full.
 */
bool syn_ringbuf_put(SYN_RingBuf *rb, uint8_t byte);

/**
 * @brief Get a byte from the ring buffer.
 *
 * @param rb    Ring buffer.
 * @param byte  [out] The retrieved byte.
 * @return true if a byte was available, false if the buffer is empty.
 */
bool syn_ringbuf_get(SYN_RingBuf *rb, uint8_t *byte);

/**
 * @brief Peek at the next byte without removing it.
 *
 * @param rb    Ring buffer.
 * @param byte  [out] The peeked byte.
 * @return true if a byte was available, false if the buffer is empty.
 */
bool syn_ringbuf_peek(const SYN_RingBuf *rb, uint8_t *byte);

/**
 * @brief Check if the ring buffer is full.
 * @param rb  Ring buffer.
 * @return true if full.
 */
bool syn_ringbuf_full(const SYN_RingBuf *rb);

/**
 * @brief Check if the ring buffer is empty.
 * @param rb  Ring buffer.
 * @return true if empty.
 */
bool syn_ringbuf_empty(const SYN_RingBuf *rb);

/**
 * @brief Return the number of bytes currently stored in the buffer.
 * @param rb  Ring buffer.
 * @return Byte count.
 */
size_t syn_ringbuf_count(const SYN_RingBuf *rb);

/**
 * @brief Return the number of free bytes available for writing.
 * @param rb  Ring buffer.
 * @return Free byte count.
 */
size_t syn_ringbuf_free(const SYN_RingBuf *rb);

/**
 * @brief Write multiple bytes into the ring buffer.
 *
 * Writes up to @p len bytes from @p data. Returns the number of bytes
 * actually written (may be less than @p len if the buffer fills up).
 * Uses memcpy internally for efficiency across wrap boundaries.
 *
 * @param rb    Ring buffer.
 * @param data  Source data to write.
 * @param len   Number of bytes to write.
 * @return Number of bytes actually written.
 */
size_t syn_ringbuf_write(SYN_RingBuf *rb, const uint8_t *data, size_t len);

/**
 * @brief Read multiple bytes from the ring buffer.
 *
 * Reads up to @p len bytes into @p data. Returns the number of bytes
 * actually read (may be less than @p len if the buffer doesn't have enough).
 * Uses memcpy internally for efficiency across wrap boundaries.
 *
 * @param rb    Ring buffer.
 * @param data  [out] Destination buffer.
 * @param len   Maximum number of bytes to read.
 * @return Number of bytes actually read.
 */
size_t syn_ringbuf_read(SYN_RingBuf *rb, uint8_t *data, size_t len);

/**
 * @brief Peek at up to @p len bytes without removing them.
 *
 * Copies bytes starting from the read position into @p data without
 * advancing the tail. Subsequent calls to syn_ringbuf_read will return
 * the same bytes.
 *
 * @param rb    Ring buffer.
 * @param data  [out] Destination buffer.
 * @param len   Maximum number of bytes to peek.
 * @return Number of bytes actually copied (may be less than @p len).
 */
size_t syn_ringbuf_peek_n(const SYN_RingBuf *rb, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RINGBUF_H */
