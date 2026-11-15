

# File syn\_aes.c

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes.c**](syn__aes_8c.md)

[Go to the documentation of this file](syn__aes_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AES) || SYN_USE_AES


#include "syn_aes.h"

#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
#include "../port/syn_port_aes.h"
#endif

#include <string.h>

/*
 * Portions of constant-time bitsliced AES implementation adapted from ctaes
 * (https://github.com/bitcoin-core/ctaes)
 * Copyright (c) 2016 Pieter Wuille
 * Distributed under the MIT software license.
 *
 * S-box Boolean circuit based on:
 * Joan Boyar and Rene Peralta, "A depth-16 circuit for the AES S-box"
 * https://eprint.iacr.org/2011/332.pdf
 */

static void syn_secure_zero(void *v, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)v;
    while (n-- > 0U) {
        *p++ = 0U;
    }
}

static void syn_secure_zero_u32(uint32_t *v, size_t words)
{
    volatile uint32_t *p = (volatile uint32_t *)v;
    while (words-- > 0U) {
        *p++ = 0U;
    }
}

static inline uint32_t aes_load_be32(const uint8_t p[4])
{
    return ((uint32_t)p[0] << 24U) | ((uint32_t)p[1] << 16U) | ((uint32_t)p[2] << 8U) |
           (uint32_t)p[3];
}

static inline void aes_store_be32(uint8_t p[4], uint32_t w)
{
    p[0] = (uint8_t)((w >> 24U) & 0xFFU);
    p[1] = (uint8_t)((w >> 16U) & 0xFFU);
    p[2] = (uint8_t)((w >> 8U) & 0xFFU);
    p[3] = (uint8_t)(w & 0xFFU);
}

#if defined(__AVR__)
#include <avr/pgmspace.h>
#define SYN_AES_PROGMEM PROGMEM
#define SYN_AES_SBOX(x) pgm_read_byte(&sbox[(x)])
#define SYN_AES_RSBOX(x) pgm_read_byte(&rsbox[(x)])
#define SYN_AES_RCON(x) pgm_read_byte(&rcon[(x)])

/* ── AES Forward S-Box (AVR PROGMEM) ────────────────────────────────────── */

static const uint8_t sbox[256] SYN_AES_PROGMEM = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
/* ── AES Inverse S-Box (AVR PROGMEM) ────────────────────────────────────── */

static const uint8_t rsbox[256] SYN_AES_PROGMEM = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};
#endif

/* ── AES Round Constants (AVR PROGMEM) ──────────────────────────────────── */

static const uint8_t rcon[11] SYN_AES_PROGMEM = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10,
                                                 0x20, 0x40, 0x80, 0x1b, 0x36};

static inline uint32_t aes_xtime4(uint32_t x)
{
    uint32_t hi = (x & 0x80808080UL) >> 7U;
    return ((x & 0x7F7F7F7FUL) << 1U) ^ (hi * 0x1BUL);
}

static inline uint32_t aes_mix_col(uint32_t c)
{
    uint32_t x2 = aes_xtime4(c);
    uint32_t r1 = (c << 8U) | (c >> 24U);
    uint32_t r2 = (c << 16U) | (c >> 16U);
    uint32_t r3 = (c << 24U) | (c >> 8U);
    uint32_t x2_r1 = (x2 << 8U) | (x2 >> 24U);
    return x2 ^ x2_r1 ^ r1 ^ r2 ^ r3;
}

static inline uint32_t aes_sub_shift(uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3)
{
    return ((uint32_t)SYN_AES_SBOX((uint8_t)(c0 >> 24U)) << 24U) |
           ((uint32_t)SYN_AES_SBOX((uint8_t)((c1 >> 16U) & 0xFFU)) << 16U) |
           ((uint32_t)SYN_AES_SBOX((uint8_t)((c2 >> 8U) & 0xFFU)) << 8U) |
           (uint32_t)SYN_AES_SBOX((uint8_t)(c3 & 0xFFU));
}

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
static inline uint8_t gmult(uint8_t a, uint8_t b)
{
    uint8_t p = 0U;
    for (int i = 0; i < 8; i++) {
        if ((b & 1U) != 0U) {
            p ^= a;
        }
        uint8_t hi_bit_set = (uint8_t)(a & 0x80U);
        a <<= 1;
        if (hi_bit_set != 0U) {
            a ^= 0x1bU;
        }
        b >>= 1;
    }
    return p;
}
#endif

#else /* !defined(__AVR__) */

/* ── Constant-Time Bitsliced AES Engine (Boyar-Peralta S-Box) ────────────── */

typedef struct {
    uint32_t slice[8]; 
} AES_state2;

#define SWAPMOVE(a, b, mask, shift)                          \
    do {                                                     \
        uint32_t sm_tmp = (((a) >> (shift)) ^ (b)) & (mask); \
        (b) ^= sm_tmp;                                       \
        (a) ^= (sm_tmp << (shift));                          \
    } while (0)

static void aes_ortho32(uint32_t q[8])
{
    /* Stage 1: shift 1, mask 0x55555555 */
    SWAPMOVE(q[0], q[1], 0x55555555UL, 1U);
    SWAPMOVE(q[2], q[3], 0x55555555UL, 1U);
    SWAPMOVE(q[4], q[5], 0x55555555UL, 1U);
    SWAPMOVE(q[6], q[7], 0x55555555UL, 1U);

    /* Stage 2: shift 2, mask 0x33333333 */
    SWAPMOVE(q[0], q[2], 0x33333333UL, 2U);
    SWAPMOVE(q[1], q[3], 0x33333333UL, 2U);
    SWAPMOVE(q[4], q[6], 0x33333333UL, 2U);
    SWAPMOVE(q[5], q[7], 0x33333333UL, 2U);

    /* Stage 3: shift 4, mask 0x0F0F0F0F */
    SWAPMOVE(q[0], q[4], 0x0F0F0F0FUL, 4U);
    SWAPMOVE(q[1], q[5], 0x0F0F0F0FUL, 4U);
    SWAPMOVE(q[2], q[6], 0x0F0F0F0FUL, 4U);
    SWAPMOVE(q[3], q[7], 0x0F0F0F0FUL, 4U);
}

static void LoadBytes1(AES_state2 *s, const uint8_t *b0)
{
    for (size_t c = 0U; c < 4U; c++) {
        s->slice[c] = (uint32_t)b0[4U * c] | ((uint32_t)b0[4U * c + 2U] << 8U);
        s->slice[4U + c] = (uint32_t)b0[4U * c + 1U] | ((uint32_t)b0[4U * c + 3U] << 8U);
    }
    aes_ortho32(s->slice);
}

static void SaveBytes1(uint8_t *b0, AES_state2 *s)
{
    aes_ortho32(s->slice);
    for (size_t c = 0U; c < 4U; c++) {
        uint32_t v0 = s->slice[c];
        uint32_t v1 = s->slice[4U + c];
        b0[4U * c] = (uint8_t)v0;
        b0[4U * c + 1U] = (uint8_t)v1;
        b0[4U * c + 2U] = (uint8_t)(v0 >> 8U);
        b0[4U * c + 3U] = (uint8_t)(v1 >> 8U);
    }
}

static void LoadBytes2(AES_state2 *s, const uint8_t *b0, const uint8_t *b1)
{
    for (size_t c = 0U; c < 4U; c++) {
        s->slice[c] = (uint32_t)b0[4U * c] | ((uint32_t)b0[4U * c + 2U] << 8U) |
                      ((uint32_t)b1[4U * c] << 16U) | ((uint32_t)b1[4U * c + 2U] << 24U);
        s->slice[4U + c] = (uint32_t)b0[4U * c + 1U] | ((uint32_t)b0[4U * c + 3U] << 8U) |
                           ((uint32_t)b1[4U * c + 1U] << 16U) | ((uint32_t)b1[4U * c + 3U] << 24U);
    }
    aes_ortho32(s->slice);
}

static void SaveBytes2(uint8_t *b0, uint8_t *b1, AES_state2 *s)
{
    aes_ortho32(s->slice);
    for (size_t c = 0U; c < 4U; c++) {
        uint32_t v0 = s->slice[c];
        uint32_t v1 = s->slice[4U + c];
        b0[4U * c] = (uint8_t)v0;
        b0[4U * c + 1U] = (uint8_t)v1;
        b0[4U * c + 2U] = (uint8_t)(v0 >> 8U);
        b0[4U * c + 3U] = (uint8_t)(v1 >> 8U);
        b1[4U * c] = (uint8_t)(v0 >> 16U);
        b1[4U * c + 1U] = (uint8_t)(v1 >> 16U);
        b1[4U * c + 2U] = (uint8_t)(v0 >> 24U);
        b1[4U * c + 3U] = (uint8_t)(v1 >> 24U);
    }
}

