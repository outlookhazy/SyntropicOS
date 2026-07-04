/**
 * @file syn_blake2s.h
 * @brief BLAKE2s cryptographic hash — RFC 7693, pure C99.
 *
 * Supports both unkeyed hashing and keyed MAC mode. WireGuard uses
 * both: unkeyed BLAKE2s-256 for chaining hash, keyed BLAKE2s for MAC.
 *
 * Context is caller-owned (~120 bytes on 32-bit targets). No heap.
 *
 * Also provides HMAC-BLAKE2s as static inline functions — used
 * internally by the WireGuard module for HKDF key derivation.
 *
 * @par Usage
 * @code
 *   // Unkeyed hash:
 *   uint8_t hash[32];
 *   syn_blake2s(data, len, hash, 32);
 *
 *   // Keyed MAC (16-byte tag):
 *   uint8_t mac[16];
 *   syn_blake2s_mac(key, 32, data, len, mac, 16);
 *
 *   // Streaming:
 *   SYN_BLAKE2s ctx;
 *   syn_blake2s_init(&ctx, 32);
 *   syn_blake2s_update(&ctx, chunk1, len1);
 *   syn_blake2s_update(&ctx, chunk2, len2);
 *   syn_blake2s_final(&ctx, hash);
 * @endcode
 * @ingroup syn_crypto
 */

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

#define SYN_BLAKE2S_BLOCK_SIZE   64  /**< Input block size (bytes)       */
#define SYN_BLAKE2S_MAX_DIGEST   32  /**< Maximum digest size (bytes)    */
#define SYN_BLAKE2S_MAX_KEY      32  /**< Maximum key size (bytes)       */

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief BLAKE2s hash context — caller-owned.
 *
 * Typical size: ~120 bytes on a 32-bit target.
 */
typedef struct {
    uint32_t h[8];                          /**< Running hash state          */
    uint32_t t[2];                          /**< Byte counter (low, high)    */
    uint8_t  buf[SYN_BLAKE2S_BLOCK_SIZE];   /**< Partial block buffer        */
    uint8_t  buflen;                        /**< Bytes in buffer (0–64)      */
    uint8_t  outlen;                        /**< Desired output length       */
} SYN_BLAKE2s;

/* ── Core API ───────────────────────────────────────────────────────────── */

/**
 * @brief Initialize BLAKE2s for unkeyed hashing.
 *
 * @param ctx     Context to initialize.
 * @param outlen  Desired digest length (1–32).
 */
void syn_blake2s_init(SYN_BLAKE2s *ctx, size_t outlen);

/**
 * @brief Initialize BLAKE2s for keyed hashing (MAC mode).
 *
 * @param ctx     Context to initialize.
 * @param key     Secret key.
 * @param keylen  Key length (1–32).
 * @param outlen  Desired digest length (1–32).
 */
void syn_blake2s_init_keyed(SYN_BLAKE2s *ctx,
                            const void *key, size_t keylen,
                            size_t outlen);

/**
 * @brief Feed data into the hash.
 *
 * @param ctx   BLAKE2s context.
 * @param data  Data to hash.
 * @param len   Length in bytes.
 */
void syn_blake2s_update(SYN_BLAKE2s *ctx, const void *data, size_t len);

/**
 * @brief Finalize and produce the digest.
 *
 * After calling this, the context must be re-initialized before reuse.
 *
 * @param ctx   BLAKE2s context.
 * @param out   Output buffer (must be at least ctx->outlen bytes).
 */
void syn_blake2s_final(SYN_BLAKE2s *ctx, uint8_t *out);

/* ── Convenience ────────────────────────────────────────────────────────── */

/**
 * @brief One-shot unkeyed BLAKE2s hash.
 *
 * @param data    Data to hash.
 * @param len     Length in bytes.
 * @param out     Output buffer.
 * @param outlen  Desired digest length (1–32).
 */
static inline void syn_blake2s(const void *data, size_t len,
                               uint8_t *out, size_t outlen)
{
    SYN_BLAKE2s ctx;
    syn_blake2s_init(&ctx, outlen);
    syn_blake2s_update(&ctx, data, len);
    syn_blake2s_final(&ctx, out);
}

/**
 * @brief One-shot keyed BLAKE2s MAC.
 *
 * @param key     Secret key.
 * @param keylen  Key length (1–32).
 * @param data    Data to authenticate.
 * @param len     Data length in bytes.
 * @param out     Output buffer.
 * @param outlen  Desired MAC length (1–32).
 */
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

/**
 * @brief HMAC-BLAKE2s context — caller-owned.
 *
 * Used internally by HKDF for WireGuard key derivation.
 */
typedef struct {
    SYN_BLAKE2s inner;                            /**< Inner hash context    */
    uint8_t     o_key_pad[SYN_BLAKE2S_BLOCK_SIZE]; /**< Outer key pad       */
} SYN_HMAC_BLAKE2s;

/**
 * @brief Initialize HMAC-BLAKE2s with a key.
 *
 * @param ctx     HMAC context.
 * @param key     Secret key.
 * @param keylen  Key length in bytes.
 */
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

/**
 * @brief Feed message data into the HMAC.
 * @param ctx   HMAC context.
 * @param data  Message data.
 * @param len   Data length in bytes.
 */
static inline void syn_hmac_blake2s_update(SYN_HMAC_BLAKE2s *ctx,
                                           const void *data, size_t len)
{
    syn_blake2s_update(&ctx->inner, data, len);
}

/**
 * @brief Finalize HMAC-BLAKE2s and produce 32-byte MAC.
 * @param ctx HMAC context.
 * @param mac Output buffer for the 32-byte MAC.
 */
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

/**
 * @brief One-shot HMAC-BLAKE2s.
 * @param key     Secret key.
 * @param keylen  Key length in bytes.
 * @param data    Message data.
 * @param datalen Message length in bytes.
 * @param mac     Output buffer for the 32-byte MAC.
 */
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
