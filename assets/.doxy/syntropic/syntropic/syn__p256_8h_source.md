

# File syn\_p256.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_p256.h**](syn__p256_8h.md)

[Go to the documentation of this file](syn__p256_8h.md)


```C++

#ifndef SYN_P256_H
#define SYN_P256_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_P256_BYTE_LEN 32U

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

#ifdef __cplusplus
}
#endif

#endif /* SYN_P256_H */
```


