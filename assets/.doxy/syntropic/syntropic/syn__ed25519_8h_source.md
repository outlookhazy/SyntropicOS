

# File syn\_ed25519.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.h**](syn__ed25519_8h.md)

[Go to the documentation of this file](syn__ed25519_8h.md)


```C++

#ifndef SYN_ED25519_H
#define SYN_ED25519_H

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

#ifdef __cplusplus
}
#endif

#endif /* SYN_ED25519_H */
```


