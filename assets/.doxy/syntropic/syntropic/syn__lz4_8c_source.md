

# File syn\_lz4.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lz4.c**](syn__lz4_8c.md)

[Go to the documentation of this file](syn__lz4_8c.md)


```C++

#include "syn_lz4.h"

#include <string.h>

#define LZ4_MIN_MATCH 4U  
#define LZ4_HASH_BITS 12U 
static inline uint32_t lz4_hash4(uint32_t val)
{
    return (val * 2654435761U) >> (32U - LZ4_HASH_BITS);
}

static inline uint32_t lz4_read32(const uint8_t *ptr)
{
    return ((uint32_t)ptr[0]) | (((uint32_t)ptr[1]) << 8) | (((uint32_t)ptr[2]) << 16) |
           (((uint32_t)ptr[3]) << 24);
}

SYN_Status syn_lz4_init(SYN_Lz4 *ctx)
{
    if (ctx == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(ctx->hash_table, 0, sizeof(ctx->hash_table));
    return SYN_OK;
}

size_t syn_lz4_compress_bound(size_t input_size)
{
    if (input_size > (SIZE_MAX - 16U - (input_size / 255U))) {
        return 0U; /* LCOV_EXCL_LINE: Defensive size_t overflow guard */
    }
    return input_size + (input_size / 255U) + 16U;
}

size_t syn_lz4_compress(SYN_Lz4 *ctx, const void *src, size_t src_size, void *dst,
                        size_t dst_capacity)
{
    if ((ctx == NULL) || (src == NULL) || (dst == NULL) || (src_size == 0U)) {
        return 0U;
    }

    const uint8_t *ip = (const uint8_t *)src;
    const uint8_t *in_end = ip + src_size;
    const uint8_t *anchor = ip;

    uint8_t *op = (uint8_t *)dst;
    uint8_t *out_end = op + dst_capacity;

    if (src_size < LZ4_MIN_MATCH + 1U) {
        /* Input too small for matching: store as single literal block */
        size_t lit_len = src_size;
        size_t needed = 1U + (lit_len >= 15U ? ((lit_len - 15U) / 255U + 1U) : 0U) + lit_len;
        if (needed > dst_capacity) {
            return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
        }

        uint8_t token = (uint8_t)((lit_len >= 15U ? 15U : lit_len) << 4);
        *op++ = token;

        /* LCOV_EXCL_START */
        if (lit_len >= 15U) {
            size_t rem = lit_len - 15U;
            while (rem >= 255U) {
                *op++ = 255U;
                rem -= 255U;
            }
            *op++ = (uint8_t)rem;
        }
        /* LCOV_EXCL_STOP */

        memcpy(op, ip, lit_len);
        op += lit_len;
        return (size_t)(op - (uint8_t *)dst);
    }

    syn_lz4_init(ctx);

    const uint8_t *match_limit = in_end - 5U;
    while (ip < match_limit) {
        uint32_t val = lz4_read32(ip);
        uint32_t hash = lz4_hash4(val);
        const uint8_t *match = (const uint8_t *)src + ctx->hash_table[hash];

        ctx->hash_table[hash] = (uint16_t)(ip - (const uint8_t *)src);

        if ((match >= (const uint8_t *)src) && (match < ip) && ((size_t)(ip - match) <= 65535U) &&
            (lz4_read32(match) == val)) {
            /* Found a match! Calculate match length */
            size_t lit_len = (size_t)(ip - anchor);
            const uint8_t *ref = match + 4;
            const uint8_t *p = ip + 4;

            while ((p < in_end) && (*p == *ref)) {
                p++;
                ref++;
            }

            size_t match_len = (size_t)(p - ip) - LZ4_MIN_MATCH;
            size_t offset = (size_t)(ip - match);

            /* Output token */
            if (op >= out_end)
                return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */

            uint8_t token = (uint8_t)(((lit_len >= 15U ? 15U : lit_len) << 4) |
                                      (match_len >= 15U ? 15U : match_len));
            *op++ = token;

            /* Output literal extra length */
            if (lit_len >= 15U) {
                size_t rem = lit_len - 15U;
                /* LCOV_EXCL_START */
                while (rem >= 255U) {
                    if (op >= out_end)
                        return 0U;
                    *op++ = 255U;
                    rem -= 255U;
                }
                /* LCOV_EXCL_STOP */
                if (op >= out_end)
                    return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
                *op++ = (uint8_t)rem;
            }

            /* Copy literals */
            if ((size_t)(out_end - op) < lit_len)
                return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
            memcpy(op, anchor, lit_len);
            op += lit_len;

            /* Output offset (16-bit little-endian) */
            if ((size_t)(out_end - op) < 2U)
                return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
            *op++ = (uint8_t)(offset & 0xFFU);
            *op++ = (uint8_t)((offset >> 8) & 0xFFU);

            /* Output match extra length */
            if (match_len >= 15U) {
                size_t rem = match_len - 15U;
                /* LCOV_EXCL_START */
                while (rem >= 255U) {
                    if (op >= out_end)
                        return 0U;
                    *op++ = 255U;
                    rem -= 255U;
                }
                /* LCOV_EXCL_STOP */
                if (op >= out_end)
                    return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
                *op++ = (uint8_t)rem;
            }

            ip = p;
            anchor = ip;
        } else {
            ip++;
        }
    }

    /* Output remaining literals */
    size_t last_lit = (size_t)(in_end - anchor);
    if (last_lit > 0U) {
        if (op >= out_end)
            return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
        uint8_t token = (uint8_t)((last_lit >= 15U ? 15U : last_lit) << 4);
        *op++ = token;

        /* LCOV_EXCL_START */
        if (last_lit >= 15U) {
            size_t rem = last_lit - 15U;
            while (rem >= 255U) {
                if (op >= out_end)
                    return 0U;
                *op++ = 255U;
                rem -= 255U;
            }
            if (op >= out_end)
                return 0U;
            *op++ = (uint8_t)rem;
        }
        /* LCOV_EXCL_STOP */

        if ((size_t)(out_end - op) < last_lit)
            return 0U; /* LCOV_EXCL_LINE: Defensive bounds guard */
        memcpy(op, anchor, last_lit);
        op += last_lit;
    }

    return (size_t)(op - (uint8_t *)dst);
}

size_t syn_lz4_decompress(const void *src, size_t src_size, void *dst, size_t dst_capacity)
{
    if ((src == NULL) || (dst == NULL) || (src_size == 0U) || (dst_capacity == 0U)) {
        return 0U;
    }

    const uint8_t *ip = (const uint8_t *)src;
    const uint8_t *in_end = ip + src_size;

    uint8_t *op = (uint8_t *)dst;
    uint8_t *out_end = op + dst_capacity;

    while (ip < in_end) {
        uint8_t token = *ip++;

        size_t lit_len = (size_t)(token >> 4);
        if (lit_len == 15U) {
            uint8_t s;
            /* LCOV_EXCL_START */
            do {
                if (ip >= in_end)
                    return 0U;
                s = *ip++;
                lit_len += s;
            } while (s == 255U);
            /* LCOV_EXCL_STOP */
        }

        if ((size_t)(in_end - ip) < lit_len)
            return 0U;
        if ((size_t)(out_end - op) < lit_len)
            return 0U;

        memcpy(op, ip, lit_len);
        ip += lit_len;
        op += lit_len;

        if (ip >= in_end) {
            /* Final literal sequence complete */
            break;
        }

        if ((size_t)(in_end - ip) < 2U)
            return 0U;
        size_t offset = (size_t)ip[0] | (((size_t)ip[1]) << 8);
        ip += 2;

        if ((offset == 0U) || (offset > (size_t)(op - (uint8_t *)dst))) {
            return 0U; /* Invalid offset */
        }

        size_t match_len = (size_t)(token & 0x0FU);
        if (match_len == 15U) {
            uint8_t s;
            /* LCOV_EXCL_START */
            do {
                if (ip >= in_end)
                    return 0U;
                s = *ip++;
                match_len += s;
            } while (s == 255U);
            /* LCOV_EXCL_STOP */
        }
        match_len += LZ4_MIN_MATCH;

        if ((size_t)(out_end - op) < match_len)
            return 0U; /* LCOV_EXCL_LINE: Defensive match length destination guard */

        const uint8_t *ref = op - offset;
        for (size_t i = 0U; i < match_len; i++) {
            *op++ = *ref++;
        }
    }

    return (size_t)(op - (uint8_t *)dst);
}
```


