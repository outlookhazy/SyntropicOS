

# File syn\_p256.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_p256.h**](syn__p256_8h.md)

[Go to the documentation of this file](syn__p256_8h.md)


```C++


#ifndef SYN_P256_H
#define SYN_P256_H

#include "../common/syn_defs.h"

#if !defined(SYN_USE_P256) || SYN_USE_P256
#define SYN_P256_ENABLED 1
#else
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

#define SYN_P256_BYTE_LEN 32U

typedef struct {
    uint32_t x[8]; 
    uint32_t y[8]; 
    uint32_t z[8]; 
    bool infinity; 
} SYN_P256_Point;

typedef struct {
    SYN_P256_Point acc;      
    SYN_P256_Point table[3]; 
    uint32_t k1[8];          
    uint32_t k2[8];          
    uint32_t px[8];          
    uint32_t py[8];          
    uint32_t aux[8];         
    uint32_t inv_res[8];     
    uint32_t inv_base[8];    
#if SYN_HMAC_DRBG_ENABLED
    SYN_HMAC_DRBG drbg; 
#endif
    SYN_PT sub_pt;        
    uint16_t idx;         
    uint8_t op;           
    uint8_t attempts;     
    uint8_t max_attempts; 
    bool use_drbg;        
    bool ok;              
    bool active;          
} SYN_P256_Ctx;

bool syn_p256_base_mul(const uint8_t scalar[SYN_P256_BYTE_LEN], uint8_t pub_x[SYN_P256_BYTE_LEN],
                       uint8_t pub_y[SYN_P256_BYTE_LEN]);

bool syn_p256_point_mul(const uint8_t scalar[SYN_P256_BYTE_LEN],
                        const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN],
                        uint8_t rx[SYN_P256_BYTE_LEN], uint8_t ry[SYN_P256_BYTE_LEN]);

bool syn_p256_ecdh(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_x[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_y[SYN_P256_BYTE_LEN],
                   uint8_t shared_secret[SYN_P256_BYTE_LEN]);

bool syn_p256_sign_ecdsa(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                         const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                         const uint8_t hash[SYN_P256_BYTE_LEN], uint8_t r_out[SYN_P256_BYTE_LEN],
                         uint8_t s_out[SYN_P256_BYTE_LEN]);

bool syn_p256_sign_ecdsa_deterministic(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                       const uint8_t hash[SYN_P256_BYTE_LEN],
                                       uint8_t r_out[SYN_P256_BYTE_LEN],
                                       uint8_t s_out[SYN_P256_BYTE_LEN]);

bool syn_p256_verify_ecdsa(const uint8_t hash[SYN_P256_BYTE_LEN],
                           const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                           const uint8_t pub_x[SYN_P256_BYTE_LEN],
                           const uint8_t pub_y[SYN_P256_BYTE_LEN]);

bool syn_p256_is_on_curve(const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN]);

/* ── Cooperative Resumable API ──────────────────────────────────────────── */

void syn_p256_wipe(SYN_P256_Ctx *ctx);

bool syn_p256_base_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN]);

bool syn_p256_point_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN],
                             const uint8_t px[SYN_P256_BYTE_LEN],
                             const uint8_t py[SYN_P256_BYTE_LEN]);

SYN_PT_Status syn_p256_mul_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t rx[SYN_P256_BYTE_LEN],
                              uint8_t ry[SYN_P256_BYTE_LEN]);

bool syn_p256_sign_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                        const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                        const uint8_t hash[SYN_P256_BYTE_LEN]);

bool syn_p256_sign_deterministic_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                      const uint8_t hash[SYN_P256_BYTE_LEN]);

SYN_PT_Status syn_p256_sign_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t r_out[SYN_P256_BYTE_LEN],
                               uint8_t s_out[SYN_P256_BYTE_LEN], bool *out_ok);

bool syn_p256_verify_init(SYN_P256_Ctx *ctx, const uint8_t hash[SYN_P256_BYTE_LEN],
                          const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                          const uint8_t pub_x[SYN_P256_BYTE_LEN],
                          const uint8_t pub_y[SYN_P256_BYTE_LEN]);

SYN_PT_Status syn_p256_verify_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, bool *out_valid);

#ifdef __cplusplus
}
#endif

#endif /* SYN_P256_ENABLED */

#endif /* SYN_P256_H */
```


