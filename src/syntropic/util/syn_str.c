/**
 * @file syn_str.c
 * @brief Zero-allocation string parsing, trimming, and tokenization utilities.
 */

#include "syn_str.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

size_t syn_str_split(char *str, char delimiter, char *tokens[], size_t max_tokens)
{
    if (str == NULL || tokens == NULL || max_tokens == 0U) {
        return 0U;
    }

    size_t count = 0U;
    tokens[count++] = str;

    while (*str != '\0' && count < max_tokens) {
        if (*str == delimiter) {
            *str = '\0';
            tokens[count++] = str + 1;
        } else if (*str == '*') {
            /* NMEA / checksum terminator support */
            *str = '\0';
            break;
        }
        str++;
    }
    return count;
}

char *syn_str_trim(char *str)
{
    if (str == NULL) {
        return NULL;
    }

    /* Trim leading whitespace */
    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    /* Trim trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

bool syn_str_to_i32(const char *str, int32_t *out_val)
{
    if (str == NULL || *str == '\0' || out_val == NULL) {
        return false;
    }

    errno = 0;
    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);
    if ((endptr == str) || (*endptr != '\0') || (errno == ERANGE)) {
        return false;
    }
#if LONG_MAX > INT32_MAX
    if (val < (long)INT32_MIN || val > (long)INT32_MAX) {
        return false;
    }
#endif

    *out_val = (int32_t)val;
    return true;
}

bool syn_str_to_u32(const char *str, uint32_t *out_val)
{
    if (str == NULL || *str == '\0' || out_val == NULL) {
        return false;
    }

    const char *p = str;
    while (isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '-') {
        return false;
    }

    errno = 0;
    char *endptr = NULL;
    unsigned long val = strtoul(p, &endptr, 10);
    if ((endptr == p) || (*endptr != '\0') || (errno == ERANGE)) {
        return false;
    }
#if ULONG_MAX > UINT32_MAX
    if (val > (unsigned long)UINT32_MAX) {
        return false;
    }
#endif

    *out_val = (uint32_t)val;
    return true;
}
