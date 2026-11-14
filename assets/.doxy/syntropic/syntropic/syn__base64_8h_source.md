

# File syn\_base64.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_base64.h**](syn__base64_8h.md)

[Go to the documentation of this file](syn__base64_8h.md)


```C++


#ifndef SYN_BASE64_H
#define SYN_BASE64_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BASE64) || SYN_USE_BASE64

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BASE64_MAX_RAW_LEN (((SIZE_MAX - 4U) / 4U) * 3U)

static inline size_t syn_base64_calc_encoded_size(size_t raw_len)
{
    if (raw_len > SYN_BASE64_MAX_RAW_LEN) {
        return 0U;
    }
    return ((raw_len / 3U) * 4U) + (((raw_len % 3U) != 0U) ? 4U : 0U) + 1U;
}

static inline size_t syn_base64_calc_max_decoded_size(size_t b64_len)
{
    if (b64_len > (SIZE_MAX - 3U)) {
        return 0U;
    }
    return ((b64_len / 4U) * 3U) + (((b64_len % 4U) != 0U) ? 3U : 0U);
}

bool syn_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                       size_t *out_len);

bool syn_base64_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                       size_t *out_len);

bool syn_base64url_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                          bool with_padding, size_t *out_len);

bool syn_base64url_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                          size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_BASE64 */

#endif /* SYN_BASE64_H */
```


