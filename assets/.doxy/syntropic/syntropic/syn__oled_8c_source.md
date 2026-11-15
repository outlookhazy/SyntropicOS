

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

static void oled_set_cursor(SYN_OLED *oled, uint16_t page, uint16_t col)
{
    uint8_t hw_col = (uint8_t)(oled->col_offset + col);
    send_command(oled, (uint8_t)(0xB0U + (uint8_t)page));
    send_command(oled, (uint8_t)(hw_col & 0x0FU));
    send_command(oled, (uint8_t)(0x10U | (uint8_t)(hw_col >> 4U)));
}

static void oled_flush_page_buffer(SYN_OLED *oled, const uint8_t *buf, uint16_t src_width,
                                   size_t buf_len)
{
    uint16_t pages = (uint16_t)((oled->height + 7U) / 8U);

    for (uint16_t p = 0U; p < pages; p++) {
        oled_set_cursor(oled, p, 0U);

        for (uint16_t x = 0U; x < oled->width; x++) {
            uint8_t byte = 0U;
            if (x < src_width) {
                size_t idx = (size_t)p * (size_t)src_width + (size_t)x;
                if (idx < buf_len) {
                    byte = buf[idx];
                }
            }
            write_i2c_reg(&oled->i2c, oled->i2c_addr, 0x40U, byte);
        }
    }
}

void syn_oled_flush_canvas(SYN_OLED *oled, const SYN_Canvas *c)
{
    if (oled == NULL || c == NULL || c->framebuf == NULL || c->bpp != 1U) {
        return;
    }

    oled->flush_offset = 0U;
    oled_flush_page_buffer(oled, c->framebuf, c->width, c->buf_size);
}

void syn_oled_canvas_flush_cb(const uint8_t *buf, size_t len, void *ctx)
{
    if (buf == NULL || ctx == NULL || len == 0U) {
        return;
    }

    SYN_OLED *oled = (SYN_OLED *)ctx;
    uint16_t pages = (uint16_t)((oled->height + 7U) / 8U);
    size_t total = (size_t)pages * (size_t)oled->width;
    if (total == 0U) {
        return;
    }

    if (len >= total) {
        oled->flush_offset = 0U;
        len = total;
    }

    for (size_t i = 0U; i < len; i++) {
        size_t pos = ((size_t)oled->flush_offset + i) % total;
        uint16_t page = (uint16_t)(pos / (size_t)oled->width);
        uint16_t col = (uint16_t)(pos % (size_t)oled->width);
        if (i == 0U || col == 0U) {
            oled_set_cursor(oled, page, col);
        }
        write_i2c_reg(&oled->i2c, oled->i2c_addr, 0x40U, buf[i]);
    }

    oled->flush_offset = (uint16_t)(((size_t)oled->flush_offset + len) % total);
}
```