static void SubBytes_enc2(AES_state2 *s)
{
    uint32_t U0 = s->slice[7], U1 = s->slice[6], U2 = s->slice[5], U3 = s->slice[4];
    uint32_t U4 = s->slice[3], U5 = s->slice[2], U6 = s->slice[1], U7 = s->slice[0];

    /* Linear preprocessing. */
    uint32_t T1 = U0 ^ U3;
    uint32_t T2 = U0 ^ U5;
    uint32_t T3 = U0 ^ U6;
    uint32_t T4 = U3 ^ U5;
    uint32_t T5 = U4 ^ U6;
    uint32_t T6 = T1 ^ T5;
    uint32_t T7 = U1 ^ U2;
    uint32_t T8 = U7 ^ T6;
    uint32_t T9 = U7 ^ T7;
    uint32_t T10 = T6 ^ T7;
    uint32_t T11 = U1 ^ U5;
    uint32_t T12 = U2 ^ U5;
    uint32_t T13 = T3 ^ T4;
    uint32_t T14 = T6 ^ T11;
    uint32_t T15 = T5 ^ T11;
    uint32_t T16 = T5 ^ T12;
    uint32_t T17 = T9 ^ T16;
    uint32_t T18 = U3 ^ U7;
    uint32_t T19 = T7 ^ T18;
    uint32_t T20 = T1 ^ T19;
    uint32_t T21 = U6 ^ U7;
    uint32_t T22 = T7 ^ T21;
    uint32_t T23 = T2 ^ T22;
    uint32_t T24 = T2 ^ T10;
    uint32_t T25 = T20 ^ T17;
    uint32_t T26 = T3 ^ T16;
    uint32_t T27 = T1 ^ T12;
    uint32_t D = U7;

    /* Non-linear transformation */
    uint32_t M1 = T13 & T6;
    uint32_t M6 = T3 & T16;
    uint32_t M11 = T1 & T15;
    uint32_t M13 = (T4 & T27) ^ M11;
    uint32_t M15 = (T2 & T10) ^ M11;
    uint32_t M20 = T14 ^ M1 ^ (T23 & T8) ^ M13;
    uint32_t M21 = (T19 & D) ^ M1 ^ T24 ^ M15;
    uint32_t M22 = T26 ^ M6 ^ (T22 & T9) ^ M13;
    uint32_t M23 = (T20 & T17) ^ M6 ^ M15 ^ T25;
    uint32_t M25 = M22 & M20;
    uint32_t M37 = M21 ^ ((M20 ^ M21) & (M23 ^ M25));
    uint32_t M38 = M20 ^ M25 ^ (M21 | (M20 & M23));
    uint32_t M39 = M23 ^ ((M22 ^ M23) & (M21 ^ M25));
    uint32_t M40 = M22 ^ M25 ^ (M23 | (M21 & M22));
    uint32_t M41 = M38 ^ M40;
    uint32_t M42 = M37 ^ M39;
    uint32_t M43 = M37 ^ M38;
    uint32_t M44 = M39 ^ M40;
    uint32_t M45 = M42 ^ M41;
    uint32_t M46 = M44 & T6;
    uint32_t M47 = M40 & T8;
    uint32_t M48 = M39 & D;
    uint32_t M49 = M43 & T16;
    uint32_t M50 = M38 & T9;
    uint32_t M51 = M37 & T17;
    uint32_t M52 = M42 & T15;
    uint32_t M53 = M45 & T27;
    uint32_t M54 = M41 & T10;
    uint32_t M55 = M44 & T13;
    uint32_t M56 = M40 & T23;
    uint32_t M57 = M39 & T19;
    uint32_t M58 = M43 & T3;
    uint32_t M59 = M38 & T22;
    uint32_t M60 = M37 & T20;
    uint32_t M61 = M42 & T1;
    uint32_t M62 = M45 & T4;
    uint32_t M63 = M41 & T2;

    /* Linear postprocessing */
    uint32_t L0 = M61 ^ M62;
    uint32_t L1 = M50 ^ M56;
    uint32_t L2 = M46 ^ M48;
    uint32_t L3 = M47 ^ M55;
    uint32_t L4 = M54 ^ M58;
    uint32_t L5 = M49 ^ M61;
    uint32_t L6 = M62 ^ L5;
    uint32_t L7 = M46 ^ L3;
    uint32_t L8 = M51 ^ M59;
    uint32_t L9 = M52 ^ M53;
    uint32_t L10 = M53 ^ L4;
    uint32_t L11 = M60 ^ L2;
    uint32_t L12 = M48 ^ M51;
    uint32_t L13 = M50 ^ L0;
    uint32_t L14 = M52 ^ M61;
    uint32_t L15 = M55 ^ L1;
    uint32_t L16 = M56 ^ L0;
    uint32_t L17 = M57 ^ L1;
    uint32_t L18 = M58 ^ L8;
    uint32_t L19 = M63 ^ L4;
    uint32_t L20 = L0 ^ L1;
    uint32_t L21 = L1 ^ L7;
    uint32_t L22 = L3 ^ L12;
    uint32_t L23 = L18 ^ L2;
    uint32_t L24 = L15 ^ L9;
    uint32_t L25 = L6 ^ L10;
    uint32_t L26 = L7 ^ L9;
    uint32_t L27 = L8 ^ L10;
    uint32_t L28 = L11 ^ L14;
    uint32_t L29 = L11 ^ L17;

    s->slice[7] = L6 ^ L24;
    s->slice[6] = ~(L16 ^ L26);
    s->slice[5] = ~(L19 ^ L28);
    s->slice[4] = L6 ^ L21;
    s->slice[3] = L20 ^ L22;
    s->slice[2] = L25 ^ L29;
    s->slice[1] = ~(L13 ^ L27);
    s->slice[0] = ~(L6 ^ L23);
}

static inline uint32_t aes_shift_row_slice(uint32_t v)
{
    return (v & 0x000F000FUL) | ((v & 0x00100010UL) << 3U) | ((v & 0x00E000E0UL) >> 1U) |
           ((v & 0x03000300UL) << 2U) | ((v & 0x0C000C00UL) >> 2U) | ((v & 0x70007000UL) << 1U) |
           ((v & 0x80008000UL) >> 3U);
}

static inline uint32_t aes_rotr16_4_2(uint32_t v)
{
    return ((v >> 4U) & 0x0FFF0FFFUL) | ((v & 0x000F000FUL) << 12U);
}

static inline uint32_t aes_rotr16_12_2(uint32_t v)
{
    return ((v >> 12U) & 0x000F000FUL) | ((v & 0x0FFF0FFFUL) << 4U);
}

static inline void MixColumns_enc2(AES_state2 *s)
{
    uint32_t s0 = s->slice[0], s1 = s->slice[1], s2 = s->slice[2], s3 = s->slice[3];
    uint32_t s4 = s->slice[4], s5 = s->slice[5], s6 = s->slice[6], s7 = s->slice[7];

    uint32_t a0 = s0 ^ aes_rotr16_4_2(s0);
    uint32_t a1 = s1 ^ aes_rotr16_4_2(s1);
    uint32_t a2 = s2 ^ aes_rotr16_4_2(s2);
    uint32_t a3 = s3 ^ aes_rotr16_4_2(s3);
    uint32_t a4 = s4 ^ aes_rotr16_4_2(s4);
    uint32_t a5 = s5 ^ aes_rotr16_4_2(s5);
    uint32_t a6 = s6 ^ aes_rotr16_4_2(s6);
    uint32_t a7 = s7 ^ aes_rotr16_4_2(s7);

    s->slice[0] = a7 ^ aes_rotr16_4_2(a0) ^ aes_rotr16_12_2(s0);
    s->slice[1] = a7 ^ a0 ^ aes_rotr16_4_2(a1) ^ aes_rotr16_12_2(s1);
    s->slice[2] = a1 ^ aes_rotr16_4_2(a2) ^ aes_rotr16_12_2(s2);
    s->slice[3] = a7 ^ a2 ^ aes_rotr16_4_2(a3) ^ aes_rotr16_12_2(s3);
    s->slice[4] = a7 ^ a3 ^ aes_rotr16_4_2(a4) ^ aes_rotr16_12_2(s4);
    s->slice[5] = a4 ^ aes_rotr16_4_2(a5) ^ aes_rotr16_12_2(s5);
    s->slice[6] = a5 ^ aes_rotr16_4_2(a6) ^ aes_rotr16_12_2(s6);
    s->slice[7] = a6 ^ aes_rotr16_4_2(a7) ^ aes_rotr16_12_2(s7);
}

static inline void AddRoundKey2(AES_state2 *s, const uint16_t rk[8])
{
    for (int b = 0; b < 8; b++) {
        uint32_t k = (uint32_t)rk[b];
        s->slice[b] ^= k | (k << 16U);
    }
}

static inline void ShiftMixAdd2(AES_state2 *s, const uint16_t rk[8])
{
    uint32_t s0 = aes_shift_row_slice(s->slice[0]);
    uint32_t s1 = aes_shift_row_slice(s->slice[1]);
    uint32_t s2 = aes_shift_row_slice(s->slice[2]);
    uint32_t s3 = aes_shift_row_slice(s->slice[3]);
    uint32_t s4 = aes_shift_row_slice(s->slice[4]);
    uint32_t s5 = aes_shift_row_slice(s->slice[5]);
    uint32_t s6 = aes_shift_row_slice(s->slice[6]);
    uint32_t s7 = aes_shift_row_slice(s->slice[7]);

    uint32_t a0 = s0 ^ aes_rotr16_4_2(s0);
    uint32_t a1 = s1 ^ aes_rotr16_4_2(s1);
    uint32_t a2 = s2 ^ aes_rotr16_4_2(s2);
    uint32_t a3 = s3 ^ aes_rotr16_4_2(s3);
    uint32_t a4 = s4 ^ aes_rotr16_4_2(s4);
    uint32_t a5 = s5 ^ aes_rotr16_4_2(s5);
    uint32_t a6 = s6 ^ aes_rotr16_4_2(s6);
    uint32_t a7 = s7 ^ aes_rotr16_4_2(s7);

    uint32_t k0 = rk[0], k1 = rk[1], k2 = rk[2], k3 = rk[3];
    uint32_t k4 = rk[4], k5 = rk[5], k6 = rk[6], k7 = rk[7];

    s->slice[0] = a7 ^ aes_rotr16_4_2(a0) ^ aes_rotr16_12_2(s0) ^ k0 ^ (k0 << 16U);
    s->slice[1] = a7 ^ a0 ^ aes_rotr16_4_2(a1) ^ aes_rotr16_12_2(s1) ^ k1 ^ (k1 << 16U);
    s->slice[2] = a1 ^ aes_rotr16_4_2(a2) ^ aes_rotr16_12_2(s2) ^ k2 ^ (k2 << 16U);
    s->slice[3] = a7 ^ a2 ^ aes_rotr16_4_2(a3) ^ aes_rotr16_12_2(s3) ^ k3 ^ (k3 << 16U);
    s->slice[4] = a7 ^ a3 ^ aes_rotr16_4_2(a4) ^ aes_rotr16_12_2(s4) ^ k4 ^ (k4 << 16U);
    s->slice[5] = a4 ^ aes_rotr16_4_2(a5) ^ aes_rotr16_12_2(s5) ^ k5 ^ (k5 << 16U);
    s->slice[6] = a5 ^ aes_rotr16_4_2(a6) ^ aes_rotr16_12_2(s6) ^ k6 ^ (k6 << 16U);
    s->slice[7] = a6 ^ aes_rotr16_4_2(a7) ^ aes_rotr16_12_2(s7) ^ k7 ^ (k7 << 16U);
}

