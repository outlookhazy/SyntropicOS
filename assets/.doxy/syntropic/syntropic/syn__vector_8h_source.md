

# File syn\_vector.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_vector.h**](syn__vector_8h.md)

[Go to the documentation of this file](syn__vector_8h.md)


```C++

#ifndef SYN_VECTOR_H
#define SYN_VECTOR_H

#include "../common/syn_defs.h"
#include "syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void syn_vec_add(const q16_t *a, const q16_t *b, q16_t *out, uint16_t n);

void syn_vec_sub(const q16_t *a, const q16_t *b, q16_t *out, uint16_t n);

void syn_vec_scale(const q16_t *v, q16_t scale, q16_t *out, uint16_t n);

void syn_vec_clamp(const q16_t *v, q16_t min_val, q16_t max_val, q16_t *out, uint16_t n);

q16_t syn_vec_min(const q16_t *v, uint16_t n);

q16_t syn_vec_max(const q16_t *v, uint16_t n);

q16_t syn_vec_mean(const q16_t *v, uint16_t n);

q16_t syn_vec_variance(const q16_t *v, uint16_t n);

q16_t syn_vec_rms(const q16_t *v, uint16_t n);

#ifdef __cplusplus
}
#endif

#endif /* SYN_VECTOR_H */
```


