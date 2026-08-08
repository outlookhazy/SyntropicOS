

# File syn\_lz4.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lz4.h**](syn__lz4_8h.md)

[Go to the documentation of this file](syn__lz4_8h.md)


```C++

#ifndef SYN_LZ4_H
#define SYN_LZ4_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_LZ4_HASH_SIZE
#define SYN_LZ4_HASH_SIZE 4096U 
#endif

typedef struct {
    uint16_t hash_table[SYN_LZ4_HASH_SIZE]; 
} SYN_Lz4;

SYN_Status syn_lz4_init(SYN_Lz4 *ctx);

size_t syn_lz4_compress_bound(size_t input_size);

size_t syn_lz4_compress(SYN_Lz4 *ctx, const void *src, size_t src_size, void *dst,
                        size_t dst_capacity);

size_t syn_lz4_decompress(const void *src, size_t src_size, void *dst, size_t dst_capacity);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LZ4_H */
```


