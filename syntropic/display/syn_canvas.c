#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CANVAS) || SYN_USE_CANVAS

/**
 * @file syn_canvas.c
 * @brief Display canvas implementation.
 */

#include "syn_canvas.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Built-in 5×7 font (ASCII 32-126, column-major) ────────────────────── */

/** @brief Built-in 5×7 font data (ASCII 32–126, column-major). */
static const uint8_t font_5x7_data[] = {
    /* Each glyph is 5 bytes (5 columns × 7 rows packed into 5 bytes).
     * Bit 0 = top row, bit 6 = bottom row. */
    0x00,0x00,0x00,0x00,0x00, /* 32: space */
    0x00,0x00,0x5F,0x00,0x00, /* 33: ! */
    0x00,0x07,0x00,0x07,0x00, /* 34: " */
    0x14,0x7F,0x14,0x7F,0x14, /* 35: # */
    0x24,0x2A,0x7F,0x2A,0x12, /* 36: $ */
    0x23,0x13,0x08,0x64,0x62, /* 37: % */
    0x36,0x49,0x55,0x22,0x50, /* 38: & */
    0x00,0x05,0x03,0x00,0x00, /* 39: ' */
    0x00,0x1C,0x22,0x41,0x00, /* 40: ( */
    0x00,0x41,0x22,0x1C,0x00, /* 41: ) */
    0x08,0x2A,0x1C,0x2A,0x08, /* 42: * */
    0x08,0x08,0x3E,0x08,0x08, /* 43: + */
    0x00,0x50,0x30,0x00,0x00, /* 44: , */
    0x08,0x08,0x08,0x08,0x08, /* 45: - */
    0x00,0x60,0x60,0x00,0x00, /* 46: . */
    0x20,0x10,0x08,0x04,0x02, /* 47: / */
    0x3E,0x51,0x49,0x45,0x3E, /* 48: 0 */
    0x00,0x42,0x7F,0x40,0x00, /* 49: 1 */
    0x42,0x61,0x51,0x49,0x46, /* 50: 2 */
    0x21,0x41,0x45,0x4B,0x31, /* 51: 3 */
    0x18,0x14,0x12,0x7F,0x10, /* 52: 4 */
    0x27,0x45,0x45,0x45,0x39, /* 53: 5 */
    0x3C,0x4A,0x49,0x49,0x30, /* 54: 6 */
    0x01,0x71,0x09,0x05,0x03, /* 55: 7 */
    0x36,0x49,0x49,0x49,0x36, /* 56: 8 */
    0x06,0x49,0x49,0x29,0x1E, /* 57: 9 */
    0x00,0x36,0x36,0x00,0x00, /* 58: : */
    0x00,0x56,0x36,0x00,0x00, /* 59: ; */
    0x00,0x08,0x14,0x22,0x41, /* 60: < */
    0x14,0x14,0x14,0x14,0x14, /* 61: = */
    0x41,0x22,0x14,0x08,0x00, /* 62: > */
    0x02,0x01,0x51,0x09,0x06, /* 63: ? */
    0x32,0x49,0x79,0x41,0x3E, /* 64: @ */
    0x7E,0x11,0x11,0x11,0x7E, /* 65: A */
    0x7F,0x49,0x49,0x49,0x36, /* 66: B */
    0x3E,0x41,0x41,0x41,0x22, /* 67: C */
    0x7F,0x41,0x41,0x22,0x1C, /* 68: D */
    0x7F,0x49,0x49,0x49,0x41, /* 69: E */
    0x7F,0x09,0x09,0x01,0x01, /* 70: F */
    0x3E,0x41,0x41,0x51,0x32, /* 71: G */
    0x7F,0x08,0x08,0x08,0x7F, /* 72: H */
    0x00,0x41,0x7F,0x41,0x00, /* 73: I */
    0x20,0x40,0x41,0x3F,0x01, /* 74: J */
    0x7F,0x08,0x14,0x22,0x41, /* 75: K */
    0x7F,0x40,0x40,0x40,0x40, /* 76: L */
    0x7F,0x02,0x04,0x02,0x7F, /* 77: M */
    0x7F,0x04,0x08,0x10,0x7F, /* 78: N */
    0x3E,0x41,0x41,0x41,0x3E, /* 79: O */
    0x7F,0x09,0x09,0x09,0x06, /* 80: P */
    0x3E,0x41,0x51,0x21,0x5E, /* 81: Q */
    0x7F,0x09,0x19,0x29,0x46, /* 82: R */
    0x46,0x49,0x49,0x49,0x31, /* 83: S */
    0x01,0x01,0x7F,0x01,0x01, /* 84: T */
    0x3F,0x40,0x40,0x40,0x3F, /* 85: U */
    0x1F,0x20,0x40,0x20,0x1F, /* 86: V */
    0x7F,0x20,0x18,0x20,0x7F, /* 87: W */
    0x63,0x14,0x08,0x14,0x63, /* 88: X */
    0x03,0x04,0x78,0x04,0x03, /* 89: Y */
    0x61,0x51,0x49,0x45,0x43, /* 90: Z */
    0x00,0x00,0x7F,0x41,0x41, /* 91: [ */
    0x02,0x04,0x08,0x10,0x20, /* 92: backslash */
    0x41,0x41,0x7F,0x00,0x00, /* 93: ] */
    0x04,0x02,0x01,0x02,0x04, /* 94: ^ */
    0x40,0x40,0x40,0x40,0x40, /* 95: _ */
    0x00,0x01,0x02,0x04,0x00, /* 96: ` */
    0x20,0x54,0x54,0x54,0x78, /* 97: a */
    0x7F,0x48,0x44,0x44,0x38, /* 98: b */
    0x38,0x44,0x44,0x44,0x20, /* 99: c */
    0x38,0x44,0x44,0x48,0x7F, /* 100: d */
    0x38,0x54,0x54,0x54,0x18, /* 101: e */
    0x08,0x7E,0x09,0x01,0x02, /* 102: f */
    0x08,0x14,0x54,0x54,0x3C, /* 103: g */
    0x7F,0x08,0x04,0x04,0x78, /* 104: h */
    0x00,0x44,0x7D,0x40,0x00, /* 105: i */
    0x20,0x40,0x44,0x3D,0x00, /* 106: j */
    0x00,0x7F,0x10,0x28,0x44, /* 107: k */
    0x00,0x41,0x7F,0x40,0x00, /* 108: l */
    0x7C,0x04,0x18,0x04,0x78, /* 109: m */
    0x7C,0x08,0x04,0x04,0x78, /* 110: n */
    0x38,0x44,0x44,0x44,0x38, /* 111: o */
    0x7C,0x14,0x14,0x14,0x08, /* 112: p */
    0x08,0x14,0x14,0x18,0x7C, /* 113: q */
    0x7C,0x08,0x04,0x04,0x08, /* 114: r */
    0x48,0x54,0x54,0x54,0x20, /* 115: s */
    0x04,0x3F,0x44,0x40,0x20, /* 116: t */
    0x3C,0x40,0x40,0x20,0x7C, /* 117: u */
    0x1C,0x20,0x40,0x20,0x1C, /* 118: v */
    0x3C,0x40,0x30,0x40,0x3C, /* 119: w */
    0x44,0x28,0x10,0x28,0x44, /* 120: x */
    0x0C,0x50,0x50,0x50,0x3C, /* 121: y */
    0x44,0x64,0x54,0x4C,0x44, /* 122: z */
    0x00,0x08,0x36,0x41,0x00, /* 123: { */
    0x00,0x00,0x7F,0x00,0x00, /* 124: | */
    0x00,0x41,0x36,0x08,0x00, /* 125: } */
    0x08,0x08,0x2A,0x1C,0x08, /* 126: ~ */
};

