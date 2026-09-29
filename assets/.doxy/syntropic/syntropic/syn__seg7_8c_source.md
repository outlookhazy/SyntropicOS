

# File syn\_seg7.c

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_seg7.c**](syn__seg7_8c.md)

[Go to the documentation of this file](syn__seg7_8c.md)


```C++

#include "syn_seg7.h"

#include "../util/syn_assert.h"

#include <stdio.h>
#include <string.h>

static uint8_t char_to_seg7(char c)
{
    switch (c) {
    case '0':
        return 0x3F;
    case '1':
        return 0x06;
    case '2':
        return 0x5B;
    case '3':
        return 0x4F;
    case '4':
        return 0x66;
    case '5':
        return 0x6D;
    case '6':
        return 0x7D;
    case '7':
        return 0x07;
    case '8':
        return 0x7F;
    case '9':
        return 0x6F;
    case 'A':
    case 'a':
        return 0x77;
    case 'B':
    case 'b':
        return 0x7C;
    case 'C':
    case 'c':
        return 0x39;
    case 'D':
    case 'd':
        return 0x5E;
    case 'E':
    case 'e':
        return 0x79;
    case 'F':
    case 'f':
        return 0x71;
    case 'H':
    case 'h':
        return 0x76;
    case 'L':
    case 'l':
        return 0x38;
    case 'P':
    case 'p':
        return 0x73;
    case 'U':
    case 'u':
        return 0x3E;
    case '-':
        return 0x40;
    case '_':
        return 0x08;
    case ' ':
    default:
        return 0x00;
    }
}

SYN_Status syn_seg7_init(SYN_Seg7 *seg, const SYN_GPIO_Pin segments[8], const SYN_GPIO_Pin *digits,
                         uint8_t num_digits, SYN_Seg7Type type)
{
    SYN_ASSERT(seg != NULL);
    SYN_ASSERT(segments != NULL);
    SYN_ASSERT(digits != NULL);
    SYN_ASSERT(num_digits > 0 && num_digits <= SYN_SEG7_MAX_DIGITS);

    memset(seg, 0, sizeof(*seg));
    seg->num_digits = num_digits;
    seg->type = type;
    seg->active_digit = 0;
    seg->leading_zeros = false;

    memcpy(seg->segment_pins, segments, sizeof(SYN_GPIO_Pin) * 8);
    memcpy(seg->digit_pins, digits, sizeof(SYN_GPIO_Pin) * num_digits);

    /* Initialize segment output pins */
    SYN_GPIO_State idle_seg_state =
        (type == SYN_SEG7_COMMON_CATHODE) ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    for (uint8_t i = 0; i < 8; i++) {
        syn_port_gpio_init(seg->segment_pins[i], SYN_GPIO_OUTPUT);
        syn_port_gpio_write(seg->segment_pins[i], idle_seg_state);
    }

    /* Initialize digit select output pins (idle OFF) */
    SYN_GPIO_State idle_digit_state =
        (type == SYN_SEG7_COMMON_CATHODE) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    for (uint8_t d = 0; d < num_digits; d++) {
        syn_port_gpio_init(seg->digit_pins[d], SYN_GPIO_OUTPUT);
        syn_port_gpio_write(seg->digit_pins[d], idle_digit_state);
    }

    return SYN_OK;
}

void syn_seg7_scan(SYN_Seg7 *seg)
{
    if (seg == NULL || seg->num_digits == 0)
        return;

    /* Turn OFF previous active digit selector */
    SYN_GPIO_State off_digit_state =
        (seg->type == SYN_SEG7_COMMON_CATHODE) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    syn_port_gpio_write(seg->digit_pins[seg->active_digit], off_digit_state);

    /* Advance to next digit */
    seg->active_digit = (uint8_t)((seg->active_digit + 1) % seg->num_digits);
    uint8_t mask = seg->digit_buffers[seg->active_digit];

    /* Set segment output levels according to mask & wiring type */
    for (uint8_t s = 0; s < 8; s++) {
        bool bit = (mask & (1u << s)) != 0;
        SYN_GPIO_State state = (seg->type == SYN_SEG7_COMMON_CATHODE)
                                   ? (bit ? SYN_GPIO_HIGH : SYN_GPIO_LOW)
                                   : (bit ? SYN_GPIO_LOW : SYN_GPIO_HIGH);
        syn_port_gpio_write(seg->segment_pins[s], state);
    }

    /* Turn ON active digit selector */
    SYN_GPIO_State on_digit_state =
        (seg->type == SYN_SEG7_COMMON_CATHODE) ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    syn_port_gpio_write(seg->digit_pins[seg->active_digit], on_digit_state);
}

void syn_seg7_clear(SYN_Seg7 *seg)
{
    if (seg == NULL)
        return;
    memset(seg->digit_buffers, 0, sizeof(seg->digit_buffers));
}

void syn_seg7_set_digit_raw(SYN_Seg7 *seg, uint8_t digit_idx, uint8_t seg_mask)
{
    if (seg == NULL || digit_idx >= seg->num_digits)
        return;
    seg->digit_buffers[digit_idx] = seg_mask;
}

void syn_seg7_print_int(SYN_Seg7 *seg, int32_t val)
{
    if (seg == NULL)
        return;

    syn_seg7_clear(seg);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int)val);
    size_t len = strlen(buf);

    if (len > seg->num_digits) {
        /* Overflow: display dashes ---- */
        for (uint8_t i = 0; i < seg->num_digits; i++) {
            seg->digit_buffers[i] = 0x40; /* '-' */
        }
        return;
    }

    /* Right-align string onto digit buffers */
    int start_digit = (int)seg->num_digits - (int)len;
    for (size_t i = 0; i < len; i++) {
        seg->digit_buffers[start_digit + (int)i] = char_to_seg7(buf[i]);
    }
}

void syn_seg7_print_float(SYN_Seg7 *seg, float val, uint8_t decimals)
{
    if (seg == NULL)
        return;

    if (decimals >= seg->num_digits) {
        decimals = (uint8_t)(seg->num_digits > 0 ? seg->num_digits - 1 : 0);
    }

    syn_seg7_clear(seg);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.*f", (int)decimals, (double)val);

    uint8_t dig_idx = 0;
    for (size_t i = 0; buf[i] != '\0' && dig_idx < seg->num_digits; i++) {
        if (buf[i] == '.') {
            if (dig_idx > 0) {
                seg->digit_buffers[dig_idx - 1] |= 0x80; /* Set DP bit */
            }
        } else {
            seg->digit_buffers[dig_idx++] = char_to_seg7(buf[i]);
        }
    }
}

void syn_seg7_print_hex(SYN_Seg7 *seg, uint32_t val)
{
    if (seg == NULL)
        return;

    syn_seg7_clear(seg);

    char buf[16];
    snprintf(buf, sizeof(buf), "%X", (unsigned int)val);
    size_t len = strlen(buf);

    if (len > seg->num_digits) {
        len = seg->num_digits;
    }

    int start_digit = (int)seg->num_digits - (int)len;
    for (size_t i = 0; i < len; i++) {
        seg->digit_buffers[start_digit + (int)i] = char_to_seg7(buf[i]);
    }
}

void syn_seg7_print_str(SYN_Seg7 *seg, const char *str)
{
    if (seg == NULL || str == NULL)
        return;

    syn_seg7_clear(seg);

    uint8_t dig_idx = 0;
    for (size_t i = 0; str[i] != '\0' && dig_idx < seg->num_digits; i++) {
        if (str[i] == '.') {
            if (dig_idx > 0) {
                seg->digit_buffers[dig_idx - 1] |= 0x80;
            }
        } else {
            seg->digit_buffers[dig_idx++] = char_to_seg7(str[i]);
        }
    }
}
```


