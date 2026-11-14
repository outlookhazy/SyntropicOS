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

/**
 * @brief Resumable X25519 scalar multiplication working state (caller-owned, zero-heap).
 */
typedef struct {
    uint8_t z[32]; /**< Clamped private scalar bytes */
    int32_t x[16]; /**< Unpacked base point */
    int32_t a[16]; /**< Montgomery ladder X_2 / final numerator */
    int32_t b[16]; /**< Montgomery ladder X_3 */
    int32_t c[16]; /**< Montgomery ladder Z_2 / field inversion accumulator */
    int32_t d[16]; /**< Montgomery ladder Z_3 / field inversion base Z_2 */
    SYN_PT sub_pt; /**< Child protothread continuation for ladder and inversion loops */
    uint16_t idx;  /**< Loop step index within active child protothread */
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
    key[0] &= 248U;  /* Clear low 3 bits (multiple of 8) */
    key[31] &= 127U; /* Clear high bit */
    key[31] |= 64U;  /* Set second-highest bit */
}

/**
 * @brief Constant-time check that a 32-byte X25519 shared secret is non-zero (RFC 7748 §6.1).
 *
 * Diffie-Hellman protocol consumers must verify that the scalar multiplication
 * output is not the all-zero value to reject low-order points.
 *
 * @param secret 32-byte shared secret buffer.
 * @return true if at least one byte is non-zero, false if all-zero.
 */
static inline bool syn_x25519_is_nonzero(const uint8_t secret[32])
{
    uint8_t acc = 0U;
    for (size_t i = 0U; i < 32U; i++) {
        acc |= secret[i];
    }
    return acc != 0U;
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
 * @brief Securely zeroize an X25519 working context (for cancellation / abort).
 * @param ctx Context to wipe (safe if NULL).
 */
void syn_x25519_wipe(SYN_X25519_Ctx *ctx);

/**
 * @brief Cooperative protothread for sliced X25519 scalar multiplication.
 *
 * Executes one Montgomery ladder or field inversion step per slice and yields
 * PT_MORE via PT_CONTINUE(pt). Automatically wipes @p ctx before completing
 * with PT_ENDED. Composes with PT_SPAWN_IF_NEEDED(pt, task, child, syn_x25519_pt(...))
 * for demand-driven yielding.
 *
 * @param pt         Protothread control block.
 * @param ctx        Initialized X25519 context.
 * @param shared_out [out] 32-byte output buffer populated on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_x25519_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx, uint8_t shared_out[32]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_X25519 */

#endif /* SYN_X25519_H */
