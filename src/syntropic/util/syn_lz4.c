/**
 * @file syn_lz4.c
 * @brief Implementation of Zero-Heap LZ4 Raw Block Compression & Decompression.
 * @ingroup syn_util
 */

#include "syn_lz4.h"

#include <string.h>

#define LZ4_MIN_MATCH 4U    /**< Minimum byte match length for LZ4 compression */
#define LZ4_LASTLITERALS 5U /**< Mandatory trailing literal bytes at end of block */
#define LZ4_MFLIMIT 12U     /**< Minimum distance from block end to start a match */
#define LZ4_MIN_LENGTH 13U  /**< Minimum block size eligible for match search */
#define LZ4_HASH_BITS 12U   /**< Log2 size of 4096-entry hash table */

/**
 * @brief Hash 4 bytes into 12-bit hash index using Knuth multiplicative hash.
 * @param val 32-bit integer word to hash.
 * @return 12-bit hash index (0..4095).
 */
static inline uint32_t lz4_hash4(uint32_t val)
{
    return (val * 2654435761U) >> (32U - LZ4_HASH_BITS);
}

/**
 * @brief Read 32-bit little-endian word from unaligned byte pointer.
 * @param ptr Pointer to byte stream.
 * @return 32-bit unsigned integer value.
 */
static inline uint32_t lz4_read32(const uint8_t *ptr)
{
    return ((uint32_t)ptr[0]) | (((uint32_t)ptr[1]) << 8) | (((uint32_t)ptr[2]) << 16) |
           (((uint32_t)ptr[3]) << 24);
}

/**
 * @brief Encode extended LZ4 length bytes (>= 15) into output buffer.
 * @param op      Current output pointer.
 * @param out_end End of output buffer.
 * @param rem     Remaining length after subtracting 15.
 * @return Updated output pointer, or NULL on buffer overflow.
 */
static uint8_t *lz4_write_len(uint8_t *op, const uint8_t *out_end, size_t rem)
{
    while (rem >= 255U) {
        if (op >= out_end) {
            return NULL;
        }
        *op++ = 255U;
        rem -= 255U;
    }
    if (op >= out_end) {
        return NULL;
    }
    *op++ = (uint8_t)rem;
    return op;
}

/**
 * @brief Emit one LZ4 sequence (token, literal run, 16-bit offset, extra match length).
 * @param op        Current output pointer.
 * @param out_end   End of output buffer.
 * @param anchor    Start of literal run.
 * @param lit_len   Literal run byte count.
 * @param offset    Backward match offset (1..65535).
 * @param match_len Match length minus LZ4_MIN_MATCH.
 * @return Updated output pointer, or NULL on buffer overflow.
 */
static uint8_t *lz4_emit_sequence(uint8_t *op, const uint8_t *out_end, const uint8_t *anchor,
                                  size_t lit_len, size_t offset, size_t match_len)
{
    if (op >= out_end) {
        return NULL;
    }

    uint8_t lit_tok = (uint8_t)((lit_len >= 15U) ? 15U : lit_len);
    uint8_t mat_tok = (uint8_t)((match_len >= 15U) ? 15U : match_len);
    *op++ = (uint8_t)((lit_tok << 4) | mat_tok);

    if (lit_len >= 15U) {
        op = lz4_write_len(op, out_end, lit_len - 15U);
        if (op == NULL) {
            return NULL;
        }
    }

    if ((size_t)(out_end - op) < lit_len) {
        return NULL;
    }
    if (lit_len > 0U) {
        memcpy(op, anchor, lit_len);
        op += lit_len;
    }

    if ((size_t)(out_end - op) < 2U) {
        return NULL;
    }
    *op++ = (uint8_t)(offset & 0xFFU);
    *op++ = (uint8_t)((offset >> 8) & 0xFFU);

    if (match_len >= 15U) {
        op = lz4_write_len(op, out_end, match_len - 15U);
    }
    return op;
}

/**
 * @brief Decode extended LZ4 length bytes when nibble equals 15.
 * @param ip      Current input pointer.
 * @param in_end  End of compressed input buffer.
 * @param max_len Maximum valid accumulated length for remaining buffer capacity.
 * @param len_io  [in,out] Accumulated length.
 * @return Updated input pointer, or NULL on truncated input or length overflow.
 */
