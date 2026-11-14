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

#include "../pt/syn_pt.h"
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
 * @brief Resumable Ed25519 working context (caller-owned, zero-heap).
 */
typedef struct {
    int32_t p[4][16]; /**< Extended Edwards25519 point accumulator */
    int32_t q[4][16]; /**< Extended Edwards25519 base / unpacked peer point */
    union {
        int32_t aux[4][16]; /**< Inversion/pow2523 temporaries or Shamir sum point */
        SYN_SHA512 sha;     /**< Streaming SHA-512 context for nonce/challenge digests */
    } u;                    /**< Phase-disjoint scratch storage */
    const uint8_t *msg;     /**< Caller message buffer (valid until sign/verify completes) */
    size_t msg_len;         /**< Message length in bytes */
    size_t msg_off;         /**< Bytes hashed so far in active SHA-512 phase */
    uint8_t d[64];          /**< Expanded secret scalar / prefix or reduced challenge h */
    uint8_t scalar[32];     /**< Reduced nonce r (sign) or signature scalar S (verify) */
    uint8_t pk[32];         /**< Compressed 32-byte public key / scratch packed point */
    uint8_t r_pt[32];       /**< Compressed 32-byte signature R point */
    SYN_PT sub_pt;          /**< First-level child protothread continuation */
    SYN_PT sub2_pt;         /**< Nested child protothread continuation */
    uint16_t idx;           /**< Step index within active child protothread */
    uint8_t op;             /**< Active operation kind */
    bool has_pk;            /**< True when sign context received caller-supplied public key */
    bool prehashed;         /**< True when verify context initialized from precomputed H(R||A||M) */
    bool active;            /**< True while initialized and awaiting completion */
} SYN_Ed25519_Ctx;

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

/* ── Cooperative Resumable API ──────────────────────────────────────────── */

/**
 * @brief Securely zeroize a resumable Ed25519 working context (for cancellation / abort).
 * @param ctx Context to wipe (safe if NULL).
 */
void syn_ed25519_wipe(SYN_Ed25519_Ctx *ctx);

/**
 * @brief Initialize resumable Ed25519 public-key derivation from a 32-byte secret key.
 * @param ctx        Caller-owned Ed25519 working context.
 * @param secret_key 32-byte private key/seed.
 * @return true if initialized, false on NULL pointer.
 */
bool syn_ed25519_publickey_init(SYN_Ed25519_Ctx *ctx,
                                const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE]);

/**
 * @brief Cooperative child protothread for sliced Ed25519 public-key derivation.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt         Child protothread control block.
 * @param ctx        Initialized Ed25519 public-key context.
 * @param public_key [out] 32-byte compressed public key populated on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_ed25519_publickey_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                       uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Initialize resumable Ed25519 signing.
 *
 * @param ctx        Caller-owned Ed25519 working context.
 * @param msg        Message buffer (must remain valid until signing completes).
 * @param msg_len    Length of message in bytes.
 * @param secret_key 32-byte private key/seed.
 * @param public_key Optional 32-byte public key (if NULL, derived cooperatively in-context).
 * @return true if initialized, false on invalid parameter.
 */
bool syn_ed25519_sign_init(SYN_Ed25519_Ctx *ctx, const uint8_t *msg, size_t msg_len,
                           const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Cooperative child protothread for sliced Ed25519 signing.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt  Child protothread control block.
 * @param ctx Initialized Ed25519 signing context.
 * @param sig [out] 64-byte signature populated on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_ed25519_sign_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                  uint8_t sig[SYN_ED25519_SIGNATURE_SIZE]);

/**
 * @brief Initialize resumable Ed25519 signature verification over a message buffer.
 *
 * Rejects NULL inputs, non-canonical signature scalars (S >= L), and non-canonical
 * encoded public-key y-coordinates (y >= p) upfront.
 *
 * @param ctx        Caller-owned Ed25519 working context.
 * @param sig        64-byte signature (R || S).
 * @param msg        Message buffer (must remain valid until verification completes).
 * @param msg_len    Message length in bytes.
 * @param public_key 32-byte Ed25519 public key.
 * @return true if parameters and canonical encodings are valid and initialized; false otherwise.
 */
bool syn_ed25519_verify_init(SYN_Ed25519_Ctx *ctx, const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                             const uint8_t *msg, size_t msg_len,
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Initialize resumable Ed25519 verification from a precomputed 64-byte challenge digest.
 *
 * @param ctx        Caller-owned Ed25519 working context.
 * @param sig        64-byte signature (R || S).
 * @param h          64-byte SHA-512 challenge digest H(R || A || M).
 * @param public_key 32-byte Ed25519 public key.
 * @return true if parameters and canonical encodings are valid and initialized; false otherwise.
 */
bool syn_ed25519_verify_hash_init(SYN_Ed25519_Ctx *ctx,
                                  const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                                  const uint8_t h[64],
                                  const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/**
 * @brief Cooperative child protothread for sliced Ed25519 signature verification.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt        Child protothread control block.
 * @param ctx       Initialized Ed25519 verify context.
 * @param out_valid [out] Receives true if signature is valid on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_ed25519_verify_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, bool *out_valid);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ED25519_ENABLED */

#endif /* SYN_ED25519_H */
