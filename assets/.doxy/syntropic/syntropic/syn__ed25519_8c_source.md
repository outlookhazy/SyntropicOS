

# File syn\_ed25519.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_ed25519.c**](syn__ed25519_8c.md)

[Go to the documentation of this file](syn__ed25519_8c.md)


```C++


#include "syn_ed25519.h"

#if SYN_ED25519_ENABLED

#include <string.h>


/* ── Curve25519 Field Arithmetic (GF(2^255 - 19), 16 × 16-bit limbs) ──────── */

typedef int32_t gf[16];

typedef enum { ED_OP_PUBKEY = 1, ED_OP_SIGN = 2, ED_OP_VERIFY = 3 } EdOp;

#define ED_HASH_CHUNK_BYTES 128U
#define ED_EXP_ITERS_PER_STEP 2U

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

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

static void car25519_64(int64_t o[16])
{
    for (int i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        int64_t c = o[i] >> 16;
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
    int64_t m[16];
    int64_t t[16];
    for (int i = 0; i < 16; i++) {
        t[i] = (int64_t)n[i];
    }
    car25519_64(t);
    car25519_64(t);
    car25519_64(t);
    for (int j = 0; j < 2; j++) {
        m[0] = t[0] - 0xFFED;
        for (int i = 1; i < 15; i++) {
            m[i] = t[i] - 0xFFFF - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xFFFF;
        }
        m[15] = t[15] - 0x7FFF - ((m[14] >> 16) & 1);
        int64_t b = (m[15] >> 16) & 1;
        m[14] &= 0xFFFF;
        sel25519_64(t, m, (int)(1 - b));
    }
    for (int i = 0; i < 16; i++) {
        uint16_t v = (uint16_t)t[i];
        o[2 * i] = (uint8_t)(v & 0xFFU);
        o[2 * i + 1] = (uint8_t)((v >> 8) & 0xFFU);
    }
    syn_secure_zero(m, sizeof(m));
    syn_secure_zero(t, sizeof(t));
}

static int crypto_verify_32(const uint8_t *x, const uint8_t *y)
{
    uint32_t d = 0U;
    for (int i = 0; i < 32; i++) {
        d |= (uint32_t)(x[i] ^ y[i]);
    }
    uint16_t eq = (uint16_t)((d - 1U) >> 8U) & 1U;
    return (int)eq - 1;
}

static int neq25519(const gf a, const gf b)
{
    uint8_t c[32];
    uint8_t d[32];
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
        uint16_t v = (uint16_t)n[2 * i] | (uint16_t)((uint16_t)n[2 * i + 1] << 8);
        o[i] = (int32_t)v;
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
    int64_t t[31] = {0};
    for (int i = 0; i < 16; i++) {
        int64_t ai = (int64_t)a[i];
        for (int j = 0; j < 16; j++) {
            t[i + j] += ai * (int64_t)b[j];
        }
    }
    for (int i = 0; i < 15; i++) {
        t[i] += 38 * t[i + 16];
    }
    car25519_64(t);
    car25519_64(t);
    for (int i = 0; i < 16; i++) {
        o[i] = (int32_t)t[i];
    }
}

static void S(gf o, const gf a)
{
    int64_t t[31] = {0};
    for (int i = 0; i < 16; i++) {
        int64_t ai = (int64_t)a[i];
        t[2 * i] += ai * ai;
        int64_t ai2 = ai * 2;
        for (int j = i + 1; j < 16; j++) {
            t[i + j] += ai2 * (int64_t)a[j];
        }
    }
    for (int i = 0; i < 15; i++) {
        t[i] += 38 * t[i + 16];
    }
    car25519_64(t);
    car25519_64(t);
    for (int i = 0; i < 16; i++) {
        o[i] = (int32_t)t[i];
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
    gf a;
    gf b;
    gf c;
    gf d_pt;
    gf t;
    gf e;
    gf f;
    gf g;
    gf h;

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

static void add_base(gf p[4])
{
    gf b_pt[4];
    set25519(b_pt[0], X);
    set25519(b_pt[1], Y);
    set25519(b_pt[2], gf1);
    M(b_pt[3], X, Y);
    add(p, b_pt);
}

static void cswap(gf p[4], gf q[4], uint8_t b)
{
    for (int i = 0; i < 4; i++) {
        sel25519(p[i], q[i], (int)b);
    }
}

static bool ed25519_y_is_canonical(const uint8_t p[32])
{
    uint32_t borrow = (((uint32_t)p[0] - 0xEDU) >> 31U) & 1U;
    for (uint32_t i = 1U; i < 31U; i++) {
        uint32_t diff = (uint32_t)p[i] - 0xFFU - borrow;
        borrow = (diff >> 31U) & 1U;
    }
    uint32_t top = ((uint32_t)p[31] & 0x7FU) - 0x7FU - borrow;
    return (((top >> 31U) & 1U) != 0U);
}

/* ── Scalar Arithmetic Modulo L (RFC 8032 Section 5.1) ───────────────────── */

static const int64_t ORDER_L[32] = {
    0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0x10};

static bool ed25519_scalar_is_canonical(const uint8_t s[32])
{
    uint32_t borrow = 0U;
    for (uint32_t i = 0U; i < 32U; i++) {
        uint32_t si = (uint32_t)s[i];
        uint32_t li = (uint32_t)ORDER_L[i];
        uint32_t diff = si - li - borrow;
        borrow = (diff >> 31U) & 1U;
    }
    return (borrow != 0U);
}

static void modL(uint8_t *r, int64_t x[64])
{
    int64_t carry;
    int64_t i;
    int64_t j;
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
    syn_secure_zero(x, sizeof(x));
}

/* ── Cooperative Protothread Helpers ──────────────────────────────────────── */

static bool ed_pt_args_ok(const SYN_PT *pt, const SYN_Ed25519_Ctx *ctx, uint8_t expected_op)
{
    return (pt != NULL) && (ctx != NULL) && ctx->active && (ctx->op == expected_op);
}

static void ed_expand_secret(const uint8_t secret_key[32], uint8_t d[64])
{
    SYN_SHA512 hash_ctx;
    syn_sha512_init(&hash_ctx);
    syn_sha512_update(&hash_ctx, secret_key, SYN_ED25519_SECRET_KEY_SIZE);
    syn_sha512_final(&hash_ctx, d);
    syn_secure_zero(&hash_ctx, sizeof(hash_ctx));

    d[0] &= 248U;
    d[31] &= 127U;
    d[31] |= 64U;
}

static void ed_scalarbase_setup(SYN_Ed25519_Ctx *ctx)
{
    set25519(ctx->p[0], gf0);
    set25519(ctx->p[1], gf1);
    set25519(ctx->p[2], gf1);
    set25519(ctx->p[3], gf0);

    set25519(ctx->q[0], X);
    set25519(ctx->q[1], Y);
    set25519(ctx->q[2], gf1);
    M(ctx->q[3], X, Y);
}

static uint8_t ed_scalar_bit(const uint8_t s[32], uint16_t idx)
{
    uint16_t bit_pos = (uint16_t)(255U - idx);
    return (uint8_t)((s[bit_pos >> 3U] >> (bit_pos & 7U)) & 1U);
}

static SYN_PT_Status ed_ladder_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, const uint8_t s[32])
{
    PT_BEGIN(pt);
    ed_scalarbase_setup(ctx);
    PT_CONTINUE(pt);

    for (ctx->idx = 0U; ctx->idx < 256U; ctx->idx++) {
        cswap(ctx->p, ctx->q, ed_scalar_bit(s, ctx->idx));
        add(ctx->q, ctx->p);
        PT_CONTINUE(pt);

        add(ctx->p, ctx->p);
        cswap(ctx->p, ctx->q, ed_scalar_bit(s, ctx->idx));
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void ed_pack_from_inv(const SYN_Ed25519_Ctx *ctx, uint8_t out[32])
{
    gf tx;
    gf ty;
    M(tx, ctx->p[0], ctx->u.aux[0]);
    M(ty, ctx->p[1], ctx->u.aux[0]);
    pack25519(out, ty);
    out[31] ^= (uint8_t)(par25519(tx) << 7U);
    syn_secure_zero(tx, sizeof(tx));
    syn_secure_zero(ty, sizeof(ty));
}

static SYN_PT_Status ed_pack_inv_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, uint8_t out[32])
{
    PT_BEGIN(pt);
    set25519(ctx->u.aux[0], ctx->p[2]);
    ctx->idx = 0U;
    while (ctx->idx < 254U) {
        for (uint16_t k = 0U; k < ED_EXP_ITERS_PER_STEP && ctx->idx < 254U; k++) {
            uint16_t a = (uint16_t)(253U - ctx->idx);
            S(ctx->u.aux[0], ctx->u.aux[0]);
            if (a != 2U && a != 4U) {
                M(ctx->u.aux[0], ctx->u.aux[0], ctx->p[2]);
            }
            ctx->idx++;
        }
        PT_CONTINUE(pt);
    }

    ed_pack_from_inv(ctx, out);
    PT_CONTINUE(pt);
    PT_END(pt);
}

static SYN_PT_Status ed_base_mul_pack_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, const uint8_t scalar[32],
                                         uint8_t out[32])
{
    PT_BEGIN(pt);
    PT_SPAWN(pt, &ctx->sub2_pt, ed_ladder_pt(&ctx->sub2_pt, ctx, scalar));
    PT_SPAWN(pt, &ctx->sub2_pt, ed_pack_inv_pt(&ctx->sub2_pt, ctx, out));
    PT_END(pt);
}

static SYN_PT_Status ed_hash_msg_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx)
{
    PT_BEGIN(pt);
    ctx->msg_off = 0U;
    while (ctx->msg_off < ctx->msg_len) {
        size_t rem = ctx->msg_len - ctx->msg_off;
        size_t chunk = (rem > ED_HASH_CHUNK_BYTES) ? ED_HASH_CHUNK_BYTES : rem;
        syn_sha512_update(&ctx->u.sha, ctx->msg + ctx->msg_off, chunk);
        ctx->msg_off += chunk;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void ed_start_nonce_hash(SYN_Ed25519_Ctx *ctx)
{
    syn_sha512_init(&ctx->u.sha);
    syn_sha512_update(&ctx->u.sha, ctx->d + 32, 32U);
}

static void ed_finalize_nonce_hash(SYN_Ed25519_Ctx *ctx)
{
    uint8_t r_full[64];
    syn_sha512_final(&ctx->u.sha, r_full);
    reduce(r_full);
    (void)memcpy(ctx->scalar, r_full, 32U);
    syn_secure_zero(r_full, sizeof(r_full));
}

static void ed_start_challenge_hash(SYN_Ed25519_Ctx *ctx)
{
    syn_sha512_init(&ctx->u.sha);
    syn_sha512_update(&ctx->u.sha, ctx->r_pt, 32U);
    syn_sha512_update(&ctx->u.sha, ctx->pk, 32U);
}

static void ed_finish_challenge_hash(SYN_Ed25519_Ctx *ctx)
{
    uint8_t h[64];
    syn_sha512_final(&ctx->u.sha, h);
    reduce(h);
    (void)memcpy(ctx->pk, h, 32U);
    syn_secure_zero(h, sizeof(h));
}

static void ed_finish_sign_scalar(SYN_Ed25519_Ctx *ctx, uint8_t sig[64])
{
    int64_t x[64];
    for (int i = 0; i < 64; i++) {
        x[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        x[i] = (int64_t)ctx->scalar[i];
    }
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            x[i + j] += (int64_t)ctx->pk[i] * (int64_t)ctx->d[j];
        }
    }
    (void)memcpy(sig, ctx->r_pt, 32U);
    modL(sig + 32, x);

    syn_secure_zero(x, sizeof(x));
    syn_ed25519_wipe(ctx);
}

static void ed_finalize_verify_hash(SYN_Ed25519_Ctx *ctx)
{
    uint8_t h_full[64];
    syn_sha512_final(&ctx->u.sha, h_full);
    reduce(h_full);
    (void)memcpy(ctx->d, h_full, 32U);
    syn_secure_zero(h_full, sizeof(h_full));
}

static void ed_unpackneg_start(SYN_Ed25519_Ctx *ctx)
{
    gf den2;
    gf den4;
    gf den6;

    set25519(ctx->q[2], gf1);
    unpack25519(ctx->q[1], ctx->pk);
    S(ctx->u.aux[0], ctx->q[1]);
    M(ctx->u.aux[1], ctx->u.aux[0], D);
    Z(ctx->u.aux[0], ctx->u.aux[0], ctx->q[2]);
    A(ctx->u.aux[1], ctx->q[2], ctx->u.aux[1]);

    S(den2, ctx->u.aux[1]);
    S(den4, den2);
    M(den6, den4, den2);
    M(ctx->u.aux[2], den6, ctx->u.aux[0]);
    M(ctx->u.aux[2], ctx->u.aux[2], ctx->u.aux[1]);
    set25519(ctx->q[0], ctx->u.aux[2]);
}

static SYN_PT_Status ed_pow2523_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx)
{
    PT_BEGIN(pt);
    ctx->idx = 0U;
    while (ctx->idx < 251U) {
        for (uint16_t k = 0U; k < ED_EXP_ITERS_PER_STEP && ctx->idx < 251U; k++) {
            uint16_t a = (uint16_t)(250U - ctx->idx);
            S(ctx->q[0], ctx->q[0]);
            if (a != 1U) {
                M(ctx->q[0], ctx->q[0], ctx->u.aux[2]);
            }
            ctx->idx++;
        }
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void ed_unpackneg_root(SYN_Ed25519_Ctx *ctx)
{
    gf t;
    gf chk;

    M(t, ctx->q[0], ctx->u.aux[0]);
    M(t, t, ctx->u.aux[1]);
    M(t, t, ctx->u.aux[1]);
    M(ctx->q[0], t, ctx->u.aux[1]);

    S(chk, ctx->q[0]);
    M(chk, chk, ctx->u.aux[1]);
    if (neq25519(chk, ctx->u.aux[0]) != 0) {
        M(ctx->q[0], ctx->q[0], I);
    }
}

static bool ed_unpackneg_finish(SYN_Ed25519_Ctx *ctx)
{
    gf chk;

    S(chk, ctx->q[0]);
    M(chk, chk, ctx->u.aux[1]);
    if (neq25519(chk, ctx->u.aux[0]) != 0) {
        return false;
    }

    if (neq25519(ctx->q[0], gf0) == 0 && (ctx->pk[31] & 0x80U) != 0U) {
        return false;
    }

    if (par25519(ctx->q[0]) == (uint8_t)(ctx->pk[31] >> 7U)) {
        Z(ctx->q[0], gf0, ctx->q[0]);
    }
    M(ctx->q[3], ctx->q[0], ctx->q[1]);
    return true;
}

static void ed_shamir_setup(SYN_Ed25519_Ctx *ctx)
{
    set25519(ctx->p[0], gf0);
    set25519(ctx->p[1], gf1);
    set25519(ctx->p[2], gf1);
    set25519(ctx->p[3], gf0);

    memcpy(ctx->u.aux, ctx->q, sizeof(ctx->u.aux));
    add_base(ctx->u.aux);
}

static void ed_shamir_add_bit(SYN_Ed25519_Ctx *ctx)
{
    uint16_t bit_pos = (uint16_t)(255U - ctx->idx);
    uint8_t bs = (uint8_t)((ctx->scalar[bit_pos >> 3U] >> (bit_pos & 7U)) & 1U);
    uint8_t bh = (uint8_t)((ctx->d[bit_pos >> 3U] >> (bit_pos & 7U)) & 1U);
    if (bs != 0U && bh != 0U) {
        add(ctx->p, ctx->u.aux);
    } else if (bs != 0U) {
        add_base(ctx->p);
    } else if (bh != 0U) {
        add(ctx->p, ctx->q);
    }
}

static SYN_PT_Status ed_shamir_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx)
{
    PT_BEGIN(pt);
    for (ctx->idx = 0U; ctx->idx < 256U; ctx->idx++) {
        add(ctx->p, ctx->p);
        PT_CONTINUE(pt);

        ed_shamir_add_bit(ctx);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}


/* ── Public API ──────────────────────────────────────────────────────────── */

void syn_ed25519_wipe(SYN_Ed25519_Ctx *ctx)
{
    if (ctx != NULL) {
        syn_secure_zero(ctx, sizeof(*ctx));
    }
}

bool syn_ed25519_publickey_init(SYN_Ed25519_Ctx *ctx,
                                const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE])
{
    if (ctx == NULL || secret_key == NULL) {
        return false;
    }
    syn_ed25519_wipe(ctx);
    ed_expand_secret(secret_key, ctx->d);
    ctx->op = (uint8_t)ED_OP_PUBKEY;
    ctx->active = true;
    return true;
}

SYN_PT_Status syn_ed25519_publickey_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                       uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (!ed_pt_args_ok(pt, ctx, (uint8_t)ED_OP_PUBKEY) || public_key == NULL) {
        if (ctx != NULL) {
            syn_ed25519_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    PT_SPAWN(pt, &ctx->sub_pt, ed_base_mul_pack_pt(&ctx->sub_pt, ctx, ctx->d, public_key));
    syn_ed25519_wipe(ctx);
    PT_END(pt);
}

bool syn_ed25519_sign_init(SYN_Ed25519_Ctx *ctx, const uint8_t *msg, size_t msg_len,
                           const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (ctx == NULL || (msg == NULL && msg_len > 0U) || secret_key == NULL) {
        return false;
    }
    syn_ed25519_wipe(ctx);
    ed_expand_secret(secret_key, ctx->d);
    ctx->msg = msg;
    ctx->msg_len = msg_len;
    ctx->op = (uint8_t)ED_OP_SIGN;
    ctx->active = true;

    if (public_key != NULL) {
        (void)memcpy(ctx->pk, public_key, SYN_ED25519_PUBLIC_KEY_SIZE);
        ctx->has_pk = true;
    } else {
        ctx->has_pk = false;
    }
    return true;
}

SYN_PT_Status syn_ed25519_sign_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx,
                                  uint8_t sig[SYN_ED25519_SIGNATURE_SIZE])
{
    if (!ed_pt_args_ok(pt, ctx, (uint8_t)ED_OP_SIGN) || sig == NULL) {
        if (ctx != NULL) {
            syn_ed25519_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    if (!ctx->has_pk) {
        PT_SPAWN(pt, &ctx->sub_pt, ed_base_mul_pack_pt(&ctx->sub_pt, ctx, ctx->d, ctx->pk));
    }

    ed_start_nonce_hash(ctx);
    PT_SPAWN(pt, &ctx->sub_pt, ed_hash_msg_pt(&ctx->sub_pt, ctx));
    ed_finalize_nonce_hash(ctx);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, ed_base_mul_pack_pt(&ctx->sub_pt, ctx, ctx->scalar, ctx->r_pt));

    ed_start_challenge_hash(ctx);
    PT_SPAWN(pt, &ctx->sub_pt, ed_hash_msg_pt(&ctx->sub_pt, ctx));
    ed_finish_challenge_hash(ctx);
    PT_CONTINUE(pt);
    ed_finish_sign_scalar(ctx, sig);

    PT_END(pt);
}

bool syn_ed25519_verify_init(SYN_Ed25519_Ctx *ctx, const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                             const uint8_t *msg, size_t msg_len,
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (ctx == NULL || sig == NULL || (msg == NULL && msg_len > 0U) || public_key == NULL) {
        return false;
    }
    if (!ed25519_scalar_is_canonical(sig + 32) || !ed25519_y_is_canonical(public_key)) {
        return false;
    }

    syn_ed25519_wipe(ctx);
    (void)memcpy(ctx->r_pt, sig, 32U);
    (void)memcpy(ctx->scalar, sig + 32, 32U);
    (void)memcpy(ctx->pk, public_key, 32U);
    ctx->msg = msg;
    ctx->msg_len = msg_len;

    syn_sha512_init(&ctx->u.sha);
    syn_sha512_update(&ctx->u.sha, ctx->r_pt, 32U);
    syn_sha512_update(&ctx->u.sha, ctx->pk, 32U);

    ctx->op = (uint8_t)ED_OP_VERIFY;
    ctx->prehashed = false;
    ctx->active = true;
    return true;
}

bool syn_ed25519_verify_hash_init(SYN_Ed25519_Ctx *ctx,
                                  const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE],
                                  const uint8_t h[64],
                                  const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (ctx == NULL || sig == NULL || h == NULL || public_key == NULL) {
        return false;
    }
    if (!ed25519_scalar_is_canonical(sig + 32) || !ed25519_y_is_canonical(public_key)) {
        return false;
    }

    syn_ed25519_wipe(ctx);
    (void)memcpy(ctx->r_pt, sig, 32U);
    (void)memcpy(ctx->scalar, sig + 32, 32U);
    (void)memcpy(ctx->pk, public_key, 32U);

    uint8_t h_full[64];
    (void)memcpy(h_full, h, 64U);
    reduce(h_full);
    (void)memcpy(ctx->d, h_full, 32U);
    syn_secure_zero(h_full, sizeof(h_full));

    ctx->op = (uint8_t)ED_OP_VERIFY;
    ctx->prehashed = true;
    ctx->active = true;
    return true;
}

SYN_PT_Status syn_ed25519_verify_pt(SYN_PT *pt, SYN_Ed25519_Ctx *ctx, bool *out_valid)
{
    if (out_valid != NULL) {
        *out_valid = false;
    }
    if (!ed_pt_args_ok(pt, ctx, (uint8_t)ED_OP_VERIFY) || out_valid == NULL) {
        if (ctx != NULL) {
            syn_ed25519_wipe(ctx);
        }
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    if (!ctx->prehashed) {
        PT_SPAWN(pt, &ctx->sub_pt, ed_hash_msg_pt(&ctx->sub_pt, ctx));
        ed_finalize_verify_hash(ctx);
        PT_CONTINUE(pt);
    }

    ed_unpackneg_start(ctx);
    PT_CONTINUE(pt);

    PT_SPAWN(pt, &ctx->sub_pt, ed_pow2523_pt(&ctx->sub_pt, ctx));

    ed_unpackneg_root(ctx);
    PT_CONTINUE(pt);

    if (!ed_unpackneg_finish(ctx)) {
        *out_valid = false;
    } else {
        PT_CONTINUE(pt);

        ed_shamir_setup(ctx);
        PT_CONTINUE(pt);

        PT_SPAWN(pt, &ctx->sub_pt, ed_shamir_pt(&ctx->sub_pt, ctx));
        PT_SPAWN(pt, &ctx->sub_pt, ed_pack_inv_pt(&ctx->sub_pt, ctx, ctx->pk));
        *out_valid = (crypto_verify_32(ctx->r_pt, ctx->pk) == 0);
    }
    syn_ed25519_wipe(ctx);

    PT_END(pt);
}

bool syn_ed25519_publickey(const uint8_t secret_key[SYN_ED25519_SECRET_KEY_SIZE],
                           uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    if (public_key == NULL) {
        return false;
    }
    SYN_Ed25519_Ctx ctx;
    SYN_PT pt;
    if (!syn_ed25519_publickey_init(&ctx, secret_key)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_ed25519_publickey_pt(&pt, &ctx, public_key) == PT_MORE) {
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
    if (sig == NULL) {
        return false;
    }
    SYN_Ed25519_Ctx ctx;
    SYN_PT pt;
    if (!syn_ed25519_sign_init(&ctx, msg, msg_len, secret_key, public_key)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_ed25519_sign_pt(&pt, &ctx, sig) == PT_MORE) {
    }
    return true;
}

bool syn_ed25519_verify_hash(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t h[64],
                             const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    SYN_Ed25519_Ctx ctx;
    SYN_PT pt;
    bool valid = false;
    if (!syn_ed25519_verify_hash_init(&ctx, sig, h, public_key)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_ed25519_verify_pt(&pt, &ctx, &valid) == PT_MORE) {
    }
    return valid;
}

bool syn_ed25519_verify(const uint8_t sig[SYN_ED25519_SIGNATURE_SIZE], const uint8_t *msg,
                        size_t msg_len, const uint8_t public_key[SYN_ED25519_PUBLIC_KEY_SIZE])
{
    SYN_Ed25519_Ctx ctx;
    SYN_PT pt;
    bool valid = false;
    if (!syn_ed25519_verify_init(&ctx, sig, msg, msg_len, public_key)) {
        return false;
    }
    PT_INIT(&pt);
    while (syn_ed25519_verify_pt(&pt, &ctx, &valid) == PT_MORE) {
    }
    return valid;
}

#endif /* SYN_ED25519_ENABLED */
```


