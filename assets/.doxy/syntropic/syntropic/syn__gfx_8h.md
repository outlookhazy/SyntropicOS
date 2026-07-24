

# File syn\_gfx.h



[**FileList**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_gfx.h**](syn__gfx_8h.md)

[Go to the source code of this file](syn__gfx_8h_source.md)

_Compile-time graphics renderer abstraction._ [More...](#detailed-description)

* `#include "syn_canvas.h"`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**SYN\_Canvas**](structSYN__Canvas.md) \* | [**SYN\_GfxContext**](#typedef-syn_gfxcontext)  <br> |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_GFX\_BACKEND**](syn__gfx_8h.md#define-syn_gfx_backend)  `[**SYN\_GFX\_BACKEND\_CANVAS**](syn__gfx_8h.md#define-syn_gfx_backend_canvas)`<br>_Selected graphics backend (SYN\_GFX\_BACKEND\_CANVAS or SYN\_GFX\_BACKEND\_DIRECT)._  |
| define  | [**SYN\_GFX\_BACKEND\_CANVAS**](syn__gfx_8h.md#define-syn_gfx_backend_canvas)  `0`<br> |
| define  | [**SYN\_GFX\_BACKEND\_DIRECT**](syn__gfx_8h.md#define-syn_gfx_backend_direct)  `1`<br> |
| define  | [**syn\_gfx\_bitmap**](syn__gfx_8h.md#define-syn_gfx_bitmap) (ctx, x, y, bmp, w, h, color) `[**syn\_canvas\_bitmap**](syn__canvas_8c.md#function-syn_canvas_bitmap)((ctx), (x), (y), (bmp), (w), (h), (color))`<br> |
| define  | [**syn\_gfx\_char**](syn__gfx_8h.md#define-syn_gfx_char) (ctx, x, y, ch, color) `[**syn\_canvas\_char**](syn__canvas_8c.md#function-syn_canvas_char)((ctx), (x), (y), ([**ch**](syn__sha256_8c.md#function-ch)), (color))`<br> |
| define  | [**syn\_gfx\_circle**](syn__gfx_8h.md#define-syn_gfx_circle) (ctx, cx, cy, r, color) `[**syn\_canvas\_circle**](syn__canvas_8c.md#function-syn_canvas_circle)((ctx), (cx), (cy), (r), (color))`<br> |
| define  | [**syn\_gfx\_circle\_fill**](syn__gfx_8h.md#define-syn_gfx_circle_fill) (ctx, cx, cy, r, color) `[**syn\_canvas\_circle\_fill**](syn__canvas_8c.md#function-syn_canvas_circle_fill)((ctx), (cx), (cy), (r), (color))`<br> |
| define  | [**syn\_gfx\_clear**](syn__gfx_8h.md#define-syn_gfx_clear) (ctx) `[**syn\_canvas\_clear**](syn__canvas_8c.md#function-syn_canvas_clear)(ctx)`<br> |
| define  | [**syn\_gfx\_fill**](syn__gfx_8h.md#define-syn_gfx_fill) (ctx, color) `[**syn\_canvas\_fill**](syn__canvas_8c.md#function-syn_canvas_fill)((ctx), (color))`<br> |
| define  | [**syn\_gfx\_flush**](syn__gfx_8h.md#define-syn_gfx_flush) (ctx) `[**syn\_canvas\_flush**](syn__canvas_8c.md#function-syn_canvas_flush)(ctx)`<br>_Flush the entire framebuffer to the display._  |
| define  | [**syn\_gfx\_flush\_partial**](syn__gfx_8h.md#define-syn_gfx_flush_partial) (ctx, offset, len) `[**syn\_canvas\_flush\_partial**](syn__canvas_8c.md#function-syn_canvas_flush_partial)((ctx), (offset), (len))`<br>_Flush a partial region of the framebuffer._  |
| define  | [**syn\_gfx\_font\_width**](syn__gfx_8h.md#define-syn_gfx_font_width) (ctx) `((ctx)-&gt;font ? (ctx)-&gt;font-&gt;width : 5)`<br> |
| define  | [**syn\_gfx\_hline**](syn__gfx_8h.md#define-syn_gfx_hline) (ctx, x, y, w, color) `[**syn\_canvas\_hline**](syn__canvas_8c.md#function-syn_canvas_hline)((ctx), (x), (y), (w), (color))`<br> |
| define  | [**syn\_gfx\_line**](syn__gfx_8h.md#define-syn_gfx_line) (ctx, x0, y0, x1, y1, color) `[**syn\_canvas\_line**](syn__canvas_8c.md#function-syn_canvas_line)((ctx), (x0), (y0), (x1), (y1), (color))`<br> |
| define  | [**syn\_gfx\_pixel**](syn__gfx_8h.md#define-syn_gfx_pixel) (ctx, x, y, color) `[**syn\_canvas\_pixel**](syn__canvas_8c.md#function-syn_canvas_pixel)((ctx), (x), (y), (color))`<br> |
| define  | [**syn\_gfx\_rect**](syn__gfx_8h.md#define-syn_gfx_rect) (ctx, x, y, w, h, color) `[**syn\_canvas\_rect**](syn__canvas_8c.md#function-syn_canvas_rect)((ctx), (x), (y), (w), (h), (color))`<br> |
| define  | [**syn\_gfx\_rect\_fill**](syn__gfx_8h.md#define-syn_gfx_rect_fill) (ctx, x, y, w, h, color) `[**syn\_canvas\_rect\_fill**](syn__canvas_8c.md#function-syn_canvas_rect_fill)((ctx), (x), (y), (w), (h), (color))`<br> |
| define  | [**syn\_gfx\_rect\_round**](syn__gfx_8h.md#define-syn_gfx_rect_round) (ctx, x, y, w, h, r, color) `[**syn\_canvas\_rect\_round**](syn__canvas_8c.md#function-syn_canvas_rect_round)((ctx), (x), (y), (w), (h), (r), (color))`<br> |
| define  | [**syn\_gfx\_rect\_round\_fill**](syn__gfx_8h.md#define-syn_gfx_rect_round_fill) (ctx, x, y, w, h, r, color) `[**syn\_canvas\_rect\_round\_fill**](syn__canvas_8c.md#function-syn_canvas_rect_round_fill)((ctx), (x), (y), (w), (h), (r), (color))`<br> |
| define  | [**syn\_gfx\_reset\_clip**](syn__gfx_8h.md#define-syn_gfx_reset_clip) (ctx) `[**syn\_canvas\_reset\_clip**](syn__canvas_8c.md#function-syn_canvas_reset_clip)(ctx)`<br> |
| define  | [**syn\_gfx\_set\_clip**](syn__gfx_8h.md#define-syn_gfx_set_clip) (ctx, x, y, w, h) `[**syn\_canvas\_set\_clip**](syn__canvas_8c.md#function-syn_canvas_set_clip)((ctx), (x), (y), (w), (h))`<br> |
| define  | [**syn\_gfx\_text**](syn__gfx_8h.md#define-syn_gfx_text) (ctx, x, y, str, color) `[**syn\_canvas\_text**](syn__canvas_8c.md#function-syn_canvas_text)((ctx), (x), (y), (str), (color))`<br> |
| define  | [**syn\_gfx\_text\_height**](syn__gfx_8h.md#define-syn_gfx_text_height) (ctx) `[**syn\_canvas\_text\_height**](syn__canvas_8c.md#function-syn_canvas_text_height)(ctx)`<br> |
| define  | [**syn\_gfx\_text\_width**](syn__gfx_8h.md#define-syn_gfx_text_width) (ctx, str) `[**syn\_canvas\_text\_width**](syn__canvas_8c.md#function-syn_canvas_text_width)((ctx), (str))`<br> |
| define  | [**syn\_gfx\_vline**](syn__gfx_8h.md#define-syn_gfx_vline) (ctx, x, y, h, color) `[**syn\_canvas\_vline**](syn__canvas_8c.md#function-syn_canvas_vline)((ctx), (x), (y), (h), (color))`<br> |

## Detailed Description


Provides a hardware-independent drawing API that the IMGUI widget layer (and user code) can call without being tied to a specific rendering backend.


The active backend is selected at compile time via the `SYN_GFX_BACKEND` macro in `syn_config.h`. If not defined, the framebuffer canvas backend (`SYN_GFX_BACKEND_CANVAS`) is used by default.




**
**


|Macro value   |Backend    |
|-----|-----|
|`SYN_GFX_BACKEND_CANVAS`   |Framebuffer canvas (syn\_canvas)    |
|`SYN_GFX_BACKEND_DIRECT`   |Direct-draw (no framebuffer)   |






All `syn_gfx_*` symbols resolve to direct function calls (or macros) at compile time — zero indirection overhead at runtime.




**
**


```C++
// Widget / application code includes only this header:
#include "display/syn_gfx.h"

syn_gfx_clear(gfx);
syn_gfx_rect_fill(gfx, 0, 0, 40, 12, SYN_COLOR_WHITE);
syn_gfx_text(gfx, 4, 2, "Hello", SYN_COLOR_BLACK);
syn_gfx_flush(gfx);
```
 





    
## Public Types Documentation




### typedef SYN\_GfxContext 

```C++
typedef SYN_Canvas* SYN_GfxContext;
```



Opaque graphics context — resolves to a canvas pointer. 


        

<hr>
## Macro Definition Documentation





### define SYN\_GFX\_BACKEND 

_Selected graphics backend (SYN\_GFX\_BACKEND\_CANVAS or SYN\_GFX\_BACKEND\_DIRECT)._ 
```C++
#define SYN_GFX_BACKEND `SYN_GFX_BACKEND_CANVAS`
```




<hr>



### define SYN\_GFX\_BACKEND\_CANVAS 

```C++
#define SYN_GFX_BACKEND_CANVAS `0`
```



Framebuffer canvas (default) 
 


        

<hr>



### define SYN\_GFX\_BACKEND\_DIRECT 

```C++
#define SYN_GFX_BACKEND_DIRECT `1`
```



Direct-draw (no framebuffer) 
 


        

<hr>



### define syn\_gfx\_bitmap 

```C++
#define syn_gfx_bitmap (
    ctx,
    x,
    y,
    bmp,
    w,
    h,
    color
) `syn_canvas_bitmap ((ctx), (x), (y), (bmp), (w), (h), (color))`
```



Draw 1-bit bitmap. 


        

<hr>



### define syn\_gfx\_char 

```C++
#define syn_gfx_char (
    ctx,
    x,
    y,
    ch,
    color
) `syn_canvas_char ((ctx), (x), (y), ( ch ), (color))`
```



Draw single char. 


        

<hr>



### define syn\_gfx\_circle 

```C++
#define syn_gfx_circle (
    ctx,
    cx,
    cy,
    r,
    color
) `syn_canvas_circle ((ctx), (cx), (cy), (r), (color))`
```



Draw circle outline. 


        

<hr>



### define syn\_gfx\_circle\_fill 

```C++
#define syn_gfx_circle_fill (
    ctx,
    cx,
    cy,
    r,
    color
) `syn_canvas_circle_fill ((ctx), (cx), (cy), (r), (color))`
```



Draw filled circle. 


        

<hr>



### define syn\_gfx\_clear 

```C++
#define syn_gfx_clear (
    ctx
) `syn_canvas_clear (ctx)`
```



Clear framebuffer. 


        

<hr>



### define syn\_gfx\_fill 

```C++
#define syn_gfx_fill (
    ctx,
    color
) `syn_canvas_fill ((ctx), (color))`
```



Fill with solid color. 


        

<hr>



### define syn\_gfx\_flush 

_Flush the entire framebuffer to the display._ 
```C++
#define syn_gfx_flush (
    ctx
) `syn_canvas_flush (ctx)`
```




<hr>



### define syn\_gfx\_flush\_partial 

_Flush a partial region of the framebuffer._ 
```C++
#define syn_gfx_flush_partial (
    ctx,
    offset,
    len
) `syn_canvas_flush_partial ((ctx), (offset), (len))`
```




<hr>



### define syn\_gfx\_font\_width 

```C++
#define syn_gfx_font_width (
    ctx
) `((ctx)->font ? (ctx)->font->width : 5)`
```



Return the glyph width of the active font (pixels). 


        

<hr>



### define syn\_gfx\_hline 

```C++
#define syn_gfx_hline (
    ctx,
    x,
    y,
    w,
    color
) `syn_canvas_hline ((ctx), (x), (y), (w), (color))`
```



Horizontal line. 


        

<hr>



### define syn\_gfx\_line 

```C++
#define syn_gfx_line (
    ctx,
    x0,
    y0,
    x1,
    y1,
    color
) `syn_canvas_line ((ctx), (x0), (y0), (x1), (y1), (color))`
```



Draw line. 


        

<hr>



### define syn\_gfx\_pixel 

```C++
#define syn_gfx_pixel (
    ctx,
    x,
    y,
    color
) `syn_canvas_pixel ((ctx), (x), (y), (color))`
```



Draw pixel. 


        

<hr>



### define syn\_gfx\_rect 

```C++
#define syn_gfx_rect (
    ctx,
    x,
    y,
    w,
    h,
    color
) `syn_canvas_rect ((ctx), (x), (y), (w), (h), (color))`
```



Draw rect outline. 


        

<hr>



### define syn\_gfx\_rect\_fill 

```C++
#define syn_gfx_rect_fill (
    ctx,
    x,
    y,
    w,
    h,
    color
) `syn_canvas_rect_fill ((ctx), (x), (y), (w), (h), (color))`
```



Draw filled rect. 


        

<hr>



### define syn\_gfx\_rect\_round 

```C++
#define syn_gfx_rect_round (
    ctx,
    x,
    y,
    w,
    h,
    r,
    color
) `syn_canvas_rect_round ((ctx), (x), (y), (w), (h), (r), (color))`
```



Draw rounded rect. 


        

<hr>



### define syn\_gfx\_rect\_round\_fill 

```C++
#define syn_gfx_rect_round_fill (
    ctx,
    x,
    y,
    w,
    h,
    r,
    color
) `syn_canvas_rect_round_fill ((ctx), (x), (y), (w), (h), (r), (color))`
```



Draw filled rounded rect. 


        

<hr>



### define syn\_gfx\_reset\_clip 

```C++
#define syn_gfx_reset_clip (
    ctx
) `syn_canvas_reset_clip (ctx)`
```



Reset clip to full canvas. 


        

<hr>



### define syn\_gfx\_set\_clip 

```C++
#define syn_gfx_set_clip (
    ctx,
    x,
    y,
    w,
    h
) `syn_canvas_set_clip ((ctx), (x), (y), (w), (h))`
```



Set clip rectangle. 


        

<hr>



### define syn\_gfx\_text 

```C++
#define syn_gfx_text (
    ctx,
    x,
    y,
    str,
    color
) `syn_canvas_text ((ctx), (x), (y), (str), (color))`
```



Draw text string. 


        

<hr>



### define syn\_gfx\_text\_height 

```C++
#define syn_gfx_text_height (
    ctx
) `syn_canvas_text_height (ctx)`
```



Get text line height. 


        

<hr>



### define syn\_gfx\_text\_width 

```C++
#define syn_gfx_text_width (
    ctx,
    str
) `syn_canvas_text_width ((ctx), (str))`
```



Measure text width. 


        

<hr>



### define syn\_gfx\_vline 

```C++
#define syn_gfx_vline (
    ctx,
    x,
    y,
    h,
    color
) `syn_canvas_vline ((ctx), (x), (y), (h), (color))`
```



Vertical line. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_gfx.h`

