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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_X25519 */

#endif /* SYN_X25519_H */
