

# File syn\_hmac\_drbg.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hmac\_drbg.c**](syn__hmac__drbg_8c.md)

[Go to the documentation of this file](syn__hmac__drbg_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HMAC_DRBG) || SYN_USE_HMAC_DRBG

#include "../util/syn_assert.h"
#include "syn_hmac_drbg.h"

#include <string.h>

static void drbg_update(SYN_HMAC_DRBG *ctx, const uint8_t *c1, size_t l1, const uint8_t *c2,
                        size_t l2, const uint8_t *c3, size_t l3)
{
    bool has_data = (c1 != NULL && l1 > 0U) || (c2 != NULL && l2 > 0U) || (c3 != NULL && l3 > 0U);
    SYN_HMAC_SHA256 hmac;

    /* Step 1: K = HMAC(K, V || 0x00 || provided_data) */
    syn_hmac_sha256_init(&hmac, ctx->k, sizeof(ctx->k));
    syn_hmac_sha256_update(&hmac, ctx->v, sizeof(ctx->v));
    const uint8_t sep0 = 0x00U;
    syn_hmac_sha256_update(&hmac, &sep0, 1U);
    if (c1 != NULL && l1 > 0U) {
        syn_hmac_sha256_update(&hmac, c1, l1);
    }
    if (c2 != NULL && l2 > 0U) {
        syn_hmac_sha256_update(&hmac, c2, l2);
    }
    if (c3 != NULL && l3 > 0U) {
        syn_hmac_sha256_update(&hmac, c3, l3);
    }
    syn_hmac_sha256_final(&hmac, ctx->k);

    /* Step 2: V = HMAC(K, V) */
    syn_hmac_sha256(ctx->k, sizeof(ctx->k), ctx->v, sizeof(ctx->v), ctx->v);

    if (!has_data) {
        return;
    }

    /* Step 4: K = HMAC(K, V || 0x01 || provided_data) */
    syn_hmac_sha256_init(&hmac, ctx->k, sizeof(ctx->k));
    syn_hmac_sha256_update(&hmac, ctx->v, sizeof(ctx->v));
    const uint8_t sep1 = 0x01U;
    syn_hmac_sha256_update(&hmac, &sep1, 1U);
    if (c1 != NULL && l1 > 0U) {
        syn_hmac_sha256_update(&hmac, c1, l1);
    }
    if (c2 != NULL && l2 > 0U) {
        syn_hmac_sha256_update(&hmac, c2, l2);
    }
    if (c3 != NULL && l3 > 0U) {
        syn_hmac_sha256_update(&hmac, c3, l3);
    }
    syn_hmac_sha256_final(&hmac, ctx->k);

    /* Step 5: V = HMAC(K, V) */
    syn_hmac_sha256(ctx->k, sizeof(ctx->k), ctx->v, sizeof(ctx->v), ctx->v);
}

SYN_Status syn_hmac_drbg_init(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                              const uint8_t *nonce, size_t nonce_len, const uint8_t *pers_str,
                              size_t pers_len)
{
    if (ctx == NULL || entropy == NULL || ent_len < SYN_HMAC_DRBG_MIN_ENTROPY_LEN) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(ctx->k, 0x00, sizeof(ctx->k));
    (void)memset(ctx->v, 0x01, sizeof(ctx->v));
    ctx->reseed_interval = SYN_HMAC_DRBG_DEFAULT_RESEED_INTERVAL;

    drbg_update(ctx, entropy, ent_len, nonce, nonce_len, pers_str, pers_len);

    ctx->reseed_counter = 1U;
    ctx->initialized = true;

    return SYN_OK;
}

SYN_Status syn_hmac_drbg_reseed(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                                const uint8_t *add_input, size_t add_len)
{
    if (ctx == NULL || !ctx->initialized || entropy == NULL ||
        ent_len < SYN_HMAC_DRBG_MIN_ENTROPY_LEN) {
        return SYN_INVALID_PARAM;
    }

    drbg_update(ctx, entropy, ent_len, add_input, add_len, NULL, 0U);

    ctx->reseed_counter = 1U;
    return SYN_OK;
}

SYN_Status syn_hmac_drbg_generate(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                  const uint8_t *add_input, size_t add_len)
{
    if (ctx == NULL || !ctx->initialized || out == NULL ||
        out_len > SYN_HMAC_DRBG_MAX_REQUEST_BYTES) {
        return SYN_INVALID_PARAM;
    }

    if (out_len == 0U) {
        return SYN_OK;
    }

    if (ctx->reseed_counter > ctx->reseed_interval) {
        return SYN_ERROR;
    }

    if (add_input != NULL && add_len > 0U) {
        drbg_update(ctx, add_input, add_len, NULL, 0U, NULL, 0U);
    }

    size_t generated = 0U;
    while (generated < out_len) {
        syn_hmac_sha256(ctx->k, sizeof(ctx->k), ctx->v, sizeof(ctx->v), ctx->v);
        size_t to_copy = out_len - generated;
        if (to_copy > sizeof(ctx->v)) {
            to_copy = sizeof(ctx->v);
        }
        (void)memcpy(&out[generated], ctx->v, to_copy);
        generated += to_copy;
    }

    drbg_update(ctx, add_input, add_len, NULL, 0U, NULL, 0U);

    ctx->reseed_counter++;
    return SYN_OK;
}

SYN_Status syn_hmac_drbg_generate_pr(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                     const uint8_t *entropy, size_t ent_len,
                                     const uint8_t *add_input, size_t add_len)
{
    if (ctx == NULL || !ctx->initialized || out == NULL ||
        out_len > SYN_HMAC_DRBG_MAX_REQUEST_BYTES || entropy == NULL ||
        ent_len < SYN_HMAC_DRBG_MIN_ENTROPY_LEN) {
        return SYN_INVALID_PARAM;
    }

    (void)syn_hmac_drbg_reseed(ctx, entropy, ent_len, add_input, add_len);
    return syn_hmac_drbg_generate(ctx, out, out_len, NULL, 0U);
}

void syn_hmac_drbg_set_reseed_interval(SYN_HMAC_DRBG *ctx, uint32_t interval)
{
    if (ctx != NULL && interval > 0U) {
        ctx->reseed_interval = interval;
    }
}

void syn_hmac_drbg_wipe(SYN_HMAC_DRBG *ctx)
{
    if (ctx == NULL) {
        return;
    }

    volatile uint8_t *p_k = (volatile uint8_t *)ctx->k;
    for (size_t i = 0U; i < sizeof(ctx->k); i++) {
        p_k[i] = 0U;
    }

    volatile uint8_t *p_v = (volatile uint8_t *)ctx->v;
    for (size_t i = 0U; i < sizeof(ctx->v); i++) {
        p_v[i] = 0U;
    }

    ctx->reseed_counter = 0U;
    ctx->reseed_interval = 0U;
    ctx->initialized = false;
}

#endif /* SYN_USE_HMAC_DRBG */
```


