/**
 * @file syn_hmac_drbg.c
 * @brief NIST SP 800-90A HMAC-DRBG (Deterministic Random Bit Generator) implementation.
 */

#include "syn_hmac_drbg.h"

#if SYN_HMAC_DRBG_ENABLED

#include "../util/syn_assert.h"

#include <string.h>

/**
 * @brief Zeroize sensitive stack buffers without compiler dead-store elimination.
 *
 * @param v Buffer pointer.
 * @param n Number of bytes to zeroize.
 */
static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

/**
 * @brief Execute one HMAC-DRBG K/V derivation round with stack-context zeroization.
 *
 * @param ctx Pointer to DRBG context.
 * @param sep Domain separator byte (0x00 or 0x01).
 * @param c1  First chunk of provided data (or NULL).
 * @param l1  Length of first chunk.
 * @param c2  Second chunk of provided data (or NULL).
 * @param l2  Length of second chunk.
 * @param c3  Third chunk of provided data (or NULL).
 * @param l3  Length of third chunk.
 */
static void drbg_hmac_round(SYN_HMAC_DRBG *ctx, uint8_t sep, const uint8_t *c1, size_t l1,
                            const uint8_t *c2, size_t l2, const uint8_t *c3, size_t l3)
{
    SYN_HMAC_SHA256 hmac;

    syn_hmac_sha256_init(&hmac, ctx->k, sizeof(ctx->k));
    syn_hmac_sha256_update(&hmac, ctx->v, sizeof(ctx->v));
    syn_hmac_sha256_update(&hmac, &sep, 1U);
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
    syn_secure_zero(&hmac, sizeof(hmac));

    syn_hmac_sha256(ctx->k, sizeof(ctx->k), ctx->v, sizeof(ctx->v), ctx->v);
}

/**
 * @brief Internal HMAC-DRBG state update function per NIST SP 800-90A Rev 1 Section 10.1.2.2.
 *
 * @param ctx Pointer to DRBG context.
 * @param c1  First chunk of provided data (or NULL).
 * @param l1  Length of first chunk.
 * @param c2  Second chunk of provided data (or NULL).
 * @param l2  Length of second chunk.
 * @param c3  Third chunk of provided data (or NULL).
 * @param l3  Length of third chunk.
 */
static void drbg_update(SYN_HMAC_DRBG *ctx, const uint8_t *c1, size_t l1, const uint8_t *c2,
                        size_t l2, const uint8_t *c3, size_t l3)
{
    bool has_data = (c1 != NULL && l1 > 0U) || (c2 != NULL && l2 > 0U) || (c3 != NULL && l3 > 0U);

    drbg_hmac_round(ctx, 0x00U, c1, l1, c2, l2, c3, l3);
    if (has_data) {
        drbg_hmac_round(ctx, 0x01U, c1, l1, c2, l2, c3, l3);
    }
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
        ctx->reseed_interval = (interval == UINT32_MAX) ? (UINT32_MAX - 1U) : interval;
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
