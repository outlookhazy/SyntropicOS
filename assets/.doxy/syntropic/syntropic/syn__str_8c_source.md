

# File syn\_str.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_str.c**](syn__str_8c.md)

[Go to the documentation of this file](syn__str_8c.md)


```C++

#include "syn_str.h"
#include "syn_assert.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

size_t syn_str_split(char *str, char delimiter, char *tokens[], size_t max_tokens)
{
    if (!str || !tokens || max_tokens == 0) return 0;

    size_t count = 0;
    tokens[count++] = str;

    while (*str && count < max_tokens) {
        if (*str == delimiter) {
            *str = '\0';
            tokens[count++] = str + 1;
        } else if (*str == '*') {
            /* NMEA / Checksum terminator support */
            *str = '\0';
            break;
        }
        str++;
    }
    return count;
}

char *syn_str_trim(char *str)
{
    if (!str) return NULL;

    /* Trim leading whitespace */
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') return str;

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
    if (!str || !*str || !out_val) return false;

    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);
    if (endptr == str) return false; /* No digits parsed */

    *out_val = (int32_t)val;
    return true;
}

bool syn_str_to_u32(const char *str, uint32_t *out_val)
{
    if (!str || !*str || !out_val) return false;

    char *endptr = NULL;
    unsigned long val = strtoul(str, &endptr, 10);
    if (endptr == str) return false; /* No digits parsed */

    *out_val = (uint32_t)val;
    return true;
}
```


