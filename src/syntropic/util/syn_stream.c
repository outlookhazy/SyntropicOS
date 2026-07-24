/**
 * @file syn_stream.c
 * @brief Cooperative byte stream implementation.
 */

#include "syn_stream.h"
#include "syn_ringbuf.h"

#include <string.h>

/* ── Internal: delimiter scan ──────────────────────────────────────────── */

/**
 * @brief Scan the ringbuf for the delimiter byte.
 * @param s Stream to scan.
 * @return 1-based position of the first delimiter, or 0 if not found.
 */
static size_t stream_find_delimiter(const SYN_Stream *s)
{
    const SYN_RingBuf *rb = &s->rb;
    const size_t count = syn_ringbuf_count(rb);
    size_t idx = rb->tail;

    for (size_t i = 0; i < count; i++) {
        if (rb->buf[idx] == s->delimiter) {
            return i + 1;  /* 1-based position (byte count including delim) */
        }
        idx++;
        if (idx >= rb->size) idx = 0;
    }
    return 0;
}

/* ── Init / config ─────────────────────────────────────────────────────── */

void syn_stream_init(SYN_Stream *s, uint8_t *buf, size_t size)
{
    syn_ringbuf_init(&s->rb, buf, size);
    s->threshold = 0;
    s->delimiter = 0;
    s->delim_en  = false;
}

void syn_stream_set_threshold(SYN_Stream *s, size_t n)
{
    s->threshold = n;
}

void syn_stream_set_delimiter(SYN_Stream *s, uint8_t delim)
{
    s->delimiter = delim;
    s->delim_en  = true;
}

void syn_stream_clear_delimiter(SYN_Stream *s)
{
    s->delim_en = false;
}

/* ── Producer ──────────────────────────────────────────────────────────── */

size_t syn_stream_write(SYN_Stream *s, const uint8_t *data, size_t len)
{
    return syn_ringbuf_write(&s->rb, data, len);
}

bool syn_stream_put(SYN_Stream *s, uint8_t byte)
{
    return syn_ringbuf_put(&s->rb, byte);
}

/* ── Consumer ──────────────────────────────────────────────────────────── */

bool syn_stream_readable(const SYN_Stream *s)
{
    const size_t count = syn_ringbuf_count(&s->rb);
    if (count == 0) {
        return false;
    }

    /* Delimiter mode takes precedence */
    if (s->delim_en) {
        return stream_find_delimiter(s) > 0;
    }

    /* Threshold mode */
    if (s->threshold > 0) {
        return count >= s->threshold;
    }

    /* Default: any bytes */
    return true;
}

size_t syn_stream_read(SYN_Stream *s, uint8_t *buf, size_t max_len)
{
    return syn_ringbuf_read(&s->rb, buf, max_len);
}

size_t syn_stream_read_line(SYN_Stream *s, uint8_t *buf, size_t max_len)
{
    if (!s->delim_en) {
        /* No delimiter mode — behave like plain read */
        return syn_ringbuf_read(&s->rb, buf, max_len);
    }

    /* Find delimiter position */
    size_t delim_pos = stream_find_delimiter(s);
    if (delim_pos == 0) {
        return 0;  /* No delimiter found — don't consume anything */
    }

    /* Read up to delimiter (inclusive), capped by max_len */
    size_t to_read = (delim_pos < max_len) ? delim_pos : max_len;
    return syn_ringbuf_read(&s->rb, buf, to_read);
}

size_t syn_stream_count(const SYN_Stream *s)
{
    return syn_ringbuf_count(&s->rb);
}

size_t syn_stream_free(const SYN_Stream *s)
{
    return syn_ringbuf_free(&s->rb);
}

void syn_stream_flush(SYN_Stream *s)
{
    syn_ringbuf_reset(&s->rb);
}
