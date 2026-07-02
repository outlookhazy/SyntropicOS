

# File syn\_canvas.h

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_canvas.h**](syn__canvas_8h.md)

[Go to the documentation of this file](syn__canvas_8h.md)


```C++

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

typedef struct {
    const uint8_t *data;        
    uint8_t        width;       
    uint8_t        height;      
    uint8_t        first_char;  
    uint8_t        char_count;  
} SYN_Font;

extern const SYN_Font syn_font_5x7;

/* ── Flush callback ─────────────────────────────────────────────────────── */

typedef void (*SYN_Canvas_FlushFn)(const uint8_t *buf, size_t len, void *ctx);

/* ── Canvas instance ────────────────────────────────────────────────────── */

typedef struct {
    uint8_t             *framebuf;     
    size_t               buf_size;     
    uint16_t             width;        
    uint16_t             height;       
    uint8_t              bpp;          
    const SYN_Font     *font;         
    SYN_Canvas_FlushFn  flush_fn;     
    void                *flush_ctx;    
    int16_t              clip_x;       
    int16_t              clip_y;       
    int16_t              clip_w;       
    int16_t              clip_h;       
} SYN_Canvas;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_canvas_init(SYN_Canvas *c, uint8_t *buf,
                       uint16_t w, uint16_t h, uint8_t bpp,
                       SYN_Canvas_FlushFn flush, void *ctx);

void syn_canvas_set_clip(SYN_Canvas *c, int16_t x, int16_t y, int16_t w, int16_t h);

void syn_canvas_reset_clip(SYN_Canvas *c);

void syn_canvas_set_font(SYN_Canvas *c, const SYN_Font *font);

void syn_canvas_clear(SYN_Canvas *c);

void syn_canvas_fill(SYN_Canvas *c, uint16_t color);

void syn_canvas_pixel(SYN_Canvas *c, int16_t x, int16_t y, uint16_t color);

void syn_canvas_line(SYN_Canvas *c,
                       int16_t x0, int16_t y0,
                       int16_t x1, int16_t y1,
                       uint16_t color);

void syn_canvas_rect(SYN_Canvas *c,
                       int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color);

void syn_canvas_rect_fill(SYN_Canvas *c,
                             int16_t x, int16_t y,
                             int16_t w, int16_t h,
                             uint16_t color);

void syn_canvas_circle(SYN_Canvas *c,
                          int16_t cx, int16_t cy,
                          int16_t r, uint16_t color);

void syn_canvas_circle_fill(SYN_Canvas *c,
                               int16_t cx, int16_t cy,
                               int16_t r, uint16_t color);

void syn_canvas_rect_round(SYN_Canvas *c,
                              int16_t x, int16_t y,
                              int16_t w, int16_t h,
                              int16_t r, uint16_t color);

void syn_canvas_rect_round_fill(SYN_Canvas *c,
                                   int16_t x, int16_t y,
                                   int16_t w, int16_t h,
                                   int16_t r, uint16_t color);

void syn_canvas_bitmap(SYN_Canvas *c,
                          int16_t x, int16_t y,
                          const uint8_t *bitmap,
                          int16_t w, int16_t h,
                          uint16_t color);

void syn_canvas_text(SYN_Canvas *c, int16_t x, int16_t y,
                       const char *str, uint16_t color);

uint8_t syn_canvas_char(SYN_Canvas *c, int16_t x, int16_t y,
                           char ch, uint16_t color);

uint16_t syn_canvas_text_width(const SYN_Canvas *c, const char *str);

uint8_t syn_canvas_text_height(const SYN_Canvas *c);

void syn_canvas_hline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t w, uint16_t color);

void syn_canvas_vline(SYN_Canvas *c,
                        int16_t x, int16_t y,
                        int16_t h, uint16_t color);

void syn_canvas_flush(SYN_Canvas *c);

void syn_canvas_flush_partial(SYN_Canvas *c, size_t offset, size_t len);

/* ── Color helpers (RGB565) ─────────────────────────────────────────────── */

static inline uint16_t syn_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

#define SYN_COLOR_BLACK   0x0000  
#define SYN_COLOR_WHITE   0xFFFF  
#define SYN_COLOR_RED     0xF800  
#define SYN_COLOR_GREEN   0x07E0  
#define SYN_COLOR_BLUE    0x001F  
#ifdef __cplusplus
}
#endif

#endif /* SYN_CANVAS_H */
```


