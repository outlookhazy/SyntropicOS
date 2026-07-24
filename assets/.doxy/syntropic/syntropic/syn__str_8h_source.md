

# File syn\_str.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_str.h**](syn__str_8h.md)

[Go to the documentation of this file](syn__str_8h.md)


```C++

#ifndef SYN_STR_H
#define SYN_STR_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t syn_str_split(char *str, char delimiter, char *tokens[], size_t max_tokens);

char *syn_str_trim(char *str);

bool syn_str_to_i32(const char *str, int32_t *out_val);

bool syn_str_to_u32(const char *str, uint32_t *out_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_STR_H */
```


