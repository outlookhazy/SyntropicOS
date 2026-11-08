/**
 * @file syn_lz4.h
 * @brief Zero-Heap LZ4 Raw Block Compression & Decompression.
 * @ingroup syn_util
 *
 * Implements standard LZ4 raw block format compression and decompression
 * without dynamic memory allocation or complex framing overhead.
 */

#ifndef SYN_LZ4_H
#define SYN_LZ4_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_LZ4_HASH_SIZE
#define SYN_LZ4_HASH_SIZE 4096U /**< LZ4 hash table size (4096 entries = 8 KB) */
#endif

/**
 * @brief LZ4 Compression Workspace Context.
 *
 * Must be allocated by caller and initialized with syn_lz4_init() before
 * calling syn_lz4_compress().
 *
 * @warning Default SYN_Lz4 workspace is 8 KB (4096 x uint16_t) and must be
 *          allocated in static/BSS storage rather than on a 1024 B task stack.
 */
typedef struct {
    uint16_t hash_table[SYN_LZ4_HASH_SIZE]; /**< Hash table indexed by 4-byte sequence hash */
} SYN_Lz4;

/**
 * @brief Initialize an LZ4 compression context.
 *
 * @param ctx Pointer to LZ4 context instance.
 * @return SYN_OK on success, or SYN_INVALID_PARAM.
 */
SYN_Status syn_lz4_init(SYN_Lz4 *ctx);

/**
 * @brief Calculate worst-case compressed output bound for a given input size.
 *
 * @param input_size Uncompressed payload size in bytes.
 * @return Maximum compressed buffer size required.
 */
size_t syn_lz4_compress_bound(size_t input_size);

/**
 * @brief Compress data buffer into LZ4 raw block format.
 *
 * @param ctx          Pointer to initialized LZ4 context.
 * @param src          Source data buffer to compress.
 * @param src_size     Source data size in bytes.
 * @param dst          Destination buffer for compressed output.
 * @param dst_capacity Capacity of destination buffer in bytes.
 * @return Number of compressed bytes written to dst, or 0 on error/overflow.
 */
size_t syn_lz4_compress(SYN_Lz4 *ctx, const void *src, size_t src_size, void *dst,
                        size_t dst_capacity);

/**
 * @brief Decompress LZ4 raw block buffer.
 *
 * Decompression is fully stateless and requires no context workspace.
 *
 * @param src          Compressed source buffer.
 * @param src_size     Compressed source size in bytes.
 * @param dst          Destination buffer for decompressed output.
 * @param dst_capacity Capacity of destination buffer in bytes.
 * @return Number of decompressed bytes written to dst, or 0 on corruption/overflow.
 */
size_t syn_lz4_decompress(const void *src, size_t src_size, void *dst, size_t dst_capacity);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LZ4_H */
