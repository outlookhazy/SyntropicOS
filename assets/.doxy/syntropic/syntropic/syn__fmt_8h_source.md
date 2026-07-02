

# File syn\_fmt.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fmt.h**](syn__fmt_8h.md)

[Go to the documentation of this file](syn__fmt_8h.md)


```C++

#ifndef SYN_FMT_H
#define SYN_FMT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t syn_fmt_int(char *buf, size_t size, int32_t val);

size_t syn_fmt_uint(char *buf, size_t size, uint32_t val);

size_t syn_fmt_hex(char *buf, size_t size, uint32_t val, uint8_t min_digits);

size_t syn_fmt_q16(char *buf, size_t size, int32_t q16_val,
                    uint8_t frac_digits);

size_t syn_fmt_hexdump(char *buf, size_t size,
                        const uint8_t *data, size_t len);

size_t syn_fmt_fixed(char *buf, size_t size, int32_t val, uint8_t places);

size_t syn_fmt_concat(char *buf, size_t size,
                       const char *const *parts, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FMT_H */
```


