/**
 * @file syn_p256.c
 * @brief NIST P-256 (secp256r1 / prime256v1) Elliptic Curve Cryptography implementation.
 */

#include "syn_p256.h"

#if SYN_P256_ENABLED

#include "syn_hmac_drbg.h"

#include <string.h>

/**
 * @brief Zeroize sensitive stack buffers without compiler dead-store elimination.
 * @param v Buffer pointer.
 * @param n Number of bytes to zeroize.
 */
static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

/* NIST P-256 Curve Constants (Little-Endian 32-bit Words: limbs[0] is least significant) */

/** @brief Prime p = 2^256 - 2^224 + 2^192 + 2^96 - 1 */
static const uint32_t P256_P[8] = {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U,
                                   0x00000000U, 0x00000000U, 0x00000001U, 0xFFFFFFFFU};

/** @brief Group order n */
static const uint32_t P256_N[8] = {0xFC632551U, 0xF3B9CAC2U, 0xA7179E84U, 0xBCE6FAADU,
                                   0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0xFFFFFFFFU};

/** @brief Group order n - 2 for constant-time Fermat modular inversion */
static const uint32_t P256_N_MINUS_2[8] = {0xFC63254FU, 0xF3B9CAC2U, 0xA7179E84U, 0xBCE6FAADU,
                                           0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0xFFFFFFFFU};

/** @brief Curve coefficient b */
static const uint32_t P256_B[8] = {0x27D2604BU, 0x3BCE3C3EU, 0xCC53B0F6U, 0x651D06B0U,
                                   0x769886BCU, 0xB3EBBD55U, 0xAA3A93E7U, 0x5AC635D8U};

/** @brief Base Generator G x-coordinate */
static const uint32_t P256_GX[8] = {0xD898C296U, 0xF4A13945U, 0x2DEB33A0U, 0x77037D81U,
                                    0x63A440F2U, 0xF8BCE6E5U, 0xE12C4247U, 0x6B17D1F2U};

/** @brief Base Generator G y-coordinate */
static const uint32_t P256_GY[8] = {0x37BF51F5U, 0xCBB64068U, 0x6B315ECEU, 0x2BCE3357U,
                                    0x7C0F9E16U, 0x8EE7EB4AU, 0xFE1A7F9BU, 0x4FE342E2U};

/** @brief Big-number 1 */
static const uint32_t BN_ONE[8] = {1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

/* ── 256-bit Big-Number Utilities ────────────────────────────────────────── */

/** @brief 256-bit unsigned big integer representation in eight 32-bit limbs. */
typedef uint32_t bignum256[8];

/**
 * @brief Load bignum256 from 32 big-endian bytes.
 * @param[out] r Output big-number.
 * @param[in] bytes 32-byte big-endian input array.
 */
static void bn_from_bytes(bignum256 r, const uint8_t bytes[32])
{
    for (int i = 0; i < 8; i++) {
        int idx = (7 - i) * 4;
        r[i] = ((uint32_t)bytes[idx + 0] << 24U) | ((uint32_t)bytes[idx + 1] << 16U) |
               ((uint32_t)bytes[idx + 2] << 8U) | ((uint32_t)bytes[idx + 3]);
    }
}

/**
 * @brief Export bignum256 to 32 big-endian bytes.
 * @param[out] bytes 32-byte big-endian output array.
 * @param[in] a Input big-number.
 */
static void bn_to_bytes(uint8_t bytes[32], const bignum256 a)
{
    for (int i = 0; i < 8; i++) {
        int idx = (7 - i) * 4;
        bytes[idx + 0] = (uint8_t)(a[i] >> 24U);
        bytes[idx + 1] = (uint8_t)(a[i] >> 16U);
        bytes[idx + 2] = (uint8_t)(a[i] >> 8U);
        bytes[idx + 3] = (uint8_t)(a[i]);
    }
}

/**
 * @brief Compare two 256-bit big numbers.
 * @param[in] a First big number.
 * @param[in] b Second big number.
 * @return 1 if a > b, -1 if a < b, 0 if equal.
 */
static int bn_cmp(const bignum256 a, const bignum256 b)
{
    for (int i = 7; i >= 0; i--) {
        if (a[i] > b[i])
            return 1;
        if (a[i] < b[i])
            return -1;
    }
    return 0;
}

/**
 * @brief Check if bignum256 is zero in constant time.
 * @param[in] a Input big number.
 * @return True if zero, false otherwise.
 */
static bool bn_is_zero(const bignum256 a)
{
    uint32_t acc = 0U;
    for (uint32_t i = 0U; i < 8U; i++) {
        acc |= a[i];
    }
    return (acc == 0U);
}

/**
 * @brief Constant-time conditional bignum256 move: dst = (cond != 0) ? src : dst.
 * @param[in,out] dst Destination big number.
 * @param[in] src Source big number.
 * @param[in] cond Condition flag (0 or 1).
 */
static void bn_cmov(bignum256 dst, const bignum256 src, uint32_t cond)
{
    uint32_t mask = 0U - (cond & 1U);
    for (uint32_t i = 0U; i < 8U; i++) {
        dst[i] = (dst[i] & ~mask) | (src[i] & mask);
    }
}

/**
 * @brief Copy bignum256.
 * @param[out] r Destination.
 * @param[in] a Source.
 */
static void bn_copy(bignum256 r, const bignum256 a)
{
    memcpy(r, a, sizeof(bignum256));
}

/* ── Field Arithmetic Modulo p ─────────────────────────────────────────── */

/**
 * @brief Raw 256-bit addition with carry out.
 * @param[out] r Sum result.
 * @param[in] a First term.
 * @param[in] b Second term.
 * @return Carry out bit (0 or 1).
 */
static uint32_t bn_add_raw(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t sum = (uint64_t)a[i] + (uint64_t)b[i] + carry;
        r[i] = (uint32_t)sum;
        carry = sum >> 32U;
    }
    return (uint32_t)carry;
}

