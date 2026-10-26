

# File syn\_oled.c

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_oled.c**](syn__oled_8c.md)

[Go to the documentation of this file](syn__oled_8c.md)


```C++

#include "syn_oled.h"

#include "../util/syn_assert.h"

#include <string.h>

static void write_i2c_reg(SYN_SoftI2C *i2c, uint8_t dev_addr, uint8_t reg, uint8_t val)
{
    syn_soft_i2c_start(i2c);
    syn_soft_i2c_write(i2c, (uint8_t)(dev_addr << 1));
    syn_soft_i2c_write(i2c, reg);
    syn_soft_i2c_write(i2c, val);
    syn_soft_i2c_stop(i2c);
}

static void send_command(SYN_OLED *oled, uint8_t cmd)
{
    write_i2c_reg(&oled->i2c, oled->i2c_addr, 0x00, cmd);
}

SYN_Status syn_oled_init(SYN_OLED *oled, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                         uint16_t w, uint16_t h, SYN_OLEDType type)
{
    SYN_ASSERT(oled != NULL);
    SYN_ASSERT(w > 0 && h > 0);

    memset(oled, 0, sizeof(*oled));
    oled->type = type;
    oled->width = w;
    oled->height = h;
    oled->i2c_addr = i2c_addr;
    oled->col_offset = (type == SYN_OLED_SH1106) ? 2 : 0;
    oled->display_on = true;
    oled->contrast = 0xCF;

    syn_soft_i2c_init(&oled->i2c, scl, sda, 10);

    /* Initialization sequence */
    send_command(oled, 0xAE); /* Display OFF */
    send_command(oled, 0xD5); /* Set Display Clock Divide */
    send_command(oled, 0x80);
    send_command(oled, 0xA8); /* Set Multiplex Ratio */
    send_command(oled, (uint8_t)(h - 1));
    send_command(oled, 0xD3); /* Set Display Offset */
    send_command(oled, 0x00);
    send_command(oled, 0x40); /* Set Start Line 0 */

    send_command(oled, 0x8D); /* Charge Pump */
    send_command(oled, 0x14);

    send_command(oled, 0x20); /* Memory Addressing Mode: Page Addressing */
    send_command(oled, 0x02);

    send_command(oled, 0xA1); /* Segment Re-map */
    send_command(oled, 0xC8); /* COM Output Scan Direction */

    send_command(oled, 0xDA); /* Set COM Pins Hardware Configuration */
    send_command(oled, (h == 64) ? 0x12 : 0x02);

    send_command(oled, 0x81); /* Set Contrast Control */
    send_command(oled, 0xCF);

    send_command(oled, 0xD9); /* Set Pre-charge Period */
    send_command(oled, 0xF1);

    send_command(oled, 0xDB); /* Set VCOMH Deselect Level */
    send_command(oled, 0x40);

    send_command(oled, 0xA4); /* Entire Display ON resume */
    send_command(oled, 0xA6); /* Normal Display */
    send_command(oled, 0xAF); /* Display ON */

    return SYN_OK;
}

void syn_oled_set_contrast(SYN_OLED *oled, uint8_t contrast)
{
    if (oled == NULL)
        return;
    oled->contrast = contrast;
    send_command(oled, 0x81);
    send_command(oled, contrast);
}

void syn_oled_set_invert(SYN_OLED *oled, bool invert)
{
    if (oled == NULL)
        return;
    oled->inverted = invert;
    send_command(oled, invert ? 0xA7 : 0xA6);
}

void syn_oled_set_display_on(SYN_OLED *oled, bool on)
{
    if (oled == NULL)
        return;
    oled->display_on = on;
    send_command(oled, on ? 0xAF : 0xAE);
}

static bool get_canvas_pixel(const SYN_Canvas *c, uint16_t x, uint16_t y)
{
    if (c == NULL || c->framebuf == NULL || x >= c->width || y >= c->height)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    size_t pixel_idx = (size_t)y * c->width + x;
    size_t byte_idx = pixel_idx / 8;
    uint8_t bit_idx = (uint8_t)(7 - (pixel_idx % 8));
    return (c->framebuf[byte_idx] & (1u << bit_idx)) != 0;
}

void syn_oled_flush_canvas(SYN_OLED *oled, const SYN_Canvas *c)
{
    if (oled == NULL || c == NULL || c->framebuf == NULL)
        return;

    uint8_t pages = (uint8_t)(oled->height / 8);

    for (uint8_t p = 0; p < pages; p++) {
        send_command(oled, (uint8_t)(0xB0 + p)); /* Page address */

        uint8_t col = (uint8_t)(0x00 + oled->col_offset);
        send_command(oled, (uint8_t)(col & 0x0F));        /* Column Low Nibble */
        send_command(oled, (uint8_t)(0x10 | (col >> 4))); /* Column High Nibble */

        for (uint16_t x = 0; x < oled->width; x++) {
            uint8_t byte = 0;
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint16_t y = (uint16_t)(p * 8 + bit);
                if (get_canvas_pixel(c, x, y)) {
                    byte |= (uint8_t)(1u << bit);
                }
            }
            write_i2c_reg(&oled->i2c, oled->i2c_addr, 0x40, byte);
        }
    }
}

void syn_oled_canvas_flush_cb(const uint8_t *buf, size_t len, void *ctx)
{
    (void)buf;
    (void)len;
    (void)ctx;
}
```


