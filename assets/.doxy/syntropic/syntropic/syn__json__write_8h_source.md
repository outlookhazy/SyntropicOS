

# File syn\_json\_write.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_write.h**](syn__json__write_8h.md)

[Go to the documentation of this file](syn__json__write_8h.md)


```C++

#ifndef SYN_JSON_WRITE_H
#define SYN_JSON_WRITE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Writer state ──────────────────────────────────────────────────────── */

#define SYN_JSON_MAX_DEPTH  8  
typedef struct {
    char    *buf;              
    size_t   capacity;         
    size_t   len;              
    uint8_t  depth;            
    bool     needs_comma;      
    bool     overflow;         
} SYN_JsonWriter;

/* ── Init ──────────────────────────────────────────────────────────────── */

void syn_json_init(SYN_JsonWriter *w, char *buf, size_t capacity);

/* ── Structure ─────────────────────────────────────────────────────────── */

void syn_json_obj_open(SYN_JsonWriter *w);

void syn_json_obj_close(SYN_JsonWriter *w);

void syn_json_arr_open(SYN_JsonWriter *w);

void syn_json_arr_close(SYN_JsonWriter *w);

/* ── Key-value pairs (inside objects) ──────────────────────────────────── */

void syn_json_key_str(SYN_JsonWriter *w, const char *key, const char *val);

void syn_json_key_int(SYN_JsonWriter *w, const char *key, int32_t val);

void syn_json_key_uint(SYN_JsonWriter *w, const char *key, uint32_t val);

void syn_json_key_bool(SYN_JsonWriter *w, const char *key, bool val);

void syn_json_key_null(SYN_JsonWriter *w, const char *key);

void syn_json_key(SYN_JsonWriter *w, const char *key);

/* ── Array values (inside arrays) ──────────────────────────────────────── */

void syn_json_val_str(SYN_JsonWriter *w, const char *val);

void syn_json_val_int(SYN_JsonWriter *w, int32_t val);

void syn_json_val_uint(SYN_JsonWriter *w, uint32_t val);

void syn_json_val_bool(SYN_JsonWriter *w, bool val);

/* ── Query ─────────────────────────────────────────────────────────────── */

static inline size_t syn_json_len(const SYN_JsonWriter *w)
{
    return w->len;
}

static inline const char *syn_json_str(const SYN_JsonWriter *w)
{
    return w->buf;
}

static inline bool syn_json_ok(const SYN_JsonWriter *w)
{
    return !w->overflow;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_JSON_WRITE_H */
```


