

# File syn\_canvas.c



[**FileList**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_canvas.c**](syn__canvas_8c.md)

[Go to the source code of this file](syn__canvas_8c_source.md)

_Display canvas implementation._ 

* `#include "syn_canvas.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_Font**](structSYN__Font.md) | [**syn\_font\_5x7**](#variable-syn_font_5x7)   = `/* multi line expression */`<br> |


## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**font\_5x7\_data**](#variable-font_5x7_data)  <br>_Built-in 5×7 font data (ASCII 32–126, column-major)._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_canvas\_bitmap**](#function-syn_canvas_bitmap) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, const uint8\_t \* bitmap, int16\_t w, int16\_t h, uint16\_t color) <br>_Draw a monochrome 1bpp bitmap._  |
|  uint8\_t | [**syn\_canvas\_char**](#function-syn_canvas_char) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, char ch, uint16\_t color) <br>_Draw a single character._  |
|  void | [**syn\_canvas\_circle**](#function-syn_canvas_circle) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t cx, int16\_t cy, int16\_t r, uint16\_t color) <br>_Draw a circle (Bresenham)._  |
|  void | [**syn\_canvas\_circle\_fill**](#function-syn_canvas_circle_fill) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t cx, int16\_t cy, int16\_t r, uint16\_t color) <br>_Draw a filled circle._  |
|  void | [**syn\_canvas\_clear**](#function-syn_canvas_clear) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c) <br>_Clear the entire framebuffer (fill with zero)._  |
|  void | [**syn\_canvas\_fill**](#function-syn_canvas_fill) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, uint16\_t color) <br>_Fill the entire framebuffer with a color._  |
|  void | [**syn\_canvas\_flush**](#function-syn_canvas_flush) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c) <br>_Push framebuffer to display via the flush callback._  |
|  void | [**syn\_canvas\_flush\_partial**](#function-syn_canvas_flush_partial) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, size\_t offset, size\_t len) <br>_Push a slice of the framebuffer to the display._  |
|  void | [**syn\_canvas\_hline**](#function-syn_canvas_hline) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, uint16\_t color) <br>_Draw a fast horizontal line (optimised; no Bresenham overhead)._  |
|  void | [**syn\_canvas\_init**](#function-syn_canvas_init) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, uint8\_t \* buf, uint16\_t w, uint16\_t h, uint8\_t bpp, [**SYN\_Canvas\_FlushFn**](syn__canvas_8h.md#typedef-syn_canvas_flushfn) flush, void \* ctx) <br>_Initialize the canvas._  |
|  void | [**syn\_canvas\_line**](#function-syn_canvas_line) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x0, int16\_t y0, int16\_t x1, int16\_t y1, uint16\_t color) <br>_Draw a line (Bresenham)._  |
|  void | [**syn\_canvas\_pixel**](#function-syn_canvas_pixel) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, uint16\_t color) <br>_Set a single pixel._  |
|  void | [**syn\_canvas\_rect**](#function-syn_canvas_rect) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h, uint16\_t color) <br>_Draw a rectangle (outline only)._  |
|  void | [**syn\_canvas\_rect\_fill**](#function-syn_canvas_rect_fill) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h, uint16\_t color) <br>_Draw a filled rectangle._  |
|  void | [**syn\_canvas\_rect\_round**](#function-syn_canvas_rect_round) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h, int16\_t r, uint16\_t color) <br>_Draw a rounded rectangle (outline only)._  |
|  void | [**syn\_canvas\_rect\_round\_fill**](#function-syn_canvas_rect_round_fill) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h, int16\_t r, uint16\_t color) <br>_Draw a filled rounded rectangle._  |
|  void | [**syn\_canvas\_reset\_clip**](#function-syn_canvas_reset_clip) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c) <br>_Reset clip rectangle to the full display area._  |
|  void | [**syn\_canvas\_set\_clip**](#function-syn_canvas_set_clip) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Set the clip rectangle. Drawing is restricted to this region._  |
|  void | [**syn\_canvas\_set\_font**](#function-syn_canvas_set_font) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, const [**SYN\_Font**](structSYN__Font.md) \* font) <br>_Set the active font._  |
|  void | [**syn\_canvas\_text**](#function-syn_canvas_text) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, const char \* str, uint16\_t color) <br>_Draw a text string at (x,y) using the active font._  |
|  uint8\_t | [**syn\_canvas\_text\_height**](#function-syn_canvas_text_height) (const [**SYN\_Canvas**](structSYN__Canvas.md) \* c) <br>_Return the height of the active font in pixels._  |
|  uint16\_t | [**syn\_canvas\_text\_width**](#function-syn_canvas_text_width) (const [**SYN\_Canvas**](structSYN__Canvas.md) \* c, const char \* str) <br>_Measure text width in pixels (without drawing)._  |
|  void | [**syn\_canvas\_vline**](#function-syn_canvas_vline) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t h, uint16\_t color) <br>_Draw a fast vertical line._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**draw\_rounded\_corners**](#function-draw_rounded_corners) ([**SYN\_Canvas**](structSYN__Canvas.md) \* c, int16\_t x, int16\_t y, int16\_t w, int16\_t h, int16\_t r, uint16\_t color) <br>_Draw the four rounded corners of a rectangle._  |


























## Public Attributes Documentation




### variable syn\_font\_5x7 

```C++
const SYN_Font syn_font_5x7;
```



Built-in 5×7 font (ASCII 32-126). 


        

<hr>
## Public Static Attributes Documentation




### variable font\_5x7\_data 

_Built-in 5×7 font data (ASCII 32–126, column-major)._ 
```C++
const uint8_t font_5x7_data[];
```




<hr>
## Public Functions Documentation




### function syn\_canvas\_bitmap 

_Draw a monochrome 1bpp bitmap._ 
```C++
void syn_canvas_bitmap (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    const uint8_t * bitmap,
    int16_t w,
    int16_t h,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Destination X. 
* `y` Destination Y. 
* `bitmap` 1bpp bitmap data (row-major). 
* `w` Bitmap width in pixels. 
* `h` Bitmap height in pixels. 
* `color` Foreground color for set bits. 




        

<hr>



### function syn\_canvas\_char 

_Draw a single character._ 
```C++
uint8_t syn_canvas_char (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    char ch,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` X position. 
* `y` Y position. 
* `ch` Character to draw. 
* `color` Text color. 



**Returns:**

Advance width in pixels. 





        

<hr>



### function syn\_canvas\_circle 

_Draw a circle (Bresenham)._ 
```C++
void syn_canvas_circle (
    SYN_Canvas * c,
    int16_t cx,
    int16_t cy,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `cx` Center X. 
* `cy` Center Y. 
* `r` Radius. 
* `color` Outline color. 




        

<hr>



### function syn\_canvas\_circle\_fill 

_Draw a filled circle._ 
```C++
void syn_canvas_circle_fill (
    SYN_Canvas * c,
    int16_t cx,
    int16_t cy,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `cx` Center X. 
* `cy` Center Y. 
* `r` Radius. 
* `color` Fill color. 




        

<hr>



### function syn\_canvas\_clear 

_Clear the entire framebuffer (fill with zero)._ 
```C++
void syn_canvas_clear (
    SYN_Canvas * c
) 
```





**Parameters:**


* `c` Canvas. 




        

<hr>



### function syn\_canvas\_fill 

_Fill the entire framebuffer with a color._ 
```C++
void syn_canvas_fill (
    SYN_Canvas * c,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `color` Fill color (0/1 for mono, RGB565 for color). 




        

<hr>



### function syn\_canvas\_flush 

_Push framebuffer to display via the flush callback._ 
```C++
void syn_canvas_flush (
    SYN_Canvas * c
) 
```





**Parameters:**


* `c` Canvas. 




        

<hr>



### function syn\_canvas\_flush\_partial 

_Push a slice of the framebuffer to the display._ 
```C++
void syn_canvas_flush_partial (
    SYN_Canvas * c,
    size_t offset,
    size_t len
) 
```



Calls the flush callback with a pointer into the framebuffer at `offset` with length `len`. Intended for coroutine-friendly incremental flushing — yield between chunks to avoid blocking the scheduler during slow SPI/I2C transfers.




**
**


```C++
static size_t flush_pos;
flush_pos = 0;
while (flush_pos < canvas.buf_size) {
    size_t chunk = 128u;
    if (chunk > canvas.buf_size - flush_pos)
        chunk = canvas.buf_size - flush_pos;
    syn_canvas_flush_partial(&canvas, flush_pos, chunk);
    flush_pos += chunk;
    PT_YIELD(pt);   // other tasks run here
}
```





**Parameters:**


* `c` Canvas. 
* `offset` Byte offset into the framebuffer to start from. 
* `len` Number of bytes to send.



**Note:**

The display driver's flush\_fn receives ``(framebuf+offset, len, ctx) — the same three-argument signature as a full flush. It must be written to handle sequential partial writes correctly (i.e. not re-issue a start-of-frame command before each chunk). 





        

<hr>



### function syn\_canvas\_hline 

_Draw a fast horizontal line (optimised; no Bresenham overhead)._ 
```C++
void syn_canvas_hline (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Start X. 
* `y` Y position. 
* `w` Width in pixels. 
* `color` Color. 




        

<hr>



### function syn\_canvas\_init 

_Initialize the canvas._ 
```C++
void syn_canvas_init (
    SYN_Canvas * c,
    uint8_t * buf,
    uint16_t w,
    uint16_t h,
    uint8_t bpp,
    SYN_Canvas_FlushFn flush,
    void * ctx
) 
```





**Parameters:**


* `c` Canvas instance. 
* `buf` Framebuffer (caller allocates: w\*h/8 for 1bpp, w\*h\*2 for 16bpp). 
* `w` Width in pixels. 
* `h` Height in pixels. 
* `bpp` Bits per pixel (1 for mono, 16 for RGB565). 
* `flush` Callback to push framebuf to display hardware. 
* `ctx` Context for flush callback. 




        

<hr>



### function syn\_canvas\_line 

_Draw a line (Bresenham)._ 
```C++
void syn_canvas_line (
    SYN_Canvas * c,
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x0` Start X. 
* `y0` Start Y. 
* `x1` End X. 
* `y1` End Y. 
* `color` Line color. 




        

<hr>



### function syn\_canvas\_pixel 

_Set a single pixel._ 
```C++
void syn_canvas_pixel (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `color` Pixel color (0/1 for mono, RGB565 for color). 




        

<hr>



### function syn\_canvas\_rect 

_Draw a rectangle (outline only)._ 
```C++
void syn_canvas_rect (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Left edge. 
* `y` Top edge. 
* `w` Width. 
* `h` Height. 
* `color` Outline color. 




        

<hr>



### function syn\_canvas\_rect\_fill 

_Draw a filled rectangle._ 
```C++
void syn_canvas_rect_fill (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Left edge. 
* `y` Top edge. 
* `w` Width. 
* `h` Height. 
* `color` Fill color. 




        

<hr>



### function syn\_canvas\_rect\_round 

_Draw a rounded rectangle (outline only)._ 
```C++
void syn_canvas_rect_round (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Left edge. 
* `y` Top edge. 
* `w` Width. 
* `h` Height. 
* `r` Corner radius. 
* `color` Outline color. 




        

<hr>



### function syn\_canvas\_rect\_round\_fill 

_Draw a filled rounded rectangle._ 
```C++
void syn_canvas_rect_round_fill (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Left edge. 
* `y` Top edge. 
* `w` Width. 
* `h` Height. 
* `r` Corner radius. 
* `color` Fill color. 




        

<hr>



### function syn\_canvas\_reset\_clip 

_Reset clip rectangle to the full display area._ 
```C++
void syn_canvas_reset_clip (
    SYN_Canvas * c
) 
```





**Parameters:**


* `c` Canvas. 




        

<hr>



### function syn\_canvas\_set\_clip 

_Set the clip rectangle. Drawing is restricted to this region._ 
```C++
void syn_canvas_set_clip (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Clip left edge. 
* `y` Clip top edge. 
* `w` Clip width. 
* `h` Clip height. 




        

<hr>



### function syn\_canvas\_set\_font 

_Set the active font._ 
```C++
void syn_canvas_set_font (
    SYN_Canvas * c,
    const SYN_Font * font
) 
```





**Parameters:**


* `c` Canvas. 
* `font` Font descriptor, or NULL for built-in 5×7. 




        

<hr>



### function syn\_canvas\_text 

_Draw a text string at (x,y) using the active font._ 
```C++
void syn_canvas_text (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    const char * str,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Start X. 
* `y` Start Y. 
* `str` Null-terminated string. 
* `color` Text color. 




        

<hr>



### function syn\_canvas\_text\_height 

_Return the height of the active font in pixels._ 
```C++
uint8_t syn_canvas_text_height (
    const SYN_Canvas * c
) 
```





**Parameters:**


* `c` Canvas. 



**Returns:**

Font height in pixels. 





        

<hr>



### function syn\_canvas\_text\_width 

_Measure text width in pixels (without drawing)._ 
```C++
uint16_t syn_canvas_text_width (
    const SYN_Canvas * c,
    const char * str
) 
```





**Parameters:**


* `c` Canvas (uses active font). 
* `str` Null-terminated string. 



**Returns:**

Width in pixels. 





        

<hr>



### function syn\_canvas\_vline 

_Draw a fast vertical line._ 
```C++
void syn_canvas_vline (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t h,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` X position. 
* `y` Start Y. 
* `h` Height in pixels. 
* `color` Color. 




        

<hr>
## Public Static Functions Documentation




### function draw\_rounded\_corners 

_Draw the four rounded corners of a rectangle._ 
```C++
static void draw_rounded_corners (
    SYN_Canvas * c,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Canvas. 
* `x` Rectangle X. 
* `y` Rectangle Y. 
* `w` Rectangle width. 
* `h` Rectangle height. 
* `r` Corner radius. 
* `color` Pixel color. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_canvas.c`

