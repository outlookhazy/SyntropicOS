

# File syn\_qmath.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_qmath.h**](syn__qmath_8h.md)

[Go to the documentation of this file](syn__qmath_8h.md)


```C++

#ifndef SYN_QMATH_H
#define SYN_QMATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q16.16 type ────────────────────────────────────────────────────────── */

typedef int32_t q16_t;

#define Q16_SHIFT    16                               
#define Q16_ONE      ((q16_t)(1L << Q16_SHIFT))        
#define Q16_HALF     ((q16_t)(1L << (Q16_SHIFT - 1)))  
/* ── Conversion macros ──────────────────────────────────────────────────── */

#define Q16_FROM_INT(n)       ((q16_t)((int32_t)(n) * Q16_ONE))

#define Q16_FROM_FRAC(num, den) ((q16_t)(((int64_t)((uint64_t)(int64_t)(num) << Q16_SHIFT)) / (den)))

#define Q16_FROM_FLOAT(f)     ((q16_t)((f) * (1L << Q16_SHIFT)))

#define Q16_TO_INT(q)         ((int32_t)((q) >> Q16_SHIFT))

#define Q16_TO_INT_ROUND(q)   ((int32_t)(((q) + Q16_HALF) >> Q16_SHIFT))

#define Q16_FRAC_1000(q)      ((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT))

/* ── Arithmetic ─────────────────────────────────────────────────────────── */

static inline q16_t q16_mul(q16_t a, q16_t b)
{
    return (q16_t)(((int64_t)a * b) >> Q16_SHIFT);
}

static inline q16_t q16_div(q16_t a, q16_t b)
{
    return (q16_t)(((int64_t)((uint64_t)(int64_t)a << Q16_SHIFT)) / b);
}

static inline q16_t q16_add(q16_t a, q16_t b)
{
    return a + b;
}

static inline q16_t q16_sub(q16_t a, q16_t b)
{
    return a - b;
}

static inline q16_t q16_abs(q16_t a)
{
    return (a < 0) ? -a : a;
}

static inline q16_t q16_add_sat(q16_t a, q16_t b)
{
    int64_t r = (int64_t)a + b;
    if (r > INT32_MAX) return INT32_MAX;
    if (r < INT32_MIN) return INT32_MIN;
    return (q16_t)r;
}

static inline q16_t q16_mul_sat(q16_t a, q16_t b)
{
    int64_t r = ((int64_t)a * b) >> Q16_SHIFT;
    if (r > INT32_MAX) return INT32_MAX;
    if (r < INT32_MIN) return INT32_MIN;
    return (q16_t)r;
}

static inline q16_t q16_lerp(q16_t a, q16_t b, q16_t t)
{
    return a + q16_mul(b - a, t);
}

static inline q16_t q16_clamp(q16_t val, q16_t lo, q16_t hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

#define Q16_PI          205887  
#define Q16_PI_2        102944  
#define Q16_2_PI        411775  
static inline q16_t q16_sin(q16_t x)
{
    /* Normalize x to [-PI, PI] */
    while (x > Q16_PI)  x -= Q16_2_PI;
    while (x < -Q16_PI) x += Q16_2_PI;

    /* Map to [-PI/2, PI/2] */
    if (x > Q16_PI_2) {
        x = Q16_PI - x;
    } else if (x < -Q16_PI_2) {
        x = -Q16_PI - x;
    }

    /* Taylor series: sin(x) ≈ x - x^3/6 + x^5/120 */
    int64_t x_sq = ((int64_t)x * x) >> 16;
    int64_t x_cube = (x_sq * x) >> 16;
    int64_t x_five = (((x_cube * x) >> 16) * x) >> 16;

    int64_t term3 = x_cube / 6;
    int64_t term5 = x_five / 120;

    return (q16_t)(x - term3 + term5);
}

static inline q16_t q16_cos(q16_t x)
{
    return q16_sin(x + Q16_PI_2);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_QMATH_H */
```


