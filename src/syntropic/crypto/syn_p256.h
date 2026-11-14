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

#include "../pt/syn_pt.h"
#include "syn_hmac_drbg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief NIST P-256 coordinate / scalar byte size (32 bytes). */
#define SYN_P256_BYTE_LEN 32U

/**
 * @brief Jacobian projective point used inside resumable P-256 contexts.
 */
typedef struct {
    uint32_t x[8]; /**< Projective coordinate X */
    uint32_t y[8]; /**< Projective coordinate Y */
    uint32_t z[8]; /**< Projective coordinate Z */
    bool infinity; /**< True if point is at infinity */
} SYN_P256_Point;

/**
 * @brief Resumable NIST P-256 working context (caller-owned, zero-heap).
 */
typedef struct {
    SYN_P256_Point acc;      /**< Active Jacobian point accumulator */
    SYN_P256_Point table[3]; /**< Windowed / Shamir precomputed points */
    uint32_t k1[8];          /**< Primary scalar (k, nonce_k, or u1) */
    uint32_t k2[8];          /**< Secondary scalar (private key d or u2) */
    uint32_t px[8];          /**< Base point X coordinate */
    uint32_t py[8];          /**< Base point Y coordinate */
    uint32_t aux[8];         /**< Reduced hash e (sign) or signature r (verify) */
    uint32_t inv_res[8];     /**< Modular inversion accumulator */
    uint32_t inv_base[8];    /**< Modular inversion base */
#if SYN_HMAC_DRBG_ENABLED
    SYN_HMAC_DRBG drbg; /**< RFC 6979 deterministic nonce generator */
#endif
    SYN_PT sub_pt;        /**< Child protothread continuation for inner curve/inversion loops */
    uint16_t idx;         /**< Sub-step index within active child protothread */
    uint8_t op;           /**< Active operation kind */
    uint8_t attempts;     /**< Sign nonce attempts executed */
    uint8_t max_attempts; /**< Max sign nonce attempts allowed */
    bool use_drbg;        /**< True when drawing k from RFC 6979 DRBG */
    bool ok;              /**< Latched signing outcome across context zeroization */
    bool active;          /**< True while initialized and awaiting completion */
} SYN_P256_Ctx;

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

/* ── Cooperative Resumable API ──────────────────────────────────────────── */

/**
 * @brief Securely zeroize a resumable P-256 working context (for cancellation / abort).
 * @param ctx Context to wipe (safe if NULL).
 */
void syn_p256_wipe(SYN_P256_Ctx *ctx);

/**
 * @brief Initialize resumable P-256 base-generator multiplication (Q = scalar * G).
 * @param ctx    Caller-owned P-256 working context.
 * @param scalar 32-byte big-endian scalar in [1, n-1].
 * @return true if initialized, false on NULL or out-of-range scalar.
 */
bool syn_p256_base_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN]);

/**
 * @brief Initialize resumable P-256 arbitrary-point multiplication / ECDH (R = scalar * P).
 * @param ctx    Caller-owned P-256 working context.
 * @param scalar 32-byte big-endian scalar in [1, n-1].
 * @param px     32-byte X-coordinate of input point P.
 * @param py     32-byte Y-coordinate of input point P.
 * @return true if initialized, false on NULL, off-curve point, or out-of-range scalar.
 */
bool syn_p256_point_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN],
                             const uint8_t px[SYN_P256_BYTE_LEN],
                             const uint8_t py[SYN_P256_BYTE_LEN]);

/**
 * @brief Cooperative child protothread for sliced P-256 scalar multiplication / ECDH.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt  Child protothread control block.
 * @param ctx Initialized P-256 scalar-mul context.
 * @param rx  [out] 32-byte affine X-coordinate output.
 * @param ry  [out] Optional 32-byte affine Y-coordinate output (may be NULL).
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_p256_mul_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t rx[SYN_P256_BYTE_LEN],
                              uint8_t ry[SYN_P256_BYTE_LEN]);

/**
 * @brief Initialize resumable P-256 ECDSA signing with an explicit nonce k.
 * @param ctx      Caller-owned P-256 working context.
 * @param priv_key 32-byte private key scalar in [1, n-1].
 * @param nonce_k  32-byte ephemeral nonce k.
 * @param hash     32-byte message digest.
 * @return true if initialized, false on NULL or invalid private key.
 */
bool syn_p256_sign_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                        const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                        const uint8_t hash[SYN_P256_BYTE_LEN]);

/**
 * @brief Initialize resumable deterministic P-256 ECDSA signing (RFC 6979 / HMAC-DRBG).
 * @param ctx      Caller-owned P-256 working context.
 * @param priv_key 32-byte private key scalar in [1, n-1].
 * @param hash     32-byte message digest.
 * @return true if initialized, false on NULL, invalid private key, or disabled HMAC-DRBG.
 */
bool syn_p256_sign_deterministic_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                      const uint8_t hash[SYN_P256_BYTE_LEN]);

/**
 * @brief Cooperative child protothread for sliced P-256 ECDSA signing.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt     Child protothread control block.
 * @param ctx    Initialized P-256 signing context.
 * @param r_out  [out] 32-byte signature component r.
 * @param s_out  [out] 32-byte signature component s.
 * @param out_ok [out] Optional boolean receiving true on valid signature.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_p256_sign_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t r_out[SYN_P256_BYTE_LEN],
                               uint8_t s_out[SYN_P256_BYTE_LEN], bool *out_ok);

/**
 * @brief Initialize resumable P-256 ECDSA signature verification.
 * @param ctx   Caller-owned P-256 working context.
 * @param hash  32-byte message digest.
 * @param r     32-byte signature component r.
 * @param s     32-byte signature component s.
 * @param pub_x 32-byte public key X-coordinate.
 * @param pub_y 32-byte public key Y-coordinate.
 * @return true if parameters, point, and (r, s) bounds are valid and initialized; false otherwise.
 */
bool syn_p256_verify_init(SYN_P256_Ctx *ctx, const uint8_t hash[SYN_P256_BYTE_LEN],
                          const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                          const uint8_t pub_x[SYN_P256_BYTE_LEN],
                          const uint8_t pub_y[SYN_P256_BYTE_LEN]);

/**
 * @brief Cooperative child protothread for sliced P-256 ECDSA signature verification.
 *
 * Automatically wipes @p ctx via syn_secure_zero() before returning PT_ENDED.
 *
 * @param pt        Child protothread control block.
 * @param ctx       Initialized P-256 verify context.
 * @param out_valid [out] Receives true if signature is valid on PT_ENDED.
 * @return PT_MORE while in progress, PT_ENDED on completion, or PT_EXITED on invalid args.
 */
SYN_PT_Status syn_p256_verify_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, bool *out_valid);

#ifdef __cplusplus
}
#endif

#endif /* SYN_P256_ENABLED */

#endif /* SYN_P256_H */
