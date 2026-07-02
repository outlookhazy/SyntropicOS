

# File syn\_cbor\_write.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_cbor\_write.h**](syn__cbor__write_8h.md)

[Go to the documentation of this file](syn__cbor__write_8h.md)


```C++

#ifndef SYN_CBOR_WRITE_H
#define SYN_CBOR_WRITE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Writer state ────────────────────────────────────────────────────────── */

typedef struct {
    uint8_t *buf;       
    size_t   cap;       
    size_t   len;       
    bool     overflow;  
} SYN_CborWriter;

/* ── Init ────────────────────────────────────────────────────────────────── */

void syn_cbor_writer_init(SYN_CborWriter *w, uint8_t *buf, size_t cap);

/* ── Collections ─────────────────────────────────────────────────────────── */

void syn_cbor_write_map_begin(SYN_CborWriter *w, size_t count);

void syn_cbor_write_array_begin(SYN_CborWriter *w, size_t count);

/* ── Scalars ─────────────────────────────────────────────────────────────── */

void syn_cbor_write_uint(SYN_CborWriter *w, uint64_t v);

void syn_cbor_write_int(SYN_CborWriter *w, int64_t v);

void syn_cbor_write_float(SYN_CborWriter *w, float v);

void syn_cbor_write_bool(SYN_CborWriter *w, bool v);

void syn_cbor_write_null(SYN_CborWriter *w);

/* ── Strings ─────────────────────────────────────────────────────────────── */

void syn_cbor_write_text(SYN_CborWriter *w, const char *str, size_t len);

void syn_cbor_write_text_cstr(SYN_CborWriter *w, const char *str);

void syn_cbor_write_bytes(SYN_CborWriter *w, const uint8_t *data, size_t len);

/* ── Status ──────────────────────────────────────────────────────────────── */

static inline size_t syn_cbor_writer_len(const SYN_CborWriter *w)
{
    return w->len;
}

static inline bool syn_cbor_writer_ok(const SYN_CborWriter *w)
{
    return !w->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CBOR_WRITE_H */
```


