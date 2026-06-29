#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CBOR) || SYN_USE_CBOR

/**
 * @file syn_cbor_read.c
 * @brief CBOR decoder implementation.
 */

#include "syn_cbor_read.h"
#include <string.h>


/**
 * @brief Consume and return one byte from the CBOR stream.
 * @param r  CBOR reader.
 * @return Consumed byte, or 0 on underrun.
 */
static uint8_t consume_byte(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    return r->buf[r->pos++];
}


/**
 * @brief Peek at the next byte without consuming it.
 * @param r  CBOR reader.
 * @return Next byte, or 0 if at end.
 */
static uint8_t peek_byte(const SYN_CborReader *r)
{
    if (r->pos >= r->len) return 0u;
    return r->buf[r->pos];
}


/**
 * @brief Decode a CBOR argument from the bottom-5-bit info field.
 *
 * Reads the argument value associated with @p info. Consumes 0, 1, 2, 4,
 * or 8 additional bytes. Sets `r->ok = false` for reserved info values.
 *
 * @param r     CBOR reader.
 * @param info  Bottom 5 bits of the header byte.
 * @return Decoded argument value.
 */
static uint64_t decode_arg(SYN_CborReader *r, uint8_t info)
{
    if (info <= 23u) {
        return (uint64_t)info;
    }
    switch (info) {
    case 24u: return (uint64_t)consume_byte(r);
    case 25u: {
        uint64_t hi = consume_byte(r);
        uint64_t lo = consume_byte(r);
        return (hi << 8u) | lo;
    }
    case 26u: {
        uint64_t b3 = consume_byte(r);
        uint64_t b2 = consume_byte(r);
        uint64_t b1 = consume_byte(r);
        uint64_t b0 = consume_byte(r);
        return (b3 << 24u) | (b2 << 16u) | (b1 << 8u) | b0;
    }
    case 27u: {
        uint64_t v = 0u;
        uint8_t  i;
        for (i = 0u; i < 8u; i++) {
            v = (v << 8u) | (uint64_t)consume_byte(r);
        }
        return v;
    }
    default:
        r->ok = false;
        return 0u;
    }
}


void syn_cbor_reader_init(SYN_CborReader *r,
                          const uint8_t  *buf,
                          size_t          len)
{
    r->buf = buf;
    r->len = len;
    r->pos = 0u;
    r->ok  = true;
}


SYN_CborType syn_cbor_peek_type(const SYN_CborReader *r)
{
    if (r->pos >= r->len) return SYN_CBOR_ERROR;
    uint8_t b    = peek_byte(r);
    uint8_t major = b >> 5u;
    uint8_t info  = b & 0x1Fu;

    if (major <= 5u) {
        return (SYN_CborType)major;
    }
    /* Major type 7: distinguish float, bool, null */
    if (info == 20u || info == 21u) return SYN_CBOR_BOOL;
    if (info == 22u)                return SYN_CBOR_NULL;
    if (info == 25u || info == 26u || info == 27u) return SYN_CBOR_FLOAT;
    return SYN_CBOR_ERROR;
}


/** @brief Maximum nesting depth for syn_cbor_skip. */
#define SKIP_MAX_DEPTH 8u

void syn_cbor_skip(SYN_CborReader *r)
{
    size_t pending[SKIP_MAX_DEPTH]; /* remaining items at each depth */
    uint8_t depth = 0u;
    pending[0] = 1u;                /* we must skip exactly 1 top-level item */

    while (depth < SKIP_MAX_DEPTH && pending[depth] > 0u) {
        pending[depth]--;

        if (r->pos >= r->len) { r->ok = false; return; }
        uint8_t b     = consume_byte(r);
        uint8_t major = b >> 5u;
        uint8_t info  = b & 0x1Fu;

        if (major == 7u) {
            /* float32: 4 more bytes; float16: 2; float64: 8; else 0 */
            if (info == 25u) { r->pos += 2u; }
            else if (info == 26u) { r->pos += 4u; }
            else if (info == 27u) { r->pos += 8u; }
            /* bool/null/simple: 0 extra bytes */
        } else if (major == 2u || major == 3u) {
            /* byte/text string: skip 'arg' data bytes */
            uint64_t n = decode_arg(r, info);
            r->pos += (size_t)n;
            if (r->pos > r->len) { r->ok = false; return; }
        } else if (major == 4u) {
            /* array: push 'arg' items */
            uint64_t n = decode_arg(r, info);
            depth++;
            if (depth >= SKIP_MAX_DEPTH) { r->ok = false; return; }
            pending[depth] = (size_t)n;
        } else if (major == 5u) {
            /* map: push 'arg * 2' items (each pair = key + value) */
            uint64_t n = decode_arg(r, info);
            depth++;
            if (depth >= SKIP_MAX_DEPTH) { r->ok = false; return; }
            pending[depth] = (size_t)(n * 2u);
        } else {
            /* major type 0 or 1: uint/int, arg is the value, no data bytes */
            decode_arg(r, info);
        }

        /* Pop completed levels */
        while (depth > 0u && pending[depth] == 0u) {
            depth--;
        }
    }
#undef SKIP_MAX_DEPTH
}