static const uint8_t *lz4_read_len(const uint8_t *ip, const uint8_t *in_end, size_t max_len,
                                   size_t *len_io)
{
    uint8_t s = 0U;
    do {
        if ((ip >= in_end) || (*len_io > max_len) || ((size_t)*ip > (max_len - *len_io))) {
            return NULL;
        }
        s = *ip++;
        *len_io += s;
    } while (s == 255U);
    return ip;
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
        return 0U;
    }
    return input_size + (input_size / 255U) + 16U;
}

size_t syn_lz4_compress(SYN_Lz4 *ctx, const void *src, size_t src_size, void *dst,
                        size_t dst_capacity)
{
    if ((ctx == NULL) || (src == NULL) || (dst == NULL) || (src_size == 0U)) {
        return 0U;
    }

    const uint8_t *src_base = (const uint8_t *)src;
    const uint8_t *ip = src_base;
    const uint8_t *in_end = src_base + src_size;
    const uint8_t *anchor = ip;

    uint8_t *op = (uint8_t *)dst;
    const uint8_t *out_end = op + dst_capacity;

    if (src_size >= LZ4_MIN_LENGTH) {
        (void)syn_lz4_init(ctx);
        const uint8_t *mflimit = in_end - LZ4_MFLIMIT;
        const uint8_t *match_bound = in_end - LZ4_LASTLITERALS;

        while (ip <= mflimit) {
            uint32_t val = lz4_read32(ip);
            uint32_t hash = lz4_hash4(val);
            const uint8_t *match = src_base + ctx->hash_table[hash];
            ctx->hash_table[hash] = (uint16_t)(ip - src_base);

            if ((match < ip) && ((size_t)(ip - match) <= 65535U) && (lz4_read32(match) == val)) {
                const uint8_t *ref = match + LZ4_MIN_MATCH;
                const uint8_t *p = ip + LZ4_MIN_MATCH;
                while ((p < match_bound) && (*p == *ref)) {
                    p++;
                    ref++;
                }

                op = lz4_emit_sequence(op, out_end, anchor, (size_t)(ip - anchor),
                                       (size_t)(ip - match), (size_t)(p - ip) - LZ4_MIN_MATCH);
                if (op == NULL) {
                    return 0U;
                }
                ip = p;
                anchor = ip;
            } else {
                ip++;
            }
        }
    }

    size_t last_lit = (size_t)(in_end - anchor);
    if (op >= out_end) {
        return 0U;
    }
    *op++ = (uint8_t)(((last_lit >= 15U) ? 15U : last_lit) << 4);
    if (last_lit >= 15U) {
        op = lz4_write_len(op, out_end, last_lit - 15U);
        if (op == NULL) {
            return 0U;
        }
    }
    if ((size_t)(out_end - op) < last_lit) {
        return 0U;
    }
    memcpy(op, anchor, last_lit);
    op += last_lit;

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
    const uint8_t *out_end = op + dst_capacity;

    while (ip < in_end) {
        uint8_t token = *ip++;

        size_t lit_len = (size_t)(token >> 4);
        if (lit_len == 15U) {
            ip = lz4_read_len(ip, in_end, (size_t)(out_end - op), &lit_len);
            if (ip == NULL) {
                return 0U;
            }
        }

        if (((size_t)(in_end - ip) < lit_len) || ((size_t)(out_end - op) < lit_len)) {
            return 0U;
        }

        if (lit_len > 0U) {
            memcpy(op, ip, lit_len);
            ip += lit_len;
            op += lit_len;
        }

        if (ip >= in_end) {
            break;
        }

        if ((size_t)(in_end - ip) < 2U) {
            return 0U;
        }
        size_t offset = (size_t)ip[0] | (((size_t)ip[1]) << 8);
        ip += 2;

        if ((offset == 0U) || (offset > (size_t)(op - (uint8_t *)dst))) {
            return 0U;
        }

        size_t match_len = (size_t)(token & 0x0FU);
        if (match_len == 15U) {
            size_t rem_out = (size_t)(out_end - op);
            size_t max_mat = (rem_out >= LZ4_MIN_MATCH) ? (rem_out - LZ4_MIN_MATCH) : 0U;
            ip = lz4_read_len(ip, in_end, max_mat, &match_len);
            if (ip == NULL) {
                return 0U;
            }
        }
        match_len += LZ4_MIN_MATCH;

        if ((size_t)(out_end - op) < match_len) {
            return 0U;
        }

        const uint8_t *ref = op - offset;
        for (size_t i = 0U; i < match_len; i++) {
            *op++ = *ref++;
        }
    }

    return (size_t)(op - (uint8_t *)dst);
}
