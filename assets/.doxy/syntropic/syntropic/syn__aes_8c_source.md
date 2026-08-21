

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

/* ── AES Forward S-Box ───────────────────────────────────────────────────── */

static const uint8_t sbox[256] = {
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
/* ── AES Inverse S-Box ──────────────────────────────────────────────────── */

static const uint8_t rsbox[256] = {
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

/* ── AES Round Constants ─────────────────────────────────────────────────── */

static const uint8_t rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/* ── Galois Field Helpers ───────────────────────────────────────────────── */

static inline uint8_t gmult2(uint8_t a)
{
    return (uint8_t)((a << 1) ^ (((a >> 7) & 1U) * 0x1bU));
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
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];

            i++;
            temp[0] ^= rcon[i];
        } else if ((nk == 8U) && ((k % 32U) == 16U)) {
            /* SubWord for AES-256 word 4 */
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
        }

        ctx->round_keys[k] = (uint8_t)(ctx->round_keys[k - (nk * 4U)] ^ temp[0]);
        ctx->round_keys[k + 1U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 1U] ^ temp[1]);
        ctx->round_keys[k + 2U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 2U] ^ temp[2]);
        ctx->round_keys[k + 3U] = (uint8_t)(ctx->round_keys[k - (nk * 4U) + 3U] ^ temp[3]);
        k = (uint8_t)(k + 4U);
    }

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

    uint8_t state[4][4];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = (uint8_t)(in[r + 4 * c] ^ ctx->round_keys[r + 4 * c]);
        }
    }

    for (int round = 1; round <= (int)ctx->nr; round++) {
        /* SubBytes */
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                state[r][c] = sbox[state[r][c]];
            }
        }

        /* ShiftRows */
        uint8_t t;
        t = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = t;

        t = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = t;
        t = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = t;

        t = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = t;

        /* MixColumns (except last round) */
        if (round < (int)ctx->nr) {
            for (int c = 0; c < 4; c++) {
                uint8_t a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
                state[0][c] = (uint8_t)(gmult2(a0) ^ gmult2(a1) ^ a1 ^ a2 ^ a3);
                state[1][c] = (uint8_t)(a0 ^ gmult2(a1) ^ gmult2(a2) ^ a2 ^ a3);
                state[2][c] = (uint8_t)(a0 ^ a1 ^ gmult2(a2) ^ gmult2(a3) ^ a3);
                state[3][c] = (uint8_t)(gmult2(a0) ^ a0 ^ a1 ^ a2 ^ gmult2(a3));
            }
        }

        /* AddRoundKey */
        int rk_offset = round * 16;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                state[r][c] ^= ctx->round_keys[rk_offset + r + 4 * c];
            }
        }
    }

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            out[r + 4 * c] = state[r][c];
        }
    }
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
                state[r][c] = rsbox[state[r][c]];
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
}
#endif

#if !defined(SYN_USE_AES_CBC) || SYN_USE_AES_CBC
/* ── CBC Mode ───────────────────────────────────────────────────────────── */

