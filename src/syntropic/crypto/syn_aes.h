/**
 * @file syn_aes.h
 * @brief Unified AES cipher engine (128/192/256-bit keys, ECB, CBC, CTR, and GCM AEAD).
 *
 * Zero-heap implementation supporting AES-128, AES-192, and AES-256 with configurable
 * compile-time memory footprint guardrails. Software rounds use 256-byte S-box tables
 * (constant-time on single-cycle SRAM/Flash MCUs without data caches; enable
 * SYN_USE_PORT_AES hardware acceleration on cores with D-cache).
 * @ingroup syn_crypto
 */

#ifndef SYN_AES_H
#define SYN_AES_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Compile-time configuration knobs ───────────────────────────────────── */

#ifndef SYN_AES_MAX_KEY_BITS
/** @brief Maximum key size in bits (128, 192, or 256). Controls context RAM footprint. */
#define SYN_AES_MAX_KEY_BITS 256
#endif

#if (SYN_AES_MAX_KEY_BITS == 128)
/** @brief Maximum rounds for configured key size. */
#define SYN_AES_MAX_ROUNDS 10U
/** @brief Maximum expanded round keys storage size in bytes. */
#define SYN_AES_MAX_EXPANDED_KEY 176U
#elif (SYN_AES_MAX_KEY_BITS == 192)
/** @brief Maximum rounds for configured key size. */
#define SYN_AES_MAX_ROUNDS 12U
/** @brief Maximum expanded round keys storage size in bytes. */
#define SYN_AES_MAX_EXPANDED_KEY 208U
#elif (SYN_AES_MAX_KEY_BITS == 256)
/** @brief Maximum rounds for configured key size. */
#define SYN_AES_MAX_ROUNDS 14U
/** @brief Maximum expanded round keys storage size in bytes. */
#define SYN_AES_MAX_EXPANDED_KEY 240U
#else
#error "SYN_AES_MAX_KEY_BITS must be 128, 192, or 256"
#endif

#ifndef SYN_USE_AES_DECRYPT
/** @brief Enable ECB and CBC decryption functions (pulls rsbox, +256B Flash). */
#define SYN_USE_AES_DECRYPT 1
#endif

#ifndef SYN_USE_AES_CBC
/** @brief Enable CBC mode encryption and decryption with PKCS#7 padding. */
#define SYN_USE_AES_CBC 1
#endif

#ifndef SYN_USE_AES_CTR
/** @brief Enable CTR stream cipher mode. */
#define SYN_USE_AES_CTR 1
#endif

#ifndef SYN_USE_AES_GCM
/** @brief Enable GCM AEAD authenticated encryption and decryption. */
#define SYN_USE_AES_GCM 1
#endif

#ifndef SYN_AES_GCM_TABLE
/**
 * @brief GCM GHASH acceleration table reservation knob.
 * 0 = bit-by-bit GHASH (0 bytes table, default),
 * 4 = reserve 4-bit nibble table (256 bytes per GCM context; unused by software GHASH),
 * 8 = reserve 8-bit byte table (4096 bytes per GCM context; unused by software GHASH).
 */
#define SYN_AES_GCM_TABLE 0
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

/** @brief AES cipher block size in bytes (16 bytes / 128 bits). */
#define SYN_AES_BLOCK_SIZE 16U

/** @brief AES-128 key size in bytes (16 bytes / 128 bits). */
#define SYN_AES_KEY_SIZE_128 16U

/** @brief AES-192 key size in bytes (24 bytes / 192 bits). */
#define SYN_AES_KEY_SIZE_192 24U

/** @brief AES-256 key size in bytes (32 bytes / 256 bits). */
#define SYN_AES_KEY_SIZE_256 32U

/** @brief Standard GCM authentication tag size in bytes (16 bytes / 128 bits). */
#define SYN_AES_GCM_TAG_SIZE 16U

/** @brief Recommended GCM initialization vector size in bytes (12 bytes / 96 bits). */
#define SYN_AES_GCM_IV_DEFAULT_SIZE 12U

/* ── Core AES Context ───────────────────────────────────────────────────── */

/**
 * @brief AES context containing expanded round keys.
 */
typedef struct {
    uint8_t round_keys[SYN_AES_MAX_EXPANDED_KEY]; /**< Expanded round key schedule */
    uint8_t nr;                                   /**< Number of rounds (10, 12, or 14) */
} SYN_AES_Context;

/**
 * @brief Initialize AES context and perform key expansion for 128/192/256-bit key.
 *
 * @param[out] ctx      Pointer to AES context to initialize.
 * @param[in]  key      Secret key buffer.
 * @param[in]  key_len  Length of secret key in bytes (16, 24, or 32).
 * @return SYN_OK on success, or SYN_INVALID_PARAM on invalid key size or NULL pointers.
 */
