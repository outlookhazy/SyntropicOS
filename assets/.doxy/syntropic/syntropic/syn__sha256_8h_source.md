

# File syn\_sha256.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_sha256.h**](syn__sha256_8h.md)

[Go to the documentation of this file](syn__sha256_8h.md)


```C++

#ifndef SYN_SHA256_H
#define SYN_SHA256_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SHA256_DIGEST_SIZE 32

#define SYN_SHA256_BLOCK_SIZE 64

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t state[8];                  
    uint8_t buf[SYN_SHA256_BLOCK_SIZE]; 
    uint32_t buf_len;                   
    uint32_t total_len_lo;              
    uint32_t total_len_hi;              
} SYN_SHA256;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sha256_init(SYN_SHA256 *ctx);

void syn_sha256_update(SYN_SHA256 *ctx, const void *data, size_t len);

void syn_sha256_final(SYN_SHA256 *ctx, uint8_t hash[SYN_SHA256_DIGEST_SIZE]);

static inline void syn_sha256(const void *data, size_t len, uint8_t hash[SYN_SHA256_DIGEST_SIZE])
{
    SYN_SHA256 ctx;
    syn_sha256_init(&ctx);
    syn_sha256_update(&ctx, data, len);
    syn_sha256_final(&ctx, hash);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SHA256 */

#endif /* SYN_SHA256_H */
```


