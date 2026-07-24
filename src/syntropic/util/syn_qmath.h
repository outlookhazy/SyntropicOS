/**
 * @file syn_qmath.h
 * @brief Fixed-point Q16.16 arithmetic — no floating point.
 *
 * All operations use signed 32-bit values with 16 integer bits and
 * 16 fractional bits. Multiply/divide use 64-bit intermediates to
 * avoid overflow.
 *
 * Basic arithmetic (add, sub, mul, div, lerp, clamp) is inline in this
 * header. Transcendental functions (sin, cos, sqrt, atan2, exp, log)
 * and string I/O are compiled in syn_qmath.c.
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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q16.16 type ────────────────────────────────────────────────────────── */

/** Fixed-point Q16.16 type: 16 integer bits, 16 fractional bits. */
typedef int32_t q16_t;

#define Q16_SHIFT    16                               /**< Fractional bit count       */
#define Q16_ONE      ((q16_t)(1L << Q16_SHIFT))        /**< 1.0 in Q16.16              */
#define Q16_HALF     ((q16_t)(1L << (Q16_SHIFT - 1)))  /**< 0.5 in Q16.16              */

/* ── Constants ──────────────────────────────────────────────────────────── */

#define Q16_PI          205887  /**< 3.14159265 in Q16.16 */
#define Q16_PI_2        102944  /**< 1.57079633 in Q16.16 */
#define Q16_2_PI        411775  /**< 6.28318531 in Q16.16 */
#define Q16_E           178145  /**< 2.71828183 in Q16.16 */
#define Q16_LN2          45426  /**< 0.69314718 in Q16.16 */
#define Q16_SQRT2        92682  /**< 1.41421356 in Q16.16 */

/* ── Conversion macros ──────────────────────────────────────────────────── */

/** Integer to Q16. */
#define Q16_FROM_INT(n)       ((q16_t)((int32_t)(n) * Q16_ONE))

/** Fraction to Q16: Q16_FROM_FRAC(1, 3) ≈ 0.333. */
#define Q16_FROM_FRAC(num, den) ((q16_t)(((int64_t)((uint64_t)(int64_t)(num) << Q16_SHIFT)) / (den)))

/** Float literal to Q16 (compile-time only, avoid at runtime). */
#define Q16_FROM_FLOAT(f)     ((q16_t)((f) * (1L << Q16_SHIFT)))

/** Q16 to integer (truncates toward zero). */
#define Q16_TO_INT(q)         ((int32_t)((q) >> Q16_SHIFT))

/** Q16 to integer (rounded). */
#define Q16_TO_INT_ROUND(q)   ((int32_t)(((q) + Q16_HALF) >> Q16_SHIFT))

/** Fractional part as 0–999 (for printf: "%d.%03d"). */
#define Q16_FRAC_1000(q)      ((int32_t)((((q) & 0xFFFF) * 1000L) >> Q16_SHIFT))

/** Fractional part as 0–9999 (for printf: "%d.%04d"). */
#define Q16_FRAC_10000(q)     ((int32_t)((((q) & 0xFFFF) * 10000L) >> Q16_SHIFT))

/* ── Inline arithmetic ──────────────────────────────────────────────────── */

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
    return (q16_t)(((int64_t)((uint64_t)(int64_t)a << Q16_SHIFT)) / b);
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
 * @brief Saturating subtract (clamp to INT32 range).
 * @param a  Minuend.
 * @param b  Subtrahend.
 * @return Clamped difference in Q16.
 */
