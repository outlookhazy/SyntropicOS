

# File syn\_ed25519.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.h**](syn__ed25519_8h.md)

[Go to the documentation of this file](syn__ed25519_8h.md)


```C++


#ifndef SYN_ED25519_H
#define SYN_ED25519_H

#include "../common/syn_defs.h"

#if (!defined(SYN_USE_ED25519) || SYN_USE_ED25519) && (!defined(SYN_USE_SHA512) || SYN_USE_SHA512)
#define SYN_ED25519_ENABLED 1
#else
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

#define SYN_ED25519_PUBLIC_KEY_SIZE 32U
#define SYN_ED25519_SECRET_KEY_SIZE 32U
#define SYN_ED25519_SEED_SIZE 32U
#define SYN_ED25519_SIGNATURE_SIZE 64U

typedef SYN_SHA512 SYN_SHA512_Ctx;

typedef struct {
    int32_t p[4][16]; 
    int32_t q[4][16]; 
    union {
        int32_t aux[4][16]; 
        SYN_SHA512 sha;     
    } u;                    
    const uint8_t *msg;     
    size_t msg_len;         
    size_t msg_off;         
    uint8_t d[64];          
    uint8_t scalar[32];     
    uint8_t pk[32];         
    uint8_t r_pt[32];       
    SYN_PT sub_pt;          
    SYN_PT sub2_pt;         
    uint16_t idx;           
    uint8_t op;             
    bool has_pk;            
    bool prehashed;         
    bool active;            
} SYN_Ed25519_Ctx;

bool syn_ed25519_publickey(const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

bool syn_ed25519_create_keypair(uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                                uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                                const uint8_t seed[SYN_ED25519_SEED_SIZE]);

bool syn_ed25519_sign(const uint8_t *msg, size_t msg_len,
                      const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                      const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                      uint8_t sig[SYN_ED25519_SIGNATURE_SIZE]);

bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

bool syn_ed25519_verify_hash(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t h[64],
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

/* ── Cooperative Resumable API ──────────────────────────────────────────── */

void syn_ed25519_wipe(SYN_Ed25519_Ctx *ctx);

bool syn_ed25519_publickey_init(SYN_Ed25519_Ctx *ctx,
                                const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE]);

SYN_PT_Status syn_ed25519_publickey_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                       uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

bool syn_ed25519_sign_init(SYN_Ed25519_Ctx *ctx, const uint8_t *msg, size_t msg_len,
                           const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

SYN_PT_Status syn_ed25519_sign_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                  uint8_t sig[SYN_ED25519_SIGNATURE_SIZE]);

bool syn_ed25519_verify_init(SYN_Ed25519_Ctx *ctx, const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                             const uint8_t *msg, size_t msg_len,
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

bool syn_ed25519_verify_hash_init(SYN_Ed25519_Ctx *ctx,
                                  const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                                  const uint8_t h[64],
                                  const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

SYN_PT_Status syn_ed25519_verify_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, bool *out_valid);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ED25519_ENABLED */

#endif /* SYN_ED25519_H */
```