static void aes_ct_encrypt_core(AES_state2 *s, const uint16_t rk[][8], uint8_t nr)
{
    AddRoundKey2(s, rk[0]);
    for (uint8_t round = 1U; round < nr; round++) {
        SubBytes_enc2(s);
        ShiftMixAdd2(s, rk[round]);
    }
    SubBytes_enc2(s);
    for (int b = 0; b < 8; b++) {
        uint32_t k = (uint32_t)rk[nr][b];
        s->slice[b] = aes_shift_row_slice(s->slice[b]) ^ k ^ (k << 16U);
    }
}

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
static void SubBytes_dec2(AES_state2 *s)
{
    uint32_t U0 = s->slice[7], U1 = s->slice[6], U2 = s->slice[5], U3 = s->slice[4];
    uint32_t U4 = s->slice[3], U5 = s->slice[2], U6 = s->slice[1], U7 = s->slice[0];

    /* Undo linear postprocessing */
    uint32_t T23 = U0 ^ U3;
    uint32_t T22 = ~(U1 ^ U3);
    uint32_t T2 = ~(U0 ^ U1);
    uint32_t T1 = U3 ^ U4;
    uint32_t T24 = ~(U4 ^ U7);
    uint32_t R5 = U6 ^ U7;
    uint32_t T8 = ~(U1 ^ T23);
    uint32_t T19 = T22 ^ R5;
    uint32_t T9 = ~(U7 ^ T1);
    uint32_t T10 = T2 ^ T24;
    uint32_t T13 = T2 ^ R5;
    uint32_t T3 = T1 ^ R5;
    uint32_t T25 = ~(U2 ^ T1);
    uint32_t R13 = U1 ^ U6;
    uint32_t T17 = ~(U2 ^ T19);
    uint32_t T20 = T24 ^ R13;
    uint32_t T4 = U4 ^ T8;
    uint32_t R17 = ~(U2 ^ U5);
    uint32_t R18 = ~(U5 ^ U6);
    uint32_t R19 = ~(U2 ^ U4);
    uint32_t D = U0 ^ R17;
    uint32_t T6 = T22 ^ R17;
    uint32_t T16 = R13 ^ R19;
    uint32_t T27 = T1 ^ R18;
    uint32_t T15 = T10 ^ T27;
    uint32_t T14 = T10 ^ R18;
    uint32_t T26 = T3 ^ T16;

    /* Non-linear transformation */
    uint32_t M1 = T13 & T6;
    uint32_t M6 = T3 & T16;
    uint32_t M11 = T1 & T15;
    uint32_t M13 = (T4 & T27) ^ M11;
    uint32_t M15 = (T2 & T10) ^ M11;
    uint32_t M20 = T14 ^ M1 ^ (T23 & T8) ^ M13;
    uint32_t M21 = (T19 & D) ^ M1 ^ T24 ^ M15;
    uint32_t M22 = T26 ^ M6 ^ (T22 & T9) ^ M13;
    uint32_t M23 = (T20 & T17) ^ M6 ^ M15 ^ T25;
    uint32_t M25 = M22 & M20;
    uint32_t M37 = M21 ^ ((M20 ^ M21) & (M23 ^ M25));
    uint32_t M38 = M20 ^ M25 ^ (M21 | (M20 & M23));
    uint32_t M39 = M23 ^ ((M22 ^ M23) & (M21 ^ M25));
    uint32_t M40 = M22 ^ M25 ^ (M23 | (M21 & M22));
    uint32_t M41 = M38 ^ M40;
    uint32_t M42 = M37 ^ M39;
    uint32_t M43 = M37 ^ M38;
    uint32_t M44 = M39 ^ M40;
    uint32_t M45 = M42 ^ M41;
    uint32_t M46 = M44 & T6;
    uint32_t M47 = M40 & T8;
    uint32_t M48 = M39 & D;
    uint32_t M49 = M43 & T16;
    uint32_t M50 = M38 & T9;
    uint32_t M51 = M37 & T17;
    uint32_t M52 = M42 & T15;
    uint32_t M53 = M45 & T27;
    uint32_t M54 = M41 & T10;
    uint32_t M55 = M44 & T13;
    uint32_t M56 = M40 & T23;
    uint32_t M57 = M39 & T19;
    uint32_t M58 = M43 & T3;
    uint32_t M59 = M38 & T22;
    uint32_t M60 = M37 & T20;
    uint32_t M61 = M42 & T1;
    uint32_t M62 = M45 & T4;
    uint32_t M63 = M41 & T2;

    /* Undo linear preprocessing */
    uint32_t P0 = M52 ^ M61;
    uint32_t P1 = M58 ^ M59;
    uint32_t P2 = M54 ^ M62;
    uint32_t P3 = M47 ^ M50;
    uint32_t P4 = M48 ^ M56;
    uint32_t P5 = M46 ^ M51;
    uint32_t P6 = M49 ^ M60;
    uint32_t P7 = P0 ^ P1;
    uint32_t P8 = M50 ^ M53;
    uint32_t P9 = M55 ^ M63;
    uint32_t P10 = M57 ^ P4;
    uint32_t P11 = P0 ^ P3;
    uint32_t P12 = M46 ^ M48;
    uint32_t P13 = M49 ^ M51;
    uint32_t P14 = M49 ^ M62;
    uint32_t P15 = M54 ^ M59;
    uint32_t P16 = M57 ^ M61;
    uint32_t P17 = M58 ^ P2;
    uint32_t P18 = M63 ^ P5;
    uint32_t P19 = P2 ^ P3;
    uint32_t P20 = P4 ^ P6;
    uint32_t P22 = P2 ^ P7;
    uint32_t P23 = P7 ^ P8;
    uint32_t P24 = P5 ^ P7;
    uint32_t P25 = P6 ^ P10;
    uint32_t P26 = P9 ^ P11;
    uint32_t P27 = P10 ^ P18;
    uint32_t P28 = P11 ^ P25;
    uint32_t P29 = P15 ^ P20;

    s->slice[7] = P13 ^ P22;
    s->slice[6] = P26 ^ P29;
    s->slice[5] = P17 ^ P28;
    s->slice[4] = P12 ^ P22;
    s->slice[3] = P23 ^ P27;
    s->slice[2] = P19 ^ P24;
    s->slice[1] = P14 ^ P23;
    s->slice[0] = P9 ^ P16;
}

static inline void InvShiftRows2(AES_state2 *s)
{
    for (int i = 0; i < 8; i++) {
        uint32_t v = s->slice[i];
        s->slice[i] = (v & 0x000F000FUL) | ((v & 0x00700070UL) << 1U) | ((v & 0x00800080UL) >> 3U) |
                      ((v & 0x03000300UL) << 2U) | ((v & 0x0C000C00UL) >> 2U) |
                      ((v & 0x10001000UL) << 3U) | ((v & 0xE000E000UL) >> 1U);
    }
}

static inline uint32_t aes_rotr16_8_2(uint32_t v)
{
    return ((v & 0xFF00FF00UL) >> 8U) | ((v & 0x00FF00FFUL) << 8U);
}

static inline void MixColumns_dec2(AES_state2 *s)
{
    uint32_t t0_02 = s->slice[0] ^ aes_rotr16_8_2(s->slice[0]);
    uint32_t t1_02 = s->slice[1] ^ aes_rotr16_8_2(s->slice[1]);
    uint32_t t2_02 = s->slice[2] ^ aes_rotr16_8_2(s->slice[2]);
    uint32_t t3_02 = s->slice[3] ^ aes_rotr16_8_2(s->slice[3]);
    uint32_t t4_02 = s->slice[4] ^ aes_rotr16_8_2(s->slice[4]);
    uint32_t t5_02 = s->slice[5] ^ aes_rotr16_8_2(s->slice[5]);
    uint32_t t6_02 = s->slice[6] ^ aes_rotr16_8_2(s->slice[6]);
    uint32_t t7_02 = s->slice[7] ^ aes_rotr16_8_2(s->slice[7]);

    s->slice[0] ^= t6_02;
    s->slice[1] ^= t6_02 ^ t7_02;
    s->slice[2] ^= t0_02 ^ t7_02;
    s->slice[3] ^= t1_02 ^ t6_02;
    s->slice[4] ^= t2_02 ^ t6_02 ^ t7_02;
    s->slice[5] ^= t3_02 ^ t7_02;
    s->slice[6] ^= t4_02;
    s->slice[7] ^= t5_02;

    MixColumns_enc2(s);
}