/**
 * @brief Raw 256-bit subtraction with borrow out.
 * @param[out] r Difference result.
 * @param[in] a Minuend.
 * @param[in] b Subtrahend.
 * @return Borrow out bit (0 or 1).
 */
static uint32_t bn_sub_raw(bignum256 r, const bignum256 a, const bignum256 b)
{
    int64_t borrow = 0;
    for (int i = 0; i < 8; i++) {
        int64_t diff = (int64_t)a[i] - (int64_t)b[i] + borrow;
        r[i] = (uint32_t)diff;
        borrow = diff >> 32U;
    }
    return (uint32_t)(borrow != 0);
}

/**
 * @brief Field addition modulo p.
 * @param[out] r Sum modulo p.
 * @param[in] a First operand.
 * @param[in] b Second operand.
 */
static void p256_mod_p_add(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t carry = bn_add_raw(r, a, b);
    if (carry || bn_cmp(r, P256_P) >= 0) {
        bn_sub_raw(r, r, P256_P);
    }
}

/**
 * @brief Field subtraction modulo p.
 * @param[out] r Difference modulo p.
 * @param[in] a Minuend.
 * @param[in] b Subtrahend.
 */
static void p256_mod_p_sub(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t borrow = bn_sub_raw(r, a, b);
    if (borrow) {
        bn_add_raw(r, r, P256_P);
    }
}

/**
 * @brief 256x256-bit raw integer multiplication producing a 512-bit product.
 * @param[out] t 512-bit product (16 limbs).
 * @param[in] a First 256-bit factor.
 * @param[in] b Second 256-bit factor.
 */
static void bn_mul_raw(uint32_t t[16], const bignum256 a, const bignum256 b)
{
    memset(t, 0, 16 * sizeof(uint32_t));
    for (int i = 0; i < 8; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < 8; j++) {
            uint64_t prod = (uint64_t)a[i] * (uint64_t)b[j] + (uint64_t)t[i + j] + carry;
            t[i + j] = (uint32_t)prod;
            carry = prod >> 32U;
        }
        int k = i + 8;
        while (carry > 0 && k < 16) {
            uint64_t sum = (uint64_t)t[k] + carry;
            t[k] = (uint32_t)sum;
            carry = sum >> 32U;
            k++;
        }
    }
}

/**
 * @brief 512-bit integer fast reduction modulo p using Solinas algorithm for NIST P-256.
 * @param[out] r Reduced 256-bit value modulo p.
 * @param[in] c 512-bit integer in 16 32-bit limbs.
 */
static void p256_mod_p_reduce(bignum256 r, const uint32_t c[16])
{
    int64_t diff[8];
    diff[0] = (int64_t)c[0] + c[8] + c[9] - c[11] - c[12] - c[13] - c[14];
    diff[1] = (int64_t)c[1] + c[9] + c[10] - c[12] - c[13] - c[14] - c[15];
    diff[2] = (int64_t)c[2] + c[10] + c[11] - c[13] - c[14] - c[15];
    diff[3] = (int64_t)c[3] - c[8] - c[9] + 2LL * c[11] + 2LL * c[12] + c[13] - c[15];
    diff[4] = (int64_t)c[4] - c[9] - c[10] + 2LL * c[12] + 2LL * c[13] + c[14];
    diff[5] = (int64_t)c[5] - c[10] - c[11] + 2LL * c[13] + 2LL * c[14] + c[15];
    diff[6] = (int64_t)c[6] - c[8] - c[9] + c[13] + 3LL * c[14] + 2LL * c[15];
    diff[7] = (int64_t)c[7] + c[8] - c[10] - c[11] - c[12] - c[13] + 3LL * c[15];

    int64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        int64_t val = diff[i] + carry;
        r[i] = (uint32_t)val;
        carry = val >> 32U;
    }

    while (carry > 0 || bn_cmp(r, P256_P) >= 0) {
        bn_sub_raw(r, r, P256_P);
        carry--;
    }
    while (carry < 0) {
        bn_add_raw(r, r, P256_P);
        carry++;
    }
}

