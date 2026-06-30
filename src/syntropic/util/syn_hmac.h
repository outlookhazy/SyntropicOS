/**
 * @file syn_hmac.h
 * @brief HMAC-SHA256 — keyed-hash message authentication code.
 *
 * Built on top of syn_sha256. Provides the same streaming pattern:
 *   init (with key) → update → final
 *
 * The context wraps two SHA-256 contexts (~224 bytes caller-owned).
 * No heap, no external dependencies.
 *
 * @par Usage
 * @code
 *   // One-shot:
 *   uint8_t mac[32];
 *   syn_hmac_sha256(key, key_len, message, msg_len, mac);
 *
 *   // Streaming:
 *   SYN_HMAC_SHA256 ctx;
 *   syn_hmac_sha256_init(&ctx, key, key_len);
 *   syn_hmac_sha256_update(&ctx, chunk1, len1);
 *   syn_hmac_sha256_update(&ctx, chunk2, len2);
 *   syn_hmac_sha256_final(&ctx, mac);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_HMAC_H
#define SYN_HMAC_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256

#include "syn_sha256.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief HMAC-SHA256 output size in bytes. */
#define SYN_HMAC_SHA256_SIZE  SYN_SHA256_DIGEST_SIZE

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief HMAC-SHA256 context — caller-owned.
 *
 * Contains the inner SHA-256 context (used during update) and the
 * outer key pad (used during finalization).
 */
typedef struct {
    SYN_SHA256  inner;                              /**< Inner hash context   */
    uint8_t     o_key_pad[SYN_SHA256_BLOCK_SIZE];   /**< Outer key pad (K ⊕ opad) */
} SYN_HMAC_SHA256;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize HMAC-SHA256 with a key.
 *
 * If key_len > 64, the key is first hashed with SHA-256.
 *
 * @param ctx      HMAC context.
 * @param key      Secret key.
 * @param key_len  Key length in bytes.
 */
static inline void syn_hmac_sha256_init(SYN_HMAC_SHA256 *ctx,
                                         const void *key, size_t key_len)
{
    uint8_t k_buf[SYN_SHA256_BLOCK_SIZE];
    unsigned i;

    memset(k_buf, 0, sizeof(k_buf));

    if (key_len > SYN_SHA256_BLOCK_SIZE) {
        /* Keys longer than block size are hashed first */
        syn_sha256(key, key_len, k_buf);
    } else {
        memcpy(k_buf, key, key_len);
    }

    /* Compute i_key_pad = K ⊕ 0x36  and  o_key_pad = K ⊕ 0x5C */
    uint8_t i_key_pad[SYN_SHA256_BLOCK_SIZE];
    for (i = 0; i < SYN_SHA256_BLOCK_SIZE; i++) {
        i_key_pad[i]        = k_buf[i] ^ 0x36u;
        ctx->o_key_pad[i]   = k_buf[i] ^ 0x5Cu;
    }

    /* Inner hash: SHA-256(i_key_pad || message...) */
    syn_sha256_init(&ctx->inner);
    syn_sha256_update(&ctx->inner, i_key_pad, SYN_SHA256_BLOCK_SIZE);
}

/**
 * @brief Feed message data into the HMAC.
 * @param ctx   HMAC context.
 * @param data  Message data.
 * @param len   Length in bytes.
 */
static inline void syn_hmac_sha256_update(SYN_HMAC_SHA256 *ctx,
                                            const void *data, size_t len)
{
    syn_sha256_update(&ctx->inner, data, len);
}

/**
 * @brief Finalize and produce the 32-byte HMAC.
 * @param ctx  HMAC context.
 * @param mac  Output buffer (must be at least 32 bytes).
 */
static inline void syn_hmac_sha256_final(SYN_HMAC_SHA256 *ctx,
                                           uint8_t mac[SYN_HMAC_SHA256_SIZE])
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
}

/**
 * @brief Compute HMAC-SHA256 in one shot.
 * @param key       Secret key.
 * @param key_len   Key length in bytes.
 * @param data      Message data.
 * @param data_len  Message length in bytes.
 * @param mac       Output buffer (must be at least 32 bytes).
 */
static inline void syn_hmac_sha256(const void *key, size_t key_len,
                                     const void *data, size_t data_len,
                                     uint8_t mac[SYN_HMAC_SHA256_SIZE])
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
