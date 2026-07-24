

# File syn\_random.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_random.h**](syn__random_8h.md)

[Go to the documentation of this file](syn__random_8h.md)


```C++

#ifndef SYN_RANDOM_H
#define SYN_RANDOM_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_random_fill(void *buf, size_t len);

SYN_Status syn_random_fallback_fill(void *buf, size_t len);

uint32_t syn_random_u32(void);

uint32_t syn_random_range(uint32_t min, uint32_t max);

/* ── Porting Hook ───────────────────────────────────────────────────────── */

SYN_WEAK SYN_Status syn_port_random_fill(void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RANDOM_H */
```


