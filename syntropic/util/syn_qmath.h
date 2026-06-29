/**
 * @file syn_qmath.h
 * @brief Fixed-point Q16.16 arithmetic — header-only, no floating point.
 *
 * All operations use signed 32-bit values with 16 integer bits and
 * 16 fractional bits. Multiply/divide use 64-bit intermediates to
 * avoid overflow.
 *
 * @par Usage
 * @code
 *   q16_t a = Q16_FROM_INT(3);         // 3.0
 *   q16_t b = Q16_FROM_FRAC(1, 2);     // 0.5
 *   q16_t c = q16_mul(a, b);           // 1.5
 *   int   i = Q16_TO_INT(c);           // 1
 *   int   f = Q16_FRAC_1000(c);        // 500 (fractional part × 1000)
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_QMATH_H
#define SYN_QMATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q16.16 type ────────────────────────────────────────────────────────── */

/** Fixed-point Q16.16 type: 16 integer bits, 16 fractional bits. */
typedef int32_t q16_t;

#define Q16_SHIFT    16                               /**< Fractional bit count       */
#define Q16_ONE      ((q16_t)(1 << Q16_SHIFT))         /**< 1.0 in Q16.16              */
#define Q16_HALF     ((q16_t)(1 << (Q16_SHIFT - 1)))   /**< 0.5 in Q16.16              */

/* ── Conversion macros ──────────────────────────────────────────────────── */

/** Integer to Q16. */
#define Q16_FROM_INT(n)       ((q16_t)((int32_t)(n) * Q16_ONE))

/** Fraction to Q16: Q16_FROM_FRAC(1, 3) ≈ 0.333. */
#define Q16_FROM_FRAC(num, den) ((q16_t)(((int64_t)(num) << Q16_SHIFT) / (den)))

/** Float literal to Q16 (compile-time only, avoid at runtime). */
#define Q16_FROM_FLOAT(f)     ((q16_t)((f) * (1 << Q16_SHIFT)))

/** Q16 to integer (truncates toward zero). */
#define Q16_TO_INT(q)         ((int32_t)((q) >> Q16_SHIFT))

/** Q16 to integer (rounded). */
#define Q16_TO_INT_ROUND(q)   ((int32_t)(((q) + Q16_HALF) >> Q16_SHIFT))

/** Fractional part as 0–999 (for printf: "%d.%03d"). */
#define Q16_FRAC_1000(q)      ((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT))

/* ── Arithmetic ─────────────────────────────────────────────────────────── */

/**
 * @brief Multiply two Q16 values.
 * @param a  First operand.
 * @param b  Second operand.
 * @return Product in Q16.
 */
static inline q16_t q16_mul(q16_t a, q16_t b)
{
    return (q16_t)(((int64_t)a * b) >> Q16_SHIFT);
}

/**
 * @brief Divide two Q16 values.
 * @param a  Dividend.
 * @param b  Divisor (must not be zero).
 * @return Quotient in Q16.
 */
static inline q16_t q16_div(q16_t a, q16_t b)
{
    return (q16_t)(((int64_t)a << Q16_SHIFT) / b);
}

/**
 * @brief Add two Q16 values.
 * @param a  First operand.
 * @param b  Second operand.
 * @return Sum in Q16.
 */
static inline q16_t q16_add(q16_t a, q16_t b)
{
    return a + b;
}

/**
 * @brief Subtract two Q16 values.
 * @param a  Minuend.
 * @param b  Subtrahend.
 * @return Difference in Q16.
 */
static inline q16_t q16_sub(q16_t a, q16_t b)
{
    return a - b;
}

/**
 * @brief Absolute value of a Q16 number.
 * @param a  Input value.
 * @return |a| in Q16.
 */
static inline q16_t q16_abs(q16_t a)
{
    return (a < 0) ? -a : a;
}

/**
 * @brief Saturating add (clamp to INT32 range).
 * @param a  First operand.
 * @param b  Second operand.
 * @return Clamped sum in Q16.
 */
static inline q16_t q16_add_sat(q16_t a, q16_t b)
{
    int64_t r = (int64_t)a + b;
    if (r > INT32_MAX) return INT32_MAX;
    if (r < INT32_MIN) return INT32_MIN;
    return (q16_t)r;
}

/**
 * @brief Saturating multiply.
 * @param a  First operand.
 * @param b  Second operand.
 * @return Clamped product in Q16.
 */
static inline q16_t q16_mul_sat(q16_t a, q16_t b)
{
    int64_t r = ((int64_t)a * b) >> Q16_SHIFT;
    if (r > INT32_MAX) return INT32_MAX;
    if (r < INT32_MIN) return INT32_MIN;
    return (q16_t)r;
}

/**
 * @brief Linear interpolation: lerp(a, b, t) where t is Q16 in [0, 1.0].
 * @param a  Start value.
 * @param b  End value.
 * @param t  Interpolation factor (Q16, 0 to Q16_ONE).
 * @return Interpolated value in Q16.
 */
static inline q16_t q16_lerp(q16_t a, q16_t b, q16_t t)
{
    return a + q16_mul(b - a, t);
}

/**
 * @brief Clamp a Q16 value to [lo, hi].
 * @param val  Input value.
 * @param lo   Lower bound.
 * @param hi   Upper bound.
 * @return Clamped value.
 */
static inline q16_t q16_clamp(q16_t val, q16_t lo, q16_t hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

#define Q16_PI          205887  /**< 3.14159 in Q16.16 */
#define Q16_PI_2        102944  /**< 1.57079 in Q16.16 */
#define Q16_2_PI        411775  /**< 6.28318 in Q16.16 */

/**
 * @brief Sine approximation using 5th-order Taylor series.
 * @param x  Angle in Q16 radians.
 * @return sin(x) in Q16.
 */
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

/**
 * @brief Cosine approximation via sin(x + PI/2).
 * @param x  Angle in Q16 radians.
 * @return cos(x) in Q16.
 */
static inline q16_t q16_cos(q16_t x)
{
    return q16_sin(x + Q16_PI_2);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_QMATH_H */
