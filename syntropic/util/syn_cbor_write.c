#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CBOR) || SYN_USE_CBOR

/**
 * @file syn_cbor_write.c
 * @brief CBOR streaming encoder implementation.
 */

#include "syn_cbor_write.h"
#include <string.h>

/**
 * @brief Emit a single byte to the CBOR output.
 * @param w  CBOR writer.
 * @param b  Byte to emit.
 */
static void emit_byte(SYN_CborWriter *w, uint8_t b)
{
    if (w->overflow) return;
    if (w->len >= w->cap) {
        w->overflow = true;
        return;
    }
    w->buf[w->len++] = b;
}

/**
 * @brief Emit N raw bytes.
 * @param w     CBOR writer.
 * @param data  Source data.
 * @param n     Number of bytes.
 */
static void emit_raw(SYN_CborWriter *w, const void *data, size_t n)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t i;
    for (i = 0u; i < n; i++) {
        emit_byte(w, p[i]);
    }
}

/**
 * @brief Encode a CBOR header (major type + argument).
 *
 * Follows the CBOR additional-info rules for all major types 0-5.
 *
 * @param w      CBOR writer.
 * @param major  Major type (0-7).
 * @param val    Argument value.
 */
static void emit_head(SYN_CborWriter *w, uint8_t major, uint64_t val)
{
    uint8_t mt = (uint8_t)((major & 0x07u) << 5u);

    if (val <= 23u) {
        emit_byte(w, mt | (uint8_t)val);
    } else if (val <= 0xFFu) {
        emit_byte(w, mt | 24u);
        emit_byte(w, (uint8_t)val);
    } else if (val <= 0xFFFFu) {
        emit_byte(w, mt | 25u);
        emit_byte(w, (uint8_t)(val >> 8u));
        emit_byte(w, (uint8_t)(val));
    } else if (val <= 0xFFFFFFFFu) {
        emit_byte(w, mt | 26u);
        emit_byte(w, (uint8_t)(val >> 24u));
        emit_byte(w, (uint8_t)(val >> 16u));
        emit_byte(w, (uint8_t)(val >>  8u));
        emit_byte(w, (uint8_t)(val));
    } else {
        emit_byte(w, mt | 27u);
        emit_byte(w, (uint8_t)(val >> 56u));
        emit_byte(w, (uint8_t)(val >> 48u));
        emit_byte(w, (uint8_t)(val >> 40u));
        emit_byte(w, (uint8_t)(val >> 32u));
        emit_byte(w, (uint8_t)(val >> 24u));
        emit_byte(w, (uint8_t)(val >> 16u));
        emit_byte(w, (uint8_t)(val >>  8u));
        emit_byte(w, (uint8_t)(val));
    }
}

void syn_cbor_writer_init(SYN_CborWriter *w, uint8_t *buf, size_t cap)
{
    w->buf      = buf;
    w->cap      = cap;
    w->len      = 0u;
    w->overflow = false;
}

/* ── Collections ─────────────────────────────────────────────────────────── */

void syn_cbor_write_map_begin(SYN_CborWriter *w, size_t count)
{
    emit_head(w, 5u, (uint64_t)count);
}

void syn_cbor_write_array_begin(SYN_CborWriter *w, size_t count)
{
    emit_head(w, 4u, (uint64_t)count);
}

/* ── Scalars ─────────────────────────────────────────────────────────────── */

void syn_cbor_write_uint(SYN_CborWriter *w, uint64_t v)
{
    emit_head(w, 0u, v);
}

void syn_cbor_write_int(SYN_CborWriter *w, int64_t v)
{
    if (v >= 0) {
        emit_head(w, 0u, (uint64_t)v);
    } else {
        /* CBOR negative: major type 1, value = -1 - v */
        emit_head(w, 1u, (uint64_t)(-1 - v));
    }
}

void syn_cbor_write_float(SYN_CborWriter *w, float v)
{
    /* 0xFA = major type 7, info=26 (float32) */
    emit_byte(w, 0xFAu);
    /* Reinterpret float bits as uint32 — no UB via memcpy */
    uint32_t bits = 0u;
    memcpy(&bits, &v, sizeof(bits));
    /* Emit big-endian */
    emit_byte(w, (uint8_t)(bits >> 24u));
    emit_byte(w, (uint8_t)(bits >> 16u));
    emit_byte(w, (uint8_t)(bits >>  8u));
    emit_byte(w, (uint8_t)(bits));
}

void syn_cbor_write_bool(SYN_CborWriter *w, bool v)
{
    emit_byte(w, v ? 0xF5u : 0xF4u);
}

void syn_cbor_write_null(SYN_CborWriter *w)
{
    emit_byte(w, 0xF6u);
}

/* ── Strings ─────────────────────────────────────────────────────────────── */

void syn_cbor_write_text(SYN_CborWriter *w, const char *str, size_t len)
{
    emit_head(w, 3u, (uint64_t)len);
    emit_raw(w, str, len);
}

void syn_cbor_write_text_cstr(SYN_CborWriter *w, const char *str)
{
    size_t len = strlen(str);
    syn_cbor_write_text(w, str, len);
}

void syn_cbor_write_bytes(SYN_CborWriter *w, const uint8_t *data, size_t len)
{
    emit_head(w, 2u, (uint64_t)len);
    emit_raw(w, data, len);
}

#endif /* SYN_USE_CBOR */
