#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_JSON) || SYN_USE_JSON

/**
 * @file syn_json_read.c
 * @brief Minimal JSON reader — in-place tokenizer.
 */

#include "../util/syn_assert.h"
#include "syn_json_read.h"

#include <string.h>

/* ── Internal helpers ──────────────────────────────────────────────────── */

/**
 * @brief Skip whitespace.
 * @param p    Current position.
 * @param end  End of buffer.
 * @return Pointer to next non-whitespace char.
 */
static char *skip_ws(char *p, const char *end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        p++;
    }
    return p;
}

/**
 * @brief Parse a signed 32-bit integer from a bounded span with saturation.
 * @param s      Start of numeric token.
 * @param s_end  One-past-end of numeric token.
 * @return Parsed int32_t value (saturated to [INT32_MIN, INT32_MAX]).
 */
static int32_t parse_int_span(const char *s, const char *s_end)
{
    bool neg = false;
    if (s < s_end && *s == '-') {
        neg = true;
        s++;
    }

    uint32_t limit = neg ? ((uint32_t)INT32_MAX + 1U) : (uint32_t)INT32_MAX;
    uint32_t acc = 0U;
    while (s < s_end && *s >= '0' && *s <= '9') {
        uint32_t digit = (uint32_t)(*s - '0');
        if (acc > (limit - digit) / 10U) {
            acc = limit;
        } else {
            acc = (acc * 10U) + digit;
        }
        s++;
    }

    if (neg) {
        return (acc == ((uint32_t)INT32_MAX + 1U)) ? INT32_MIN : -(int32_t)acc;
    }
    return (int32_t)acc;
}

/**
 * @brief Parse a JSON string starting at p.
 *
 * Must point to opening '"'. Null-terminates in-place.
 *
 * @param p    Current position (at opening '"').
 * @param end  End of buffer.
 * @param out  [out] Start of the string content.
 * @return Pointer past the closing '"', or NULL on error.
 */
static char *parse_string(char *p, const char *end, const char **out)
{
    if (p >= end || *p != '"')
        return NULL;
    p++; /* skip opening quote */
    *out = p;

    while (p < end) {
        if (*p == '\\') {
            /* Skip escaped char (we don't unescape in-place for simplicity) */
            p++;
            if (p < end)
                p++;
            continue;
        }
        if (*p == '"') {
            *p = '\0'; /* null-terminate */
            return p + 1;
        }
        p++;
    }
    return NULL; /* unterminated string */
}

/**
 * @brief Skip a JSON value (any type).
 * @param p    Current position.
 * @param end  End of buffer.
 * @return Pointer past the value.
 */
static char *skip_value(char *p, const char *end)
{
    p = skip_ws(p, end);
    if (p >= end)
        return NULL; /* LCOV_EXCL_LINE: Unreachable guard; caller checks p < end before invoking
                        skip_value */

    if (*p == '"') {
        /* String */
        p++;
        while (p < end) {
            if (*p == '\\') {
                p++;
                if (p < end)
                    p++;
                continue;
            }
            if (*p == '"')
                return p + 1;
            p++;
        }
        return NULL; /* LCOV_EXCL_LINE: Unterminated string in skip_value */
    }

    if (*p == '{' || *p == '[') {
        /* Object or array — count nesting */
        char open = *p;
        char close = (open == '{') ? '}' : ']';
        int depth = 1;
        p++;
        while (p < end && depth > 0) {
            if (*p == '"') {
                p++;
                while (p < end && *p != '"') {
                    if (*p == '\\') {
                        p++;
                        if (p < end)
                            p++;
                        continue;
                    }
                    p++;
                }
                if (p < end)
                    p++; /* skip closing quote */
                continue;
            }
            if (*p == open)
                depth++;
            if (*p == close)
                depth--;
            p++;
        }
        return (depth == 0) ? p : NULL;
    }

    /* Number, bool, or null — read until delimiter */
    while (p < end && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && *p != '\t' &&
           *p != '\n' && *p != '\r') {
        p++;
    }
    return p;
}

/**
 * @brief Classify and parse a JSON scalar span (true, false, null, or integer).
 * @param tok       [out] Destination token.
 * @param val_start Start of scalar characters.
 * @param vlen      Exact length of scalar span.
 * @return true if valid JSON scalar token, false otherwise.
 */
static bool json_parse_scalar_token(SYN_JsonToken *tok, const char *val_start, size_t vlen)
{
    if (vlen == 4U && memcmp(val_start, "true", 4U) == 0) {
        tok->type = SYN_JSON_BOOL;
        tok->int_val = 1;
        return true;
    }
    if (vlen == 5U && memcmp(val_start, "false", 5U) == 0) {
        tok->type = SYN_JSON_BOOL;
        tok->int_val = 0;
        return true;
    }
    if (vlen == 4U && memcmp(val_start, "null", 4U) == 0) {
        tok->type = SYN_JSON_NULL;
        tok->int_val = 0;
        return true;
    }
    if ((val_start[0] >= '0' && val_start[0] <= '9') ||
        (val_start[0] == '-' && vlen > 1U && val_start[1] >= '0' && val_start[1] <= '9')) {
        tok->type = SYN_JSON_NUMBER;
        tok->int_val = parse_int_span(val_start, val_start + vlen);
        return true;
    }
    return false;
}