static inline q16_t q16_sub_sat(q16_t a, q16_t b)
{
    int64_t r = (int64_t)a - b;
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

/* ── Transcendental functions (compiled in syn_qmath.c) ─────────────────── */

/**
 * @brief Compute floor of 32-bit unsigned integer square root: ⌊√n⌋.
 * @param n  Unsigned 32-bit integer.
 * @return ⌊√n⌋.
 */
uint32_t syn_isqrt32(uint32_t n);

/**
 * @brief Compute floor of 64-bit unsigned integer square root: ⌊√n⌋.
 * @param n  Unsigned 64-bit integer.
 * @return ⌊√n⌋.
 */
uint64_t syn_isqrt64(uint64_t n);

/**
 * @brief Sine approximation (5th-order Taylor series).
 * @param x  Angle in Q16 radians.
 * @return sin(x) in Q16.
 */
q16_t q16_sin(q16_t x);

/**
 * @brief Cosine approximation via sin(x + π/2).
 * @param x  Angle in Q16 radians.
 * @return cos(x) in Q16.
 */
q16_t q16_cos(q16_t x);

/**
 * @brief Tangent: sin(x) / cos(x).
 * @param x  Angle in Q16 radians. Must not be near ±π/2.
 * @return tan(x) in Q16.
 */
q16_t q16_tan(q16_t x);

/**
 * @brief Four-quadrant arctangent (minimax polynomial).
 * @param y  Y coordinate in Q16.
 * @param x  X coordinate in Q16.
 * @return Angle in Q16 radians [−π, π].
 */
q16_t q16_atan2(q16_t y, q16_t x);

/**
 * @brief Arc-sine (5th-order Chebyshev polynomial).
 * @param x  Input in Q16, must be in [−1.0, 1.0].
 * @return Angle in Q16 radians [−π/2, π/2].
 */
q16_t q16_asin(q16_t x);

/**
 * @brief Arc-cosine: π/2 − asin(x).
 * @param x  Input in Q16, must be in [−1.0, 1.0].
 * @return Angle in Q16 radians [0, π].
 */
q16_t q16_acos(q16_t x);

/**
 * @brief Fixed-point square root (binary restoring algorithm).
 * @param x  Input in Q16 (must be ≥ 0).
 * @return √x in Q16.
 */
q16_t q16_sqrt(q16_t x);

/**
 * @brief Overflow-safe hypotenuse: √(x² + y²).
 * @param x  First coordinate in Q16.
 * @param y  Second coordinate in Q16.
 * @return Magnitude in Q16.
 */
q16_t q16_hypot(q16_t x, q16_t y);

/**
 * @brief Fixed-point exponential e^x (range reduction + minimax polynomial).
 * @param x  Exponent in Q16.
 * @return e^x in Q16. Returns INT32_MAX on overflow.
 */
q16_t q16_exp(q16_t x);

/**
 * @brief Fixed-point natural logarithm ln(x) (CLZ + minimax polynomial).
 * @param x  Input in Q16 (must be > 0).
 * @return ln(x) in Q16.
 */
q16_t q16_log(q16_t x);

/**
 * @brief Fixed-point power: base^exp = exp(exp * log(base)).
 * @param base  Base in Q16 (must be > 0).
 * @param exp   Exponent in Q16.
 * @return base^exp in Q16.
 */
q16_t q16_pow(q16_t base, q16_t exp);

/**
 * @brief Fixed-point floor: largest integer <= x.
 * @param x Input in Q16.
 * @return Floor value in Q16.
 */
q16_t q16_floor(q16_t x);

/**
 * @brief Fixed-point ceil: smallest integer >= x.
 * @param x Input in Q16.
 * @return Ceil value in Q16.
 */
q16_t q16_ceil(q16_t x);

/**
 * @brief Fixed-point round: nearest integer (half rounded up).
 * @param x Input in Q16.
 * @return Rounded value in Q16.
 */
q16_t q16_round(q16_t x);

/**
 * @brief Evaluate polynomial P(x) = c0 + c1*x + c2*x^2 + ... + cn*x^n via Horner's method.
 * @param coeffs Array of polynomial coefficients [c0, c1, ..., cn].
 * @param n      Number of coefficients (order = n - 1).
 * @param x      Evaluation point in Q16.
 * @return P(x) in Q16.
 */
q16_t q16_poly_eval(const q16_t *coeffs, uint8_t n, q16_t x);

/* ── String I/O (compiled in syn_qmath.c) ───────────────────────────────── */

/**
 * @brief Format a Q16 value as a decimal string (e.g. "-12.345").
 *
 * No heap allocation. The caller provides the output buffer.
 *
 * @param val       Value to format.
 * @param buf       Output buffer (must be large enough).
 * @param buf_len   Size of output buffer in bytes.
 * @param decimals  Number of fractional digits (0–4).
 * @return Number of characters written (excluding NUL), or 0 on error.
 */
size_t q16_to_str(q16_t val, char *buf, size_t buf_len, uint8_t decimals);

/**
 * @brief Parse a decimal string (e.g. "-3.1415") into Q16.
 *
 * Handles optional sign, integer part, optional decimal point and
 * fractional digits. Stops at first non-numeric character.
 *
 * @param str  Input string (NUL-terminated).
 * @param out  Output Q16 value.
 * @return Number of characters consumed, or 0 on parse error.
 */
size_t q16_from_str(const char *str, q16_t *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_QMATH_H */
