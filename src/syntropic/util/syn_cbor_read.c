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
    /* LCOV_EXCL_START: Unreachable static helper guard (caller guarantees pos < len) */
    if (r->pos >= r->len)
        return 0u;
    /* LCOV_EXCL_STOP */
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
    case 24u:
        return (uint64_t)consume_byte(r);
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
        uint8_t i;
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

void syn_cbor_reader_init(SYN_CborReader *r, const uint8_t *buf, size_t len)
{
    r->buf = buf;
    r->len = len;
    r->pos = 0u;
    r->ok = true;
}

SYN_CborType syn_cbor_peek_type(const SYN_CborReader *r)
{
    if (r->pos >= r->len)
        return SYN_CBOR_ERROR;
    uint8_t b = peek_byte(r);
    uint8_t major = b >> 5u;
    uint8_t info = b & 0x1Fu;

    if (major <= 5u) {
        return (SYN_CborType)major;
    }
    /* Major type 7: distinguish float, bool, null */
    if (info == 20u || info == 21u)
        return SYN_CBOR_BOOL;
    if (info == 22u)
        return SYN_CBOR_NULL;
    if (info == 25u || info == 26u || info == 27u)
        return SYN_CBOR_FLOAT;
    return SYN_CBOR_ERROR;
}

/** @brief Maximum nesting depth for syn_cbor_skip. */
#define SKIP_MAX_DEPTH 8u

/**
 * @brief Skip major type 7 payload bytes (floats, bool, null, simple).
 * @param r    CBOR reader state.
 * @param info 5-bit additional info field.
 * @return true if sufficient unread bytes exist, false on truncation.
 */
static bool skip_major7(SYN_CborReader *r, uint8_t info)
{
    if (info >= 28u) {
        r->ok = false;
        return false;
    }
    size_t skip_bytes = 0u;
    if (info == 24u) {
        skip_bytes = 1u;
    } else if (info == 25u) {
        skip_bytes = 2u;
    } else if (info == 26u) {
        skip_bytes = 4u;
    } else if (info == 27u) {
        skip_bytes = 8u;
    }
    if ((r->len - r->pos) < skip_bytes) {
        r->ok = false;
        return false;
    }
    r->pos += skip_bytes;
    return true;
}

/**
 * @brief Validate and push nested collection onto skip stack.
 * @param r         CBOR reader state.
 * @param count     Declared element or pair count.
 * @param item_mult Items per declared entry (1 for array, 2 for map).
 * @param pending   Remaining item counters per nesting level.
 * @param depth     Current nesting depth pointer.
 * @return true if collection fits bounds and depth limit, false otherwise.
 */
static bool skip_push(SYN_CborReader *r, uint64_t count, size_t item_mult, size_t *pending,
                      uint8_t *depth)
{
    if (!r->ok || count > (uint64_t)((r->len - r->pos) / item_mult) ||
        *depth + 1u >= SKIP_MAX_DEPTH) {
        r->ok = false;
        return false;
    }
    (*depth)++;
    pending[*depth] = (size_t)count * item_mult;
    return true;
}

/**
 * @brief Advance or push a single CBOR item during syn_cbor_skip.
 * @param r       CBOR reader state.
 * @param b       Initial header byte of the CBOR item.
 * @param pending Remaining item counters per nesting level.
 * @param depth   Current nesting depth pointer.
 * @return true on valid item advance/push, false on malformed or truncated data.
 */
static bool skip_step(SYN_CborReader *r, uint8_t b, size_t *pending, uint8_t *depth)
{
    uint8_t major = b >> 5u;
    uint8_t info = b & 0x1Fu;

    if (major == 7u) {
        return skip_major7(r, info);
    }
    if (major == 2u || major == 3u) {
        uint64_t n = decode_arg(r, info);
        if (!r->ok || n > (uint64_t)(r->len - r->pos)) {
            r->ok = false;
            return false;
        }
        r->pos += (size_t)n;
        return true;
    }
    if (major == 4u) {
        return skip_push(r, decode_arg(r, info), 1u, pending, depth);
    }
    if (major == 5u) {
        return skip_push(r, decode_arg(r, info), 2u, pending, depth);
    }
    if (major == 6u) {
        (void)decode_arg(r, info);
        return skip_push(r, 1u, 1u, pending, depth);
    }
    (void)decode_arg(r, info);
    return r->ok;
}

void syn_cbor_skip(SYN_CborReader *r)
{
    size_t pending[SKIP_MAX_DEPTH]; /* remaining items at each depth */
    uint8_t depth = 0u;
    pending[0] = 1u; /* we must skip exactly 1 top-level item */

    while (depth < SKIP_MAX_DEPTH && pending[depth] > 0u) {
        pending[depth]--;

        if (r->pos >= r->len) {
            r->ok = false;
            return;
        }
        if (!skip_step(r, consume_byte(r), pending, &depth)) {
            return;
        }

        /* Pop completed levels */
        while (depth > 0u && pending[depth] == 0u) {
            depth--;
        }
    }
}
#undef SKIP_MAX_DEPTH

