

# File syn\_imgui.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md) **>** [**syn\_imgui.h**](syn__imgui_8h.md)

[Go to the documentation of this file](syn__imgui_8h.md)


```C++

#ifndef SYN_IMGUI_H
#define SYN_IMGUI_H

#include "../common/syn_defs.h"
#include "../display/syn_gfx.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Style / theme ──────────────────────────────────────────────────────── */

typedef struct {
    uint16_t fg;        
    uint16_t bg;        
    uint16_t highlight; 
    uint16_t fg_inv;    
    uint16_t accent;    
    uint16_t disabled;  
    int16_t  padding;   
    int16_t  spacing;   
} SYN_IMGUI_Style;

/* ── Layout cursor ───────────────────────────────────────────────────────── */

#define SYN_IMGUI_MAX_ROW_COLS 8

typedef struct {
    bool    in_layout; 
    int16_t cx;        
    int16_t cy;        
    int16_t origin_x;  
    int16_t width;     
    int16_t row_h;     
    bool    same_line; 
    int16_t row_x;         
    int16_t row_y;         
    int16_t prev_row_h;    
    /* Multi-column layout row state */
    int     row_items;     
    int     row_item_idx;  
    int16_t row_height;    
    int16_t row_widths[SYN_IMGUI_MAX_ROW_COLS]; 
} SYN_IMGUI_Layout;

/* ── Scroll region state ────────────────────────────────────────────────── */

typedef struct {
    bool    in_scroll;     
    int16_t vp_x;          
    int16_t vp_y;          
    int16_t vp_w;          
    int16_t vp_h;          
    int16_t content_start; 
    int16_t *scroll_p;     
    uint16_t focus_y;      
    uint16_t focus_h;      
} SYN_IMGUI_Scroll;

typedef struct {
    SYN_GfxContext gfx;  
    /* Physical button/encoder inputs for the current frame */
    bool btn_select;   
    bool btn_back;     
    int32_t enc_delta; 
    /* Touch screen inputs for the current frame */
    bool touch_down;   
    int16_t touch_x;   
    int16_t touch_y;   
    /* Internal Navigation State */
    uint16_t next_id;       
    uint16_t focused_id;    
    uint16_t active_id;     
    uint16_t last_max_id;   
    /* Style and layout */
    SYN_IMGUI_Style  style;  
    SYN_IMGUI_Layout layout; 
    SYN_IMGUI_Scroll scroll; 
    uint8_t          disabled_depth; 
    bool             updated_focus;  
} SYN_IMGUI_Context;

void syn_imgui_init(SYN_IMGUI_Context *ctx);

void syn_imgui_set_style(SYN_IMGUI_Context *ctx, const SYN_IMGUI_Style *style);

SYN_IMGUI_Style syn_imgui_default_style(void);

void syn_imgui_begin(SYN_IMGUI_Context *ctx, SYN_GfxContext gfx,
                      bool select, bool back, int32_t enc_delta,
                      bool touch_down, int16_t touch_x, int16_t touch_y);

void syn_imgui_end(SYN_IMGUI_Context *ctx);

/* ── Auto-layout ─────────────────────────────────────────────────────────── */

void syn_imgui_layout_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w);

void syn_imgui_layout_end(SYN_IMGUI_Context *ctx);

void syn_imgui_same_line(SYN_IMGUI_Context *ctx);

void syn_imgui_spacing(SYN_IMGUI_Context *ctx, int16_t pixels);

/* ── Simple display widgets ─────────────────────────────────────────────── */

void syn_imgui_label(SYN_IMGUI_Context *ctx, const char *text,
                      int16_t x, int16_t y);

void syn_imgui_separator(SYN_IMGUI_Context *ctx,
                          int16_t x, int16_t y, int16_t w);

bool syn_imgui_spinner(SYN_IMGUI_Context *ctx, const char *label,
                        int32_t *value, int32_t min, int32_t max, int32_t step,
                        int16_t x, int16_t y, int16_t w, int16_t h);

bool syn_imgui_button(SYN_IMGUI_Context *ctx, const char *label,
                       int16_t x, int16_t y, int16_t w, int16_t h);

bool syn_imgui_checkbox(SYN_IMGUI_Context *ctx, const char *label,
                         bool *checked, int16_t x, int16_t y, int16_t w, int16_t h);

bool syn_imgui_slider(SYN_IMGUI_Context *ctx, const char *label,
                       int32_t *value, int32_t min, int32_t max,
                       int16_t x, int16_t y, int16_t w, int16_t h);

bool syn_imgui_radio(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t *selection, int32_t button_val,
                      int16_t x, int16_t y, int16_t w, int16_t h);

void syn_imgui_progress_bar(SYN_IMGUI_Context *ctx, int32_t value, int32_t min, int32_t max,
                             int16_t x, int16_t y, int16_t w, int16_t h);

bool syn_imgui_combo(SYN_IMGUI_Context *ctx, const char *label,
                      const char **options, size_t count, int32_t *selected,
                      int16_t x, int16_t y, int16_t w, int16_t h);

void syn_imgui_graph(SYN_IMGUI_Context *ctx, const char *title,
                      const int32_t *data, size_t count,
                      int32_t min_val, int32_t max_val,
                      int16_t x, int16_t y, int16_t w, int16_t h);

void syn_imgui_gauge(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t value, int32_t min, int32_t max,
                      int16_t cx, int16_t cy, int16_t radius);

bool syn_imgui_dialog(SYN_IMGUI_Context *ctx, const char *message,
                       bool *ok_clicked, int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Scroll region ──────────────────────────────────────────────────────── */

void syn_imgui_scroll_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t *scroll);

void syn_imgui_scroll_end(SYN_IMGUI_Context *ctx);

/* ── Toggle switch ──────────────────────────────────────────────────────── */

bool syn_imgui_toggle(SYN_IMGUI_Context *ctx, const char *label,
                       bool *state, int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Disabled state ─────────────────────────────────────────────────────── */

void syn_imgui_begin_disabled(SYN_IMGUI_Context *ctx);

void syn_imgui_end_disabled(SYN_IMGUI_Context *ctx);

/* ── Text alignment helpers ─────────────────────────────────────────────── */

void syn_imgui_label_colored(SYN_IMGUI_Context *ctx, const char *text,
                              uint16_t color, int16_t x, int16_t y);

void syn_imgui_label_right(SYN_IMGUI_Context *ctx, const char *text,
                            int16_t x, int16_t y, int16_t w);

void syn_imgui_label_centered(SYN_IMGUI_Context *ctx, const char *text,
                               int16_t x, int16_t y, int16_t w);

/* ── Group box ──────────────────────────────────────────────────────────── */

void syn_imgui_group_begin(SYN_IMGUI_Context *ctx, const char *title,
                            int16_t x, int16_t y, int16_t w, int16_t h);

void syn_imgui_group_end(SYN_IMGUI_Context *ctx);

/* ── Tab bar ────────────────────────────────────────────────────────────── */

bool syn_imgui_tabs(SYN_IMGUI_Context *ctx, const char **labels,
                    size_t count, int32_t *active,
                    int16_t x, int16_t y, int16_t w);

/* ── Bar chart ──────────────────────────────────────────────────────────── */

void syn_imgui_bar_chart(SYN_IMGUI_Context *ctx, const char *title,
                          const int32_t *data, size_t count,
                          int32_t min_val, int32_t max_val,
                          int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Icon button ────────────────────────────────────────────────────────── */

bool syn_imgui_icon_button(SYN_IMGUI_Context *ctx,
                            const uint8_t *icon, int16_t icon_w, int16_t icon_h,
                            int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Status bar ─────────────────────────────────────────────────────────── */

void syn_imgui_status_bar(SYN_IMGUI_Context *ctx, const char *text,
                           int16_t x, int16_t y, int16_t w);

/* ── Separator text ─────────────────────────────────────────────────────── */

void syn_imgui_separator_text(SYN_IMGUI_Context *ctx, const char *text,
                                int16_t x, int16_t y, int16_t w);

/* ── Checkbox flags ─────────────────────────────────────────────────────── */

bool syn_imgui_checkbox_flags(SYN_IMGUI_Context *ctx, const char *label,
                                uint32_t *flags, uint32_t mask,
                                int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Value display ──────────────────────────────────────────────────────── */

void syn_imgui_value_int(SYN_IMGUI_Context *ctx, const char *label,
                           int32_t value, int16_t x, int16_t y);

/* ── Progress bar with overlay text ─────────────────────────────────────── */

void syn_imgui_progress_bar_ex(SYN_IMGUI_Context *ctx, int32_t value,
                                 int32_t min, int32_t max,
                                 const char *overlay,
                                 int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Selectable ─────────────────────────────────────────────────────────── */

bool syn_imgui_selectable(SYN_IMGUI_Context *ctx, const char *label,
                            bool *selected,
                            int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Collapsing header ──────────────────────────────────────────────────── */

bool syn_imgui_collapsing_header(SYN_IMGUI_Context *ctx, const char *label,
                                   bool *expanded,
                                   int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Text word-wrap ─────────────────────────────────────────────────────── */

void syn_imgui_text_wrapped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y, int16_t w);

/* ── Layout row (multi-column) ──────────────────────────────────────────── */

void syn_imgui_layout_row(SYN_IMGUI_Context *ctx, int items,
                            const int16_t *widths, int16_t height);

/* ── Visibility culling ─────────────────────────────────────────────────── */

bool syn_imgui_widget_visible(const SYN_IMGUI_Context *ctx,
                                int16_t y, int16_t h);

/* ── Text clipped ───────────────────────────────────────────────────────── */

void syn_imgui_text_clipped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y,
                              int16_t clip_x, int16_t clip_y,
                              int16_t clip_w, int16_t clip_h);

/* ── Text marquee ───────────────────────────────────────────────────────── */

void syn_imgui_text_marquee(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t *offset,
                              int16_t x, int16_t y, int16_t w,
                              int16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IMGUI_H */
```