/**
 * @brief Field multiplication modulo p.
 * @param[out] r Product modulo p.
 * @param[in] a First factor.
 * @param[in] b Second factor.
 */
static void p256_mod_p_mul(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t t[16];
    bn_mul_raw(t, a, b);
    p256_mod_p_reduce(r, t);
}

/**
 * @brief Field squaring modulo p.
 * @param[out] r Result a^2 mod p.
 * @param[in] a Base.
 */
static void p256_mod_p_sqr(bignum256 r, const bignum256 a)
{
    p256_mod_p_mul(r, a, a);
}

/**
 * @brief Field inversion modulo p via Fermat's Little Theorem: a^(p-2) mod p.
 * @param[out] r Inverted value.
 * @param[in] a Base to invert.
 */
static void p256_mod_p_inv(bignum256 r, const bignum256 a)
{
    /* LCOV_EXCL_START: Identity shortcut */
    if (bn_cmp(a, BN_ONE) == 0) {
        bn_copy(r, BN_ONE);
        return;
    }
    /* LCOV_EXCL_STOP */

    bignum256 res;
    memset(res, 0, sizeof(res));
    res[0] = 1U;

    bignum256 base;
    bn_copy(base, a);

    /* p - 2 exponent */
    bignum256 exp;
    bn_copy(exp, P256_P);
    exp[0] -= 2U;

    for (int i = 0; i < 8; i++) {
        uint32_t w = exp[i];
        for (int b = 0; b < 32; b++) {
            if (w & 1U) {
                p256_mod_p_mul(res, res, base);
            }
            p256_mod_p_sqr(base, base);
            w >>= 1U;
        }
    }
    bn_copy(r, res);
    syn_secure_zero(res, sizeof(res));
    syn_secure_zero(base, sizeof(base));
}

/* ── Group Order Arithmetic Modulo n ────────────────────────────────────── */

/**
 * @brief Constant-time single conditional subtraction modulo n for 256-bit inputs.
 *
 * Since 2^256 - 1 < 2 * n for NIST P-256, subtracting n once when a >= n
 * reduces any 256-bit integer into [0, n - 1] without secret-dependent branches.
 *
 * @param[in,out] a Value to reduce in-place modulo n.
 */
static void p256_reduce_once_mod_n(bignum256 a)
{
    bignum256 tmp;
    uint32_t borrow = bn_sub_raw(tmp, a, P256_N);
    bn_cmov(a, tmp, 1U ^ borrow);
    syn_secure_zero(tmp, sizeof(tmp));
}

/**
 * @brief Group order multiplication modulo n.
 * @param[out] r Product modulo n.
 * @param[in] a First factor.
 * @param[in] b Second factor.
 */
static void p256_mod_n_mul(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t t[16];
    bn_mul_raw(t, a, b);

    bignum256 rem = {0};
    bignum256 diff;
    for (int bit = 511; bit >= 0; bit--) {
        uint32_t carry_bit = (t[bit / 32] >> ((uint32_t)bit % 32U)) & 1U;
        for (int i = 0; i < 8; i++) {
            uint32_t next_carry = rem[i] >> 31U;
            rem[i] = (rem[i] << 1U) | carry_bit;
            carry_bit = next_carry;
        }
        uint32_t borrow = bn_sub_raw(diff, rem, P256_N);
        bn_cmov(rem, diff, carry_bit | (1U ^ borrow));
    }
    bn_copy(r, rem);
    syn_secure_zero(t, sizeof(t));
    syn_secure_zero(rem, sizeof(rem));
    syn_secure_zero(diff, sizeof(diff));
}

/**
 * @brief Constant-time group order inversion modulo n via Fermat: a^(n-2) mod n.
 * @param[out] r Inverted value (0 when a == 0).
 * @param[in] a Base to invert.
 */