static void AES_decrypt(const uint16_t rounds[][8], int nrounds, uint8_t *plain16,
                        const uint8_t *cipher16)
{
    AES_state2 s;
    LoadBytes1(&s, cipher16);
    AddRoundKey2(&s, rounds[nrounds]);
    for (int round = nrounds - 1; round > 0; round--) {
        InvShiftRows2(&s);
        SubBytes_dec2(&s);
        AddRoundKey2(&s, rounds[round]);
        MixColumns_dec2(&s);
    }
    InvShiftRows2(&s);
    SubBytes_dec2(&s);
    AddRoundKey2(&s, rounds[0]);
    SaveBytes1(plain16, &s);
    syn_secure_zero_u32(s.slice, 8U);
}
#endif

static void MultX(AES_state2 *s)
{
    uint32_t top = s->slice[7] & 1U;
    s->slice[7] = s->slice[6] & 1U;
    s->slice[6] = s->slice[5] & 1U;
    s->slice[5] = s->slice[4] & 1U;
    s->slice[4] = (s->slice[3] ^ top) & 1U;
    s->slice[3] = (s->slice[2] ^ top) & 1U;
    s->slice[2] = s->slice[1] & 1U;
    s->slice[1] = (s->slice[0] ^ top) & 1U;
    s->slice[0] = top;
}

static void KeySetupTransform(AES_state2 *s, const AES_state2 *r)
{
    for (int b = 0; b < 8; b++) {
        uint32_t v = s->slice[b] & 0x1111U;
        s->slice[b] = (((v >> 4U) | (v << 12U)) & 0x1111U) ^ (r->slice[b] & 1U);
    }
}

static void KeySetupColumnMix(AES_state2 *s, uint16_t r[8], const uint16_t a[8], int c1, int c2)
{
    for (int b = 0; b < 8; b++) {
        uint16_t col_a = (uint16_t)((a[b] >> c2) & 0x1111U);
        uint16_t mixed = (uint16_t)(((uint16_t)s->slice[b] & 0x1111U) ^ col_a);
        s->slice[b] = mixed;
        r[b] |= (uint16_t)((mixed & 0x1111U) << c1);
    }
}

static void GetOneColumn(AES_state2 *s, const uint16_t a[8], int c)
{
    for (int b = 0; b < 8; b++) {
        s->slice[b] = (uint32_t)((a[b] >> c) & 0x1111U);
    }
}

static void LoadByte(uint16_t rk[8], uint8_t byte, int r, int c)
{
    int shift = r * 4 + c;
    for (int b = 0; b < 8; b++) {
        rk[b] |= (uint16_t)(((byte >> b) & 1U) << shift);
    }
}

static void AES_setup(uint16_t rounds[][8], const uint8_t *key, int nkeywords, int nrounds)
{
    int i;
    AES_state2 rcon;
    memset(&rcon, 0, sizeof(rcon));
    rcon.slice[0] = 1U;
    int pos = 0;
    AES_state2 column;
    memset(&column, 0, sizeof(column));

    for (i = 0; i < nrounds + 1; i++) {
        for (int b = 0; b < 8; b++) {
            rounds[i][b] = 0U;
        }
    }

    for (i = 0; i < nkeywords; i++) {
        for (int r = 0; r < 4; r++) {
            LoadByte(rounds[i >> 2], *(key++), r, i & 3);
        }
    }

    GetOneColumn(&column, rounds[(nkeywords - 1) >> 2], (nkeywords - 1) & 3);

    for (i = nkeywords; i < 4 * (nrounds + 1); i++) {
        if (pos == 0) {
            SubBytes_enc2(&column);
            KeySetupTransform(&column, &rcon);
            MultX(&rcon);
        } else if (nkeywords > 6 && pos == 4) {
            SubBytes_enc2(&column);
            for (int b = 0; b < 8; b++) {
                column.slice[b] &= 0x1111U;
            }
        }
        if (++pos == nkeywords) {
            pos = 0;
        }
        KeySetupColumnMix(&column, rounds[i >> 2], rounds[(i - nkeywords) >> 2], i & 3,
                          (i - nkeywords) & 3);
    }

    syn_secure_zero_u32(rcon.slice, 8U);
    syn_secure_zero_u32(column.slice, 8U);
}

#endif /* defined(__AVR__) */

/* ── Key Expansion & Initialization ─────────────────────────────────────── */

SYN_Status syn_aes_init(SYN_AES_Context *ctx, const uint8_t *key, size_t key_len)
{
    if (ctx == NULL || key == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t nk = 0U;
    uint8_t nr = 0U;

    if (key_len == 16U) {
        nk = 4U;
        nr = 10U;
    } else if (key_len == 24U) {
        nk = 6U;
        nr = 12U;
    } else if (key_len == 32U) {
        nk = 8U;
        nr = 14U;
    } else {
        return SYN_INVALID_PARAM;
    }

#if (SYN_AES_MAX_KEY_BITS < 256)
    if ((key_len * 8U) > (size_t)SYN_AES_MAX_KEY_BITS) {
        return SYN_INVALID_PARAM;
    }
#endif

    ctx->nr = nr;

#if defined(__AVR__)
    memcpy(ctx->round_keys, key, key_len);

    uint8_t temp[4];
    uint8_t i = 0U;
    uint8_t k = (uint8_t)key_len;
    uint8_t total_bytes = (uint8_t)((nr + 1U) * 16U);

    while (k < total_bytes) {
        temp[0] = ctx->round_keys[k - 4U];
        temp[1] = ctx->round_keys[k - 3U];
        temp[2] = ctx->round_keys[k - 2U];
        temp[3] = ctx->round_keys[k - 1U];

        if ((k % (nk * 4U)) == 0U) {
            /* RotWord */
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;

            /* SubWord */
            temp[0] = SYN_AES_SBOX(temp[0]);
            temp[1] = SYN_AES_SBOX(temp[1]);
            temp[2] = SYN_AES_SBOX(temp[2]);
            temp[3] = SYN_AES_SBOX(temp[3]);

            i++;
            temp[0] ^= SYN_AES_RCON(i);
        } else if ((nk == 8U) && ((k % 32U) == 16U)) {
            /* SubWord for AES-256 word 4 */
            temp[0] = SYN_AES_SBOX(temp[0]);
            temp[1] = SYN_AES_SBOX(temp[1]);
            temp[2] = SYN_AES_SBOX(temp[2]);
            temp[3] = SYN_AES_SBOX(temp[3]);
        }

        ctx->round_keys[k] = (uint8_t)(ctx->round_keys[k - (nk * 4U)] ^ temp[0]);
        ctx->round_keys[k + 1U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 1U] ^ temp[1]);
        ctx->round_keys[k + 2U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 2U] ^ temp[2]);
        ctx->round_keys[k + 3U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 3U] ^ temp[3]);
        k = (uint8_t)(k + 4U);
    }

    syn_secure_zero(temp, sizeof(temp));
#else
#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    memset(ctx->round_keys, 0, sizeof(ctx->round_keys));
    memcpy(ctx->round_keys, key, key_len);
#endif
    AES_setup(ctx->rk_slice, key, (int)nk, (int)nr);
#endif
    return SYN_OK;
}

/* ── Block Encryption ───────────────────────────────────────────────────── */

void syn_aes_encrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE])
{
    if (ctx == NULL || in == NULL || out == NULL) {
        return;
    }

#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (syn_port_aes_encrypt_block(ctx->round_keys, ctx->nr, in, out) == SYN_OK) {
        return;
    }
#endif

#if defined(__AVR__)
    const uint8_t *rk = ctx->round_keys;
    uint32_t s0 = aes_load_be32(in + 0U) ^ aes_load_be32(rk + 0U);
    uint32_t s1 = aes_load_be32(in + 4U) ^ aes_load_be32(rk + 4U);
    uint32_t s2 = aes_load_be32(in + 8U) ^ aes_load_be32(rk + 8U);
    uint32_t s3 = aes_load_be32(in + 12U) ^ aes_load_be32(rk + 12U);

    for (uint8_t round = 1U; round < ctx->nr; round++) {
        rk += 16U;
        uint32_t t0 = aes_mix_col(aes_sub_shift(s0, s1, s2, s3)) ^ aes_load_be32(rk + 0U);
        uint32_t t1 = aes_mix_col(aes_sub_shift(s1, s2, s3, s0)) ^ aes_load_be32(rk + 4U);
        uint32_t t2 = aes_mix_col(aes_sub_shift(s2, s3, s0, s1)) ^ aes_load_be32(rk + 8U);
        uint32_t t3 = aes_mix_col(aes_sub_shift(s3, s0, s1, s2)) ^ aes_load_be32(rk + 12U);
        s0 = t0;
        s1 = t1;
        s2 = t2;
        s3 = t3;
    }

    rk += 16U;
    aes_store_be32(out + 0U, aes_sub_shift(s0, s1, s2, s3) ^ aes_load_be32(rk + 0U));
    aes_store_be32(out + 4U, aes_sub_shift(s1, s2, s3, s0) ^ aes_load_be32(rk + 4U));
    aes_store_be32(out + 8U, aes_sub_shift(s2, s3, s0, s1) ^ aes_load_be32(rk + 8U));
    aes_store_be32(out + 12U, aes_sub_shift(s3, s0, s1, s2) ^ aes_load_be32(rk + 12U));
#else
    AES_state2 s;
    LoadBytes1(&s, in);
    aes_ct_encrypt_core(&s, ctx->rk_slice, ctx->nr);
    SaveBytes1(out, &s);
    syn_secure_zero_u32(s.slice, 8U);
#endif
}

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
/* ── Block Decryption ───────────────────────────────────────────────────── */

void syn_aes_decrypt_block(const SYN_AES_Context *ctx, const uint8_t in[SYN_AES_BLOCK_SIZE],
                           uint8_t out[SYN_AES_BLOCK_SIZE])
{
    if (ctx == NULL || in == NULL || out == NULL) {
        return;
    }

#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (syn_port_aes_decrypt_block(ctx->round_keys, ctx->nr, in, out) == SYN_OK) {
        return;
    }
