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

#include "../common/syn_defs.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q16.16 type ────────────────────────────────────────────────────────── */

/** Fixed-point Q16.16 type: 16 integer bits, 16 fractional bits. */
typedef int32_t q16_t;

#define Q16_SHIFT 16                              /**< Fractional bit count       */
#define Q16_ONE ((q16_t)(1L << Q16_SHIFT))        /**< 1.0 in Q16.16              */
#define Q16_HALF ((q16_t)(1L << (Q16_SHIFT - 1))) /**< 0.5 in Q16.16              */

/* ── Q0.7 (INT8) & Q0.15 (INT16) Types ─────────────────────────────────── */

/** Fixed-point Q0.7 type (INT8): 1 sign bit, 7 fractional bits (-1.0 to +0.992). */
typedef int8_t q7_t;

#define Q7_SHIFT 7            /**< Q0.7 fractional shift count */
#define Q7_ONE ((q7_t)127)    /**< Q0.7 maximum positive value (+1.0) */
#define Q7_MIN ((q7_t) - 128) /**< Q0.7 minimum value (-1.0) */

/** Fixed-point Q0.15 type (INT16): 1 sign bit, 15 fractional bits (-1.0 to +0.99996). */
typedef int16_t q15_t;

#define Q15_SHIFT 15              /**< Q0.15 fractional shift count */
#define Q15_ONE ((q15_t)32767)    /**< Q0.15 maximum positive value (+1.0) */
#define Q15_MIN ((q15_t) - 32768) /**< Q0.15 minimum value (-1.0) */

/* ── Constants ──────────────────────────────────────────────────────────── */

#define Q16_PI 205887   /**< 3.14159265 in Q16.16 */
#define Q16_PI_2 102944 /**< 1.57079633 in Q16.16 */
#define Q16_2_PI 411775 /**< 6.28318531 in Q16.16 */
#define Q16_E 178145    /**< 2.71828183 in Q16.16 */
#define Q16_LN2 45426   /**< 0.69314718 in Q16.16 */
#define Q16_SQRT2 92682 /**< 1.41421356 in Q16.16 */

/* ── Conversion macros ──────────────────────────────────────────────────── */

/** Integer to Q16. */
#define Q16_FROM_INT(n) ((q16_t)((uint32_t)(int32_t)(n) << Q16_SHIFT))

/** Fraction to Q16: Q16_FROM_FRAC(1, 3) ≈ 0.333. */
#define Q16_FROM_FRAC(num, den) \
    ((q16_t)(((int64_t)((uint64_t)(int64_t)(num) << Q16_SHIFT)) / (den)))

/** Float literal to Q16 (compile-time only, avoid at runtime). */
#define Q16_FROM_FLOAT(f) ((q16_t)((f) * (1L << Q16_SHIFT)))

/** Q16 to integer (truncates toward negative infinity). */
#define Q16_TO_INT(q) ((int32_t)((q) >> Q16_SHIFT))

/** Q16 to integer (rounded, 64-bit intermediate avoids INT32_MAX overflow). */
#define Q16_TO_INT_ROUND(q) ((int32_t)(((int64_t)(q) + (int64_t)Q16_HALF) >> Q16_SHIFT))

/**
 * @brief Extract scaled fractional magnitude of a Q16 value safely across full int32_t range.
 * @param q      Input Q16 value.
 * @param scale  Decimal scale factor (e.g. 1000 or 10000).
 * @return Scaled fractional magnitude in [0, scale - 1].
 */
static inline int32_t syn_q16_frac_scaled(q16_t q, uint32_t scale)
{
    uint64_t mag = (q < 0) ? (uint64_t)(-(int64_t)q) : (uint64_t)(uint32_t)q;
    return (int32_t)(((mag & 0xFFFFULL) * (uint64_t)scale) >> Q16_SHIFT);
}

/** Fractional part as 0–999 (for printf: "%d.%03d"). */
#define Q16_FRAC_1000(q) syn_q16_frac_scaled((q), 1000U)

/** Fractional part as 0–9999 (for printf: "%d.%04d"). */
#define Q16_FRAC_10000(q) syn_q16_frac_scaled((q), 10000U)

/** Float literal to Q7. */
#define Q7_FROM_FLOAT(f)                 \
    ((q7_t)((f) * 128.0f >= 127.0f ? 127 \
                                   : ((f) * 128.0f <= -128.0f ? -128 : (int8_t)((f) * 128.0f))))

/** Q7 to float. */
#define Q7_TO_FLOAT(q) ((float)(q) / 128.0f)

/** Float literal to Q15. */
#define Q15_FROM_FLOAT(f)               \
    ((q15_t)((f) * 32768.0f >= 32767.0f \
                 ? 32767                \
                 : ((f) * 32768.0f <= -32768.0f ? -32768 : (int16_t)((f) * 32768.0f))))

/** Q15 to float. */
#define Q15_TO_FLOAT(q) ((float)(q) / 32768.0f)

/* ── Cross-Format Conversions ───────────────────────────────────────────── */

/**
 * @brief Convert Q7 (INT8) to Q16.16.
 * @param q Q7 input value.
 * @return Q16.16 converted value.
 */
static inline q16_t q7_to_q16(q7_t q)
{
    return (q16_t)((uint32_t)(int32_t)q << 9U);
}