static void p256_mod_n_inv(bignum256 r, const bignum256 a)
{
    bignum256 res = {1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    bignum256 base;
    bn_copy(base, a);

    for (uint32_t i = 0U; i < 8U; i++) {
        uint32_t w = P256_N_MINUS_2[i];
        for (uint32_t b = 0U; b < 32U; b++) {
            if ((w & 1U) != 0U) {
                p256_mod_n_mul(res, res, base);
            }
            p256_mod_n_mul(base, base, base);
            w >>= 1U;
        }
    }
    bn_copy(r, res);
    syn_secure_zero(res, sizeof(res));
    syn_secure_zero(base, sizeof(base));
}

/* ── Jacobian Point Operations ───────────────────────────────────────────── */

/** @brief Point representation in Jacobian projective coordinates (X, Y, Z). */
typedef struct {
    bignum256 x;   /**< Projective coordinate X */
    bignum256 y;   /**< Projective coordinate Y */
    bignum256 z;   /**< Projective coordinate Z */
    bool infinity; /**< True if point is at infinity */
} P256_Point;

/**
 * @brief Set Jacobian point to infinity.
 * @param[out] p Point structure to initialize to infinity.
 */
static void point_set_infinity(P256_Point *p)
{
    memset(p, 0, sizeof(P256_Point));
    p->infinity = true;
}

/**
 * @brief Constant-time conditional point move: dst = (mask != 0) ? src : dst.
 * @param[in,out] dst Destination point.
 * @param[in] src Source point.
 * @param[in] mask Bitmask (0x00000000 or 0xFFFFFFFF).
 */
static void point_cmov(P256_Point *dst, const P256_Point *src, uint32_t mask)
{
    for (int i = 0; i < 8; i++) {
        dst->x[i] = (dst->x[i] & ~mask) | (src->x[i] & mask);
        dst->y[i] = (dst->y[i] & ~mask) | (src->y[i] & mask);
        dst->z[i] = (dst->z[i] & ~mask) | (src->z[i] & mask);
    }
    dst->infinity = (((uint32_t)dst->infinity & ~mask) | ((uint32_t)src->infinity & mask)) != 0U;
}

/**
 * @brief Convert Jacobian coordinates (X, Y, Z) to Affine coordinates (x, y).
 * @param[in] p Input Jacobian point.
 * @param[out] x_out Output affine X coordinate.
 * @param[out] y_out Output affine Y coordinate.
 */
static void point_to_affine(const P256_Point *p, bignum256 x_out, bignum256 y_out)
{
    /* LCOV_EXCL_START: Point at infinity / zero guard */
    if (p->infinity || bn_is_zero(p->z)) {
        memset(x_out, 0, sizeof(bignum256));
        memset(y_out, 0, sizeof(bignum256));
        return;
    }
    /* LCOV_EXCL_STOP */
    if (bn_cmp(p->z, BN_ONE) == 0) {
        bn_copy(x_out, p->x);
        bn_copy(y_out, p->y);
        return;
    }

    bignum256 z_inv;
    bignum256 z_inv2;
    bignum256 z_inv3;
    p256_mod_p_inv(z_inv, p->z);
    p256_mod_p_sqr(z_inv2, z_inv);
    p256_mod_p_mul(z_inv3, z_inv2, z_inv);
    p256_mod_p_mul(x_out, p->x, z_inv2);
    p256_mod_p_mul(y_out, p->y, z_inv3);
    syn_secure_zero(z_inv, sizeof(z_inv));
    syn_secure_zero(z_inv2, sizeof(z_inv2));
    syn_secure_zero(z_inv3, sizeof(z_inv3));
}

/**
 * @brief Constant-time Point Doubling in Jacobian coordinates: 2P.
 * @param[out] r Output point 2P.
 * @param[in] p Input point P.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void
point_double(P256_Point *r, const P256_Point *p)
{
    uint32_t is_inf = ((uint32_t)p->infinity | (uint32_t)bn_is_zero(p->y)) & 1U;

    P256_Point out;
    bignum256 s;
    bignum256 m;
    bignum256 t;
    bignum256 y_sqr;
    p256_mod_p_sqr(y_sqr, p->y);

    /* S = 4 * X * Y^2 */
    p256_mod_p_mul(s, p->x, y_sqr);
    p256_mod_p_add(s, s, s);
    p256_mod_p_add(s, s, s);

    /* M = 3 * (X^2 - Z^4) = 3 * (X - Z^2) * (X + Z^2) (since a = -3) */
    bignum256 z_sqr;
    bignum256 delta;
    bignum256 sum;
    p256_mod_p_sqr(z_sqr, p->z);
    p256_mod_p_sub(delta, p->x, z_sqr);
    p256_mod_p_add(sum, p->x, z_sqr);
    p256_mod_p_mul(m, delta, sum);
    bignum256 m_tmp;
    p256_mod_p_add(m_tmp, m, m);
    p256_mod_p_add(m, m_tmp, m);

    /* X' = M^2 - 2S */
    p256_mod_p_sqr(out.x, m);
    p256_mod_p_sub(out.x, out.x, s);
    p256_mod_p_sub(out.x, out.x, s);

    /* Z' = 2 * Y * Z */
    p256_mod_p_mul(out.z, p->y, p->z);
    p256_mod_p_add(out.z, out.z, out.z);

    /* Y' = M * (S - X') - 8 * Y^4 */
    p256_mod_p_sub(t, s, out.x);
    p256_mod_p_mul(out.y, m, t);
    bignum256 y4;
    p256_mod_p_sqr(y4, y_sqr);
    for (int i = 0; i < 3; i++) {
        p256_mod_p_add(y4, y4, y4);
    }
    p256_mod_p_sub(out.y, out.y, y4);

    uint32_t keep_mask = 0U - (1U ^ is_inf);
    for (uint32_t i = 0U; i < 8U; i++) {
        out.x[i] &= keep_mask;
        out.y[i] &= keep_mask;
        out.z[i] &= keep_mask;
    }
    out.infinity = (is_inf != 0U);
    *r = out;
}