#endif

#if defined(__AVR__)
    uint8_t state[4][4];
    int last_rk = (int)ctx->nr * 16;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = (uint8_t)(in[r + 4 * c] ^ ctx->round_keys[last_rk + r + 4 * c]);
        }
    }

    for (int round = (int)ctx->nr - 1; round >= 0; round--) {
        /* InvShiftRows */
        uint8_t t;
        t = state[1][3];
        state[1][3] = state[1][2];
        state[1][2] = state[1][1];
        state[1][1] = state[1][0];
        state[1][0] = t;

        t = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = t;
        t = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = t;

        t = state[3][0];
        state[3][0] = state[3][1];
        state[3][1] = state[3][2];
        state[3][2] = state[3][3];
        state[3][3] = t;

        /* InvSubBytes */
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                state[r][c] = SYN_AES_RSBOX(state[r][c]);
            }
        }

        /* AddRoundKey */
        int rk_offset = round * 16;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                state[r][c] ^= ctx->round_keys[rk_offset + r + 4 * c];
            }
        }

        /* InvMixColumns (except last round) */
        if (round > 0) {
            for (int c = 0; c < 4; c++) {
                uint8_t a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
                state[0][c] = (uint8_t)(gmult(a0, 0x0e) ^ gmult(a1, 0x0b) ^ gmult(a2, 0x0d) ^
                                        gmult(a3, 0x09));
                state[1][c] = (uint8_t)(gmult(a0, 0x09) ^ gmult(a1, 0x0e) ^ gmult(a2, 0x0b) ^
                                        gmult(a3, 0x0d));
                state[2][c] = (uint8_t)(gmult(a0, 0x0d) ^ gmult(a1, 0x09) ^ gmult(a2, 0x0e) ^
                                        gmult(a3, 0x0b));
                state[3][c] = (uint8_t)(gmult(a0, 0x0b) ^ gmult(a1, 0x0d) ^ gmult(a2, 0x09) ^
                                        gmult(a3, 0x0e));
            }
        }
    }

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            out[r + 4 * c] = state[r][c];
        }
    }
    syn_secure_zero(state, sizeof(state));
#else
    AES_decrypt(ctx->rk_slice, (int)ctx->nr, out, in);
#endif
}
#endif

static void aes_encrypt_two_blocks(const SYN_AES_Context *ctx, const uint8_t in0[16],
                                   const uint8_t in1[16], uint8_t out0[16], uint8_t out1[16])
{
#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (syn_port_aes_encrypt_block(ctx->round_keys, ctx->nr, in0, out0) == SYN_OK) {
        (void)syn_port_aes_encrypt_block(ctx->round_keys, ctx->nr, in1, out1);
        return;
    }
#endif
#if defined(__AVR__)
    syn_aes_encrypt_block(ctx, in0, out0);
    syn_aes_encrypt_block(ctx, in1, out1);
#else
    AES_state2 s;
    LoadBytes2(&s, in0, in1);
    aes_ct_encrypt_core(&s, ctx->rk_slice, ctx->nr);
    SaveBytes2(out0, out1, &s);
    syn_secure_zero_u32(s.slice, 8U);
#endif
}

#if !defined(SYN_USE_AES_CBC) || SYN_USE_AES_CBC
/* ── CBC Mode ───────────────────────────────────────────────────────────── */

SYN_Status syn_aes_cbc_encrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len)
{
    if (ctx == NULL || iv == NULL || (in == NULL && in_len > 0U) || out == NULL ||
        out_len == NULL || in_len > (SIZE_MAX - 16U)) {
        return SYN_INVALID_PARAM;
    }

    size_t pad_len = 16U - (in_len % 16U);
    size_t total_len = in_len + pad_len;

    if (out_capacity < total_len) {
        return SYN_INVALID_PARAM;
    }

    uint8_t current_iv[16];
    memcpy(current_iv, iv, 16);

    uint8_t block[16];
    size_t offset = 0U;

    while (offset < total_len) {
        if ((in_len - offset) >= 16U) {
            memcpy(block, in + offset, 16);
        } else {
            size_t remaining = in_len - offset;
            if (remaining > 0U) {
                memcpy(block, in + offset, remaining);
            }
            memset(block + remaining, (uint8_t)pad_len, 16U - remaining);
        }

        for (int i = 0; i < 16; i++) {
            block[i] ^= current_iv[i];
        }

        syn_aes_encrypt_block(ctx, block, out + offset);
        memcpy(current_iv, out + offset, 16);
        offset += 16U;
    }

    syn_secure_zero(current_iv, sizeof(current_iv));
    syn_secure_zero(block, sizeof(block));
    *out_len = total_len;
    return SYN_OK;
}

#if !defined(SYN_USE_AES_DECRYPT) || SYN_USE_AES_DECRYPT
SYN_Status syn_aes_cbc_decrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len)
{
    if (ctx == NULL || iv == NULL || in == NULL || out == NULL || out_len == NULL || in_len == 0U ||
        (in_len % 16U) != 0U || out_capacity < in_len) {
        return SYN_INVALID_PARAM;
    }

    uint8_t current_iv[16];
    memcpy(current_iv, iv, 16);

    uint8_t block[16];
    size_t offset = 0U;

    while (offset < in_len) {
        syn_aes_decrypt_block(ctx, in + offset, block);
        for (int i = 0; i < 16; i++) {
            block[i] ^= current_iv[i];
        }
        memcpy(current_iv, in + offset, 16);
        memcpy(out + offset, block, 16);
        offset += 16U;
    }
    syn_secure_zero(current_iv, sizeof(current_iv));
    syn_secure_zero(block, sizeof(block));

    /* Constant-time PKCS#7 unpadding validation over the final 16-byte block */
    uint8_t pad_val = out[in_len - 1U];
    uint8_t bad = (uint8_t)(((uint32_t)pad_val - 1U) & 0xF0U);

    for (size_t i = 0U; i < 16U; i++) {
        uint32_t need_pad = ((uint32_t)(15U - i) - (uint32_t)pad_val) >> 31U;
        uint8_t mask = (uint8_t)(0U - need_pad);
        bad |= (uint8_t)((out[in_len - 16U + i] ^ pad_val) & mask);
    }

    if (bad != 0U) {
        syn_secure_zero(out, in_len);
        return SYN_INVALID_PARAM;
    }

    *out_len = in_len - (size_t)pad_val;
    return SYN_OK;
}
#endif
#endif

#if !defined(SYN_USE_AES_CTR) || SYN_USE_AES_CTR
/* ── CTR Mode ───────────────────────────────────────────────────────────── */

static void inc128(uint8_t counter[16])
{
    for (int i = 15; i >= 0; i--) {
        counter[i]++;
        if (counter[i] != 0U) {
            break;
        }
    }
}

