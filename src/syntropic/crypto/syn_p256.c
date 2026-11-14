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

/** @brief Field prime p - 2 for constant-time Fermat modular inversion */
static const uint32_t P256_P_MINUS_2[8] = {0xFFFFFFFDU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U,
                                           0x00000000U, 0x00000000U, 0x00000001U, 0xFFFFFFFFU};

/** @brief Group order n */
static const uint32_t P256_N[8] = {0xFC632551U, 0xF3B9CAC2U, 0xA7179E84U, 0xBCE6FAADU,
                                   0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0xFFFFFFFFU};

/** @brief Group order n - 2 for constant-time Fermat modular inversion */
static const uint32_t P256_N_MINUS_2[8] = {0xFC63254FU, 0xF3B9CAC2U, 0xA7179E84U, 0xBCE6FAADU,
                                           0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0xFFFFFFFFU};

/** @brief Montgomery constant -n[0]^-1 mod 2^32 for CIOS reduction modulo n */
#define P256_N0_INV 0xEE00BC4FU

/** @brief R^2 mod n (R = 2^256) for converting into Montgomery domain modulo n */
static const uint32_t P256_R2_MOD_N[8] = {0xBE79EEA2U, 0x83244C95U, 0x49BD6FA6U, 0x4699799CU,
                                          0x2B6BEC59U, 0x2845B239U, 0xF3D95620U, 0x66E12D94U};

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
        if (a[i] > b[i]) {
            return 1;
        }
        if (a[i] < b[i]) {
            return -1;
        }
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
 * @brief Process one 4-bit slice of Fermat inversion modulo p: a^(p-2) mod p.
 * @param[in,out] res    Running product modulo p.
 * @param[in,out] base   Running square base modulo p.
 * @param[in]     nib_ix Exponent nibble index in [0, 63].
 */
