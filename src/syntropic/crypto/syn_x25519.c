#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_X25519) || SYN_USE_X25519

/**
 * @file syn_x25519.c
 * @brief X25519 Diffie-Hellman — RFC 7748.
 *
 * Based on the TweetNaCl approach: 16 × int64_t limbs (16 bits each).
 * Simpler and more auditable than 10-limb donna at the cost of some
 * speed. The entire scalar multiplication takes ~10M multiplies.
 *
 * On Cortex-M4 @ 64 MHz this is ~2-4 seconds — acceptable since
 * handshakes only happen every ~2 minutes.
 */

#include "syn_x25519.h"
#include <string.h>

/** @brief Field element (GF(2^255 - 19)) represented by 16 × 16-bit limbs. */
typedef int64_t gf[16];

/** @brief Field element 0. */
static const gf gf0 = {0};
/** @brief Field element 1. */
static const gf gf1 = {1};
/** @brief Constant (121665) used in Montgomery arithmetic. */
static const gf _121665 = {0xDB41, 1};

/* ── Field Arithmetic (GF(2^255 - 19), 16 × 16-bit limbs) ──────────── */

/** @brief Carry-propagate across 16 limbs of a GF(2^255-19) element.
 * @param o Element to normalize.
 */
static void car25519(gf o)
{
    int64_t c;
    int i;
    for (i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        c = o[i] >> 16;
        o[(i+1) * (i < 15)] += c - 1 + 37 * (c-1) * (i == 15);
        o[i] -= c << 16;
    }
}

/** @brief Constant-time conditional swap of @p p and @p q (if b=1).
 * @param p First element.
 * @param q Second element.
 * @param b Condition (1 to swap, 0 to keep).
 */
static void sel25519(gf p, gf q, int b)
{
    int64_t t, c = ~(int64_t)(b-1);
    int i;
    for (i = 0; i < 16; i++) {
        t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

/** @brief Reduce and serialise a GF element to 32 bytes (little-endian).
 * @param o Output buffer (32 bytes).
 * @param n Field element to serialize.
 */
static void pack25519(uint8_t o[32], const gf n)
{
    int i, j;
    int64_t b;
    gf m, t;
    memcpy(t, n, sizeof(gf));
    car25519(t);
    car25519(t);
    car25519(t);
    for (j = 0; j < 2; j++) {
        m[0] = t[0] - 0xFFED;
        for (i = 1; i < 15; i++) {
            m[i] = t[i] - 0xFFFF - ((m[i-1] >> 16) & 1);
            m[i-1] &= 0xFFFF;
        }
        m[15] = t[15] - 0x7FFF - ((m[14] >> 16) & 1);
        b = (m[15] >> 16) & 1;
        m[14] &= 0xFFFF;
        sel25519(t, m, (int)(1 - b));
    }
    for (i = 0; i < 16; i++) {
        o[2*i]     = (uint8_t)(t[i] & 0xFF);
        o[2*i + 1] = (uint8_t)(t[i] >> 8);
    }
}

/** @brief Deserialise 32 bytes into a GF element.
 * @param o Output field element.
 * @param n 32-byte input buffer.
 */
static void unpack25519(gf o, const uint8_t n[32])
{
    int i;
    for (i = 0; i < 16; i++)
        o[i] = (int64_t)n[2*i] | ((int64_t)n[2*i+1] << 8);
    o[15] &= 0x7FFF;
}

/** @brief Field addition: o = a + b.
 * @param o Output element.
 * @param a First operand.
 * @param b Second operand.
 */
static void A(gf o, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; i++) o[i] = a[i] + b[i];
}

/** @brief Field subtraction: o = a - b.
 * @param o Output element.
 * @param a Minuend.
 * @param b Subtrahend.
 */
static void Z(gf o, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; i++) o[i] = a[i] - b[i];
}

/** @brief Field multiplication: o = a * b mod p.
 * @param o Output element.
 * @param a Multiplicand.
 * @param b Multiplier.
 */
static void M(gf o, const gf a, const gf b)
{
    int64_t t[31];
    int i, j;
    for (i = 0; i < 31; i++) t[i] = 0;
    for (i = 0; i < 16; i++)
        for (j = 0; j < 16; j++)
            t[i+j] += a[i] * b[j];
    for (i = 0; i < 15; i++)
        t[i] += 38 * t[i+16];
    memcpy(o, t, 16 * sizeof(int64_t));
    car25519(o);
    car25519(o);
}

/** @brief Field squaring: o = a^2 mod p.
 * @param o Output element.
 * @param a Element to square.
 */
static void S(gf o, const gf a)
{
    M(o, a, a);
}

/** @brief Field inversion: o = a^(-1) mod p via Fermat's little theorem.
 * @param o Output element.
 * @param a Element to invert.
 */
static void inv25519(gf o, const gf a)
{
    gf c;
    int i;
    memcpy(c, a, sizeof(gf));
    /* Compute a^(p-2) = a^(2^255 - 21) via repeated squaring */
    for (i = 253; i >= 0; i--) {
        S(c, c);
        if (i != 2 && i != 4)
            M(c, c, a);
    }
    memcpy(o, c, sizeof(gf));
}

/* ── X25519 scalar multiplication (Montgomery ladder) ───────────────── */

void syn_x25519(uint8_t shared_out[32],
                const uint8_t scalar[32],
                const uint8_t point[32])
{
    uint8_t z[32];
    gf x, a, b, c, d, e, f;
    int64_t r;
    int i;

    memcpy(z, scalar, 32);
    z[31] = (z[31] & 127) | 64;
    z[0] &= 248;

    unpack25519(x, point);

    memcpy(b, x, sizeof(gf));
    memcpy(a, gf1, sizeof(gf));
    memcpy(d, gf1, sizeof(gf));
    memcpy(c, gf0, sizeof(gf));

    for (i = 254; i >= 0; i--) {
        r = (z[i >> 3] >> (i & 7)) & 1;
        sel25519(a, b, (int)r);
        sel25519(c, d, (int)r);
        A(e, a, c);
        Z(a, a, c);
        A(c, b, d);
        Z(b, b, d);
        S(d, e);
        S(f, a);
        M(a, c, a);
        M(c, b, e);
        A(e, a, c);
        Z(a, a, c);
        S(b, a);
        Z(c, d, f);
        M(a, c, _121665);
        A(a, a, d);
        M(c, c, a);
        M(a, d, f);
        M(d, b, x);
        S(b, e);
        sel25519(a, b, (int)r);
        sel25519(c, d, (int)r);
    }
    inv25519(c, c);
    M(a, a, c);
    pack25519(shared_out, a);
}

void syn_x25519_pubkey(uint8_t public_out[32],
                       const uint8_t private_key[32])
{
    static const uint8_t basepoint[32] = { 9 };
    syn_x25519(public_out, private_key, basepoint);
}

#endif /* SYN_USE_X25519 */
