

# File syn\_chacha20poly1305.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_chacha20poly1305.h**](syn__chacha20poly1305_8h.md)

[Go to the documentation of this file](syn__chacha20poly1305_8h.md)


```C++

#ifndef SYN_CHACHA20POLY1305_H
#define SYN_CHACHA20POLY1305_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CHACHA20POLY1305) || SYN_USE_CHACHA20POLY1305

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── ChaCha20 ───────────────────────────────────────────────────────────── */

void syn_chacha20_xor(const uint8_t key[32],
                      const uint8_t nonce[12],
                      uint32_t counter,
                      const uint8_t *in, size_t len,
                      uint8_t *out);

void syn_chacha20_block(const uint8_t key[32],
                        const uint8_t nonce[12],
                        uint32_t counter,
                        uint8_t out[64]);

/* ── ChaCha20-Poly1305 AEAD ─────────────────────────────────────────────── */

void syn_aead_encrypt(const uint8_t key[32],
                      const uint8_t nonce[12],
                      const uint8_t *aad, size_t aad_len,
                      const uint8_t *plaintext, size_t pt_len,
                      uint8_t *ciphertext,
                      uint8_t tag[16]);

bool syn_aead_decrypt(const uint8_t key[32],
                      const uint8_t nonce[12],
                      const uint8_t *aad, size_t aad_len,
                      const uint8_t *ciphertext, size_t ct_len,
                      const uint8_t tag[16],
                      uint8_t *plaintext);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_CHACHA20POLY1305 */

#endif /* SYN_CHACHA20POLY1305_H */
```