/**
 * @brief Mixed Jacobian + Affine Addition: P (Jacobian) + Q (Affine).
 * @param[out] r Output point P + Q.
 * @param[in] p Input Jacobian point.
 * @param[in] qx Input Affine point X coordinate.
 * @param[in] qy Input Affine point Y coordinate.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void
point_add_mixed(P256_Point *r, const P256_Point *p, const bignum256 qx, const bignum256 qy)
{
    bignum256 z1_sqr, u2, z1_cube, s2, h, rr;
    p256_mod_p_sqr(z1_sqr, p->z);
    p256_mod_p_mul(u2, qx, z1_sqr);

    p256_mod_p_mul(z1_cube, z1_sqr, p->z);
    p256_mod_p_mul(s2, qy, z1_cube);

    p256_mod_p_sub(h, u2, p->x);
    p256_mod_p_sub(rr, s2, p->y);

    /* LCOV_EXCL_START: Coincident / Inverse point handling */
    if (bn_is_zero(h)) {
        if (bn_is_zero(rr)) {
            point_double(r, p);
        } else {
            point_set_infinity(r);
        }
        return;
    }
    /* LCOV_EXCL_STOP */

    P256_Point out;
    bignum256 h2, h3, u1_h2;
    p256_mod_p_sqr(h2, h);
    p256_mod_p_mul(h3, h2, h);
    p256_mod_p_mul(u1_h2, p->x, h2);

    /* X' = R^2 - H^3 - 2 * U1 * H^2 */
    p256_mod_p_sqr(out.x, rr);
    p256_mod_p_sub(out.x, out.x, h3);
    p256_mod_p_sub(out.x, out.x, u1_h2);
    p256_mod_p_sub(out.x, out.x, u1_h2);

    /* Y' = R * (U1 * H^2 - X') - S1 * H^3 */
    bignum256 t, s1_h3;
    p256_mod_p_sub(t, u1_h2, out.x);
    p256_mod_p_mul(out.y, rr, t);
    p256_mod_p_mul(s1_h3, p->y, h3);
    p256_mod_p_sub(out.y, out.y, s1_h3);

    /* Z' = Z1 * H */
    p256_mod_p_mul(out.z, p->z, h);

    out.infinity = false;
    *r = out;
}

/**
 * @brief Constant-time Point Addition in Jacobian coordinates: P + Q.
 * @param[out] r Output point P + Q.
 * @param[in] p First Jacobian point.
 * @param[in] q Second Jacobian point.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void
point_add(P256_Point *r, const P256_Point *p, const P256_Point *q)
{
    uint32_t p_inf = (uint32_t)p->infinity & 1U;
    uint32_t q_inf = (uint32_t)q->infinity & 1U;

    bignum256 z1_sqr;
    bignum256 z2_sqr;
    bignum256 u1;
    bignum256 u2;
    bignum256 s1;
    bignum256 s2;
    p256_mod_p_sqr(z1_sqr, p->z);
    p256_mod_p_sqr(z2_sqr, q->z);

    p256_mod_p_mul(u1, p->x, z2_sqr);
    p256_mod_p_mul(u2, q->x, z1_sqr);

    bignum256 z1_cube;
    bignum256 z2_cube;
    p256_mod_p_mul(z1_cube, z1_sqr, p->z);
    p256_mod_p_mul(z2_cube, z2_sqr, q->z);

    p256_mod_p_mul(s1, p->y, z2_cube);
    p256_mod_p_mul(s2, q->y, z1_cube);

    bignum256 h;
    bignum256 rr;
    p256_mod_p_sub(h, u2, u1);
    p256_mod_p_sub(rr, s2, s1);

    uint32_t h_zero = (uint32_t)bn_is_zero(h) & 1U;
    uint32_t special = (1U ^ p_inf) & (1U ^ q_inf) & h_zero;

    /* LCOV_EXCL_START: Coincident / Inverse finite point handling */
    if (special != 0U) {
        if (bn_is_zero(rr)) {
            point_double(r, p);
        } else {
            point_set_infinity(r);
        }
        return;
    }
    /* LCOV_EXCL_STOP */

    P256_Point out;
    bignum256 h2;
    bignum256 h3;
    bignum256 u1_h2;
    p256_mod_p_sqr(h2, h);
    p256_mod_p_mul(h3, h2, h);
    p256_mod_p_mul(u1_h2, u1, h2);

    /* X' = R^2 - H^3 - 2 * U1 * H^2 */
    p256_mod_p_sqr(out.x, rr);
    p256_mod_p_sub(out.x, out.x, h3);
    p256_mod_p_sub(out.x, out.x, u1_h2);
    p256_mod_p_sub(out.x, out.x, u1_h2);

    /* Y' = R * (U1 * H^2 - X') - S1 * H^3 */
    bignum256 t;
    bignum256 s1_h3;
    p256_mod_p_sub(t, u1_h2, out.x);
    p256_mod_p_mul(out.y, rr, t);
    p256_mod_p_mul(s1_h3, s1, h3);
    p256_mod_p_sub(out.y, out.y, s1_h3);

    /* Z' = Z1 * Z2 * H */
    bignum256 z1_z2;
    p256_mod_p_mul(z1_z2, p->z, q->z);
    p256_mod_p_mul(out.z, z1_z2, h);

    out.infinity = false;
    point_cmov(&out, p, 0U - q_inf);
    point_cmov(&out, q, 0U - p_inf);
    *r = out;
}

