#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_JSON) || SYN_USE_JSON

/**
 * @file syn_json_read.c
 * @brief Minimal JSON reader — in-place tokenizer.
 */

#include "syn_json_read.h"
#include "../util/syn_assert.h"

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
 * @brief Parse a signed integer from a string.
 * @param s  Input string.
 * @return Parsed value.
 */
static int32_t parse_int(const char *s)
{
    bool neg = false;
    if (*s == '-') { neg = true; s++; }

    int32_t val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    return neg ? -val : val;
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
    if (p >= end || *p != '"') return NULL;
    p++; /* skip opening quote */
    *out = p;

    while (p < end) {
        if (*p == '\\') {
            /* Skip escaped char (we don't unescape in-place for simplicity) */
            p++;
            if (p < end) p++;
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
    if (p >= end) return NULL;

    if (*p == '"') {
        /* String */
        p++;
        while (p < end) {
            if (*p == '\\') { p += 2; continue; }
            if (*p == '"') return p + 1;
            p++;
        }
        return NULL;
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
                    if (*p == '\\') p++;
                    p++;
                }
                if (p < end) p++; /* skip closing quote */
                continue;
            }
            if (*p == open) depth++;
            if (*p == close) depth--;
            p++;
        }
        return (depth == 0) ? p : NULL;
    }

    /* Number, bool, or null — read until delimiter */
    while (p < end && *p != ',' && *p != '}' && *p != ']' &&
           *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
        p++;
    }
    return p;
}

/**
 * @brief Recursive JSON object parser.
 *
 * Tokenizes key-value pairs at the given depth.
 *
 * @param r      JSON reader instance.
 * @param p      Current position.
 * @param end    End of buffer.
 * @param depth  Current nesting depth.
 * @return Pointer past the object.
 */
static char *parse_object(SYN_JsonReader *r, char *p, const char *end,
                            uint8_t depth)
{
    p = skip_ws(p, end);
    if (p >= end || *p != '{') return NULL;
    p++; /* skip '{' */

    for (;;) {
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == '}') return p + 1;

        /* Parse key */
        const char *key = NULL;
        p = parse_string(p, end, &key);
        if (p == NULL) return NULL;

        p = skip_ws(p, end);
        if (p >= end || *p != ':') return NULL;
        p++; /* skip ':' */

        p = skip_ws(p, end);
        if (p >= end) return NULL;

        /* Check if we have room for another token */
        if (r->token_count >= SYN_JSON_MAX_TOKENS) {
            /* Skip the value */
            p = skip_value(p, end);
            if (p == NULL) return NULL;
        } else if (*p == '{') {
            /* Nested object — add a marker token and recurse */
            SYN_JsonToken *tok = &r->tokens[r->token_count++];
            tok->type = SYN_JSON_OBJECT;
            tok->key = key;
            tok->value = NULL;
            tok->int_val = 0;
            tok->depth = depth;

            p = parse_object(r, p, end, depth + 1);
            if (p == NULL) return NULL;
        } else if (*p == '[') {
            /* Array — add a marker token, skip for now */
            SYN_JsonToken *tok = &r->tokens[r->token_count++];
            tok->type = SYN_JSON_ARRAY;
            tok->key = key;
            tok->value = NULL;
            tok->int_val = 0;
            tok->depth = depth;

            p = skip_value(p - 0, end); /* skip_value handles '[' */
            if (p == NULL) return NULL;
        } else if (*p == '"') {
            /* String value */
            const char *val = NULL;
            p = parse_string(p, end, &val);
            if (p == NULL) return NULL;

            SYN_JsonToken *tok = &r->tokens[r->token_count++];
            tok->type = SYN_JSON_STRING;
            tok->key = key;
            tok->value = val;
            tok->int_val = 0;
            tok->depth = depth;
        } else {
            /* Number, bool, or null */
            const char *val_start = p;
            p = skip_value(p, end);
            if (p == NULL) return NULL;

            /* Null-terminate the value by inserting \0 */
            char saved = *p;
            *p = '\0';

            SYN_JsonToken *tok = &r->tokens[r->token_count++];
            tok->key = key;
            tok->value = val_start;
            tok->depth = depth;

            if (val_start[0] == 't' || val_start[0] == 'f') {
                tok->type = SYN_JSON_BOOL;
                tok->int_val = (val_start[0] == 't') ? 1 : 0;
            } else if (memcmp(val_start, "null", 4) == 0) {
                tok->type = SYN_JSON_NULL;
                tok->int_val = 0;
            } else {
                tok->type = SYN_JSON_NUMBER;
                tok->int_val = parse_int(val_start);
            }

            /* Restore delimiter if needed for further parsing */
            if (saved != ',' && saved != '}' && saved != ']') {
                /* Was whitespace — no need to restore */
            } else {
                *p = saved;
            }
        }

        /* Next element or end of object */
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == ',') { p++; continue; }
        if (*p == '}') return p + 1;
        return NULL; /* unexpected char */
    }
}