SYN_Status syn_aes_init(SYN_AES_Context *ctx, const uint8_t *key, size_t key_len);

/**
 * @brief Encrypt a single 16-byte block (ECB mode).
 *
 * @param[in]  ctx Initialized AES context.
 * @param[in]  in  16-byte plaintext input block.
 * @param[out] out 16-byte ciphertext output block.
 */
void syn_aes_encrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE]);

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
/**
 * @brief Decrypt a single 16-byte block (ECB mode).
 *
 * @param[in]  ctx Initialized AES context.
 * @param[in]  in  16-byte ciphertext input block.
 * @param[out] out 16-byte plaintext output block.
 */
void syn_aes_decrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE]);
#endif

#if !defined(SYN_USE_AES_CBC) || SYN_USE_AES_CBC
/**
 * @brief Encrypt data using AES-CBC with PKCS#7 padding.
 *
 * @param[in]  ctx          Initialized AES context.
 * @param[in]  iv           16-byte initialization vector.
 * @param[in]  in           Plaintext buffer (may be NULL if in_len is 0).
 * @param[in]  in_len       Plaintext length in bytes.
 * @param[out] out          Ciphertext output buffer.
 * @param[in]  out_capacity Maximum capacity of output buffer (must be >= in_len + PKCS#7 pad).
 * @param[out] out_len      Number of ciphertext bytes written.
 * @return SYN_OK on success, or SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_aes_cbc_encrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len);

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
/**
 * @brief Decrypt data using AES-CBC with PKCS#7 unpadding.
 *
 * @param[in]  ctx          Initialized AES context.
 * @param[in]  iv           16-byte initialization vector.
 * @param[in]  in           Ciphertext buffer (must be non-empty multiple of 16 bytes).
 * @param[in]  in_len       Ciphertext length in bytes.
 * @param[out] out          Plaintext output buffer.
 * @param[in]  out_capacity Capacity of plaintext output buffer.
 * @param[out] out_len      Number of plaintext bytes written.
 * @return SYN_OK on success, or SYN_INVALID_PARAM on invalid padding or parameters.
 */
SYN_Status syn_aes_cbc_decrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len);
#endif
#endif

#if !defined(SYN_USE_AES_CTR) || SYN_USE_AES_CTR
/**
 * @brief Encrypt/decrypt arbitrary length data using AES-CTR stream mode (NIST SP 800-38A).
 *
 * @param[in]  ctx   Initialized AES context.
 * @param[in]  nonce 16-byte initial counter block (incremented as 128-bit big-endian).
 * @param[in]  in    Input data buffer (plaintext for encrypt, ciphertext for decrypt).
 * @param[in]  len   Length of input and output data in bytes.
 * @param[out] out   Output data buffer.
 * @return SYN_OK on success, or SYN_INVALID_PARAM on NULL pointers.
 */
SYN_Status syn_aes_ctr(const SYN_AES_Context *ctx, const uint8_t nonce[SYN_AES_BLOCK_SIZE],
                       const uint8_t *in, size_t len, uint8_t *out);
#endif

#if !defined(SYN_USE_AES_GCM) || SYN_USE_AES_GCM
/**
 * @brief AES-GCM AEAD context — wraps AES key schedule and GHASH subkey/tables.
 */
typedef struct {
    SYN_AES_Context aes;           /**< Base AES cipher context */
    uint8_t h[SYN_AES_BLOCK_SIZE]; /**< GHASH subkey H = AES(K, 0) */
#if (SYN_AES_GCM_TABLE == 4)
    uint8_t htable[16][SYN_AES_BLOCK_SIZE]; /**< Reserved 4-bit GHASH table (unused in SW) */
#elif (SYN_AES_GCM_TABLE == 8)
    uint8_t htable[256][SYN_AES_BLOCK_SIZE]; /**< Reserved 8-bit GHASH table (unused in SW) */
#endif
} SYN_AES_GCM_Context;

/**
 * @brief Initialize AES-GCM AEAD context (expands key and computes GHASH subkey H).
 *
 * @param[out] ctx      Pointer to GCM context to initialize.
 * @param[in]  key      Secret key buffer.
 * @param[in]  key_len  Length of secret key in bytes (16, 24, or 32).
 * @return SYN_OK on success, or SYN_INVALID_PARAM on failure.
 */
SYN_Status syn_aes_gcm_init(SYN_AES_GCM_Context *ctx, const uint8_t *key, size_t key_len);

