

# File syn\_quaternion.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_quaternion.h**](syn__quaternion_8h.md)

[Go to the documentation of this file](syn__quaternion_8h.md)


```C++

#ifndef SYN_QUATERNION_H
#define SYN_QUATERNION_H

#include "../common/syn_defs.h"
#include "syn_matrix.h"
#include "syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    q16_t w; 
    q16_t x; 
    q16_t y; 
    q16_t z; 
} SYN_Quaternion;

void syn_quat_identity(SYN_Quaternion *q);

void syn_quat_init(SYN_Quaternion *q, q16_t w, q16_t x, q16_t y, q16_t z);

void syn_quat_mul(const SYN_Quaternion *q1, const SYN_Quaternion *q2, SYN_Quaternion *out);

q16_t syn_quat_norm(const SYN_Quaternion *q);

SYN_Status syn_quat_normalize(SYN_Quaternion *q);

void syn_quat_conjugate(const SYN_Quaternion *q, SYN_Quaternion *out);

SYN_Status syn_quat_inverse(const SYN_Quaternion *q, SYN_Quaternion *out);

void syn_quat_rotate_vec3(const SYN_Quaternion *q, const q16_t *v, q16_t *out);

void syn_quat_to_mat3x3(const SYN_Quaternion *q, SYN_Matrix *out);

void syn_quat_from_euler(SYN_Quaternion *q, q16_t roll, q16_t pitch, q16_t yaw);

void syn_quat_to_euler(const SYN_Quaternion *q, q16_t *roll, q16_t *pitch, q16_t *yaw);

void syn_quat_slerp(const SYN_Quaternion *q1, const SYN_Quaternion *q2, q16_t t,
                    SYN_Quaternion *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_QUATERNION_H */
```


