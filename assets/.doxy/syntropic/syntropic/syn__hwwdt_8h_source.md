

# File syn\_hwwdt.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_hwwdt.h**](syn__hwwdt_8h.md)

[Go to the documentation of this file](syn__hwwdt_8h.md)


```C++

#ifndef SYN_HWWDT_H
#define SYN_HWWDT_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_hwwdt_init(uint32_t timeout_ms);

void syn_hwwdt_feed(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HWWDT_H */
```


