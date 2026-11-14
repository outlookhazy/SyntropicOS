

# File syn\_stream.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_stream.c**](syn__stream_8c.md)

[Go to the documentation of this file](syn__stream_8c.md)


```C++


#include "syn_stream.h"

#include "../common/syn_barrier.h"
#include "syn_ringbuf.h"

#include <string.h>

/* ── Internal helpers ──────────────────────────────────────────────────── */

static size_t stream_find_delimiter(const SYN_Stream *s)
{
    const SYN_RingBuf *rb = &s->rb;
    const size_t count = syn_ringbuf_count(rb);
    size_t idx = SYN_LOAD_ACQUIRE(&rb->tail);

    for (size_t i = 0U; i < count; i++) {
        if (rb->buf[idx] == s->delimiter) {
            return i + 1U; /* 1-based position (byte count including delim) */
        }
        idx++;
        if (idx >= rb->size) {
            idx = 0U;
        }
    }
    return 0U;
}

static void stream_rewind_partial_frame(SYN_Stream *s)
{
    size_t tail = SYN_LOAD_ACQUIRE(&s->rb.tail);
    size_t prev = (s->frame_head == 0U) ? (s->rb.size - 1U) : (s->frame_head - 1U);

    if (s->frame_head != tail && s->rb.buf[prev] == s->delimiter) {
        SYN_STORE_RELEASE(&s->rb.head, s->frame_head);
    } else {
        s->frame_head = tail;
        SYN_STORE_RELEASE(&s->rb.head, tail);
    }
}

/* ── Init / config ─────────────────────────────────────────────────────── */

void syn_stream_init(SYN_Stream *s, uint8_t *buf, size_t size)
{
    if (s == NULL) {
        return;
    }
    (void)memset(s, 0, sizeof(*s));
    if (buf == NULL || size <= 1U) {
        return;
    }
    syn_ringbuf_init(&s->rb, buf, size);
}

void syn_stream_set_threshold(SYN_Stream *s, size_t n)
{
    if (s == NULL) {
        return;
    }
    s->threshold = n;
}

void syn_stream_set_delimiter(SYN_Stream *s, uint8_t delim)
{
    if (s == NULL) {
        return;
    }
    s->delimiter = delim;
    s->delim_en = true;
    s->overflowing = false;
    s->frame_head = SYN_LOAD_ACQUIRE(&s->rb.head);
}

void syn_stream_clear_delimiter(SYN_Stream *s)
{
    if (s == NULL) {
        return;
    }
    s->delim_en = false;
    s->overflowing = false;
}

/* ── Producer ──────────────────────────────────────────────────────────── */

size_t syn_stream_write(SYN_Stream *s, const uint8_t *data, size_t len)
{
    if (s == NULL || data == NULL || len == 0U) {
        return 0U;
    }
    size_t written = 0U;
    for (size_t i = 0U; i < len; i++) {
        if (syn_stream_put(s, data[i])) {
            written++;
        }
    }
    return written;
}

bool syn_stream_put(SYN_Stream *s, uint8_t byte)
{
    if (s == NULL || s->rb.buf == NULL) {
        return false;
    }

    if (s->delim_en && s->overflowing) {
        /* Discard tail bytes until trailing delimiter arrives to complete resync */
        if (byte == s->delimiter) {
            s->overflowing = false;
            s->frame_head = SYN_LOAD_ACQUIRE(&s->rb.head);
        }
        return false;
    }

    bool ok = syn_ringbuf_put(&s->rb, byte);
    if (ok) {
        if (s->delim_en && byte == s->delimiter) {
            s->frame_head = SYN_LOAD_ACQUIRE(&s->rb.head);
        }
    } else if (s->delim_en) {
        /* Buffer full without completing frame: rewind partial frame & enter resync state */
        stream_rewind_partial_frame(s);
        s->overflow_count++;
        s->overflowing = (byte != s->delimiter);
    }
    return ok;
}

/* ── Consumer ──────────────────────────────────────────────────────────── */

bool syn_stream_readable(const SYN_Stream *s)
{
    if (s == NULL) {
        return false;
    }

    const size_t count = syn_ringbuf_count(&s->rb);
    if (count == 0U) {
        return false;
    }

    /* Delimiter mode takes precedence */
    if (s->delim_en) {
        return stream_find_delimiter(s) > 0U;
    }

    /* Threshold mode */
    if (s->threshold > 0U) {
        return count >= s->threshold;
    }

    /* Default: any bytes */
    return true;
}

size_t syn_stream_read(SYN_Stream *s, uint8_t *buf, size_t max_len)
{
    if (s == NULL || buf == NULL || max_len == 0U) {
        return 0U;
    }
    return syn_ringbuf_read(&s->rb, buf, max_len);
}

size_t syn_stream_read_line(SYN_Stream *s, uint8_t *buf, size_t max_len)
{
    if (s == NULL || buf == NULL || max_len == 0U) {
        return 0U;
    }

    if (!s->delim_en) {
        /* No delimiter mode — behave like plain read */
        return syn_ringbuf_read(&s->rb, buf, max_len);
    }

    /* Find delimiter position */
    size_t delim_pos = stream_find_delimiter(s);
    if (delim_pos == 0U) {
        return 0U; /* No delimiter found — don't consume anything */
    }

    /* Read up to delimiter (inclusive), capped by max_len */
    size_t to_read = (delim_pos < max_len) ? delim_pos : max_len;
    return syn_ringbuf_read(&s->rb, buf, to_read);
}

size_t syn_stream_count(const SYN_Stream *s)
{
    if (s == NULL) {
        return 0U;
    }
    return syn_ringbuf_count(&s->rb);
}

size_t syn_stream_free(const SYN_Stream *s)
{
    if (s == NULL) {
        return 0U;
    }
    return syn_ringbuf_free(&s->rb);
}

void syn_stream_flush(SYN_Stream *s)
{
    if (s == NULL) {
        return;
    }
    syn_ringbuf_reset(&s->rb);
    s->frame_head = 0U;
    s->overflowing = false;
}
```


