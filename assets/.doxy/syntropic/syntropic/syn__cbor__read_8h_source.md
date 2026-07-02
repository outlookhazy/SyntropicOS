

# File syn\_cbor\_read.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_cbor\_read.h**](syn__cbor__read_8h.md)

[Go to the documentation of this file](syn__cbor__read_8h.md)


```C++

#ifndef SYN_CBOR_READ_H
#define SYN_CBOR_READ_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Item type ───────────────────────────────────────────────────────────── */

typedef enum {
    SYN_CBOR_UINT   = 0,   
    SYN_CBOR_INT    = 1,   
    SYN_CBOR_BYTES  = 2,   
    SYN_CBOR_TEXT   = 3,   
    SYN_CBOR_ARRAY  = 4,   
    SYN_CBOR_MAP    = 5,   
    SYN_CBOR_FLOAT  = 6,   
    SYN_CBOR_BOOL   = 7,   
    SYN_CBOR_NULL   = 8,   
    SYN_CBOR_ERROR  = 0xFF 
} SYN_CborType;

/* ── Reader state ────────────────────────────────────────────────────────── */

typedef struct {
    const uint8_t *buf;  
    size_t         len;  
    size_t         pos;  
    bool           ok;   
} SYN_CborReader;

/* ── Init ────────────────────────────────────────────────────────────────── */

void syn_cbor_reader_init(SYN_CborReader *r,
                          const uint8_t  *buf,
                          size_t          len);

/* ── Navigation ──────────────────────────────────────────────────────────── */

SYN_CborType syn_cbor_peek_type(const SYN_CborReader *r);

void syn_cbor_skip(SYN_CborReader *r);

/* ── Collections ─────────────────────────────────────────────────────────── */

size_t syn_cbor_read_map_begin(SYN_CborReader *r);

size_t syn_cbor_read_array_begin(SYN_CborReader *r);

/* ── Scalars ─────────────────────────────────────────────────────────────── */

uint64_t syn_cbor_read_uint(SYN_CborReader *r);

int64_t syn_cbor_read_int(SYN_CborReader *r);

float syn_cbor_read_float(SYN_CborReader *r);

bool syn_cbor_read_bool(SYN_CborReader *r);

void syn_cbor_read_null(SYN_CborReader *r);

/* ── Strings ─────────────────────────────────────────────────────────────── */

size_t syn_cbor_read_text(SYN_CborReader *r, char *buf, size_t cap);

size_t syn_cbor_read_bytes(SYN_CborReader *r, uint8_t *buf, size_t cap);

/* ── Status ──────────────────────────────────────────────────────────────── */

static inline bool syn_cbor_reader_ok(const SYN_CborReader *r)
{
    return r->ok;
}

static inline bool syn_cbor_reader_done(const SYN_CborReader *r)
{
    return r->pos >= r->len;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CBOR_READ_H */
```


