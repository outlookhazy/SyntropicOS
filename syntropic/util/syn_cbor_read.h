/**
 * @file syn_cbor_read.h
 * @brief CBOR decoder — zero-alloc, streaming read from a byte buffer.
 *
 * Walks a CBOR-encoded buffer sequentially.  Caller peeks the type of
 * the next item, then calls the matching read function.  Reading the
 * wrong type sets the error flag and returns a zero/empty value.
 *
 * @par Usage — read a 2-entry integer-keyed map
 * @code
 *   SYN_CborReader r;
 *   syn_cbor_reader_init(&r, buf, len);
 *
 *   size_t pairs = syn_cbor_read_map_begin(&r);   // A2 -> 2
 *   for (size_t i = 0; i < pairs; i++) {
 *       uint64_t key = syn_cbor_read_uint(&r);
 *       if      (key == 1) temperature = syn_cbor_read_float(&r);
 *       else if (key == 2) humidity    = (uint8_t)syn_cbor_read_uint(&r);
 *       else    syn_cbor_skip(&r);
 *   }
 *   if (!syn_cbor_reader_ok(&r)) { // handle error }
 * @endcode
 * @ingroup syn_protocol
 */

#ifndef SYN_CBOR_READ_H
#define SYN_CBOR_READ_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Item type ───────────────────────────────────────────────────────────── */

/**
 * @brief CBOR item type as seen by the reader.
 *
 * Returned by syn_cbor_peek_type() before consuming an item.
 */
typedef enum {
    SYN_CBOR_UINT   = 0,   /**< Unsigned integer (major type 0) */
    SYN_CBOR_INT    = 1,   /**< Negative integer (major type 1) */
    SYN_CBOR_BYTES  = 2,   /**< Byte string      (major type 2) */
    SYN_CBOR_TEXT   = 3,   /**< Text string      (major type 3) */
    SYN_CBOR_ARRAY  = 4,   /**< Array            (major type 4) */
    SYN_CBOR_MAP    = 5,   /**< Map              (major type 5) */
    SYN_CBOR_FLOAT  = 6,   /**< Float32/64       (major type 7, info 25-27) */
    SYN_CBOR_BOOL   = 7,   /**< true / false     (major type 7, info 20-21) */
    SYN_CBOR_NULL   = 8,   /**< null             (major type 7, info 22)    */
    SYN_CBOR_ERROR  = 0xFF /**< Unknown or error                             */
} SYN_CborType;

/* ── Reader state ────────────────────────────────────────────────────────── */

/** @brief CBOR decoder state.  Caller-allocated; zero heap. */
typedef struct {
    const uint8_t *buf;  /**< Input buffer                  */
    size_t         len;  /**< Buffer length                 */
    size_t         pos;  /**< Current read position         */
    bool           ok;   /**< false after any decode error  */
} SYN_CborReader;

/* ── Init ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a CBOR reader.
 * @param r    Reader to initialize.
 * @param buf  Input buffer containing encoded CBOR.
 * @param len  Buffer length in bytes.
 */
void syn_cbor_reader_init(SYN_CborReader *r,
                          const uint8_t  *buf,
                          size_t          len);

/* ── Navigation ──────────────────────────────────────────────────────────── */

/**
 * @brief Return the type of the next item without consuming it.
 * @param r  Reader.
 * @return   Item type, or SYN_CBOR_ERROR on buffer underrun.
 */
SYN_CborType syn_cbor_peek_type(const SYN_CborReader *r);

/**
 * @brief Skip the next complete item (including nested contents).
 *
 * Handles nested arrays and maps up to 8 levels deep.
 *
 * @param r  Reader.
 */
void syn_cbor_skip(SYN_CborReader *r);

/* ── Collections ─────────────────────────────────────────────────────────── */

/**
 * @brief Consume a map header; return the number of key-value pairs.
 * @param r  Reader.
 * @return   Pair count, or 0 on error.
 */
size_t syn_cbor_read_map_begin(SYN_CborReader *r);

/**
 * @brief Consume an array header; return the number of items.
 * @param r  Reader.
 * @return   Item count, or 0 on error.
 */
size_t syn_cbor_read_array_begin(SYN_CborReader *r);

/* ── Scalars ─────────────────────────────────────────────────────────────── */

/**
 * @brief Read an unsigned integer (major type 0).
 * @param r  Reader.
 * @return   Decoded value, or 0 on error.
 */
uint64_t syn_cbor_read_uint(SYN_CborReader *r);

/**
 * @brief Read a signed integer (major type 0 or 1).
 * @param r  Reader.
 * @return   Decoded value, or 0 on error.
 */
int64_t syn_cbor_read_int(SYN_CborReader *r);

/**
 * @brief Read a float32 value (major type 7, info=26).
 * @param r  Reader.
 * @return   Decoded float, or 0.0f on error.
 */
float syn_cbor_read_float(SYN_CborReader *r);

/**
 * @brief Read a boolean (major type 7, info 20 or 21).
 * @param r  Reader.
 * @return   Decoded value, or false on error.
 */
bool syn_cbor_read_bool(SYN_CborReader *r);

/**
 * @brief Consume a null item (major type 7, info=22).
 * @param r  Reader.
 */
void syn_cbor_read_null(SYN_CborReader *r);

/* ── Strings ─────────────────────────────────────────────────────────────── */

/**
 * @brief Read a text string into a caller-provided buffer.
 *
 * Copies at most @p cap-1 bytes and null-terminates.  The CBOR item is
 * fully consumed regardless of buffer capacity.
 *
 * @param r    Reader.
 * @param buf  Output buffer.
 * @param cap  Output buffer capacity (including space for null terminator).
 * @return     Actual UTF-8 byte count in the CBOR item (may exceed @p cap-1).
 */
size_t syn_cbor_read_text(SYN_CborReader *r, char *buf, size_t cap);

/**
 * @brief Read a byte string into a caller-provided buffer.
 *
 * Copies at most @p cap bytes.  The CBOR item is fully consumed.
 *
 * @param r    Reader.
 * @param buf  Output buffer.
 * @param cap  Output buffer capacity.
 * @return     Actual byte count in the CBOR item (may exceed @p cap).
 */
size_t syn_cbor_read_bytes(SYN_CborReader *r, uint8_t *buf, size_t cap);

/* ── Status ──────────────────────────────────────────────────────────────── */

/**
 * @brief Return true if no decode error has occurred.
 * @param r  Reader.
 * @return true if OK.
 */
static inline bool syn_cbor_reader_ok(const SYN_CborReader *r)
{
    return r->ok;
}

/**
 * @brief Return true when all bytes have been consumed.
 * @param r  Reader.
 * @return true if done.
 */
static inline bool syn_cbor_reader_done(const SYN_CborReader *r)
{
    return r->pos >= r->len;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CBOR_READ_H */
