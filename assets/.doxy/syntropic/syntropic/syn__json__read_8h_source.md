

# File syn\_json\_read.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_read.h**](syn__json__read_8h.md)

[Go to the documentation of this file](syn__json__read_8h.md)


```C++

#ifndef SYN_JSON_READ_H
#define SYN_JSON_READ_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Token types ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_JSON_NONE    = 0,  
    SYN_JSON_STRING  = 1,  
    SYN_JSON_NUMBER  = 2,  
    SYN_JSON_BOOL    = 3,  
    SYN_JSON_NULL    = 4,  
    SYN_JSON_OBJECT  = 5,  
    SYN_JSON_ARRAY   = 6,  
} SYN_JsonType;

/* ── Token ─────────────────────────────────────────────────────────────── */

#define SYN_JSON_MAX_TOKENS  32

typedef struct {
    SYN_JsonType  type;      
    const char   *key;       
    const char   *value;     
    int32_t       int_val;   
    uint8_t       depth;     
} SYN_JsonToken;

/* ── Reader ────────────────────────────────────────────────────────────── */

typedef struct {
    SYN_JsonToken  tokens[SYN_JSON_MAX_TOKENS]; 
    size_t         token_count; 
    bool           valid;      
} SYN_JsonReader;

/* ── API ───────────────────────────────────────────────────────────────── */

bool syn_json_parse(SYN_JsonReader *r, char *json, size_t len);

const SYN_JsonToken *syn_json_find(const SYN_JsonReader *r, const char *key);

bool syn_json_get_str(const SYN_JsonReader *r, const char *key,
                       char *out, size_t out_sz);

bool syn_json_get_int(const SYN_JsonReader *r, const char *key, int32_t *out);

bool syn_json_get_bool(const SYN_JsonReader *r, const char *key, bool *out);

bool syn_json_is_null(const SYN_JsonReader *r, const char *key);

SYN_JsonType syn_json_get_type(const SYN_JsonReader *r, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* SYN_JSON_READ_H */
```


