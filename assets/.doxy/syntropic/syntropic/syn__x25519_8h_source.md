

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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void syn_x25519(uint8_t shared_out[32],
                const uint8_t scalar[32],
                const uint8_t point[32]);

void syn_x25519_pubkey(uint8_t public_out[32],
                       const uint8_t private_key[32]);

static inline void syn_x25519_clamp(uint8_t key[32])
{
    key[0]  &= 248;   /* Clear low 3 bits (multiple of 8) */
    key[31] &= 127;   /* Clear high bit */
    key[31] |= 64;    /* Set second-highest bit */
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_X25519 */

#endif /* SYN_X25519_H */
```