static void p256_inv_p_nibble(bignum256 res, bignum256 base, uint16_t nib_ix)
{
    uint32_t limb = (uint32_t)nib_ix >> 3U;
    uint32_t shift = ((uint32_t)nib_ix & 7U) * 4U;
    uint32_t w = (P256_P_MINUS_2[limb] >> shift) & 0xFU;
    for (uint32_t b = 0U; b < 4U; b++) {
        if ((w & 1U) != 0U) {
            p256_mod_p_mul(res, res, base);
        }
        p256_mod_p_sqr(base, base);
        w >>= 1U;
    }
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
 * @brief Constant-time 8-limb Montgomery multiplication modulo n: r = a * b * R^-1 mod n.
 * @param[out] r Montgomery product modulo n.
 * @param[in]  a First operand (< n).
 * @param[in]  b Second operand (< n).
 */
static void p256_mont_mul_n(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t t[17];
    memset(t, 0, sizeof(t));
    bn_mul_raw(t, a, b);

    for (int i = 0; i < 8; i++) {
        uint32_t m = t[i] * P256_N0_INV;
        uint64_t carry = 0ULL;
        for (int j = 0; j < 8; j++) {
            uint64_t prod = (uint64_t)m * (uint64_t)P256_N[j] + (uint64_t)t[i + j] + carry;
            t[i + j] = (uint32_t)prod;
            carry = prod >> 32U;
        }
        for (int k = i + 8; carry > 0ULL && k < 17; k++) {
            uint64_t sum = (uint64_t)t[k] + carry;
            t[k] = (uint32_t)sum;
            carry = sum >> 32U;
        }
    }

    bignum256 cand;
    bignum256 diff;
    for (int i = 0; i < 8; i++) {
        cand[i] = t[i + 8];
    }
    uint32_t borrow = bn_sub_raw(diff, cand, P256_N);
    bn_cmov(cand, diff, t[16] | (1U ^ borrow));
    bn_copy(r, cand);
    syn_secure_zero(t, sizeof(t));
    syn_secure_zero(cand, sizeof(cand));
    syn_secure_zero(diff, sizeof(diff));
}

/**
 * @brief Group order multiplication modulo n: r = a * b mod n.
 * @param[out] r Product modulo n.
 * @param[in]  a First factor (< n).
 * @param[in]  b Second factor (< n).
 */
static void p256_mod_n_mul(bignum256 r, const bignum256 a, const bignum256 b)
{
    bignum256 a_mont;
    p256_mont_mul_n(a_mont, a, P256_R2_MOD_N);
    p256_mont_mul_n(r, a_mont, b);
    syn_secure_zero(a_mont, sizeof(a_mont));
}

/**
 * @brief Initialize Montgomery-domain Fermat inversion state for a^(n-2) mod n.
 * @param[out] res  Initialized to R mod n.
 * @param[out] base Initialized to a * R mod n.
 * @param[in]  a    Input value to invert (< n).
 */
static void p256_inv_n_start(bignum256 res, bignum256 base, const bignum256 a)
{
    p256_mont_mul_n(res, BN_ONE, P256_R2_MOD_N);
    p256_mont_mul_n(base, a, P256_R2_MOD_N);
}

/**
 * @brief Process one 4-bit slice of Montgomery-domain Fermat inversion modulo n.
 * @param[in,out] res    Running Montgomery product.
 * @param[in,out] base   Running Montgomery square base.
 * @param[in]     nib_ix Exponent nibble index in [0, 63].
 */
static void p256_inv_n_nibble(bignum256 res, bignum256 base, uint16_t nib_ix)
{
    uint32_t limb = (uint32_t)nib_ix >> 3U;
    uint32_t shift = ((uint32_t)nib_ix & 7U) * 4U;
    uint32_t w = (P256_N_MINUS_2[limb] >> shift) & 0xFU;
    for (uint32_t b = 0U; b < 4U; b++) {
        if ((w & 1U) != 0U) {
            p256_mont_mul_n(res, res, base);
        }
        p256_mont_mul_n(base, base, base);
        w >>= 1U;
    }
}

/* ── Jacobian Point Operations ───────────────────────────────────────────── */

/** @brief Internal alias for Jacobian projective point type. */
typedef SYN_P256_Point P256_Point;

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
    bignum256 z1_sqr;
    bignum256 u2;
    bignum256 z1_cube;
    bignum256 s2;
    bignum256 h;
    bignum256 rr;
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
    bignum256 h2;
    bignum256 h3;
    bignum256 u1_h2;
    p256_mod_p_sqr(h2, h);
    p256_mod_p_mul(h3, h2, h);
    p256_mod_p_mul(u1_h2, p->x, h2);

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

/* ── Resumable Cooperative Engine ────────────────────────────────────────── */

/** @cond INTERNAL */
typedef enum { P256_OP_MUL = 1U, P256_OP_SIGN = 2U, P256_OP_VERIFY = 3U } P256_OpKind;

static bool p256_pt_args_ok(const SYN_PT *pt, const SYN_P256_Ctx *ctx, uint8_t expected_op)
{
    return (pt != NULL) && (ctx != NULL) && ctx->active && (ctx->op == expected_op);
}

static bool p256_scalar_in_range(const bignum256 k)
{
    return !bn_is_zero(k) && (bn_cmp(k, P256_N) < 0);
}

static SYN_PT_Status p256_precomp_w2_pt(SYN_PT *pt, SYN_P256_Ctx *ctx)
{
    PT_BEGIN(pt);

    bn_copy(ctx->table[0].x, ctx->px);
    bn_copy(ctx->table[0].y, ctx->py);
    bn_copy(ctx->table[0].z, BN_ONE);
    ctx->table[0].infinity = false;
    point_double(&ctx->table[1], &ctx->table[0]);
    PT_CONTINUE(pt);

    point_add_mixed(&ctx->table[2], &ctx->table[1], ctx->px, ctx->py);
    point_set_infinity(&ctx->acc);
    PT_CONTINUE(pt);

    PT_END(pt);
}

static void p256_scalar_w2_add(SYN_P256_Ctx *ctx)
{
    uint32_t win = 127U - (uint32_t)ctx->idx;
    uint32_t bits = (ctx->k1[win / 16U] >> ((win % 16U) * 2U)) & 0x03U;
    P256_Point term;
    point_set_infinity(&term);
    for (uint32_t v = 1U; v <= 3U; v++) {
        uint32_t mask = 0U - (uint32_t)(bits == v);
        point_cmov(&term, &ctx->table[v - 1U], mask);
    }
    point_add(&ctx->acc, &ctx->acc, &term);
    syn_secure_zero(&term, sizeof(term));
}

static SYN_PT_Status p256_scalar_w2_pt(SYN_PT *pt, SYN_P256_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 128U; ctx->idx++) {
        point_double(&ctx->acc, &ctx->acc);
        point_double(&ctx->acc, &ctx->acc);
        PT_CONTINUE(pt);

        p256_scalar_w2_add(ctx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status p256_inv_z_pt(SYN_PT *pt, SYN_P256_Ctx *ctx)
{
    PT_BEGIN(pt);
    bn_copy(ctx->inv_res, BN_ONE);
    bn_copy(ctx->inv_base, ctx->acc.z);
    for (ctx->idx = 0U; ctx->idx < 64U; ctx->idx++) {
        p256_inv_p_nibble(ctx->inv_res, ctx->inv_base, ctx->idx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status p256_inv_n_pt(SYN_PT *pt, SYN_P256_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 64U; ctx->idx++) {
        p256_inv_n_nibble(ctx->inv_res, ctx->inv_base, ctx->idx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void p256_affine_from_inv_z(const SYN_P256_Ctx *ctx, bignum256 out_x, bignum256 out_y)
{
    bignum256 z_inv2;
    bignum256 z_inv3;
    p256_mod_p_sqr(z_inv2, ctx->inv_res);
    p256_mod_p_mul(z_inv3, z_inv2, ctx->inv_res);
    p256_mod_p_mul(out_x, ctx->acc.x, z_inv2);
    p256_mod_p_mul(out_y, ctx->acc.y, z_inv3);
    syn_secure_zero(z_inv2, sizeof(z_inv2));
    syn_secure_zero(z_inv3, sizeof(z_inv3));
}

static void p256_mul_finish(const SYN_P256_Ctx *ctx, uint8_t rx[SYN_P256_BYTE_LEN],
                            uint8_t ry[SYN_P256_BYTE_LEN])
{
    bignum256 out_x;
    bignum256 out_y;
    p256_affine_from_inv_z(ctx, out_x, out_y);
    bn_to_bytes(rx, out_x);
    if (ry != NULL) {
        bn_to_bytes(ry, out_y);
    }
    syn_secure_zero(out_x, sizeof(out_x));
    syn_secure_zero(out_y, sizeof(out_y));
}

static void p256_drbg_rfc6979_round(SYN_P256_Ctx *ctx, uint8_t sep)
{
#if SYN_HMAC_DRBG_ENABLED
    uint8_t priv_bytes[SYN_P256_BYTE_LEN];
    uint8_t h1_reduced[SYN_P256_BYTE_LEN];
    SYN_HMAC_SHA256 hmac;

    bn_to_bytes(priv_bytes, ctx->k2);
    bn_to_bytes(h1_reduced, ctx->aux);

    if (sep == 0x00U) {
        (void)memset(ctx->drbg.k, 0x00, sizeof(ctx->drbg.k));
        (void)memset(ctx->drbg.v, 0x01, sizeof(ctx->drbg.v));
        ctx->drbg.reseed_interval = SYN_HMAC_DRBG_DEFAULT_RESEED_INTERVAL;
    }

    syn_hmac_sha256_init(&hmac, ctx->drbg.k, sizeof(ctx->drbg.k));
    syn_hmac_sha256_update(&hmac, ctx->drbg.v, sizeof(ctx->drbg.v));
    syn_hmac_sha256_update(&hmac, &sep, 1U);
    syn_hmac_sha256_update(&hmac, priv_bytes, SYN_P256_BYTE_LEN);
    syn_hmac_sha256_update(&hmac, h1_reduced, SYN_P256_BYTE_LEN);
    syn_hmac_sha256_final(&hmac, ctx->drbg.k);
    syn_secure_zero(&hmac, sizeof(hmac));
    syn_secure_zero(priv_bytes, sizeof(priv_bytes));
    syn_secure_zero(h1_reduced, sizeof(h1_reduced));

    syn_hmac_sha256(ctx->drbg.k, sizeof(ctx->drbg.k), ctx->drbg.v, sizeof(ctx->drbg.v),
                    ctx->drbg.v);

    if (sep == 0x01U) {
        ctx->drbg.reseed_counter = 1U;
        ctx->drbg.initialized = true;
    }
#else
    (void)ctx;
    (void)sep;
#endif
}

static void p256_sign_drbg_draw(SYN_P256_Ctx *ctx)
{
#if SYN_HMAC_DRBG_ENABLED
    uint8_t nonce_bytes[SYN_P256_BYTE_LEN];
    (void)syn_hmac_drbg_generate(&ctx->drbg, nonce_bytes, sizeof(nonce_bytes), NULL, 0U);
    bn_from_bytes(ctx->k1, nonce_bytes);
    syn_secure_zero(nonce_bytes, sizeof(nonce_bytes));
#else
    (void)ctx;
#endif
}

static void p256_sign_prep_inv_n(SYN_P256_Ctx *ctx)
{
    bignum256 rx;
    bignum256 ry;
    p256_affine_from_inv_z(ctx, rx, ry);
    p256_reduce_once_mod_n(rx);
    bn_copy(ctx->py, rx);
    p256_inv_n_start(ctx->inv_res, ctx->inv_base, ctx->k1);
    syn_secure_zero(rx, sizeof(rx));
    syn_secure_zero(ry, sizeof(ry));
}

static void p256_sign_rs_num(SYN_P256_Ctx *ctx)
{
    bignum256 xr;
    bignum256 num_sub;
    p256_mod_n_mul(xr, ctx->k2, ctx->py);
    uint32_t carry = bn_add_raw(ctx->inv_base, ctx->aux, xr);
    uint32_t borrow = bn_sub_raw(num_sub, ctx->inv_base, P256_N);
    bn_cmov(ctx->inv_base, num_sub, carry | (1U ^ borrow));
    syn_secure_zero(xr, sizeof(xr));
    syn_secure_zero(num_sub, sizeof(num_sub));
}

static bool p256_sign_rs_fin(const SYN_P256_Ctx *ctx, uint8_t r_out[SYN_P256_BYTE_LEN],
                             uint8_t s_out[SYN_P256_BYTE_LEN])
{
    bignum256 k_inv;
    bignum256 s;
    p256_mont_mul_n(k_inv, ctx->inv_res, BN_ONE);
    p256_mod_n_mul(s, k_inv, ctx->inv_base);

    bool ok = (((uint32_t)bn_is_zero(ctx->py) | (uint32_t)bn_is_zero(s)) == 0U);
    if (ok) {
        bn_to_bytes(r_out, ctx->py);
        bn_to_bytes(s_out, s);
    }
    syn_secure_zero(k_inv, sizeof(k_inv));
    syn_secure_zero(s, sizeof(s));
    return ok;
}

static void p256_verify_shamir_scalars(SYN_P256_Ctx *ctx)
{
    bignum256 w;
    p256_mont_mul_n(w, ctx->inv_res, BN_ONE);
    p256_mod_n_mul(ctx->k1, ctx->k1, w);
    p256_mod_n_mul(ctx->k2, ctx->aux, w);
    syn_secure_zero(w, sizeof(w));
}

static void p256_verify_shamir_table(SYN_P256_Ctx *ctx)
{
    bn_copy(ctx->table[0].x, P256_GX);
    bn_copy(ctx->table[0].y, P256_GY);
    bn_copy(ctx->table[0].z, BN_ONE);
    ctx->table[0].infinity = false;

    bn_copy(ctx->table[1].x, ctx->px);
    bn_copy(ctx->table[1].y, ctx->py);
    bn_copy(ctx->table[1].z, BN_ONE);
    ctx->table[1].infinity = false;

    point_add(&ctx->table[2], &ctx->table[0], &ctx->table[1]);
    point_set_infinity(&ctx->acc);
}

static void p256_verify_shamir_add(SYN_P256_Ctx *ctx)
{
    uint32_t bit = 255U - (uint32_t)ctx->idx;
    uint32_t b1 = (ctx->k1[bit / 32U] >> (bit % 32U)) & 1U;
    uint32_t b2 = (ctx->k2[bit / 32U] >> (bit % 32U)) & 1U;
    uint32_t sel = b1 | (b2 << 1U);
    if (sel != 0U) {
        point_add(&ctx->acc, &ctx->acc, &ctx->table[sel - 1U]);
    }
}

static SYN_PT_Status p256_verify_shamir_pt(SYN_PT *pt, SYN_P256_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 256U; ctx->idx++) {
        point_double(&ctx->acc, &ctx->acc);
        PT_CONTINUE(pt);

        p256_verify_shamir_add(ctx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static bool p256_verify_check_r(const SYN_P256_Ctx *ctx)
{
    bignum256 rx;
    bignum256 ry;
    p256_affine_from_inv_z(ctx, rx, ry);
    p256_reduce_once_mod_n(rx);
    bool valid = (bn_cmp(rx, ctx->aux) == 0);
    syn_secure_zero(rx, sizeof(rx));
    syn_secure_zero(ry, sizeof(ry));
    return valid;
}
/** @endcond */

/* ── Public API Implementation ───────────────────────────────────────────── */

void syn_p256_wipe(SYN_P256_Ctx *ctx)
{
    if (ctx != NULL) {
        syn_secure_zero(ctx, sizeof(*ctx));
    }
}

bool syn_p256_is_on_curve(const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN])
{
    if (px == NULL || py == NULL) {
        return false;
    }

    bignum256 x;
    bignum256 y;
    bn_from_bytes(x, px);
    bn_from_bytes(y, py);

    if (bn_cmp(x, P256_P) >= 0 || bn_cmp(y, P256_P) >= 0) {
        return false;
    }

    /* Left hand side: y^2 mod p */
    bignum256 lhs;
    p256_mod_p_sqr(lhs, y);

    /* Right hand side: x^3 - 3x + b mod p */
    bignum256 rhs;
    bignum256 x3;
    bignum256 three_x;
    p256_mod_p_sqr(x3, x);
    p256_mod_p_mul(x3, x3, x);

    p256_mod_p_add(three_x, x, x);
    p256_mod_p_add(three_x, three_x, x);

    p256_mod_p_sub(rhs, x3, three_x);
    p256_mod_p_add(rhs, rhs, P256_B);

    return (bn_cmp(lhs, rhs) == 0);
}

bool syn_p256_base_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN])
{
    if (ctx == NULL || scalar == NULL) {
        return false;
    }
    syn_p256_wipe(ctx);
    bn_from_bytes(ctx->k1, scalar);
    if (!p256_scalar_in_range(ctx->k1)) {
        syn_p256_wipe(ctx);
        return false;
    }
    bn_copy(ctx->px, P256_GX);
    bn_copy(ctx->py, P256_GY);
    ctx->op = (uint8_t)P256_OP_MUL;
    ctx->active = true;
    return true;
}

bool syn_p256_point_mul_init(SYN_P256_Ctx *ctx, const uint8_t scalar[SYN_P256_BYTE_LEN],
                             const uint8_t px[SYN_P256_BYTE_LEN],
                             const uint8_t py[SYN_P256_BYTE_LEN])
{
    if (ctx == NULL || scalar == NULL || px == NULL || py == NULL ||
        !syn_p256_is_on_curve(px, py)) {
        return false;
    }
    syn_p256_wipe(ctx);
    bn_from_bytes(ctx->k1, scalar);
    if (!p256_scalar_in_range(ctx->k1)) {
        syn_p256_wipe(ctx);
        return false;
    }
    bn_from_bytes(ctx->px, px);
    bn_from_bytes(ctx->py, py);
    ctx->op = (uint8_t)P256_OP_MUL;
    ctx->active = true;
    return true;
}

SYN_PT_Status syn_p256_mul_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t rx[SYN_P256_BYTE_LEN],
                              uint8_t ry[SYN_P256_BYTE_LEN])
{
    if (!p256_pt_args_ok(pt, ctx, (uint8_t)P256_OP_MUL) || rx == NULL) {
        if (ctx != NULL) {
            syn_p256_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, p256_precomp_w2_pt(&ctx->sub_pt, ctx));
    PT_SPAWN(pt, &ctx->sub_pt, p256_scalar_w2_pt(&ctx->sub_pt, ctx));
    PT_SPAWN(pt, &ctx->sub_pt, p256_inv_z_pt(&ctx->sub_pt, ctx));
    p256_mul_finish(ctx, rx, ry);
    syn_p256_wipe(ctx);

    PT_END(pt);
}

bool syn_p256_sign_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                        const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                        const uint8_t hash[SYN_P256_BYTE_LEN])
{
    if (ctx == NULL || priv_key == NULL || nonce_k == NULL || hash == NULL) {
        return false;
    }
    syn_p256_wipe(ctx);
    bn_from_bytes(ctx->k2, priv_key);
    if (!p256_scalar_in_range(ctx->k2)) {
        syn_p256_wipe(ctx);
        return false;
    }
    bn_from_bytes(ctx->k1, nonce_k);
    if (!p256_scalar_in_range(ctx->k1)) {
        syn_p256_wipe(ctx);
        return false;
    }
    bn_from_bytes(ctx->aux, hash);
    p256_reduce_once_mod_n(ctx->aux);
    ctx->op = (uint8_t)P256_OP_SIGN;
    ctx->max_attempts = 1U;
    ctx->use_drbg = false;
    ctx->active = true;
    return true;
}

bool syn_p256_sign_deterministic_init(SYN_P256_Ctx *ctx, const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                      const uint8_t hash[SYN_P256_BYTE_LEN])
{
#if SYN_HMAC_DRBG_ENABLED
    if (ctx == NULL || priv_key == NULL || hash == NULL) {
        return false;
    }
    syn_p256_wipe(ctx);
    bn_from_bytes(ctx->k2, priv_key);
    if (!p256_scalar_in_range(ctx->k2)) {
        syn_p256_wipe(ctx);
        return false;
    }

    bn_from_bytes(ctx->aux, hash);
    p256_reduce_once_mod_n(ctx->aux);

    ctx->op = (uint8_t)P256_OP_SIGN;
    ctx->max_attempts = 100U;
    ctx->use_drbg = true;
    ctx->active = true;
    return true;
#else
    (void)ctx;
    (void)priv_key;
    (void)hash;
    return false;
#endif
}

SYN_PT_Status syn_p256_sign_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, uint8_t r_out[SYN_P256_BYTE_LEN],
                               uint8_t s_out[SYN_P256_BYTE_LEN], bool *out_ok)
{
    if (out_ok != NULL) {
        *out_ok = false;
    }
    if (!p256_pt_args_ok(pt, ctx, (uint8_t)P256_OP_SIGN) || r_out == NULL || s_out == NULL ||
        out_ok == NULL) {
        if (ctx != NULL) {
            syn_p256_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    if (ctx->use_drbg) {
        p256_drbg_rfc6979_round(ctx, 0x00U);
        PT_CONTINUE(pt);
        p256_drbg_rfc6979_round(ctx, 0x01U);
        PT_CONTINUE(pt);
    }

    while (ctx->attempts < ctx->max_attempts) {
        ctx->attempts++;
        if (ctx->use_drbg) {
            p256_sign_drbg_draw(ctx);
            PT_CONTINUE(pt);
        }
        if (!p256_scalar_in_range(ctx->k1)) {
            continue;
        }

        bn_copy(ctx->px, P256_GX);
        bn_copy(ctx->py, P256_GY);
        PT_SPAWN(pt, &ctx->sub_pt, p256_precomp_w2_pt(&ctx->sub_pt, ctx));
        PT_SPAWN(pt, &ctx->sub_pt, p256_scalar_w2_pt(&ctx->sub_pt, ctx));
        PT_SPAWN(pt, &ctx->sub_pt, p256_inv_z_pt(&ctx->sub_pt, ctx));

        p256_sign_prep_inv_n(ctx);
        PT_CONTINUE(pt);

        PT_SPAWN(pt, &ctx->sub_pt, p256_inv_n_pt(&ctx->sub_pt, ctx));
        p256_sign_rs_num(ctx);
        PT_CONTINUE(pt);
        if (p256_sign_rs_fin(ctx, r_out, s_out)) {
            ctx->ok = true;
            break;
        }
    }

    if (out_ok != NULL) {
        *out_ok = ctx->ok;
    }
    syn_p256_wipe(ctx);

    PT_END(pt);
}

bool syn_p256_verify_init(SYN_P256_Ctx *ctx, const uint8_t hash[SYN_P256_BYTE_LEN],
                          const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                          const uint8_t pub_x[SYN_P256_BYTE_LEN],
                          const uint8_t pub_y[SYN_P256_BYTE_LEN])
{
    if (ctx == NULL || hash == NULL || r == NULL || s == NULL || pub_x == NULL || pub_y == NULL ||
        !syn_p256_is_on_curve(pub_x, pub_y)) {
        return false;
    }
    syn_p256_wipe(ctx);
    bignum256 bn_s;
    bn_from_bytes(ctx->aux, r);
    bn_from_bytes(bn_s, s);
    if (!p256_scalar_in_range(ctx->aux) || !p256_scalar_in_range(bn_s)) {
        syn_secure_zero(bn_s, sizeof(bn_s));
        syn_p256_wipe(ctx);
        return false;
    }

    bn_from_bytes(ctx->k1, hash);
    p256_reduce_once_mod_n(ctx->k1);
    bn_from_bytes(ctx->px, pub_x);
    bn_from_bytes(ctx->py, pub_y);
    p256_inv_n_start(ctx->inv_res, ctx->inv_base, bn_s);
    syn_secure_zero(bn_s, sizeof(bn_s));

    ctx->op = (uint8_t)P256_OP_VERIFY;
    ctx->active = true;
    return true;
}

SYN_PT_Status syn_p256_verify_pt(SYN_PT *pt, SYN_P256_Ctx *ctx, bool *out_valid)
{
    if (out_valid != NULL) {
        *out_valid = false;
    }
    if (!p256_pt_args_ok(pt, ctx, (uint8_t)P256_OP_VERIFY) || out_valid == NULL) {
        if (ctx != NULL) {
            syn_p256_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, p256_inv_n_pt(&ctx->sub_pt, ctx));

    p256_verify_shamir_scalars(ctx);
    PT_CONTINUE(pt);

    p256_verify_shamir_table(ctx);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, p256_verify_shamir_pt(&ctx->sub_pt, ctx));

    if (ctx->acc.infinity) {
        *out_valid = false;
    } else {
        PT_SPAWN(pt, &ctx->sub_pt, p256_inv_z_pt(&ctx->sub_pt, ctx));
        *out_valid = p256_verify_check_r(ctx);
    }
    syn_p256_wipe(ctx);

    PT_END(pt);
}

bool syn_p256_base_mul(const uint8_t scalar[SYN_P256_BYTE_LEN], uint8_t pub_x[SYN_P256_BYTE_LEN],
                       uint8_t pub_y[SYN_P256_BYTE_LEN])
{
    if (pub_x == NULL || pub_y == NULL) {
        return false;
    }
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    if (!syn_p256_base_mul_init(&ctx, scalar)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_mul_pt(&pt, &ctx, pub_x, pub_y) == PT_MORE) {
    }
    return true;
}

bool syn_p256_point_mul(const uint8_t scalar[SYN_P256_BYTE_LEN],
                        const uint8_t px[SYN_P256_BYTE_LEN], const uint8_t py[SYN_P256_BYTE_LEN],
                        uint8_t rx[SYN_P256_BYTE_LEN], uint8_t ry[SYN_P256_BYTE_LEN])
{
    if (rx == NULL || ry == NULL) {
        return false;
    }
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    if (!syn_p256_point_mul_init(&ctx, scalar, px, py)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_mul_pt(&pt, &ctx, rx, ry) == PT_MORE) {
    }
    return true;
}

bool syn_p256_ecdh(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_x[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_y[SYN_P256_BYTE_LEN],
                   uint8_t shared_secret[SYN_P256_BYTE_LEN])
{
    if (shared_secret == NULL) {
        return false;
    }
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    if (!syn_p256_point_mul_init(&ctx, priv_key, peer_pub_x, peer_pub_y)) {
        syn_secure_zero(shared_secret, SYN_P256_BYTE_LEN);
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_mul_pt(&pt, &ctx, shared_secret, NULL) == PT_MORE) {
    }
    return true;
}

bool syn_p256_sign_ecdsa(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                         const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                         const uint8_t hash[SYN_P256_BYTE_LEN], uint8_t r_out[SYN_P256_BYTE_LEN],
                         uint8_t s_out[SYN_P256_BYTE_LEN])
{
    if (r_out == NULL || s_out == NULL) {
        return false;
    }
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    bool ok = false;
    if (!syn_p256_sign_init(&ctx, priv_key, nonce_k, hash)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_sign_pt(&pt, &ctx, r_out, s_out, &ok) == PT_MORE) {
    }
    return ok;
}

bool syn_p256_sign_ecdsa_deterministic(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                       const uint8_t hash[SYN_P256_BYTE_LEN],
                                       uint8_t r_out[SYN_P256_BYTE_LEN],
                                       uint8_t s_out[SYN_P256_BYTE_LEN])
{
    if (r_out == NULL || s_out == NULL) {
        return false;
    }
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    bool ok = false;
    if (!syn_p256_sign_deterministic_init(&ctx, priv_key, hash)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_sign_pt(&pt, &ctx, r_out, s_out, &ok) == PT_MORE) {
    }
    return ok;
}

bool syn_p256_verify_ecdsa(const uint8_t hash[SYN_P256_BYTE_LEN],
                           const uint8_t r[SYN_P256_BYTE_LEN], const uint8_t s[SYN_P256_BYTE_LEN],
                           const uint8_t pub_x[SYN_P256_BYTE_LEN],
                           const uint8_t pub_y[SYN_P256_BYTE_LEN])
{
    SYN_P256_Ctx ctx;
    SYN_PT pt;
    bool valid = false;
    if (!syn_p256_verify_init(&ctx, hash, r, s, pub_x, pub_y)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_p256_verify_pt(&pt, &ctx, &valid) == PT_MORE) {
    }
    return valid;
}

#endif /* SYN_P256_ENABLED */
