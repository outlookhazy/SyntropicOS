

# File syn\_x25519.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x25519.c**](syn__x25519_8c.md)

[Go to the documentation of this file](syn__x25519_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_X25519) || SYN_USE_X25519


#include "../util/syn_pack.h"
#include "syn_x25519.h"

#include <string.h>

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

typedef int32_t gf[16];

static const gf gf0 = {0};
static const gf gf1 = {1};
static const gf _121665 = {0xDB41, 1};

/* ── Field Arithmetic (GF(2^255 - 19), 16 × 16-bit limbs) ──────────── */

static void car25519_64(int64_t o[16])
{
    int64_t c;
    int i;
    for (i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= (int64_t)((uint64_t)c << 16);
    }
}

static void sel25519(gf p, gf q, int b)
{
    uint32_t mask = 0U - (uint32_t)(b & 1);
    for (int i = 0; i < 16; i++) {
        uint32_t t = mask & ((uint32_t)p[i] ^ (uint32_t)q[i]);
        p[i] = (int32_t)((uint32_t)p[i] ^ t);
        q[i] = (int32_t)((uint32_t)q[i] ^ t);
    }
}

static void sel25519_64(int64_t p[16], int64_t q[16], int b)
{
    uint64_t mask = 0ULL - (uint64_t)(b & 1);
    for (int i = 0; i < 16; i++) {
        uint64_t t = mask & ((uint64_t)p[i] ^ (uint64_t)q[i]);
        p[i] = (int64_t)((uint64_t)p[i] ^ t);
        q[i] = (int64_t)((uint64_t)q[i] ^ t);
    }
}

static void pack25519(uint8_t o[32], const gf n)
{
    int i;
    int j;
    int64_t b;
    int64_t m[16];
    int64_t t[16];
    for (i = 0; i < 16; i++) {
        t[i] = (int64_t)n[i];
    }
    car25519_64(t);
    car25519_64(t);
    car25519_64(t);
    for (j = 0; j < 2; j++) {
        m[0] = t[0] - 0xFFED;
        for (i = 1; i < 15; i++) {
            m[i] = t[i] - 0xFFFF - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xFFFF;
        }
        m[15] = t[15] - 0x7FFF - ((m[14] >> 16) & 1);
        b = (m[15] >> 16) & 1;
        m[14] &= 0xFFFF;
        sel25519_64(t, m, (int)(1 - b));
    }
    for (i = 0; i < 16; i++) {
        syn_poke_u16_le((uint16_t)t[i], o, 2 * i);
    }
    syn_secure_zero(m, sizeof(m));
    syn_secure_zero(t, sizeof(t));
}

static void unpack25519(gf o, const uint8_t n[32])
{
    int i;
    for (i = 0; i < 16; i++) {
        o[i] = (int32_t)syn_peek_u16_le(n, 2 * i);
    }
    o[15] &= 0x7FFF;
}

static void A(gf o, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; i++) {
        o[i] = a[i] + b[i];
    }
}

static void Z(gf o, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; i++) {
        o[i] = a[i] - b[i];
    }
}

static void M(gf o, const gf a, const gf b)
{
    int64_t t[31] = {0};
    int i;
    int j;
    for (i = 0; i < 16; i++) {
        int64_t ai = (int64_t)a[i];
        for (j = 0; j < 16; j++) {
            t[i + j] += ai * (int64_t)b[j];
        }
    }
    for (i = 0; i < 15; i++) {
        t[i] += 38LL * t[i + 16];
    }
    car25519_64(t);
    car25519_64(t);
    for (i = 0; i < 16; i++) {
        o[i] = (int32_t)t[i];
    }
}

static void S(gf o, const gf a)
{
    int64_t t[31] = {0};
    int i;
    int j;
    for (i = 0; i < 16; i++) {
        int64_t ai = (int64_t)a[i];
        t[2 * i] += ai * ai;
        int64_t ai2 = ai * 2;
        for (j = i + 1; j < 16; j++) {
            t[i + j] += ai2 * (int64_t)a[j];
        }
    }
    for (i = 0; i < 15; i++) {
        t[i] += 38LL * t[i + 16];
    }
    car25519_64(t);
    car25519_64(t);
    for (i = 0; i < 16; i++) {
        o[i] = (int32_t)t[i];
    }
}

