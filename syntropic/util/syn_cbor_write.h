/**
 * @file syn_cbor_write.h
 * @brief Streaming CBOR encoder — zero-alloc, caller-provided buffer.
 *
 * Encodes CBOR (RFC 8949) items into a fixed buffer.  No heap, no
 * recursion, no indefinite-length items.  Map and array sizes must
 * be known before writing begins.
 *
 * Supported types:
 *   unsigned int, signed int, float32, bool, null,
 *   text string, byte string, array, map.
 *
 * @par Usage — sensor telemetry with integer keys
 * @code
 *   uint8_t buf[32];
 *   SYN_CborWriter w;
 *   syn_cbor_writer_init(&w, buf, sizeof(buf));
 *   syn_cbor_write_map_begin(&w, 2);         // A2
 *     syn_cbor_write_uint(&w, 1);            // key 1 = temperature
 *     syn_cbor_write_float(&w, 23.5f);       // 23.5
 *     syn_cbor_write_uint(&w, 2);            // key 2 = humidity
 *     syn_cbor_write_uint(&w, 60);           // 60
 *   // result: A2 01 FA41BC0000 02 183C  (10 bytes)
 * @endcode
 *
 * @par Usage — named string keys
 * @code
 *   syn_cbor_write_map_begin(&w, 1);
 *     syn_cbor_write_text_cstr(&w, "temp");
 *     syn_cbor_write_float(&w, 23.5f);
 * @endcode
 * @ingroup syn_protocol
 */

#ifndef SYN_CBOR_WRITE_H
#define SYN_CBOR_WRITE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Writer state ────────────────────────────────────────────────────────── */

/** @brief CBOR encoder state.  Caller-allocated; zero heap. */
typedef struct {
    uint8_t *buf;       /**< Output buffer                 */
    size_t   cap;       /**< Buffer capacity in bytes      */
    size_t   len;       /**< Bytes encoded so far          */
    bool     overflow;  /**< Set if buffer capacity exceeded */
} SYN_CborWriter;

/* ── Init ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a CBOR writer.
 * @param w    Writer to initialize.  Must not be NULL.
 * @param buf  Output buffer.  Must not be NULL.
 * @param cap  Buffer capacity in bytes.
 */
void syn_cbor_writer_init(SYN_CborWriter *w, uint8_t *buf, size_t cap);

/* ── Collections ─────────────────────────────────────────────────────────── */

/**
 * @brief Open a CBOR map with @p count key-value pairs.
 *
 * Caller must write exactly @p count pairs (key item + value item each)
 * after this call.
 *
 * @param w      Writer.
 * @param count  Number of key-value pairs that follow.
 */
void syn_cbor_write_map_begin(SYN_CborWriter *w, size_t count);

/**
 * @brief Open a CBOR array with @p count items.
 *
 * Caller must write exactly @p count items after this call.
 *
 * @param w      Writer.
 * @param count  Number of items that follow.
 */
void syn_cbor_write_array_begin(SYN_CborWriter *w, size_t count);

/* ── Scalars ─────────────────────────────────────────────────────────────── */

/**
 * @brief Write an unsigned integer.
 * @param w  Writer.
 * @param v  Value to encode (CBOR major type 0).
 */
void syn_cbor_write_uint(SYN_CborWriter *w, uint64_t v);

/**
 * @brief Write a signed integer.
 *
 * Positive values are encoded as major type 0 (uint).
 * Negative values are encoded as major type 1.
 *
 * @param w  Writer.
 * @param v  Signed value to encode.
 */
void syn_cbor_write_int(SYN_CborWriter *w, int64_t v);

/**
 * @brief Write an IEEE 754 single-precision float (major type 7, info=26).
 * @param w  Writer.
 * @param v  Float value.
 */
void syn_cbor_write_float(SYN_CborWriter *w, float v);

/**
 * @brief Write a boolean (0xF4 = false, 0xF5 = true).
 * @param w  Writer.
 * @param v  Value.
 */
void syn_cbor_write_bool(SYN_CborWriter *w, bool v);

/**
 * @brief Write a null (0xF6).
 * @param w  Writer.
 */
void syn_cbor_write_null(SYN_CborWriter *w);

/* ── Strings ─────────────────────────────────────────────────────────────── */

/**
 * @brief Write a UTF-8 text string (major type 3).
 * @param w    Writer.
 * @param str  Pointer to string data.
 * @param len  Byte length (not counting any null terminator).
 */
void syn_cbor_write_text(SYN_CborWriter *w, const char *str, size_t len);

/**
 * @brief Write a null-terminated UTF-8 text string.
 * @param w    Writer.
 * @param str  Null-terminated string.
 */
void syn_cbor_write_text_cstr(SYN_CborWriter *w, const char *str);

/**
 * @brief Write a byte string (major type 2).
 * @param w    Writer.
 * @param data Pointer to raw bytes.
 * @param len  Byte count.
 */
void syn_cbor_write_bytes(SYN_CborWriter *w, const uint8_t *data, size_t len);

/* ── Status ──────────────────────────────────────────────────────────────── */

/**
 * @brief Return bytes encoded so far.
 * @param w  Writer.
 * @return Byte count.
 */
static inline size_t syn_cbor_writer_len(const SYN_CborWriter *w)
{
    return w->len;
}

/**
 * @brief Return true if no overflow has occurred.
 * @param w  Writer.
 * @return true if OK.
 */
static inline bool syn_cbor_writer_ok(const SYN_CborWriter *w)
{
    return !w->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CBOR_WRITE_H */
