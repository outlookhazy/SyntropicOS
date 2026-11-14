/**
 * @file syn_hmac_drbg.h
 * @brief NIST SP 800-90A HMAC-DRBG (Deterministic Random Bit Generator) with SHA-256.
 *
 * Implements a cryptographically secure pseudo-random number generator conforming to
 * NIST SP 800-90A Rev. 1 Section 10.1.2.
 *
 * Features:
 *   - **HMAC-SHA256 PRF**: 256-bit maximum security strength.
 *   - **Deterministic Instantiation**: Supports entropy input, nonce, and personalization string.
 *   - **Reseeding**: Allows periodic re-injection of entropy and additional input.
 *   - **Prediction Resistance (PR)**: Optional on-demand reseeding per generate request.
 *   - **Zero Dynamic Allocation**: 72-byte caller-owned context.
 *   - **Secure Zeroization**: Explicit memory wipe to scrub cryptographic material.
 *
 * @par Usage
 * @code
 *   SYN_HMAC_DRBG drbg;
 *   const uint8_t entropy[32] = { ... 256 bits of hardware entropy ... };
 *   const uint8_t nonce[16]   = { ... 128 bits unique nonce ... };
 *
 *   // 1. Instantiate
 *   syn_hmac_drbg_init(&drbg, entropy, sizeof(entropy), nonce, sizeof(nonce), NULL, 0);
 *
 *   // 2. Generate Random Bytes
 *   uint8_t random_bytes[32];
 *   syn_hmac_drbg_generate(&drbg, random_bytes, sizeof(random_bytes), NULL, 0);
 *
 *   // 3. Wipe When Done
 *   syn_hmac_drbg_wipe(&drbg);
 * @endcode
 * @ingroup syn_crypto
 */

#ifndef SYN_HMAC_DRBG_H
#define SYN_HMAC_DRBG_H

#include "../common/syn_defs.h"

#if (!defined(SYN_USE_HMAC_DRBG) || SYN_USE_HMAC_DRBG) && \
    (!defined(SYN_USE_SHA256) || SYN_USE_SHA256)
/** @brief Non-zero when HMAC-DRBG and SHA-256 dependencies are enabled. */
#define SYN_HMAC_DRBG_ENABLED 1
#else
/** @brief Non-zero when HMAC-DRBG and SHA-256 dependencies are enabled. */
#define SYN_HMAC_DRBG_ENABLED 0
#endif

#if SYN_HMAC_DRBG_ENABLED

#include "syn_hmac.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Minimum entropy length in bytes for 256-bit security strength (32 bytes). */
#define SYN_HMAC_DRBG_MIN_ENTROPY_LEN 32U

/** @brief Maximum output bytes allowed per generate request (2^19 bits = 64 KiB). */
#define SYN_HMAC_DRBG_MAX_REQUEST_BYTES 65536UL

/** @brief Default maximum number of generate requests before a reseed is required. */
#define SYN_HMAC_DRBG_DEFAULT_RESEED_INTERVAL 10000U

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief HMAC-DRBG state context (NIST SP 800-90A).
 */
typedef struct {
    uint8_t k[SYN_HMAC_SHA256_SIZE]; /**< Internal working key K (32 bytes) */
    uint8_t v[SYN_HMAC_SHA256_SIZE]; /**< Internal working state V (32 bytes) */
    uint32_t reseed_counter;         /**< Number of generate calls since init/reseed */
    uint32_t reseed_interval;        /**< Maximum generate calls before reseed required */
    bool initialized;                /**< True if DRBG has been instantiated */
} SYN_HMAC_DRBG;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Instantiate the HMAC-DRBG state with initial entropy, nonce, and optional personalization.
 *
 * Implements NIST SP 800-90A Rev 1 Section 10.1.2.3.
 *
 * @param ctx       Pointer to caller-allocated DRBG context.
 * @param entropy   Initial entropy input buffer (must be at least 32 bytes).
 * @param ent_len   Length of entropy in bytes (must be >= 32).
 * @param nonce     Nonce buffer (can be NULL if ent_len includes extra entropy).
 * @param nonce_len Length of nonce in bytes.
 * @param pers_str  Optional personalization string (can be NULL).
 * @param pers_len  Length of personalization string in bytes.
 * @return SYN_OK on success, SYN_INVALID_PARAM if inputs are invalid.
 */
SYN_Status syn_hmac_drbg_init(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                              const uint8_t *nonce, size_t nonce_len, const uint8_t *pers_str,
                              size_t pers_len);

/**
 * @brief Reseed the HMAC-DRBG with fresh entropy and optional additional input.
 *
 * Implements NIST SP 800-90A Rev 1 Section 10.1.2.4.
 *
 * @param ctx       Pointer to initialized DRBG context.
 * @param entropy   Fresh entropy input buffer (must be at least 32 bytes).
 * @param ent_len   Length of entropy in bytes (must be >= 32).
 * @param add_input Optional additional input buffer (can be NULL).
 * @param add_len   Length of additional input in bytes.
 * @return SYN_OK on success, SYN_INVALID_PARAM if inputs or state are invalid.
 */
SYN_Status syn_hmac_drbg_reseed(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                                const uint8_t *add_input, size_t add_len);

/**
 * @brief Generate pseudorandom bytes from the HMAC-DRBG.
 *
 * Implements NIST SP 800-90A Rev 1 Section 10.1.2.5 without prediction resistance.
 *
 * @param ctx       Pointer to initialized DRBG context.
 * @param out       [out] Destination buffer for generated random bytes.
 * @param out_len   Number of random bytes to generate (<= 65536).
 * @param add_input Optional additional input buffer (can be NULL).
 * @param add_len   Length of additional input in bytes.
 * @return SYN_OK on success,
 *         SYN_ERROR if reseed interval is exceeded (reseed required),
 *         SYN_INVALID_PARAM if parameters or state are invalid.
 */
SYN_Status syn_hmac_drbg_generate(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                  const uint8_t *add_input, size_t add_len);

/**
 * @brief Generate pseudorandom bytes with Prediction Resistance (PR).
 *
 * Reseeds the DRBG with fresh entropy before generating pseudorandom bytes.
 *
 * @param ctx       Pointer to initialized DRBG context.
 * @param out       [out] Destination buffer for generated random bytes.
 * @param out_len   Number of random bytes to generate (<= 65536).
 * @param entropy   Fresh entropy for prediction resistance (must be >= 32 bytes).
 * @param ent_len   Length of entropy in bytes (must be >= 32).
 * @param add_input Optional additional input buffer (can be NULL).
 * @param add_len   Length of additional input in bytes.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_hmac_drbg_generate_pr(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                     const uint8_t *entropy, size_t ent_len,
                                     const uint8_t *add_input, size_t add_len);

/**
 * @brief Set the maximum reseed interval for the DRBG instance.
 *
 * @param ctx      Pointer to DRBG context.
 * @param interval Maximum number of generate calls before reseed is required (default: 10000).
 */
void syn_hmac_drbg_set_reseed_interval(SYN_HMAC_DRBG *ctx, uint32_t interval);

/**
 * @brief Securely wipe internal working key, state vector, and counters.
 *
 * @param ctx Pointer to DRBG context.
 */
void syn_hmac_drbg_wipe(SYN_HMAC_DRBG *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_HMAC_DRBG */

#endif /* SYN_HMAC_DRBG_H */
