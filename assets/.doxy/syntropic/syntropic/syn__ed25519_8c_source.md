

# File syn\_ed25519.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.c**](syn__ed25519_8c.md)

[Go to the documentation of this file](syn__ed25519_8c.md)


```C++

#include "syntropic/crypto/syn_ed25519.h"

#include <string.h>

/* ── SHA-512 Implementation (RFC 6234) ──────────────────────────────────── */

typedef struct {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t buffer[128];
} SYN_SHA512_Ctx;

/* LCOV_EXCL_START */
static void sha512_init(SYN_SHA512_Ctx *ctx)
{
    ctx->state[0] = 0x6a09e667f3bcc908ULL;
    ctx->state[1] = 0xbb67ae8584caa73bULL;
    ctx->state[2] = 0x3c6ef372fe94f82bULL;
    ctx->state[3] = 0xa54ff53a5f1d36f1ULL;
    ctx->state[4] = 0x510e527fea941957ULL;
    ctx->state[5] = 0x9b05688c2b3e6c1fULL;
    ctx->state[6] = 0x1f83d9abfb41bd6bULL;
    ctx->state[7] = 0x5be0cd19137e2179ULL;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

#define ROR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0_512(x) (ROR64(x, 28) ^ ROR64(x, 34) ^ ROR64(x, 39))
#define Sigma1_512(x) (ROR64(x, 14) ^ ROR64(x, 18) ^ ROR64(x, 41))
#define sigma0_512(x) (ROR64(x, 1) ^ ROR64(x, 8) ^ ((x) >> 7))
#define sigma1_512(x) (ROR64(x, 19) ^ ROR64(x, 61) ^ ((x) >> 6))

static const uint64_t K512[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edd465ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeef9ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cedc65b38ULL, 0x5c6858056211d423ULL, 0x6e00bf334a076391ULL};

static void sha512_transform(uint64_t state[8], const uint8_t block[128])
{
    uint64_t W[80];
    for (int i = 0; i < 16; i++) {
        W[i] = ((uint64_t)block[i * 8] << 56) | ((uint64_t)block[i * 8 + 1] << 48) |
               ((uint64_t)block[i * 8 + 2] << 40) | ((uint64_t)block[i * 8 + 3] << 32) |
               ((uint64_t)block[i * 8 + 4] << 24) | ((uint64_t)block[i * 8 + 5] << 16) |
               ((uint64_t)block[i * 8 + 6] << 8) | ((uint64_t)block[i * 8 + 7]);
    }
    for (int i = 16; i < 80; i++) {
        W[i] = sigma1_512(W[i - 2]) + W[i - 7] + sigma0_512(W[i - 15]) + W[i - 16];
    }

    uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 80; i++) {
        uint64_t T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[i] + W[i];
        uint64_t T2 = Sigma0_512(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

static void sha512_update(SYN_SHA512_Ctx *ctx, const uint8_t *data, size_t len)
{
    size_t index = (size_t)((ctx->count[0] >> 3) & 0x7F);
    ctx->count[0] += (uint64_t)len << 3;
    if (ctx->count[0] < ((uint64_t)len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += (uint64_t)len >> 61;

    size_t part_len = 128 - index;
    size_t i = 0;
    if (len >= part_len) {
        memcpy(&ctx->buffer[index], data, part_len);
        sha512_transform(ctx->state, ctx->buffer);
        for (i = part_len; i + 127 < len; i += 128) {
            sha512_transform(ctx->state, &data[i]);
        }
        index = 0;
    }
    memcpy(&ctx->buffer[index], &data[i], len - i);
}

static void sha512_final(SYN_SHA512_Ctx *ctx, uint8_t digest[64])
{
    uint8_t bits[16];
    for (int i = 0; i < 8; i++) {
        bits[i] = (uint8_t)((ctx->count[1] >> (56 - i * 8)) & 0xFF);
        bits[i + 8] = (uint8_t)((ctx->count[0] >> (56 - i * 8)) & 0xFF);
    }

    size_t index = (size_t)((ctx->count[0] >> 3) & 0x7F);
    size_t pad_len = (index < 112) ? (112 - index) : (240 - index);

    static const uint8_t padding[128] = {0x80};
    sha512_update(ctx, padding, pad_len);
    sha512_update(ctx, bits, 16);

    for (int i = 0; i < 8; i++) {
        digest[i * 8] = (uint8_t)((ctx->state[i] >> 56) & 0xFF);
        digest[i * 8 + 1] = (uint8_t)((ctx->state[i] >> 48) & 0xFF);
        digest[i * 8 + 2] = (uint8_t)((ctx->state[i] >> 40) & 0xFF);
        digest[i * 8 + 3] = (uint8_t)((ctx->state[i] >> 32) & 0xFF);
        digest[i * 8 + 4] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
        digest[i * 8 + 5] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
        digest[i * 8 + 6] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
        digest[i * 8 + 7] = (uint8_t)(ctx->state[i] & 0xFF);
    }
}

/* sha512_hash removed (unused helper) */

/* ── Curve25519 & Ed25519 Math Helpers ───────────────────────────────────── */

typedef int64_t fe[10];

static void fe_zero(fe h)
{
    memset(h, 0, sizeof(fe));
}

static void fe_one(fe h)
{
    memset(h, 0, sizeof(fe));
    h[0] = 1;
}

static void fe_copy(fe h, const fe f)
{
    memcpy(h, f, sizeof(fe));
}

static void fe_frombytes(fe h, const uint8_t *s)
{
    int64_t h0 = s[0] | ((int64_t)s[1] << 8) | ((int64_t)s[2] << 16) | ((int64_t)(s[3] & 3) << 24);
    int64_t h1 = (s[3] >> 2) | ((int64_t)s[4] << 6) | ((int64_t)s[5] << 14) | ((int64_t)s[6] << 22);
    int64_t h2 =
        (s[6] >> 2) | ((int64_t)s[7] << 6) | ((int64_t)s[8] << 14) | ((int64_t)(s[9] & 15) << 22);
    int64_t h3 =
        (s[9] >> 4) | ((int64_t)s[10] << 4) | ((int64_t)s[11] << 12) | ((int64_t)s[12] << 20);
    int64_t h4 = (s[12] >> 4) | ((int64_t)s[13] << 4) | ((int64_t)s[14] << 12) |
                 ((int64_t)(s[15] & 63) << 20);
    int64_t h5 =
        (s[15] >> 6) | ((int64_t)s[16] << 2) | ((int64_t)s[17] << 10) | ((int64_t)s[18] << 18);
    int64_t h6 = (s[18] >> 6) | ((int64_t)s[19] << 2) | ((int64_t)s[20] << 10) |
                 ((int64_t)(s[21] & 127) << 18);
    int64_t h7 =
        (s[21] >> 7) | ((int64_t)s[22] << 1) | ((int64_t)s[23] << 9) | ((int64_t)s[24] << 17);
    int64_t h8 = (s[24] >> 7) | ((int64_t)s[25] << 1) | ((int64_t)s[26] << 9) |
                 ((int64_t)(s[27] & 31) << 17);
    int64_t h9 = (s[27] >> 5) | ((int64_t)s[28] << 3) | ((int64_t)s[29] << 11) |
                 ((int64_t)s[30] << 19) | ((int64_t)s[31] << 27);

    h[0] = h0;
    h[1] = h1;
    h[2] = h2;
    h[3] = h3;
    h[4] = h4;
    h[5] = h5;
    h[6] = h6;
    h[7] = h7;
    h[8] = h8;
    h[9] = h9;
}

static void fe_tobytes(uint8_t *s, const fe h)
{
    int64_t t[10];
    memcpy(t, h, sizeof(t));

    int64_t q = (t[0] + (1 << 25)) >> 26;
    t[1] += q;
    t[0] -= q * 67108864;
    q = (t[1] + (1 << 24)) >> 25;
    t[2] += q;
    t[1] -= q * 33554432;
    q = (t[2] + (1 << 25)) >> 26;
    t[3] += q;
    t[2] -= q * 67108864;
    q = (t[3] + (1 << 24)) >> 25;
    t[4] += q;
    t[3] -= q * 33554432;
    q = (t[4] + (1 << 25)) >> 26;
    t[5] += q;
    t[4] -= q * 67108864;
    q = (t[5] + (1 << 24)) >> 25;
    t[6] += q;
    t[5] -= q * 33554432;
    q = (t[6] + (1 << 25)) >> 26;
    t[7] += q;
    t[6] -= q * 67108864;
    q = (t[7] + (1 << 24)) >> 25;
    t[8] += q;
    t[7] -= q * 33554432;
    q = (t[8] + (1 << 25)) >> 26;
    t[9] += q;
    t[8] -= q * 67108864;
    q = (t[9] + (1 << 24)) >> 25;
    t[0] += q * 19;
    t[9] -= q * 33554432;

    q = (t[0] + (1 << 25)) >> 26;
    t[1] += q;
    t[0] -= q * 67108864;

    uint64_t u0 = (uint64_t)t[0];
    uint64_t u1 = (uint64_t)t[1];
    uint64_t u2 = (uint64_t)t[2];
    uint64_t u3 = (uint64_t)t[3];
    uint64_t u4 = (uint64_t)t[4];
    uint64_t u5 = (uint64_t)t[5];
    uint64_t u6 = (uint64_t)t[6];
    uint64_t u7 = (uint64_t)t[7];
    uint64_t u8 = (uint64_t)t[8];
    uint64_t u9 = (uint64_t)t[9];

    s[0] = (uint8_t)u0;
    s[1] = (uint8_t)(u0 >> 8);
    s[2] = (uint8_t)(u0 >> 16);
    s[3] = (uint8_t)((u0 >> 24) | (u1 << 2));
    s[4] = (uint8_t)(u1 >> 6);
    s[5] = (uint8_t)(u1 >> 14);
    s[6] = (uint8_t)((u1 >> 22) | (u2 << 2));
    s[7] = (uint8_t)(u2 >> 6);
    s[8] = (uint8_t)(u2 >> 14);
    s[9] = (uint8_t)((u2 >> 22) | (u3 << 4));
    s[10] = (uint8_t)(u3 >> 4);
    s[11] = (uint8_t)(u3 >> 12);
    s[12] = (uint8_t)((u3 >> 20) | (u4 << 4));
    s[13] = (uint8_t)(u4 >> 4);
    s[14] = (uint8_t)(u4 >> 12);
    s[15] = (uint8_t)((u4 >> 20) | (u5 << 6));
    s[16] = (uint8_t)(u5 >> 2);
    s[17] = (uint8_t)(u5 >> 10);
    s[18] = (uint8_t)((u5 >> 18) | (u6 << 2));
    s[19] = (uint8_t)(u6 >> 6);
    s[20] = (uint8_t)(u6 >> 14);
    s[21] = (uint8_t)((u6 >> 22) | (u7 << 1));
    s[22] = (uint8_t)(u7 >> 7);
    s[23] = (uint8_t)(u7 >> 15);
    s[24] = (uint8_t)((u7 >> 23) | (u8 << 1));
    s[25] = (uint8_t)(u8 >> 7);
    s[26] = (uint8_t)(u8 >> 15);
    s[27] = (uint8_t)((u8 >> 23) | (u9 << 3));
    s[28] = (uint8_t)(u9 >> 5);
    s[29] = (uint8_t)(u9 >> 13);
    s[30] = (uint8_t)(u9 >> 21);
    s[31] = (uint8_t)(u9 >> 29);
}

static void fe_add(fe h, const fe f, const fe g)
{
    for (int i = 0; i < 10; i++) {
        h[i] = (int64_t)((uint64_t)f[i] + (uint64_t)g[i]);
    }
}

static void fe_sub(fe h, const fe f, const fe g)
{
    for (int i = 0; i < 10; i++) {
        h[i] = (int64_t)((uint64_t)f[i] - (uint64_t)g[i]);
    }
}

static void fe_mul(fe h, const fe f, const fe g)
{
#if defined(__SIZEOF_INT128__)
    typedef __int128_t i128;
#else
    typedef int64_t i128;
#endif

    i128 f0 = f[0], f1 = f[1], f2 = f[2], f3 = f[3], f4 = f[4];
    i128 f5 = f[5], f6 = f[6], f7 = f[7], f8 = f[8], f9 = f[9];
    i128 g0 = g[0], g1 = g[1], g2 = g[2], g3 = g[3], g4 = g[4];
    i128 g5 = g[5], g6 = g[6], g7 = g[7], g8 = g[8], g9 = g[9];

    i128 g1_19 = 19 * g1, g2_19 = 19 * g2, g3_19 = 19 * g3, g4_19 = 19 * g4;
    i128 g5_19 = 19 * g5, g6_19 = 19 * g6, g7_19 = 19 * g7, g8_19 = 19 * g8, g9_19 = 19 * g9;

    i128 h0 = f0 * g0 + f1 * g9_19 + f2 * g8_19 + f3 * g7_19 + f4 * g6_19 + f5 * g5_19 +
              f6 * g4_19 + f7 * g3_19 + f8 * g2_19 + f9 * g1_19;
    i128 h1 = f0 * g1 + f1 * g0 + f2 * g9_19 + f3 * g8_19 + f4 * g7_19 + f5 * g6_19 + f6 * g5_19 +
              f7 * g4_19 + f8 * g3_19 + f9 * g2_19;
    i128 h2 = f0 * g2 + f1 * g1 + f2 * g0 + f3 * g9_19 + f4 * g8_19 + f5 * g7_19 + f6 * g6_19 +
              f7 * g5_19 + f8 * g4_19 + f9 * g3_19;
    i128 h3 = f0 * g3 + f1 * g2 + f2 * g1 + f3 * g0 + f4 * g9_19 + f5 * g8_19 + f6 * g7_19 +
              f7 * g6_19 + f8 * g5_19 + f9 * g4_19;
    i128 h4 = f0 * g4 + f1 * g3 + f2 * g2 + f3 * g1 + f4 * g0 + f5 * g9_19 + f6 * g8_19 +
              f7 * g7_19 + f8 * g6_19 + f9 * g5_19;
    i128 h5 = f0 * g5 + f1 * g4 + f2 * g3 + f3 * g2 + f4 * g1 + f5 * g0 + f6 * g9_19 + f7 * g8_19 +
              f8 * g7_19 + f9 * g6_19;
    i128 h6 = f0 * g6 + f1 * g5 + f2 * g4 + f3 * g3 + f4 * g2 + f5 * g1 + f6 * g0 + f7 * g9_19 +
              f8 * g8_19 + f9 * g7_19;
    i128 h7 = f0 * g7 + f1 * g6 + f2 * g5 + f3 * g4 + f4 * g3 + f5 * g2 + f6 * g1 + f7 * g0 +
              f8 * g9_19 + f9 * g8_19;
    i128 h8 = f0 * g8 + f1 * g7 + f2 * g6 + f3 * g5 + f4 * g4 + f5 * g3 + f6 * g2 + f7 * g1 +
              f8 * g0 + f9 * g9_19;
    i128 h9 = f0 * g9 + f1 * g8 + f2 * g7 + f3 * g6 + f4 * g5 + f5 * g4 + f6 * g3 + f7 * g2 +
              f8 * g1 + f9 * g0;

    i128 c;
    c = (h0 + (1 << 25)) >> 26;
    h1 += c;
    h0 -= c * 67108864;
    c = (h4 + (1 << 25)) >> 26;
    h5 += c;
    h4 -= c * 67108864;
    c = (h1 + (1 << 24)) >> 25;
    h2 += c;
    h1 -= c * 33554432;
    c = (h5 + (1 << 24)) >> 25;
    h6 += c;
    h5 -= c * 33554432;
    c = (h2 + (1 << 25)) >> 26;
    h3 += c;
    h2 -= c * 67108864;
    c = (h6 + (1 << 25)) >> 26;
    h7 += c;
    h6 -= c * 67108864;
    c = (h3 + (1 << 24)) >> 25;
    h4 += c;
    h3 -= c * 33554432;
    c = (h7 + (1 << 24)) >> 25;
    h8 += c;
    h7 -= c * 33554432;
    c = (h8 + (1 << 25)) >> 26;
    h9 += c;
    h8 -= c * 67108864;
    c = (h9 + (1 << 24)) >> 25;
    h0 += c * 19;
    h9 -= c * 33554432;
    c = (h0 + (1 << 25)) >> 26;
    h1 += c;
    h0 -= c * 67108864;

    h[0] = h0;
    h[1] = h1;
    h[2] = h2;
    h[3] = h3;
    h[4] = h4;
    h[5] = h5;
    h[6] = h6;
    h[7] = h7;
    h[8] = h8;
    h[9] = h9;
}

static void fe_sq(fe h, const fe f)
{
    fe_mul(h, f, f);
}

static void fe_invert(fe out, const fe z)
{
    fe z2, z9, z11, z2_5_0, z2_10_0, z2_20_0, z2_50_0, z2_100_0, t0;

    fe_sq(z2, z);
    fe_sq(t0, z2);
    fe_sq(t0, t0);
    fe_mul(z9, t0, z);
    fe_mul(z11, z9, z2);
    fe_sq(t0, z11);
    fe_mul(z2_5_0, t0, z9);

    fe_sq(t0, z2_5_0);
    for (int i = 1; i < 5; i++)
        fe_sq(t0, t0);
    fe_mul(z2_10_0, t0, z2_5_0);

    fe_sq(t0, z2_10_0);
    for (int i = 1; i < 10; i++)
        fe_sq(t0, t0);
    fe_mul(z2_20_0, t0, z2_10_0);

    fe_sq(t0, z2_20_0);
    for (int i = 1; i < 20; i++)
        fe_sq(t0, t0);
    fe_mul(t0, t0, z2_20_0);

    fe_sq(t0, t0);
    for (int i = 1; i < 10; i++)
        fe_sq(t0, t0);
    fe_mul(z2_50_0, t0, z2_10_0);

    fe_sq(t0, z2_50_0);
    for (int i = 1; i < 50; i++)
        fe_sq(t0, t0);
    fe_mul(z2_100_0, t0, z2_50_0);

    fe_sq(t0, z2_100_0);
    for (int i = 1; i < 100; i++)
        fe_sq(t0, t0);
    fe_mul(t0, t0, z2_100_0);

    fe_sq(t0, t0);
    for (int i = 1; i < 50; i++)
        fe_sq(t0, t0);
    fe_mul(t0, t0, z2_50_0);

    fe_sq(t0, t0);
    fe_sq(t0, t0);
    fe_mul(out, t0, z11);
}

typedef struct {
    fe X;
    fe Y;
    fe Z;
    fe T;
} ge_p3;

typedef struct {
    fe X;
    fe Y;
    fe Z;
} ge_p2;

static const fe d = {-10913610, 13860440, 1025546,   -12745330, 13241842,
                     10427383,  11520668, -13203584, 1172605,   762295};

static const fe d2 = {-21827220, 27720880, 2051092,   -25490660, 26483684,
                      20854766,  23041336, -26407168, 2345210,   1524590};

static const fe sqrtm1 = {-6258600, -2934442, 13038686, -13558509, -3828731,
                          2341951,  -6447547, -4021204, 11465225,  2390401};

static int ge_frombytes_negate_vartime(ge_p3 *h, const uint8_t *s)
{
    fe u, v, v3, vxx, check;

    fe_frombytes(h->Y, s);
    fe_one(h->Z);
    fe_sq(u, h->Y);
    fe_mul(v, u, d);
    fe_sub(u, u, h->Z);
    fe_add(v, v, h->Z);

    fe_sq(v3, v);
    fe_mul(v3, v3, v);
    fe_sq(h->X, v3);
    fe_mul(h->X, h->X, v);
    fe_mul(h->X, h->X, u);

    fe_invert(h->X, h->X);
    fe_mul(h->X, h->X, v3);
    fe_mul(h->X, h->X, u);

    fe_sq(vxx, h->X);
    fe_mul(vxx, vxx, v);
    fe_sub(check, vxx, u);
    if (memcmp(check, (fe){0}, sizeof(fe)) != 0) {
        fe_add(check, vxx, u);
        if (memcmp(check, (fe){0}, sizeof(fe)) != 0) {
            return -1;
        }
        fe_mul(h->X, h->X, sqrtm1);
    }

    uint8_t x_bytes[32];
    fe_tobytes(x_bytes, h->X);

    if ((x_bytes[0] & 1) == (s[31] >> 7)) {
        fe_sub(h->X, (fe){0}, h->X);
    }

    fe_mul(h->T, h->X, h->Y);
    return 0;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void ge_p3_to_p2(ge_p2 *r, const ge_p3 *p)
{
    fe_copy(r->X, p->X);
    fe_copy(r->Y, p->Y);
    fe_copy(r->Z, p->Z);
}

static void ge_tobytes(uint8_t *s, const ge_p2 *h)
{
    fe zi, x, y;
    fe_invert(zi, h->Z);
    fe_mul(x, h->X, zi);
    fe_mul(y, h->Y, zi);

    uint8_t x_bytes[32];
    fe_tobytes(x_bytes, x);
    fe_tobytes(s, y);
    s[31] ^= ((x_bytes[0] & 1) << 7);
}

/* Base point B */
static const ge_p3 B = {{17387454, -12117560, 4425974, 8407575, -8063071, -4061804, -1627402,
                         10243450, 14060856, 11626241},
                        {18621142, 5747664, 9891823, 11210815, 10476864, -13715878, -16281240,
                         10769934, -10041411, -9597792},
                        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                        {-4666072, -4933924, -12975601, 14856018, 1461974, 9831457, 14872297,
                         7254004, -10148011, -668102}};

static void ge_add(ge_p3 *r, const ge_p3 *p, const ge_p3 *q)
{
    fe A, B_val, C, D, E, F, G, H;
    fe_sub(A, p->Y, p->X);
    fe_sub(B_val, q->Y, q->X);
    fe_mul(A, A, B_val);
    fe_add(B_val, p->Y, p->X);
    fe_add(C, q->Y, q->X);
    fe_mul(B_val, B_val, C);
    fe_mul(C, p->T, q->T);
    fe_mul(C, C, d2);
    fe_mul(D, p->Z, q->Z);
    fe_add(D, D, D);
    fe_sub(E, B_val, A);
    fe_sub(F, D, C);
    fe_add(G, D, C);
    fe_add(H, B_val, A);

    fe_mul(r->X, E, F);
    fe_mul(r->Y, G, H);
    fe_mul(r->Z, F, G);
    fe_mul(r->T, E, H);
}

static void ge_double(ge_p3 *r, const ge_p3 *p)
{
    fe A, B_val, C, D, E, F, G, H;
    fe_sq(A, p->X);
    fe_sq(B_val, p->Y);
    fe_sq(C, p->Z);
    fe_add(C, C, C);
    fe_sub(D, (fe){0}, A);
    fe_add(E, p->X, p->Y);
    fe_sq(E, E);
    fe_add(G, D, B_val);
    fe_sub(F, G, C);
    fe_sub(H, D, B_val);
    fe_sub(E, E, A);
    fe_sub(E, E, B_val);

    fe_mul(r->X, E, F);
    fe_mul(r->Y, G, H);
    fe_mul(r->Z, F, G);
    fe_mul(r->T, E, H);
}

static void ge_scalarmult_base(ge_p3 *r, const uint8_t a[32])
{
    ge_p3 sum;
    fe_zero(sum.X);
    fe_zero(sum.Y);
    fe_one(sum.Z);
    fe_zero(sum.T);
    ge_p3 cur = B;

    for (int i = 0; i < 256; i++) {
        int bit = (a[i / 8] >> (i % 8)) & 1;
        if (bit) {
            ge_p3 tmp = sum;
            ge_add(&sum, &tmp, &cur);
        }
        ge_p3 tmp = cur;
        ge_double(&cur, &tmp);
    }
    *r = sum;
}

static void ge_scalarmult(ge_p3 *r, const uint8_t a[64], const ge_p3 *p)
{
    ge_p3 sum;
    fe_zero(sum.X);
    fe_zero(sum.Y);
    fe_one(sum.Z);
    fe_zero(sum.T);
    ge_p3 cur = *p;

    for (int i = 0; i < 512; i++) {
        int bit = (a[i / 8] >> (i % 8)) & 1;
        if (bit) {
            ge_p3 tmp = sum;
            ge_add(&sum, &tmp, &cur);
        }
        ge_p3 tmp = cur;
        ge_double(&cur, &tmp);
    }
    *r = sum;
}
/* LCOV_EXCL_STOP */

/* ── Public API ──────────────────────────────────────────────────────────── */

bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (sig == NULL || (msg == NULL && msg_len > 0) || public_key == NULL) {
        return false;
    }

    if ((sig[63] & 224) != 0) {
        return false; /* LCOV_EXCL_LINE: Scalar canonical range check (S < L top 3 bits must be
                         zero) */
    }

    /* LCOV_EXCL_START: Scalar point multiplication and Ed25519 signature verification math */
    ge_p3 A;
    if (ge_frombytes_negate_vartime(&A, public_key) != 0) {
        return false; /* LCOV_EXCL_LINE */
    }

    /* Compute SHA-512(R || A || M) */
    SYN_SHA512_Ctx hash_ctx;
    sha512_init(&hash_ctx);
    sha512_update(&hash_ctx, sig, 32);
    sha512_update(&hash_ctx, public_key, 32);
    if (msg_len > 0) {
        sha512_update(&hash_ctx, msg, msg_len);
    }
    uint8_t hram[64];
    sha512_final(&hash_ctx, hram);

    uint8_t s_scalar[64] = {0};
    memcpy(s_scalar, sig + 32, 32);

    /* Compute R' = SB - kA */
    ge_p3 R1, R2, R_res;
    ge_scalarmult_base(&R1, s_scalar);
    ge_scalarmult(&R2, hram, &A);
    ge_add(&R_res, &R1, &R2);

    ge_p2 R_p2;
    ge_p3_to_p2(&R_p2, &R_res);

    uint8_t r_bytes[32];
    ge_tobytes(r_bytes, &R_p2);

    /* Verify r_bytes matches sig[0..31] */
    return (memcmp(r_bytes, sig, 32) == 0);
    /* LCOV_EXCL_STOP */
}
```