/**
 * @brief AES-GCM authenticated encryption (NIST SP 800-38D).
 *
 * @param[in]  ctx       Initialized AES-GCM context.
 * @param[in]  nonce     Initialization vector / nonce buffer.
 * @param[in]  nonce_len Nonce length in bytes (standard is 12 bytes).
 * @param[in]  aad       Additional authenticated data (may be NULL if aad_len is 0).
 * @param[in]  aad_len   AAD length in bytes.
 * @param[in]  in        Plaintext buffer to encrypt (may be NULL if in_len is 0).
 * @param[in]  in_len    Plaintext length in bytes.
 * @param[out] out       Ciphertext output buffer (must be at least in_len bytes).
 * @param[out] tag       16-byte authentication tag output buffer.
 * @return SYN_OK on success, or SYN_INVALID_PARAM on NULL pointers or invalid params.
 */
SYN_Status syn_aes_gcm_encrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               uint8_t tag[SYN_AES_GCM_TAG_SIZE]);

/**
 * @brief AES-GCM authenticated decryption and tag verification (NIST SP 800-38D).
 *
 * Decrypts ciphertext and verifies the authentication tag in constant time.
 * If authentication fails, plaintext output is zeroed (when out != in) and SYN_ERROR is returned.
 *
 * @param[in]  ctx       Initialized AES-GCM context.
 * @param[in]  nonce     Initialization vector / nonce buffer.
 * @param[in]  nonce_len Nonce length in bytes.
 * @param[in]  aad       Additional authenticated data (may be NULL if aad_len is 0).
 * @param[in]  aad_len   AAD length in bytes.
 * @param[in]  in        Ciphertext buffer to decrypt (may be NULL if in_len is 0).
 * @param[in]  in_len    Ciphertext length in bytes.
 * @param[out] out       Plaintext output buffer (must be at least in_len bytes).
 * @param[in]  tag       16-byte expected authentication tag to verify against.
 * @return SYN_OK on successful verification, SYN_ERROR on tag mismatch,
 *         or SYN_INVALID_PARAM on invalid parameters.
 */
SYN_Status syn_aes_gcm_decrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               const uint8_t tag[SYN_AES_GCM_TAG_SIZE]);

/**
 * @brief Multiply 16-byte block by GHASH subkey H in GF(2^128).
 *
 * @param[in]  x   Input 16-byte field element.
 * @param[in]  h   Input 16-byte GHASH subkey H.
 * @param[out] out Output 16-byte product block.
 */
void syn_aes_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16]);
#endif

#if !defined(SYN_USE_AES_CCM) || SYN_USE_AES_CCM
/**
 * @brief AES-CCM authenticated encryption (NIST SP 800-38C / RFC 3610).
 *
 * Computes CBC-MAC authentication tag and encrypts payload in CTR mode.
 *
 * @param[in]  ctx       Initialized AES context.
 * @param[in]  nonce     Nonce buffer (length must be 7..13 bytes).
 * @param[in]  nonce_len Nonce length in bytes (7 to 13).
 * @param[in]  aad       Additional authenticated data (may be NULL if aad_len is 0).
 * @param[in]  aad_len   AAD length in bytes.
 * @param[in]  in        Plaintext buffer to encrypt (may be NULL if in_len is 0).
 * @param[in]  in_len    Plaintext length in bytes.
 * @param[out] out       Ciphertext output buffer (must be at least in_len bytes).
 * @param[out] tag       Authentication tag output buffer.
 * @param[in]  tag_len   Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16).
 * @return SYN_OK on success, or SYN_INVALID_PARAM on invalid parameters.
 */
SYN_Status syn_aes_ccm_encrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               uint8_t *out, uint8_t *tag, size_t tag_len);

/**
 * @brief AES-CCM authenticated decryption and tag verification (NIST SP 800-38C / RFC 3610).
 *
 * Decrypts ciphertext in CTR mode and verifies CBC-MAC authentication tag in constant time.
 * If verification fails, plaintext buffer is zeroed (if out != in) and SYN_ERROR is returned.
 *
 * @param[in]  ctx       Initialized AES context.
 * @param[in]  nonce     Nonce buffer (length must be 7..13 bytes).
 * @param[in]  nonce_len Nonce length in bytes (7 to 13).
 * @param[in]  aad       Additional authenticated data (may be NULL if aad_len is 0).
 * @param[in]  aad_len   AAD length in bytes.
 * @param[in]  in        Ciphertext buffer to decrypt (may be NULL if in_len is 0).
 * @param[in]  in_len    Ciphertext length in bytes.
 * @param[in]  tag       Authentication tag to verify against.
 * @param[in]  tag_len   Length of authentication tag in bytes (4, 6, 8, 10, 12, 14, or 16).
 * @param[out] out       Plaintext output buffer (must be at least in_len bytes).
 * @return SYN_OK on success, SYN_ERROR on tag mismatch, or SYN_INVALID_PARAM on invalid params.
 */
SYN_Status syn_aes_ccm_decrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               const uint8_t *tag, size_t tag_len, uint8_t *out);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_AES_H */