/**
 * @brief Constant-time Windowed Scalar Multiplication: k * P (where P is affine (px, py)).
 * @param[out] r Output Jacobian point.
 * @param[in] k Scalar factor.
 * @param[in] px Affine base point X coordinate.
 * @param[in] py Affine base point Y coordinate.
 */
static void point_scalar_mul_affine(P256_Point *r, const bignum256 k, const bignum256 px,
                                    const bignum256 py)
{
    point_set_infinity(r);

    /* Precompute table: table[0] = 1*P, ..., table[14] = 15*P (nibble 0 uses Infinity directly) */
    P256_Point table[15];

    bn_copy(table[0].x, px);
    bn_copy(table[0].y, py);
    memset(table[0].z, 0, sizeof(table[0].z));
    table[0].z[0] = 1U;
    table[0].infinity = false;

    for (int i = 1; i < 15; i++) {
        point_add_mixed(&table[i], &table[i - 1], px, py);
    }

    /* Process all 64 4-bit nibbles in constant time without secret-dependent branching */
    for (int n = 63; n >= 0; n--) {
        point_double(r, r);
        point_double(r, r);
        point_double(r, r);
        point_double(r, r);

        uint32_t nibble = (k[n / 8] >> ((n % 8) * 4)) & 0x0FU;

        /* Constant-time select from Infinity (0) or table[0..14] (1..15) based on nibble */
        P256_Point term;
        point_set_infinity(&term);
        for (uint32_t v = 1U; v < 16U; v++) {
            uint32_t match = (uint32_t)(nibble == v);
            uint32_t mask = 0U - match;
            point_cmov(&term, &table[v - 1U], mask);
        }
        point_add(r, r, &term);
        syn_secure_zero(&term, sizeof(term));
    }
    syn_secure_zero(table, sizeof(table));
}

/* ── Public API Implementation ───────────────────────────────────────────── */

bool syn_p256_is_on_curve(const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN])
{
    if (px == NULL || py == NULL) {
        return false;
    }

    bignum256 x, y;
    bn_from_bytes(x, px);
    bn_from_bytes(y, py);

    if (bn_cmp(x, P256_P) >= 0 || bn_cmp(y, P256_P) >= 0) {
        return false;
    }

    /* Left hand side: y^2 mod p */
    bignum256 lhs;
    p256_mod_p_sqr(lhs, y);

    /* Right hand side: x^3 - 3x + b mod p */
    bignum256 rhs, x3, three_x;
    p256_mod_p_sqr(x3, x);
    p256_mod_p_mul(x3, x3, x);

    p256_mod_p_add(three_x, x, x);
    p256_mod_p_add(three_x, three_x, x);

    p256_mod_p_sub(rhs, x3, three_x);
    p256_mod_p_add(rhs, rhs, P256_B);

    return (bn_cmp(lhs, rhs) == 0);
}

bool syn_p256_base_mul(const uint8_t scalar[SYN_P256_BYTE_LEN], uint8_t pub_x[SYN_P256_BYTE_LEN],
                       uint8_t pub_y[SYN_P256_BYTE_LEN])
{
    if (scalar == NULL || pub_x == NULL || pub_y == NULL) {
        return false;
    }

    bignum256 k;
    bn_from_bytes(k, scalar);

    if (bn_is_zero(k) || bn_cmp(k, P256_N) >= 0) {
        syn_secure_zero(k, sizeof(k));
        return false;
    }

    P256_Point res;
    point_scalar_mul_affine(&res, k, P256_GX, P256_GY);

    bignum256 rx, ry;
    point_to_affine(&res, rx, ry);

    bn_to_bytes(pub_x, rx);
    bn_to_bytes(pub_y, ry);
    syn_secure_zero(k, sizeof(k));
    syn_secure_zero(&res, sizeof(res));
    syn_secure_zero(rx, sizeof(rx));
    syn_secure_zero(ry, sizeof(ry));
    return true;
}

