

# File syn\_p256.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_p256.c**](syn__p256_8c.md)

[Go to the documentation of this file](syn__p256_8c.md)


```C++

#include "syn_p256.h"

#if !defined(SYN_USE_HMAC_DRBG) || SYN_USE_HMAC_DRBG
#include "syn_hmac_drbg.h"
#endif

#include <string.h>

/* NIST P-256 Curve Constants (Little-Endian 32-bit Words: limbs[0] is least significant) */

static const uint32_t P256_P[8] = {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U,
                                   0x00000000U, 0x00000000U, 0x00000001U, 0xFFFFFFFFU};

static const uint32_t P256_N[8] = {0xFC632551U, 0xF3B9CAC2U, 0xA7179E84U, 0xBCE6FAADU,
                                   0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0xFFFFFFFFU};

static const uint32_t P256_B[8] = {0x27D2604BU, 0x3BCE3C3EU, 0xCC53B0F6U, 0x651D06B0U,
                                   0x769886BCU, 0xB3EBBD55U, 0xAA3A93E7U, 0x5AC635D8U};

static const uint32_t P256_GX[8] = {0xD898C296U, 0xF4A13945U, 0x2DEB33A0U, 0x77037D81U,
                                    0x63A440F2U, 0xF8BCE6E5U, 0xE12C4247U, 0x6B17D1F2U};

static const uint32_t P256_GY[8] = {0x37BF51F5U, 0xCBB64068U, 0x6B315ECEU, 0x2BCE3357U,
                                    0x7C0F9E16U, 0x8EE7EB4AU, 0xFE1A7F9BU, 0x4FE342E2U};

static const uint32_t BN_ONE[8] = {1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

/* ── 256-bit Big-Number Utilities ────────────────────────────────────────── */

typedef uint32_t bignum256[8];

static void bn_from_bytes(bignum256 r, const uint8_t bytes[32])
{
    for (int i = 0; i < 8; i++) {
        int idx = (7 - i) * 4;
        r[i] = ((uint32_t)bytes[idx + 0] << 24U) | ((uint32_t)bytes[idx + 1] << 16U) |
               ((uint32_t)bytes[idx + 2] << 8U) | ((uint32_t)bytes[idx + 3]);
    }
}

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

static bool bn_is_zero(const bignum256 a)
{
    for (int i = 0; i < 8; i++) {
        if (a[i] != 0U)
            return false;
    }
    return true;
}

static void bn_copy(bignum256 r, const bignum256 a)
{
    memcpy(r, a, sizeof(bignum256));
}

/* ── Field Arithmetic Modulo p ─────────────────────────────────────────── */

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

static void p256_mod_p_add(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t carry = bn_add_raw(r, a, b);
    if (carry || bn_cmp(r, P256_P) >= 0) {
        bn_sub_raw(r, r, P256_P);
    }
}

static void p256_mod_p_sub(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t borrow = bn_sub_raw(r, a, b);
    if (borrow) {
        bn_add_raw(r, r, P256_P);
    }
}

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

static void p256_mod_p_mul(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t t[16];
    bn_mul_raw(t, a, b);
    p256_mod_p_reduce(r, t);
}

static void p256_mod_p_sqr(bignum256 r, const bignum256 a)
{
    p256_mod_p_mul(r, a, a);
}

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
}

/* ── Group Order Arithmetic Modulo n ────────────────────────────────────── */

static void p256_mod_n_mul(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t t[16];
    bn_mul_raw(t, a, b);

    bignum256 rem = {0};
    for (int bit = 511; bit >= 0; bit--) {
        uint32_t carry_bit = (t[bit / 32] >> (bit % 32)) & 1U;
        for (int i = 0; i < 8; i++) {
            uint32_t next_carry = rem[i] >> 31U;
            rem[i] = (rem[i] << 1U) | carry_bit;
            carry_bit = next_carry;
        }
        if (carry_bit || bn_cmp(rem, P256_N) >= 0) {
            bn_sub_raw(rem, rem, P256_N);
        }
    }
    bn_copy(r, rem);
}

static void p256_mod_n_sub(bignum256 r, const bignum256 a, const bignum256 b)
{
    uint32_t borrow = bn_sub_raw(r, a, b);
    if (borrow != 0U) {
        bn_add_raw(r, r, P256_N);
    }
}

static void bn_rshift1(bignum256 r, const bignum256 a)
{
    uint32_t carry = 0U;
    for (int i = 7; i >= 0; i--) {
        uint32_t next_carry = a[i] & 1U;
        r[i] = (a[i] >> 1U) | (carry << 31U);
        carry = next_carry;
    }
}

static void bn_div2_mod_n(bignum256 r, const bignum256 a)
{
    if ((a[0] & 1U) != 0U) {
        bignum256 sum;
        uint32_t carry = bn_add_raw(sum, a, P256_N);
        for (int i = 7; i >= 0; i--) {
            uint32_t next_carry = sum[i] & 1U;
            r[i] = (sum[i] >> 1U) | (carry << 31U);
            carry = next_carry;
        }
    } else {
        bn_rshift1(r, a);
    }
}

static void p256_mod_n_inv(bignum256 r, const bignum256 a)
{
    /* LCOV_EXCL_START: Identity and zero shortcuts */
    if (bn_is_zero(a)) {
        memset(r, 0, sizeof(bignum256));
        return;
    }
    if (bn_cmp(a, BN_ONE) == 0) {
        bn_copy(r, BN_ONE);
        return;
    }
    /* LCOV_EXCL_STOP */

    bignum256 u, v, x1, x2;
    bn_copy(u, a);
    bn_copy(v, P256_N);
    memset(x1, 0, sizeof(x1));
    x1[0] = 1U;
    memset(x2, 0, sizeof(x2));

    while (!bn_is_zero(u)) {
        while ((u[0] & 1U) == 0U) {
            bn_rshift1(u, u);
            bn_div2_mod_n(x1, x1);
        }
        while ((v[0] & 1U) == 0U) {
            bn_rshift1(v, v);
            bn_div2_mod_n(x2, x2);
        }
        if (bn_cmp(u, v) >= 0) {
            bn_sub_raw(u, u, v);
            p256_mod_n_sub(x1, x1, x2);
        } else {
            bn_sub_raw(v, v, u);
            p256_mod_n_sub(x2, x2, x1);
        }
    }
    bn_copy(r, x2);
}

/* ── Jacobian Point Operations ───────────────────────────────────────────── */

typedef struct {
    bignum256 x;   
    bignum256 y;   
    bignum256 z;   
    bool infinity; 
} P256_Point;

static void point_set_infinity(P256_Point *p)
{
    memset(p, 0, sizeof(P256_Point));
    p->infinity = true;
}

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

    bignum256 z_inv, z_inv2, z_inv3;
    p256_mod_p_inv(z_inv, p->z);
    p256_mod_p_sqr(z_inv2, z_inv);
    p256_mod_p_mul(z_inv3, z_inv2, z_inv);
    p256_mod_p_mul(x_out, p->x, z_inv2);
    p256_mod_p_mul(y_out, p->y, z_inv3);
}

static void point_double(P256_Point *r, const P256_Point *p)
{
    /* LCOV_EXCL_START: Point at infinity guard */
    if (p->infinity || bn_is_zero(p->y)) {
        point_set_infinity(r);
        return;
    }
    /* LCOV_EXCL_STOP */

    P256_Point out;
    bignum256 s, m, t, y_sqr;
    p256_mod_p_sqr(y_sqr, p->y);

    /* S = 4 * X * Y^2 */
    p256_mod_p_mul(s, p->x, y_sqr);
    p256_mod_p_add(s, s, s);
    p256_mod_p_add(s, s, s);

    /* M = 3 * (X^2 - Z^4) = 3 * (X - Z^2) * (X + Z^2) (since a = -3) */
    bignum256 z_sqr, delta, sum;
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

    out.infinity = false;
    *r = out;
}

static void point_add_mixed(P256_Point *r, const P256_Point *p, const bignum256 qx,
                            const bignum256 qy)
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

static void point_add(P256_Point *r, const P256_Point *p, const P256_Point *q)
{
    if (p->infinity) {
        *r = *q;
        return;
    }
    if (q->infinity) {
        *r = *p;
        return;
    }

    bignum256 z1_sqr, z2_sqr, u1, u2, s1, s2;
    p256_mod_p_sqr(z1_sqr, p->z);
    p256_mod_p_sqr(z2_sqr, q->z);

    p256_mod_p_mul(u1, p->x, z2_sqr);
    p256_mod_p_mul(u2, q->x, z1_sqr);

    bignum256 z1_cube, z2_cube;
    p256_mod_p_mul(z1_cube, z1_sqr, p->z);
    p256_mod_p_mul(z2_cube, z2_sqr, q->z);

    p256_mod_p_mul(s1, p->y, z2_cube);
    p256_mod_p_mul(s2, q->y, z1_cube);

    bignum256 h, rr;
    p256_mod_p_sub(h, u2, u1);
    p256_mod_p_sub(rr, s2, s1);

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
    p256_mod_p_mul(u1_h2, u1, h2);

    /* X' = R^2 - H^3 - 2 * U1 * H^2 */
    p256_mod_p_sqr(out.x, rr);
    p256_mod_p_sub(out.x, out.x, h3);
    p256_mod_p_sub(out.x, out.x, u1_h2);
    p256_mod_p_sub(out.x, out.x, u1_h2);

    /* Y' = R * (U1 * H^2 - X') - S1 * H^3 */
    bignum256 t, s1_h3;
    p256_mod_p_sub(t, u1_h2, out.x);
    p256_mod_p_mul(out.y, rr, t);
    p256_mod_p_mul(s1_h3, s1, h3);
    p256_mod_p_sub(out.y, out.y, s1_h3);

    /* Z' = Z1 * Z2 * H */
    bignum256 z1_z2;
    p256_mod_p_mul(z1_z2, p->z, q->z);
    p256_mod_p_mul(out.z, z1_z2, h);

    out.infinity = false;
    *r = out;
}

static void point_cmov(P256_Point *dst, const P256_Point *src, uint32_t mask)
{
    for (int i = 0; i < 8; i++) {
        dst->x[i] = (dst->x[i] & ~mask) | (src->x[i] & mask);
        dst->y[i] = (dst->y[i] & ~mask) | (src->y[i] & mask);
        dst->z[i] = (dst->z[i] & ~mask) | (src->z[i] & mask);
    }
    dst->infinity = (((uint32_t)dst->infinity & ~mask) | ((uint32_t)src->infinity & mask)) != 0;
}

static void point_scalar_mul_affine(P256_Point *r, const bignum256 k, const bignum256 px,
                                    const bignum256 py)
{
    point_set_infinity(r);

    /* Precompute table: table[0] = Infinity, table[1] = 1*P, ..., table[15] = 15*P */
    P256_Point table[16];
    point_set_infinity(&table[0]);

    bn_copy(table[1].x, px);
    bn_copy(table[1].y, py);
    memset(table[1].z, 0, sizeof(table[1].z));
    table[1].z[0] = 1U;
    table[1].infinity = false;

    for (int i = 2; i < 16; i++) {
        point_add_mixed(&table[i], &table[i - 1], px, py);
    }

    /* Process all 64 4-bit nibbles in constant time without secret-dependent branching */
    for (int n = 63; n >= 0; n--) {
        point_double(r, r);
        point_double(r, r);
        point_double(r, r);
        point_double(r, r);

        uint32_t nibble = (k[n / 8] >> ((n % 8) * 4)) & 0x0FU;

        /* Constant-time select from table[0..15] based on nibble */
        P256_Point term;
        point_set_infinity(&term);
        for (uint32_t v = 0; v < 16; v++) {
            uint32_t match = (uint32_t)(nibble == v);
            uint32_t mask = 0U - match;
            point_cmov(&term, &table[v], mask);
        }
        point_add(r, r, &term);
    }
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
        return false;
    }

    P256_Point res;
    point_scalar_mul_affine(&res, k, P256_GX, P256_GY);

    bignum256 rx, ry;
    point_to_affine(&res, rx, ry);

    bn_to_bytes(pub_x, rx);
    bn_to_bytes(pub_y, ry);
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
        return false;
    }

    bignum256 ax, ay;
    bn_from_bytes(ax, px);
    bn_from_bytes(ay, py);

    P256_Point res;
    point_scalar_mul_affine(&res, k, ax, ay);

    bignum256 out_x, out_y;
    point_to_affine(&res, out_x, out_y);

    bn_to_bytes(rx, out_x);
    bn_to_bytes(ry, out_y);
    return true;
}

bool syn_p256_ecdh(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_x[SYN_P256_BYTE_LEN],
                   const uint8_t peer_pub_y[SYN_P256_BYTE_LEN],
                   uint8_t shared_secret[SYN_P256_BYTE_LEN])
{
    uint8_t dummy_y[SYN_P256_BYTE_LEN];
    return syn_p256_point_mul(priv_key, peer_pub_x, peer_pub_y, shared_secret, dummy_y);
}

bool syn_p256_sign_ecdsa(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                         const uint8_t nonce_k[SYN_P256_BYTE_LEN],
                         const uint8_t hash[SYN_P256_BYTE_LEN], uint8_t r_out[SYN_P256_BYTE_LEN],
                         uint8_t s_out[SYN_P256_BYTE_LEN])
{
    if (priv_key == NULL || nonce_k == NULL || hash == NULL || r_out == NULL || s_out == NULL) {
        return false;
    }

    /* 1. R = k * G */
    uint8_t rx[SYN_P256_BYTE_LEN], ry[SYN_P256_BYTE_LEN];
    if (!syn_p256_base_mul(nonce_k, rx, ry)) {
        return false;
    }

    bignum256 bn_r;
    bn_from_bytes(bn_r, rx);

    /* 2. s = k^-1 * (e + d * r) mod n */
    bignum256 k, k_inv, d, e, xr, num, s;
    bn_from_bytes(k, nonce_k);
    p256_mod_n_inv(k_inv, k);

    bn_from_bytes(d, priv_key);
    bn_from_bytes(e, hash);
    while (bn_cmp(e, P256_N) >= 0) {
        bn_sub_raw(e, e, P256_N);
    }

    p256_mod_n_mul(xr, d, bn_r);

    /* num = e + d*r mod n */
    uint32_t carry = bn_add_raw(num, e, xr);
    if (carry || bn_cmp(num, P256_N) >= 0) {
        bn_sub_raw(num, num, P256_N);
    }

    p256_mod_n_mul(s, k_inv, num);
    if (bn_is_zero(s)) {
        return false;
    }

    bn_to_bytes(r_out, bn_r);
    bn_to_bytes(s_out, s);
    return true;
}

bool syn_p256_sign_ecdsa_deterministic(const uint8_t priv_key[SYN_P256_BYTE_LEN],
                                       const uint8_t hash[SYN_P256_BYTE_LEN],
                                       uint8_t r_out[SYN_P256_BYTE_LEN],
                                       uint8_t s_out[SYN_P256_BYTE_LEN])
{
#if !defined(SYN_USE_HMAC_DRBG) || SYN_USE_HMAC_DRBG
    if (priv_key == NULL || hash == NULL || r_out == NULL || s_out == NULL) {
        return false;
    }

    SYN_HMAC_DRBG drbg;
    (void)syn_hmac_drbg_init(&drbg, priv_key, SYN_P256_BYTE_LEN, hash, SYN_P256_BYTE_LEN, NULL, 0U);

    uint8_t nonce_k[SYN_P256_BYTE_LEN];
    bool success = false;

    for (uint32_t attempts = 0U; attempts < 100U; attempts++) {
        (void)syn_hmac_drbg_generate(&drbg, nonce_k, sizeof(nonce_k), NULL, 0U);

        if (syn_p256_sign_ecdsa(priv_key, nonce_k, hash, r_out, s_out)) {
            success = true;
            break;
        }
    }

    syn_hmac_drbg_wipe(&drbg);
    volatile uint8_t *vp = (volatile uint8_t *)nonce_k;
    for (size_t i = 0U; i < sizeof(nonce_k); i++) {
        vp[i] = 0U;
    }
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
    while (bn_cmp(e, P256_N) >= 0) {
        bn_sub_raw(e, e, P256_N);
    }

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

    bignum256 rx, ry;
    point_to_affine(&res_pt, rx, ry);

    return (bn_cmp(rx, bn_r) == 0);
}
```