static void x25519_ladder_step(SYN_X25519_Ctx *ctx)
{
    gf e;
    gf f;
    uint16_t bit = (uint16_t)(254U - ctx->idx);
    int r = (int)((ctx->z[bit >> 3U] >> (bit & 7U)) & 1U);

    sel25519(ctx->a, ctx->b, r);
    sel25519(ctx->c, ctx->d, r);
    A(e, ctx->a, ctx->c);
    Z(ctx->a, ctx->a, ctx->c);
    A(ctx->c, ctx->b, ctx->d);
    Z(ctx->b, ctx->b, ctx->d);
    S(ctx->d, e);
    S(f, ctx->a);
    M(ctx->a, ctx->c, ctx->a);
    M(ctx->c, ctx->b, e);
    A(e, ctx->a, ctx->c);
    Z(ctx->a, ctx->a, ctx->c);
    S(ctx->b, ctx->a);
    Z(ctx->c, ctx->d, f);
    M(ctx->a, ctx->c, _121665);
    A(ctx->a, ctx->a, ctx->d);
    M(ctx->c, ctx->c, ctx->a);
    M(ctx->a, ctx->d, f);
    M(ctx->d, ctx->b, ctx->x);
    S(ctx->b, e);
    sel25519(ctx->a, ctx->b, r);
    sel25519(ctx->c, ctx->d, r);

    syn_secure_zero(e, sizeof(e));
    syn_secure_zero(f, sizeof(f));
}

static SYN_PT_Status x25519_ladder_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 255U; ctx->idx++) {
        x25519_ladder_step(ctx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status x25519_inv_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 254U; ctx->idx++) {
        uint16_t inv_i = (uint16_t)(253U - ctx->idx);
        S(ctx->c, ctx->c);
        if (inv_i != 2U && inv_i != 4U) {
            M(ctx->c, ctx->c, ctx->d);
        }
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

/* ── X25519 scalar multiplication (Montgomery ladder) ───────────────── */

bool syn_x25519_init(SYN_X25519_Ctx *ctx, const uint8_t scalar[32], const uint8_t point[32])
{
    if (ctx == NULL || scalar == NULL || point == NULL) {
        return false;
    }

    syn_secure_zero(ctx, sizeof(*ctx));
    memcpy(ctx->z, scalar, 32U);
    ctx->z[31] = (uint8_t)((ctx->z[31] & 127U) | 64U);
    ctx->z[0] &= 248U;

    unpack25519(ctx->x, point);

    memcpy(ctx->b, ctx->x, sizeof(gf));
    memcpy(ctx->a, gf1, sizeof(gf));
    memcpy(ctx->d, gf1, sizeof(gf));
    memcpy(ctx->c, gf0, sizeof(gf));
    ctx->active = true;
    return true;
}

bool syn_x25519_pubkey_init(SYN_X25519_Ctx *ctx, const uint8_t private_key[32])
{
    static const uint8_t basepoint[32] = {9U};
    return syn_x25519_init(ctx, private_key, basepoint);
}

void syn_x25519_wipe(SYN_X25519_Ctx *ctx)
{
    if (ctx != NULL) {
        syn_secure_zero(ctx, sizeof(*ctx));
    }
}

SYN_PT_Status syn_x25519_pt(SYN_PT *pt, SYN_X25519_Ctx *ctx, uint8_t shared_out[32])
{
    if (pt == NULL || ctx == NULL || shared_out == NULL || !ctx->active) {
        if (ctx != NULL) {
            syn_x25519_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    PT_SPAWN(pt, &ctx->sub_pt, x25519_ladder_pt(&ctx->sub_pt, ctx));
    memcpy(ctx->d, ctx->c, sizeof(gf));
    PT_SPAWN(pt, &ctx->sub_pt, x25519_inv_pt(&ctx->sub_pt, ctx));

    M(ctx->a, ctx->a, ctx->c);
    pack25519(shared_out, ctx->a);
    syn_x25519_wipe(ctx);

    PT_END(pt);
}

void syn_x25519(uint8_t shared_out[32], const uint8_t scalar[32], const uint8_t point[32])
{
    SYN_X25519_Ctx ctx;
    SYN_PT pt;
    if (shared_out == NULL || !syn_x25519_init(&ctx, scalar, point)) {
        return;
    }
    PT_INIT(&pt);
    while (syn_x25519_pt(&pt, &ctx, shared_out) == PT_MORE) {
    }
}

void syn_x25519_pubkey(uint8_t public_out[32], const uint8_t private_key[32])
{
    static const uint8_t basepoint[32] = {9};
    syn_x25519(public_out, private_key, basepoint);
}

#endif /* SYN_USE_X25519 */
```


