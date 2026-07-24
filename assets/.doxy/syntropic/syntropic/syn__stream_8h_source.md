

# File syn\_stream.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_stream.h**](syn__stream_8h.md)

[Go to the documentation of this file](syn__stream_8h.md)


```C++

#ifndef SYN_STREAM_H
#define SYN_STREAM_H

#include "../util/syn_ringbuf.h"
#include "../pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Stream instance ────────────────────────────────────────────────────── */

typedef struct {
    SYN_RingBuf  rb;           
    size_t       threshold;    
    uint8_t      delimiter;    
    bool         delim_en;     
} SYN_Stream;

/* ── Init / config ──────────────────────────────────────────────────────── */

void syn_stream_init(SYN_Stream *s, uint8_t *buf, size_t size);

void syn_stream_set_threshold(SYN_Stream *s, size_t n);

void syn_stream_set_delimiter(SYN_Stream *s, uint8_t delim);

void syn_stream_clear_delimiter(SYN_Stream *s);

/* ── Producer (ISR-safe, never blocks) ──────────────────────────────────── */

size_t syn_stream_write(SYN_Stream *s, const uint8_t *data, size_t len);

bool syn_stream_put(SYN_Stream *s, uint8_t byte);

/* ── Consumer ───────────────────────────────────────────────────────────── */

bool syn_stream_readable(const SYN_Stream *s);

size_t syn_stream_read(SYN_Stream *s, uint8_t *buf, size_t max_len);

size_t syn_stream_read_line(SYN_Stream *s, uint8_t *buf, size_t max_len);

size_t syn_stream_count(const SYN_Stream *s);

size_t syn_stream_free(const SYN_Stream *s);

void syn_stream_flush(SYN_Stream *s);

/* ── Protothread integration ────────────────────────────────────────────── */

#define PT_STREAM_WAIT(pt, stream)  \
    PT_WAIT_UNTIL(pt, syn_stream_readable(stream))

#ifdef __cplusplus
}
#endif

#endif /* SYN_STREAM_H */
```


