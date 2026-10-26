

# File syn\_sha512.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_sha512.c**](syn__sha512_8c.md)

[Go to the documentation of this file](syn__sha512_8c.md)


```C++

#include "syn_sha512.h"

#if !defined(SYN_USE_SHA512) || SYN_USE_SHA512

#include <string.h>

/* ── 80-Round Constants (FIPS 180-4 Section 4.2.3) ───────────────────────── */

static const uint64_t K512[80] = {
    0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL, 0xB5C0FBCFEC4D3B2FULL, 0xE9B5DBA58189DBBCULL,
    0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL, 0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL,
    0xD807AA98A3030242ULL, 0x12835B0145706FBEULL, 0x243185BE4EE4B28CULL, 0x550C7DC3D5FFB4E2ULL,
    0x72BE5D74F27B896FULL, 0x80DEB1FE3B1696B1ULL, 0x9BDC06A725C71235ULL, 0xC19BF174CF692694ULL,
    0xE49B69C19EF14AD2ULL, 0xEFBE4786384F25E3ULL, 0x0FC19DC68B8CD5B5ULL, 0x240CA1CC77AC9C65ULL,
    0x2DE92C6F592B0275ULL, 0x4A7484AA6EA6E483ULL, 0x5CB0A9DCBD41FBD4ULL, 0x76F988DA831153B5ULL,
    0x983E5152EE66DFABULL, 0xA831C66D2DB43210ULL, 0xB00327C898FB213FULL, 0xBF597FC7BEEF0EE4ULL,
    0xC6E00BF33DA88FC2ULL, 0xD5A79147930AA725ULL, 0x06CA6351E003826FULL, 0x142929670A0E6E70ULL,
    0x27B70A8546D22FFCULL, 0x2E1B21385C26C926ULL, 0x4D2C6DFC5AC42AEDULL, 0x53380D139D95B3DFULL,
    0x650A73548BAF63DEULL, 0x766A0ABB3C77B2A8ULL, 0x81C2C92E47EDAEE6ULL, 0x92722C851482353BULL,
    0xA2BFE8A14CF10364ULL, 0xA81A664BBC423001ULL, 0xC24B8B70D0F89791ULL, 0xC76C51A30654BE30ULL,
    0xD192E819D6EF5218ULL, 0xD69906245565A910ULL, 0xF40E35855771202AULL, 0x106AA07032BBD1B8ULL,
    0x19A4C116B8D2D0C8ULL, 0x1E376C085141AB53ULL, 0x2748774CDF8EEB99ULL, 0x34B0BCB5E19B48A8ULL,
    0x391C0CB3C5C95A63ULL, 0x4ED8AA4AE3418ACBULL, 0x5B9CCA4F7763E373ULL, 0x682E6FF3D6B2B8A3ULL,
    0x748F82EE5DEFB2FCULL, 0x78A5636F43172F60ULL, 0x84C87814A1F0AB72ULL, 0x8CC702081A6439ECULL,
    0x90BEFFFA23631E28ULL, 0xA4506CEBDE82BDE9ULL, 0xBEF9A3F7B2C67915ULL, 0xC67178F2E372532BULL,
    0xCA273ECEEA26619CULL, 0xD186B8C721C0C207ULL, 0xEADA7DD6CDE0EB1EULL, 0xF57D4F7FEE6ED178ULL,
    0x06F067AA72176FBAULL, 0x0A637DC5A2C898A6ULL, 0x113F9804BEF90DAEULL, 0x1B710B35131C471BULL,
    0x28DB77F523047D84ULL, 0x32CAAB7B40C72493ULL, 0x3C9EBE0A15C9BEBCULL, 0x431D67C49C100D4CULL,
    0x4CC5D4BECB3E42B6ULL, 0x597F299CFC657E2AULL, 0x5FCB6FAB3AD6FAECULL, 0x6C44198C4A475817ULL};

/* ── Macros ──────────────────────────────────────────────────────────────── */

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64U - (n))))
#define SHR64(x, n) ((x) >> (n))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define SIGMA0(x) (ROTR64(x, 28U) ^ ROTR64(x, 34U) ^ ROTR64(x, 39U))
#define SIGMA1(x) (ROTR64(x, 14U) ^ ROTR64(x, 18U) ^ ROTR64(x, 41U))
#define SIGMA_LOWER0(x) (ROTR64(x, 1U) ^ ROTR64(x, 8U) ^ SHR64(x, 7U))
#define SIGMA_LOWER1(x) (ROTR64(x, 19U) ^ ROTR64(x, 61U) ^ SHR64(x, 6U))

static inline uint64_t load_be64(const uint8_t *p)
{
    return ((uint64_t)p[0] << 56U) | ((uint64_t)p[1] << 48U) | ((uint64_t)p[2] << 40U) |
           ((uint64_t)p[3] << 32U) | ((uint64_t)p[4] << 24U) | ((uint64_t)p[5] << 16U) |
           ((uint64_t)p[6] << 8U) | (uint64_t)p[7];
}

static inline void store_be64(uint8_t *p, uint64_t v)
{
    p[0] = (uint8_t)((v >> 56U) & 0xFFU);
    p[1] = (uint8_t)((v >> 48U) & 0xFFU);
    p[2] = (uint8_t)((v >> 40U) & 0xFFU);
    p[3] = (uint8_t)((v >> 32U) & 0xFFU);
    p[4] = (uint8_t)((v >> 24U) & 0xFFU);
    p[5] = (uint8_t)((v >> 16U) & 0xFFU);
    p[6] = (uint8_t)((v >> 8U) & 0xFFU);
    p[7] = (uint8_t)(v & 0xFFU);
}

/* ── Core Block Transform ────────────────────────────────────────────────── */

static void sha512_transform(uint64_t state[8], const uint8_t block[SYN_SHA512_BLOCK_SIZE])
{
    uint64_t w[80];

    for (size_t t = 0U; t < 16U; t++) {
        w[t] = load_be64(&block[t * 8U]);
    }
    for (size_t t = 16U; t < 80U; t++) {
        w[t] = SIGMA_LOWER1(w[t - 2U]) + w[t - 7U] + SIGMA_LOWER0(w[t - 15U]) + w[t - 16U];
    }

    uint64_t a = state[0];
    uint64_t b = state[1];
    uint64_t c = state[2];
    uint64_t d = state[3];
    uint64_t e = state[4];
    uint64_t f = state[5];
    uint64_t g = state[6];
    uint64_t h = state[7];

    for (size_t t = 0U; t < 80U; t++) {
        uint64_t t1 = h + SIGMA1(e) + CH(e, f, g) + K512[t] + w[t];
        uint64_t t2 = SIGMA0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
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

static void sha512_pad_and_finalize(SYN_SHA512 *ctx)
{
    /* Append 0x80 */
    ctx->buf[ctx->buf_len++] = 0x80U;

    if (ctx->buf_len > (SYN_SHA512_BLOCK_SIZE - 16U)) {
        memset(&ctx->buf[ctx->buf_len], 0, SYN_SHA512_BLOCK_SIZE - ctx->buf_len);
        sha512_transform(ctx->state, ctx->buf);
        ctx->buf_len = 0U;
    }

    memset(&ctx->buf[ctx->buf_len], 0, (SYN_SHA512_BLOCK_SIZE - 16U) - ctx->buf_len);

    /* 128-bit bit length: [total_len_hi * 8 + overflow] || [total_len_lo * 8] */
    uint64_t bit_len_hi = (ctx->total_len_hi << 3U) | (ctx->total_len_lo >> 61U);
    uint64_t bit_len_lo = ctx->total_len_lo << 3U;

    store_be64(&ctx->buf[SYN_SHA512_BLOCK_SIZE - 16U], bit_len_hi);
    store_be64(&ctx->buf[SYN_SHA512_BLOCK_SIZE - 8U], bit_len_lo);

    sha512_transform(ctx->state, ctx->buf);
}

/* ── Hash API ────────────────────────────────────────────────────────────── */

void syn_sha512_init(SYN_SHA512 *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->state[0] = 0x6a09e667f3bcc908ULL;
    ctx->state[1] = 0xbb67ae8584caa73bULL;
    ctx->state[2] = 0x3c6ef372fe94f82bULL;
    ctx->state[3] = 0xa54ff53a5f1d36f1ULL;
    ctx->state[4] = 0x510e527fade682d1ULL;
    ctx->state[5] = 0x9b05688c2b3e6c1fULL;
    ctx->state[6] = 0x1f83d9abfb41bd6bULL;
    ctx->state[7] = 0x5be0cd19137e2179ULL;
    ctx->buf_len = 0U;
    ctx->total_len_lo = 0ULL;
    ctx->total_len_hi = 0ULL;
}

void syn_sha384_init(SYN_SHA512 *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->state[0] = 0xcbbb9d5dc1059ed8ULL;
    ctx->state[1] = 0x629a292a367cd507ULL;
    ctx->state[2] = 0x9159015a3070dd17ULL;
    ctx->state[3] = 0x152fecd8f70e5939ULL;
    ctx->state[4] = 0x67332667ffc00b31ULL;
    ctx->state[5] = 0x8eb44a8768581511ULL;
    ctx->state[6] = 0xdb0c2e0d64f98fa7ULL;
    ctx->state[7] = 0x47b5481dbefa4fa4ULL;
    ctx->buf_len = 0U;
    ctx->total_len_lo = 0ULL;
    ctx->total_len_hi = 0ULL;
}

void syn_sha512_update(SYN_SHA512 *ctx, const void *data, size_t len)
{
    if (ctx == NULL || data == NULL || len == 0U) {
        return;
    }

    const uint8_t *p = (const uint8_t *)data;

    uint64_t prev_lo = ctx->total_len_lo;
    ctx->total_len_lo += (uint64_t)len;
    if (ctx->total_len_lo < prev_lo) {
        ctx->total_len_hi++;
    }

    if (ctx->buf_len > 0U) {
        size_t needed = SYN_SHA512_BLOCK_SIZE - ctx->buf_len;
        if (len < needed) {
            memcpy(&ctx->buf[ctx->buf_len], p, len);
            ctx->buf_len += (uint32_t)len;
            return;
        }
        memcpy(&ctx->buf[ctx->buf_len], p, needed);
        sha512_transform(ctx->state, ctx->buf);
        p += needed;
        len -= needed;
        ctx->buf_len = 0U;
    }

    while (len >= SYN_SHA512_BLOCK_SIZE) {
        sha512_transform(ctx->state, p);
        p += SYN_SHA512_BLOCK_SIZE;
        len -= SYN_SHA512_BLOCK_SIZE;
    }

    if (len > 0U) {
        memcpy(ctx->buf, p, len);
        ctx->buf_len = (uint32_t)len;
    }
}

void syn_sha512_final(SYN_SHA512 *ctx, uint8_t hash[SYN_SHA512_DIGEST_SIZE])
{
    if (ctx == NULL || hash == NULL) {
        return;
    }
    sha512_pad_and_finalize(ctx);
    for (size_t i = 0U; i < 8U; i++) {
        store_be64(&hash[i * 8U], ctx->state[i]);
    }
}

void syn_sha384_final(SYN_SHA512 *ctx, uint8_t hash[SYN_SHA384_DIGEST_SIZE])
{
    if (ctx == NULL || hash == NULL) {
        return;
    }
    sha512_pad_and_finalize(ctx);
    for (size_t i = 0U; i < 6U; i++) {
        store_be64(&hash[i * 8U], ctx->state[i]);
    }
}

void syn_sha512(const void *data, size_t len, uint8_t hash[SYN_SHA512_DIGEST_SIZE])
{
    SYN_SHA512 ctx;
    syn_sha512_init(&ctx);
    syn_sha512_update(&ctx, data, len);
    syn_sha512_final(&ctx, hash);
}

void syn_sha384(const void *data, size_t len, uint8_t hash[SYN_SHA384_DIGEST_SIZE])
{
    SYN_SHA512 ctx;
    syn_sha384_init(&ctx);
    syn_sha512_update(&ctx, data, len);
    syn_sha384_final(&ctx, hash);
}

/* ── HMAC API ────────────────────────────────────────────────────────────── */

void syn_hmac_sha512_init(SYN_HMAC_SHA512 *ctx, const void *key, size_t key_len)
{
    if (ctx == NULL) {
        return;
    }

    uint8_t k_buf[SYN_SHA512_BLOCK_SIZE];
    memset(k_buf, 0, sizeof(k_buf));

    if (key_len > SYN_SHA512_BLOCK_SIZE) {
        syn_sha512(key, key_len, k_buf);
    } else if (key != NULL && key_len > 0U) {
        memcpy(k_buf, key, key_len);
    }

    uint8_t i_pad[SYN_SHA512_BLOCK_SIZE];
    for (size_t i = 0U; i < SYN_SHA512_BLOCK_SIZE; i++) {
        i_pad[i] = (uint8_t)(k_buf[i] ^ 0x36U);
        ctx->o_key_pad[i] = (uint8_t)(k_buf[i] ^ 0x5CU);
    }

    syn_sha512_init(&ctx->inner);
    syn_sha512_update(&ctx->inner, i_pad, sizeof(i_pad));
}

void syn_hmac_sha512_update(SYN_HMAC_SHA512 *ctx, const void *data, size_t len)
{
    if (ctx == NULL || data == NULL || len == 0U) {
        return;
    }
    syn_sha512_update(&ctx->inner, data, len);
}

void syn_hmac_sha512_final(SYN_HMAC_SHA512 *ctx, uint8_t mac[SYN_SHA512_DIGEST_SIZE])
{
    if (ctx == NULL || mac == NULL) {
        return;
    }

    uint8_t inner_hash[SYN_SHA512_DIGEST_SIZE];
    syn_sha512_final(&ctx->inner, inner_hash);

    SYN_SHA512 outer;
    syn_sha512_init(&outer);
    syn_sha512_update(&outer, ctx->o_key_pad, sizeof(ctx->o_key_pad));
    syn_sha512_update(&outer, inner_hash, sizeof(inner_hash));
    syn_sha512_final(&outer, mac);
}

void syn_hmac_sha384_init(SYN_HMAC_SHA384 *ctx, const void *key, size_t key_len)
{
    if (ctx == NULL) {
        return;
    }

    uint8_t k_buf[SYN_SHA512_BLOCK_SIZE];
    memset(k_buf, 0, sizeof(k_buf));

    if (key_len > SYN_SHA512_BLOCK_SIZE) {
        syn_sha384(key, key_len, k_buf);
    } else if (key != NULL && key_len > 0U) {
        memcpy(k_buf, key, key_len);
    }

    uint8_t i_pad[SYN_SHA512_BLOCK_SIZE];
    for (size_t i = 0U; i < SYN_SHA512_BLOCK_SIZE; i++) {
        i_pad[i] = (uint8_t)(k_buf[i] ^ 0x36U);
        ctx->o_key_pad[i] = (uint8_t)(k_buf[i] ^ 0x5CU);
    }

    syn_sha384_init(&ctx->inner);
    syn_sha512_update(&ctx->inner, i_pad, sizeof(i_pad));
}

void syn_hmac_sha384_update(SYN_HMAC_SHA384 *ctx, const void *data, size_t len)
{
    if (ctx == NULL || data == NULL || len == 0U) {
        return;
    }
    syn_sha512_update(&ctx->inner, data, len);
}

void syn_hmac_sha384_final(SYN_HMAC_SHA384 *ctx, uint8_t mac[SYN_SHA384_DIGEST_SIZE])
{
    if (ctx == NULL || mac == NULL) {
        return;
    }

    uint8_t inner_hash[SYN_SHA384_DIGEST_SIZE];
    syn_sha384_final(&ctx->inner, inner_hash);

    SYN_SHA512 outer;
    syn_sha384_init(&outer);
    syn_sha512_update(&outer, ctx->o_key_pad, sizeof(ctx->o_key_pad));
    syn_sha512_update(&outer, inner_hash, sizeof(inner_hash));
    syn_sha384_final(&outer, mac);
}

void syn_hmac_sha512(const void *key, size_t key_len, const void *data, size_t data_len,
                     uint8_t mac[SYN_SHA512_DIGEST_SIZE])
{
    if (mac == NULL || (key == NULL && key_len > 0U) || (data == NULL && data_len > 0U)) {
        return;
    }

    SYN_HMAC_SHA512 ctx;
    syn_hmac_sha512_init(&ctx, key, key_len);
    syn_hmac_sha512_update(&ctx, data, data_len);
    syn_hmac_sha512_final(&ctx, mac);
}

void syn_hmac_sha384(const void *key, size_t key_len, const void *data, size_t data_len,
                     uint8_t mac[SYN_SHA384_DIGEST_SIZE])
{
    if (mac == NULL || (key == NULL && key_len > 0U) || (data == NULL && data_len > 0U)) {
        return;
    }

    SYN_HMAC_SHA384 ctx;
    syn_hmac_sha384_init(&ctx, key, key_len);
    syn_hmac_sha384_update(&ctx, data, data_len);
    syn_hmac_sha384_final(&ctx, mac);
}

#endif /* SYN_USE_SHA512 */
```