/** @brief Maximum supported JSON object nesting depth (iterative state bound). */
#define SYN_JSON_MAX_DEPTH 8U

/**
 * @brief Parse a JSON object member key and colon separator, positioning at value start.
 * @param p       Current buffer cursor at start of member key.
 * @param end     One-past-end of JSON buffer.
 * @param key_out [out] Null-terminated key string pointer.
 * @return Cursor positioned at start of member value, or NULL on syntax error.
 */
static char *json_parse_member_header(char *p, const char *end, const char **key_out)
{
    p = parse_string(p, end, key_out);
    if (p == NULL) {
        return NULL;
    }
    p = skip_ws(p, end);
    if ((p >= end) || (*p != ':')) {
        return NULL;
    }
    p = skip_ws(p + 1, end);
    return (p < end) ? p : NULL;
}

/**
 * @brief Parse or skip a non-object JSON member value (array, string, or scalar).
 * @param r     JSON reader instance.
 * @param p     Current buffer cursor at start of value.
 * @param end   One-past-end of JSON buffer.
 * @param key   Null-terminated member key string.
 * @param depth Current object nesting depth.
 * @return Cursor past the parsed value, or NULL on syntax error.
 */
static char *json_parse_non_object_value(SYN_JsonReader *r, char *p, const char *end,
                                         const char *key, uint8_t depth)
{
    if (r->token_count >= SYN_JSON_MAX_TOKENS) {
        return skip_value(p, end);
    }
    if (*p == '[') {
        SYN_JsonToken *tok = &r->tokens[r->token_count++];
        tok->type = SYN_JSON_ARRAY;
        tok->key = key;
        tok->value = NULL;
        tok->int_val = 0;
        tok->depth = depth;
        return skip_value(p, end);
    }
    if (*p == '"') {
        const char *val = NULL;
        p = parse_string(p, end, &val);
        if (p == NULL) {
            return NULL;
        }
        SYN_JsonToken *tok = &r->tokens[r->token_count++];
        tok->type = SYN_JSON_STRING;
        tok->key = key;
        tok->value = val;
        tok->int_val = 0;
        tok->depth = depth;
        return p;
    }

    const char *val_start = p;
    p = skip_value(p, end);
    if (p >= end) {
        return NULL;
    }

    size_t vlen = (size_t)(p - val_start);
    char saved = *p;
    *p = '\0';

    SYN_JsonToken *tok = &r->tokens[r->token_count];
    tok->key = key;
    tok->value = val_start;
    tok->depth = depth;
    if (!json_parse_scalar_token(tok, val_start, vlen)) {
        *p = saved;
        return NULL;
    }
    r->token_count++;

    if ((saved == ',') || (saved == '}') || (saved == ']')) {
        *p = saved;
    } else {
        p++;
    }
    return p;
}

/**
 * @brief Advance past post-value delimiters and pop closed object frames iteratively.
 * @param p     Current buffer cursor after a member value or at an empty object brace.
 * @param end   One-past-end of JSON buffer.
 * @param depth [in,out] Current open-object nesting depth.
 * @param done  [out] Set to true when the root object closes.
 * @return Updated cursor after ',' or root '}', or NULL on syntax/truncation error.
 */
static char *json_consume_delimiter(char *p, const char *end, uint8_t *depth, bool *done)
{
    for (;;) {
        p = skip_ws(p, end);
        if (p >= end) {
            return NULL;
        }
        if (*p == ',') {
            return p + 1;
        }
        if (*p != '}') {
            return NULL;
        }
        p++;
        if (*depth == 0U) {
            *done = true;
            return p;
        }
        (*depth)--;
    }
}

/**
 * @brief Iterative bounded-depth JSON object parser.
 *
 * Tokenizes key-value pairs up to SYN_JSON_MAX_DEPTH without recursion.
 *
 * @param r   JSON reader instance.
 * @param p   Current position (caller verified leading '{').
 * @param end End of buffer.
 * @return Pointer past the root closing '}', or NULL on error.
 */