/* ── Collections ─────────────────────────────────────────────────────────── */

size_t syn_cbor_read_map_begin(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return 0u; }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 5u) { r->ok = false; return 0u; }
    return (size_t)decode_arg(r, b & 0x1Fu);
}

size_t syn_cbor_read_array_begin(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return 0u; }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 4u) { r->ok = false; return 0u; }
    return (size_t)decode_arg(r, b & 0x1Fu);
}

/* ── Scalars ─────────────────────────────────────────────────────────────── */

uint64_t syn_cbor_read_uint(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return 0u; }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 0u) { r->ok = false; return 0u; }
    return decode_arg(r, b & 0x1Fu);
}

int64_t syn_cbor_read_int(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return 0; }
    uint8_t b     = consume_byte(r);
    uint8_t major = b >> 5u;
    uint8_t info  = b & 0x1Fu;

    if (major == 0u) {
        return (int64_t)decode_arg(r, info);
    }
    if (major == 1u) {
        return (int64_t)(-1 - (int64_t)decode_arg(r, info));
    }
    r->ok = false;
    return 0;
}

float syn_cbor_read_float(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return 0.0f; }
    uint8_t b = consume_byte(r);
    /* Must be major type 7, info=26 (float32) */
    if (b != 0xFAu) { r->ok = false; return 0.0f; }
    uint32_t bits = 0u;
    bits |= (uint32_t)consume_byte(r) << 24u;
    bits |= (uint32_t)consume_byte(r) << 16u;
    bits |= (uint32_t)consume_byte(r) <<  8u;
    bits |= (uint32_t)consume_byte(r);
    float v = 0.0f;
    memcpy(&v, &bits, sizeof(v));
    return v;
}

bool syn_cbor_read_bool(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return false; }
    uint8_t b = consume_byte(r);
    if (b == 0xF5u) return true;
    if (b == 0xF4u) return false;
    r->ok = false;
    return false;
}

void syn_cbor_read_null(SYN_CborReader *r)
{
    if (r->pos >= r->len) { r->ok = false; return; }
    uint8_t b = consume_byte(r);
    if (b != 0xF6u) { r->ok = false; }
}

/* ── Strings ─────────────────────────────────────────────────────────────── */

size_t syn_cbor_read_text(SYN_CborReader *r, char *buf, size_t cap)
{
    if (r->pos >= r->len) { r->ok = false; return 0u; }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 3u) { r->ok = false; return 0u; }
    size_t n = (size_t)decode_arg(r, b & 0x1Fu);

    size_t copy = (n < cap) ? n : (cap > 0u ? cap - 1u : 0u);
    if (buf != NULL && cap > 0u) {
        memcpy(buf, r->buf + r->pos, copy);
        buf[copy] = '\0';
    }
    r->pos += n;
    if (r->pos > r->len) { r->ok = false; }
    return n;
}

size_t syn_cbor_read_bytes(SYN_CborReader *r, uint8_t *buf, size_t cap)
{
    if (r->pos >= r->len) { r->ok = false; return 0u; }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 2u) { r->ok = false; return 0u; }
    size_t n = (size_t)decode_arg(r, b & 0x1Fu);

    size_t copy = (n < cap) ? n : cap;
    if (buf != NULL) {
        memcpy(buf, r->buf + r->pos, copy);
    }
    r->pos += n;
    if (r->pos > r->len) { r->ok = false; }
    return n;
}

#endif /* SYN_USE_CBOR */
