

# File syn\_aes128.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_aes128.c**](syn__aes128_8c.md)

[Go to the documentation of this file](syn__aes128_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AES128) || SYN_USE_AES128

#include "syn_aes128.h"
#include "syn_assert.h"

#include <string.h>

/* ── AES-128 S-Box ───────────────────────────────────────────────────────── */
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

static const uint8_t rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

static inline uint8_t gmult2(uint8_t a)
{
    return (uint8_t)((a << 1) ^ (((a >> 7) & 1) * 0x1b));
}

static inline uint8_t gmult(uint8_t a, uint8_t b)
{
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1)
            p ^= a;
        uint8_t hi_bit_set = a & 0x80;
        a <<= 1;
        if (hi_bit_set)
            a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

SYN_Status syn_aes128_init(SYN_AES128_Context *ctx, const uint8_t key[SYN_AES128_KEY_SIZE])
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(key != NULL);

    if (ctx == NULL || key == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    memcpy(ctx->round_keys, key, 16);
    uint8_t temp[4];
    uint8_t i = 0;
    uint8_t k = 16;

    while (k < 176) {
        temp[0] = ctx->round_keys[k - 4];
        temp[1] = ctx->round_keys[k - 3];
        temp[2] = ctx->round_keys[k - 2];
        temp[3] = ctx->round_keys[k - 1];

        if (k % 16 == 0) {
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
        }

        ctx->round_keys[k] = ctx->round_keys[k - 16] ^ temp[0];
        ctx->round_keys[k + 1] = ctx->round_keys[k - 15] ^ temp[1];
        ctx->round_keys[k + 2] = ctx->round_keys[k - 14] ^ temp[2];
        ctx->round_keys[k + 3] = ctx->round_keys[k - 13] ^ temp[3];
        k += 4;
    }

    return SYN_OK;
}

void syn_aes128_encrypt_block(const SYN_AES128_Context *ctx, const uint8_t in[16], uint8_t out[16])
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(in != NULL);
    SYN_ASSERT(out != NULL);

    if (ctx == NULL || in == NULL || out == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in release mode */
    }

    uint8_t state[4][4];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = in[r + 4 * c] ^ ctx->round_keys[r + 4 * c];
        }
    }

    for (int round = 1; round <= 10; round++) {
        /* SubBytes */
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state[r][c] = sbox[state[r][c]];

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
        if (round < 10) {
            for (int c = 0; c < 4; c++) {
                uint8_t a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
                state[0][c] = gmult2(a0) ^ gmult2(a1) ^ a1 ^ a2 ^ a3;
                state[1][c] = a0 ^ gmult2(a1) ^ gmult2(a2) ^ a2 ^ a3;
                state[2][c] = a0 ^ a1 ^ gmult2(a2) ^ gmult2(a3) ^ a3;
                state[3][c] = gmult2(a0) ^ a0 ^ a1 ^ a2 ^ gmult2(a3);
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

void syn_aes128_decrypt_block(const SYN_AES128_Context *ctx, const uint8_t in[16], uint8_t out[16])
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(in != NULL);
    SYN_ASSERT(out != NULL);

    if (ctx == NULL || in == NULL || out == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in release mode */
    }

    uint8_t state[4][4];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state[r][c] = in[r + 4 * c] ^ ctx->round_keys[160 + r + 4 * c];
        }
    }

    for (int round = 9; round >= 0; round--) {
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
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state[r][c] = rsbox[state[r][c]];

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
                state[0][c] = gmult(a0, 0x0e) ^ gmult(a1, 0x0b) ^ gmult(a2, 0x0d) ^ gmult(a3, 0x09);
                state[1][c] = gmult(a0, 0x09) ^ gmult(a1, 0x0e) ^ gmult(a2, 0x0b) ^ gmult(a3, 0x0d);
                state[2][c] = gmult(a0, 0x0d) ^ gmult(a1, 0x09) ^ gmult(a2, 0x0e) ^ gmult(a3, 0x0b);
                state[3][c] = gmult(a0, 0x0b) ^ gmult(a1, 0x0d) ^ gmult(a2, 0x09) ^ gmult(a3, 0x0e);
            }
        }
    }

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            out[r + 4 * c] = state[r][c];
        }
    }
}

SYN_Status syn_aes128_cbc_encrypt(const SYN_AES128_Context *ctx, const uint8_t iv[16],
                                  const uint8_t *in, size_t in_len, uint8_t *out,
                                  size_t out_capacity, size_t *out_len)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(iv != NULL);
    SYN_ASSERT(in != NULL || in_len == 0);
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out_len != NULL);

    if (ctx == NULL || iv == NULL || (in == NULL && in_len > 0) || out == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    size_t pad_len = 16 - (in_len % 16);
    size_t total_len = in_len + pad_len;

    if (out_capacity < total_len)
        return SYN_INVALID_PARAM;

    uint8_t current_iv[16];
    memcpy(current_iv, iv, 16);

    uint8_t block[16];
    size_t offset = 0;

    while (offset < total_len) {
        if (offset + 16 <= in_len) {
            memcpy(block, in + offset, 16);
        } else {
            size_t remaining = in_len - offset;
            if (remaining > 0) {
                memcpy(block, in + offset, remaining);
            }
            memset(block + remaining, (uint8_t)pad_len, 16 - remaining);
        }

        for (int i = 0; i < 16; i++) {
            block[i] ^= current_iv[i];
        }

        syn_aes128_encrypt_block(ctx, block, out + offset);
        memcpy(current_iv, out + offset, 16);
        offset += 16;
    }

    *out_len = total_len;
    return SYN_OK;
}

SYN_Status syn_aes128_cbc_decrypt(const SYN_AES128_Context *ctx, const uint8_t iv[16],
                                  const uint8_t *in, size_t in_len, uint8_t *out,
                                  size_t out_capacity, size_t *out_len)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(iv != NULL);
    SYN_ASSERT(in != NULL);
    SYN_ASSERT(out != NULL);
    SYN_ASSERT(out_len != NULL);

    if (ctx == NULL || iv == NULL || in == NULL || out == NULL || out_len == NULL || in_len == 0 ||
        in_len % 16 != 0 || out_capacity < in_len) {
        return SYN_INVALID_PARAM;
    }

    uint8_t current_iv[16];
    memcpy(current_iv, iv, 16);

    uint8_t block[16];
    size_t offset = 0;

    while (offset < in_len) {
        syn_aes128_decrypt_block(ctx, in + offset, block);
        for (int i = 0; i < 16; i++) {
            block[i] ^= current_iv[i];
        }
        memcpy(current_iv, in + offset, 16);
        memcpy(out + offset, block, 16);
        offset += 16;
    }

    /* PKCS#7 Unpadding */
    uint8_t pad_val = out[in_len - 1];
    if (pad_val == 0 || pad_val > 16 || pad_val > in_len) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = in_len - pad_val; i < in_len; i++) {
        if (out[i] != pad_val) {
            return SYN_INVALID_PARAM;
        }
    }

    *out_len = in_len - pad_val;
    return SYN_OK;
}

#endif /* SYN_USE_AES128 */
```


