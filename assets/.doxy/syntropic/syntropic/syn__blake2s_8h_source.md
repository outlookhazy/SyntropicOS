

# File syn\_blake2s.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_blake2s.h**](syn__blake2s_8h.md)

[Go to the documentation of this file](syn__blake2s_8h.md)


```C++

#ifndef SYN_BLAKE2S_H
#define SYN_BLAKE2S_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BLAKE2S) || SYN_USE_BLAKE2S

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

#define SYN_BLAKE2S_BLOCK_SIZE   64  
#define SYN_BLAKE2S_MAX_DIGEST   32  
#define SYN_BLAKE2S_MAX_KEY      32  
/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t h[8];                          
    uint32_t t[2];                          
    uint8_t  buf[SYN_BLAKE2S_BLOCK_SIZE];   
    uint8_t  buflen;                        
    uint8_t  outlen;                        
} SYN_BLAKE2s;

/* ── Core API ───────────────────────────────────────────────────────────── */

void syn_blake2s_init(SYN_BLAKE2s *ctx, size_t outlen);

void syn_blake2s_init_keyed(SYN_BLAKE2s *ctx,
                            const void *key, size_t keylen,
                            size_t outlen);

void syn_blake2s_update(SYN_BLAKE2s *ctx, const void *data, size_t len);

void syn_blake2s_final(SYN_BLAKE2s *ctx, uint8_t *out);

/* ── Convenience ────────────────────────────────────────────────────────── */

static inline void syn_blake2s(const void *data, size_t len,
                               uint8_t *out, size_t outlen)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, outlen);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, out);
}

static inline void syn_blake2s_mac(const void *key, size_t keylen,
                                   const void *data, size_t len,
                                   uint8_t *out, size_t outlen)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init_keyed(&ctx, key, keylen, outlen);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, out);
}

/* ── HMAC-BLAKE2s (for HKDF) ───────────────────────────────────────────── */

typedef struct {
    SYN_BLAKE2s inner;                            
    uint8_t     o_key_pad[SYN_BLAKE2S_BLOCK_SIZE]; 
} SYN_HMAC_BLAKE2s;

static inline void syn_hmac_blake2s_init(SYN_HMAC_BLAKE2s *ctx,
                                         const void *key, size_t keylen)
{
    uint8_t k_buf[SYN_BLAKE2S_BLOCK_SIZE];
    uint8_t i_key_pad[SYN_BLAKE2S_BLOCK_SIZE];
    unsigned i;

    memset(k_buf, 0, sizeof(k_buf));

    if (keylen > SYN_BLAKE2S_BLOCK_SIZE) {
        syn_blake2s(key, keylen, k_buf, SYN_BLAKE2S_MAX_DIGEST);
    } else {
        memcpy(k_buf, key, keylen);
    }

    for (i = 0; i < SYN_BLAKE2S_BLOCK_SIZE; i++) {
        i_key_pad[i]       = k_buf[i] ^ 0x36u;
        ctx->o_key_pad[i]  = k_buf[i] ^ 0x5Cu;
    }

    syn_blake2s_init(&ctx->inner, SYN_BLAKE2S_MAX_DIGEST);
    syn_blake2s_update(&ctx->inner, i_key_pad, SYN_BLAKE2S_BLOCK_SIZE);
}

static inline void syn_hmac_blake2s_update(SYN_HMAC_BLAKE2s *ctx,
                                           const void *data, size_t len)
{
    syn_blake2s_update(&ctx->inner, data, len);
}

static inline void syn_hmac_blake2s_final(SYN_HMAC_BLAKE2s *ctx,
                                          uint8_t mac[SYN_BLAKE2S_MAX_DIGEST])
{
    uint8_t inner_hash[SYN_BLAKE2S_MAX_DIGEST];

    syn_blake2s_final(&ctx->inner, inner_hash);

    SYN_BLAKE2s outer;
    syn_blake2s_init(&outer, SYN_BLAKE2S_MAX_DIGEST);
    syn_blake2s_update(&outer, ctx->o_key_pad, SYN_BLAKE2S_BLOCK_SIZE);
    syn_blake2s_update(&outer, inner_hash, SYN_BLAKE2S_MAX_DIGEST);
    syn_blake2s_final(&outer, mac);
}

static inline void syn_hmac_blake2s(const void *key, size_t keylen,
                                    const void *data, size_t datalen,
                                    uint8_t mac[SYN_BLAKE2S_MAX_DIGEST])
{
    SYN_HMAC_BLAKE2s ctx;
    syn_hmac_blake2s_init(&ctx, key, keylen);
    syn_hmac_blake2s_update(&ctx, data, datalen);
    syn_hmac_blake2s_final(&ctx, mac);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_BLAKE2S */

#endif /* SYN_BLAKE2S_H */
```


