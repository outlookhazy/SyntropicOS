/**
 * @file syn_x25519.h
 * @brief X25519 Diffie-Hellman key exchange — RFC 7748, pure C99.
 *
 * Elliptic-curve Diffie-Hellman on Curve25519 using the Montgomery
 * ladder. Provides shared-secret computation and public-key derivation.
 *
 * This is the most computationally expensive WireGuard operation
 * (~1–2 seconds on Cortex-M4 @ 64 MHz) but only runs during
 * handshake, not on every packet.
 *
 * @par Usage
 * @code
 *   uint8_t my_priv[32], my_pub[32], shared[32];
 *
 *   // Generate keypair (private key = 32 random bytes, then clamp):
 *   fill_random(my_priv, 32);
 *   syn_x25519_clamp(my_priv);
 *   syn_x25519_pubkey(my_pub, my_priv);
 *
 *   // Compute shared secret with peer's public key:
 *   syn_x25519(shared, my_priv, peer_pub);
 * @endcode
 * @ingroup syn_crypto
 */

#ifndef SYN_X25519_H
#define SYN_X25519_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_X25519) || SYN_USE_X25519

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Total cooperative steps in an X25519 scalar multiplication (255 ladder + 254 inversion).
 */
#define SYN_X25519_TOTAL_STEPS 509U

/** @brief Default step count per cooperative protothread slice. */
#define SYN_X25519_DEFAULT_CHUNK 16U

/**
 * @brief Resumable X25519 scalar multiplication working state (caller-owned, zero-heap).
 */
typedef struct {
    uint8_t z[32]; /**< Clamped private scalar bytes */
    int64_t x[16]; /**< Unpacked base point */
    int64_t a[16]; /**< Montgomery ladder X_2 / final numerator */
    int64_t b[16]; /**< Montgomery ladder X_3 */
    int64_t c[16]; /**< Montgomery ladder Z_2 / field inversion accumulator */
    int64_t d[16]; /**< Montgomery ladder Z_3 / field inversion base Z_2 */
    uint16_t step; /**< Next step index in [0, SYN_X25519_TOTAL_STEPS] */
    bool active;   /**< true while initialized and awaiting completion */
} SYN_X25519_Ctx;

/**
 * @brief X25519 scalar multiplication.
 *
 * Computes shared = scalar * point on Curve25519.
 * Callers invoking syn_x25519() directly in Diffie-Hellman protocols MUST
 * verify in constant time that shared_out is not all-zero (RFC 7748 Section 6.1)
 * to reject low-order input points.
 *
 * @param shared_out  Output: 32-byte shared secret.
 * @param scalar      32-byte private scalar (clamped internally per RFC 7748).
 * @param point       32-byte public point (peer's public key).
 */
void syn_x25519(uint8_t shared_out[32], const uint8_t scalar[32], const uint8_t point[32]);

/**
 * @brief Derive a public key from a private key.
 *
 * Computes public = scalar * basepoint (the Curve25519 generator, u=9).
 *
 * @param public_out   Output: 32-byte public key.
 * @param private_key  32-byte private key (should be clamped).
 */
void syn_x25519_pubkey(uint8_t public_out[32], const uint8_t private_key[32]);

/**
 * @brief Clamp a 32-byte private key per RFC 7748.
 *
 * Sets bits to ensure the scalar is a multiple of the cofactor (8)
 * and has the high bit set for constant-time operation.
 *
 * @param key  32-byte key to clamp (modified in place).
 */
static inline void syn_x25519_clamp(uint8_t key[32])
{
    key[0] &= 248;  /* Clear low 3 bits (multiple of 8) */
    key[31] &= 127; /* Clear high bit */
    key[31] |= 64;  /* Set second-highest bit */
}

/**
 * @brief Initialize a resumable X25519 scalar multiplication context.
 * @param ctx    Caller-owned X25519 working context.
 * @param scalar 32-byte private scalar (clamped internally per RFC 7748).
 * @param point  32-byte public point (peer's public key or basepoint).
 * @return true if initialized, false on NULL pointer.
 */
bool syn_x25519_init(SYN_X25519_Ctx *ctx, const uint8_t scalar[32], const uint8_t point[32]);

/**
 * @brief Initialize a resumable X25519 public-key derivation context (scalar * basepoint).
 * @param ctx         Caller-owned X25519 working context.
 * @param private_key 32-byte private key.
 * @return true if initialized, false on NULL pointer.
 */
bool syn_x25519_pubkey_init(SYN_X25519_Ctx *ctx, const uint8_t private_key[32]);

/**
 * @brief Advance an initialized X25519 context by up to @p max_steps iterations.
 *
 * Automatically wipes @p ctx via syn_secure_zero() upon completing step 509.
 *
 * @param ctx        Active X25519 context.
 * @param max_steps  Maximum ladder/inversion steps to execute (> 0).
 * @param shared_out [out] 32-byte output buffer populated when SYN_OK is returned.
 * @return SYN_BUSY while steps remain, SYN_OK on completion, or SYN_INVALID_PARAM.
 */
SYN_Status syn_x25519_step(SYN_X25519_Ctx *ctx, uint16_t max_steps, uint8_t shared_out[32]);

/**
 * @brief Cooperative child protothread for sliced X25519 scalar multiplication.
 *
 * Executes @p steps_per_chunk steps per invocation (defaults to SYN_X25519_DEFAULT_CHUNK
 * when 0) and yields PT_MORE via PT_CONTINUE(pt) while SYN_BUSY. Composes with
 * PT_SPAWN_IF_NEEDED(pt, task, child, syn_x25519_pt(...)) for demand-driven yielding.
 *
 * @param pt              Child protothread control block.
 * @param ctx             Initialized X25519 context.
 * @param steps_per_chunk Steps per slice (0 uses SYN_X25519_DEFAULT_CHUNK).
 * @param shared_out      [out] 32-byte output buffer populated on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_x25519_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx, uint16_t steps_per_chunk,
                            uint8_t shared_out[32]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_X25519 */

#endif /* SYN_X25519_H */
