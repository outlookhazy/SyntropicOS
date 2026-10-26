

# File syn\_uds\_util.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds\_util.c**](syn__uds__util_8c.md)

[Go to the documentation of this file](syn__uds__util_8c.md)


```C++

#include "syntropic/proto/syn_uds_util.h"

#include "syntropic/util/syn_fmt.h"

#include <ctype.h>
#include <string.h>

static int hex_char_to_nibble(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

bool syn_uds_dtc_from_str(const char *str, uint32_t *dtc_out)
{
    if (!str || !dtc_out) {
        return false;
    }

    /* Category prefix check */
    char prefix = (char)toupper((unsigned char)str[0]);
    uint8_t category_bits = 0;
    switch (prefix) {
    case 'P':
        category_bits = 0x00U;
        break;
    case 'C':
        category_bits = 0x01U;
        break;
    case 'B':
        category_bits = 0x02U;
        break;
    case 'U':
        category_bits = 0x03U;
        break;
    default:
        return false;
    }

    size_t len = strlen(str);
    if (len < 5U || len > 7U) {
        return false;
    }

    /* Parse up to 6 remaining hex characters */
    uint8_t nibbles[6] = {0};
    for (size_t i = 1U; i < len; i++) {
        int n = hex_char_to_nibble(str[i]);
        if (n < 0) {
            return false;
        }
        nibbles[i - 1U] = (uint8_t)n;
    }

    /* High Byte = (Category << 6) | (Type << 4) | Subsystem */
    uint8_t high_byte = (category_bits << 6) | ((nibbles[0] & 0x03U) << 4) | (nibbles[1] & 0x0FU);

    /* Middle Byte = (Digit 3 << 4) | Digit 4 */
    uint8_t mid_byte = ((nibbles[2] & 0x0FU) << 4) | (nibbles[3] & 0x0FU);

    /* Low Byte = (Digit 5 << 4) | Digit 6 (or 0x00 if 5 digits) */
    uint8_t low_byte = 0;
    if (len >= 6U) {
        low_byte |= (uint8_t)((nibbles[4] & 0x0FU) << 4);
    }
    if (len >= 7U) {
        low_byte |= (uint8_t)(nibbles[5] & 0x0FU);
    }

    *dtc_out = ((uint32_t)high_byte << 16) | ((uint32_t)mid_byte << 8) | low_byte;
    return true;
}

bool syn_uds_dtc_to_str(uint32_t dtc, char *out_buf, size_t buf_size)
{
    if (!out_buf || buf_size < 8U) {
        return false;
    }

    uint8_t high_byte = (uint8_t)((dtc >> 16) & 0xFFU);
    uint8_t mid_byte = (uint8_t)((dtc >> 8) & 0xFFU);
    uint8_t low_byte = (uint8_t)(dtc & 0xFFU);

    static const char CATEGORY_CHARS[4] = {'P', 'C', 'B', 'U'};
    uint8_t cat_idx = (high_byte >> 6) & 0x03U;
    uint8_t type_digit = (high_byte >> 4) & 0x03U;
    uint8_t sub_digit = high_byte & 0x0FU;

    out_buf[0] = CATEGORY_CHARS[cat_idx];
    out_buf[1] = (char)('0' + type_digit);

    static const char HEX_DIGITS[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    out_buf[2] = HEX_DIGITS[sub_digit];
    out_buf[3] = HEX_DIGITS[(mid_byte >> 4) & 0x0FU];
    out_buf[4] = HEX_DIGITS[mid_byte & 0x0FU];
    out_buf[5] = HEX_DIGITS[(low_byte >> 4) & 0x0FU];
    out_buf[6] = HEX_DIGITS[low_byte & 0x0FU];
    out_buf[7] = '\0';

    return true;
}
```