static char *parse_object(SYN_JsonReader *r, char *p, const char *end)
{
    uint8_t depth = 0U;
    bool done = false;
    p++; /* skip root '{' */

    for (;;) {
        p = skip_ws(p, end);
        if (p >= end) {
            return NULL;
        }
        if (*p == '}') {
            p = json_consume_delimiter(p, end, &depth, &done);
            if ((p == NULL) || done) {
                return p;
            }
            continue;
        }

        const char *key = NULL;
        p = json_parse_member_header(p, end, &key);
        if (p == NULL) {
            return NULL;
        }

        if ((*p == '{') && (r->token_count < SYN_JSON_MAX_TOKENS)) {
            if ((depth + 1U) >= SYN_JSON_MAX_DEPTH) {
                return NULL;
            }
            SYN_JsonToken *tok = &r->tokens[r->token_count++];
            tok->type = SYN_JSON_OBJECT;
            tok->key = key;
            tok->value = NULL;
            tok->int_val = 0;
            tok->depth = depth;
            depth++;
            p++;
            continue;
        }

        p = json_parse_non_object_value(r, p, end, key, depth);
        if (p == NULL) {
            return NULL;
        }
        p = json_consume_delimiter(p, end, &depth, &done);
        if ((p == NULL) || done) {
            return p;
        }
    }
}

/* ── API ───────────────────────────────────────────────────────────────── */

bool syn_json_parse(SYN_JsonReader *r, char *json, size_t len)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(json != NULL);
    if (r == NULL || json == NULL || len == 0) {
        return false; /* LCOV_EXCL_LINE: Defensive NULL and zero len check after SYN_ASSERT macro in
                         release mode */
    }

    memset(r, 0, sizeof(*r));

    const char *end = json + len;
    char *p = skip_ws(json, end);

    if (p >= end || *p != '{') {
        r->valid = false;
        return false;
    }

    const char *result = parse_object(r, p, end);
    r->valid = (result != NULL);
    return r->valid;
}

const SYN_JsonToken *syn_json_find(const SYN_JsonReader *r, const char *key)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(key != NULL);

    if (r == NULL || key == NULL || !r->valid)
        return NULL;

    /* Check for dot notation: "parent.child" */
    const char *dot = strchr(key, '.');

    if (dot == NULL) {
        /* Simple key lookup at depth 0 */
        for (size_t i = 0; i < r->token_count; i++) {
            const SYN_JsonToken *tok = &r->tokens[i];
            if (tok->depth == 0 && tok->key != NULL && strcmp(tok->key, key) == 0) {
                return tok;
            }
        }
        return NULL;
    }

    /* Dot notation: find parent object, then child key */
    size_t parent_len = (size_t)(dot - key);
    const char *child_key = dot + 1;

    /* Find the parent object token */
    size_t parent_idx = 0;
    bool found_parent = false;
    for (size_t i = 0; i < r->token_count; i++) {
        const SYN_JsonToken *tok = &r->tokens[i];
        if (tok->depth == 0 && tok->type == SYN_JSON_OBJECT && tok->key != NULL &&
            strlen(tok->key) == parent_len && memcmp(tok->key, key, parent_len) == 0) {
            parent_idx = i;
            found_parent = true;
            break;
        }
    }

    if (!found_parent)
        return NULL;

    /* Search tokens after the parent for child at depth 1 */
    for (size_t i = parent_idx + 1; i < r->token_count; i++) {
        const SYN_JsonToken *tok = &r->tokens[i];
        if (tok->depth < 1)
            break; /* left the parent object */
        if (tok->depth == 1 && tok->key != NULL && strcmp(tok->key, child_key) == 0) {
            return tok;
        }
    }

    return NULL;
}

bool syn_json_get_str(const SYN_JsonReader *r, const char *key, char *out, size_t out_sz)
{
    if (r == NULL || key == NULL || out == NULL || out_sz == 0) {
        return false;
    }
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if (tok == NULL || tok->type != SYN_JSON_STRING)
        return false;
    if (tok->value == NULL)
        return false; /* LCOV_EXCL_LINE: Defensive string token value check */

    size_t vlen = strlen(tok->value);
    if (vlen >= out_sz)
        vlen = out_sz - 1;
    memcpy(out, tok->value, vlen);
    out[vlen] = '\0';
    return true;
}

bool syn_json_get_int(const SYN_JsonReader *r, const char *key, int32_t *out)
{
    if ((r == NULL) || (key == NULL) || (out == NULL)) {
        return false;
    }
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if ((tok == NULL) || (tok->type != SYN_JSON_NUMBER)) {
        return false;
    }
    *out = tok->int_val;
    return true;
}

bool syn_json_get_bool(const SYN_JsonReader *r, const char *key, bool *out)
{
    if ((r == NULL) || (key == NULL) || (out == NULL)) {
        return false;
    }
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if ((tok == NULL) || (tok->type != SYN_JSON_BOOL)) {
        return false;
    }
    *out = (tok->int_val != 0);
    return true;
}

bool syn_json_is_null(const SYN_JsonReader *r, const char *key)
{
    const SYN_JsonToken *tok = syn_json_find(r, key);
    return (tok != NULL && tok->type == SYN_JSON_NULL);
}

SYN_JsonType syn_json_get_type(const SYN_JsonReader *r, const char *key)
{
    const SYN_JsonToken *tok = syn_json_find(r, key);
    return (tok != NULL) ? tok->type : SYN_JSON_NONE;
}

#endif /* SYN_USE_JSON */
