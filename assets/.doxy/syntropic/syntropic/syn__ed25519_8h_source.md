

# File syn\_ed25519.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.h**](syn__ed25519_8h.md)

[Go to the documentation of this file](syn__ed25519_8h.md)


```C++

#ifndef SYN_ED25519_H
#define SYN_ED25519_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_ED25519_PUBLIC_KEY_SIZE 32U
#define SYN_ED25519_SIGNATURE_SIZE 64U

bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ED25519_H */
```