const SYN_Font syn_font_5x7 = {
    .data       = font_5x7_data,
    .width      = 5,
    .height     = 7,
    .first_char = 32,
    .char_count = 95,
};

/* ── Init ───────────────────────────────────────────────────────────────── */

void syn_canvas_init(SYN_Canvas *c, uint8_t *buf,
                       uint16_t w, uint16_t h, uint8_t bpp,
                       SYN_Canvas_FlushFn flush, void *ctx)
{
    SYN_ASSERT(c != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(bpp == 1 || bpp == 16);

    memset(c, 0, sizeof(*c));
    c->framebuf  = buf;
    c->width     = w;
    c->height    = h;
    c->bpp       = bpp;
    c->font      = &syn_font_5x7;
    c->flush_fn  = flush;
    c->flush_ctx = ctx;
    c->clip_x    = 0;
    c->clip_y    = 0;
    c->clip_w    = (int16_t)w;
    c->clip_h    = (int16_t)h;

    if (bpp == 1) {
        c->buf_size = (size_t)((w + 7) / 8) * h;
    } else {
        c->buf_size = (size_t)w * h * 2;
    }
}

void syn_canvas_set_clip(SYN_Canvas *c, int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(c != NULL);
    c->clip_x = x;
    c->clip_y = y;
    c->clip_w = w;
    c->clip_h = h;
}

void syn_canvas_reset_clip(SYN_Canvas *c)
{
    SYN_ASSERT(c != NULL);
    c->clip_x = 0;
    c->clip_y = 0;
    c->clip_w = (int16_t)c->width;
    c->clip_h = (int16_t)c->height;
}

void syn_canvas_set_font(SYN_Canvas *c, const SYN_Font *font)
{
    SYN_ASSERT(c != NULL);
    c->font = (font != NULL) ? font : &syn_font_5x7;
}

void syn_canvas_clear(SYN_Canvas *c)
{
    SYN_ASSERT(c != NULL);
    memset(c->framebuf, 0, c->buf_size);
}

void syn_canvas_fill(SYN_Canvas *c, uint16_t color)
{
    SYN_ASSERT(c != NULL);
    if (c->bpp == 1) {
        memset(c->framebuf, color ? 0xFF : 0x00, c->buf_size);
    } else {
        /* RGB565: fill with 16-bit color */
        uint8_t hi = (uint8_t)(color >> 8);
        uint8_t lo = (uint8_t)(color);
        for (size_t i = 0; i < c->buf_size; i += 2) {
            c->framebuf[i]     = hi;
            c->framebuf[i + 1] = lo;
        }
    }
}

/* ── Pixel ──────────────────────────────────────────────────────────────── */

void syn_canvas_pixel(SYN_Canvas *c, int16_t x, int16_t y, uint16_t color)
{
    if (x < c->clip_x || x >= c->clip_x + c->clip_w) return;
    if (y < c->clip_y || y >= c->clip_y + c->clip_h) return;

    if (c->bpp == 1) {
        /* Mono: column-major page layout (SSD1306 style).
         * Each byte is a vertical 8-pixel column.
         * Page = y / 8, bit = y % 8. */
        size_t page = (size_t)y / 8;
        size_t idx  = page * c->width + (size_t)x;
        uint8_t bit = (uint8_t)(1 << (y % 8));
        if (color) {
            c->framebuf[idx] |= bit;
        } else {
            c->framebuf[idx] &= ~bit;
        }
    } else {
        /* RGB565: row-major, 2 bytes per pixel */
        size_t idx = ((size_t)y * c->width + (size_t)x) * 2;
        c->framebuf[idx]     = (uint8_t)(color >> 8);
        c->framebuf[idx + 1] = (uint8_t)(color);
    }
}

/* ── Line (Bresenham) ───────────────────────────────────────────────────── */

void syn_canvas_line(SYN_Canvas *c,
                       int16_t x0, int16_t y0,
                       int16_t x1, int16_t y1,
                       uint16_t color)
{
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    for (;;) {
        syn_canvas_pixel(c, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = err * 2;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/* ── Rectangle ──────────────────────────────────────────────────────────── */

void syn_canvas_rect(SYN_Canvas *c,
                       int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color)
{
    syn_canvas_line(c, x, y, x + w - 1, y, color);
    syn_canvas_line(c, x + w - 1, y, x + w - 1, y + h - 1, color);
    syn_canvas_line(c, x + w - 1, y + h - 1, x, y + h - 1, color);
    syn_canvas_line(c, x, y + h - 1, x, y, color);
}

void syn_canvas_rect_fill(SYN_Canvas *c,
                             int16_t x, int16_t y,
                             int16_t w, int16_t h,
                             uint16_t color)
{
    for (int16_t row = y; row < y + h; row++) {
        for (int16_t col = x; col < x + w; col++) {
            syn_canvas_pixel(c, col, row, color);
        }
    }
}

/* ── Circle (Bresenham midpoint) ────────────────────────────────────────── */

void syn_canvas_circle(SYN_Canvas *c,
                          int16_t cx, int16_t cy,
                          int16_t r, uint16_t color)
{
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 1 - r;

    while (x >= y) {
        syn_canvas_pixel(c, cx + x, cy + y, color);
        syn_canvas_pixel(c, cx + y, cy + x, color);
        syn_canvas_pixel(c, cx - y, cy + x, color);
        syn_canvas_pixel(c, cx - x, cy + y, color);
        syn_canvas_pixel(c, cx - x, cy - y, color);
        syn_canvas_pixel(c, cx - y, cy - x, color);
        syn_canvas_pixel(c, cx + y, cy - x, color);
        syn_canvas_pixel(c, cx + x, cy - y, color);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* ── Filled Circle ──────────────────────────────────────────────────────── */

void syn_canvas_circle_fill(SYN_Canvas *c,
                               int16_t cx, int16_t cy,
                               int16_t r, uint16_t color)
{
    SYN_ASSERT(c != NULL);
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 1 - r;

    while (x >= y) {
        for (int16_t i = cx - x; i <= cx + x; i++) {
            syn_canvas_pixel(c, i, cy + y, color);
            syn_canvas_pixel(c, i, cy - y, color);
        }
        for (int16_t i = cx - y; i <= cx + y; i++) {
            syn_canvas_pixel(c, i, cy + x, color);
            syn_canvas_pixel(c, i, cy - x, color);
        }
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* ── Rounded Rectangle ──────────────────────────────────────────────────── */

/**
 * @brief Draw the four rounded corners of a rectangle.
 * @param c      Canvas.
 * @param x      Rectangle X.
 * @param y      Rectangle Y.
 * @param w      Rectangle width.
 * @param h      Rectangle height.
 * @param r      Corner radius.
 * @param color  Pixel color.
 */
static void draw_rounded_corners(SYN_Canvas *c,
                                 int16_t x, int16_t y,
                                 int16_t w, int16_t h,
                                 int16_t r, uint16_t color)
{
    int16_t x_tr = x + w - 1 - r, y_tr = y + r;
    int16_t x_tl = x + r,         y_tl = y + r;
    int16_t x_bl = x + r,         y_bl = y + h - 1 - r;
    int16_t x_br = x + w - 1 - r, y_br = y + h - 1 - r;

    int16_t px = r;
    int16_t py = 0;
    int16_t err = 1 - r;

    while (px >= py) {
        /* Top-Right */
        syn_canvas_pixel(c, x_tr + px, y_tr - py, color);
        syn_canvas_pixel(c, x_tr + py, y_tr - px, color);
        /* Top-Left */
        syn_canvas_pixel(c, x_tl - px, y_tl - py, color);
        syn_canvas_pixel(c, x_tl - py, y_tl - px, color);
        /* Bottom-Left */
        syn_canvas_pixel(c, x_bl - px, y_bl + py, color);
        syn_canvas_pixel(c, x_bl - py, y_bl + px, color);
        /* Bottom-Right */
        syn_canvas_pixel(c, x_br + px, y_br + py, color);
        syn_canvas_pixel(c, x_br + py, y_br + px, color);

        py++;
        if (err < 0) {
            err += 2 * py + 1;
        } else {
            px--;
            err += 2 * (py - px) + 1;
        }
    }
}

void syn_canvas_rect_round(SYN_Canvas *c,
                              int16_t x, int16_t y,
                              int16_t w, int16_t h,
                              int16_t r, uint16_t color)
{
    SYN_ASSERT(c != NULL);
    if (w <= 0 || h <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    if (r <= 0) {
        syn_canvas_rect(c, x, y, w, h, color);
        return;
    }

    /* Draw straight sides */
    syn_canvas_line(c, x + r, y, x + w - 1 - r, y, color);
    syn_canvas_line(c, x + r, y + h - 1, x + w - 1 - r, y + h - 1, color);
    syn_canvas_line(c, x, y + r, x, y + h - 1 - r, color);
    syn_canvas_line(c, x + w - 1, y + r, x + w - 1, y + h - 1 - r, color);

    /* Draw corners */
    draw_rounded_corners(c, x, y, w, h, r, color);
}

void syn_canvas_rect_round_fill(SYN_Canvas *c,
                                   int16_t x, int16_t y,
                                   int16_t w, int16_t h,
                                   int16_t r, uint16_t color)
{
    SYN_ASSERT(c != NULL);
    if (w <= 0 || h <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    if (r <= 0) {
        syn_canvas_rect_fill(c, x, y, w, h, color);
        return;
    }

    /* 1. Draw the middle body (full width rows) */
    for (int16_t row = y + r; row < y + h - r; row++) {
        for (int16_t col = x; col < x + w; col++) {
            syn_canvas_pixel(c, col, row, color);
        }
    }

    /* 2. Draw the rounded top/bottom parts scanline by scanline using Bresenham */
    int16_t px = r;
    int16_t py = 0;
    int16_t err = 1 - r;

    while (px >= py) {
        /* top py */
        for (int16_t col = x + r - px; col <= x + w - 1 - r + px; col++) {
            syn_canvas_pixel(c, col, y + r - py - 1, color);
        }
        /* top px */
        for (int16_t col = x + r - py; col <= x + w - 1 - r + py; col++) {
            syn_canvas_pixel(c, col, y + r - px - 1, color);
        }
        /* bottom py */
        for (int16_t col = x + r - px; col <= x + w - 1 - r + px; col++) {
            syn_canvas_pixel(c, col, y + h - r + py, color);
        }
        /* bottom px */
        for (int16_t col = x + r - py; col <= x + w - 1 - r + py; col++) {
            syn_canvas_pixel(c, col, y + h - r + px, color);
        }

        py++;
        if (err < 0) {
            err += 2 * py + 1;
        } else {
            px--;
            err += 2 * (py - px) + 1;
        }
    }
}

/* ── Bitmap ─────────────────────────────────────────────────────────────── */

void syn_canvas_bitmap(SYN_Canvas *c,
                          int16_t x, int16_t y,
                          const uint8_t *bitmap,
                          int16_t w, int16_t h,
                          uint16_t color)
{
    SYN_ASSERT(c != NULL);
    SYN_ASSERT(bitmap != NULL);
    if (w <= 0 || h <= 0) return;

    int16_t bytes_per_row = (w + 7) / 8;

    for (int16_t row = 0; row < h; row++) {
        for (int16_t col = 0; col < w; col++) {
            int16_t byte_idx = row * bytes_per_row + col / 8;
            uint8_t bit_mask = (uint8_t)(1 << (7 - (col % 8)));
            if (bitmap[byte_idx] & bit_mask) {
                syn_canvas_pixel(c, x + col, y + row, color);
            }
        }
    }
}

/* ── Text ───────────────────────────────────────────────────────────────── */


uint8_t syn_canvas_char(SYN_Canvas *c, int16_t x, int16_t y,
                           char ch, uint16_t color)
{
    const SYN_Font *f = c->font;
    if (f == NULL) return 0;

    uint8_t idx = (uint8_t)ch - f->first_char;
    if (idx >= f->char_count) return f->width + 1;

    const uint8_t *glyph = &f->data[(size_t)idx * f->width];

    for (uint8_t col = 0; col < f->width; col++) {
        uint8_t column = glyph[col];
        for (uint8_t row = 0; row < f->height; row++) {
            if (column & (1 << row)) {
                syn_canvas_pixel(c, x + col, y + row, color);
            }
        }
    }

    return f->width + 1; /* advance = glyph width + 1px spacing */
}

void syn_canvas_text(SYN_Canvas *c, int16_t x, int16_t y,
                       const char *str, uint16_t color)
{
    SYN_ASSERT(c != NULL);
    if (str == NULL) return;

    while (*str) {
        x += (int16_t)syn_canvas_char(c, x, y, *str, color);
        str++;
    }
}

uint16_t syn_canvas_text_width(const SYN_Canvas *c, const char *str)
{
    SYN_ASSERT(c != NULL);
    if (str == NULL || c->font == NULL) return 0;

    uint16_t w = 0;
    while (*str) {
        w += c->font->width + 1;
        str++;
    }
    return (w > 0) ? w - 1 : 0; /* remove trailing spacing */
}

uint8_t syn_canvas_text_height(const SYN_Canvas *c)
{
    SYN_ASSERT(c != NULL);
    if (c->font == NULL) return 0;
    return c->font->height;
}

/* ── Fast horizontal / vertical lines ───────────────────────────────────── */

void syn_canvas_hline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t w, uint16_t color)
{
    int16_t i;
    for (i = 0; i < w; i++) {
        syn_canvas_pixel(c, x + i, y, color);
    }
}

void syn_canvas_vline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t h, uint16_t color)
{
    int16_t i;
    for (i = 0; i < h; i++) {
        syn_canvas_pixel(c, x, y + i, color);
    }
}

/* ── Flush ──────────────────────────────────────────────────────────────── */

void syn_canvas_flush(SYN_Canvas *c)
{
    SYN_ASSERT(c != NULL);
    if (c->flush_fn != NULL) {
        c->flush_fn(c->framebuf, c->buf_size, c->flush_ctx);
    }
}

void syn_canvas_flush_partial(SYN_Canvas *c, size_t offset, size_t len)
{
    SYN_ASSERT(c != NULL);
    if (c->flush_fn == NULL) return;
    if (offset >= c->buf_size) return;
    /* Clamp to buffer bounds */
    if (len > c->buf_size - offset) {
        len = c->buf_size - offset;
    }
    c->flush_fn(c->framebuf + offset, len, c->flush_ctx);
}

#endif /* SYN_USE_CANVAS */
