

# File syn\_ed25519.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.c**](syn__ed25519_8c.md)

[Go to the documentation of this file](syn__ed25519_8c.md)


```C++

#include "syn_ed25519.h"

#include <string.h>

/* ── Curve25519 Field Arithmetic (GF(2^255 - 19), 16 × 16-bit limbs) ──────── */

typedef int64_t gf[16];

static const gf gf0 = {0};
static const gf gf1 = {1};

static const gf D = {0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070,
                     0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203};

static const gf D2 = {0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0,
                      0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406};

static const gf X = {0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c,
                     0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169};

static const gf Y = {0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666,
                     0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666};

static const gf I = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43,
                     0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83};

static void car25519(gf o)
{
    int i;
    for (i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        int64_t c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c * 65536LL;
    }
}

static void sel25519(gf p, gf q, int b)
{
    int64_t t, c = ~(int64_t)(b - 1);
    for (int i = 0; i < 16; i++) {
        t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void pack25519(uint8_t o[32], const gf n)
{
    int i, j;
    int64_t b;
    gf m, t;
    for (i = 0; i < 16; i++) {
        t[i] = n[i];
    }
    car25519(t);
    car25519(t);
    car25519(t);
    for (j = 0; j < 2; j++) {
        m[0] = t[0] - 0xFFED;
        for (i = 1; i < 15; i++) {
            m[i] = t[i] - 0xFFFF - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xFFFF;
        }
        m[15] = t[15] - 0x7FFF - ((m[14] >> 16) & 1);
        b = (m[15] >> 16) & 1;
        m[14] &= 0xFFFF;
        sel25519(t, m, (int)(1 - b));
    }
    for (i = 0; i < 16; i++) {
        uint16_t v = (uint16_t)t[i];
        o[2 * i] = (uint8_t)(v & 0xFFU);
        o[2 * i + 1] = (uint8_t)((v >> 8) & 0xFFU);
    }
}

static int crypto_verify_32(const uint8_t *x, const uint8_t *y)
{
    uint32_t d = 0;
    for (int i = 0; i < 32; i++) {
        d |= (uint32_t)(x[i] ^ y[i]);
    }
    return (int)((1U & ((d - 1U) >> 8)) - 1U);
}

static int neq25519(const gf a, const gf b)
{
    uint8_t c[32], d[32];
    pack25519(c, a);
    pack25519(d, b);
    return crypto_verify_32(c, d);
}

static uint8_t par25519(const gf a)
{
    uint8_t d[32];
    pack25519(d, a);
    return (uint8_t)(d[0] & 1U);
}

static void unpack25519(gf o, const uint8_t n[32])
{
    for (int i = 0; i < 16; i++) {
        uint16_t v = (uint16_t)n[2 * i] | ((uint16_t)n[2 * i + 1] << 8);
        o[i] = (int64_t)v;
    }
    o[15] &= 0x7FFF;
}

static void A(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; i++) {
        o[i] = a[i] + b[i];
    }
}

static void Z(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; i++) {
        o[i] = a[i] - b[i];
    }
}

static void M(gf o, const gf a, const gf b)
{
    int64_t t[31];
    int i, j;
    for (i = 0; i < 31; i++) {
        t[i] = 0;
    }
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            t[i + j] += a[i] * b[j];
        }
    }
    for (i = 0; i < 15; i++) {
        t[i] += 38 * t[i + 16];
    }
    for (i = 0; i < 16; i++) {
        o[i] = t[i];
    }
    car25519(o);
    car25519(o);
}

static void S(gf o, const gf a)
{
    M(o, a, a);
}

static void inv25519(gf o, const gf i_val)
{
    gf c;
    int a;
    for (a = 0; a < 16; a++) {
        c[a] = i_val[a];
    }
    for (a = 253; a >= 0; a--) {
        S(c, c);
        if (a != 2 && a != 4) {
            M(c, c, i_val);
        }
    }
    for (a = 0; a < 16; a++) {
        o[a] = c[a];
    }
}

static void pow2523(gf o, const gf i_val)
{
    gf c;
    int a;
    for (a = 0; a < 16; a++) {
        c[a] = i_val[a];
    }
    for (a = 250; a >= 0; a--) {
        S(c, c);
        if (a != 1) {
            M(c, c, i_val);
        }
    }
    for (a = 0; a < 16; a++) {
        o[a] = c[a];
    }
}

static void set25519(gf r, const gf a)
{
    for (int i = 0; i < 16; i++) {
        r[i] = a[i];
    }
}

/* ── Group Operations on Edwards25519 ─────────────────────────────────────── */

static void add(gf p[4], gf q[4])
{
    gf a, b, c, d_pt, t, e, f, g, h;

    Z(a, p[1], p[0]);
    Z(t, q[1], q[0]);
    M(a, a, t);
    A(b, p[0], p[1]);
    A(t, q[0], q[1]);
    M(b, b, t);
    M(c, p[3], q[3]);
    M(c, c, D2);
    M(d_pt, p[2], q[2]);
    A(d_pt, d_pt, d_pt);
    Z(e, b, a);
    Z(f, d_pt, c);
    A(g, d_pt, c);
    A(h, b, a);

    M(p[0], e, f);
    M(p[1], h, g);
    M(p[2], g, f);
    M(p[3], e, h);
}

static void cswap(gf p[4], gf q[4], uint8_t b)
{
    for (int i = 0; i < 4; i++) {
        sel25519(p[i], q[i], (int)b);
    }
}

static void pack(uint8_t *r, gf p[4])
{
    gf tx, ty, zi;
    inv25519(zi, p[2]);
    M(tx, p[0], zi);
    M(ty, p[1], zi);
    pack25519(r, ty);
    r[31] ^= (uint8_t)(par25519(tx) << 7);
}

static void scalarmult(gf p[4], gf q[4], const uint8_t *s)
{
    set25519(p[0], gf0);
    set25519(p[1], gf1);
    set25519(p[2], gf1);
    set25519(p[3], gf0);
    for (int i = 255; i >= 0; --i) {
        uint8_t b = (uint8_t)((s[i / 8] >> (i & 7)) & 1);
        cswap(p, q, b);
        add(q, p);
        add(p, p);
        cswap(p, q, b);
    }
}

static void scalarbase(gf p[4], const uint8_t *s)
{
    gf q[4];
    set25519(q[0], X);
    set25519(q[1], Y);
    set25519(q[2], gf1);
    M(q[3], X, Y);
    scalarmult(p, q, s);
}

static int unpackneg(gf r[4], const uint8_t p[32])
{
    gf t, chk, num, den, den2, den4, den6;
    set25519(r[2], gf1);
    unpack25519(r[1], p);
    S(num, r[1]);
    M(den, num, D);
    Z(num, num, r[2]);
    A(den, r[2], den);

    S(den2, den);
    S(den4, den2);
    M(den6, den4, den2);
    M(t, den6, num);
    M(t, t, den);

    pow2523(t, t);
    M(t, t, num);
    M(t, t, den);
    M(t, t, den);
    M(r[0], t, den);

    S(chk, r[0]);
    M(chk, chk, den);
    if (neq25519(chk, num) != 0) {
        M(r[0], r[0], I);
    }

    S(chk, r[0]);
    M(chk, chk, den);
    if (neq25519(chk, num) != 0) {
        return -1;
    }

    if (par25519(r[0]) == (uint8_t)(p[31] >> 7)) {
        Z(r[0], gf0, r[0]);
    }

    M(r[3], r[0], r[1]);
    return 0;
}

/* ── Scalar Arithmetic Modulo L (RFC 8032 Section 5.1) ───────────────────── */

static const int64_t ORDER_L[32] = {
    0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0x10};

static void modL(uint8_t *r, int64_t x[64])
{
    int64_t carry, i, j;
    for (i = 63; i >= 32; --i) {
        carry = 0;
        for (j = i - 32; j < i - 12; ++j) {
            x[j] += carry - 16 * x[i] * ORDER_L[j - (i - 32)];
            carry = (x[j] + 128) >> 8;
            x[j] -= carry * 256;
        }
        x[j] += carry;
        x[i] = 0;
    }
    carry = 0;
    for (j = 0; j < 32; j++) {
        x[j] += carry - (x[31] >> 4) * ORDER_L[j];
        carry = x[j] >> 8;
        x[j] &= 255;
    }
    for (j = 0; j < 32; j++) {
        x[j] -= carry * ORDER_L[j];
    }
    for (i = 0; i < 32; i++) {
        x[i + 1] += x[i] >> 8;
        r[i] = (uint8_t)(x[i] & 255);
    }
}

static void reduce(uint8_t *r)
{
    int64_t x[64];
    for (int i = 0; i < 64; i++) {
        x[i] = (int64_t)r[i];
    }
    for (int i = 0; i < 64; i++) {
        r[i] = 0;
    }
    modL(r, x);
}

/* ── Public API ──────────────────────────────────────────────────────────── */

bool syn_ed25519_publickey(const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (secret_key == NULL || public_key == NULL) {
        return false;
    }

    uint8_t d[64];
    gf p[4];

    SYN_SHA512_Ctx hash_ctx;
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, secret_key, SYN_ED25519_SECRET_KEY_SIZE);
    syn_sha512_final(&hash_ctx, d);

    d[0] &= 248U;
    d[31] &= 127U;
    d[31] |= 64U;

    scalarbase(p, d);
    pack(public_key, p);

    volatile uint8_t *vp = (volatile uint8_t *)d;
    for (size_t i = 0; i < sizeof(d); i++) {
        vp[i] = 0U;
    }
    return true;
}

bool syn_ed25519_create_keypair(uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                                uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                                const uint8_t seed[SYN_ED25519_SEED_SIZE])
{
    if (public_key == NULL || secret_key == NULL || seed == NULL) {
        return false;
    }

    (void)memcpy(secret_key, seed, SYN_ED25519_SECRET_KEY_SIZE);
    return syn_ed25519_publickey(secret_key, public_key);
}

bool syn_ed25519_sign(const uint8_t *msg, size_t msg_len,
                      const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                      const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE],
                      uint8_t sig[SYN_ED25519_SIGNATURE_SIZE])
{
    if ((msg == NULL && msg_len > 0U) || secret_key == NULL || sig == NULL) {
        return false;
    }

    uint8_t pk_buf[SYN_ED25519_PUBLIC_KEY_SIZE];
    const uint8_t *pk = public_key;
    if (pk == NULL) {
        (void)syn_ed25519_publickey(secret_key, pk_buf);
        pk = pk_buf;
    }

    uint8_t d[64], r[64];
    int64_t x[64];
    gf p[4];

    SYN_SHA512_Ctx hash_ctx;
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, secret_key, SYN_ED25519_SECRET_KEY_SIZE);
    syn_sha512_final(&hash_ctx, d);

    d[0] &= 248U;
    d[31] &= 127U;
    d[31] |= 64U;

    /* Nonce r = SHA-512(d[32..63] || msg) */
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, d + 32, 32U);
    if (msg != NULL && msg_len > 0U) {
        syn_sha512_update(&hash_ctx, msg, msg_len);
    }
    syn_sha512_final(&hash_ctx, r);
    reduce(r);

    /* R = r * B */
    scalarbase(p, r);
    pack(sig, p);

    /* Challenge h = SHA-512(R || pk || msg) */
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, sig, 32U);
    syn_sha512_update(&hash_ctx, pk, 32U);
    if (msg != NULL && msg_len > 0U) {
        syn_sha512_update(&hash_ctx, msg, msg_len);
    }
    uint8_t h[64];
    syn_sha512_final(&hash_ctx, h);
    reduce(h);

    /* S = (r + h * d) mod L */
    for (int i = 0; i < 64; i++) {
        x[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        x[i] = (int64_t)r[i];
    }
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            x[i + j] += (int64_t)h[i] * (int64_t)d[j];
        }
    }
    modL(sig + 32, x);

    volatile uint8_t *vp = (volatile uint8_t *)d;
    for (size_t i = 0; i < sizeof(d); i++) {
        vp[i] = 0U;
    }
    vp = (volatile uint8_t *)r;
    for (size_t i = 0; i < sizeof(r); i++) {
        vp[i] = 0U;
    }
    return true;
}

bool syn_ed25519_verify_hash(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t h[64],
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (sig == NULL || h == NULL || public_key == NULL) {
        return false;
    }

    if ((sig[63] & 224U) != 0U) {
        return false;
    }

    gf p[4], q[4];
    if (unpackneg(q, public_key) != 0) {
        return false;
    }

    uint8_t h_reduced[64];
    for (size_t i = 0; i < 64U; i++) {
        h_reduced[i] = h[i];
    }
    reduce(h_reduced);

    scalarmult(p, q, h_reduced);
    scalarbase(q, sig + 32);
    add(p, q);

    uint8_t t[32];
    pack(t, p);

    return (crypto_verify_32(sig, t) == 0);
}

bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (sig == NULL || (msg == NULL && msg_len > 0U) || public_key == NULL) {
        return false;
    }

    /* Challenge h = SHA-512(R || pk || msg) */
    SYN_SHA512_Ctx hash_ctx;
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, sig, 32U);
    syn_sha512_update(&hash_ctx, public_key, 32U);
    if (msg != NULL && msg_len > 0U) {
        syn_sha512_update(&hash_ctx, msg, msg_len);
    }
    uint8_t h[64];
    syn_sha512_final(&hash_ctx, h);

    return syn_ed25519_verify_hash(sig, h, public_key);
}
```