/* ── Collections ─────────────────────────────────────────────────────────── */

size_t syn_cbor_read_map_begin(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 5u) {
        r->ok = false;
        return 0u;
    }
    uint64_t val = decode_arg(r, b & 0x1Fu);
    if (!r->ok || val > (uint64_t)((r->len - r->pos) / 2u)) {
        r->ok = false;
        return 0u;
    }
    return (size_t)val;
}

size_t syn_cbor_read_array_begin(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 4u) {
        r->ok = false;
        return 0u;
    }
    uint64_t val = decode_arg(r, b & 0x1Fu);
    if (!r->ok || val > (uint64_t)(r->len - r->pos)) {
        r->ok = false;
        return 0u;
    }
    return (size_t)val;
}

/* ── Scalars ─────────────────────────────────────────────────────────────── */

uint64_t syn_cbor_read_uint(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 0u) {
        r->ok = false;
        return 0u;
    }
    return decode_arg(r, b & 0x1Fu);
}

int64_t syn_cbor_read_int(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0;
    }
    uint8_t b = consume_byte(r);
    uint8_t major = b >> 5u;
    uint8_t info = b & 0x1Fu;

    if ((major == 0u) || (major == 1u)) {
        uint64_t arg = decode_arg(r, info);
        if ((!r->ok) || (arg > (uint64_t)INT64_MAX)) {
            r->ok = false;
            return 0;
        }
        return (major == 0u) ? (int64_t)arg : (-1 - (int64_t)arg);
    }
    r->ok = false;
    return 0;
}

float syn_cbor_read_float(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0.0f;
    }
    uint8_t b = consume_byte(r);
    /* Must be major type 7, info=26 (float32) */
    if (b != 0xFAu) {
        r->ok = false;
        return 0.0f;
    }
    uint32_t bits = 0u;
    bits |= (uint32_t)consume_byte(r) << 24u;
    bits |= (uint32_t)consume_byte(r) << 16u;
    bits |= (uint32_t)consume_byte(r) << 8u;
    bits |= (uint32_t)consume_byte(r);
    if (!r->ok) {
        return 0.0f;
    }
    float v = 0.0f;
    memcpy(&v, &bits, sizeof(v));
    return v;
}

bool syn_cbor_read_bool(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return false;
    }
    uint8_t b = consume_byte(r);
    if (b == 0xF5u)
        return true;
    if (b == 0xF4u)
        return false;
    r->ok = false;
    return false;
}

void syn_cbor_read_null(SYN_CborReader *r)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return;
    }
    uint8_t b = consume_byte(r);
    if (b != 0xF6u) {
        r->ok = false;
    }
}

/* ── Strings ─────────────────────────────────────────────────────────────── */

size_t syn_cbor_read_text(SYN_CborReader *r, char *buf, size_t cap)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 3u) {
        r->ok = false;
        return 0u;
    }
    uint64_t raw_n = decode_arg(r, b & 0x1Fu);
    size_t available = (r->pos < r->len) ? (r->len - r->pos) : 0u;
    size_t n = (raw_n > (uint64_t)SIZE_MAX) ? SIZE_MAX : (size_t)raw_n;
    if (!r->ok || raw_n > (uint64_t)available) {
        r->ok = false;
    }
    size_t copy = (n < cap) ? n : (cap > 0u ? cap - 1u : 0u);
    if (copy > available) {
        copy = available;
    }
    if (buf != NULL && cap > 0u) {
        if (copy > 0u) {
            memcpy(buf, r->buf + r->pos, copy);
        }
        buf[copy] = '\0';
    }
    if (r->ok && raw_n <= (uint64_t)available) {
        r->pos += n;
    } else {
        r->pos = r->len;
        r->ok = false;
    }
    return n;
}

size_t syn_cbor_read_bytes(SYN_CborReader *r, uint8_t *buf, size_t cap)
{
    if (r->pos >= r->len) {
        r->ok = false;
        return 0u;
    }
    uint8_t b = consume_byte(r);
    if ((b >> 5u) != 2u) {
        r->ok = false;
        return 0u;
    }
    uint64_t raw_n = decode_arg(r, b & 0x1Fu);
    size_t available = (r->pos < r->len) ? (r->len - r->pos) : 0u;
    size_t n = (raw_n > (uint64_t)SIZE_MAX) ? SIZE_MAX : (size_t)raw_n;
    if (!r->ok || raw_n > (uint64_t)available) {
        r->ok = false;
    }
    size_t copy = (n < cap) ? n : cap;
    if (copy > available) {
        copy = available;
    }
    if (buf != NULL && copy > 0u) {
        memcpy(buf, r->buf + r->pos, copy);
    }
    if (r->ok && raw_n <= (uint64_t)available) {
        r->pos += n;
    } else {
        r->pos = r->len;
        r->ok = false;
    }
    return n;
}

#endif /* SYN_USE_CBOR */