SYN_Status syn_aes_cbc_encrypt(const SYN_AES_Context *ctx, const uint8_t iv[SYN_AES_BLOCK_SIZE],
                               const uint8_t *in, size_t in_len, uint8_t *out, size_t out_capacity,
                               size_t *out_len)
{
    if (ctx == NULL || iv == NULL || (in == NULL && in_len > 0U) || out == NULL ||
        out_len == NULL) {
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
        if (offset + 16U <= in_len) {
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

    /* PKCS#7 Unpadding validation */
    uint8_t pad_val = out[in_len - 1U];
    if (pad_val == 0U || pad_val > 16U || (size_t)pad_val > in_len) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = in_len - (size_t)pad_val; i < in_len; i++) {
        if (out[i] != pad_val) {
            return SYN_INVALID_PARAM;
        }
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

    uint8_t stream[16];
    size_t offset = 0U;

    while (offset < len) {
        syn_aes_encrypt_block(ctx, cb, stream);
        inc128(cb);

        size_t chunk = (len - offset >= 16U) ? 16U : (len - offset);
        for (size_t i = 0U; i < chunk; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream[i]);
        }
        offset += chunk;
    }

    return SYN_OK;
}
#endif

#if !defined(SYN_USE_AES_GCM) || SYN_USE_AES_GCM
/* ── GCM AEAD Mode ──────────────────────────────────────────────────────── */

static void ghash_mult_bit(const uint8_t x[16], const uint8_t y[16], uint8_t out[16])
{
#if defined(SYN_USE_PORT_AES) && SYN_USE_PORT_AES
    if (syn_port_ghash_mult(x, y, out) == SYN_OK) {
        return;
    }
#endif

    uint8_t v[16];
    uint8_t z[16] = {0};
    memcpy(v, y, 16);

    for (int i = 0; i < 16; i++) {
        uint8_t byte_val = x[i];
        for (int b = 7; b >= 0; b--) {
            if (((byte_val >> b) & 1U) != 0U) {
                for (int j = 0; j < 16; j++) {
                    z[j] ^= v[j];
                }
            }
            uint8_t carry = 0U;
            for (int j = 0; j < 16; j++) {
                uint8_t next_carry = (uint8_t)(v[j] & 1U);
                v[j] = (uint8_t)((v[j] >> 1) | carry);
                carry = (uint8_t)(next_carry << 7);
            }
            if (carry != 0U) {
                v[0] ^= 0xe1U;
            }
        }
    }
    memcpy(out, z, 16);
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

static void ghash_process_blocks(const SYN_AES_GCM_Context *ctx, const uint8_t *data, size_t len,
                                 uint8_t y[16])
{
    size_t offset = 0U;
    while (offset < len) {
        size_t chunk = (len - offset >= 16U) ? 16U : (len - offset);
        uint8_t block[16] = {0};
        memcpy(block, data + offset, chunk);
        for (int j = 0; j < 16; j++) {
            y[j] ^= block[j];
        }
        uint8_t tmp[16];
        ghash_mult_bit(y, ctx->h, tmp);
        memcpy(y, tmp, 16);
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
        memset(j0, 0, 16);
        ghash_process_blocks(ctx, nonce, nonce_len, j0);
        uint8_t len_block[16] = {0};
        put_be64(&len_block[8], (uint64_t)nonce_len * 8ULL);
        for (int j = 0; j < 16; j++) {
            j0[j] ^= len_block[j];
        }
        uint8_t tmp[16];
        ghash_mult_bit(j0, ctx->h, tmp);
        memcpy(j0, tmp, 16);
    }
}

SYN_Status syn_aes_gcm_encrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               uint8_t tag[SYN_AES_GCM_TAG_SIZE])
{
    if (ctx == NULL || (nonce == NULL && nonce_len > 0U) || nonce_len == 0U ||
        (aad == NULL && aad_len > 0U) || (in == NULL && in_len > 0U) ||
        (out == NULL && in_len > 0U) || tag == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t j0[16];
    gcm_compute_j0(ctx, nonce, nonce_len, j0);

    /* Encrypt plaintext with CTR mode starting at inc32(J0) */
    uint8_t cb[16];
    memcpy(cb, j0, 16);
    inc32(cb);

    uint8_t stream[16];
    size_t offset = 0U;
    while (offset < in_len) {
        syn_aes_encrypt_block(&ctx->aes, cb, stream);
        inc32(cb);
        size_t chunk = (in_len - offset >= 16U) ? 16U : (in_len - offset);
        for (size_t i = 0U; i < chunk; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream[i]);
        }
        offset += chunk;
    }

    /* Compute GHASH over AAD and Ciphertext */
    uint8_t s[16] = {0};
    if (aad_len > 0U) {
        ghash_process_blocks(ctx, aad, aad_len, s);
    }
    if (in_len > 0U) {
        ghash_process_blocks(ctx, out, in_len, s);
    }

    /* Append Length block: [8 * aad_len]_64 || [8 * in_len]_64 */
    uint8_t len_block[16];
    put_be64(&len_block[0], (uint64_t)aad_len * 8ULL);
    put_be64(&len_block[8], (uint64_t)in_len * 8ULL);
    for (int j = 0; j < 16; j++) {
        s[j] ^= len_block[j];
    }
    uint8_t ghash_out[16];
    ghash_mult_bit(s, ctx->h, ghash_out);

    /* Tag = GHASH_out ^ AES(K, J0) */
    uint8_t j0_enc[16];
    syn_aes_encrypt_block(&ctx->aes, j0, j0_enc);
    for (int j = 0; j < 16; j++) {
        tag[j] = (uint8_t)(ghash_out[j] ^ j0_enc[j]);
    }

    return SYN_OK;
}

SYN_Status syn_aes_gcm_decrypt(const SYN_AES_GCM_Context *ctx, const uint8_t *nonce,
                               size_t nonce_len, const uint8_t *aad, size_t aad_len,
                               const uint8_t *in, size_t in_len, uint8_t *out,
                               const uint8_t tag[SYN_AES_GCM_TAG_SIZE])
{
    if (ctx == NULL || (nonce == NULL && nonce_len > 0U) || nonce_len == 0U ||
        (aad == NULL && aad_len > 0U) || (in == NULL && in_len > 0U) ||
        (out == NULL && in_len > 0U) || tag == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t j0[16];
    gcm_compute_j0(ctx, nonce, nonce_len, j0);

    /* Compute GHASH over AAD and Ciphertext */
    uint8_t s[16] = {0};
    if (aad_len > 0U) {
        ghash_process_blocks(ctx, aad, aad_len, s);
    }
    if (in_len > 0U) {
        ghash_process_blocks(ctx, in, in_len, s);
    }

    /* Append Length block */
    uint8_t len_block[16];
    put_be64(&len_block[0], (uint64_t)aad_len * 8ULL);
    put_be64(&len_block[8], (uint64_t)in_len * 8ULL);
    for (int j = 0; j < 16; j++) {
        s[j] ^= len_block[j];
    }
    uint8_t ghash_out[16];
    ghash_mult_bit(s, ctx->h, ghash_out);

    /* Expected Tag = GHASH_out ^ AES(K, J0) */
    uint8_t j0_enc[16];
    syn_aes_encrypt_block(&ctx->aes, j0, j0_enc);
    uint8_t expected_tag[16];
    for (int j = 0; j < 16; j++) {
        expected_tag[j] = (uint8_t)(ghash_out[j] ^ j0_enc[j]);
    }

    /* Constant-time tag comparison */
    uint8_t diff = 0U;
    for (size_t i = 0U; i < SYN_AES_GCM_TAG_SIZE; i++) {
        diff |= (uint8_t)(expected_tag[i] ^ tag[i]);
    }

    if (diff != 0U) {
        if (out != NULL && out != in && in_len > 0U) {
            memset(out, 0, in_len);
        }
        return SYN_ERROR;
    }

    /* Decrypt ciphertext with CTR mode starting at inc32(J0) */
    uint8_t cb[16];
    memcpy(cb, j0, 16);
    inc32(cb);

    uint8_t stream[16];
    size_t offset = 0U;
    while (offset < in_len) {
        syn_aes_encrypt_block(&ctx->aes, cb, stream);
        inc32(cb);
        size_t chunk = (in_len - offset >= 16U) ? 16U : (in_len - offset);
        for (size_t i = 0U; i < chunk; i++) {
            out[offset + i] = (uint8_t)(in[offset + i] ^ stream[i]);
        }
        offset += chunk;
    }

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
        b0[15U - i] = (uint8_t)((data_len >> (8U * i)) & 0xFFU);
    }

    syn_aes_encrypt_block(ctx, b0, mac);

    uint8_t blk[16];
    size_t blk_len = 0U;

    /* Process Associated Data */
    if (aad_len > 0U) {
        uint8_t hdr[10];
        size_t hdr_len = 0U;
        if (aad_len < 65280U) {
            hdr[0] = (uint8_t)((aad_len >> 8U) & 0xFFU);
            hdr[1] = (uint8_t)(aad_len & 0xFFU);
            hdr_len = 2U;
        } else {
            hdr[0] = 0xFFU;
            hdr[1] = 0xFEU;
            hdr[2] = (uint8_t)((aad_len >> 24U) & 0xFFU);
            hdr[3] = (uint8_t)((aad_len >> 16U) & 0xFFU);
            hdr[4] = (uint8_t)((aad_len >> 8U) & 0xFFU);
            hdr[5] = (uint8_t)(aad_len & 0xFFU);
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

    return SYN_OK;
}

SYN_Status syn_aes_ccm_encrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               uint8_t *out, uint8_t *tag, size_t tag_len)
{
    if (ctx == NULL || nonce == NULL || tag == NULL || nonce_len < 7U || nonce_len > 13U ||
        tag_len < 4U || tag_len > 16U || (tag_len % 2U) != 0U ||
        (in_len > 0U && (in == NULL || out == NULL)) || (aad_len > 0U && aad == NULL)) {
        return SYN_INVALID_PARAM;
    }

    size_t L = 15U - nonce_len;
    if (L < 4U) {
        uint64_t max_len = (1ULL << (8U * L)) - 1ULL;
        if ((uint64_t)in_len > max_len) {
            return SYN_INVALID_PARAM;
        }
    }

    /* 1. Compute CBC-MAC */
    uint8_t mac[16];
    ccm_compute_mac(ctx, nonce, nonce_len, L, aad, aad_len, in, in_len, tag_len, mac);

    /* 2. CTR Payload Encryption */
    uint64_t ctr = 1U;
    size_t offset = 0U;
    while (offset < in_len) {
        uint8_t ai[16];
        ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
        uint8_t si[16];
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

    return SYN_OK;
}

SYN_Status syn_aes_ccm_decrypt(const SYN_AES_Context *ctx, const uint8_t *nonce, size_t nonce_len,
                               const uint8_t *aad, size_t aad_len, const uint8_t *in, size_t in_len,
                               const uint8_t *tag, size_t tag_len, uint8_t *out)
{
    if (ctx == NULL || nonce == NULL || tag == NULL || nonce_len < 7U || nonce_len > 13U ||
        tag_len < 4U || tag_len > 16U || (tag_len % 2U) != 0U ||
        (in_len > 0U && (in == NULL || out == NULL)) || (aad_len > 0U && aad == NULL)) {
        return SYN_INVALID_PARAM;
    }

    size_t L = 15U - nonce_len;
    if (L < 4U) {
        uint64_t max_len = (1ULL << (8U * L)) - 1ULL;
        if ((uint64_t)in_len > max_len) {
            return SYN_INVALID_PARAM;
        }
    }

    /* 1. Compute S0 and unmask expected tag T */
    uint8_t a0[16];
    uint8_t s0[16];
    ccm_format_ctr(a0, nonce, nonce_len, L, 0U);
    syn_aes_encrypt_block(ctx, a0, s0);
    uint8_t unmasked_tag[16];
    for (size_t j = 0U; j < tag_len; j++) {
        unmasked_tag[j] = (uint8_t)(tag[j] ^ s0[j]);
    }

    /* 2. CTR Decrypt Ciphertext */
    uint64_t ctr = 1U;
    size_t offset = 0U;
    while (offset < in_len) {
        uint8_t ai[16];
        ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
        uint8_t si[16];
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

    if (diff != 0U) {
        if (out == in) {
            /* In-place decryption failed: invert CTR keystream to restore original ciphertext */
            ctr = 1U;
            offset = 0U;
            while (offset < in_len) {
                uint8_t ai[16];
                ccm_format_ctr(ai, nonce, nonce_len, L, ctr++);
                uint8_t si[16];
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
        return SYN_ERROR;
    }

    return SYN_OK;
}
#endif

#endif /* SYN_USE_AES */
```