/* ── API ───────────────────────────────────────────────────────────────── */

bool syn_json_parse(SYN_JsonReader *r, char *json, size_t len)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(json != NULL);

    memset(r, 0, sizeof(*r));

    const char *end = json + len;
    char *p = skip_ws(json, end);

    if (p >= end || *p != '{') {
        r->valid = false;
        return false;
    }

    const char *result = parse_object(r, p, end, 0);
    r->valid = (result != NULL);
    return r->valid;
}

const SYN_JsonToken *syn_json_find(const SYN_JsonReader *r, const char *key)
{
    SYN_ASSERT(r != NULL);
    SYN_ASSERT(key != NULL);

    if (!r->valid) return NULL;

    /* Check for dot notation: "parent.child" */
    const char *dot = strchr(key, '.');

    if (dot == NULL) {
        /* Simple key lookup at depth 0 */
        for (size_t i = 0; i < r->token_count; i++) {
            const SYN_JsonToken *tok = &r->tokens[i];
            if (tok->depth == 0 && tok->key != NULL &&
                strcmp(tok->key, key) == 0) {
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
        if (tok->depth == 0 && tok->type == SYN_JSON_OBJECT &&
            tok->key != NULL && strlen(tok->key) == parent_len &&
            memcmp(tok->key, key, parent_len) == 0) {
            parent_idx = i;
            found_parent = true;
            break;
        }
    }

    if (!found_parent) return NULL;

    /* Search tokens after the parent for child at depth 1 */
    for (size_t i = parent_idx + 1; i < r->token_count; i++) {
        const SYN_JsonToken *tok = &r->tokens[i];
        if (tok->depth < 1) break; /* left the parent object */
        if (tok->depth == 1 && tok->key != NULL &&
            strcmp(tok->key, child_key) == 0) {
            return tok;
        }
    }

    return NULL;
}

bool syn_json_get_str(const SYN_JsonReader *r, const char *key,
                       char *out, size_t out_sz)
{
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if (tok == NULL || tok->type != SYN_JSON_STRING) return false;
    if (tok->value == NULL) return false;

    size_t vlen = strlen(tok->value);
    if (vlen >= out_sz) vlen = out_sz - 1;
    memcpy(out, tok->value, vlen);
    out[vlen] = '\0';
    return true;
}

bool syn_json_get_int(const SYN_JsonReader *r, const char *key, int32_t *out)
{
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if (tok == NULL || tok->type != SYN_JSON_NUMBER) return false;
    *out = tok->int_val;
    return true;
}

bool syn_json_get_bool(const SYN_JsonReader *r, const char *key, bool *out)
{
    const SYN_JsonToken *tok = syn_json_find(r, key);
    if (tok == NULL || tok->type != SYN_JSON_BOOL) return false;
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
