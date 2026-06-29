/**
 * @file syn_canvas.h
 * @brief Display canvas — hardware-independent framebuffer + drawing.
 *
 * Supports both monochrome (1bpp, e.g. SSD1306 OLED) and color
 * (16bpp RGB565, e.g. ST7735 TFT) displays. The flush callback
 * pushes the framebuffer to the actual hardware.
 *
 * Usage:
 * @code
 *   // Mono OLED (128×64, 1bpp = 1024 bytes)
 *   static uint8_t fb[128 * 64 / 8];
 *   static SYN_Canvas canvas;
 *   syn_canvas_init(&canvas, fb, 128, 64, 1, my_oled_flush, NULL);
 *
 *   syn_canvas_clear(&canvas);
 *   syn_canvas_text(&canvas, 0, 0, "Hello SyntropicOS!");
 *   syn_canvas_line(&canvas, 0, 16, 127, 16);
 *   syn_canvas_flush(&canvas);
 * @endcode
 * @ingroup syn_display
 */

#ifndef SYN_CANVAS_H
#define SYN_CANVAS_H

#include "../common/syn_defs.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Font descriptor ────────────────────────────────────────────────────── */

/** @brief Font descriptor for text rendering. */
typedef struct {
    const uint8_t *data;        /**< Bitmap data (column-major per glyph)  */
    uint8_t        width;       /**< Glyph width in pixels                 */
    uint8_t        height;      /**< Glyph height in pixels                */
    uint8_t        first_char;  /**< First ASCII code (usually 32)         */
    uint8_t        char_count;  /**< Number of glyphs                      */
} SYN_Font;

/** Built-in 5×7 font (ASCII 32-126). */
extern const SYN_Font syn_font_5x7;

/* ── Flush callback ─────────────────────────────────────────────────────── */

/** @brief Callback to push framebuffer bytes to display hardware. */
typedef void (*SYN_Canvas_FlushFn)(const uint8_t *buf, size_t len, void *ctx);

/* ── Canvas instance ────────────────────────────────────────────────────── */

/** @brief Canvas instance — framebuffer + drawing state. */
typedef struct {
    uint8_t             *framebuf;     /**< Caller-owned pixel buffer       */
    size_t               buf_size;     /**< Total buffer size (bytes)       */
    uint16_t             width;        /**< Display width in pixels         */
    uint16_t             height;       /**< Display height in pixels        */
    uint8_t              bpp;          /**< Bits per pixel (1 or 16)        */
    const SYN_Font     *font;         /**< Active font                     */
    SYN_Canvas_FlushFn  flush_fn;     /**< Push framebuf to display        */
    void                *flush_ctx;    /**< Context for flush callback      */
    int16_t              clip_x;       /**< Clip region left edge           */
    int16_t              clip_y;       /**< Clip region top edge            */
    int16_t              clip_w;       /**< Clip region width               */
    int16_t              clip_h;       /**< Clip region height              */
} SYN_Canvas;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the canvas.
 *
 * @param c       Canvas instance.
 * @param buf     Framebuffer (caller allocates: w*h/8 for 1bpp, w*h*2 for 16bpp).
 * @param w       Width in pixels.
 * @param h       Height in pixels.
 * @param bpp     Bits per pixel (1 for mono, 16 for RGB565).
 * @param flush   Callback to push framebuf to display hardware.
 * @param ctx     Context for flush callback.
 */
void syn_canvas_init(SYN_Canvas *c, uint8_t *buf,
                       uint16_t w, uint16_t h, uint8_t bpp,
                       SYN_Canvas_FlushFn flush, void *ctx);

/**
 * @brief Set the clip rectangle. Drawing is restricted to this region.
 * @param c  Canvas.
 * @param x  Clip left edge.
 * @param y  Clip top edge.
 * @param w  Clip width.
 * @param h  Clip height.
 */
