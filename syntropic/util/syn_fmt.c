#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FMT) || SYN_USE_FMT

/**
 * @file syn_fmt.c
 * @brief Lightweight formatting — no libc printf dependency.
 */

#include "syn_fmt.h"

/* ── Internal helpers ───────────────────────────────────────────────────── */

/**
 * @brief Write a single char to the output buffer.
 * @param buf   Output buffer.
 * @param size  Buffer capacity.
 * @param pos   Current write position.
 * @param c     Character to write.
 * @return New position (may exceed size if truncated).
 */
static size_t write_char(char *buf, size_t size, size_t pos, char c)
{
    if (pos < size - 1) {
        buf[pos] = c;
    }
    return pos + 1;
}

/**
 * @brief Write a null-terminated string to the output buffer.
 * @param buf   Output buffer.
 * @param size  Buffer capacity.
 * @param pos   Current write position.
 * @param s     String to write.
 * @return New position.
 */
static size_t write_str(char *buf, size_t size, size_t pos, const char *s)
{
    while (*s) {
        pos = write_char(buf, size, pos, *s++);
    }
    return pos;
}

/* ── Unsigned integer to decimal ────────────────────────────────────────── */

size_t syn_fmt_uint(char *buf, size_t size, uint32_t val)
{
    if (size == 0) return 0;

    char tmp[11]; /* max 10 digits for uint32 */
    int i = 0;

    if (val == 0) {
        tmp[i++] = '0';
    } else {
        while (val > 0) {
            tmp[i++] = (char)('0' + (val % 10));
            val /= 10;
        }
    }

    /* Reverse into output */
    size_t pos = 0;
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Signed integer to decimal ──────────────────────────────────────────── */

size_t syn_fmt_int(char *buf, size_t size, int32_t val)
{
    if (size == 0) return 0;

    size_t pos = 0;
    uint32_t uval;

    if (val < 0) {
        pos = write_char(buf, size, pos, '-');
        /* Handle INT32_MIN safely */
        uval = (uint32_t)(-(val + 1)) + 1;
    } else {
        uval = (uint32_t)val;
    }

    char tmp[11];
    int i = 0;

    if (uval == 0) {
        tmp[i++] = '0';
    } else {
        while (uval > 0) {
            tmp[i++] = (char)('0' + (uval % 10));
            uval /= 10;
        }
    }

    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Hex ────────────────────────────────────────────────────────────────── */

/** @brief Hex digit lookup table. */
static const char hex_chars[] = "0123456789ABCDEF";

size_t syn_fmt_hex(char *buf, size_t size, uint32_t val, uint8_t min_digits)
{
    if (size == 0) return 0;
    if (min_digits > 8) min_digits = 8;

    char tmp[8];
    int i = 0;

    if (val == 0 && min_digits == 0) {
        min_digits = 1;
    }

    while (val > 0 || i < (int)min_digits) {
        tmp[i++] = hex_chars[val & 0xF];
        val >>= 4;
        if (i >= 8) break;
    }

    size_t pos = 0;
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Q16.16 fixed-point ─────────────────────────────────────────────────── */

size_t syn_fmt_q16(char *buf, size_t size, int32_t q16_val,
                    uint8_t frac_digits)
{
    if (size == 0) return 0;
    if (frac_digits > 6) frac_digits = 6;

    size_t pos = 0;
    uint32_t abs_val;

    if (q16_val < 0) {
        pos = write_char(buf, size, pos, '-');
        abs_val = (uint32_t)(-(q16_val + 1)) + 1;
    } else {
        abs_val = (uint32_t)q16_val;
    }

    /* Integer part */
    uint32_t int_part = abs_val >> 16;
    char tmp[11];
    int i = 0;

    if (int_part == 0) {
        tmp[i++] = '0';
    } else {
        while (int_part > 0) {
            tmp[i++] = (char)('0' + (int_part % 10));
            int_part /= 10;
        }
    }
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    /* Fractional part */
    if (frac_digits > 0) {
        pos = write_char(buf, size, pos, '.');

        /* frac = (frac_bits / 65536) × 10^frac_digits */
        uint32_t frac_bits = abs_val & 0xFFFF;
        static const uint32_t powers[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
        uint8_t idx = frac_digits & 0x07u; /* clamp already done above; mask silences OOB analysis */
        uint32_t frac = (uint32_t)(((uint64_t)frac_bits * powers[idx]) >> 16);

        /* Zero-pad to frac_digits width */
        int d;
        for (d = (int)frac_digits - 1; d >= 0; d--) {
            tmp[d] = (char)('0' + (frac % 10));
            frac /= 10;
        }
        for (d = 0; d < (int)frac_digits; d++) {
            pos = write_char(buf, size, pos, tmp[d]);
        }
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Hex dump ───────────────────────────────────────────────────────────── */

size_t syn_fmt_hexdump(char *buf, size_t size,
                        const uint8_t *data, size_t len)
{
    if (size == 0) return 0;

    size_t pos = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        if (i > 0) {
            pos = write_char(buf, size, pos, ' ');
        }
        pos = write_char(buf, size, pos, hex_chars[(data[i] >> 4) & 0xF]);
        pos = write_char(buf, size, pos, hex_chars[data[i] & 0xF]);
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Fixed-point decimal ────────────────────────────────────────────────── */

size_t syn_fmt_fixed(char *buf, size_t size, int32_t val, uint8_t places)
{
    if (size == 0) return 0;
    if (places > 9) places = 9;

    size_t pos = 0;
    uint32_t abs_val;

    if (val < 0) {
        pos = write_char(buf, size, pos, '-');
        abs_val = (uint32_t)(-(val + 1)) + 1;
    } else {
        abs_val = (uint32_t)val;
    }

    /* Compute divisor = 10^places */
    uint32_t div = 1;
    uint8_t p;
    for (p = 0; p < places; p++) div *= 10;

    uint32_t int_part  = abs_val / div;
    /* Integer part */
    char tmp[11];
    int i = 0;
    if (int_part == 0) {
        tmp[i++] = '0';
    } else {
        while (int_part > 0) {
            tmp[i++] = (char)('0' + (int_part % 10));
            int_part /= 10;
        }
    }
    while (i > 0) {
        pos = write_char(buf, size, pos, tmp[--i]);
    }

    if (places > 0) {
        pos = write_char(buf, size, pos, '.');

        /* Zero-padded fractional part */
        uint32_t frac_part = abs_val % div;
        int d;
        for (d = (int)places - 1; d >= 0; d--) {
            tmp[d] = (char)('0' + (frac_part % 10));
            frac_part /= 10;
        }
        for (d = 0; d < (int)places; d++) {
            pos = write_char(buf, size, pos, tmp[d]);
        }
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

/* ── Concat ─────────────────────────────────────────────────────────────── */

size_t syn_fmt_concat(char *buf, size_t size,
                       const char *const *parts, size_t n)
{
    if (size == 0) return 0;

    size_t pos = 0;
    size_t i;

    for (i = 0; i < n; i++) {
        if (parts[i] != NULL) {
            pos = write_str(buf, size, pos, parts[i]);
        }
    }

    if (pos < size) buf[pos] = '\0';
    else buf[size - 1] = '\0';

    return (pos < size) ? pos : size - 1;
}

#endif /* SYN_USE_FMT */
