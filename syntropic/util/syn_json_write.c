#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_JSON) || SYN_USE_JSON

/**
 * @file syn_json_write.c
 * @brief Streaming JSON builder implementation.
 */

#include "syn_json_write.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Internal helpers ──────────────────────────────────────────────────── */

/**
 * @brief Append a single char to the output buffer.
 * @param w   JSON writer.
 * @param ch  Character to append.
 */
static void jw_putc(SYN_JsonWriter *w, char ch)
{
    if (w->overflow) return;
    if (w->len + 1 >= w->capacity) {
        w->overflow = true;
        return;
    }
    w->buf[w->len++] = ch;
    w->buf[w->len] = '\0';
}

/**
 * @brief Append a raw string (no escaping).
 * @param w  JSON writer.
 * @param s  String to append.
 */
static void jw_puts(SYN_JsonWriter *w, const char *s)
{
    if (w->overflow) return;
    size_t slen = strlen(s);
    if (w->len + slen >= w->capacity) {
        w->overflow = true;
        return;
    }
    memcpy(w->buf + w->len, s, slen);
    w->len += slen;
    w->buf[w->len] = '\0';
}

/**
 * @brief Write an escaped JSON string (with surrounding quotes).
 * @param w  JSON writer.
 * @param s  String to encode.
 */
static void jw_str(SYN_JsonWriter *w, const char *s)
{
    jw_putc(w, '"');
    while (*s && !w->overflow) {
        char ch = *s++;
        switch (ch) {
            case '"':  jw_puts(w, "\\\""); break;
            case '\\': jw_puts(w, "\\\\"); break;
            case '\n': jw_puts(w, "\\n");  break;
            case '\r': jw_puts(w, "\\r");  break;
            case '\t': jw_puts(w, "\\t");  break;
            default:
                if ((uint8_t)ch < 0x20) {
                    /* Skip control chars */
                } else {
                    jw_putc(w, ch);
                }
                break;
        }
    }
    jw_putc(w, '"');
}

/**
 * @brief Write a signed integer.
 * @param w    JSON writer.
 * @param val  Value to write.
 */
static void jw_int(SYN_JsonWriter *w, int32_t val)
{
    char tmp[12]; /* -2147483648 + null = 12 */
    int pos = 0;
    uint32_t uval;
    bool neg = false;

    if (val < 0) {
        neg = true;
        uval = (uint32_t)(-(val + 1)) + 1u;
    } else {
        uval = (uint32_t)val;
    }

    /* Build digits in reverse */
    char digits[10];
    int dpos = 0;
    if (uval == 0) {
        digits[dpos++] = '0';
    } else {
        while (uval > 0) {
            digits[dpos++] = (char)('0' + (uval % 10));
            uval /= 10;
        }
    }

    if (neg) tmp[pos++] = '-';
    for (int i = dpos - 1; i >= 0; i--) {
        tmp[pos++] = digits[i];
    }
    tmp[pos] = '\0';

    jw_puts(w, tmp);
}

/**
 * @brief Write an unsigned integer.
 * @param w    JSON writer.
 * @param val  Value to write.
 */
static void jw_uint(SYN_JsonWriter *w, uint32_t val)
{
    char tmp[11]; /* 4294967295 + null = 11 */
    int pos = 0;

    char digits[10];
    int dpos = 0;
    if (val == 0) {
        digits[dpos++] = '0';
    } else {
        while (val > 0) {
            digits[dpos++] = (char)('0' + (val % 10));
            val /= 10;
        }
    }

    for (int i = dpos - 1; i >= 0; i--) {
        tmp[pos++] = digits[i];
    }
    tmp[pos] = '\0';

    jw_puts(w, tmp);
}

/**
 * @brief Insert comma separator if needed.
 * @param w  JSON writer.
 */
static void jw_comma(SYN_JsonWriter *w)
{
    if (w->needs_comma) {
        jw_putc(w, ',');
    }
    w->needs_comma = true;
}

/* ── API ───────────────────────────────────────────────────────────────── */

void syn_json_init(SYN_JsonWriter *w, char *buf, size_t capacity)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(capacity > 0);

    w->buf = buf;
    w->capacity = capacity;
    w->len = 0;
    w->depth = 0;
    w->needs_comma = false;
    w->overflow = false;
    w->buf[0] = '\0';
}

void syn_json_obj_open(SYN_JsonWriter *w)
{
    SYN_ASSERT(w != NULL);
    if (w->depth >= SYN_JSON_MAX_DEPTH) { w->overflow = true; return; }
    /* Only insert comma if this isn't following a key */
    if (w->needs_comma) jw_putc(w, ',');
    jw_putc(w, '{');
    w->depth++;
    w->needs_comma = false;
}

void syn_json_obj_close(SYN_JsonWriter *w)
{
    SYN_ASSERT(w != NULL);
    jw_putc(w, '}');
    if (w->depth > 0) w->depth--;
    w->needs_comma = true;
}

void syn_json_arr_open(SYN_JsonWriter *w)
{
    SYN_ASSERT(w != NULL);
    if (w->depth >= SYN_JSON_MAX_DEPTH) { w->overflow = true; return; }
    if (w->needs_comma) jw_putc(w, ',');
    jw_putc(w, '[');
    w->depth++;
    w->needs_comma = false;
}

void syn_json_arr_close(SYN_JsonWriter *w)
{
    SYN_ASSERT(w != NULL);
    jw_putc(w, ']');
    if (w->depth > 0) w->depth--;
    w->needs_comma = true;
}

void syn_json_key(SYN_JsonWriter *w, const char *key)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    w->needs_comma = false; /* value follows immediately */
}

void syn_json_key_str(SYN_JsonWriter *w, const char *key, const char *val)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    jw_str(w, val != NULL ? val : "");
}

void syn_json_key_int(SYN_JsonWriter *w, const char *key, int32_t val)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    jw_int(w, val);
}

void syn_json_key_uint(SYN_JsonWriter *w, const char *key, uint32_t val)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    jw_uint(w, val);
}

void syn_json_key_bool(SYN_JsonWriter *w, const char *key, bool val)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    jw_puts(w, val ? "true" : "false");
}

void syn_json_key_null(SYN_JsonWriter *w, const char *key)
{
    SYN_ASSERT(w != NULL);
    SYN_ASSERT(key != NULL);
    jw_comma(w);
    jw_str(w, key);
    jw_putc(w, ':');
    jw_puts(w, "null");
}

void syn_json_val_str(SYN_JsonWriter *w, const char *val)
{
    SYN_ASSERT(w != NULL);
    jw_comma(w);
    jw_str(w, val != NULL ? val : "");
}

void syn_json_val_int(SYN_JsonWriter *w, int32_t val)
{
    SYN_ASSERT(w != NULL);
    jw_comma(w);
    jw_int(w, val);
}

void syn_json_val_uint(SYN_JsonWriter *w, uint32_t val)
{
    SYN_ASSERT(w != NULL);
    jw_comma(w);
    jw_uint(w, val);
}

void syn_json_val_bool(SYN_JsonWriter *w, bool val)
{
    SYN_ASSERT(w != NULL);
    jw_comma(w);
    jw_puts(w, val ? "true" : "false");
}

#endif /* SYN_USE_JSON */