SYN_Status syn_aes_ctr(const SYN_AES_Context *ctx, const uint8_t nonce[SYN_AES_BLOCK_SIZE],
                       const uint8_t *in, size_t len, uint8_t *out)
{
    if (ctx == NULL || nonce == NULL || (in == NULL && len > 0U) || (out == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    if (len == 0U) {
        return SYN_OK;
    }

    uint8_t cb[16];
    memcpy(cb, nonce, 16);

    uint8_t cb0[16];
    uint8_t cb1[16];
    uint8_t stream0[16];
    uint8_t stream1[16];
    size_t offset = 0U;

    while ((len - offset) > 16U) {
        memcpy(cb0, cb, 16);
        inc128(cb);
        memcpy(cb1, cb, 16);
        inc128(cb);
        aes_encrypt_two_blocks(ctx, cb0, cb1, stream0, stream1);
        for (size_t i = 0U; i < 16U; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
        }
        offset += 16U;
        size_t chunk1 = (len - offset >= 16U) ? 16U : (len - offset);
        for (size_t i = 0U; i < chunk1; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream1[i]);
        }
        offset += chunk1;
    }

    if (offset < len) {
        syn_aes_encrypt_block(ctx, cb, stream0);
        inc128(cb);
        size_t chunk = len - offset;
        for (size_t i = 0U; i < chunk; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
        }
    }

    syn_secure_zero(cb, sizeof(cb));
    syn_secure_zero(cb0, sizeof(cb0));
    syn_secure_zero(cb1, sizeof(cb1));
    syn_secure_zero(stream0, sizeof(stream0));
    syn_secure_zero(stream1, sizeof(stream1));
    return SYN_OK;
}
#endif

#if !defined(SYN_USE_AES_GCM) || SYN_USE_AES_GCM
/* ── GCM AEAD Mode ──────────────────────────────────────────────────────── */

static inline uint32_t ghash_rev32(uint32_t x)
{
    x = ((x & 0x55555555UL) << 1U) | ((x >> 1U) & 0x55555555UL);
    x = ((x & 0x33333333UL) << 2U) | ((x >> 2U) & 0x33333333UL);
    x = ((x & 0x0F0F0F0FUL) << 4U) | ((x >> 4U) & 0x0F0F0F0FUL);
    x = ((x & 0x00FF00FFUL) << 8U) | ((x >> 8U) & 0x00FF00FFUL);
    return (x << 16U) | (x >> 16U);
}

static inline uint64_t ghash_bmul32(uint32_t x, uint32_t y)
{
    uint32_t x0 = x & 0x11111111UL;
    uint32_t x1 = (x >> 1U) & 0x11111111UL;
    uint32_t x2 = (x >> 2U) & 0x11111111UL;
    uint32_t x3 = (x >> 3U) & 0x11111111UL;
    uint32_t y0 = y & 0x11111111UL;
    uint32_t y1 = (y >> 1U) & 0x11111111UL;
    uint32_t y2 = (y >> 2U) & 0x11111111UL;
    uint32_t y3 = (y >> 3U) & 0x11111111UL;

    uint64_t z0 = ((uint64_t)x0 * y0 ^
                   ((((uint64_t)x1 * y3) ^ ((uint64_t)x2 * y2) ^ ((uint64_t)x3 * y1)) << 4U)) &
                  0x1111111111111111ULL;
    uint64_t z1 = (((uint64_t)x0 * y1) ^ ((uint64_t)x1 * y0) ^
                   ((((uint64_t)x2 * y3) ^ ((uint64_t)x3 * y2)) << 4U)) &
                  0x1111111111111111ULL;
    uint64_t z2 = (((uint64_t)x0 * y2) ^ ((uint64_t)x1 * y1) ^ ((uint64_t)x2 * y0) ^
                   (((uint64_t)x3 * y3) << 4U)) &
                  0x1111111111111111ULL;
    uint64_t z3 =
        (((uint64_t)x0 * y3) ^ ((uint64_t)x1 * y2) ^ ((uint64_t)x2 * y1) ^ ((uint64_t)x3 * y0)) &
        0x1111111111111111ULL;

    return z0 | (z1 << 1U) | (z2 << 2U) | (z3 << 3U);
}

#if defined(__GNUC__) && !defined(__AVR__)
__attribute__((noinline))
#endif
static void
ghash_bmul64(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t r[4])
{
    uint64_t m0 = ghash_bmul32(x0, y0);
    uint64_t m2 = ghash_bmul32(x1, y1);
    uint64_t m1 = ghash_bmul32(x0 ^ x1, y0 ^ y1) ^ m0 ^ m2;

    r[0] = (uint32_t)m0;
    r[1] = (uint32_t)(m0 >> 32U) ^ (uint32_t)m1;
    r[2] = (uint32_t)m2 ^ (uint32_t)(m1 >> 32U);
    r[3] = (uint32_t)(m2 >> 32U);
}

static void ghash_mult_reduce(uint32_t a[4], const uint32_t b[4])
{
    uint32_t z0[4];
    uint32_t z2[4];
    uint32_t zm[4];

    ghash_bmul64(a[0], a[1], b[0], b[1], z0);
    ghash_bmul64(a[2], a[3], b[2], b[3], z2);
    ghash_bmul64(a[0] ^ a[2], a[1] ^ a[3], b[0] ^ b[2], b[1] ^ b[3], zm);

    uint32_t w0 = z0[0];
    uint32_t w1 = z0[1];
    uint32_t w2 = z0[2] ^ zm[0] ^ z0[0] ^ z2[0];
    uint32_t w3 = z0[3] ^ zm[1] ^ z0[1] ^ z2[1];
    uint32_t w4 = z2[0] ^ zm[2] ^ z0[2] ^ z2[2];
    uint32_t w5 = z2[1] ^ zm[3] ^ z0[3] ^ z2[3];
    uint32_t w6 = z2[2];
    uint32_t w7 = z2[3];

    w3 ^= w7 ^ (w7 << 1U) ^ (w7 << 2U) ^ (w7 << 7U);
    w4 ^= (w7 >> 31U) ^ (w7 >> 30U) ^ (w7 >> 25U);
    w2 ^= w6 ^ (w6 << 1U) ^ (w6 << 2U) ^ (w6 << 7U);
    w3 ^= (w6 >> 31U) ^ (w6 >> 30U) ^ (w6 >> 25U);
    w1 ^= w5 ^ (w5 << 1U) ^ (w5 << 2U) ^ (w5 << 7U);
    w2 ^= (w5 >> 31U) ^ (w5 >> 30U) ^ (w5 >> 25U);
    w0 ^= w4 ^ (w4 << 1U) ^ (w4 << 2U) ^ (w4 << 7U);
    w1 ^= (w4 >> 31U) ^ (w4 >> 30U) ^ (w4 >> 25U);

    a[0] = w0;
    a[1] = w1;
    a[2] = w2;
    a[3] = w3;
}

static void ghash_step(const SYN_AES_GCM_Context *ctx, const uint8_t block[16], uint32_t yw[4])
{
#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (ctx->hw_ghash) {
        uint8_t y_in[16];
        uint8_t y_out[16];
        for (size_t i = 0U; i < 4U; i++) {
            aes_store_be32(y_in + (i * 4U), ghash_rev32(yw[i]));
        }
        for (size_t i = 0U; i < 16U; i++) {
            y_in[i] ^= block[i];
        }
        (void)syn_port_ghash_mult(y_in, ctx->h, y_out);
        for (size_t i = 0U; i < 4U; i++) {
            yw[i] = ghash_rev32(aes_load_be32(y_out + (i * 4U)));
        }
        syn_secure_zero(y_in, sizeof(y_in));
        syn_secure_zero(y_out, sizeof(y_out));
        return;
    }
#endif
    yw[0] ^= ghash_rev32(aes_load_be32(block + 0U));
    yw[1] ^= ghash_rev32(aes_load_be32(block + 4U));
    yw[2] ^= ghash_rev32(aes_load_be32(block + 8U));
    yw[3] ^= ghash_rev32(aes_load_be32(block + 12U));
    ghash_mult_reduce(yw, ctx->h_rev);
}

static void ghash_mult_bit(const uint8_t x[16], const uint8_t y[16], uint8_t out[16])
{
#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (syn_port_ghash_mult(x, y, out) == SYN_OK) {
        return;
    }
#endif

    uint32_t a[4];
    uint32_t b[4];

    for (size_t i = 0U; i < 4U; i++) {
        a[i] = ghash_rev32(aes_load_be32(x + (i * 4U)));
        b[i] = ghash_rev32(aes_load_be32(y + (i * 4U)));
    }

    ghash_mult_reduce(a, b);

    for (size_t i = 0U; i < 4U; i++) {
        aes_store_be32(out + (i * 4U), ghash_rev32(a[i]));
    }

    syn_secure_zero_u32(a, 4U);
    syn_secure_zero_u32(b, 4U);
}

void syn_aes_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16])
{
    if (x == NULL || h == NULL || out == NULL) {
        return;
    }
    ghash_mult_bit(x, h, out);
}

static void inc32(uint8_t block[16])
{
    for (int i = 15; i >= 12; i--) {
        block[i]++;
        if (block[i] != 0U) {
            break;
        }
    }
}

static void put_be64(uint8_t out[8], uint64_t val)
{
    out[0] = (uint8_t)((val >> 56) & 0xffU);
    out[1] = (uint8_t)((val >> 48) & 0xffU);
    out[2] = (uint8_t)((val >> 40) & 0xffU);
    out[3] = (uint8_t)((val >> 32) & 0xffU);
    out[4] = (uint8_t)((val >> 24) & 0xffU);
    out[5] = (uint8_t)((val >> 16) & 0xffU);
    out[6] = (uint8_t)((val >> 8) & 0xffU);
    out[7] = (uint8_t)(val & 0xffU);
}

static void ghash_process_data(const SYN_AES_GCM_Context *ctx, const uint8_t *data, size_t len,
                               uint32_t yw[4])
{
    size_t offset = 0U;
    uint8_t blk[16];
    while (offset < len) {
        size_t chunk = (len - offset >= 16U) ? 16U : (len - offset);
        if (chunk == 16U) {
            ghash_step(ctx, data + offset, yw);
        } else {
            memset(blk, 0, 16);
            memcpy(blk, data + offset, chunk);
            ghash_step(ctx, blk, yw);
            syn_secure_zero(blk, sizeof(blk));
        }
        offset += chunk;
    }
}

