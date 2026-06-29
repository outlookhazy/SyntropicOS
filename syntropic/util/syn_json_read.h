/**
 * @file syn_json_read.h
 * @brief Minimal JSON reader — in-place, zero-alloc.
 *
 * Parses a JSON string in-place, providing token-based access.
 * No dynamic memory, no callbacks — just walks through the buffer
 * and lets you query values by key path.
 *
 * @par Usage — Query by key
 * @code
 *   char json[] = "{\"ssid\":\"MyNet\",\"channel\":6,\"hidden\":false}";
 *   SYN_JsonReader r;
 *   syn_json_parse(&r, json, strlen(json));
 *
 *   char ssid[33];
 *   if (syn_json_get_str(&r, "ssid", ssid, sizeof(ssid))) { ... }
 *
 *   int32_t ch;
 *   if (syn_json_get_int(&r, "channel", &ch)) { ... }
 *
 *   bool hidden;
 *   if (syn_json_get_bool(&r, "hidden", &hidden)) { ... }
 * @endcode
 *
 * @par Limitations
 * - Top-level must be an object `{}`
 * - Nested objects accessed via dot notation: `"wifi.ssid"`
 * - No array indexing (arrays can be iterated)
 * - Modifies the input buffer (inserts null terminators)
 * - Max 32 tokens (keys + values) per parse
 * @ingroup syn_protocol
 */

#ifndef SYN_JSON_READ_H
#define SYN_JSON_READ_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Token types ───────────────────────────────────────────────────────── */

/** @brief JSON value types. */
typedef enum {
    SYN_JSON_NONE    = 0,  /**< No type / uninitialized               */
    SYN_JSON_STRING  = 1,  /**< String value                           */
    SYN_JSON_NUMBER  = 2,  /**< Numeric value                          */
    SYN_JSON_BOOL    = 3,  /**< Boolean (true/false)                   */
    SYN_JSON_NULL    = 4,  /**< Null value                             */
    SYN_JSON_OBJECT  = 5,  /**< Object (nested)                        */
    SYN_JSON_ARRAY   = 6,  /**< Array                                  */
} SYN_JsonType;

/* ── Token ─────────────────────────────────────────────────────────────── */

/** @brief Maximum number of parsed JSON tokens per document. */
#define SYN_JSON_MAX_TOKENS  32

/** @brief Parsed JSON token — key + value + type. */
typedef struct {
    SYN_JsonType  type;      /**< Value type                            */
    const char   *key;       /**< Key string (NULL for array elements)  */
    const char   *value;     /**< Value string (for string/number)      */
    int32_t       int_val;   /**< Parsed integer (for numbers)          */
    uint8_t       depth;     /**< Nesting depth                         */
} SYN_JsonToken;

/* ── Reader ────────────────────────────────────────────────────────────── */

/** @brief JSON reader — token array + parse state. */
typedef struct {
    SYN_JsonToken  tokens[SYN_JSON_MAX_TOKENS]; /**< Parsed tokens      */
    size_t         token_count; /**< Number of tokens parsed             */
    bool           valid;      /**< Parse succeeded                     */
} SYN_JsonReader;

/* ── API ───────────────────────────────────────────────────────────────── */

/**
 * @brief Parse a JSON string in-place.
 *
 * Tokenizes the JSON into key-value pairs. Modifies the input buffer
 * (inserts null terminators at string boundaries).
 *
 * @param r      Reader instance.
 * @param json   JSON string (will be modified).
 * @param len    Length of JSON string.
 * @return true if parsing succeeded.
 */
bool syn_json_parse(SYN_JsonReader *r, char *json, size_t len);

/**
 * @brief Find a token by key name.
 *
 * Use dot notation for nested keys: "wifi.ssid"
 *
 * @param r    Reader.
 * @param key  Key to find.
 * @return Pointer to token, or NULL if not found.
 */
const SYN_JsonToken *syn_json_find(const SYN_JsonReader *r, const char *key);

/**
 * @brief Get a string value by key.
 *
 * @param r       Reader.
 * @param key     Key to find.
 * @param out     Output buffer for the string.
 * @param out_sz  Buffer capacity.
 * @return true if key found and value is a string.
 */
bool syn_json_get_str(const SYN_JsonReader *r, const char *key,
                       char *out, size_t out_sz);

/**
 * @brief Get an integer value by key.
 *
 * @param r    Reader.
 * @param key  Key to find.
 * @param out  Output value.
 * @return true if key found and value is a number.
 */
bool syn_json_get_int(const SYN_JsonReader *r, const char *key, int32_t *out);

/**
 * @brief Get a boolean value by key.
 *
 * @param r    Reader.
 * @param key  Key to find.
 * @param out  Output value.
 * @return true if key found and value is a boolean.
 */
bool syn_json_get_bool(const SYN_JsonReader *r, const char *key, bool *out);

/**
 * @brief Check if a key exists and is null.
 * @param r    Reader.
 * @param key  Key to find.
 * @return true if key exists and value is null.
 */
bool syn_json_is_null(const SYN_JsonReader *r, const char *key);

/**
 * @brief Get the type of a value by key.
 * @param r    Reader.
 * @param key  Key to find.
 * @return SYN_JsonType, or SYN_JSON_NONE if not found.
 */
SYN_JsonType syn_json_get_type(const SYN_JsonReader *r, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* SYN_JSON_READ_H */
