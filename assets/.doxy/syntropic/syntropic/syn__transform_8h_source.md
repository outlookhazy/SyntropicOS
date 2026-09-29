

# File syn\_transform.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_transform.h**](syn__transform_8h.md)

[Go to the documentation of this file](syn__transform_8h.md)


```C++

#ifndef SYN_TRANSFORM_H
#define SYN_TRANSFORM_H

#include "../common/syn_defs.h"
#include "syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

void syn_cart2pol(q16_t x, q16_t y, q16_t *r, q16_t *theta);

void syn_pol2cart(q16_t r, q16_t theta, q16_t *x, q16_t *y);

void syn_cart2sph(q16_t x, q16_t y, q16_t z, q16_t *r, q16_t *theta, q16_t *phi);

void syn_sph2cart(q16_t r, q16_t theta, q16_t phi, q16_t *x, q16_t *y, q16_t *z);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSFORM_H */
```


