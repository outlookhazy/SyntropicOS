

# File syn\_fmt.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fmt.c**](syn__fmt_8c.md)

[Go to the documentation of this file](syn__fmt_8c.md)


```C++
#include "../common/syn_defs.h"

#if !defined(SYN_USE_FMT) || SYN_USE_FMT


#include "syn_fmt.h"

/* ── Internal helpers ───────────────────────────────────────────────────── */

static size_t write_char(char *buf, size_t size, size_t pos, char c)
{
    if (pos < size - 1U) {
        buf[pos] = c;
    }
    return pos + 1U;
}

static size_t write_str(char *buf, size_t size, size_t pos, const char *s)
{
    while (*s) {
        pos = write_char(buf, size, pos, *s++);
    }
    return pos;
}

static size_t terminate_buf(char *buf, size_t size, size_t pos)
{
    if (pos < size) {
        buf[pos] = '\0';
        return pos;
    }
    buf[size - 1U] = '\0';
    return size - 1U;
}

/* ── Unsigned integer to decimal ────────────────────────────────────────── */

size_t syn_fmt_uint(char *buf, size_t size, uint32_t val)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }

    char tmp[11]; /* max 10 digits for uint32 */
    int i = 0;

    if (val == 0U) {
        tmp[i++] = '0';
    } else {
        while (val > 0U) {
            tmp[i++] = (char)('0' + (val % 10U));
            val /= 10U;
        }
    }

    /* Reverse into output */
    size_t pos = 0U;
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    return terminate_buf(buf, size, pos);
}

/* ── Signed integer to decimal ──────────────────────────────────────────── */

size_t syn_fmt_int(char *buf, size_t size, int32_t val)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }

    size_t pos = 0U;
    uint32_t uval;

    if (val < 0) {
        pos = write_char(buf, size, pos, '-');
        /* Handle INT32_MIN safely */
        uval = (uint32_t)(-(val + 1)) + 1U;
    } else {
        uval = (uint32_t)val;
    }

    char tmp[11];
    int i = 0;

    if (uval == 0U) {
        tmp[i++] = '0';
    } else {
        while (uval > 0U) {
            tmp[i++] = (char)('0' + (uval % 10U));
            uval /= 10U;
        }
    }

    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    return terminate_buf(buf, size, pos);
}

/* ── Hex ────────────────────────────────────────────────────────────────── */

static const char hex_chars[] = "0123456789ABCDEF";

size_t syn_fmt_hex(char *buf, size_t size, uint32_t val, uint8_t min_digits)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }
    if (min_digits > 8U) {
        min_digits = 8U;
    }

    char tmp[8];
    int i = 0;

    if ((val == 0U) && (min_digits == 0U)) {
        min_digits = 1U;
    }

    while ((val > 0U) || (i < (int)min_digits)) {
        tmp[i++] = hex_chars[val & 0xFU];
        val >>= 4U;
        if (i >= 8) {
            break;
        }
    }

    size_t pos = 0U;
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    return terminate_buf(buf, size, pos);
}

static int hex_char_to_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    return -1;
}

size_t syn_fmt_hex_parse(const char *hex_str, uint8_t *out_bin, size_t max_bytes)
{
    if (!hex_str || !out_bin || max_bytes == 0)
        return 0;

    size_t bytes = 0;
    while (hex_str[0] != '\0' && hex_str[1] != '\0' && bytes < max_bytes) {
        int hi = hex_char_to_nibble(hex_str[0]);
        int lo = hex_char_to_nibble(hex_str[1]);
        if (hi < 0 || lo < 0)
            break;
        out_bin[bytes++] = (uint8_t)((hi << 4) | lo);
        hex_str += 2;
    }
    return bytes;
}

/* ── Q16.16 fixed-point ─────────────────────────────────────────────────── */

size_t syn_fmt_q16(char *buf, size_t size, int32_t q16_val, uint8_t frac_digits)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }
    if (frac_digits > 6U) {
        frac_digits = 6U;
    }

    size_t pos = 0U;
    uint32_t abs_val;

    if (q16_val < 0) {
        pos = write_char(buf, size, pos, '-');
        abs_val = (uint32_t)(-(q16_val + 1)) + 1U;
    } else {
        abs_val = (uint32_t)q16_val;
    }

    /* Integer part */
    uint32_t int_part = abs_val >> 16U;
    char tmp[11];
    int i = 0;

    if (int_part == 0U) {
        tmp[i++] = '0';
    } else {
        while (int_part > 0U) {
            tmp[i++] = (char)('0' + (int_part % 10U));
            int_part /= 10U;
        }
    }
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    /* Fractional part */
    if (frac_digits > 0U) {
        pos = write_char(buf, size, pos, '.');

        /* frac = (frac_bits / 65536) * 10^frac_digits */
        uint32_t frac_bits = abs_val & 0xFFFFU;
        static const uint32_t powers[] = {1U,     10U,     100U,     1000U,
                                          10000U, 100000U, 1000000U, 10000000U};
        uint8_t idx =
            frac_digits & 0x07U; /* clamp already done above; mask silences OOB analysis */
        uint32_t frac = (uint32_t)(((uint64_t)frac_bits * powers[idx]) >> 16U);

        /* Zero-pad to frac_digits width */
        int d;
        for (d = (int)frac_digits - 1; d >= 0; d--) {
            tmp[d] = (char)('0' + (frac % 10U));
            frac /= 10U;
        }
        for (d = 0; d < (int)frac_digits; d++) {
            pos = write_char(buf, size, pos, tmp[d]);
        }
    }

    return terminate_buf(buf, size, pos);
}

/* ── Hex dump ───────────────────────────────────────────────────────────── */

size_t syn_fmt_hexdump(char *buf, size_t size, const uint8_t *data, size_t len)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }
    if ((data == NULL) && (len > 0U)) {
        buf[0] = '\0';
        return 0U;
    }

    size_t pos = 0U;
    size_t i;

    for (i = 0U; i < len; i++) {
        if (i > 0U) {
            pos = write_char(buf, size, pos, ' ');
        }
        pos = write_char(buf, size, pos, hex_chars[(data[i] >> 4U) & 0xFU]);
        pos = write_char(buf, size, pos, hex_chars[data[i] & 0xFU]);
    }

    return terminate_buf(buf, size, pos);
}

/* ── Fixed-point decimal ────────────────────────────────────────────────── */

size_t syn_fmt_fixed(char *buf, size_t size, int32_t val, uint8_t places)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }
    if (places > 9U) {
        places = 9U;
    }

    size_t pos = 0U;
    uint32_t abs_val;

    if (val < 0) {
        pos = write_char(buf, size, pos, '-');
        abs_val = (uint32_t)(-(val + 1)) + 1U;
    } else {
        abs_val = (uint32_t)val;
    }

    /* Compute divisor = 10^places */
    uint32_t div = 1U;
    uint8_t p;
    for (p = 0U; p < places; p++) {
        div *= 10U;
    }

    uint32_t int_part = abs_val / div;
    /* Integer part */
    char tmp[11];
    int i = 0;
    if (int_part == 0U) {
        tmp[i++] = '0';
    } else {
        while (int_part > 0U) {
            tmp[i++] = (char)('0' + (int_part % 10U));
            int_part /= 10U;
        }
    }
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    if (places > 0U) {
        pos = write_char(buf, size, pos, '.');

        /* Zero-padded fractional part */
        uint32_t frac_part = abs_val % div;
        int d;
        for (d = (int)places - 1; d >= 0; d--) {
            tmp[d] = (char)('0' + (frac_part % 10U));
            frac_part /= 10U;
        }
        for (d = 0; d < (int)places; d++) {
            pos = write_char(buf, size, pos, tmp[d]);
        }
    }

    return terminate_buf(buf, size, pos);
}

/* ── Concat ─────────────────────────────────────────────────────────────── */

size_t syn_fmt_concat(char *buf, size_t size, const char *const *parts, size_t n)
{
    if ((buf == NULL) || (size == 0U)) {
        return 0U;
    }
    if ((parts == NULL) && (n > 0U)) {
        buf[0] = '\0';
        return 0U;
    }

    size_t pos = 0U;
    size_t i;

    for (i = 0U; i < n; i++) {
        if (parts[i] != NULL) {
            pos = write_str(buf, size, pos, parts[i]);
        }
    }

    return terminate_buf(buf, size, pos);
}

#endif /* SYN_USE_FMT */
```


