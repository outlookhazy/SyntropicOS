

# File syn\_aes.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes.h**](syn__aes_8h.md)

[Go to the documentation of this file](syn__aes_8h.md)


```C++

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
#define SYN_AES_MAX_KEY_BITS 256
#endif

#if (SYN_AES_MAX_KEY_BITS == 128)
#define SYN_AES_MAX_ROUNDS 10U
#define SYN_AES_MAX_EXPANDED_KEY 176U
#elif (SYN_AES_MAX_KEY_BITS == 192)
#define SYN_AES_MAX_ROUNDS 12U
#define SYN_AES_MAX_EXPANDED_KEY 208U
#elif (SYN_AES_MAX_KEY_BITS == 256)
#define SYN_AES_MAX_ROUNDS 14U
#define SYN_AES_MAX_EXPANDED_KEY 240U
#else
#error "SYN_AES_MAX_KEY_BITS must be 128, 192, or 256"
#endif

#ifndef SYN_USE_AES_DECRYPT
#define SYN_USE_AES_DECRYPT 1
#endif

#ifndef SYN_USE_AES_CBC
#define SYN_USE_AES_CBC 1
#endif

#ifndef SYN_USE_AES_CTR
#define SYN_USE_AES_CTR 1
#endif

#ifndef SYN_USE_AES_GCM
#define SYN_USE_AES_GCM 1
#endif

#ifndef SYN_AES_GCM_TABLE
#define SYN_AES_GCM_TABLE 0
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

#define SYN_AES_BLOCK_SIZE 16U

#define SYN_AES_KEY_SIZE_128 16U

#define SYN_AES_KEY_SIZE_192 24U

#define SYN_AES_KEY_SIZE_256 32U

#define SYN_AES_GCM_TAG_SIZE 16U

#define SYN_AES_GCM_IV_DEFAULT_SIZE 12U

/* ── Core AES Context ───────────────────────────────────────────────────── */

typedef struct {
    uint8_t round_keys[SYN_AES_MAX_EXPANDED_KEY]; 
    uint8_t nr;                                   
} SYN_AES_Context;

SYN_Status syn_aes_init(SYN_AES_Context *ctx, const uint8_t *key, size_t key_len);

void syn_aes_encrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE]);

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
void syn_aes_decrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE]);
#endif

#if !defined(SYN_USE_AES_CBC) || SYN_USE_AES_CBC
SYN_Status syn_aes_cbc_encrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len);

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
SYN_Status syn_aes_cbc_decrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len);
#endif
#endif

#if !defined(SYN_USE_AES_CTR) || SYN_USE_AES_CTR
SYN_Status syn_aes_ctr(const SYN_AES_Context *ctx, const uint8_t nonce[SYN_AES_BLOCK_SIZE],
                       const uint8_t *in, size_t len, uint8_t *out);
#endif

#if !defined(SYN_USE_AES_GCM) || SYN_USE_AES_GCM
typedef struct {
    SYN_AES_Context aes;           
    uint8_t h[SYN_AES_BLOCK_SIZE]; 
#if (SYN_AES_GCM_TABLE == 4)
    uint8_t htable[16][SYN_AES_BLOCK_SIZE]; 
#elif (SYN_AES_GCM_TABLE == 8)
    uint8_t htable[256][SYN_AES_BLOCK_SIZE]; 
#endif
} SYN_AES_GCM_Context;

SYN_Status syn_aes_gcm_init(SYN_AES_GCM_Context *ctx, const uint8_t *key, size_t key_len);

SYN_Status syn_aes_gcm_encrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               uint8_t tag[SYN_AES_GCM_TAG_SIZE]);

SYN_Status syn_aes_gcm_decrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               const uint8_t tag[SYN_AES_GCM_TAG_SIZE]);

void syn_aes_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16]);
#endif

#if !defined(SYN_USE_AES_CCM) || SYN_USE_AES_CCM
SYN_Status syn_aes_ccm_encrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               uint8_t *out, uint8_t *tag, size_t tag_len);

SYN_Status syn_aes_ccm_decrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               const uint8_t *tag, size_t tag_len, uint8_t *out);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_AES_H */
```