void syn_canvas_set_clip(SYN_Canvas *c, int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Reset clip rectangle to the full display area.
 * @param c  Canvas.
 */
void syn_canvas_reset_clip(SYN_Canvas *c);

/**
 * @brief Set the active font.
 * @param c     Canvas.
 * @param font  Font descriptor, or NULL for built-in 5×7.
 */
void syn_canvas_set_font(SYN_Canvas *c, const SYN_Font *font);

/**
 * @brief Clear the entire framebuffer (fill with zero).
 * @param c  Canvas.
 */
void syn_canvas_clear(SYN_Canvas *c);

/**
 * @brief Fill the entire framebuffer with a color.
 * @param c      Canvas.
 * @param color  Fill color (0/1 for mono, RGB565 for color).
 */
void syn_canvas_fill(SYN_Canvas *c, uint16_t color);

/**
 * @brief Set a single pixel.
 * @param c      Canvas.
 * @param x      X coordinate.
 * @param y      Y coordinate.
 * @param color  Pixel color (0/1 for mono, RGB565 for color).
 */
void syn_canvas_pixel(SYN_Canvas *c, int16_t x, int16_t y, uint16_t color);

/**
 * @brief Draw a line (Bresenham).
 * @param c      Canvas.
 * @param x0     Start X.
 * @param y0     Start Y.
 * @param x1     End X.
 * @param y1     End Y.
 * @param color  Line color.
 */
void syn_canvas_line(SYN_Canvas *c,
                       int16_t x0, int16_t y0,
                       int16_t x1, int16_t y1,
                       uint16_t color);

/**
 * @brief Draw a rectangle (outline only).
 * @param c      Canvas.
 * @param x      Left edge.
 * @param y      Top edge.
 * @param w      Width.
 * @param h      Height.
 * @param color  Outline color.
 */
void syn_canvas_rect(SYN_Canvas *c,
                       int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color);

/**
 * @brief Draw a filled rectangle.
 * @param c      Canvas.
 * @param x      Left edge.
 * @param y      Top edge.
 * @param w      Width.
 * @param h      Height.
 * @param color  Fill color.
 */
void syn_canvas_rect_fill(SYN_Canvas *c,
                             int16_t x, int16_t y,
                             int16_t w, int16_t h,
                             uint16_t color);

/**
 * @brief Draw a circle (Bresenham).
 * @param c      Canvas.
 * @param cx     Center X.
 * @param cy     Center Y.
 * @param r      Radius.
 * @param color  Outline color.
 */
void syn_canvas_circle(SYN_Canvas *c,
                          int16_t cx, int16_t cy,
                          int16_t r, uint16_t color);

/**
 * @brief Draw a filled circle.
 * @param c      Canvas.
 * @param cx     Center X.
 * @param cy     Center Y.
 * @param r      Radius.
 * @param color  Fill color.
 */
void syn_canvas_circle_fill(SYN_Canvas *c,
                               int16_t cx, int16_t cy,
                               int16_t r, uint16_t color);

/**
 * @brief Draw a rounded rectangle (outline only).
 * @param c      Canvas.
 * @param x      Left edge.
 * @param y      Top edge.
 * @param w      Width.
 * @param h      Height.
 * @param r      Corner radius.
 * @param color  Outline color.
 */
void syn_canvas_rect_round(SYN_Canvas *c,
                              int16_t x, int16_t y,
                              int16_t w, int16_t h,
                              int16_t r, uint16_t color);

/**
 * @brief Draw a filled rounded rectangle.
 * @param c      Canvas.
 * @param x      Left edge.
 * @param y      Top edge.
 * @param w      Width.
 * @param h      Height.
 * @param r      Corner radius.
 * @param color  Fill color.
 */
void syn_canvas_rect_round_fill(SYN_Canvas *c,
                                   int16_t x, int16_t y,
                                   int16_t w, int16_t h,
                                   int16_t r, uint16_t color);

/**
 * @brief Draw a monochrome 1bpp bitmap.
 * @param c       Canvas.
 * @param x       Destination X.
 * @param y       Destination Y.
 * @param bitmap  1bpp bitmap data (row-major).
 * @param w       Bitmap width in pixels.
 * @param h       Bitmap height in pixels.
 * @param color   Foreground color for set bits.
 */
void syn_canvas_bitmap(SYN_Canvas *c,
                          int16_t x, int16_t y,
                          const uint8_t *bitmap,
                          int16_t w, int16_t h,
                          uint16_t color);

/**
 * @brief Draw a text string at (x,y) using the active font.
 * @param c      Canvas.
 * @param x      Start X.
 * @param y      Start Y.
 * @param str    Null-terminated string.
 * @param color  Text color.
 */
void syn_canvas_text(SYN_Canvas *c, int16_t x, int16_t y,
                       const char *str, uint16_t color);

/**
 * @brief Draw a single character.
 * @param c      Canvas.
 * @param x      X position.
 * @param y      Y position.
 * @param ch     Character to draw.
 * @param color  Text color.
 * @return Advance width in pixels.
 */
uint8_t syn_canvas_char(SYN_Canvas *c, int16_t x, int16_t y,
                           char ch, uint16_t color);

/**
 * @brief Measure text width in pixels (without drawing).
 * @param c    Canvas (uses active font).
 * @param str  Null-terminated string.
 * @return Width in pixels.
 */
uint16_t syn_canvas_text_width(const SYN_Canvas *c, const char *str);

/**
 * @brief Return the height of the active font in pixels.
 * @param c  Canvas.
 * @return Font height in pixels.
 */
uint8_t syn_canvas_text_height(const SYN_Canvas *c);

/**
 * @brief Draw a fast horizontal line (optimised; no Bresenham overhead).
 * @param c      Canvas.
 * @param x      Start X.
 * @param y      Y position.
 * @param w      Width in pixels.
 * @param color  Color.
 */
void syn_canvas_hline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t w, uint16_t color);