/**
 * @brief Convert Q16.16 to Q7 (INT8) with saturation clamping.
 * @param q Q16.16 input value.
 * @return Clamped Q7 output.
 */
static inline q7_t q16_to_q7(q16_t q)
{
    int32_t val = q >> 9;
    if (val > 127)
        return 127;
    if (val < -128)
        return -128;
    return (q7_t)val;
}

/**
 * @brief Convert Q15 (INT16) to Q16.16.
 * @param q Q15 input value.
 * @return Q16.16 converted value.
 */
static inline q16_t q15_to_q16(q15_t q)
{
    return (q16_t)((uint32_t)(int32_t)q << 1U);
}

/**
 * @brief Convert Q16.16 to Q15 (INT16) with saturation clamping.
 * @param q Q16.16 input value.
 * @return Clamped Q15 output.
 */
static inline q15_t q16_to_q15(q16_t q)
{
    int32_t val = q >> 1;
    if (val > 32767)
        return 32767;
    if (val < -32768)
        return -32768;
    return (q15_t)val;
}

/**
 * @brief Multiply two Q7 numbers.
 * @param a First Q7 factor.
 * @param b Second Q7 factor.
 * @return Q7 product.
 */
static inline q7_t q7_mul(q7_t a, q7_t b)
{
    return (q7_t)(((int16_t)a * (int16_t)b) >> 7);
}

/**
 * @brief Multiply two Q7 (INT8) numbers and accumulate into a 32-bit Q16.16 accumulator.
 * @param acc  Existing Q16.16 accumulator.
 * @param a    First Q7 factor.
 * @param b    Second Q7 factor.
 * @return New Q16.16 accumulated sum.
 */
static inline q16_t q7_mac(q16_t acc, q7_t a, q7_t b)
{
    int32_t prod = (int32_t)a * (int32_t)b;
    return acc + (int32_t)((uint32_t)prod << 2);
}

/* ── Inline arithmetic ──────────────────────────────────────────────────── */

/**
 * @brief Multiply two Q16 values.
 * @param a  First operand.
 * @param b  Second operand.
 * @return Product in Q16.
 */
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

/**
 * @brief Divide two Q16 values.
 * @param a  Dividend.
 * @param b  Divisor (must not be zero).
 * @return Quotient in Q16.
 */
static inline q16_t q16_div(q16_t a, q16_t b)
{
    if (b == 0) {
        return (a >= 0) ? INT32_MAX : INT32_MIN;
    }
    int64_t num = (int64_t)((uint64_t)(int64_t)a << Q16_SHIFT);
    int64_t quo = num / (int64_t)b;
    if (quo > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (quo < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (q16_t)quo;
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
    if (a == INT32_MIN) {
        return INT32_MAX;
    }
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
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
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
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
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
    if (r > INT32_MAX)
        return INT32_MAX;
    if (r < INT32_MIN)
        return INT32_MIN;
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
    int64_t diff = (int64_t)b - (int64_t)a;
    int64_t res = (int64_t)a + ((diff * (int64_t)t) >> Q16_SHIFT);
    if (res > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (res < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (q16_t)res;
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
    if (val < lo)
        return lo;
    if (val > hi)
        return hi;
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
 * @brief Ultra-fast Parabolic/Chebyshev Sine approximation (<20 CPU cycles).
 * @param x  Angle in Q16 radians.
 * @return sin(x) in Q16.
 */
q16_t q16_sin_fast(q16_t x);

/**
 * @brief Ultra-fast Parabolic/Chebyshev Cosine approximation (<20 CPU cycles).
 * @param x  Angle in Q16 radians.
 * @return cos(x) in Q16.
 */
q16_t q16_cos_fast(q16_t x);

/**
 * @brief Simultaneous ultra-fast sine and cosine calculation (<30 CPU cycles).
 * @param x        Angle in Q16 radians.
 * @param sin_out  Output sin(x) in Q16.
 * @param cos_out  Output cos(x) in Q16.
 */
void q16_sincos_fast(q16_t x, q16_t *sin_out, q16_t *cos_out);

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
 * @brief Fast reciprocal 1/x in Q16.16 using Newton-Raphson iterations.
 * @param x  Input in Q16 (must be > 0).
 * @return 1/x in Q16. Returns INT32_MAX on x == 0.
 */
q16_t q16_inv(q16_t x);

/**
 * @brief Fast inverse square root 1/√x in Q16.16 (Fast Quake/Newton-Raphson algorithm).
 * @param x  Input in Q16 (must be > 0).
 * @return 1/√x in Q16. Returns 0 if x <= 0.
 */
q16_t q16_rsqrt(q16_t x);

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
 * @brief Ultra-fast hybrid LUT exponential e^x (~15 CPU cycles).
 * @param x  Exponent in Q16.
 * @return e^x in Q16. Returns INT32_MAX on overflow.
 */
q16_t q16_exp_fast(q16_t x);

/**
 * @brief Fixed-point natural logarithm ln(x) (CLZ + minimax polynomial).
 * @param x  Input in Q16 (must be > 0).
 * @return ln(x) in Q16.
 */
q16_t q16_log(q16_t x);

/**
 * @brief Ultra-fast hybrid LUT natural logarithm ln(x) (~15 CPU cycles).
 * @param x  Input in Q16 (must be > 0).
 * @return ln(x) in Q16.
 */
q16_t q16_log_fast(q16_t x);

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
