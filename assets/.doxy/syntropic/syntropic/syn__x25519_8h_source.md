

# File syn\_x25519.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x25519.h**](syn__x25519_8h.md)

[Go to the documentation of this file](syn__x25519_8h.md)


```C++


#ifndef SYN_X25519_H
#define SYN_X25519_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_X25519) || SYN_USE_X25519

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t z[32]; 
    int32_t x[16]; 
    int32_t a[16]; 
    int32_t b[16]; 
    int32_t c[16]; 
    int32_t d[16]; 
    SYN_PT sub_pt; 
    uint16_t idx;  
    bool active;   
} SYN_X25519_Ctx;

void syn_x25519(uint8_t shared_out[32], const uint8_t scalar[32], const uint8_t point[32]);

void syn_x25519_pubkey(uint8_t public_out[32], const uint8_t private_key[32]);

static inline void syn_x25519_clamp(uint8_t key[32])
{
    key[0] &= 248U;  /* Clear low 3 bits (multiple of 8) */
    key[31] &= 127U; /* Clear high bit */
    key[31] |= 64U;  /* Set second-highest bit */
}

static inline bool syn_x25519_is_nonzero(const uint8_t secret[32])
{
    uint8_t acc = 0U;
    for (size_t i = 0U; i < 32U; i++) {
        acc |= secret[i];
    }
    return acc != 0U;
}

bool syn_x25519_init(SYN_X25519_Ctx *ctx, const uint8_t scalar[32], const uint8_t point[32]);

bool syn_x25519_pubkey_init(SYN_X25519_Ctx *ctx, const uint8_t private_key[32]);

void syn_x25519_wipe(SYN_X25519_Ctx *ctx);

SYN_PT_Status syn_x25519_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx, uint8_t shared_out[32]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_X25519 */

#endif /* SYN_X25519_H */
```