/**
 * @brief Draw a fast vertical line.
 * @param c      Canvas.
 * @param x      X position.
 * @param y      Start Y.
 * @param h      Height in pixels.
 * @param color  Color.
 */
void syn_canvas_vline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t h, uint16_t color);

/**
 * @brief Push framebuffer to display via the flush callback.
 * @param c  Canvas.
 */
void syn_canvas_flush(SYN_Canvas *c);

/**
 * @brief Push a slice of the framebuffer to the display.
 *
 * Calls the flush callback with a pointer into the framebuffer at
 * @p offset with length @p len.  Intended for coroutine-friendly
 * incremental flushing — yield between chunks to avoid blocking the
 * scheduler during slow SPI/I2C transfers.
 *
 * @par Coroutine usage
 * @code
 *   static size_t flush_pos;
 *   flush_pos = 0;
 *   while (flush_pos < canvas.buf_size) {
 *       size_t chunk = 128u;
 *       if (chunk > canvas.buf_size - flush_pos)
 *           chunk = canvas.buf_size - flush_pos;
 *       syn_canvas_flush_partial(&canvas, flush_pos, chunk);
 *       flush_pos += chunk;
 *       PT_YIELD(pt);   // other tasks run here
 *   }
 * @endcode
 *
 * @param c       Canvas.
 * @param offset  Byte offset into the framebuffer to start from.
 * @param len     Number of bytes to send.
 *
 * @note  The display driver's flush_fn receives
 *        @c (framebuf+offset, len, ctx) — the same three-argument
 *        signature as a full flush.  It must be written to handle
 *        sequential partial writes correctly (i.e. not re-issue a
 *        start-of-frame command before each chunk).
 */
void syn_canvas_flush_partial(SYN_Canvas *c, size_t offset, size_t len);

/* ── Color helpers (RGB565) ─────────────────────────────────────────────── */

/**
 * @brief Construct RGB565 color from 8-bit R, G, B components.
 * @param r  Red (0-255).
 * @param g  Green (0-255).
 * @param b  Blue (0-255).
 * @return RGB565 16-bit color value.
 */
static inline uint16_t syn_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

/** @name Predefined RGB565 colors
 * @{
 */
#define SYN_COLOR_BLACK   0x0000  /**< Black   */
#define SYN_COLOR_WHITE   0xFFFF  /**< White   */
#define SYN_COLOR_RED     0xF800  /**< Red     */
#define SYN_COLOR_GREEN   0x07E0  /**< Green   */
#define SYN_COLOR_BLUE    0x001F  /**< Blue    */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SYN_CANVAS_H */