SYN_Status syn_aes_gcm_init(SYN_AES_GCM_Context *ctx, const uint8_t *key, size_t key_len)
{
    if (ctx == NULL || key == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_Status st = syn_aes_init(&ctx->aes, key, key_len);
    if (st != SYN_OK) {
        return st;
    }

    const uint8_t zero_block[16] = {0};
    syn_aes_encrypt_block(&ctx->aes, zero_block, ctx->h);

    for (size_t i = 0U; i < 4U; i++) {
        ctx->h_rev[i] = ghash_rev32(aes_load_be32(ctx->h + (i * 4U)));
    }

#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    uint8_t h_probe[16];
    ctx->hw_ghash = (syn_port_ghash_mult(zero_block, ctx->h, h_probe) == SYN_OK);
    syn_secure_zero(h_probe, sizeof(h_probe));
#endif

#if (SYN_AES_GCM_TABLE == 4)
    memset(ctx->htable[0], 0, 16);
    for (uint8_t k = 1U; k < 16U; k++) {
        uint8_t nibble_block[16] = {0};
        nibble_block[0] = (uint8_t)(k << 4);
        ghash_mult_bit(nibble_block, ctx->h, ctx->htable[k]);
    }
#elif (SYN_AES_GCM_TABLE == 8)
    memset(ctx->htable[0], 0, 16);
    for (int k = 1; k < 256; k++) {
        uint8_t byte_block[16] = {0};
        byte_block[0] = (uint8_t)k;
        ghash_mult_bit(byte_block, ctx->h, ctx->htable[k]);
    }
#endif

    return SYN_OK;
}

static void gcm_compute_j0(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                           uint8_t j0[16])
{
    if (nonce_len == 12U) {
        memcpy(j0, nonce, 12);
        j0[12] = 0U;
        j0[13] = 0U;
        j0[14] = 0U;
        j0[15] = 1U;
    } else {
        uint32_t yw[4] = {0U, 0U, 0U, 0U};
        ghash_process_data(ctx, nonce, nonce_len, yw);
        uint8_t len_block[16] = {0};
        put_be64(&len_block[8], (uint64_t)nonce_len * 8ULL);
        ghash_step(ctx, len_block, yw);
        for (size_t i = 0U; i < 4U; i++) {
            aes_store_be32(j0 + (i * 4U), ghash_rev32(yw[i]));
        }
        syn_secure_zero_u32(yw, 4U);
        syn_secure_zero(len_block, sizeof(len_block));
    }
}

static bool gcm_params_valid(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                             const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                             const uint8_t *out, const uint8_t *tag)
{
    if (ctx == NULL || nonce == NULL || nonce_len == 0U || tag == NULL) {
        return false;
    }
    if ((aad == NULL && aad_len > 0U) || (in == NULL && in_len > 0U) ||
        (out == NULL && in_len > 0U)) {
        return false;
    }
#if (SIZE_MAX > 0xFFFFFFFFU)
    if (((uint64_t)in_len > 0xFFFFFFFE0ULL) || ((uint64_t)aad_len > (UINT64_MAX / 8ULL)) ||
        ((uint64_t)nonce_len > (UINT64_MAX / 8ULL))) {
        return false;
    }
#endif
    return true;
}

SYN_Status syn_aes_gcm_encrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               uint8_t tag[SYN_AES_GCM_TAG_SIZE])
{
    if (!gcm_params_valid(ctx, nonce, nonce_len, aad, aad_len, in, in_len, out, tag)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t j0[16];
    gcm_compute_j0(ctx, nonce, nonce_len, j0);

    /* Encrypt plaintext with CTR mode starting at inc32(J0) */
    uint8_t cb[16];
    memcpy(cb, j0, 16);
    inc32(cb);

    uint8_t j0_enc[16];
    uint8_t stream0[16];
    uint8_t stream1[16];
    uint8_t cb0[16];
    uint8_t cb1[16];
    size_t offset = 0U;

    if (in_len > 0U) {
        memcpy(cb0, cb, 16);
        inc32(cb);
        aes_encrypt_two_blocks(&ctx->aes, j0, cb0, j0_enc, stream0);
        size_t chunk0 = (in_len >= 16U) ? 16U : in_len;
        for (size_t i = 0U; i < chunk0; i++) {
            out[i] = (uint8_t)(in[i] ^ stream0[i]);
        }
        offset += chunk0;

        while ((in_len - offset) > 16U) {
            memcpy(cb0, cb, 16);
            inc32(cb);
            memcpy(cb1, cb, 16);
            inc32(cb);
            aes_encrypt_two_blocks(&ctx->aes, cb0, cb1, stream0, stream1);
            for (size_t i = 0U; i < 16U; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
            }
            offset += 16U;
            size_t chunk1 = (in_len - offset >= 16U) ? 16U : (in_len - offset);
            for (size_t i = 0U; i < chunk1; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream1[i]);
            }
            offset += chunk1;
        }

        if (offset < in_len) {
            syn_aes_encrypt_block(&ctx->aes, cb, stream0);
            inc32(cb);
            size_t chunk = in_len - offset;
            for (size_t i = 0U; i < chunk; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
            }
        }
    } else {
        syn_aes_encrypt_block(&ctx->aes, j0, j0_enc);
    }

    /* Compute GHASH over AAD and Ciphertext */
    uint32_t yw[4] = {0U, 0U, 0U, 0U};
    if (aad_len > 0U) {
        ghash_process_data(ctx, aad, aad_len, yw);
    }
    if (in_len > 0U) {
        ghash_process_data(ctx, out, in_len, yw);
    }

    /* Append Length block: [8 * aad_len]_64 || [8 * in_len]_64 */
    uint8_t len_block[16];
    put_be64(&len_block[0], (uint64_t)aad_len * 8ULL);
    put_be64(&len_block[8], (uint64_t)in_len * 8ULL);
    ghash_step(ctx, len_block, yw);

    /* Tag = GHASH_out ^ AES(K, J0) */
    uint8_t ghash_out[16];
    for (size_t i = 0U; i < 4U; i++) {
        aes_store_be32(ghash_out + (i * 4U), ghash_rev32(yw[i]));
    }
    for (int j = 0; j < 16; j++) {
        tag[j] = (uint8_t)(ghash_out[j] ^ j0_enc[j]);
    }

    syn_secure_zero(j0, sizeof(j0));
    syn_secure_zero(cb, sizeof(cb));
    syn_secure_zero(cb0, sizeof(cb0));
    syn_secure_zero(cb1, sizeof(cb1));
    syn_secure_zero(stream0, sizeof(stream0));
    syn_secure_zero(stream1, sizeof(stream1));
    syn_secure_zero_u32(yw, 4U);
    syn_secure_zero(len_block, sizeof(len_block));
    syn_secure_zero(ghash_out, sizeof(ghash_out));
    syn_secure_zero(j0_enc, sizeof(j0_enc));
    return SYN_OK;
}

SYN_Status syn_aes_gcm_decrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               const uint8_t tag[SYN_AES_GCM_TAG_SIZE])
{
    if (!gcm_params_valid(ctx, nonce, nonce_len, aad, aad_len, in, in_len, out, tag)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t j0[16];
    gcm_compute_j0(ctx, nonce, nonce_len, j0);

    /* Compute GHASH over AAD and Ciphertext */
    uint32_t yw[4] = {0U, 0U, 0U, 0U};
    if (aad_len > 0U) {
        ghash_process_data(ctx, aad, aad_len, yw);
    }
    if (in_len > 0U) {
        ghash_process_data(ctx, in, in_len, yw);
    }

    /* Append Length block */
    uint8_t len_block[16];
    put_be64(&len_block[0], (uint64_t)aad_len * 8ULL);
    put_be64(&len_block[8], (uint64_t)in_len * 8ULL);
    ghash_step(ctx, len_block, yw);

    uint8_t ghash_out[16];
    for (size_t i = 0U; i < 4U; i++) {
        aes_store_be32(ghash_out + (i * 4U), ghash_rev32(yw[i]));
    }

    uint8_t cb[16];
    memcpy(cb, j0, 16);
    inc32(cb);

    uint8_t j0_enc[16];
    uint8_t stream0[16];
    uint8_t stream1[16];
    uint8_t cb0[16];
    uint8_t cb1[16];

    if (in_len > 0U) {
        memcpy(cb0, cb, 16);
        inc32(cb);
        aes_encrypt_two_blocks(&ctx->aes, j0, cb0, j0_enc, stream0);
    } else {
        syn_aes_encrypt_block(&ctx->aes, j0, j0_enc);
    }

    /* Expected Tag = GHASH_out ^ AES(K, J0) */
    uint8_t expected_tag[16];
    for (int j = 0; j < 16; j++) {
        expected_tag[j] = (uint8_t)(ghash_out[j] ^ j0_enc[j]);
    }

    /* Constant-time tag comparison */
    uint8_t diff = 0U;
    for (size_t i = 0U; i < SYN_AES_GCM_TAG_SIZE; i++) {
        diff |= (uint8_t)(expected_tag[i] ^ tag[i]);
    }

    syn_secure_zero_u32(yw, 4U);
    syn_secure_zero(len_block, sizeof(len_block));
    syn_secure_zero(ghash_out, sizeof(ghash_out));
    syn_secure_zero(j0_enc, sizeof(j0_enc));
    syn_secure_zero(expected_tag, sizeof(expected_tag));

    if (diff != 0U) {
        syn_secure_zero(j0, sizeof(j0));
        syn_secure_zero(cb, sizeof(cb));
        syn_secure_zero(cb0, sizeof(cb0));
        syn_secure_zero(stream0, sizeof(stream0));
        if (out != NULL && out != in && in_len > 0U) {
            memset(out, 0, in_len);
        }
        return SYN_ERROR;
    }

    /* Decrypt ciphertext with CTR mode starting at inc32(J0) */
    size_t offset = 0U;
    if (in_len > 0U) {
        size_t chunk0 = (in_len >= 16U) ? 16U : in_len;
        for (size_t i = 0U; i < chunk0; i++) {
            out[i] = (uint8_t)(in[i] ^ stream0[i]);
        }
        offset += chunk0;

        while ((in_len - offset) > 16U) {
            memcpy(cb0, cb, 16);
            inc32(cb);
            memcpy(cb1, cb, 16);
            inc32(cb);
            aes_encrypt_two_blocks(&ctx->aes, cb0, cb1, stream0, stream1);
            for (size_t i = 0U; i < 16U; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
            }
            offset += 16U;
            size_t chunk1 = (in_len - offset >= 16U) ? 16U : (in_len - offset);
            for (size_t i = 0U; i < chunk1; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream1[i]);
            }
            offset += chunk1;
        }

        if (offset < in_len) {
            syn_aes_encrypt_block(&ctx->aes, cb, stream0);
            inc32(cb);
            size_t chunk = in_len - offset;
            for (size_t i = 0U; i < chunk; i++) {
                out[offset + i] = (uint8_t)(in[offset + i] ^ stream0[i]);
            }
        }
    }

    syn_secure_zero(j0, sizeof(j0));
    syn_secure_zero(cb, sizeof(cb));
    syn_secure_zero(cb0, sizeof(cb0));
    syn_secure_zero(cb1, sizeof(cb1));
    syn_secure_zero(stream0, sizeof(stream0));
    syn_secure_zero(stream1, sizeof(stream1));
    return SYN_OK;
}
#endif

