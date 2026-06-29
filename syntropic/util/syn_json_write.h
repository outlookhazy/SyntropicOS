/**
 * @file syn_json_write.h
 * @brief Streaming JSON builder — zero-alloc, caller-provided buffer.
 *
 * Builds JSON output incrementally into a fixed buffer. Handles
 * auto-commas, nesting, and overflow detection. No parser — this
 * is write-only for building API responses, telemetry payloads, etc.
 *
 * @par Usage
 * @code
 *   char buf[256];
 *   SYN_JsonWriter w;
 *   syn_json_init(&w, buf, sizeof(buf));
 *   syn_json_obj_open(&w);
 *     syn_json_key_str(&w, "device", "esp32");
 *     syn_json_key_int(&w, "uptime", 12345);
 *     syn_json_key_bool(&w, "wifi", true);
 *   syn_json_obj_close(&w);
 *   // buf = {"device":"esp32","uptime":12345,"wifi":true}
 * @endcode
 * @ingroup syn_protocol
 */

#ifndef SYN_JSON_WRITE_H
#define SYN_JSON_WRITE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Writer state ──────────────────────────────────────────────────────── */

#define SYN_JSON_MAX_DEPTH  8  /**< Maximum nesting depth */

/** @brief Streaming JSON writer — tracks nesting, commas, and overflow. */
typedef struct {
    char    *buf;              /**< Output buffer                          */
    size_t   capacity;         /**< Buffer capacity                        */
    size_t   len;              /**< Bytes written so far                   */
    uint8_t  depth;            /**< Current nesting depth                  */
    bool     needs_comma;      /**< Insert comma before next element       */
    bool     overflow;         /**< Set if buffer overflowed               */
} SYN_JsonWriter;

/* ── Init ──────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a JSON writer.
 *
 * @param w         Writer instance.
 * @param buf       Output buffer.
 * @param capacity  Buffer size in bytes.
 */
void syn_json_init(SYN_JsonWriter *w, char *buf, size_t capacity);

/* ── Structure ─────────────────────────────────────────────────────────── */

/**
 * @brief Open a JSON object `{`.
 * @param w  Writer.
 */
void syn_json_obj_open(SYN_JsonWriter *w);

/**
 * @brief Close a JSON object `}`.
 * @param w  Writer.
 */
void syn_json_obj_close(SYN_JsonWriter *w);

/**
 * @brief Open a JSON array `[`.
 * @param w  Writer.
 */
void syn_json_arr_open(SYN_JsonWriter *w);

/**
 * @brief Close a JSON array `]`.
 * @param w  Writer.
 */
void syn_json_arr_close(SYN_JsonWriter *w);

/* ── Key-value pairs (inside objects) ──────────────────────────────────── */

/**
 * @brief Write `"key":"value"`. Escapes `"` and `\\` in value.
 * @param w    Writer.
 * @param key  JSON key.
 * @param val  String value.
 */
void syn_json_key_str(SYN_JsonWriter *w, const char *key, const char *val);

/**
 * @brief Write `"key":123`.
 * @param w    Writer.
 * @param key  JSON key.
 * @param val  Integer value.
 */
void syn_json_key_int(SYN_JsonWriter *w, const char *key, int32_t val);

/**
 * @brief Write `"key":123` (unsigned).
 * @param w    Writer.
 * @param key  JSON key.
 * @param val  Unsigned integer value.
 */
void syn_json_key_uint(SYN_JsonWriter *w, const char *key, uint32_t val);

/**
 * @brief Write `"key":true` or `"key":false`.
 * @param w    Writer.
 * @param key  JSON key.
 * @param val  Boolean value.
 */
void syn_json_key_bool(SYN_JsonWriter *w, const char *key, bool val);

/**
 * @brief Write `"key":null`.
 * @param w    Writer.
 * @param key  JSON key.
 */
void syn_json_key_null(SYN_JsonWriter *w, const char *key);

/**
 * @brief Write a bare key `"key":` for nested objects/arrays.
 *
 * Follow with syn_json_obj_open() or syn_json_arr_open().
 *
 * @param w    Writer.
 * @param key  JSON key.
 */
void syn_json_key(SYN_JsonWriter *w, const char *key);

/* ── Array values (inside arrays) ──────────────────────────────────────── */

/**
 * @brief Append a string value to an array.
 * @param w    Writer.
 * @param val  String value.
 */
void syn_json_val_str(SYN_JsonWriter *w, const char *val);

/**
 * @brief Append an integer value to an array.
 * @param w    Writer.
 * @param val  Integer value.
 */
void syn_json_val_int(SYN_JsonWriter *w, int32_t val);

/**
 * @brief Append an unsigned integer value to an array.
 * @param w    Writer.
 * @param val  Unsigned value.
 */
void syn_json_val_uint(SYN_JsonWriter *w, uint32_t val);

/**
 * @brief Append a boolean value to an array.
 * @param w    Writer.
 * @param val  Boolean value.
 */
void syn_json_val_bool(SYN_JsonWriter *w, bool val);

/* ── Query ─────────────────────────────────────────────────────────────── */

/**
 * @brief Get the number of bytes written.
 * @param w  Writer.
 * @return Byte count.
 */
static inline size_t syn_json_len(const SYN_JsonWriter *w)
{
    return w->len;
}

/**
 * @brief Get the null-terminated output string.
 * @param w  Writer.
 * @return Pointer to the output buffer.
 */
static inline const char *syn_json_str(const SYN_JsonWriter *w)
{
    return w->buf;
}

/**
 * @brief Check if the writer is in a valid state (no overflow).
 * @param w  Writer.
 * @return true if no overflow occurred.
 */
static inline bool syn_json_ok(const SYN_JsonWriter *w)
{
    return !w->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_JSON_WRITE_H */
