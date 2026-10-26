

# File syn\_qmath.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_qmath.h**](syn__qmath_8h.md)

[Go to the documentation of this file](syn__qmath_8h.md)


```C++

#ifndef SYN_QMATH_H
#define SYN_QMATH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q16.16 type ────────────────────────────────────────────────────────── */

typedef int32_t q16_t;

#define Q16_SHIFT 16                              
#define Q16_ONE ((q16_t)(1L << Q16_SHIFT))        
#define Q16_HALF ((q16_t)(1L << (Q16_SHIFT - 1))) 
/* ── Q0.7 (INT8) & Q0.15 (INT16) Types ─────────────────────────────────── */

typedef int8_t q7_t;

#define Q7_SHIFT 7            
#define Q7_ONE ((q7_t)127)    
#define Q7_MIN ((q7_t) - 128) 
typedef int16_t q15_t;

#define Q15_SHIFT 15              
#define Q15_ONE ((q15_t)32767)    
#define Q15_MIN ((q15_t) - 32768) 
/* ── Constants ──────────────────────────────────────────────────────────── */

#define Q16_PI 205887   
#define Q16_PI_2 102944 
#define Q16_2_PI 411775 
#define Q16_E 178145    
#define Q16_LN2 45426   
#define Q16_SQRT2 92682 
/* ── Conversion macros ──────────────────────────────────────────────────── */

#define Q16_FROM_INT(n) ((q16_t)((int32_t)(n) * Q16_ONE))

#define Q16_FROM_FRAC(num, den) \
    ((q16_t)(((int64_t)((uint64_t)(int64_t)(num) << Q16_SHIFT)) / (den)))

#define Q16_FROM_FLOAT(f) ((q16_t)((f) * (1L << Q16_SHIFT)))

#define Q16_TO_INT(q) ((int32_t)((q) >> Q16_SHIFT))

#define Q16_TO_INT_ROUND(q) ((int32_t)(((q) + Q16_HALF) >> Q16_SHIFT))

#define Q16_FRAC_1000(q) ((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT))

#define Q16_FRAC_10000(q) ((int32_t)((((q) & 0xFFFF) * 10000L) >> Q16_SHIFT))

#define Q7_FROM_FLOAT(f)                 \
    ((q7_t)((f) * 128.0f >= 127.0f ? 127 \
                                   : ((f) * 128.0f <= -128.0f ? -128 : (int8_t)((f) * 128.0f))))

#define Q7_TO_FLOAT(q) ((float)(q) / 128.0f)

#define Q15_FROM_FLOAT(f)               \
    ((q15_t)((f) * 32768.0f >= 32767.0f \
                 ? 32767                \
                 : ((f) * 32768.0f <= -32768.0f ? -32768 : (int16_t)((f) * 32768.0f))))

#define Q15_TO_FLOAT(q) ((float)(q) / 32768.0f)

/* ── Cross-Format Conversions ───────────────────────────────────────────── */

static inline q16_t q7_to_q16(q7_t q)
{
    return (q16_t)((int32_t)q << 9);
}

static inline q7_t q16_to_q7(q16_t q)
{
    int32_t val = q >> 9;
    if (val > 127)
        return 127;
    if (val < -128)
        return -128;
    return (q7_t)val;
}

static inline q16_t q15_to_q16(q15_t q)
{
    return (q16_t)((int32_t)q << 1);
}

static inline q15_t q16_to_q15(q16_t q)
{
    int32_t val = q >> 1;
    if (val > 32767)
        return 32767;
    if (val < -32768)
        return -32768;
    return (q15_t)val;
}

static inline q7_t q7_mul(q7_t a, q7_t b)
{
    return (q7_t)(((int16_t)a * (int16_t)b) >> 7);
}

static inline q16_t q7_mac(q16_t acc, q7_t a, q7_t b)
{
    int32_t prod = (int32_t)a * (int32_t)b;
    return acc + (int32_t)((uint32_t)prod << 2);
}

/* ── Inline arithmetic ──────────────────────────────────────────────────── */

static inline q16_t q16_mul(q16_t a, q16_t b)
{
#if defined(__arm__) && defined(__GNUC__) && !defined(__SOFTFP__)
    int32_t res_lo, res_hi;
    __asm__("smull %0, %1, %2, %3" : "=r"(res_lo), "=r"(res_hi) : "r"(a), "r"(b));
    return (q16_t)(((uint32_t)res_lo >> 16) | ((uint32_t)res_hi << 16));
#else
    return (q16_t)(((int64_t)a * b) >> Q16_SHIFT);
#endif
}

static inline q16_t q16_div(q16_t a, q16_t b)
{
    if (b == 0) {
        return (a >= 0) ? INT32_MAX : INT32_MIN;
    }
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
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
    return (q16_t)r;
}

static inline q16_t q16_sub_sat(q16_t a, q16_t b)
{
    int64_t r = (int64_t)a - b;
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
    return (q16_t)r;
}

static inline q16_t q16_mul_sat(q16_t a, q16_t b)
{
    int64_t r = ((int64_t)a * b) >> Q16_SHIFT;
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
    return (q16_t)r;
}

static inline q16_t q16_lerp(q16_t a, q16_t b, q16_t t)
{
    return a + q16_mul(b - a, t);
}

static inline q16_t q16_clamp(q16_t val, q16_t lo, q16_t hi)
{
    if (val < lo)
        return lo;
    if (val > hi)
        return hi;
    return val;
}

/* ── Transcendental functions (compiled in syn_qmath.c) ─────────────────── */

uint32_t syn_isqrt32(uint32_t n);

uint64_t syn_isqrt64(uint64_t n);

q16_t q16_sin(q16_t x);

q16_t q16_cos(q16_t x);

q16_t q16_sin_fast(q16_t x);

q16_t q16_cos_fast(q16_t x);

void q16_sincos_fast(q16_t x, q16_t *sin_out, q16_t *cos_out);

q16_t q16_tan(q16_t x);

q16_t q16_atan2(q16_t y, q16_t x);

q16_t q16_asin(q16_t x);

q16_t q16_acos(q16_t x);

q16_t q16_sqrt(q16_t x);

q16_t q16_inv(q16_t x);

q16_t q16_rsqrt(q16_t x);

q16_t q16_hypot(q16_t x, q16_t y);

q16_t q16_exp(q16_t x);

q16_t q16_exp_fast(q16_t x);

q16_t q16_log(q16_t x);

q16_t q16_log_fast(q16_t x);

q16_t q16_pow(q16_t base, q16_t exp);

q16_t q16_floor(q16_t x);

q16_t q16_ceil(q16_t x);

q16_t q16_round(q16_t x);

q16_t q16_poly_eval(const q16_t *coeffs, uint8_t n, q16_t x);

/* ── String I/O (compiled in syn_qmath.c) ───────────────────────────────── */

size_t q16_to_str(q16_t val, char *buf, size_t buf_len, uint8_t decimals);

size_t q16_from_str(const char *str, q16_t *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_QMATH_H */
```


