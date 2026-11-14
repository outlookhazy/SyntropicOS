

# File syn\_base64.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_base64.c**](syn__base64_8c.md)

[Go to the documentation of this file](syn__base64_8c.md)


```C++
#include "../common/syn_defs.h"

#if !defined(SYN_USE_BASE64) || SYN_USE_BASE64


#include "syn_base64.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char B64_STD_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char B64_URL_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

#define B64_VAL_INVALID 0xFFU
#define B64_VAL_PAD 0xFEU
#define B64_VAL_WS 0xFDU

static uint8_t decode_special_char(char c, bool url_mode)
{
    char c62 = url_mode ? '-' : '+';
    char c63 = url_mode ? '_' : '/';
    if (c == c62) {
        return 62U;
    }
    if (c == c63) {
        return 63U;
    }
    if (c == '=') {
        return B64_VAL_PAD;
    }
    if ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n')) {
        return B64_VAL_WS;
    }
    return B64_VAL_INVALID;
}

static uint8_t decode_char(char c, bool url_mode)
{
    if ((c >= 'A') && (c <= 'Z')) {
        return (uint8_t)(c - 'A');
    }
    if ((c >= 'a') && (c <= 'z')) {
        return (uint8_t)(c - 'a' + 26);
    }
    if ((c >= '0') && (c <= '9')) {
        return (uint8_t)(c - '0' + 52);
    }
    return decode_special_char(c, url_mode);
}

static size_t base64url_calc_req_len(size_t src_len, bool with_padding)
{
    if (src_len > SYN_BASE64_MAX_RAW_LEN) {
        return 0U;
    }
    if (with_padding) {
        return syn_base64_calc_encoded_size(src_len);
    }
    size_t rem = src_len % 3U;
    size_t tail = (rem == 0U) ? 0U : (rem + 1U);
    return ((src_len / 3U) * 4U) + tail + 1U;
}

bool syn_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                       size_t *out_len)
{
    if ((dst == NULL) || ((src == NULL) && (src_len > 0U))) {
        return false;
    }

    size_t req_len = syn_base64_calc_encoded_size(src_len);
    if ((req_len == 0U) || (dst_size < req_len)) {
        return false;
    }

    size_t in_pos = 0U;
    size_t out_pos = 0U;

    while ((src_len - in_pos) >= 3U) {
        uint32_t triple = ((uint32_t)src[in_pos] << 16U) | ((uint32_t)src[in_pos + 1U] << 8U) |
                          (uint32_t)src[in_pos + 2U];
        dst[out_pos++] = B64_STD_TABLE[(triple >> 18U) & 0x3FU];
        dst[out_pos++] = B64_STD_TABLE[(triple >> 12U) & 0x3FU];
        dst[out_pos++] = B64_STD_TABLE[(triple >> 6U) & 0x3FU];
        dst[out_pos++] = B64_STD_TABLE[triple & 0x3FU];
        in_pos += 3U;
    }

    if (in_pos < src_len) {
        size_t rem = src_len - in_pos;
        if (rem == 1U) {
            uint32_t triple = (uint32_t)src[in_pos] << 16U;
            dst[out_pos++] = B64_STD_TABLE[(triple >> 18U) & 0x3FU];
            dst[out_pos++] = B64_STD_TABLE[(triple >> 12U) & 0x3FU];
            dst[out_pos++] = '=';
            dst[out_pos++] = '=';
        } else { /* rem == 2 */
            uint32_t triple = ((uint32_t)src[in_pos] << 16U) | ((uint32_t)src[in_pos + 1U] << 8U);
            dst[out_pos++] = B64_STD_TABLE[(triple >> 18U) & 0x3FU];
            dst[out_pos++] = B64_STD_TABLE[(triple >> 12U) & 0x3FU];
            dst[out_pos++] = B64_STD_TABLE[(triple >> 6U) & 0x3FU];
            dst[out_pos++] = '=';
        }
    }

    dst[out_pos] = '\0';
    if (out_len != NULL) {
        *out_len = out_pos;
    }
    return true;
}

bool syn_base64url_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_size,
                          bool with_padding, size_t *out_len)
{
    if ((dst == NULL) || ((src == NULL) && (src_len > 0U))) {
        return false;
    }

    size_t in_pos = 0U;
    size_t out_pos = 0U;

    size_t req_len = base64url_calc_req_len(src_len, with_padding);
    if ((req_len == 0U) || (dst_size < req_len)) {
        return false;
    }

    while ((src_len - in_pos) >= 3U) {
        uint32_t triple = ((uint32_t)src[in_pos] << 16U) | ((uint32_t)src[in_pos + 1U] << 8U) |
                          (uint32_t)src[in_pos + 2U];
        dst[out_pos++] = B64_URL_TABLE[(triple >> 18U) & 0x3FU];
        dst[out_pos++] = B64_URL_TABLE[(triple >> 12U) & 0x3FU];
        dst[out_pos++] = B64_URL_TABLE[(triple >> 6U) & 0x3FU];
        dst[out_pos++] = B64_URL_TABLE[triple & 0x3FU];
        in_pos += 3U;
    }

    if (in_pos < src_len) {
        size_t rem = src_len - in_pos;
        if (rem == 1U) {
            uint32_t triple = (uint32_t)src[in_pos] << 16U;
            dst[out_pos++] = B64_URL_TABLE[(triple >> 18U) & 0x3FU];
            dst[out_pos++] = B64_URL_TABLE[(triple >> 12U) & 0x3FU];
            if (with_padding) {
                dst[out_pos++] = '=';
                dst[out_pos++] = '=';
            }
        } else { /* rem == 2 */
            uint32_t triple = ((uint32_t)src[in_pos] << 16U) | ((uint32_t)src[in_pos + 1U] << 8U);
            dst[out_pos++] = B64_URL_TABLE[(triple >> 18U) & 0x3FU];
            dst[out_pos++] = B64_URL_TABLE[(triple >> 12U) & 0x3FU];
            dst[out_pos++] = B64_URL_TABLE[(triple >> 6U) & 0x3FU];
            if (with_padding) {
                dst[out_pos++] = '=';
            }
        }
    }

    dst[out_pos] = '\0';
    if (out_len != NULL) {
        *out_len = out_pos;
    }
    return true;
}

static bool base64_handle_pad(uint8_t q_pos, uint8_t *pad_count)
{
    if (*pad_count == 0U) {
        if (q_pos < 2U) {
            return false;
        }
        *pad_count = 1U;
        return true;
    }
    if ((*pad_count == 1U) && (q_pos == 2U)) {
        *pad_count = 2U;
        return true;
    }
    return false;
}

static bool base64_validate_tail(uint32_t block, uint8_t q_pos, uint8_t pad_count)
{
    if (q_pos == 1U) {
        return false;
    }
    if (q_pos == 2U) {
        if (pad_count == 1U) {
            return false;
        }
        return (block & 0x0FU) == 0U;
    }
    if (q_pos == 3U) {
        return (block & 0x03U) == 0U;
    }
    return true;
}

static bool base64_push_symbol(uint8_t val, uint32_t *block, uint8_t *bits, uint8_t *q_pos,
                               uint8_t *dst, size_t dst_size, size_t *out_pos)
{
    *block = (*block << 6U) | (uint32_t)val;
    *bits = (uint8_t)(*bits + 6U);
    (*q_pos)++;

    if (*bits >= 8U) {
        *bits = (uint8_t)(*bits - 8U);
        if (*out_pos >= dst_size) {
            return false;
        }
        dst[(*out_pos)++] = (uint8_t)((*block >> *bits) & 0xFFU);
    }
    if (*q_pos == 4U) {
        *q_pos = 0U;
        *block = 0U;
    }
    return true;
}

static bool base64_decode_internal(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                                   size_t *out_len, bool url_mode)
{
    if ((dst == NULL) || (src == NULL)) {
        return false;
    }

    if (src_len == 0U) {
        src_len = strlen(src);
    }

    uint32_t block = 0U;
    uint8_t bits = 0U;
    uint8_t q_pos = 0U;
    uint8_t pad_count = 0U;
    size_t out_pos = 0U;

    for (size_t i = 0U; i < src_len; i++) {
        uint8_t val = decode_char(src[i], url_mode);
        if (val == B64_VAL_WS) {
            continue;
        }
        if (val == B64_VAL_INVALID) {
            return false;
        }
        if (val == B64_VAL_PAD) {
            if (!base64_handle_pad(q_pos, &pad_count)) {
                return false;
            }
            continue;
        }
        if (pad_count > 0U) {
            return false;
        }
        if (!base64_push_symbol(val, &block, &bits, &q_pos, dst, dst_size, &out_pos)) {
            return false;
        }
    }

    if (!base64_validate_tail(block, q_pos, pad_count)) {
        return false;
    }

    if (out_len != NULL) {
        *out_len = out_pos;
    }
    return true;
}

bool syn_base64_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                       size_t *out_len)
{
    return base64_decode_internal(src, src_len, dst, dst_size, out_len, false);
}

bool syn_base64url_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_size,
                          size_t *out_len)
{
    return base64_decode_internal(src, src_len, dst, dst_size, out_len, true);
}

#endif /* SYN_USE_BASE64 */
```


