

# File syn\_sha256.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_sha256.c**](syn__sha256_8c.md)

[Go to the documentation of this file](syn__sha256_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256

#include "syn_sha256.h"
#include "syn_assert.h"

#include <string.h>

/* ── Constants ──────────────────────────────────────────────────────────── */

static const uint32_t H_INIT[8] = {
    0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
    0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
};

static const uint32_t K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

/* ── Bit operations (FIPS 180-4 §4.1.2) ───────────────────────────────── */

static inline uint32_t rotr(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32u - n));
}

static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (~x & z);
}

static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint32_t sigma0(uint32_t x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static inline uint32_t sigma1(uint32_t x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static inline uint32_t gamma0(uint32_t x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static inline uint32_t gamma1(uint32_t x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

/* ── Block transform ────────────────────────────────────────────────────── */

static void sha256_transform(uint32_t state[8], const uint8_t block[64])
{
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h;
    unsigned i;

    /* Prepare message schedule */
    for (i = 0; i < 16; i++) {
        W[i] = ((uint32_t)block[i * 4 + 0] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] <<  8) |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
    }

    /* Working variables */
    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    /* 64 rounds */
    for (i = 0; i < 64; i++) {
        uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + W[i];
        uint32_t t2 = sigma0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    /* Update state */
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_sha256_init(SYN_SHA256 *ctx)
{
    SYN_ASSERT(ctx != NULL);
    memcpy(ctx->state, H_INIT, sizeof(H_INIT));
    ctx->buf_len      = 0;
    ctx->total_len_lo = 0;
    ctx->total_len_hi = 0;
}

void syn_sha256_update(SYN_SHA256 *ctx, const void *data, size_t len)
{
    SYN_ASSERT(ctx != NULL);
    if (data == NULL || len == 0) return;

    const uint8_t *p = (const uint8_t *)data;

    /* Track total length in bits (split into hi/lo for >4GB messages) */
    uint32_t len_bits_lo = (uint32_t)(len << 3);
    ctx->total_len_lo += len_bits_lo;
    if (ctx->total_len_lo < len_bits_lo) {
        ctx->total_len_hi++;  /* carry */
    }
    ctx->total_len_hi += (uint32_t)(len >> 29);

    /* Fill partial buffer first */
    if (ctx->buf_len > 0) {
        uint32_t space = SYN_SHA256_BLOCK_SIZE - ctx->buf_len;
        uint32_t fill = (len < space) ? (uint32_t)len : space;
        memcpy(ctx->buf + ctx->buf_len, p, fill);
        ctx->buf_len += fill;
        p   += fill;
        len -= fill;

        if (ctx->buf_len == SYN_SHA256_BLOCK_SIZE) {
            sha256_transform(ctx->state, ctx->buf);
            ctx->buf_len = 0;
        }
    }

    /* Process full blocks directly from input */
    while (len >= SYN_SHA256_BLOCK_SIZE) {
        sha256_transform(ctx->state, p);
        p   += SYN_SHA256_BLOCK_SIZE;
        len -= SYN_SHA256_BLOCK_SIZE;
    }

    /* Buffer remaining bytes */
    if (len > 0) {
        memcpy(ctx->buf, p, len);
        ctx->buf_len = (uint32_t)len;
    }
}

void syn_sha256_final(SYN_SHA256 *ctx, uint8_t hash[SYN_SHA256_DIGEST_SIZE])
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(hash != NULL);

    /* Pad: append 0x80, then zeros, then 64-bit big-endian length */
    ctx->buf[ctx->buf_len++] = 0x80;

    if (ctx->buf_len > 56) {
        /* Not enough room for length — fill this block and process */
        memset(ctx->buf + ctx->buf_len, 0,
               SYN_SHA256_BLOCK_SIZE - ctx->buf_len);
        sha256_transform(ctx->state, ctx->buf);
        ctx->buf_len = 0;
    }

    /* Pad with zeros up to byte 56 */
    memset(ctx->buf + ctx->buf_len, 0, 56 - ctx->buf_len);

    /* Append 64-bit big-endian total length in bits */
    ctx->buf[56] = (uint8_t)(ctx->total_len_hi >> 24);
    ctx->buf[57] = (uint8_t)(ctx->total_len_hi >> 16);
    ctx->buf[58] = (uint8_t)(ctx->total_len_hi >>  8);
    ctx->buf[59] = (uint8_t)(ctx->total_len_hi);
    ctx->buf[60] = (uint8_t)(ctx->total_len_lo >> 24);
    ctx->buf[61] = (uint8_t)(ctx->total_len_lo >> 16);
    ctx->buf[62] = (uint8_t)(ctx->total_len_lo >>  8);
    ctx->buf[63] = (uint8_t)(ctx->total_len_lo);

    sha256_transform(ctx->state, ctx->buf);

    /* Produce big-endian digest */
    unsigned i;
    for (i = 0; i < 8; i++) {
        hash[i * 4 + 0] = (uint8_t)(ctx->state[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(ctx->state[i] >>  8);
        hash[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

#endif /* SYN_USE_SHA256 */
```