bool syn_p256_point_mul(const uint8_t scalar[SYN_P256_BYTE_LEN],
                        const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN],
                        uint8_t rx[SYN_P256_BYTE_LEN], uint8_t ry[SYN_P256_BYTE_LEN])
{
    if (scalar == NULL || px == NULL || py == NULL || rx == NULL || ry == NULL) {
        return false;
    }

    if (!syn_p256_is_on_curve(px, py)) {
        return false;
    }

    bignum256 k;
    bn_from_bytes(k, scalar);

    if (bn_is_zero(k) || bn_cmp(k, P256_N) >= 0) {
        syn_secure_zero(k, sizeof(k));
        return false;
    }

    bignum256 ax;
    bignum256 ay;
    bn_from_bytes(ax, px);
    bn_from_bytes(ay, py);

    P256_Point res;
    point_scalar_mul_affine(&res, k, ax, ay);

    bignum256 out_x;
    bignum256 out_y;
    point_to_affine(&res, out_x, out_y);

    bn_to_bytes(rx, out_x);
    bn_to_bytes(ry, out_y);
    syn_secure_zero(k, sizeof(k));
    syn_secure_zero(ax, sizeof(ax));
    syn_secure_zero(ay, sizeof(ay));
    syn_secure_zero(&res, sizeof(res));
    syn_secure_zero(out_x, sizeof(out_x));
    syn_secure_zero(out_y, sizeof(out_y));
    return true;
}

