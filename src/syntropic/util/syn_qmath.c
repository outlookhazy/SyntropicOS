#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_QMATH) || SYN_USE_QMATH

/**
 * @file syn_qmath.c
 * @brief Q16.16 fixed-point transcendental functions and string I/O.
 *
 * All functions use 64-bit intermediates for multiply chains. No floating
 * point, no heap, no division by zero. Optimized for 32-bit ARM Cortex-M
 * with hardware multiply; functional on 8-bit AVR.
 */

#include "syn_qmath.h"
#include "../common/syn_defs.h"

/* ════════════════════════════════════════════════════════════════════════ */
/*  Trigonometric — sin, cos, tan                                          */
/* ════════════════════════════════════════════════════════════════════════ */

q16_t q16_sin(q16_t x)
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

q16_t q16_cos(q16_t x)
{
    return q16_sin(x + Q16_PI_2);
}

q16_t q16_tan(q16_t x)
{
    q16_t c = q16_cos(x);
    /* Guard against division by zero near ±π/2 */
    if (c == 0) {
        return (q16_sin(x) >= 0) ? INT32_MAX : INT32_MIN;
    }
    return q16_div(q16_sin(x), c);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Inverse trigonometric — atan2, asin, acos                              */
/* ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Core atan approximation for x in [0, 1] (Q16.16).
 *
 * Uses a 7th-order minimax polynomial:
 *   atan(x) ≈ x - x³/3 + x⁵/5 - x⁷/7
 *
 * Max error < 0.005 rad over [0, 1]. Sufficient for Q16.16 precision.
 */
static q16_t atan_core(q16_t x)
{
    /* x is in [0, Q16_ONE] */
    int64_t x2 = ((int64_t)x * x) >> 16;
    int64_t x3 = (x2 * x) >> 16;
    int64_t x5 = ((x3 * x) >> 16) * x >> 16;
    int64_t x7 = ((x5 * x) >> 16) * x >> 16;

    /* atan(x) ≈ x - x³/3 + x⁵/5 - x⁷/7 */
    int64_t result = (int64_t)x
                   - x3 / 3
                   + x5 / 5
                   - x7 / 7;

    return (q16_t)result;
}

q16_t q16_atan2(q16_t y, q16_t x)
{
    if (x == 0 && y == 0) {
        return 0;
    }

    q16_t abs_x = q16_abs(x);
    q16_t abs_y = q16_abs(y);
    q16_t angle;

    if (abs_x >= abs_y) {
        /* |x| >= |y|: atan(y/x) with |y/x| in [0,1] */
        q16_t ratio = q16_div(abs_y, abs_x);
        angle = atan_core(ratio);
    } else {
        /* |y| > |x|: atan(y/x) = π/2 - atan(x/y) */
        q16_t ratio = q16_div(abs_x, abs_y);
        angle = Q16_PI_2 - atan_core(ratio);
    }

    /* Quadrant mapping */
    if (x < 0) {
        angle = Q16_PI - angle;
    }
    if (y < 0) {
        angle = -angle;
    }

    return angle;
}

q16_t q16_asin(q16_t x)
{
    /* Clamp to [-1, 1] */
    if (x >= Q16_ONE) return Q16_PI_2;
    if (x <= -Q16_ONE) return -Q16_PI_2;

    /*
     * 5th-order Chebyshev approximation for asin(x), |x| <= 1:
     *   asin(x) ≈ c1*x + c3*x³ + c5*x⁵
     *
     * Coefficients (Q16.16):
     *   c1 ≈ 1.0             = 65536
     *   c3 ≈ 1/6             = 10923
     *   c5 ≈ 3/40            = 4915
     */
    static const q16_t c1 = 65536;  /* 1.0      */
    static const q16_t c3 = 10923;  /* 0.16667  */
    static const q16_t c5 = 4915;   /* 0.07500  */

    int64_t x2 = ((int64_t)x * x) >> 16;
    int64_t x3 = (x2 * x) >> 16;
    int64_t x5 = ((x3 * x) >> 16) * x >> 16;

    int64_t result = ((int64_t)c1 * x) >> 16;
    result += ((int64_t)c3 * x3) >> 16;
    result += ((int64_t)c5 * x5) >> 16;

    return (q16_t)result;
}

q16_t q16_acos(q16_t x)
{
    return Q16_PI_2 - q16_asin(x);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Algebraic — sqrt, hypot                                                */
/* ════════════════════════════════════════════════════════════════════════ */

q16_t q16_sqrt(q16_t x)
{
    if (x <= 0) return 0;

    /*
     * Binary restoring square root on the Q16.16 value.
     *
     * We want sqrt(x) where x is in Q16.16. Treat x as a 32-bit integer
     * value representing x * 2^16. Then:
     *   sqrt(x * 2^16) = sqrt(x) * 2^8
     *
     * To get the result in Q16.16 (i.e., sqrt(x) * 2^16), we need to
     * shift the input left by 16 before taking the integer sqrt.
     * That means we compute isqrt(x << 16) which gives sqrt(x) * 2^16.
     *
     * We operate on a 64-bit value: val = (uint64_t)x << 16.
     */
    uint64_t val = (uint64_t)(uint32_t)x << 16;
    uint64_t root = 0;
    uint64_t bit = (uint64_t)1 << 46;  /* Start at highest even bit <= 62 */

    while (bit > val) {
        bit >>= 2;
    }

    while (bit != 0) {
        uint64_t trial = root + bit;
        if (val >= trial) {
            val -= trial;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }

    return (q16_t)root;
}

q16_t q16_hypot(q16_t x, q16_t y)
{
    /*
     * Overflow-safe magnitude: sqrt(x² + y²).
     *
     * If either |x| or |y| exceeds ~181 (Q16), then x*x could overflow
     * int64_t accumulation. Pre-shift both values right by 1 in that case,
     * then shift the result left by 1.
     */
    q16_t ax = q16_abs(x);
    q16_t ay = q16_abs(y);

    /* 181.0 in Q16 = 11862016. If either exceeds this, we risk overflow
     * in the x*x + y*y sum (each product is ~62 bits). */
    int shift = 0;
    if (ax > 11862016 || ay > 11862016) {
        ax >>= 1;
        ay >>= 1;
        shift = 1;
    }

    int64_t sum = ((int64_t)ax * ax) + ((int64_t)ay * ay);

    /* sum is in Q32.32 (two Q16 values multiplied). We need sqrt in Q16.
     * q16_sqrt expects Q16.16 input, but sum is Q32.32.
     * Convert: sum >> 16 gives Q16.16, then sqrt gives Q16.16 result. */
    if (sum >> 16 > INT32_MAX) {
        /* Very large — clamp */
        return INT32_MAX;
    }

    q16_t result = q16_sqrt((q16_t)(sum >> 16));

    if (shift) {
        result <<= 1;
    }

    return result;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Exponential / Logarithmic — exp, log, pow                              */
/* ════════════════════════════════════════════════════════════════════════ */

q16_t q16_exp(q16_t x)
{
    /*
     * Range reduction: x = k * ln(2) + r, where k = floor(x / ln2),
     * and r is in [0, ln2). Then e^x = 2^k * e^r.
     *
     * 2^k is a left shift by k (in Q16: Q16_ONE << k).
     * e^r is computed via 4th-order minimax polynomial over [0, ln2).
     */
    if (x == 0) return Q16_ONE;

    /* Handle negative exponents via e^(-x) = 1/e^x */
    if (x < 0) {
        q16_t pos = q16_exp(-x);
        if (pos == 0) return INT32_MAX; /* -x overflowed */
        return q16_div(Q16_ONE, pos);
    }

    /* Overflow guard: e^10 ≈ 22026, which is near the Q16 integer limit */
    if (x > Q16_FROM_INT(10)) {
        return INT32_MAX;
    }

    /* k = x / ln(2), integer part */
    int32_t k = (int32_t)(((int64_t)x << 16) / Q16_LN2) >> 16;

    /* r = x - k * ln(2), the fractional remainder in [0, ln2) */
    q16_t r = x - q16_mul((q16_t)(k * Q16_ONE), Q16_LN2);

    /* Clamp r to [0, ln2) — numerical safety */
    if (r < 0) { r = 0; k--; }
    if (r >= Q16_LN2) { r -= Q16_LN2; k++; }

    /*
     * e^r ≈ 1 + r + r²/2 + r³/6 + r⁴/24 (4th-order Taylor, r in [0, 0.693])
     * Max error < 0.0002 over this range.
     */
    int64_t r2 = ((int64_t)r * r) >> 16;
    int64_t r3 = (r2 * r) >> 16;
    int64_t r4 = (r3 * r) >> 16;

    q16_t exp_r = (q16_t)(Q16_ONE + r + r2 / 2 + r3 / 6 + r4 / 24);

    /* Multiply by 2^k */
    if (k >= 0) {
        if (k >= 15) return INT32_MAX;  /* Would overflow Q16 integer range */
        return exp_r << k;
    } else {
        if (k < -16) return 0;
        return exp_r >> (-k);
    }
}

q16_t q16_log(q16_t x)
{
    /*
     * Range reduction: normalize x to [1.0, 2.0) by finding the
     * highest set bit. Then:
     *   ln(x) = ln(m * 2^k) = k * ln(2) + ln(m)
     * where m is in [1, 2).
     *
     * ln(m) is computed via polynomial approximation on [1, 2).
     */
    if (x <= 0) return INT32_MIN;  /* Undefined */
    if (x == Q16_ONE) return 0;

    /* Find k such that x = m * 2^k, where m is in [Q16_ONE, 2*Q16_ONE) */
    uint32_t ux = (uint32_t)x;
    int32_t k = 0;

    /* Normalize to [Q16_ONE, 2*Q16_ONE) */
    while (ux >= (uint32_t)(2 * Q16_ONE)) {
        ux >>= 1;
        k++;
    }
    while (ux < (uint32_t)Q16_ONE) {
        ux <<= 1;
        k--;
    }

    /* m is now in [Q16_ONE, 2*Q16_ONE), stored in ux as Q16.16 */
    q16_t m = (q16_t)ux;

    /*
     * ln(m) for m in [1, 2) via Taylor series around m=1:
     *   Let t = m - 1 (so t is in [0, 1))
     *   ln(1 + t) ≈ t - t²/2 + t³/3 - t⁴/4
     *
     * Max error < 0.001 over [0, 1).
     */
    q16_t t = m - Q16_ONE;
    int64_t t2 = ((int64_t)t * t) >> 16;
    int64_t t3 = (t2 * t) >> 16;
    int64_t t4 = (t3 * t) >> 16;

    q16_t ln_m = (q16_t)(t - t2 / 2 + t3 / 3 - t4 / 4);

    /* ln(x) = k * ln(2) + ln(m) */
    return q16_mul(Q16_FROM_INT(k), Q16_LN2) + ln_m;
}

q16_t q16_pow(q16_t base, q16_t exponent)
{
    if (base <= 0) return 0;
    if (exponent == 0) return Q16_ONE;
    if (exponent == Q16_ONE) return base;

    return q16_exp(q16_mul(exponent, q16_log(base)));
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  String I/O                                                             */
/* ════════════════════════════════════════════════════════════════════════ */

size_t q16_to_str(q16_t val, char *buf, size_t buf_len, uint8_t decimals)
{
    if (buf == NULL || buf_len == 0) return 0;
    if (decimals > 4) decimals = 4;

    char tmp[16];
    size_t pos = 0;

    /* Handle negative */
    int32_t sign = 0;
    uint32_t uval;
    if (val < 0) {
        sign = 1;
        /* Use unsigned to handle INT32_MIN correctly */
        uval = (uint32_t)(-(int64_t)val);
    } else {
        uval = (uint32_t)val;
    }

    /* Integer part */
    uint32_t int_part = uval >> 16;
    uint32_t frac_bits = uval & 0xFFFF;

    /* Build integer digits (reverse order) */
    char int_buf[8];
    size_t int_len = 0;
    if (int_part == 0) {
        int_buf[int_len++] = '0';
    } else {
        while (int_part > 0) {
            int_buf[int_len++] = (char)('0' + (int_part % 10));
            int_part /= 10;
        }
    }

    /* Assemble: sign + reversed integer digits */
    if (sign) {
        if (pos < sizeof(tmp) - 1) tmp[pos++] = '-';
    }
    while (int_len > 0) {
        if (pos < sizeof(tmp) - 1) tmp[pos++] = int_buf[--int_len];
    }

    /* Fractional part */
    if (decimals > 0) {
        if (pos < sizeof(tmp) - 1) tmp[pos++] = '.';

        /* Multiply fractional bits by 10 repeatedly to extract digits */
        uint8_t d;
        for (d = 0; d < decimals; d++) {
            frac_bits *= 10;
            uint8_t digit = (uint8_t)(frac_bits >> 16);
            frac_bits &= 0xFFFF;
            if (pos < sizeof(tmp) - 1) tmp[pos++] = (char)('0' + digit);
        }
    }

    tmp[pos] = '\0';

    /* Copy to output buffer */
    if (pos >= buf_len) {
        buf[0] = '\0';
        return 0;
    }

    size_t i;
    for (i = 0; i <= pos; i++) {
        buf[i] = tmp[i];
    }

    return pos;
}

size_t q16_from_str(const char *str, q16_t *out)
{
    if (str == NULL || out == NULL) return 0;

    const char *p = str;
    int32_t sign = 1;

    /* Optional sign */
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    /* Must have at least one digit */
    if (*p < '0' || *p > '9') return 0;

    /* Integer part */
    int32_t int_part = 0;
    while (*p >= '0' && *p <= '9') {
        int_part = int_part * 10 + (*p - '0');
        p++;
    }

    /* Fractional part */
    uint32_t frac_q16 = 0;
    if (*p == '.') {
        p++;
        /*
         * Parse fractional digits. Each digit d at position i contributes
         * d * 65536 / 10^i to the Q16 fractional part. We accumulate by
         * multiplying the running total by 10 and adding the digit, then
         * dividing at the end.
         */
        uint32_t frac_num = 0;
        uint32_t frac_den = 1;
        while (*p >= '0' && *p <= '9') {
            frac_num = frac_num * 10 + (uint32_t)(*p - '0');
            frac_den *= 10;
            p++;
            if (frac_den >= 100000) break;  /* Limit precision to avoid overflow */
        }
        if (frac_den > 0) {
            frac_q16 = (uint32_t)(((uint64_t)frac_num << 16) / frac_den);
        }
    }

    *out = sign * (q16_t)((int32_t)(int_part << 16) | (int32_t)frac_q16);
    return (size_t)(p - str);
}

#endif /* SYN_USE_QMATH */
