#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CHACHA20POLY1305) || SYN_USE_CHACHA20POLY1305

/**
 * @file syn_chacha20poly1305.c
 * @brief ChaCha20-Poly1305 AEAD — RFC 8439.
 *
 * ChaCha20: add-rotate-xor stream cipher (pure 32-bit integer ops).
 * Poly1305: one-time MAC using 130-bit arithmetic with 5 × 26-bit limbs.
 * AEAD: combined construction per RFC 8439 §2.8.
 */

#include "syn_chacha20poly1305.h"
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Load 32-bit little-endian word.
 * @param p Source bytes.
 * @return 32-bit value.
 */
static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] <<  8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/**
 * @brief Store 32-bit little-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >>  8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/**
 * @brief Store 64-bit little-endian word.
 * @param p Destination bytes.
 * @param v Value to store.
 */
static inline void store64_le(uint8_t *p, uint64_t v)
{
    store32_le(p,     (uint32_t)(v));
    store32_le(p + 4, (uint32_t)(v >> 32));
}

/**
 * @brief 32-bit left rotation.
 * @param x Value to rotate.
 * @param n Bits to shift.
 * @return Rotated value.
 */
static inline uint32_t rotl32(uint32_t x, unsigned n)
{
    return (x << n) | (x >> (32 - n));
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  ChaCha20
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief ChaCha20 Quarter Round.
 * @param a,b,c,d Indices into the state vector.
 */
#define QR(a, b, c, d)           \
    do {                         \
        a += b; d ^= a; d = rotl32(d, 16); \
        c += d; b ^= c; b = rotl32(b, 12); \
        a += b; d ^= a; d = rotl32(d,  8); \
        c += d; b ^= c; b = rotl32(b,  7); \
    } while (0)

/** @brief Compute one ChaCha20 block into @p out (16 uint32_t).
 * @param out Output buffer (16 words).
 * @param in  Initial state (16 words).
 */
static void chacha20_block_core(uint32_t out[16], const uint32_t in[16])
{
    int i;
    for (i = 0; i < 16; i++) out[i] = in[i];

    for (i = 0; i < 10; i++) {
        /* Column rounds */
        QR(out[0], out[4], out[ 8], out[12]);
        QR(out[1], out[5], out[ 9], out[13]);
        QR(out[2], out[6], out[10], out[14]);
        QR(out[3], out[7], out[11], out[15]);
        /* Diagonal rounds */
        QR(out[0], out[5], out[10], out[15]);
        QR(out[1], out[6], out[11], out[12]);
        QR(out[2], out[7], out[ 8], out[13]);
        QR(out[3], out[4], out[ 9], out[14]);
    }

    for (i = 0; i < 16; i++) out[i] += in[i];
}

/** @brief Set up the ChaCha20 initial state.
 * @param state   Initial state buffer (16 words).
 * @param key     256-bit key.
 * @param nonce   96-bit nonce.
 * @param counter Initial block counter.
 */
static void chacha20_init(uint32_t state[16],
                          const uint8_t key[32],
                          const uint8_t nonce[12],
                          uint32_t counter)
{
    /* "expand 32-byte k" */
    state[0] = 0x61707865UL;
    state[1] = 0x3320646EUL;
    state[2] = 0x79622D32UL;
    state[3] = 0x6B206574UL;

    /* Key (8 words) */
    state[4]  = load32_le(key +  0);
    state[5]  = load32_le(key +  4);
    state[6]  = load32_le(key +  8);
    state[7]  = load32_le(key + 12);
    state[8]  = load32_le(key + 16);
    state[9]  = load32_le(key + 20);
    state[10] = load32_le(key + 24);
    state[11] = load32_le(key + 28);

    /* Counter + nonce */
    state[12] = counter;
    state[13] = load32_le(nonce + 0);
    state[14] = load32_le(nonce + 4);
    state[15] = load32_le(nonce + 8);
}

void syn_chacha20_block(const uint8_t key[32],
                        const uint8_t nonce[12],
                        uint32_t counter,
                        uint8_t out[64])
{
    uint32_t state[16], block[16];
    int i;

    chacha20_init(state, key, nonce, counter);
    chacha20_block_core(block, state);

    for (i = 0; i < 16; i++) {
        store32_le(out + i * 4, block[i]);
    }
}

void syn_chacha20_xor(const uint8_t key[32],
                      const uint8_t nonce[12],
                      uint32_t counter,
                      const uint8_t *in, size_t len,
                      uint8_t *out)
{
    uint32_t state[16], block[16];
    uint8_t keystream[64];
    size_t i;

    chacha20_init(state, key, nonce, counter);

    while (len > 0) {
        chacha20_block_core(block, state);

        /* Serialize keystream */
        for (i = 0; i < 16; i++) {
            store32_le(keystream + i * 4, block[i]);
        }

        /* XOR up to 64 bytes */
        size_t chunk = len < 64 ? len : 64;
        for (i = 0; i < chunk; i++) {
            out[i] = in[i] ^ keystream[i];
        }

        in  += chunk;
        out += chunk;
        len -= chunk;

        state[12]++;  /* Increment counter */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Poly1305  (donna-style, 5 × 26-bit limbs)
 * ═══════════════════════════════════════════════════════════════════════════ */

/** @brief Poly1305 context. */
typedef struct {
    uint32_t r[5];   /**< Clamped key r (26-bit limbs)  */
    uint32_t h[5];   /**< Accumulator (26-bit limbs)    */
    uint32_t pad[4]; /**< Key s = second 16 bytes       */
} Poly1305_Ctx;

/** @brief Initialise Poly1305 from a 32-byte one-time key.
 * @param ctx Context to initialize.
 * @param key 32-byte key.
 */
static void poly1305_init(Poly1305_Ctx *ctx, const uint8_t key[32])
{
    /* r = key[0..15], clamped */
    uint32_t t0 = load32_le(key +  0);
    uint32_t t1 = load32_le(key +  4);
    uint32_t t2 = load32_le(key +  8);
    uint32_t t3 = load32_le(key + 12);

    ctx->r[0] =  t0                         & 0x3FFFFFFUL;
    ctx->r[1] = ((t0 >> 26) | (t1 <<  6))   & 0x3FFFF03UL;
    ctx->r[2] = ((t1 >> 20) | (t2 << 12))   & 0x3FFC0FFUL;
    ctx->r[3] = ((t2 >> 14) | (t3 << 18))   & 0x3F03FFFUL;
    ctx->r[4] =  (t3 >>  8)                 & 0x00FFFFFUL;

    /* h = 0 */
    ctx->h[0] = ctx->h[1] = ctx->h[2] = ctx->h[3] = ctx->h[4] = 0;

    /* pad = key[16..31] */
    ctx->pad[0] = load32_le(key + 16);
    ctx->pad[1] = load32_le(key + 20);
    ctx->pad[2] = load32_le(key + 24);
    ctx->pad[3] = load32_le(key + 28);
}

/** @brief Absorb full 16-byte blocks into Poly1305 accumulator.
 * @param ctx   Poly1305 context.
 * @param data  Input data.
 * @param len   Length in bytes (must be multiple of 16).
 * @param hibit High bit to add (1<<24 for message, 0 for special uses).
 */
static void poly1305_blocks(Poly1305_Ctx *ctx,
                            const uint8_t *data, size_t len,
                            uint32_t hibit)
{
    uint32_t r0 = ctx->r[0], r1 = ctx->r[1], r2 = ctx->r[2];
    uint32_t r3 = ctx->r[3], r4 = ctx->r[4];
    uint32_t s1 = r1 * 5, s2 = r2 * 5, s3 = r3 * 5, s4 = r4 * 5;
    uint32_t h0 = ctx->h[0], h1 = ctx->h[1], h2 = ctx->h[2];
    uint32_t h3 = ctx->h[3], h4 = ctx->h[4];

    while (len >= 16) {
        /* h += m[i] */
        uint32_t t0 = load32_le(data +  0);
        uint32_t t1 = load32_le(data +  4);
        uint32_t t2 = load32_le(data +  8);
        uint32_t t3 = load32_le(data + 12);

        h0 +=  t0                         & 0x3FFFFFFUL;
        h1 += ((t0 >> 26) | (t1 <<  6))   & 0x3FFFFFFUL;
        h2 += ((t1 >> 20) | (t2 << 12))   & 0x3FFFFFFUL;
        h3 += ((t2 >> 14) | (t3 << 18))   & 0x3FFFFFFUL;
        h4 +=  (t3 >>  8)                 | hibit;

        /* h *= r */
        uint64_t d0 = (uint64_t)h0*r0 + (uint64_t)h1*s4 +
                      (uint64_t)h2*s3 + (uint64_t)h3*s2 + (uint64_t)h4*s1;
        uint64_t d1 = (uint64_t)h0*r1 + (uint64_t)h1*r0 +
                      (uint64_t)h2*s4 + (uint64_t)h3*s3 + (uint64_t)h4*s2;
        uint64_t d2 = (uint64_t)h0*r2 + (uint64_t)h1*r1 +
                      (uint64_t)h2*r0 + (uint64_t)h3*s4 + (uint64_t)h4*s3;
        uint64_t d3 = (uint64_t)h0*r3 + (uint64_t)h1*r2 +
                      (uint64_t)h2*r1 + (uint64_t)h3*r0 + (uint64_t)h4*s4;
        uint64_t d4 = (uint64_t)h0*r4 + (uint64_t)h1*r3 +
                      (uint64_t)h2*r2 + (uint64_t)h3*r1 + (uint64_t)h4*r0;

        /* Carry propagation */
        uint32_t c;
        c = (uint32_t)(d0 >> 26); h0 = (uint32_t)d0 & 0x3FFFFFFUL; d1 += c;
        c = (uint32_t)(d1 >> 26); h1 = (uint32_t)d1 & 0x3FFFFFFUL; d2 += c;
        c = (uint32_t)(d2 >> 26); h2 = (uint32_t)d2 & 0x3FFFFFFUL; d3 += c;
        c = (uint32_t)(d3 >> 26); h3 = (uint32_t)d3 & 0x3FFFFFFUL; d4 += c;
        c = (uint32_t)(d4 >> 26); h4 = (uint32_t)d4 & 0x3FFFFFFUL;
        h0 += c * 5;
        c = h0 >> 26; h0 &= 0x3FFFFFFUL; h1 += c;

        data += 16;
        len  -= 16;
    }

    ctx->h[0] = h0; ctx->h[1] = h1; ctx->h[2] = h2;
    ctx->h[3] = h3; ctx->h[4] = h4;
}

/** @brief Finalise Poly1305 and write the 16-byte MAC to @p mac.
 * @param ctx Poly1305 context.
 * @param mac Output buffer (16 bytes).
 */
static void poly1305_finish(Poly1305_Ctx *ctx, uint8_t mac[16])
{
    uint32_t h0 = ctx->h[0], h1 = ctx->h[1], h2 = ctx->h[2];
    uint32_t h3 = ctx->h[3], h4 = ctx->h[4];
    uint32_t c, g0, g1, g2, g3, g4, mask;

    /* Full carry chain */
    c = h1 >> 26; h1 &= 0x3FFFFFFUL; h2 += c;
    c = h2 >> 26; h2 &= 0x3FFFFFFUL; h3 += c;
    c = h3 >> 26; h3 &= 0x3FFFFFFUL; h4 += c;
    c = h4 >> 26; h4 &= 0x3FFFFFFUL; h0 += c * 5;
    c = h0 >> 26; h0 &= 0x3FFFFFFUL; h1 += c;

    /* Compute h - p = h - (2^130 - 5) */
    g0 = h0 + 5; c = g0 >> 26; g0 &= 0x3FFFFFFUL;
    g1 = h1 + c; c = g1 >> 26; g1 &= 0x3FFFFFFUL;
    g2 = h2 + c; c = g2 >> 26; g2 &= 0x3FFFFFFUL;
    g3 = h3 + c; c = g3 >> 26; g3 &= 0x3FFFFFFUL;
    g4 = h4 + c - (1UL << 26);

    /* Select h or h-p based on carry (constant-time) */
    mask = (g4 >> 31) - 1;  /* 0 if g4 < 0 (keep h), all-1s if g4 >= 0 (use g) */
    /* Correction: mask = all-1s if borrow, 0 if no borrow. We want g when no borrow. */
    g0 &= mask; g1 &= mask; g2 &= mask; g3 &= mask; g4 &= mask;
    mask = ~mask;
    h0 = (h0 & mask) | g0;
    h1 = (h1 & mask) | g1;
    h2 = (h2 & mask) | g2;
    h3 = (h3 & mask) | g3;
    h4 = (h4 & mask) | g4;

    /* Reassemble into 4 × 32-bit */
    uint32_t f0 = h0 | (h1 << 26);
    uint32_t f1 = (h1 >> 6) | (h2 << 20);
    uint32_t f2 = (h2 >> 12) | (h3 << 14);
    uint32_t f3 = (h3 >> 18) | (h4 << 8);

    /* mac = (h + pad) mod 2^128 */
    uint64_t t;
    t = (uint64_t)f0 + ctx->pad[0];             f0 = (uint32_t)t;
    t = (uint64_t)f1 + ctx->pad[1] + (t >> 32); f1 = (uint32_t)t;
    t = (uint64_t)f2 + ctx->pad[2] + (t >> 32); f2 = (uint32_t)t;
    t = (uint64_t)f3 + ctx->pad[3] + (t >> 32); f3 = (uint32_t)t;

    store32_le(mac +  0, f0);
    store32_le(mac +  4, f1);
    store32_le(mac +  8, f2);
    store32_le(mac + 12, f3);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  AEAD Construction (RFC 8439 §2.8)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Compute Poly1305 MAC over (aad || pad || ct || pad || len(aad) || len(ct)).
 * @param poly_key One-time Poly1305 key (32 bytes).
 * @param aad      Additional authenticated data.
 * @param aad_len  Length of @p aad.
 * @param ct       Ciphertext.
 * @param ct_len   Length of @p ct.
 * @param tag      Output 16-byte tag.
 */
static void aead_mac(const uint8_t poly_key[32],
                     const uint8_t *aad, size_t aad_len,
                     const uint8_t *ct, size_t ct_len,
                     uint8_t tag[16])
{
    Poly1305_Ctx poly;
    poly1305_init(&poly, poly_key);

    /* AAD */
    if (aad_len > 0) {
        poly1305_blocks(&poly, aad, aad_len & ~15UL, 1UL << 24);
        if (aad_len & 15) {
            uint8_t tmp[16];
            memset(tmp, 0, 16);
            memcpy(tmp, aad + (aad_len & ~15UL), aad_len & 15);
            poly1305_blocks(&poly, tmp, 16, 1UL << 24);
        }
    }

    /* Ciphertext */
    if (ct_len > 0) {
        poly1305_blocks(&poly, ct, ct_len & ~15UL, 1UL << 24);
        if (ct_len & 15) {
            uint8_t tmp[16];
            memset(tmp, 0, 16);
            memcpy(tmp, ct + (ct_len & ~15UL), ct_len & 15);
            poly1305_blocks(&poly, tmp, 16, 1UL << 24);
        }
    }

    /* Lengths (8 bytes each, little-endian) */
    {
        uint8_t lens[16];
        store64_le(lens + 0, (uint64_t)aad_len);
        store64_le(lens + 8, (uint64_t)ct_len);
        poly1305_blocks(&poly, lens, 16, 1UL << 24);
    }

    poly1305_finish(&poly, tag);
}

void syn_aead_encrypt(const uint8_t key[32],
                      const uint8_t nonce[12],
                      const uint8_t *aad, size_t aad_len,
                      const uint8_t *plaintext, size_t pt_len,
                      uint8_t *ciphertext,
                      uint8_t tag[16])
{
    /* Step 1: Generate Poly1305 one-time key (counter = 0) */
    uint8_t poly_key[64];
    syn_chacha20_block(key, nonce, 0, poly_key);
    /* Only first 32 bytes are used as the Poly1305 key */

    /* Step 2: Encrypt plaintext (counter starts at 1) */
    syn_chacha20_xor(key, nonce, 1, plaintext, pt_len, ciphertext);

    /* Step 3: Compute MAC over (AAD || ciphertext) */
    aead_mac(poly_key, aad, aad_len, ciphertext, pt_len, tag);
}

bool syn_aead_decrypt(const uint8_t key[32],
                      const uint8_t nonce[12],
                      const uint8_t *aad, size_t aad_len,
                      const uint8_t *ciphertext, size_t ct_len,
                      const uint8_t tag[16],
                      uint8_t *plaintext)
{
    /* Step 1: Generate Poly1305 one-time key */
    uint8_t poly_key[64];
    syn_chacha20_block(key, nonce, 0, poly_key);

    /* Step 2: Verify MAC */
    uint8_t computed_tag[16];
    aead_mac(poly_key, aad, aad_len, ciphertext, ct_len, computed_tag);

    /* Constant-time compare */
    uint8_t diff = 0;
    unsigned i;
    for (i = 0; i < 16; i++) {
        diff |= computed_tag[i] ^ tag[i];
    }
    if (diff != 0) {
        return false;
    }

    /* Step 3: Decrypt (counter starts at 1) */
    syn_chacha20_xor(key, nonce, 1, ciphertext, ct_len, plaintext);

    return true;
}

#endif /* SYN_USE_CHACHA20POLY1305 */
