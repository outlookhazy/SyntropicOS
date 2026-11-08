/**
 * @file syn_base64.h
 * @brief RFC 4648 Base64 and Base64URL encoding and decoding.
 *
 * Implements pure C99 zero-allocation streaming Base64 (standard and URL-safe).
 * Supports both padded and unpadded Base64URL formats, whitespace-tolerant decoding,
 * and precise buffer length calculation.
 *
 * @par Standard Base64 Encoding
 * @code
 *   const uint8_t data[] = "Hello World";
 *   char b64[32];
 *   size_t out_len = 0;
 *   syn_base64_encode(data, sizeof(data) - 1, b64, sizeof(b64), &out_len);
 *   // b64 is "SGVsbG8gV29ybGQ="
 * @endcode
 *
 * @par Base64 Decoding
 * @code
 *   uint8_t bin[32];
 *   size_t bin_len = 0;
 *   if (syn_base64_decode(b64, out_len, bin, sizeof(bin), &bin_len)) {
 *       // Decoded successfully
 *   }
 * @endcode
 *
 * @ingroup syn_util
 */

#ifndef SYN_BASE64_H
#define SYN_BASE64_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BASE64) || SYN_USE_BASE64

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum raw input length representable in Base64 without size_t overflow. */
#define SYN_BASE64_MAX_RAW_LEN (((SIZE_MAX - 4U) / 4U) * 3U)

/**
 * @brief Calculate the buffer size needed for standard Base64 encoding.
 *
 * Includes padding (`=`) and space for the null terminator.
 *
 * @param raw_len Number of raw binary bytes.
 * @return Total character buffer capacity needed (including null terminator), or 0 on overflow.
 */
static inline size_t syn_base64_calc_encoded_size(size_t raw_len)
{
    if (raw_len > SYN_BASE64_MAX_RAW_LEN) {
        return 0U;
    }
    return ((raw_len / 3U) * 4U) + (((raw_len % 3U) != 0U) ? 4U : 0U) + 1U;
}

/**
 * @brief Calculate the maximum binary buffer size needed to decode a Base64 string.
 *
 * @param b64_len Number of characters in Base64 string.
 * @return Maximum number of binary bytes after decoding, or 0 on overflow.
 */
static inline size_t syn_base64_calc_max_decoded_size(size_t b64_len)
{
    if (b64_len > (SIZE_MAX - 3U)) {
        return 0U;
    }
    return ((b64_len / 4U) * 3U) + (((b64_len % 4U) != 0U) ? 3U : 0U);
}

/**
 * @brief Encode binary data to standard RFC 4648 Base64 string.
 *
 * Uses alphabet `[A-Za-z0-9+/]` and adds `=` padding when needed.
 * Always null-terminates the output if `dst_size > 0`.
 *
 * @param src      Input binary data.
 * @param src_len  Length of input binary data in bytes.
 * @param dst      [out] Output string buffer.
 * @param dst_size Capacity of output buffer in bytes (must be >=
 * `syn_base64_calc_encoded_size(src_len)`).
 * @param out_len  [out] Optional pointer to receive character length written (excluding null).
 * @return true on success, false if parameters are invalid or buffer is too small.
 */
bool syn_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                       size_t *out_len);

/**
 * @brief Decode an RFC 4648 standard Base64 string into binary data.
 *
 * Automatically skips whitespace characters (spaces, tabs, CR, LF).
 * Rejects invalid non-base64 characters and malformed padding.
 *
 * @param src      Input Base64 string (null-terminated or bounded by `src_len`).
 * @param src_len  Length of input string (or `0` to determine via `strlen`).
 * @param dst      [out] Output binary buffer.
 * @param dst_size Capacity of output buffer in bytes.
 * @param out_len  [out] Optional pointer to receive actual binary bytes written.
 * @return true on success, false if input contains invalid characters or destination is too small.
 */
bool syn_base64_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                       size_t *out_len);

/**
 * @brief Encode binary data to RFC 4648 Base64URL string.
 *
 * Uses URL-safe alphabet `[A-Za-z0-9-_]`.
 *
 * @param src          Input binary data.
 * @param src_len      Length of input binary data in bytes.
 * @param dst          [out] Output string buffer.
 * @param dst_size     Capacity of output buffer in bytes.
 * @param with_padding If true, appends `=` padding; if false, omits padding (standard JWT format).
 * @param out_len      [out] Optional pointer to receive character length written (excluding null).
 * @return true on success, false if buffer is too small or parameters invalid.
 */
bool syn_base64url_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                          bool with_padding, size_t *out_len);

/**
 * @brief Decode an RFC 4648 Base64URL string into binary data.
 *
 * Supports both padded (`=`) and unpadded URL-safe strings.
 *
 * @param src      Input Base64URL string.
 * @param src_len  Length of input string (or `0` to determine via `strlen`).
 * @param dst      [out] Output binary buffer.
 * @param dst_size Capacity of output buffer in bytes.
 * @param out_len  [out] Optional pointer to receive actual binary bytes written.
 * @return true on success, false on invalid input or insufficient buffer capacity.
 */
bool syn_base64url_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                          size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_BASE64 */

#endif /* SYN_BASE64_H */
