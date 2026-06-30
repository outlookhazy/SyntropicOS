/**
 * @file syn_sha256.h
 * @brief SHA-256 cryptographic hash — pure C99, zero dependencies.
 *
 * Follows the same streaming pattern as syn_crc.h:
 *   init → update (repeated) → final
 *
 * The context struct is caller-owned (~112 bytes on 32-bit targets).
 * No heap allocation, no floating point, no external libraries.
 *
 * @par Usage
 * @code
 *   // One-shot:
 *   uint8_t hash[32];
 *   syn_sha256("abc", 3, hash);
 *
 *   // Streaming:
 *   SYN_SHA256 ctx;
 *   syn_sha256_init(&ctx);
 *   syn_sha256_update(&ctx, chunk1, len1);
 *   syn_sha256_update(&ctx, chunk2, len2);
 *   syn_sha256_final(&ctx, hash);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_SHA256_H
#define SYN_SHA256_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief SHA-256 digest size in bytes. */
#define SYN_SHA256_DIGEST_SIZE  32

/** @brief SHA-256 block size in bytes. */
#define SYN_SHA256_BLOCK_SIZE   64

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief SHA-256 hash context — caller-owned.
 *
 * Typical size: 112 bytes (8×4 state + 64 buffer + 2×4 counters).
 */
typedef struct {
    uint32_t state[8];                      /**< Running hash state (H0–H7) */
    uint8_t  buf[SYN_SHA256_BLOCK_SIZE];    /**< Partial block buffer       */
    uint32_t buf_len;                       /**< Bytes in buffer (0–63)     */
    uint32_t total_len_lo;                  /**< Total message length, low  */
    uint32_t total_len_hi;                  /**< Total message length, high */
} SYN_SHA256;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a SHA-256 context.
 * @param ctx  Context to initialize.
 */
void syn_sha256_init(SYN_SHA256 *ctx);

/**
 * @brief Feed data into the hash.
 *
 * Can be called repeatedly with arbitrary chunk sizes.
 *
 * @param ctx   SHA-256 context.
 * @param data  Data to hash.
 * @param len   Length in bytes.
 */
void syn_sha256_update(SYN_SHA256 *ctx, const void *data, size_t len);

/**
 * @brief Finalize the hash and produce the 32-byte digest.
 *
 * After calling this, the context must be re-initialized before reuse.
 *
 * @param ctx   SHA-256 context.
 * @param hash  Output buffer (must be at least 32 bytes).
 */
void syn_sha256_final(SYN_SHA256 *ctx, uint8_t hash[SYN_SHA256_DIGEST_SIZE]);

/**
 * @brief Compute SHA-256 of a complete buffer (one-shot convenience).
 * @param data  Data to hash.
 * @param len   Length in bytes.
 * @param hash  Output buffer (must be at least 32 bytes).
 */
static inline void syn_sha256(const void *data, size_t len,
                               uint8_t hash[SYN_SHA256_DIGEST_SIZE])
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
