

# File syn\_gfx.h

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_gfx.h**](syn__gfx_8h.md)

[Go to the documentation of this file](syn__gfx_8h.md)


```C++

#ifndef SYN_GFX_H
#define SYN_GFX_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

/* ── Backend identifiers ────────────────────────────────────────────────── */

#define SYN_GFX_BACKEND_CANVAS  0   
#define SYN_GFX_BACKEND_DIRECT  1   
/* Default to the framebuffer canvas if no backend is specified. */
#ifndef SYN_GFX_BACKEND
#define SYN_GFX_BACKEND  SYN_GFX_BACKEND_CANVAS
#endif

/* ══════════════════════════════════════════════════════════════════════════
 * Backend: Framebuffer Canvas
 * ══════════════════════════════════════════════════════════════════════════ */

#if SYN_GFX_BACKEND == SYN_GFX_BACKEND_CANVAS

#include "syn_canvas.h"

typedef SYN_Canvas *SYN_GfxContext;

#define syn_gfx_clear(ctx)                           syn_canvas_clear(ctx)              
#define syn_gfx_fill(ctx, color)                     syn_canvas_fill((ctx), (color))    
#define syn_gfx_pixel(ctx, x, y, color)              syn_canvas_pixel((ctx), (x), (y), (color))  
#define syn_gfx_line(ctx, x0, y0, x1, y1, color)     syn_canvas_line((ctx), (x0), (y0), (x1), (y1), (color))  
#define syn_gfx_rect(ctx, x, y, w, h, color)         syn_canvas_rect((ctx), (x), (y), (w), (h), (color))  
#define syn_gfx_rect_fill(ctx, x, y, w, h, color)    syn_canvas_rect_fill((ctx), (x), (y), (w), (h), (color))  
#define syn_gfx_circle(ctx, cx, cy, r, color)        syn_canvas_circle((ctx), (cx), (cy), (r), (color))  
#define syn_gfx_circle_fill(ctx, cx, cy, r, color)   syn_canvas_circle_fill((ctx), (cx), (cy), (r), (color))  
#define syn_gfx_rect_round(ctx, x, y, w, h, r, color)      syn_canvas_rect_round((ctx), (x), (y), (w), (h), (r), (color))  
#define syn_gfx_rect_round_fill(ctx, x, y, w, h, r, color) syn_canvas_rect_round_fill((ctx), (x), (y), (w), (h), (r), (color))  
#define syn_gfx_bitmap(ctx, x, y, bmp, w, h, color)  syn_canvas_bitmap((ctx), (x), (y), (bmp), (w), (h), (color))  
#define syn_gfx_hline(ctx, x, y, w, color)           syn_canvas_hline((ctx), (x), (y), (w), (color))  
#define syn_gfx_vline(ctx, x, y, h, color)           syn_canvas_vline((ctx), (x), (y), (h), (color))  
#define syn_gfx_text(ctx, x, y, str, color)          syn_canvas_text((ctx), (x), (y), (str), (color))  
#define syn_gfx_char(ctx, x, y, ch, color)           syn_canvas_char((ctx), (x), (y), (ch), (color))  
#define syn_gfx_text_width(ctx, str)                 syn_canvas_text_width((ctx), (str))  
#define syn_gfx_text_height(ctx)                     syn_canvas_text_height(ctx)  
/* ── Font query ─────────────────────────────────────────────────────────── */

#define syn_gfx_font_width(ctx)  ((ctx)->font ? (ctx)->font->width : 5)

#define syn_gfx_set_clip(ctx, x, y, w, h)    syn_canvas_set_clip((ctx), (x), (y), (w), (h))  
#define syn_gfx_reset_clip(ctx)              syn_canvas_reset_clip(ctx)  
/* ── Flush ──────────────────────────────────────────────────────────────── */

#define syn_gfx_flush(ctx)                           syn_canvas_flush(ctx)
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
```


