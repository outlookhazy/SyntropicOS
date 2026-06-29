/**
 * @file syn_gfx.h
 * @brief Compile-time graphics renderer abstraction.
 *
 * Provides a hardware-independent drawing API that the IMGUI widget
 * layer (and user code) can call without being tied to a specific
 * rendering backend.
 *
 * The active backend is selected at compile time via the
 * @c SYN_GFX_BACKEND macro in @c syn_config.h.  If not defined,
 * the framebuffer canvas backend (@c SYN_GFX_BACKEND_CANVAS) is
 * used by default.
 *
 * @par Supported backends
 * | Macro value                | Backend                          |
 * |----------------------------|----------------------------------|
 * | @c SYN_GFX_BACKEND_CANVAS | Framebuffer canvas (syn_canvas)  |
 * | @c SYN_GFX_BACKEND_DIRECT | Direct-draw (no framebuffer)     |
 *
 * All @c syn_gfx_* symbols resolve to direct function calls (or
 * macros) at compile time — zero indirection overhead at runtime.
 *
 * @par Usage
 * @code
 *   // Widget / application code includes only this header:
 *   #include "display/syn_gfx.h"
 *
 *   syn_gfx_clear(gfx);
 *   syn_gfx_rect_fill(gfx, 0, 0, 40, 12, SYN_COLOR_WHITE);
 *   syn_gfx_text(gfx, 4, 2, "Hello", SYN_COLOR_BLACK);
 *   syn_gfx_flush(gfx);
 * @endcode
 * @ingroup syn_display
 */

#ifndef SYN_GFX_H
#define SYN_GFX_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

/* ── Backend identifiers ────────────────────────────────────────────────── */

#define SYN_GFX_BACKEND_CANVAS  0   /**< Framebuffer canvas (default)      */
#define SYN_GFX_BACKEND_DIRECT  1   /**< Direct-draw (no framebuffer)      */

/* Default to the framebuffer canvas if no backend is specified. */
#ifndef SYN_GFX_BACKEND
/** @brief Selected graphics backend (SYN_GFX_BACKEND_CANVAS or SYN_GFX_BACKEND_DIRECT). */
#define SYN_GFX_BACKEND  SYN_GFX_BACKEND_CANVAS
#endif

/* ══════════════════════════════════════════════════════════════════════════
 * Backend: Framebuffer Canvas
 * ══════════════════════════════════════════════════════════════════════════ */

#if SYN_GFX_BACKEND == SYN_GFX_BACKEND_CANVAS

#include "syn_canvas.h"

/** Opaque graphics context — resolves to a canvas pointer. */
typedef SYN_Canvas *SYN_GfxContext;

/** @name Drawing primitives
 * Macro wrappers that dispatch to the selected backend.
 * @{
 */
#define syn_gfx_clear(ctx)                           syn_canvas_clear(ctx)              /**< Clear framebuffer. */
#define syn_gfx_fill(ctx, color)                     syn_canvas_fill((ctx), (color))    /**< Fill with solid color. */
#define syn_gfx_pixel(ctx, x, y, color)              syn_canvas_pixel((ctx), (x), (y), (color))  /**< Draw pixel. */
#define syn_gfx_line(ctx, x0, y0, x1, y1, color)     syn_canvas_line((ctx), (x0), (y0), (x1), (y1), (color))  /**< Draw line. */
#define syn_gfx_rect(ctx, x, y, w, h, color)         syn_canvas_rect((ctx), (x), (y), (w), (h), (color))  /**< Draw rect outline. */
#define syn_gfx_rect_fill(ctx, x, y, w, h, color)    syn_canvas_rect_fill((ctx), (x), (y), (w), (h), (color))  /**< Draw filled rect. */
#define syn_gfx_circle(ctx, cx, cy, r, color)        syn_canvas_circle((ctx), (cx), (cy), (r), (color))  /**< Draw circle outline. */
#define syn_gfx_circle_fill(ctx, cx, cy, r, color)   syn_canvas_circle_fill((ctx), (cx), (cy), (r), (color))  /**< Draw filled circle. */
#define syn_gfx_rect_round(ctx, x, y, w, h, r, color)      syn_canvas_rect_round((ctx), (x), (y), (w), (h), (r), (color))  /**< Draw rounded rect. */
#define syn_gfx_rect_round_fill(ctx, x, y, w, h, r, color) syn_canvas_rect_round_fill((ctx), (x), (y), (w), (h), (r), (color))  /**< Draw filled rounded rect. */
#define syn_gfx_bitmap(ctx, x, y, bmp, w, h, color)  syn_canvas_bitmap((ctx), (x), (y), (bmp), (w), (h), (color))  /**< Draw 1-bit bitmap. */
#define syn_gfx_hline(ctx, x, y, w, color)           syn_canvas_hline((ctx), (x), (y), (w), (color))  /**< Horizontal line. */
#define syn_gfx_vline(ctx, x, y, h, color)           syn_canvas_vline((ctx), (x), (y), (h), (color))  /**< Vertical line. */
/** @} */

/** @name Text rendering
 * @{
 */
#define syn_gfx_text(ctx, x, y, str, color)          syn_canvas_text((ctx), (x), (y), (str), (color))  /**< Draw text string. */
#define syn_gfx_char(ctx, x, y, ch, color)           syn_canvas_char((ctx), (x), (y), (ch), (color))  /**< Draw single char. */
#define syn_gfx_text_width(ctx, str)                 syn_canvas_text_width((ctx), (str))  /**< Measure text width. */
#define syn_gfx_text_height(ctx)                     syn_canvas_text_height(ctx)  /**< Get text line height. */
/** @} */

/* ── Font query ─────────────────────────────────────────────────────────── */

/** Return the glyph width of the active font (pixels). */
#define syn_gfx_font_width(ctx)  ((ctx)->font ? (ctx)->font->width : 5)

/** @name Clipping
 * @{
 */
#define syn_gfx_set_clip(ctx, x, y, w, h)    syn_canvas_set_clip((ctx), (x), (y), (w), (h))  /**< Set clip rectangle. */
#define syn_gfx_reset_clip(ctx)              syn_canvas_reset_clip(ctx)  /**< Reset clip to full canvas. */
/** @} */

/* ── Flush ──────────────────────────────────────────────────────────────── */

/** @brief Flush the entire framebuffer to the display. */
#define syn_gfx_flush(ctx)                           syn_canvas_flush(ctx)
/** @brief Flush a partial region of the framebuffer. */
#define syn_gfx_flush_partial(ctx, offset, len)      syn_canvas_flush_partial((ctx), (offset), (len))

/* ══════════════════════════════════════════════════════════════════════════
 * Backend: Direct-draw (stub — future implementation)
 * ══════════════════════════════════════════════════════════════════════════ */

#elif SYN_GFX_BACKEND == SYN_GFX_BACKEND_DIRECT

  #error "SYN_GFX_BACKEND_DIRECT is not yet implemented."

#else
  #error "Unknown SYN_GFX_BACKEND value."
#endif

#endif /* SYN_GFX_H */
