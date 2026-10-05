

# File syn\_transform.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_transform.c**](syn__transform_8c.md)

[Go to the documentation of this file](syn__transform_8c.md)


```C++

#include "syn_transform.h"

#include "syn_assert.h"

void syn_cart2pol(q16_t x, q16_t y, q16_t *r, q16_t *theta)
{
    SYN_ASSERT(r != NULL && theta != NULL);
    *r = q16_hypot(x, y);
    *theta = q16_atan2(y, x);
}

void syn_pol2cart(q16_t r, q16_t theta, q16_t *x, q16_t *y)
{
    SYN_ASSERT(x != NULL && y != NULL);
    q16_t c = q16_cos(theta);
    q16_t s = q16_sin(theta);
    *x = q16_mul(r, c);
    *y = q16_mul(r, s);
}

void syn_cart2sph(q16_t x, q16_t y, q16_t z, q16_t *r, q16_t *theta, q16_t *phi)
{
    SYN_ASSERT(r != NULL && theta != NULL && phi != NULL);
    q16_t r_xy = q16_hypot(x, y);
    *r = q16_hypot(r_xy, z);
    *theta = q16_atan2(y, x);
    *phi = q16_atan2(r_xy, z);
}

void syn_sph2cart(q16_t r, q16_t theta, q16_t phi, q16_t *x, q16_t *y, q16_t *z)
{
    SYN_ASSERT(x != NULL && y != NULL && z != NULL);
    q16_t sin_t = q16_sin(theta);
    q16_t cos_t = q16_cos(theta);
    q16_t sin_p = q16_sin(phi);
    q16_t cos_p = q16_cos(phi);

    q16_t r_sin_p = q16_mul(r, sin_p);
    *x = q16_mul(r_sin_p, cos_t);
    *y = q16_mul(r_sin_p, sin_t);
    *z = q16_mul(r, cos_p);
}
```


