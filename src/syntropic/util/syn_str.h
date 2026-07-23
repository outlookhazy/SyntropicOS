/**
 * @file syn_str.h
 * @brief Zero-allocation string parsing, trimming, and tokenization utilities.
 * @ingroup syn_core
 */

#ifndef SYN_STR_H
#define SYN_STR_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Split a string in-place by a delimiter character into a token array.
 *
 * Replaces occurrences of delimiter with '\0' and populates the tokens array.
 * Zero-allocation. Modifies the input string.
 *
 * @param str         Null-terminated input string (modified in-place).
 * @param delimiter   Character to split on (e.g. ',' or ' ').
 * @param tokens      Array of char pointers to store token starts.
 * @param max_tokens  Maximum capacity of tokens array.
 * @return Number of tokens stored in tokens array.
 */
size_t syn_str_split(char *str, char delimiter, char *tokens[], size_t max_tokens);

/**
 * @brief Trim leading and trailing whitespace characters in-place.
 *
 * @param str Null-terminated input string (modified in-place).
 * @return Pointer to the first non-whitespace character in str.
 */
char *syn_str_trim(char *str);

/**
 * @brief Parse a signed 32-bit integer from a string safely.
 *
 * @param str     Input string to parse.
 * @param out_val Pointer to store parsed int32_t result.
 * @return true if successfully parsed, false on error or empty string.
 */
bool syn_str_to_i32(const char *str, int32_t *out_val);

/**
 * @brief Parse an unsigned 32-bit integer from a string safely.
 *
 * @param str     Input string to parse.
 * @param out_val Pointer to store parsed uint32_t result.
 * @return true if successfully parsed, false on error or empty string.
 */
bool syn_str_to_u32(const char *str, uint32_t *out_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_STR_H */
