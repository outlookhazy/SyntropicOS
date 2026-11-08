/**
 * @file syn_ed25519.h
 * @brief Pure C99 Ed25519 Key Generation, Signing & Verification (RFC 8032).
 * @ingroup syn_crypto
 */

#ifndef SYN_ED25519_H
#define SYN_ED25519_H

#include "../common/syn_defs.h"

#if (!defined(SYN_USE_ED25519) || SYN_USE_ED25519) && (!defined(SYN_USE_SHA512) || SYN_USE_SHA512)
/** @brief Non-zero when Ed25519 and SHA-512 dependencies are enabled. */
#define SYN_ED25519_ENABLED 1
#else
/** @brief Non-zero when Ed25519 and SHA-512 dependencies are enabled. */
#define SYN_ED25519_ENABLED 0
#endif

#if SYN_ED25519_ENABLED

#include "syn_sha512.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Size of Ed25519 public key in bytes (32). */
#define SYN_ED25519_PUBLIC_KEY_SIZE 32U
/** @brief Size of Ed25519 secret key (seed) in bytes (32). */
#define SYN_ED25519_SECRET_KEY_SIZE 32U
/** @brief Size of Ed25519 seed in bytes (32). */
#define SYN_ED25519_SEED_SIZE 32U
/** @brief Size of Ed25519 signature in bytes (64). */
#define SYN_ED25519_SIGNATURE_SIZE 64U

/** @brief Alias for SHA-512 context */
typedef SYN_SHA512 SYN_SHA512_Ctx;

/**
 * @brief Derive a 32-byte Ed25519 public key from a 32-byte secret key (RFC 8032 Section 5.1.5).
 *
 * @param secret_key 32-byte private key/seed.
 * @param public_key Output 32-byte compressed Edwards public key.
 * @return true on success, false on invalid parameter.
 */
bool syn_ed25519_publickey(const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Create an Ed25519 keypair from a 32-byte random seed (RFC 8032).
 *
 * @param public_key Output 32-byte public key.
 * @param secret_key Output 32-byte secret key (copies seed).
 * @param seed       Input 32-byte random seed.
 * @return true on success, false on invalid parameter.
 */
bool syn_ed25519_create_keypair(uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                                uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                                const uint8_t seed[SYN_ED25519_SEED_SIZE]);

/**
 * @brief Generate an Ed25519 signature (RFC 8032 Section 5.1.6).
 *
 * @param msg        Message to sign.
 * @param msg_len    Length of message in bytes.
 * @param secret_key 32-byte private key.
 * @param public_key 32-byte public key (optional; if NULL, derived internally from secret_key).
 * @param sig        Output 64-byte signature (R || S).
 * @return true on success, false on invalid parameter.
 */
bool syn_ed25519_sign(const uint8_t *msg, size_t msg_len,
                      const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                      const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                      uint8_t sig[SYN_ED25519_SIGNATURE_SIZE]);

/**
 * @brief Verify an Ed25519 signature (RFC 8032 Section 5.1.7).
 *
 * @param sig        64-byte signature (R || S).
 * @param msg        Message bytes.
 * @param msg_len    Message length in bytes.
 * @param public_key 32-byte Ed25519 public key.
 * @return true if valid signature, false otherwise.
 */
bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Verify an Ed25519 signature from a precomputed 64-byte challenge digest H(R || A || M).
 *
 * @param sig        64-byte signature (R || S).
 * @param h          64-byte SHA-512 digest.
 * @param public_key 32-byte Ed25519 public key.
 * @return true if valid signature, false otherwise.
 */
bool syn_ed25519_verify_hash(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t h[64],
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ED25519_ENABLED */

#endif /* SYN_ED25519_H */
