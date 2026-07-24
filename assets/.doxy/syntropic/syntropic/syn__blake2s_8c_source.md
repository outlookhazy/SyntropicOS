

# File syn\_blake2s.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_blake2s.c**](syn__blake2s_8c.md)

[Go to the documentation of this file](syn__blake2s_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_BLAKE2S) || SYN_USE_BLAKE2S

#include "syn_blake2s.h"
#include <string.h>

/* ── IV (same as SHA-256, from the fractional parts of sqrt(2..19)) ───── */

static const uint32_t blake2s_iv[8] = {
    0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
    0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL,
};

/* ── Message word permutation schedule (10 rounds) ────────────────────── */

static const uint8_t sigma[10][16] = {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
    { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
    { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
    {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
    {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
    {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
    { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
    { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
    {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
    { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13,  0 },
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

static inline uint32_t rotr32(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32 - n));
}

static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] <<  8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static inline void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >>  8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/* ── G mixing function ──────────────────────────────────────────────────── */

#define G(v, a, b, c, d, x, y)       \
    do {                              \
        v[a] += v[b] + (x);          \
        v[d] = rotr32(v[d] ^ v[a], 16); \
        v[c] += v[d];                \
        v[b] = rotr32(v[b] ^ v[c], 12); \
        v[a] += v[b] + (y);          \
        v[d] = rotr32(v[d] ^ v[a],  8); \
        v[c] += v[d];                \
        v[b] = rotr32(v[b] ^ v[c],  7); \
    } while (0)

/* ── Compression function ───────────────────────────────────────────────── */

static void blake2s_compress(SYN_BLAKE2s *ctx, const uint8_t block[64],
                             int is_last)
{
    uint32_t v[16];
    uint32_t m[16];
    unsigned i;

    /* Load message words */
    for (i = 0; i < 16; i++) {
        m[i] = load32_le(block + i * 4);
    }

    /* Initialize working vector */
    for (i = 0; i < 8; i++) {
        v[i]     = ctx->h[i];
        v[i + 8] = blake2s_iv[i];
    }

    v[12] ^= ctx->t[0];
    v[13] ^= ctx->t[1];

    if (is_last) {
        v[14] ^= 0xFFFFFFFFUL;
    }

    /* 10 rounds of mixing */
    for (i = 0; i < 10; i++) {
        const uint8_t *s = sigma[i];

        /* Column step */
        G(v, 0, 4,  8, 12, m[s[ 0]], m[s[ 1]]);
        G(v, 1, 5,  9, 13, m[s[ 2]], m[s[ 3]]);
        G(v, 2, 6, 10, 14, m[s[ 4]], m[s[ 5]]);
        G(v, 3, 7, 11, 15, m[s[ 6]], m[s[ 7]]);

        /* Diagonal step */
        G(v, 0, 5, 10, 15, m[s[ 8]], m[s[ 9]]);
        G(v, 1, 6, 11, 12, m[s[10]], m[s[11]]);
        G(v, 2, 7,  8, 13, m[s[12]], m[s[13]]);
        G(v, 3, 4,  9, 14, m[s[14]], m[s[15]]);
    }

    /* Finalize */
    for (i = 0; i < 8; i++) {
        ctx->h[i] ^= v[i] ^ v[i + 8];
    }
}

/* ── Increment byte counter ─────────────────────────────────────────────── */

static inline void blake2s_increment_counter(SYN_BLAKE2s *ctx, uint32_t inc)
{
    ctx->t[0] += inc;
    if (ctx->t[0] < inc) {
        ctx->t[1]++;
    }
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void syn_blake2s_init(SYN_BLAKE2s *ctx, size_t outlen)
{
    unsigned i;

    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < 8; i++) {
        ctx->h[i] = blake2s_iv[i];
    }

    /* Parameter block: fanout=1, depth=1, digest_length=outlen */
    ctx->h[0] ^= 0x01010000UL ^ (uint32_t)outlen;
    ctx->outlen = (uint8_t)outlen;
}

void syn_blake2s_init_keyed(SYN_BLAKE2s *ctx,
                             const void *key, size_t keylen,
                             size_t outlen)
{
    uint8_t block[SYN_BLAKE2S_BLOCK_SIZE];
    unsigned i;

    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < 8; i++) {
        ctx->h[i] = blake2s_iv[i];
    }

    /* Parameter block: fanout=1, depth=1, keylen, digest_length */
    ctx->h[0] ^= 0x01010000UL
               ^ ((uint32_t)keylen << 8)
               ^ (uint32_t)outlen;
    ctx->outlen = (uint8_t)outlen;

    /* Key is the first block, zero-padded */
    memset(block, 0, sizeof(block));
    memcpy(block, key, keylen);
    syn_blake2s_update(ctx, block, SYN_BLAKE2S_BLOCK_SIZE);
}

void syn_blake2s_update(SYN_BLAKE2s *ctx, const void *data, size_t len)
{
    const uint8_t *in = (const uint8_t *)data;

    while (len > 0) {
        /* If buffer is full, compress it (it's not the last block
         * because we still have more data) */
        if (ctx->buflen == SYN_BLAKE2S_BLOCK_SIZE) {
            blake2s_increment_counter(ctx, SYN_BLAKE2S_BLOCK_SIZE);
            blake2s_compress(ctx, ctx->buf, 0);
            ctx->buflen = 0;
        }

        size_t fill = SYN_BLAKE2S_BLOCK_SIZE - ctx->buflen;
        if (fill > len) fill = len;

        memcpy(ctx->buf + ctx->buflen, in, fill);
        ctx->buflen += (uint8_t)fill;
        in  += fill;
        len -= fill;
    }
}

void syn_blake2s_final(SYN_BLAKE2s *ctx, uint8_t *out)
{
    unsigned i;

    blake2s_increment_counter(ctx, (uint32_t)ctx->buflen);

    /* Zero-pad remaining buffer */
    memset(ctx->buf + ctx->buflen, 0,
           SYN_BLAKE2S_BLOCK_SIZE - ctx->buflen);

    /* Compress final block */
    blake2s_compress(ctx, ctx->buf, 1);

    /* Extract output */
    for (i = 0; i < ctx->outlen / 4; i++) {
        store32_le(out + i * 4, ctx->h[i]);
    }

    /* Handle non-word-aligned output length */
    if (ctx->outlen & 3) {
        uint8_t tmp[4];
        store32_le(tmp, ctx->h[ctx->outlen / 4]);
        memcpy(out + (ctx->outlen & ~3u), tmp, ctx->outlen & 3);
    }
}

#endif /* SYN_USE_BLAKE2S */
```


