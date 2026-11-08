/**
 * @file syn_hkdf.h
 * @brief HMAC-based Extract-and-Expand Key Derivation Function (HKDF-SHA256, RFC 5869) & TLS 1.3
 * HKDF-Expand-Label.
 * @ingroup syn_crypto
 */

#ifndef SYN_HKDF_H
#define SYN_HKDF_H

#include "../common/syn_defs.h"

#if (!defined(SYN_USE_HKDF) || SYN_USE_HKDF) && (!defined(SYN_USE_SHA256) || SYN_USE_SHA256)
/** @brief Non-zero when HKDF-SHA256 and SHA-256 dependencies are enabled. */
#define SYN_HKDF_ENABLED 1
#else
/** @brief Non-zero when HKDF-SHA256 and SHA-256 dependencies are enabled. */
#define SYN_HKDF_ENABLED 0
#endif

#if SYN_HKDF_ENABLED && (!defined(SYN_USE_SHA512) || SYN_USE_SHA512)
/** @brief Non-zero when HKDF-SHA384 and HKDF-SHA512 extensions are enabled. */
#define SYN_HKDF_SHA512_ENABLED 1
#else
/** @brief Non-zero when HKDF-SHA384 and HKDF-SHA512 extensions are enabled. */
#define SYN_HKDF_SHA512_ENABLED 0
#endif

#if SYN_HKDF_ENABLED

#include "syn_hmac.h"
#include "syn_sha256.h"
#if SYN_HKDF_SHA512_ENABLED
#include "syn_sha512.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HKDF-SHA256 Extract step (RFC 5869 Section 2.2).
 *
 * PRK = HMAC-Hash(salt, IKM)
 *
 * @param salt      Salt value (if NULL, a string of 32 zero bytes is used).
 * @param salt_len  Salt length in bytes.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length in bytes.
 * @param prk_out   [out] Output Pseudorandom Key buffer (must be at least 32 bytes).
 */
void syn_hkdf_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                      uint8_t prk_out[SYN_SHA256_DIGEST_SIZE]);

/**
 * @brief HKDF-SHA256 Expand step (RFC 5869 Section 2.3).
 *
 * OKM = HKDF-Expand(PRK, info, L)
 *
 * @param prk       Pseudorandom Key (at least 32 bytes).
 * @param prk_len   PRK length in bytes.
 * @param info      Optional context and application specific information.
 * @param info_len  Info length in bytes.
 * @param okm_out   [out] Output keying material buffer.
 * @param okm_len   Desired length of output keying material (max 255 * 32 = 8160 bytes).
 * @return true on success, false on invalid parameters.
 */
bool syn_hkdf_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info, size_t info_len,
                     uint8_t *okm_out, size_t okm_len);

/**
 * @brief Complete HKDF-SHA256 (Extract then Expand).
 *
 * @param salt      Salt value (optional).
 * @param salt_len  Salt length.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length.
 * @param info      Context info (optional).
 * @param info_len  Info length.
 * @param okm_out   [out] Output keying material.
 * @param okm_len   Desired output length.
 * @return true on success.
 */
bool syn_hkdf(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
              const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

/**
 * @brief TLS 1.3 HKDF-Expand-Label with SHA-256 (RFC 8446 Section 7.1).
 *
 * @param secret      Secret key (at least 32 bytes).
 * @param secret_len  Secret length.
 * @param label       Label string (e.g. "c hs traffic", "s hs traffic", "derived", etc.).
 * @param label_len   Label length (excluding null terminator).
 * @param context     Transcript hash context bytes (or NULL if 0-length).
 * @param context_len Context length (e.g. 32 bytes for SHA-256 transcript hash).
 * @param out         [out] Output key buffer.
 * @param out_len     Desired output key length.
 * @return true on success.
 */
bool syn_hkdf_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                           size_t label_len, const uint8_t *context, size_t context_len,
                           uint8_t *out, size_t out_len);

#if SYN_HKDF_SHA512_ENABLED

