

# File syn\_hkdf.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hkdf.h**](syn__hkdf_8h.md)

[Go to the documentation of this file](syn__hkdf_8h.md)


```C++

#ifndef SYN_HKDF_H
#define SYN_HKDF_H

#include "syn_hmac.h"
#include "syn_sha256.h"
#include "syn_sha512.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void syn_hkdf_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                      uint8_t prk_out[SYN_SHA256_DIGEST_SIZE]);

bool syn_hkdf_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info, size_t info_len,
                     uint8_t *okm_out, size_t okm_len);

bool syn_hkdf(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
              const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

bool syn_hkdf_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                           size_t label_len, const uint8_t *context, size_t context_len,
                           uint8_t *out, size_t out_len);

void syn_hkdf_sha384_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA384_DIGEST_SIZE]);

bool syn_hkdf_sha384_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len);

bool syn_hkdf_sha384(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

bool syn_hkdf_sha384_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len);

void syn_hkdf_sha512_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm,
                             size_t ikm_len, uint8_t prk_out[SYN_SHA512_DIGEST_SIZE]);

bool syn_hkdf_sha512_expand(const uint8_t *prk, size_t prk_len, const uint8_t *info,
                            size_t info_len, uint8_t *okm_out, size_t okm_len);

bool syn_hkdf_sha512(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                     const uint8_t *info, size_t info_len, uint8_t *okm_out, size_t okm_len);

bool syn_hkdf_sha512_expand_label(const uint8_t *secret, size_t secret_len, const char *label,
                                  size_t label_len, const uint8_t *context, size_t context_len,
                                  uint8_t *out, size_t out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HKDF_H */
```


