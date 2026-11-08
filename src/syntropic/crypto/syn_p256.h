/**
 * @file syn_p256.h
 * @brief NIST P-256 (secp256r1 / prime256v1) Elliptic Curve Cryptography.
 * @ingroup syn_crypto
 *
 * Provides zero-allocation, cleanroom NIST P-256 scalar multiplication,
 * ECDH shared secret generation, and FIPS 186-4 ECDSA signature verification.
 */

#ifndef SYN_P256_H
#define SYN_P256_H

#include "../common/syn_defs.h"

#if !defined(SYN_USE_P256) || SYN_USE_P256
/** @brief Non-zero when NIST P-256 module is enabled. */
#define SYN_P256_ENABLED 1
#else
/** @brief Non-zero when NIST P-256 module is enabled. */
#define SYN_P256_ENABLED 0
#endif

#if SYN_P256_ENABLED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief NIST P-256 coordinate / scalar byte size (32 bytes). */
#define SYN_P256_BYTE_LEN 32U

/**
 * @brief Multiply the P-256 base generator G by a 32-byte scalar.
 *
 * Computes Q = scalar * G.
 *
 * @param scalar 32-byte scalar in big-endian format.
 * @param pub_x  [out] 32-byte X-coordinate of resulting point.
 * @param pub_y  [out] 32-byte Y-coordinate of resulting point.
 * @return true on success, false if scalar is 0 or >= curve order.
 */
bool syn_p256_base_mul(const uint8_t scalar[SYN_P256_BYTE_LEN], uint8_t pub_x[SYN_P256_BYTE_LEN],
                       uint8_t pub_y[SYN_P256_BYTE_LEN]);

/**
 * @brief Multiply an arbitrary P-256 point by a 32-byte scalar.
 *
 * Computes R = scalar * P.
 *
 * @param scalar 32-byte scalar in big-endian format.
 * @param px     32-byte X-coordinate of input point P.
 * @param py     32-byte Y-coordinate of input point P.
 * @param rx     [out] 32-byte X-coordinate of resulting point R.
 * @param ry     [out] 32-byte Y-coordinate of resulting point R.
 * @return true on success, false if point is invalid or scalar out of range.
 */
bool syn_p256_point_mul(const uint8_t scalar[SYN_P256_BYTE_LEN],
                        const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN],
                        uint8_t rx[SYN_P256_BYTE_LEN], uint8_t ry[SYN_P256_BYTE_LEN]);

/**
 * @brief Perform ECDH Key Agreement.
 *
 * Computes shared_secret = priv_key * peer_pub_point.
 *
 * @param priv_key       32-byte private key scalar.
 * @param peer_pub_x     32-byte X-coordinate of peer's public key.
 * @param peer_pub_y     32-byte Y-coordinate of peer's public key.
 * @param shared_secret  [out] 32-byte shared secret (X-coordinate of product point).
 * @return true on success, false on invalid point or scalar.
 */
bool syn_p256_ecdh(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_x[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_y[SYN_P256_BYTE_LEN],
                   uint8_t shared_secret[SYN_P256_BYTE_LEN]);

/**
 * @brief Generate a NIST P-256 ECDSA signature (FIPS 186-4).
 *
 * Computes signature (r, s) for a given message hash using a private key and nonce k.
 *
 * @param priv_key  32-byte private key scalar.
 * @param nonce_k   32-byte ephemeral private nonce k (must be in [1, n-1]).
 * @param hash      32-byte message hash (typically SHA-256).
 * @param r_out     [out] 32-byte signature component r.
 * @param s_out     [out] 32-byte signature component s.
 * @return true on success, false on invalid parameters.
 */
bool syn_p256_sign_ecdsa(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                         const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                         const uint8_t hash[SYN_P256_BYTE_LEN], uint8_t r_out[SYN_P256_BYTE_LEN],
                         uint8_t s_out[SYN_P256_BYTE_LEN]);

/**
 * @brief Generate a deterministic NIST P-256 ECDSA signature (RFC 6979 / FIPS 186-4).
 *
 * Derives the ephemeral nonce k deterministically using NIST SP 800-90A HMAC-DRBG (SHA-256)
 * keyed with the private key and message hash.
 *
 * @param priv_key   32-byte private key scalar.
 * @param hash       32-byte message hash (typically SHA-256).
 * @param r_out      [out] 32-byte signature component r.
 * @param s_out      [out] 32-byte signature component s.
 * @return true on success, false on invalid parameters.
 */
bool syn_p256_sign_ecdsa_deterministic(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                       const uint8_t hash[SYN_P256_BYTE_LEN],
                                       uint8_t r_out[SYN_P256_BYTE_LEN],
                                       uint8_t s_out[SYN_P256_BYTE_LEN]);

/**
 * @brief Verify a NIST P-256 ECDSA signature (FIPS 186-4).
 *
 * @param hash    32-byte message hash (typically SHA-256).
 * @param r       32-byte signature component r.
 * @param s       32-byte signature component s.
 * @param pub_x   32-byte public key X-coordinate.
 * @param pub_y   32-byte public key Y-coordinate.
 * @return true if signature is mathematically valid, false otherwise.
 */
bool syn_p256_verify_ecdsa(const uint8_t hash[SYN_P256_BYTE_LEN],
                           const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                           const uint8_t pub_x[SYN_P256_BYTE_LEN],
                           const uint8_t pub_y[SYN_P256_BYTE_LEN]);

/**
 * @brief Validate if a point (x, y) lies on the NIST P-256 curve: y^2 = x^3 - 3x + b (mod p).
 *
 * @param px 32-byte X-coordinate.
 * @param py 32-byte Y-coordinate.
 * @return true if point is on curve, false otherwise.
 */
bool syn_p256_is_on_curve(const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_P256_ENABLED */

#endif /* SYN_P256_H */