/**
 * @brief HKDF-SHA384 Extract step (RFC 5869 Section 2.2).
 * @param salt      Salt value (if NULL, a string of 48 zero bytes is used).
 * @param salt_len  Salt length in bytes.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length in bytes.
 * @param prk_out   [out] Output Pseudorandom Key buffer (must be at least 48 bytes).
 */
void syn_hkdf_sha384_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA384_DIGEST_SIZE]);

/**
 * @brief HKDF-SHA384 Expand step (RFC 5869 Section 2.3).
 * @param prk       Pseudorandom Key (at least 48 bytes).
 * @param prk_len   PRK length in bytes.
 * @param info      Optional context and application specific information.
 * @param info_len  Info length in bytes.
 * @param okm_out   [out] Output keying material buffer.
 * @param okm_len   Desired length of output keying material (max 255 * 48 = 12240 bytes).
 * @return true on success, false on invalid parameters.
 */
bool syn_hkdf_sha384_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len);

/**
 * @brief Complete HKDF-SHA384 (Extract then Expand).
 * @param salt      Salt value (optional).
 * @param salt_len  Salt length.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length.
 * @param info      Context info (optional).
 * @param info_len  Info length.
 * @param okm_out   [out] Output keying material.
 * @param okm_len   Desired output length.
 * @return true on success.
 */
bool syn_hkdf_sha384(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

/**
 * @brief TLS 1.3 HKDF-Expand-Label with SHA-384 (RFC 8446 Section 7.1).
 * @param secret      Secret key (at least 48 bytes).
 * @param secret_len  Secret length.
 * @param label       Label string.
 * @param label_len   Label length (excluding null terminator).
 * @param context     Transcript hash context bytes (or NULL if 0-length).
 * @param context_len Context length (e.g. 48 bytes for SHA-384 transcript hash).
 * @param out         [out] Output key buffer.
 * @param out_len     Desired output key length.
 * @return true on success.
 */
bool syn_hkdf_sha384_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len);

/**
 * @brief HKDF-SHA512 Extract step (RFC 5869 Section 2.2).
 * @param salt      Salt value (if NULL, a string of 64 zero bytes is used).
 * @param salt_len  Salt length in bytes.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length in bytes.
 * @param prk_out   [out] Output Pseudorandom Key buffer (must be at least 64 bytes).
 */
void syn_hkdf_sha512_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA512_DIGEST_SIZE]);

/**
 * @brief HKDF-SHA512 Expand step (RFC 5869 Section 2.3).
 * @param prk       Pseudorandom Key (at least 64 bytes).
 * @param prk_len   PRK length in bytes.
 * @param info      Optional context and application specific information.
 * @param info_len  Info length in bytes.
 * @param okm_out   [out] Output keying material buffer.
 * @param okm_len   Desired length of output keying material (max 255 * 64 = 16320 bytes).
 * @return true on success, false on invalid parameters.
 */
bool syn_hkdf_sha512_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len);

/**
 * @brief Complete HKDF-SHA512 (Extract then Expand).
 * @param salt      Salt value (optional).
 * @param salt_len  Salt length.
 * @param ikm       Input keying material.
 * @param ikm_len   IKM length.
 * @param info      Context info (optional).
 * @param info_len  Info length.
 * @param okm_out   [out] Output keying material.
 * @param okm_len   Desired output length.
 * @return true on success.
 */
bool syn_hkdf_sha512(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

/**
 * @brief TLS 1.3 HKDF-Expand-Label with SHA-512 (RFC 8446 Section 7.1).
 * @param secret      Secret key (at least 64 bytes).
 * @param secret_len  Secret length.
 * @param label       Label string.
 * @param label_len   Label length (excluding null terminator).
 * @param context     Transcript hash context bytes (or NULL if 0-length).
 * @param context_len Context length.
 * @param out         [out] Output key buffer.
 * @param out_len     Desired output key length.
 * @return true on success.
 */
bool syn_hkdf_sha512_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len);

#endif /* SYN_HKDF_SHA512_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* SYN_HKDF_ENABLED */

#endif /* SYN_HKDF_H */
