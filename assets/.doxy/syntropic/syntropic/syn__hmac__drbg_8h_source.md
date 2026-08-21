

# File syn\_hmac\_drbg.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hmac\_drbg.h**](syn__hmac__drbg_8h.md)

[Go to the documentation of this file](syn__hmac__drbg_8h.md)


```C++

#ifndef SYN_HMAC_DRBG_H
#define SYN_HMAC_DRBG_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HMAC_DRBG) || SYN_USE_HMAC_DRBG

#include "../common/syn_defs.h"
#include "syn_hmac.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_HMAC_DRBG_MIN_ENTROPY_LEN 32U

#define SYN_HMAC_DRBG_MAX_REQUEST_BYTES 65536U

#define SYN_HMAC_DRBG_DEFAULT_RESEED_INTERVAL 10000U

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    uint8_t k[SYN_HMAC_SHA256_SIZE]; 
    uint8_t v[SYN_HMAC_SHA256_SIZE]; 
    uint32_t reseed_counter;         
    uint32_t reseed_interval;        
    bool initialized;                
} SYN_HMAC_DRBG;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_hmac_drbg_init(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                              const uint8_t *nonce, size_t nonce_len, const uint8_t *pers_str,
                              size_t pers_len);

SYN_Status syn_hmac_drbg_reseed(SYN_HMAC_DRBG *ctx, const uint8_t *entropy, size_t ent_len,
                                const uint8_t *add_input, size_t add_len);

SYN_Status syn_hmac_drbg_generate(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                  const uint8_t *add_input, size_t add_len);

SYN_Status syn_hmac_drbg_generate_pr(SYN_HMAC_DRBG *ctx, uint8_t *out, size_t out_len,
                                     const uint8_t *entropy, size_t ent_len,
                                     const uint8_t *add_input, size_t add_len);

void syn_hmac_drbg_set_reseed_interval(SYN_HMAC_DRBG *ctx, uint32_t interval);

void syn_hmac_drbg_wipe(SYN_HMAC_DRBG *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_HMAC_DRBG */

#endif /* SYN_HMAC_DRBG_H */
```


