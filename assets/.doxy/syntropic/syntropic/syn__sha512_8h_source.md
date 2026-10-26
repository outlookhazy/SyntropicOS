

# File syn\_sha512.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_sha512.h**](syn__sha512_8h.md)

[Go to the documentation of this file](syn__sha512_8h.md)


```C++

#ifndef SYN_SHA512_H
#define SYN_SHA512_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA512) || SYN_USE_SHA512

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SHA512_DIGEST_SIZE 64U

#define SYN_SHA384_DIGEST_SIZE 48U

#define SYN_SHA512_BLOCK_SIZE 128U

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    uint64_t state[8];                  
    uint8_t buf[SYN_SHA512_BLOCK_SIZE]; 
    uint32_t buf_len;                   
    uint64_t total_len_lo;              
    uint64_t total_len_hi;              
} SYN_SHA512;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sha512_init(SYN_SHA512 *ctx);

void syn_sha384_init(SYN_SHA512 *ctx);

void syn_sha512_update(SYN_SHA512 *ctx, const void *data, size_t len);

void syn_sha512_final(SYN_SHA512 *ctx, uint8_t hash[SYN_SHA512_DIGEST_SIZE]);

void syn_sha384_final(SYN_SHA512 *ctx, uint8_t hash[SYN_SHA384_DIGEST_SIZE]);

void syn_sha512(const void *data, size_t len, uint8_t hash[SYN_SHA512_DIGEST_SIZE]);

void syn_sha384(const void *data, size_t len, uint8_t hash[SYN_SHA384_DIGEST_SIZE]);

typedef struct {
    SYN_SHA512 inner;                         
    uint8_t o_key_pad[SYN_SHA512_BLOCK_SIZE]; 
} SYN_HMAC_SHA512;

typedef struct {
    SYN_SHA512 inner;                         
    uint8_t o_key_pad[SYN_SHA512_BLOCK_SIZE]; 
} SYN_HMAC_SHA384;

void syn_hmac_sha512_init(SYN_HMAC_SHA512 *ctx, const void *key, size_t key_len);

void syn_hmac_sha512_update(SYN_HMAC_SHA512 *ctx, const void *data, size_t len);

void syn_hmac_sha512_final(SYN_HMAC_SHA512 *ctx, uint8_t mac[SYN_SHA512_DIGEST_SIZE]);

void syn_hmac_sha384_init(SYN_HMAC_SHA384 *ctx, const void *key, size_t key_len);

void syn_hmac_sha384_update(SYN_HMAC_SHA384 *ctx, const void *data, size_t len);

void syn_hmac_sha384_final(SYN_HMAC_SHA384 *ctx, uint8_t mac[SYN_SHA384_DIGEST_SIZE]);

void syn_hmac_sha512(const void *key, size_t key_len, const void *data, size_t data_len,
                     uint8_t mac[SYN_SHA512_DIGEST_SIZE]);

void syn_hmac_sha384(const void *key, size_t key_len, const void *data, size_t data_len,
                     uint8_t mac[SYN_SHA384_DIGEST_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SHA512 */

#endif /* SYN_SHA512_H */
```


