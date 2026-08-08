

# File syn\_aes128.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_aes128.h**](syn__aes128_8h.md)

[Go to the documentation of this file](syn__aes128_8h.md)


```C++

#ifndef SYN_AES128_H
#define SYN_AES128_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_AES128_KEY_SIZE 16           
#define SYN_AES128_BLOCK_SIZE 16         
#define SYN_AES128_EXPANDED_KEY_SIZE 176 
typedef struct {
    uint8_t round_keys[SYN_AES128_EXPANDED_KEY_SIZE]; 
} SYN_AES128_Context;

SYN_Status syn_aes128_init(SYN_AES128_Context *ctx, const uint8_t key[SYN_AES128_KEY_SIZE]);

void syn_aes128_encrypt_block(const SYN_AES128_Context *ctx, const uint8_t in[16], uint8_t out[16]);

void syn_aes128_decrypt_block(const SYN_AES128_Context *ctx, const uint8_t in[16], uint8_t out[16]);

SYN_Status syn_aes128_cbc_encrypt(const SYN_AES128_Context *ctx, const uint8_t iv[16],
                                  const uint8_t *in, size_t in_len, uint8_t *out,
                                  size_t out_capacity, size_t *out_len);

SYN_Status syn_aes128_cbc_decrypt(const SYN_AES128_Context *ctx, const uint8_t iv[16],
                                  const uint8_t *in, size_t in_len, uint8_t *out,
                                  size_t out_capacity, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AES128_H */
```