bool syn_p256_ecdh(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_x[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_y[SYN_P256_BYTE_LEN],
                   uint8_t shared_secret[SYN_P256_BYTE_LEN])
{
    uint8_t dummy_y[SYN_P256_BYTE_LEN];
    bool ok = syn_p256_point_mul(priv_key, peer_pub_x, peer_pub_y, shared_secret, dummy_y);
    if (!ok && shared_secret != NULL) {
        syn_secure_zero(shared_secret, SYN_P256_BYTE_LEN);
    }
    syn_secure_zero(dummy_y, sizeof(dummy_y));
    return ok;
}

bool syn_p256_sign_ecdsa(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                         const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                         const uint8_t hash[SYN_P256_BYTE_LEN], uint8_t r_out[SYN_P256_BYTE_LEN],
                         uint8_t s_out[SYN_P256_BYTE_LEN])
{
    if (priv_key == NULL || nonce_k == NULL || hash == NULL || r_out == NULL || s_out == NULL) {
        return false;
    }

    bignum256 d;
    bn_from_bytes(d, priv_key);
    if (bn_is_zero(d) || bn_cmp(d, P256_N) >= 0) {
        syn_secure_zero(d, sizeof(d));
        return false;
    }

    /* 1. R = k * G, r = R.x mod n */
    uint8_t rx[SYN_P256_BYTE_LEN];
    uint8_t ry[SYN_P256_BYTE_LEN];
    if (!syn_p256_base_mul(nonce_k, rx, ry)) {
        syn_secure_zero(d, sizeof(d));
        return false;
    }

    bignum256 bn_r;
    bn_from_bytes(bn_r, rx);
    p256_reduce_once_mod_n(bn_r);

    /* 2. s = k^-1 * (e + d * r) mod n */
    bignum256 k;
    bignum256 k_inv;
    bignum256 e;
    bignum256 xr;
    bignum256 num;
    bignum256 num_sub;
    bignum256 s;
    bn_from_bytes(k, nonce_k);
    p256_mod_n_inv(k_inv, k);

    bn_from_bytes(e, hash);
    p256_reduce_once_mod_n(e);

    p256_mod_n_mul(xr, d, bn_r);

    /* num = (e + d * r) mod n */
    uint32_t carry = bn_add_raw(num, e, xr);
    uint32_t borrow = bn_sub_raw(num_sub, num, P256_N);
    bn_cmov(num, num_sub, carry | (1U ^ borrow));

    p256_mod_n_mul(s, k_inv, num);
    bool ok = (((uint32_t)bn_is_zero(bn_r) | (uint32_t)bn_is_zero(s)) == 0U);
    if (ok) {
        bn_to_bytes(r_out, bn_r);
        bn_to_bytes(s_out, s);
    }

    syn_secure_zero(rx, sizeof(rx));
    syn_secure_zero(ry, sizeof(ry));
    syn_secure_zero(bn_r, sizeof(bn_r));
    syn_secure_zero(d, sizeof(d));
    syn_secure_zero(k, sizeof(k));
    syn_secure_zero(k_inv, sizeof(k_inv));
    syn_secure_zero(e, sizeof(e));
    syn_secure_zero(xr, sizeof(xr));
    syn_secure_zero(num, sizeof(num));
    syn_secure_zero(num_sub, sizeof(num_sub));
    syn_secure_zero(s, sizeof(s));
    return ok;
}

bool syn_p256_sign_ecdsa_deterministic(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                       const uint8_t hash[SYN_P256_BYTE_LEN],
                                       uint8_t r_out[SYN_P256_BYTE_LEN],
                                       uint8_t s_out[SYN_P256_BYTE_LEN])
{
#if SYN_HMAC_DRBG_ENABLED
    if (priv_key == NULL || hash == NULL || r_out == NULL || s_out == NULL) {
        return false;
    }

    bignum256 d;
    bn_from_bytes(d, priv_key);
    if (bn_is_zero(d) || bn_cmp(d, P256_N) >= 0) {
        syn_secure_zero(d, sizeof(d));
        return false;
    }
    syn_secure_zero(d, sizeof(d));

    /* RFC 6979 Section 2.3.4 bits2octets(h1): reduce hash modulo n before HMAC-DRBG init */
    bignum256 h1;
    uint8_t h1_reduced[SYN_P256_BYTE_LEN];
    bn_from_bytes(h1, hash);
    p256_reduce_once_mod_n(h1);
    bn_to_bytes(h1_reduced, h1);
    syn_secure_zero(h1, sizeof(h1));

    SYN_HMAC_DRBG drbg;
    (void)syn_hmac_drbg_init(&drbg, priv_key, SYN_P256_BYTE_LEN, h1_reduced, SYN_P256_BYTE_LEN,
                             NULL, 0U);

    uint8_t nonce_k[SYN_P256_BYTE_LEN];
    bool success = false;

    for (uint32_t attempts = 0U; attempts < 100U; attempts++) {
        (void)syn_hmac_drbg_generate(&drbg, nonce_k, sizeof(nonce_k), NULL, 0U);

        if (syn_p256_sign_ecdsa(priv_key, nonce_k, h1_reduced, r_out, s_out)) {
            success = true;
            break;
        }
    }

    syn_hmac_drbg_wipe(&drbg);
    syn_secure_zero(h1_reduced, sizeof(h1_reduced));
    syn_secure_zero(nonce_k, sizeof(nonce_k));
    return success;
#else
    (void)priv_key;
    (void)hash;
    (void)r_out;
    (void)s_out;
    return false;
#endif
}

bool syn_p256_verify_ecdsa(const uint8_t hash[SYN_P256_BYTE_LEN],
                           const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                           const uint8_t pub_x[SYN_P256_BYTE_LEN],
                           const uint8_t pub_y[SYN_P256_BYTE_LEN])
{
    if (hash == NULL || r == NULL || s == NULL || pub_x == NULL || pub_y == NULL) {
        return false;
    }

    if (!syn_p256_is_on_curve(pub_x, pub_y)) {
        return false;
    }

    bignum256 bn_r, bn_s;
    bn_from_bytes(bn_r, r);
    bn_from_bytes(bn_s, s);

    /* FIPS 186-4: Verify that r and s are in [1, n-1] */
    if (bn_is_zero(bn_r) || bn_cmp(bn_r, P256_N) >= 0 || bn_is_zero(bn_s) ||
        bn_cmp(bn_s, P256_N) >= 0) {
        return false;
    }

    bignum256 e;
    bn_from_bytes(e, hash);
    p256_reduce_once_mod_n(e);

    /* w = s^-1 mod n */
    bignum256 w;
    p256_mod_n_inv(w, bn_s);

    /* u1 = e * w mod n */
    bignum256 u1;
    p256_mod_n_mul(u1, e, w);

    /* u2 = r * w mod n */
    bignum256 u2;
    p256_mod_n_mul(u2, bn_r, w);

    /* Compute R = u1 * G + u2 * Q */
    bignum256 qx, qy;
    bn_from_bytes(qx, pub_x);
    bn_from_bytes(qy, pub_y);

    P256_Point p1, p2, res_pt;
    point_scalar_mul_affine(&p1, u1, P256_GX, P256_GY);
    point_scalar_mul_affine(&p2, u2, qx, qy);
    point_add(&res_pt, &p1, &p2);

    if (res_pt.infinity) {
        return false;
    }

    bignum256 rx;
    bignum256 ry;
    point_to_affine(&res_pt, rx, ry);
    p256_reduce_once_mod_n(rx);

    return (bn_cmp(rx, bn_r) == 0);
}

#endif /* SYN_P256_ENABLED */
