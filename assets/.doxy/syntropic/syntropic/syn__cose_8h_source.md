

# File syn\_cose.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cose.h**](syn__cose_8h.md)

[Go to the documentation of this file](syn__cose_8h.md)


```C++

#ifndef SYN_COSE_H
#define SYN_COSE_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_COSE) || SYN_USE_COSE

#include "syntropic/common/syn_defs.h"
#include "syntropic/crypto/syn_aes.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_hmac.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/util/syn_cbor_read.h"
#include "syntropic/util/syn_cbor_write.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── COSE Header Parameter Labels (RFC 9052 Section 3.1) ─────────────────── */

#define SYN_COSE_HEADER_ALG 1
#define SYN_COSE_HEADER_CRIT 2
#define SYN_COSE_HEADER_CONTENT_TYPE 3
#define SYN_COSE_HEADER_KID 4
#define SYN_COSE_HEADER_IV 5
#define SYN_COSE_HEADER_PARTIAL_IV 6

/* ── COSE Key Parameter Labels (RFC 9052 Section 7 / RFC 9053 Section 7) ─── */

#define SYN_COSE_KEY_KTY 1
#define SYN_COSE_KEY_KID 2
#define SYN_COSE_KEY_ALG 3
#define SYN_COSE_KEY_CRV (-1)
#define SYN_COSE_KEY_X (-2)
#define SYN_COSE_KEY_Y (-3)
#define SYN_COSE_KEY_D (-4)
#define SYN_COSE_KEY_K (-1)

/* ── COSE Key Types & Curves ────────────────────────────────────────────── */

typedef enum {
    SYN_COSE_KTY_RESERVED = 0,
    SYN_COSE_KTY_OKP = 1,      
    SYN_COSE_KTY_EC2 = 2,      
    SYN_COSE_KTY_SYMMETRIC = 4 
} SYN_COSE_KeyType;

typedef enum {
    SYN_COSE_CRV_RESERVED = 0,
    SYN_COSE_CRV_P256 = 1,   
    SYN_COSE_CRV_ED25519 = 6 
} SYN_COSE_Curve;

/* ── COSE Algorithm Identifiers (RFC 9053 / IANA COSE Algorithms) ────────── */

typedef enum {
    SYN_COSE_ALGO_UNKNOWN = 0,
    SYN_COSE_ALGO_A128GCM = 1,      
    SYN_COSE_ALGO_A192GCM = 2,      
    SYN_COSE_ALGO_A256GCM = 3,      
    SYN_COSE_ALGO_HMAC_256_64 = 4,  
    SYN_COSE_ALGO_HMAC_256_256 = 5, 
    SYN_COSE_ALGO_AES_CCM_16_64_128 =
        10, 
    SYN_COSE_ALGO_AES_CCM_16_64_256 =
        11, 
    SYN_COSE_ALGO_CHACHA20_POLY1305 = 24, 
    SYN_COSE_ALGO_AES_CCM_16_128_128 =
        30, 
    SYN_COSE_ALGO_AES_CCM_16_128_256 =
        31,                   
    SYN_COSE_ALGO_ES256 = -7, 
    SYN_COSE_ALGO_EDDSA = -8  
} SYN_COSE_Algorithm;

/* ── Constants ──────────────────────────────────────────────────────────── */

#define SYN_COSE_MAX_KID_LEN 32U
#define SYN_COSE_MAX_IV_LEN 16U
#define SYN_COSE_MAX_SIG_LEN 64U
#define SYN_COSE_MAX_TAG_LEN 32U
#define SYN_COSE_MAX_PROTECTED_LEN 64U

/* ── COSE Parsed Message & Key Structures ────────────────────────────────── */

typedef struct {
    SYN_COSE_KeyType kty;              
    SYN_COSE_Algorithm alg;            
    SYN_COSE_Curve crv;                
    uint8_t kid[SYN_COSE_MAX_KID_LEN]; 
    size_t kid_len;                    
    uint8_t pub_x[32];                 
    size_t pub_x_len;                  
    uint8_t pub_y[32];                 
    size_t pub_y_len;                  
    uint8_t priv_d[32];                
    size_t priv_d_len;                 
} SYN_COSE_Key;

typedef struct {
    SYN_COSE_Algorithm alg;                            
    uint8_t kid[SYN_COSE_MAX_KID_LEN];                 
    size_t kid_len;                                    
    const uint8_t *payload;                            
    size_t payload_len;                                
    const uint8_t *signature;                          
    size_t signature_len;                              
    uint8_t protected_hdr[SYN_COSE_MAX_PROTECTED_LEN]; 
    size_t protected_hdr_len;                          
} SYN_COSE_Sign1Message;

typedef struct {
    SYN_COSE_Algorithm alg;                            
    uint8_t kid[SYN_COSE_MAX_KID_LEN];                 
    size_t kid_len;                                    
    uint8_t iv[SYN_COSE_MAX_IV_LEN];                   
    size_t iv_len;                                     
    const uint8_t *ciphertext;                         
    size_t ciphertext_len;                             
    uint8_t protected_hdr[SYN_COSE_MAX_PROTECTED_LEN]; 
    size_t protected_hdr_len;                          
} SYN_COSE_Encrypt0Message;

typedef struct {
    SYN_COSE_Algorithm alg;                            
    uint8_t kid[SYN_COSE_MAX_KID_LEN];                 
    size_t kid_len;                                    
    const uint8_t *payload;                            
    size_t payload_len;                                
    const uint8_t *tag;                                
    size_t tag_len;                                    
    uint8_t protected_hdr[SYN_COSE_MAX_PROTECTED_LEN]; 
    size_t protected_hdr_len;                          
} SYN_COSE_Mac0Message;

/* ── COSE_Sign1 API ──────────────────────────────────────────────────────── */

SYN_Status syn_cose_sign1_create(SYN_COSE_Algorithm alg, const uint8_t *secret_key,
                                 const uint8_t *public_key, const uint8_t *kid, size_t kid_len,
                                 const uint8_t *payload, size_t payload_len,
                                 const uint8_t *external_aad, size_t external_aad_len,
                                 uint8_t *out_buf, size_t out_buf_size, size_t *out_len);

SYN_Status syn_cose_sign1_verify(const uint8_t *msg, size_t msg_len, const uint8_t *public_key,
                                 size_t public_key_len, const uint8_t *external_aad,
                                 size_t external_aad_len, SYN_COSE_Sign1Message *parsed_out);

/* ── COSE_Encrypt0 API ───────────────────────────────────────────────────── */

SYN_Status syn_cose_encrypt0_create(SYN_COSE_Algorithm alg, const uint8_t *key, const uint8_t *iv,
                                    size_t iv_len, const uint8_t *kid, size_t kid_len,
                                    const uint8_t *plaintext, size_t plaintext_len,
                                    const uint8_t *external_aad, size_t external_aad_len,
                                    uint8_t *out_buf, size_t out_buf_size, size_t *out_len);

SYN_Status syn_cose_encrypt0_decrypt(const uint8_t *msg, size_t msg_len, const uint8_t *key,
                                     const uint8_t *external_aad, size_t external_aad_len,
                                     uint8_t *out_plaintext, size_t out_plaintext_size,
                                     size_t *out_plaintext_len,
                                     SYN_COSE_Encrypt0Message *parsed_out);

/* ── COSE_Mac0 API ───────────────────────────────────────────────────────── */

SYN_Status syn_cose_mac0_create(SYN_COSE_Algorithm alg, const uint8_t *key, size_t key_len,
                                const uint8_t *kid, size_t kid_len, const uint8_t *payload,
                                size_t payload_len, const uint8_t *external_aad,
                                size_t external_aad_len, uint8_t *out_buf, size_t out_buf_size,
                                size_t *out_len);

SYN_Status syn_cose_mac0_verify(const uint8_t *msg, size_t msg_len, const uint8_t *key,
                                size_t key_len, const uint8_t *external_aad,
                                size_t external_aad_len, SYN_COSE_Mac0Message *parsed_out);

/* ── COSE_Key API ────────────────────────────────────────────────────────── */

SYN_Status syn_cose_key_encode(const SYN_COSE_Key *key, uint8_t *out_buf, size_t out_buf_size,
                               size_t *out_len);

SYN_Status syn_cose_key_decode(const uint8_t *buf, size_t buf_len, SYN_COSE_Key *key_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_COSE */

#endif /* SYN_COSE_H */
```


