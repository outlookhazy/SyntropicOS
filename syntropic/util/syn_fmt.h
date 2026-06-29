/**
 * @file syn_fmt.h
 * @brief Lightweight formatting — no libc printf dependency.
 *
 * Integer-to-string, hex, fixed-point, and hex dump. All functions
 * write to a caller-provided buffer and return the number of chars
 * written (excluding null terminator).
 *
 * @par Usage
 * @code
 *   char buf[16];
 *   syn_fmt_int(buf, sizeof(buf), -1234);     // "-1234"
 *   syn_fmt_hex(buf, sizeof(buf), 0xDEAD, 4); // "DEAD"
 *   syn_fmt_q16(buf, sizeof(buf), val, 3);    // "3.141"
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_FMT_H
#define SYN_FMT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format a signed integer to decimal string.
 * @param buf   Output buffer.
 * @param size  Buffer capacity.
 * @param val   Value to format.
 * @return Number of characters written (excluding null).
 */
size_t syn_fmt_int(char *buf, size_t size, int32_t val);

/**
 * @brief Format an unsigned integer to decimal string.
 * @param buf   Output buffer.
 * @param size  Buffer capacity.
 * @param val   Value to format.
 * @return Number of characters written (excluding null).
 */
size_t syn_fmt_uint(char *buf, size_t size, uint32_t val);

/**
 * @brief Format a value as hex.
 *
 * @param buf         Output buffer.
 * @param size        Buffer capacity in bytes.
 * @param val         Value to format.
 * @param min_digits  Minimum hex digits (zero-padded).
 * @return Number of characters written.
 */
size_t syn_fmt_hex(char *buf, size_t size, uint32_t val, uint8_t min_digits);

/**
 * @brief Format a Q16.16 fixed-point value.
 *
 * @param buf         Output buffer.
 * @param size        Buffer capacity in bytes.
 * @param q16_val     Q16.16 value to format.
 * @param frac_digits Number of fractional decimal digits (1–6).
 * @return Number of characters written.
 */
size_t syn_fmt_q16(char *buf, size_t size, int32_t q16_val,
                    uint8_t frac_digits);

/**
 * @brief Format a hex dump of a byte array.
 *
 * Output: "DE AD BE EF" (space-separated hex bytes).
 *
 * @param buf   Output buffer.
 * @param size  Buffer capacity.
 * @param data  Byte array.
 * @param len   Number of bytes.
 * @return Number of characters written.
 */
size_t syn_fmt_hexdump(char *buf, size_t size,
                        const uint8_t *data, size_t len);

/**
 * @brief Format a value with a fixed number of decimal places.
 *
 * E.g., syn_fmt_fixed(buf, 16, 12345, 3) → "12.345"
 *
 * @param buf     Output buffer.
 * @param size    Buffer capacity in bytes.
 * @param val     Integer value.
 * @param places  Number of decimal places from the right.
 * @return Number of characters written.
 */
size_t syn_fmt_fixed(char *buf, size_t size, int32_t val, uint8_t places);

/**
 * @brief Build a string from parts (like snprintf but simpler).
 *
 * Concatenate up to @p n string fragments into buf.
 *
 * @param buf    Output buffer.
 * @param size   Buffer capacity.
 * @param parts  Array of string pointers.
 * @param n      Number of strings.
 * @return Total characters written.
 */
size_t syn_fmt_concat(char *buf, size_t size,
                       const char *const *parts, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FMT_H */
