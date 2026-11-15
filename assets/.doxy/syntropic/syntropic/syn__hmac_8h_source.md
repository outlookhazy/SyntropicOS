

# File syn\_hmac.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hmac.h**](syn__hmac_8h.md)

[Go to the documentation of this file](syn__hmac_8h.md)


```C++


#ifndef SYN_HMAC_H
#define SYN_HMAC_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256

#include "syn_sha256.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_HMAC_SHA256_SIZE SYN_SHA256_DIGEST_SIZE

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    SYN_SHA256 inner;                         
    uint8_t o_key_pad[SYN_SHA256_BLOCK_SIZE]; 
} SYN_HMAC_SHA256;

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_hmac_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

static inline void syn_hmac_sha256_init(SYN_HMAC_SHA256 *ctx, const void *key, size_t key_len)
{
    uint8_t k_buf[SYN_SHA256_BLOCK_SIZE];
    unsigned i;

    memset(k_buf, 0, sizeof(k_buf));

    if (key_len > SYN_SHA256_BLOCK_SIZE) {
        /* Keys longer than block size are hashed first */
        syn_sha256(key, key_len, k_buf);
    } else if (key != NULL && key_len > 0U) {
        memcpy(k_buf, key, key_len);
    }

    /* Compute i_key_pad = K ⊕ 0x36  and  o_key_pad = K ⊕ 0x5C */
    uint8_t i_key_pad[SYN_SHA256_BLOCK_SIZE];
    for (i = 0U; i < SYN_SHA256_BLOCK_SIZE; i++) {
        i_key_pad[i] = (uint8_t)(k_buf[i] ^ 0x36U);
        ctx->o_key_pad[i] = (uint8_t)(k_buf[i] ^ 0x5CU);
    }

    /* Inner hash: SHA-256(i_key_pad || message...) */
    syn_sha256_init(&ctx->inner);
    syn_sha256_update(&ctx->inner, i_key_pad, SYN_SHA256_BLOCK_SIZE);

    syn_hmac_secure_zero(k_buf, sizeof(k_buf));
    syn_hmac_secure_zero(i_key_pad, sizeof(i_key_pad));
}

static inline void syn_hmac_sha256_update(SYN_HMAC_SHA256 *ctx, const void *data, size_t len)
{
    syn_sha256_update(&ctx->inner, data, len);
}

static inline void syn_hmac_sha256_final(SYN_HMAC_SHA256 *ctx, uint8_t mac[SYN_HMAC_SHA256_SIZE])
{
    uint8_t inner_hash[SYN_SHA256_DIGEST_SIZE];

    /* Finalize inner hash */
    syn_sha256_final(&ctx->inner, inner_hash);

    /* Outer hash: SHA-256(o_key_pad || inner_hash) */
    SYN_SHA256 outer;
    syn_sha256_init(&outer);
    syn_sha256_update(&outer, ctx->o_key_pad, SYN_SHA256_BLOCK_SIZE);
    syn_sha256_update(&outer, inner_hash, SYN_SHA256_DIGEST_SIZE);
    syn_sha256_final(&outer, mac);

    syn_hmac_secure_zero(inner_hash, sizeof(inner_hash));
    syn_hmac_secure_zero(&outer, sizeof(outer));
    syn_hmac_secure_zero(ctx, sizeof(*ctx));
}

static inline void syn_hmac_sha256(const void *key, size_t key_len, const void *data,
                                   size_t data_len, uint8_t mac[SYN_HMAC_SHA256_SIZE])
{
    SYN_HMAC_SHA256 ctx;
    syn_hmac_sha256_init(&ctx, key, key_len);
    syn_hmac_sha256_update(&ctx, data, data_len);
    syn_hmac_sha256_final(&ctx, mac);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SHA256 */

#endif /* SYN_HMAC_H */
```


