

# File syn\_shiftreg.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_shiftreg.c**](syn__shiftreg_8c.md)

[Go to the documentation of this file](syn__shiftreg_8c.md)


```C++

#include "syn_shiftreg.h"

#include "../util/syn_assert.h"

#include <string.h>

/* ── Output Shift Register (74HC595) Implementation ───────────────────── */

SYN_Status syn_shiftreg_out_init(SYN_ShiftRegOut *sr, SYN_GPIO_Pin data_pin, SYN_GPIO_Pin clock_pin,
                                 SYN_GPIO_Pin latch_pin, uint8_t num_chips)
{
    SYN_ASSERT(sr != NULL);
    SYN_ASSERT(num_chips > 0 && num_chips <= SYN_SHIFTREG_MAX_CHIPS);

    memset(sr, 0, sizeof(*sr));
    sr->data_pin = data_pin;
    sr->clock_pin = clock_pin;
    sr->latch_pin = latch_pin;
    sr->num_chips = num_chips;
    sr->msb_first = true;

    syn_port_gpio_init(data_pin, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(clock_pin, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(latch_pin, SYN_GPIO_OUTPUT);

    syn_port_gpio_write(data_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(clock_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(latch_pin, SYN_GPIO_LOW);

    return SYN_OK;
}

void syn_shiftreg_out_set_oe_pin(SYN_ShiftRegOut *sr, SYN_GPIO_Pin oe_pin)
{
    if (sr == NULL)
        return;
    sr->oe_pin = oe_pin;
    syn_port_gpio_init(oe_pin, SYN_GPIO_OUTPUT);
    syn_port_gpio_write(oe_pin, SYN_GPIO_LOW); /* OE active LOW */
}

void syn_shiftreg_out_set_bit_order(SYN_ShiftRegOut *sr, bool msb_first)
{
    if (sr == NULL)
        return;
    sr->msb_first = msb_first;
}

void syn_shiftreg_out_set_bit(SYN_ShiftRegOut *sr, uint16_t bit_index, bool state)
{
    if (sr == NULL || bit_index >= (uint16_t)(sr->num_chips * 8))
        return;

    uint8_t chip_idx = (uint8_t)(bit_index / 8);
    uint8_t bit_pos = (uint8_t)(bit_index % 8);

    if (state) {
        sr->buffer[chip_idx] |= (uint8_t)(1u << bit_pos);
    } else {
        sr->buffer[chip_idx] &= (uint8_t)~(1u << bit_pos);
    }
}

void syn_shiftreg_out_write_byte(SYN_ShiftRegOut *sr, uint8_t chip_index, uint8_t val)
{
    if (sr == NULL || chip_index >= sr->num_chips)
        return;
    sr->buffer[chip_index] = val;
}

void syn_shiftreg_out_flush(SYN_ShiftRegOut *sr)
{
    if (sr == NULL)
        return;

    /* Shift out chips starting from last chip in cascade chain */
    for (int c = (int)sr->num_chips - 1; c >= 0; c--) {
        uint8_t byte = sr->buffer[c];

        for (int b = 0; b < 8; b++) {
            bool bit;
            if (sr->msb_first) {
                bit = (byte & (0x80 >> b)) != 0;
            } else {
                bit = (byte & (0x01 << b)) != 0;
            }

            syn_port_gpio_write(sr->data_pin, bit ? SYN_GPIO_HIGH : SYN_GPIO_LOW);

            /* Pulse Clock High then Low */
            syn_port_gpio_write(sr->clock_pin, SYN_GPIO_HIGH);
            syn_port_gpio_write(sr->clock_pin, SYN_GPIO_LOW);
        }
    }

    /* Pulse Latch High then Low to update outputs simultaneously */
    syn_port_gpio_write(sr->latch_pin, SYN_GPIO_HIGH);
    syn_port_gpio_write(sr->latch_pin, SYN_GPIO_LOW);
}

void syn_shiftreg_out_set_enable(SYN_ShiftRegOut *sr, bool enable)
{
    if (sr == NULL || sr->oe_pin == 0)
        return;
    /* OE active LOW: LOW = enabled, HIGH = disabled/high-Z */
    syn_port_gpio_write(sr->oe_pin, enable ? SYN_GPIO_LOW : SYN_GPIO_HIGH);
}

/* ── Input Shift Register (74HC165) Implementation ─────────────────────── */

SYN_Status syn_shiftreg_in_init(SYN_ShiftRegIn *sr, SYN_GPIO_Pin data_pin, SYN_GPIO_Pin clock_pin,
                                SYN_GPIO_Pin load_pin, uint8_t num_chips)
{
    SYN_ASSERT(sr != NULL);
    SYN_ASSERT(num_chips > 0 && num_chips <= SYN_SHIFTREG_MAX_CHIPS);

    memset(sr, 0, sizeof(*sr));
    sr->data_pin = data_pin;
    sr->clock_pin = clock_pin;
    sr->load_pin = load_pin;
    sr->num_chips = num_chips;
    sr->msb_first = true;

    syn_port_gpio_init(data_pin, SYN_GPIO_INPUT);
    syn_port_gpio_init(clock_pin, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(load_pin, SYN_GPIO_OUTPUT);

    syn_port_gpio_write(clock_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(load_pin, SYN_GPIO_HIGH); /* Idle HIGH */

    return SYN_OK;
}

void syn_shiftreg_in_set_bit_order(SYN_ShiftRegIn *sr, bool msb_first)
{
    if (sr == NULL)
        return;
    sr->msb_first = msb_first;
}

void syn_shiftreg_in_read(SYN_ShiftRegIn *sr)
{
    if (sr == NULL)
        return;

    /* Pulse Load pin LOW then HIGH to sample parallel inputs into internal latch */
    syn_port_gpio_write(sr->load_pin, SYN_GPIO_LOW);
    syn_port_gpio_write(sr->load_pin, SYN_GPIO_HIGH);

    /* Clock in bits for all chips */
    for (int c = (int)sr->num_chips - 1; c >= 0; c--) {
        uint8_t val = 0;

        for (int b = 0; b < 8; b++) {
            bool bit = (syn_port_gpio_read(sr->data_pin) == SYN_GPIO_HIGH);

            if (sr->msb_first) {
                if (bit)
                    val |= (uint8_t)(0x80 >> b);
            } else {
                if (bit)
                    val |= (uint8_t)(0x01 << b);
            }

            /* Pulse Clock High then Low */
            syn_port_gpio_write(sr->clock_pin, SYN_GPIO_HIGH);
            syn_port_gpio_write(sr->clock_pin, SYN_GPIO_LOW);
        }

        sr->buffer[c] = val;
    }
}

bool syn_shiftreg_in_get_bit(const SYN_ShiftRegIn *sr, uint16_t bit_index)
{
    if (sr == NULL || bit_index >= (uint16_t)(sr->num_chips * 8))
        return false;

    uint8_t chip_idx = (uint8_t)(bit_index / 8);
    uint8_t bit_pos = (uint8_t)(bit_index % 8);

    return (sr->buffer[chip_idx] & (1u << bit_pos)) != 0;
}

uint8_t syn_shiftreg_in_get_byte(const SYN_ShiftRegIn *sr, uint8_t chip_index)
{
    if (sr == NULL || chip_index >= sr->num_chips)
        return 0;
    return sr->buffer[chip_index];
}
```


