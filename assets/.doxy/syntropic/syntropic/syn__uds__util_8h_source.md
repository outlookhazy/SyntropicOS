

# File syn\_uds\_util.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds\_util.h**](syn__uds__util_8h.md)

[Go to the documentation of this file](syn__uds__util_8h.md)


```C++

#ifndef SYN_UDS_UTIL_H
#define SYN_UDS_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool syn_uds_dtc_from_str(const char *str, uint32_t *dtc_out);

bool syn_uds_dtc_to_str(uint32_t dtc, char *out_buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDS_UTIL_H */
```