#if !defined(SYN_USE_AES_CCM) || SYN_USE_AES_CCM
static void ccm_mac_feed(const SYN_AES_Context *ctx, uint8_t mac[16], uint8_t *blk, size_t *blk_len,
                         const uint8_t *data, size_t len)
{
    for (size_t i = 0U; i < len; i++) {
        blk[(*blk_len)++] = data[i];
        if (*blk_len == 16U) {
            for (int j = 0; j < 16; j++) {
                mac[j] ^= blk[j];
            }
            syn_aes_encrypt_block(ctx, mac, mac);
            *blk_len = 0U;
        }
    }
}

static void ccm_mac_pad_zero(const SYN_AES_Context *ctx, uint8_t mac[16], uint8_t *blk,
                             size_t *blk_len)
{
    if (*blk_len > 0U) {
        while (*blk_len < 16U) {
            blk[(*blk_len)++] = 0U;
        }
        for (int j = 0; j < 16; j++) {
            mac[j] ^= blk[j];
        }
        syn_aes_encrypt_block(ctx, mac, mac);
        *blk_len = 0U;
    }
}

static void ccm_format_ctr(uint8_t a[16], const uint8_t *nonce, size_t nonce_len, size_t L,
                           uint64_t counter)
{
    a[0] = (uint8_t)(L - 1U);
    memcpy(a + 1, nonce, nonce_len);
    for (size_t i = 0U; i < L; i++) {
        a[15U - i] = (uint8_t)((counter >> (8U * i)) & 0xFFU);
    }
}

static SYN_Status ccm_compute_mac(const SYN_AES_Context *ctx, const uint8_t *nonce,
                                  size_t nonce_len, size_t L, const uint8_t *aad, size_t aad_len,
                                  const uint8_t *data, size_t data_len, size_t tag_len,
                                  uint8_t mac[16])
{
    /* Format B0 */
    uint8_t b0[16];
    uint8_t flags = (uint8_t)((L - 1U) & 0x07U);
    flags |= (uint8_t)(((tag_len - 2U) / 2U) << 3U);
    if (aad_len > 0U) {
        flags |= 0x40U;
    }
    b0[0] = flags;
    memcpy(b0 + 1, nonce, nonce_len);
    for (size_t i = 0U; i < L; i++) {
        b0[15U - i] = (uint8_t)(((uint64_t)data_len >> (8U * i)) & 0xFFU);
    }

    syn_aes_encrypt_block(ctx, b0, mac);

    uint8_t blk[16];
    size_t blk_len = 0U;

    /* Process Associated Data */
    if (aad_len > 0U) {
        uint8_t hdr[10];
        size_t hdr_len = 0U;
        uint32_t u_aad_len = (uint32_t)aad_len;
        if (u_aad_len < 65280UL) {
            hdr[0] = (uint8_t)((u_aad_len >> 8U) & 0xFFU);
            hdr[1] = (uint8_t)(u_aad_len & 0xFFU);
            hdr_len = 2U;
        } else {
            hdr[0] = 0xFFU;
            hdr[1] = 0xFEU;
            hdr[2] = (uint8_t)((u_aad_len >> 24U) & 0xFFU);
            hdr[3] = (uint8_t)((u_aad_len >> 16U) & 0xFFU);
            hdr[4] = (uint8_t)((u_aad_len >> 8U) & 0xFFU);
            hdr[5] = (uint8_t)(u_aad_len & 0xFFU);
            hdr_len = 6U;
        }
        ccm_mac_feed(ctx, mac, blk, &blk_len, hdr, hdr_len);
        ccm_mac_feed(ctx, mac, blk, &blk_len, aad, aad_len);
        ccm_mac_pad_zero(ctx, mac, blk, &blk_len);
    }

    /* Process Payload */
    if (data_len > 0U) {
        ccm_mac_feed(ctx, mac, blk, &blk_len, data, data_len);
        ccm_mac_pad_zero(ctx, mac, blk, &blk_len);
    }

    syn_secure_zero(b0, sizeof(b0));
    syn_secure_zero(blk, sizeof(blk));
    return SYN_OK;
}

static bool ccm_lengths_valid(size_t nonce_len, size_t aad_len, size_t in_len, size_t tag_len)
{
    if (nonce_len < 7U || nonce_len > 13U || tag_len < 4U || tag_len > 16U ||
        (tag_len % 2U) != 0U) {
        return false;
    }
#if (SIZE_MAX > 0xFFFFFFFFU)
    if ((uint64_t)aad_len >= 0x100000000ULL) {
        return false; /* LCOV_EXCL_LINE */
    }
#else
    (void)aad_len;
#endif
    size_t L = 15U - nonce_len;
    if (L < 8U && (uint64_t)in_len >= (1ULL << (8U * L))) {
        return false;
    }
    return true;
}

static bool ccm_params_valid(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                             const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                             const uint8_t *out, const uint8_t *tag, size_t tag_len)
{
    if (ctx == NULL || nonce == NULL || tag == NULL ||
        !ccm_lengths_valid(nonce_len, aad_len, in_len, tag_len)) {
        return false;
    }
    if ((in_len > 0U && (in == NULL || out == NULL)) || (aad_len > 0U && aad == NULL)) {
        return false;
    }
    return true;
}

SYN_Status syn_aes_ccm_encrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               uint8_t *out, uint8_t *tag, size_t tag_len)
{
    if (!ccm_params_valid(ctx, nonce, nonce_len, aad, aad_len, in, in_len, out, tag, tag_len)) {
        return SYN_INVALID_PARAM;
    }

    size_t L = 15U - nonce_len;

    /* 1. Compute CBC-MAC */
    uint8_t mac[16];
    ccm_compute_mac(ctx, nonce, nonce_len, L, aad, aad_len, in, in_len, tag_len, mac);

    /* 2. CTR Payload Encryption */
    uint8_t ai[16] = {0};
    uint8_t si[16] = {0};
    uint64_t ctr = 1U;
    size_t offset = 0U;
    while (offset < in_len) {
        ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
        syn_aes_encrypt_block(ctx, ai, si);
        size_t chunk = (in_len - offset >= 16U) ? 16U : (in_len - offset);
        for (size_t j = 0U; j < chunk; j++) {
            out[offset + j] = (uint8_t)(in[offset + j] ^ si[j]);
        }
        offset += chunk;
    }

    /* 3. Encrypt Tag with S0 */
    uint8_t a0[16];
    uint8_t s0[16];
    ccm_format_ctr(a0, nonce, nonce_len, L, 0U);
    syn_aes_encrypt_block(ctx, a0, s0);
    for (size_t j = 0U; j < tag_len; j++) {
        tag[j] = (uint8_t)(mac[j] ^ s0[j]);
    }

    syn_secure_zero(mac, sizeof(mac));
    syn_secure_zero(ai, sizeof(ai));
    syn_secure_zero(si, sizeof(si));
    syn_secure_zero(a0, sizeof(a0));
    syn_secure_zero(s0, sizeof(s0));
    return SYN_OK;
}

SYN_Status syn_aes_ccm_decrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               const uint8_t *tag, size_t tag_len, uint8_t *out)
{
    if (!ccm_params_valid(ctx, nonce, nonce_len, aad, aad_len, in, in_len, out, tag, tag_len)) {
        return SYN_INVALID_PARAM;
    }

    size_t L = 15U - nonce_len;

    /* 1. Compute S0 and unmask expected tag T */
    uint8_t a0[16];
    uint8_t s0[16];
    ccm_format_ctr(a0, nonce, nonce_len, L, 0U);
    syn_aes_encrypt_block(ctx, a0, s0);
    uint8_t unmasked_tag[16];
    for (size_t j = 0U; j < tag_len; j++) {
        unmasked_tag[j] = (uint8_t)(tag[j] ^ s0[j]);
    }
    syn_secure_zero(a0, sizeof(a0));
    syn_secure_zero(s0, sizeof(s0));

    /* 2. CTR Decrypt Ciphertext */
    uint8_t ai[16] = {0};
    uint8_t si[16] = {0};
    uint64_t ctr = 1U;
    size_t offset = 0U;
    while (offset < in_len) {
        ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
        syn_aes_encrypt_block(ctx, ai, si);
        size_t chunk = (in_len - offset >= 16U) ? 16U : (in_len - offset);
        for (size_t j = 0U; j < chunk; j++) {
            out[offset + j] = (uint8_t)(in[offset + j] ^ si[j]);
        }
        offset += chunk;
    }

    /* 3. Recompute CBC-MAC over Decrypted Plaintext */
    uint8_t mac[16];
    ccm_compute_mac(ctx, nonce, nonce_len, L, aad, aad_len, out, in_len, tag_len, mac);

    /* 4. Constant-Time Tag Verification */
    uint8_t diff = 0U;
    for (size_t j = 0U; j < tag_len; j++) {
        diff |= (uint8_t)(unmasked_tag[j] ^ mac[j]);
    }
    syn_secure_zero(unmasked_tag, sizeof(unmasked_tag));
    syn_secure_zero(mac, sizeof(mac));

    if (diff != 0U) {
        if (out == in) {
            /* In-place decryption failed: invert CTR keystream to restore original ciphertext */
            ctr = 1U;
            offset = 0U;
            while (offset < in_len) {
                ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
                syn_aes_encrypt_block(ctx, ai, si);
                size_t chunk = (in_len - offset >= 16U) ? 16U : (in_len - offset);
                for (size_t j = 0U; j < chunk; j++) {
                    out[offset + j] = (uint8_t)(out[offset + j] ^ si[j]);
                }
                offset += chunk;
            }
        } else if (out != NULL && in_len > 0U) {
            memset(out, 0, in_len);
        }
        syn_secure_zero(ai, sizeof(ai));
        syn_secure_zero(si, sizeof(si));
        return SYN_ERROR;
    }

    syn_secure_zero(ai, sizeof(ai));
    syn_secure_zero(si, sizeof(si));
    return SYN_OK;
}
#endif

#endif /* SYN_USE_AES */
```


