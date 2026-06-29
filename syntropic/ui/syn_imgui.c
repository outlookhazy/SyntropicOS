#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_IMGUI) || SYN_USE_IMGUI

/**
 * @file syn_imgui.c
 * @brief Immediate Mode GUI (IMGUI) implementation.
 */

#include "syn_imgui.h"
#include "../util/syn_assert.h"

#include <string.h>

/* Forward declarations for functions defined later in this file */
static void layout_resolve(SYN_IMGUI_Context *ctx,
                            int16_t *x, int16_t *y,
                            int16_t *w, int16_t *h,
                            int16_t default_h);

void syn_imgui_init(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));
    ctx->focused_id = 1;
    ctx->style      = syn_imgui_default_style();
}

void syn_imgui_begin(SYN_IMGUI_Context *ctx, SYN_GfxContext gfx,
                      bool select, bool back, int32_t enc_delta,
                      bool touch_down, int16_t touch_x, int16_t touch_y)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(gfx != NULL);

    ctx->gfx      = gfx;
    ctx->btn_select  = select;
    ctx->btn_back    = back;
    ctx->enc_delta   = enc_delta;
    ctx->touch_down  = touch_down;
    ctx->touch_x     = touch_x;
    ctx->touch_y     = touch_y;
    ctx->next_id     = 0;

    /* Focus navigation by encoder only if screen is NOT being touched and no slider is active */
    if (ctx->active_id == 0 && !touch_down) {
        if (enc_delta > 0) {
            for (int32_t i = 0; i < enc_delta; i++) {
                ctx->focused_id++;
                if (ctx->focused_id > ctx->last_max_id) {
                    ctx->focused_id = 1;
                }
            }
        } else if (enc_delta < 0) {
            for (int32_t i = enc_delta; i < 0; i++) {
                if (ctx->focused_id <= 1) {
                    ctx->focused_id = (ctx->last_max_id > 0) ? ctx->last_max_id : 1;
                } else {
                    ctx->focused_id--;
                }
            }
        }
    }
}

void syn_imgui_end(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);

    /* Bug #2: validate stack balance (catch forgotten end calls) */
    SYN_ASSERT(!ctx->layout.in_layout);  /* forgot layout_end? */
    SYN_ASSERT(!ctx->scroll.in_scroll);  /* forgot scroll_end? */
    SYN_ASSERT(ctx->disabled_depth == 0); /* forgot end_disabled? */

    ctx->last_max_id = ctx->next_id;

    /* Bug #3: clear focus if the focused widget was not visited this frame */
    if (!ctx->updated_focus && ctx->focused_id > 0) {
        ctx->focused_id = (ctx->last_max_id > 0) ? 1 : 0;
    }
    ctx->updated_focus = false;

    /* Safely cap focus if the number of widgets decreased */
    if (ctx->focused_id > ctx->last_max_id) {
        ctx->focused_id = (ctx->last_max_id > 0) ? 1 : 0;
    }
}

/**
 * @brief Test if touch coordinates fall within a rectangle.
 * @param ctx  IMGUI context.
 * @param x    Widget X.
 * @param y    Widget Y.
 * @param w    Widget width.
 * @param h    Widget height.
 * @return true if touch is inside.
 */
static bool is_hit_test(const SYN_IMGUI_Context *ctx, int16_t x, int16_t y, int16_t w, int16_t h)
{
    return (ctx->touch_down &&
            ctx->touch_x >= x && ctx->touch_x < x + w &&
            ctx->touch_y >= y && ctx->touch_y < y + h);
}

/**
 * @brief Track whether the focused widget was visited this frame.
 * @param ctx  IMGUI context.
 */
static inline void track_focus(SYN_IMGUI_Context *ctx)
{
    if (ctx->focused_id == ctx->next_id) {
        ctx->updated_focus = true;
    }
}

bool syn_imgui_button(SYN_IMGUI_Context *ctx, const char *label,
                       int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + 2 * ctx->style.padding);
    layout_resolve(ctx, &x, &y, &w, &h, default_h);

    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool touch_clicked = false;

    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);

    int16_t text_w = (int16_t)syn_gfx_text_width(ctx->gfx, label);
    int16_t tx = (int16_t)(x + (w - text_w) / 2);
    int16_t ty = (int16_t)(y + (h - (int16_t)fh) / 2);

    if (is_focused && ctx->disabled_depth == 0) {
        /* Draw filled background */
        syn_gfx_rect_round_fill(ctx->gfx, x, y, w, h, 4, fg);
        /* Draw text in background color */
        syn_gfx_text(ctx->gfx, tx, ty, label, ctx->style.bg);
    } else {
        /* Draw outline only */
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 4, fg);
        /* Draw text in foreground */
        syn_gfx_text(ctx->gfx, tx, ty, label, fg);
    }

    return ctx->disabled_depth == 0 && (touch_clicked || (is_focused && ctx->btn_select));
}

bool syn_imgui_checkbox(SYN_IMGUI_Context *ctx, const char *label,
                         bool *checked, int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(checked != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + 2 * ctx->style.padding);
    layout_resolve(ctx, &x, &y, &w, &h, default_h);

    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool touch_clicked = false;

    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool toggled = false;

    if (ctx->disabled_depth == 0 && (touch_clicked || (is_focused && ctx->btn_select))) {
        *checked = !(*checked);
        toggled = true;
    }

    /* Focus outline around row */
    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Render Box (10x10) */
    int16_t box_size = 10;
    int16_t bx = x + 4;
    int16_t by = (int16_t)(y + (h - box_size) / 2);
    syn_gfx_rect(ctx->gfx, bx, by, box_size, box_size, fg);

    if (*checked) {
        /* Checked inner box (6x6) */
        syn_gfx_rect_fill(ctx->gfx, bx + 2, by + 2, box_size - 4, box_size - 4, fg);
    }

    /* Label text */
    int16_t tx = x + 18;
    int16_t ty = (int16_t)(y + (h - (int16_t)fh) / 2);
    syn_gfx_text(ctx->gfx, tx, ty, label, fg);

    return toggled;
}

void syn_imgui_progress_bar(SYN_IMGUI_Context *ctx, int32_t value, int32_t min, int32_t max,
                             int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    syn_gfx_rect(ctx->gfx, x, y, w, h, fg);

    if (max > min) {
        int32_t val = value;
        if (val < min) val = min;
        if (val > max) val = max;

        int32_t range = max - min;
        int32_t current = val - min;
        int16_t fill_w = (int16_t)((current * (w - 4)) / range);
        if (fill_w > 0) {
            syn_gfx_rect_fill(ctx->gfx, x + 2, y + 2, fill_w, h - 4, fg);
        }
    }
}

bool syn_imgui_slider(SYN_IMGUI_Context *ctx, const char *label,
                       int32_t *value, int32_t min, int32_t max,
                       int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(value != NULL);

    ctx->next_id++;
    track_focus(ctx);

    /* Resolve geometry from layout cursor before using x/y/w/h */
    uint8_t fh_sl = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_sl = (int16_t)(fh_sl + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &w, &h, default_h_sl);

    bool touch_clicked = false;
    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool is_active  = (ctx->active_id == ctx->next_id);
    bool changed    = false;

    int16_t bar_w = (int16_t)(w / 2 - 8);
    int16_t bar_x = (int16_t)(x + w / 2 + 4);
    int16_t bar_h = 6;
    int16_t bar_y = (int16_t)(y + (h - bar_h) / 2);

    /* Handle direct touch settings */
    if (ctx->disabled_depth == 0 && touch_clicked) {
        ctx->active_id = ctx->next_id;
        is_active = true;

        /* Calculate absolute value position if touch fell inside slider bar bounds */
        if (ctx->touch_x >= bar_x && ctx->touch_x <= bar_x + bar_w && max > min) {
            int16_t relative_x = ctx->touch_x - bar_x;
            int32_t new_val = min + (relative_x * (max - min)) / bar_w;
            if (new_val < min) new_val = min;
            if (new_val > max) new_val = max;
            if (new_val != *value) {
                *value = new_val;
                changed = true;
            }
        }
    }

    if (is_active) {
        /* If we are actively editing, encoder delta adjusts values */
        if (ctx->disabled_depth == 0 && !touch_clicked && ctx->enc_delta != 0) {
            int32_t new_val = *value + ctx->enc_delta;
            if (new_val < min) new_val = min;
            if (new_val > max) new_val = max;
            if (new_val != *value) {
                *value = new_val;
                changed = true;
            }
        }
        /* Exit edit mode on select or back press (or if touch release happens outside row) */
        if ((ctx->btn_select || ctx->btn_back) && !touch_clicked) {
            ctx->active_id = 0;
        }
    } else if (is_focused) {
        if (ctx->btn_select) {
            ctx->active_id = ctx->next_id;
        }
    }

    /* Row highlight if focused and not actively editing */
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    if (is_focused && !is_active) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Draw label */
    int16_t tx = x + 4;
    int16_t ty = (int16_t)(y + (h - 7) / 2);
    syn_gfx_text(ctx->gfx, tx, ty, label, fg);

    /* Draw progress bar inside using helper */
    syn_imgui_progress_bar(ctx, *value, min, max, bar_x, bar_y, bar_w, bar_h);


    /* Edit mode indicator around the bar */
    if (is_active) {
        syn_gfx_rect(ctx->gfx, bar_x - 2, bar_y - 2, bar_w + 4, bar_h + 4, fg);
    }

    return changed;
}

bool syn_imgui_radio(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t *selection, int32_t button_val,
                      int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(selection != NULL);

    ctx->next_id++;
    track_focus(ctx);

    /* Resolve geometry from layout cursor */
    uint8_t fh_r = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_r = (int16_t)(fh_r + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &w, &h, default_h_r);

    bool touch_clicked = false;
    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool changed = false;

    if (ctx->disabled_depth == 0 && (touch_clicked || (is_focused && ctx->btn_select))) {
        if (*selection != button_val) {
            *selection = button_val;
            changed = true;
        }
    }

    /* Focus outline around row */
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Render Radio Circle (radius = 5, center x = x + 9, y = y + h/2) */
    int16_t cx = x + 9;
    int16_t cy = y + h / 2;
    syn_gfx_circle(ctx->gfx, cx, cy, 5, fg);

    if (*selection == button_val) {
        /* Selected center dot (radius = 2) */
        syn_gfx_circle_fill(ctx->gfx, cx, cy, 2, fg);
    }

    /* Label text */
    int16_t tx = x + 20;
    int16_t ty = (int16_t)(y + (h - 7) / 2);
    syn_gfx_text(ctx->gfx, tx, ty, label, fg);

    return changed;
}

bool syn_imgui_combo(SYN_IMGUI_Context *ctx, const char *label,
                      const char **options, size_t count, int32_t *selected,
                      int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(options != NULL);
    SYN_ASSERT(selected != NULL);
    if (count == 0) return false;

    ctx->next_id++;
    track_focus(ctx);

    /* Resolve geometry from layout cursor */
    uint8_t fh_c = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_c = (int16_t)(fh_c + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &w, &h, default_h_c);

    bool touch_clicked = is_hit_test(ctx, x, y, w, h);
    if (touch_clicked) {
        ctx->focused_id = ctx->next_id;
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool is_active  = (ctx->active_id == ctx->next_id);
    bool changed    = false;

    int16_t left_x  = (int16_t)(x + w / 2 + 2);
    int16_t right_x = (int16_t)(x + w - 14);

    /* Handle direct touch button hits on arrows */
    if (ctx->disabled_depth == 0 && touch_clicked) {
        bool hit_left = (ctx->touch_x >= left_x && ctx->touch_x < left_x + 12);
        bool hit_right = (ctx->touch_x >= right_x && ctx->touch_x < right_x + 12);
        if (hit_left) {
            int32_t idx = *selected - 1;
            if (idx < 0) idx = (int32_t)count - 1;
            *selected = idx;
            changed = true;
        } else if (hit_right) {
            int32_t idx = *selected + 1;
            if (idx >= (int32_t)count) idx = 0;
            *selected = idx;
            changed = true;
        } else {
            /* Touch in middle toggles active editing */
            ctx->active_id = is_active ? 0 : ctx->next_id;
            is_active = !is_active;
        }
    }

    if (is_active) {
        if (!touch_clicked && ctx->enc_delta != 0) {
            int32_t idx = *selected + ctx->enc_delta;
            /* Modulo wrapping for negative/positive index */
            while (idx < 0) idx += (int32_t)count;
            while (idx >= (int32_t)count) idx -= (int32_t)count;
            if (idx != *selected) {
                *selected = idx;
                changed = true;
            }
        }
        if ((ctx->btn_select || ctx->btn_back) && !touch_clicked) {
            ctx->active_id = 0;
        }
    } else if (is_focused) {
        if (ctx->btn_select) {
            ctx->active_id = ctx->next_id;
        }
    }

    /* Highlight focused (not active) row */
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    if (is_focused && !is_active) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Draw label */
    int16_t tx = x + 4;
    int16_t ty = (int16_t)(y + (h - 7) / 2);
    syn_gfx_text(ctx->gfx, tx, ty, label, fg);

    /* Draw "< option_text >" centered in the right half */
    int16_t val_w = w / 2;
    int16_t val_x = x + w / 2;
    const char *opt_str = options[*selected];
    int16_t opt_w = (int16_t)syn_gfx_text_width(ctx->gfx, opt_str);
    int16_t opt_x = (int16_t)(val_x + (val_w - opt_w) / 2);

    /* Draw left/right arrows */
    syn_gfx_text(ctx->gfx, left_x + 2, ty, "<", fg);
    syn_gfx_text(ctx->gfx, right_x + 2, ty, ">", fg);

    /* Draw the active option text */
    syn_gfx_text(ctx->gfx, opt_x, ty, opt_str, fg);

    /* Draw active edit frame around option */
    if (is_active) {
        syn_gfx_rect(ctx->gfx, left_x, (int16_t)(y + 1), (int16_t)(right_x - left_x + 12), (int16_t)(h - 2), fg);
    }

    return changed;
}

void syn_imgui_graph(SYN_IMGUI_Context *ctx, const char *title,
                      const int32_t *data, size_t count,
                      int32_t min_val, int32_t max_val,
                      int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);

    /* Resolve geometry from layout cursor; graph needs explicit w/h when used inline */
    layout_resolve(ctx, &x, &y, &w, &h, h > 0 ? h : (int16_t)32);

    if (w <= 0 || h <= 0) return;

    uint16_t fg = ctx->style.fg;

    /* Draw container border */
    syn_gfx_rect(ctx->gfx, x, y, w, h, fg);

    int16_t plot_x = x + 2;
    int16_t plot_w = w - 4;
    int16_t plot_y = y + 2;
    int16_t plot_h = h - 4;

    /* Header text if title is provided */
    if (title != NULL) {
        syn_gfx_text(ctx->gfx, x + 4, y + 4, title, fg);
        plot_y += 12;
        plot_h -= 12;
    }

    if (plot_h <= 0 || plot_w <= 0) return;

    /* Draw midpoint horizontal reference grid line */
    syn_gfx_line(ctx->gfx, plot_x, (int16_t)(plot_y + plot_h / 2),
                     (int16_t)(plot_x + plot_w - 1), (int16_t)(plot_y + plot_h / 2), fg);

    if (data == NULL || count == 0) return;

    int32_t range = max_val - min_val;
    if (range <= 0) range = 1;

    int16_t prev_x = 0;
    int16_t prev_y = 0;

    for (size_t i = 0; i < count; i++) {
        /* X position calculation */
        int16_t x_pos = plot_x;
        if (count > 1) {
            x_pos += (int16_t)((i * (size_t)(plot_w - 1)) / (count - 1));
        }

        /* Y position calculation */
        int32_t val = data[i];
        if (val < min_val) val = min_val;
        if (val > max_val) val = max_val;
        int16_t y_pos = (int16_t)(plot_y + plot_h - 1 - (int16_t)(((val - min_val) * (plot_h - 1)) / range));

        if (i > 0) {
            syn_gfx_line(ctx->gfx, prev_x, prev_y, x_pos, y_pos, fg);
        }

        prev_x = x_pos;
        prev_y = y_pos;
    }
}

/**
 * @brief Draw a semicircle (top half) using Bresenham's algorithm.
 * @param c      Graphics context.
 * @param cx     Center X.
 * @param cy     Center Y.
 * @param r      Radius.
 * @param color  Pixel color.
 */
static void draw_semi_circle(SYN_GfxContext c, int16_t cx, int16_t cy, int16_t r, uint16_t color)
{
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 1 - r;

    while (x >= y) {
        syn_gfx_pixel(c, cx - x, cy - y, color);
        syn_gfx_pixel(c, cx - y, cy - x, color);
        syn_gfx_pixel(c, cx + y, cy - x, color);
        syn_gfx_pixel(c, cx + x, cy - y, color);
        y++;
        if (err < 0) {
            err = (int16_t)(err + 2 * y + 1);
        } else {
            x--;
            err = (int16_t)(err + 2 * (y - x) + 1);
        }
    }
}

void syn_imgui_gauge(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t value, int32_t min, int32_t max,
                      int16_t cx, int16_t cy, int16_t radius)
{
    SYN_ASSERT(ctx != NULL);
    if (radius <= 0) return;

    /* Resolve geometry: gauge uses center+radius; synthesize a bounding box
     * for the layout cursor so it advances past the gauge's footprint. */
    int16_t gx = (int16_t)(cx - radius);
    int16_t gy = (int16_t)(cy - radius);
    int16_t gw = (int16_t)(radius * 2);
    int16_t gh = (int16_t)(radius + 10); /* semi-circle + label */
    layout_resolve(ctx, &gx, &gy, &gw, &gh, gh);

    uint16_t fg = ctx->style.fg;

    /* Draw arc gauge */
    draw_semi_circle(ctx->gfx, cx, cy, radius, fg);
    /* Draw base reference line */
    syn_gfx_line(ctx->gfx, cx - radius, cy, cx + radius, cy, fg);

    /* 9-point fixed-point sin/cos tables (0 to 180 degrees, scaled by 256) */
    static const int16_t sin_tbl[9] = { 0, 98, 181, 236, 256, 236, 181, 98, 0 };
    static const int16_t cos_tbl[9] = { 256, 236, 181, 98, 0, -98, -181, -236, -256 };

    /* Calculate value percentage (0 to 1000) */
    int32_t val = value;
    if (val < min) val = min;
    if (val > max) val = max;

    int32_t range = max - min;
    int32_t val_percent = (range > 0) ? ((val - min) * 1000) / range : 0;

    /* Map to table index and interpolation remainder */
    int32_t temp = (1000 - val_percent) * 8;
    int32_t idx  = temp / 1000;

    int32_t sin_val;
    int32_t cos_val;

    if (idx >= 8) {
        sin_val = sin_tbl[8];
        cos_val = cos_tbl[8];
    } else {
        int32_t rem = temp % 1000;
        sin_val = sin_tbl[idx] + (rem * (sin_tbl[idx + 1] - sin_tbl[idx])) / 1000;
        cos_val = cos_tbl[idx] + (rem * (cos_tbl[idx + 1] - cos_tbl[idx])) / 1000;
    }


    /* Draw needle */
    int16_t needle_len = radius - 4;
    int16_t nx = cx + (int16_t)((cos_val * needle_len) / 256);
    int16_t ny = cy - (int16_t)((sin_val * needle_len) / 256);

    syn_gfx_line(ctx->gfx, cx, cy, nx, ny, fg);

    /* Draw label centered under the gauge base */
    if (label != NULL) {
        int16_t text_w = (int16_t)syn_gfx_text_width(ctx->gfx, label);
        syn_gfx_text(ctx->gfx, cx - text_w / 2, cy + 2, label, fg);
    }
}

bool syn_imgui_dialog(SYN_IMGUI_Context *ctx, const char *message,
                       bool *ok_clicked, int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(message != NULL);
    SYN_ASSERT(ok_clicked != NULL);

    /* Resolve geometry from layout cursor */
    layout_resolve(ctx, &x, &y, &w, &h, h > 0 ? h : (int16_t)40);

    uint16_t ok_id     = ctx->next_id + 1;
    uint16_t cancel_id = ctx->next_id + 2;
    ctx->next_id += 2;

    /* Track focus for stale-focus detection */
    if (ctx->focused_id == ok_id || ctx->focused_id == cancel_id) {
        ctx->updated_focus = true;
    }

    /* Force focus inside dialog button bounds if it is currently outside */
    if (ctx->focused_id != ok_id && ctx->focused_id != cancel_id) {
        ctx->focused_id = ok_id;
    }

    /* Hijack navigation input while modal is open */
    if (ctx->enc_delta != 0) {
        ctx->focused_id = (ctx->focused_id == ok_id) ? cancel_id : ok_id;
        ctx->enc_delta = 0; /* Consume input */
    }

    /* Modal layout coordinates */
    int16_t btn_w  = 30;
    int16_t btn_h  = 13;
    int16_t ok_x   = (int16_t)(x + w / 4 - btn_w / 2);
    int16_t canc_x = (int16_t)(x + (3 * w) / 4 - btn_w / 2);
    int16_t btn_y  = (int16_t)(y + h - 18);

    /* Hit testing */
    bool hit_ok   = is_hit_test(ctx, ok_x, btn_y, btn_w, btn_h);
    bool hit_canc = is_hit_test(ctx, canc_x, btn_y, btn_w, btn_h);

    bool select_ok     = (ctx->focused_id == ok_id && ctx->btn_select);
    bool select_cancel = (ctx->focused_id == cancel_id && ctx->btn_select) || ctx->btn_back;

    bool dismissed = false;

    if (hit_ok || select_ok) {
        *ok_clicked = true;
        dismissed   = true;
        ctx->focused_id = 1; /* Reset focus */
    } else if (hit_canc || select_cancel) {
        *ok_clicked = false;
        dismissed   = true;
        ctx->focused_id = 1; /* Reset focus */
    }

    /* Consume touches outside the modal dialog to prevent background interaction */
    if (ctx->touch_down && !is_hit_test(ctx, x, y, w, h)) {
        ctx->touch_down = false;
    }

    /* Draw modal background outline box (clears underlying content) */
    uint16_t fg = ctx->style.fg;
    syn_gfx_rect_round_fill(ctx->gfx, x, y, w, h, 4, ctx->style.bg);
    syn_gfx_rect_round(ctx->gfx, x, y, w, h, 4, fg);

    /* Draw message centered at top */
    int16_t msg_w = (int16_t)syn_gfx_text_width(ctx->gfx, message);
    syn_gfx_text(ctx->gfx, (int16_t)(x + (w - msg_w) / 2), (int16_t)(y + 6), message, fg);

    /* Render OK button */
    if (ctx->focused_id == ok_id) {
        syn_gfx_rect_round_fill(ctx->gfx, ok_x, btn_y, btn_w, btn_h, 3, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(ok_x + (btn_w - 11) / 2), (int16_t)(btn_y + 3), "OK", ctx->style.bg);
    } else {
        syn_gfx_rect_round(ctx->gfx, ok_x, btn_y, btn_w, btn_h, 3, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(ok_x + (btn_w - 11) / 2), (int16_t)(btn_y + 3), "OK", fg);
    }

    /* Render CANCEL button */
    int16_t cn_w = (int16_t)syn_gfx_text_width(ctx->gfx, "CANC");
    if (ctx->focused_id == cancel_id) {
        syn_gfx_rect_round_fill(ctx->gfx, canc_x, btn_y, btn_w, btn_h, 3, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(canc_x + (btn_w - cn_w) / 2), (int16_t)(btn_y + 3), "CANC", ctx->style.bg);
    } else {
        syn_gfx_rect_round(ctx->gfx, canc_x, btn_y, btn_w, btn_h, 3, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(canc_x + (btn_w - cn_w) / 2), (int16_t)(btn_y + 3), "CANC", fg);
    }

    return dismissed;
}


/* ═══════════════════════════════════════════════════════════════════════════
 * Style
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_IMGUI_Style syn_imgui_default_style(void)
{
    SYN_IMGUI_Style s;
    s.fg       = SYN_COLOR_WHITE;
    s.bg       = SYN_COLOR_BLACK;
    s.highlight= SYN_COLOR_WHITE;
    s.fg_inv   = SYN_COLOR_BLACK;
    s.accent   = SYN_COLOR_WHITE;
    s.disabled = 0x7BEFu; /* mid-grey in RGB565 */
    s.padding  = 2;
    s.spacing  = 3;
    return s;
}

void syn_imgui_set_style(SYN_IMGUI_Context *ctx, const SYN_IMGUI_Style *style)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(style != NULL);
    ctx->style = *style;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Layout cursor
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_layout_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    ctx->layout.in_layout  = true;
    ctx->layout.cx         = x;
    ctx->layout.cy         = y;
    ctx->layout.origin_x   = x;
    ctx->layout.width      = w;
    ctx->layout.row_h      = 0;
    ctx->layout.same_line  = false;
    ctx->layout.row_x      = x;
    ctx->layout.row_y      = y;
    ctx->layout.prev_row_h = 0;
}

void syn_imgui_layout_end(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    ctx->layout.in_layout = false;
}

void syn_imgui_same_line(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    if (!ctx->layout.in_layout) return;
    ctx->layout.cx = ctx->layout.row_x;
    ctx->layout.cy = ctx->layout.row_y;
    ctx->layout.row_h = ctx->layout.prev_row_h;
    ctx->layout.same_line = true;
}

void syn_imgui_spacing(SYN_IMGUI_Context *ctx, int16_t pixels)
{
    SYN_ASSERT(ctx != NULL);
    if (ctx->layout.in_layout) {
        ctx->layout.cy = (int16_t)(ctx->layout.cy + pixels);
    }
}

/**
 * @brief Resolve widget geometry from the layout cursor.
 *
 * If the widget is inside a layout region AND all four geometry params
 * are 0, fill them from the cursor and default width/height.
 *
 * @param ctx        IMGUI context.
 * @param x          [in/out] Widget X.
 * @param y          [in/out] Widget Y.
 * @param w          [in/out] Widget width.
 * @param h          [in/out] Widget height.
 * @param default_h  Default height if not specified.
 */
static void layout_resolve(SYN_IMGUI_Context *ctx,
                            int16_t *x, int16_t *y,
                            int16_t *w, int16_t *h,
                            int16_t default_h)
{
    if (!ctx->layout.in_layout) return;

    if (ctx->layout.same_line) {
        /* consumed by syn_imgui_same_line cursor restoration */
        ctx->layout.same_line = false;
    } else {
        /* If the previous widget(s) left the row unwrapped (e.g. manually manipulated),
         * wrap it now before resolving this widget. */
        if (ctx->layout.row_h > 0) {
            ctx->layout.cy = (int16_t)(ctx->layout.row_y + ctx->layout.row_h + ctx->style.spacing);
            ctx->layout.cx = ctx->layout.origin_x;
            ctx->layout.row_h = 0;
        }
        ctx->layout.row_y = ctx->layout.cy;
    }

    if (*x == 0 && *y == 0 && *w == 0 && *h == 0) {
        /* Use cursor position */
        *x = ctx->layout.cx;
        *y = ctx->layout.cy;
        *w = ctx->layout.width;
        *h = default_h;
    }

    /* Track tallest widget in row */
    if (*h > ctx->layout.row_h) {
        ctx->layout.row_h = *h;
    }

    /* Save state for same_line to restore/use if called next */
    ctx->layout.row_x = (int16_t)(*x + *w + ctx->style.spacing);
    ctx->layout.row_y = *y;
    ctx->layout.prev_row_h = ctx->layout.row_h;

    /* Move to next row by default (same_line will undo this if called) */
    ctx->layout.cy = (int16_t)(ctx->layout.row_y + ctx->layout.row_h + ctx->style.spacing);
    ctx->layout.cx = ctx->layout.origin_x;
    ctx->layout.row_h = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Label
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_label(SYN_IMGUI_Context *ctx, const char *text,
                      int16_t x, int16_t y)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    int16_t lx = x;
    int16_t ly = y;
    int16_t lw = 0;
    int16_t lh = 0;
    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    layout_resolve(ctx, &lx, &ly, &lw, &lh, fh);

    syn_gfx_text(ctx->gfx, lx, ly, text, ctx->style.fg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Separator
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_separator(SYN_IMGUI_Context *ctx,
                          int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);

    int16_t sx = x;
    int16_t sy = y;
    int16_t sw = w;

    if (ctx->layout.in_layout) {
        if (sx == 0 && sy == 0) { sx = ctx->layout.cx; sy = ctx->layout.cy; }
        if (sw == 0)             { sw = ctx->layout.width; }
        /* Advance cursor */
        ctx->layout.cy = (int16_t)(sy + 1 + ctx->style.spacing);
        ctx->layout.cx = ctx->layout.origin_x;
        ctx->layout.row_h = 0;
    }

    syn_gfx_hline(ctx->gfx, sx, sy, sw, ctx->style.fg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Spinner
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_spinner(SYN_IMGUI_Context *ctx, const char *label,
                        int32_t *value, int32_t min, int32_t max, int32_t step,
                        int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(value != NULL);

    ctx->next_id++;
    track_focus(ctx);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + 2 * ctx->style.padding);

    layout_resolve(ctx, &x, &y, &w, &h, default_h);

    if (ctx->disabled_depth == 0) {
        if (is_hit_test(ctx, x, y, w, h)) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool changed    = false;

    /* Encoder changes value when this widget is focused and not disabled */
    if (ctx->disabled_depth == 0 && is_focused && ctx->enc_delta != 0) {
        int32_t new_val = *value + ctx->enc_delta * step;
        if (new_val > max) new_val = min;   /* wrap */
        if (new_val < min) new_val = max;
        if (new_val != *value) {
            *value = new_val;
            changed = true;
        }
    }

    /* ── Render ─────────────────────────────────────────────────────────── */

    /* Focus outline */
    if (is_focused) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Label on the left */
    int16_t label_w = (int16_t)syn_gfx_text_width(ctx->gfx, label);
    int16_t ty      = (int16_t)(y + (h - fh) / 2);
    syn_gfx_text(ctx->gfx, x + ctx->style.padding, ty, label, ctx->style.fg);

    /* Value box on the right: "< 42 >" */
    char val_str[16];
    {
        int32_t v = *value;
        uint8_t i = 0;
        if (v < 0) { val_str[i++] = '-'; v = -v; }
        char tmp[12];
        uint8_t j = 0;
        do { tmp[j++] = (char)('0' + (v % 10)); v /= 10; } while (v > 0 && j < 11u);
        while (j > 0u) { val_str[i++] = tmp[--j]; }
        val_str[i] = '\0';
    }
    int16_t val_w   = (int16_t)syn_gfx_text_width(ctx->gfx, val_str);
    int16_t arr_w   = (int16_t)syn_gfx_font_width(ctx->gfx);
    int16_t box_w   = (int16_t)(arr_w + ctx->style.padding + val_w + ctx->style.padding + arr_w);
    int16_t box_x   = (int16_t)(x + w - box_w - ctx->style.padding);

    (void)label_w; /* used for layout reference; value box is right-aligned */

    /* "<" arrow */
    syn_gfx_text(ctx->gfx, box_x, ty, "<", ctx->style.fg);
    /* value */
    syn_gfx_text(ctx->gfx, (int16_t)(box_x + arr_w + ctx->style.padding), ty,
                    val_str, ctx->style.fg);
    /* ">" arrow */
    syn_gfx_text(ctx->gfx,
                    (int16_t)(box_x + arr_w + ctx->style.padding + val_w + ctx->style.padding),
                    ty, ">", ctx->style.fg);

    return changed;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Scroll Region
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_scroll_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t *scroll)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(scroll != NULL);

    ctx->scroll.in_scroll     = true;
    ctx->scroll.vp_x          = x;
    ctx->scroll.vp_y          = y;
    ctx->scroll.vp_w          = w;
    ctx->scroll.vp_h          = h;
    ctx->scroll.scroll_p      = scroll;
    ctx->scroll.focus_y       = 0;
    ctx->scroll.focus_h       = 0;

    /* Set clip rect to viewport */
    syn_gfx_set_clip(ctx->gfx, x, y, w, h);

    /* Start a layout region inside the viewport, shifted by scroll offset */
    syn_imgui_layout_begin(ctx, x, (int16_t)(y - *scroll), w);

    /* Remember where content starts for height computation */
    ctx->scroll.content_start = ctx->layout.cy;
}

void syn_imgui_scroll_end(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);

    int16_t content_h = (int16_t)(ctx->layout.cy - ctx->scroll.content_start);
    int16_t vp_h      = ctx->scroll.vp_h;
    int16_t max_scroll = (content_h > vp_h) ? (int16_t)(content_h - vp_h) : 0;
    int16_t *sp        = ctx->scroll.scroll_p;

    /* Auto-scroll to keep focused widget visible */
    if (ctx->scroll.focus_h > 0) {
        int16_t fy = (int16_t)(ctx->scroll.focus_y + *sp);
        int16_t fh = (int16_t)ctx->scroll.focus_h;

        /* Widget is below viewport */
        if (fy + fh > ctx->scroll.vp_y + vp_h) {
            *sp = (int16_t)(ctx->scroll.focus_y + fh - vp_h);
        }
        /* Widget is above viewport */
        if (fy < ctx->scroll.vp_y) {
            *sp = (int16_t)ctx->scroll.focus_y;
        }
    }

    /* Clamp scroll */
    if (*sp < 0) *sp = 0;
    if (*sp > max_scroll) *sp = max_scroll;

    syn_imgui_layout_end(ctx);

    /* Reset clip rect to full display */
    syn_gfx_reset_clip(ctx->gfx);

    /* Draw scroll indicator if content overflows */
    if (content_h > vp_h && content_h > 0) {
        int16_t bar_x = (int16_t)(ctx->scroll.vp_x + ctx->scroll.vp_w - 2);
        int16_t track_h = vp_h;
        int16_t thumb_h = (int16_t)((int32_t)vp_h * vp_h / content_h);
        if (thumb_h < 3) thumb_h = 3;
        int16_t thumb_y = (int16_t)(ctx->scroll.vp_y +
                          (int32_t)*sp * (track_h - thumb_h) / max_scroll);
        syn_gfx_rect_fill(ctx->gfx, bar_x, thumb_y, 2, thumb_h, ctx->style.fg);
    }

    ctx->scroll.in_scroll = false;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Toggle Switch
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_toggle(SYN_IMGUI_Context *ctx, const char *label,
                       bool *state, int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(state != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + ctx->style.padding * 2);

    layout_resolve(ctx, &x, &y, &w, &h, default_h);
    ctx->next_id++;
    track_focus(ctx);

    /* Track focused widget position for scroll auto-follow */
    if (ctx->focused_id == ctx->next_id && ctx->scroll.in_scroll) {
        ctx->scroll.focus_y = (uint16_t)(y + *ctx->scroll.scroll_p - ctx->scroll.vp_y);
        ctx->scroll.focus_h = (uint16_t)h;
    }

    /* Skip drawing if fully outside scroll viewport */
    if (ctx->scroll.in_scroll) {
        if (y + h <= ctx->scroll.vp_y || y >= ctx->scroll.vp_y + ctx->scroll.vp_h)
            return false;
    }

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool toggled = false;

    /* Input */
    if (ctx->disabled_depth == 0) {
        bool touch_hit = is_hit_test(ctx, x, y, w, h);
        if (touch_hit) {
            ctx->focused_id = ctx->next_id;
            *state = !*state;
            toggled = true;
        }
        if (ctx->focused_id == ctx->next_id && ctx->btn_select) {
            *state = !*state;
            toggled = true;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);

    /* Draw focus outline */
    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Draw label */
    int16_t ty = (int16_t)(y + (h - fh) / 2);
    syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding), ty, label, fg);

    /* Draw toggle track (rounded rect) on the right side */
    int16_t track_w = 16;
    int16_t track_h = 8;
    int16_t track_x = (int16_t)(x + w - track_w - ctx->style.padding);
    int16_t track_y = (int16_t)(y + (h - track_h) / 2);

    syn_gfx_rect_round(ctx->gfx, track_x, track_y, track_w, track_h, 3, fg);

    /* Draw knob (filled circle) */
    int16_t knob_r = 2;
    int16_t knob_x = *state
        ? (int16_t)(track_x + track_w - knob_r - 3)
        : (int16_t)(track_x + knob_r + 3);
    int16_t knob_y = (int16_t)(track_y + track_h / 2);

    if (*state) {
        syn_gfx_circle_fill(ctx->gfx, knob_x, knob_y, knob_r, fg);
    } else {
        syn_gfx_circle(ctx->gfx, knob_x, knob_y, knob_r, fg);
    }

    return toggled;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Disabled State
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_begin_disabled(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    ctx->disabled_depth++;
}

void syn_imgui_end_disabled(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(ctx->disabled_depth > 0); /* unbalanced end_disabled */
    ctx->disabled_depth--;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Text Alignment Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_label_colored(SYN_IMGUI_Context *ctx, const char *text,
                              uint16_t color, int16_t x, int16_t y)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    int16_t lx = x, ly = y, lw = 0, lh = 0;
    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    layout_resolve(ctx, &lx, &ly, &lw, &lh, fh);

    syn_gfx_text(ctx->gfx, lx, ly, text, color);
}

void syn_imgui_label_right(SYN_IMGUI_Context *ctx, const char *text,
                            int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    int16_t lx = x, ly = y, lw = w, lh = 0;
    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    layout_resolve(ctx, &lx, &ly, &lw, &lh, fh);

    int16_t tw = (int16_t)syn_gfx_text_width(ctx->gfx, text);
    syn_gfx_text(ctx->gfx, (int16_t)(lx + lw - tw), ly, text, ctx->style.fg);
}

void syn_imgui_label_centered(SYN_IMGUI_Context *ctx, const char *text,
                               int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    int16_t lx = x, ly = y, lw = w, lh = 0;
    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    layout_resolve(ctx, &lx, &ly, &lw, &lh, fh);

    int16_t tw = (int16_t)syn_gfx_text_width(ctx->gfx, text);
    syn_gfx_text(ctx->gfx, (int16_t)(lx + (lw - tw) / 2), ly, text, ctx->style.fg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Group Box
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_group_begin(SYN_IMGUI_Context *ctx, const char *title,
                            int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;

    /* Draw the border */
    syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, fg);

    /* Draw title with background to "cut" the border */
    if (title != NULL && title[0] != '\0') {
        int16_t tw = (int16_t)syn_gfx_text_width(ctx->gfx, title);
        int16_t tx = (int16_t)(x + ctx->style.padding + 2);

        /* Clear the border behind the title text */
        syn_gfx_hline(ctx->gfx, tx - 1, y, (int16_t)(tw + 2), ctx->style.bg);
        syn_gfx_text(ctx->gfx, tx, (int16_t)(y - fh / 2), title, fg);
    }

    /* Push a sub-layout inside the group box */
    int16_t inset = (int16_t)(ctx->style.padding + 2);
    syn_imgui_layout_begin(ctx,
                            (int16_t)(x + inset),
                            (int16_t)(y + fh / 2 + ctx->style.spacing),
                            (int16_t)(w - inset * 2));
}

void syn_imgui_group_end(SYN_IMGUI_Context *ctx)
{
    SYN_ASSERT(ctx != NULL);
    syn_imgui_layout_end(ctx);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Tab Bar
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_tabs(SYN_IMGUI_Context *ctx, const char **labels,
                    size_t count, int32_t *active,
                    int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(labels != NULL);
    SYN_ASSERT(active != NULL);
    SYN_ASSERT(count > 0);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t tab_h = (int16_t)(fh + ctx->style.padding * 2);

    /* Resolve layout (use full width, tab height) */
    int16_t lx = x, ly = y, lw = w, lh = 0;
    layout_resolve(ctx, &lx, &ly, &lw, &lh, tab_h);
    if (!ctx->layout.in_layout) {
        lh = tab_h;
    }

    int16_t tab_w = (int16_t)(lw / (int16_t)count);

    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool changed = false;

    /* Input — like combo: encoder cycles tabs when focused */
    if (ctx->disabled_depth == 0 && is_focused && ctx->active_id == ctx->next_id) {
        if (ctx->enc_delta != 0) {
            int32_t new_val = *active + ctx->enc_delta;
            if (new_val >= (int32_t)count) new_val = 0;
            if (new_val < 0) new_val = (int32_t)count - 1;
            if (new_val != *active) {
                *active = new_val;
                changed = true;
            }
        }
        /* Select exits edit mode */
        if (ctx->btn_select) {
            ctx->active_id = 0;
        }
    } else if (ctx->disabled_depth == 0 && is_focused && ctx->btn_select) {
        /* Enter edit mode */
        ctx->active_id = ctx->next_id;
    }

    /* Touch input: detect which tab was tapped */
    if (ctx->disabled_depth == 0 && ctx->touch_down) {
        for (size_t i = 0; i < count; i++) {
            int16_t tx = (int16_t)(lx + (int16_t)i * tab_w);
            if (is_hit_test(ctx, tx, ly, tab_w, lh)) {
                ctx->focused_id = ctx->next_id;
                if ((int32_t)i != *active) {
                    *active = (int32_t)i;
                    changed = true;
                }
                break;
            }
        }
    }

    /* Draw tabs */
    for (size_t i = 0; i < count; i++) {
        int16_t tx = (int16_t)(lx + (int16_t)i * tab_w);
        int16_t tw = (int16_t)syn_gfx_text_width(ctx->gfx, labels[i]);
        int16_t text_y = (int16_t)(ly + ctx->style.padding);

        /* Clip text to tab slot and center if it fits, else left-align with 1px margin */
        if (tw <= tab_w - 2) {
            int16_t text_x = (int16_t)(tx + (tab_w - tw) / 2);
            syn_gfx_text(ctx->gfx, text_x, text_y, labels[i], fg);
        } else {
            /* Text too wide: use text_clipped to truncate within the tab */
            syn_imgui_text_clipped(ctx, labels[i],
                                   (int16_t)(tx + 1), text_y,
                                   (int16_t)(tx + 1), ly,
                                   (int16_t)(tab_w - 2), lh);
        }

        /* Active tab: underline */
        if ((int32_t)i == *active) {
            syn_gfx_hline(ctx->gfx, tx, (int16_t)(ly + lh - 1), tab_w, fg);
        }

        /* Divider between tabs (not after the last) */
        if (i + 1 < count) {
            int16_t div_x = (int16_t)(tx + tab_w - 1);
            syn_gfx_vline(ctx->gfx, div_x, ly, lh, fg);
        }
    }

    /* Focus highlight */
    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect(ctx->gfx, lx, ly, lw, lh, ctx->style.highlight);
    }

    /* Draw bottom divider */
    syn_gfx_hline(ctx->gfx, lx, (int16_t)(ly + lh), lw, fg);

    return changed;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Bar Chart
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_bar_chart(SYN_IMGUI_Context *ctx, const char *title,
                          const int32_t *data, size_t count,
                          int32_t min_val, int32_t max_val,
                          int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(data != NULL);

    /* Resolve geometry from layout cursor */
    layout_resolve(ctx, &x, &y, &w, &h, h > 0 ? h : (int16_t)32);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;

    /* Border */
    syn_gfx_rect(ctx->gfx, x, y, w, h, fg);

    /* Title */
    if (title != NULL && title[0] != '\0') {
        syn_gfx_text(ctx->gfx, (int16_t)(x + 4), (int16_t)(y + 4), title, fg);
    }

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t plot_y = (int16_t)(y + fh + 6);
    int16_t plot_h = (int16_t)(h - fh - 8);
    int16_t plot_x = (int16_t)(x + 2);
    int16_t plot_w = (int16_t)(w - 4);

    if (count == 0 || plot_h <= 0 || plot_w <= 0) return;

    int32_t range = max_val - min_val;
    if (range <= 0) range = 1;

    int16_t bar_w = (int16_t)(plot_w / (int16_t)count);
    if (bar_w < 1) bar_w = 1;
    int16_t gap = (bar_w > 2) ? 1 : 0;

    for (size_t i = 0; i < count; i++) {
        int32_t val = data[i];
        if (val < min_val) val = min_val;
        if (val > max_val) val = max_val;

        int16_t bar_h = (int16_t)((int32_t)(val - min_val) * plot_h / range);
        if (bar_h < 1 && val > min_val) bar_h = 1;

        int16_t bx = (int16_t)(plot_x + (int16_t)i * bar_w + gap);
        int16_t by = (int16_t)(plot_y + plot_h - bar_h);

        syn_gfx_rect_fill(ctx->gfx, bx, by, (int16_t)(bar_w - gap), bar_h, fg);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Icon Button
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_icon_button(SYN_IMGUI_Context *ctx,
                            const uint8_t *icon, int16_t icon_w, int16_t icon_h,
                            int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(icon != NULL);

    int16_t default_h = (int16_t)(icon_h + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &w, &h, default_h);
    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool clicked = false;

    if (ctx->disabled_depth == 0) {
        bool touch_hit = is_hit_test(ctx, x, y, w, h);
        if (touch_hit) {
            ctx->focused_id = ctx->next_id;
            clicked = true;
        }
        if (ctx->focused_id == ctx->next_id && ctx->btn_select) {
            clicked = true;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);

    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect_round_fill(ctx->gfx, x, y, w, h, 4, fg);
        /* Draw icon inverted */
        int16_t ix = (int16_t)(x + (w - icon_w) / 2);
        int16_t iy = (int16_t)(y + (h - icon_h) / 2);
        syn_gfx_bitmap(ctx->gfx, ix, iy, icon, icon_w, icon_h, ctx->style.bg);
    } else {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 4, fg);
        int16_t ix = (int16_t)(x + (w - icon_w) / 2);
        int16_t iy = (int16_t)(y + (h - icon_h) / 2);
        syn_gfx_bitmap(ctx->gfx, ix, iy, icon, icon_w, icon_h, fg);
    }

    return clicked;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Status Bar
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_status_bar(SYN_IMGUI_Context *ctx, const char *text,
                           int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    /* Resolve geometry from layout cursor */
    uint8_t fh_sb = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_sb = (int16_t)(fh_sb + 4); /* separator line + text + padding */
    int16_t lh_sb = 0;
    layout_resolve(ctx, &x, &y, &w, &lh_sb, default_h_sb);
    if (!ctx->layout.in_layout) {
        lh_sb = default_h_sb;
    }

    uint16_t fg = ctx->style.fg;

    /* Separator line above the text */
    syn_gfx_hline(ctx->gfx, x, y, w, fg);

    /* Text below the separator */
    syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding),
                 (int16_t)(y + 2), text, fg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Separator Text  (inspired by Dear ImGui SeparatorText)
 * ─── Section Title ───────────────────────────────────────────────────────
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_separator_text(SYN_IMGUI_Context *ctx, const char *text,
                                int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    uint16_t fg = ctx->style.fg;
    uint8_t  fh = syn_gfx_text_height(ctx->gfx);
    int16_t  h  = (int16_t)(fh + 2); /* height of this row */

    /* Participate in the layout: resolve (x,y,w) from cursor, advance it */
    int16_t lh = 0;
    layout_resolve(ctx, &x, &y, &w, &lh, h);
    if (!ctx->layout.in_layout) {
        lh = h;
    }

    int16_t text_w = (int16_t)syn_gfx_text_width(ctx->gfx, text);
    int16_t mid_y  = (int16_t)(y + fh / 2);
    int16_t gap    = 4; /* pixels between line end and text */

    /* Left line segment */
    syn_gfx_hline(ctx->gfx, x, mid_y, gap, fg);

    /* Text */
    int16_t text_x = (int16_t)(x + gap * 2);
    syn_gfx_text(ctx->gfx, text_x, y, text, fg);

    /* Right line segment */
    int16_t right_start = (int16_t)(text_x + text_w + gap);
    int16_t right_end   = (int16_t)(x + w);
    if (right_end > right_start) {
        syn_gfx_hline(ctx->gfx, right_start, mid_y,
                       (int16_t)(right_end - right_start), fg);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Checkbox Flags  (inspired by Dear ImGui CheckboxFlags / Nuklear
 *                   nk_checkbox_flags_label)
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_checkbox_flags(SYN_IMGUI_Context *ctx, const char *label,
                                uint32_t *flags, uint32_t mask,
                                int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(flags != NULL);

    bool checked = (*flags & mask) == mask;
    bool toggled = syn_imgui_checkbox(ctx, label, &checked, x, y, w, h);
    if (toggled) {
        if (checked) {
            *flags |= mask;
        } else {
            *flags &= ~mask;
        }
    }
    return toggled;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Value display  (inspired by Dear ImGui Value / Nuklear nk_value_int)
 * Draws "label: value" as a non-interactive text pair.
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_value_int(SYN_IMGUI_Context *ctx, const char *label,
                           int32_t value, int16_t x, int16_t y)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);

    /* Participate in layout: resolve position and advance cursor */
    uint8_t fh_vi = syn_gfx_text_height(ctx->gfx);
    int16_t lw_vi = 0, lh_vi = 0;
    int16_t default_h_vi = (int16_t)(fh_vi + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &lw_vi, &lh_vi, default_h_vi);
    (void)lw_vi;

    uint16_t fg = ctx->style.fg;

    /* Draw label */
    syn_gfx_text(ctx->gfx, x, y, label, fg);
    int16_t label_w = (int16_t)syn_gfx_text_width(ctx->gfx, label);

    /* Format value */
    char buf[16];
    int32_t v = value;
    uint8_t i = 0;
    if (v < 0) { buf[i++] = '-'; v = -v; }
    char tmp[12];
    uint8_t j = 0;
    do { tmp[j++] = (char)('0' + (v % 10)); v /= 10; } while (v > 0 && j < 11u);
    while (j > 0u) { buf[i++] = tmp[--j]; }
    buf[i] = '\0';

    /* Draw ": value" */
    int16_t colon_x = (int16_t)(x + label_w);
    syn_gfx_text(ctx->gfx, colon_x, y, ": ", fg);
    int16_t colon_w = (int16_t)syn_gfx_text_width(ctx->gfx, ": ");
    syn_gfx_text(ctx->gfx, (int16_t)(colon_x + colon_w), y, buf, fg);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Progress bar with overlay text  (inspired by Dear ImGui ProgressBar)
 *
 * overlay == NULL → auto "XX%" text
 * overlay == ""   → no text
 * value < min     → indeterminate mode (bouncing bar)
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_progress_bar_ex(SYN_IMGUI_Context *ctx, int32_t value,
                                 int32_t min, int32_t max,
                                 const char *overlay,
                                 int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);

    /* Resolve geometry from layout cursor */
    uint8_t fh_pb = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_pb = (int16_t)(fh_pb + ctx->style.padding * 2);
    layout_resolve(ctx, &x, &y, &w, &h, default_h_pb);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool indeterminate = (value < min);

    /* Outer frame */
    syn_gfx_rect(ctx->gfx, x, y, w, h, fg);

    if (max > min) {
        int16_t inner_w = (int16_t)(w - 4);
        if (indeterminate) {
            /* Bouncing 20% wide bar based on low bits of value */
            int32_t phase = (value < 0 ? -value : value) % (inner_w * 2);
            int16_t bar_w = (int16_t)(inner_w / 5);
            if (bar_w < 2) bar_w = 2;
            int16_t bar_x;
            if (phase < inner_w) {
                bar_x = (int16_t)(x + 2 + phase);
            } else {
                bar_x = (int16_t)(x + 2 + (inner_w * 2 - phase));
            }
            /* Clamp to bounds */
            if (bar_x + bar_w > x + w - 2) bar_w = (int16_t)(x + w - 2 - bar_x);
            if (bar_w > 0) {
                syn_gfx_rect_fill(ctx->gfx, bar_x, (int16_t)(y + 2),
                                   bar_w, (int16_t)(h - 4), fg);
            }
        } else {
            /* Normal fill */
            int32_t val = value;
            if (val < min) val = min;
            if (val > max) val = max;
            int32_t range = max - min;
            int32_t current = val - min;
            int16_t fill_w = (int16_t)((current * inner_w) / range);
            if (fill_w > 0) {
                syn_gfx_rect_fill(ctx->gfx, (int16_t)(x + 2), (int16_t)(y + 2),
                                   fill_w, (int16_t)(h - 4), fg);
            }
        }
    }

    /* Overlay text */
    if (!indeterminate) {
        char auto_buf[8];
        const char *text = overlay;
        if (text == NULL && max > min) {
            /* Auto-generate "XX%" */
            int32_t pct = ((value - min) * 100) / (max - min);
            if (pct < 0) pct = 0;
            if (pct > 100) pct = 100;
            uint8_t i = 0;
            if (pct >= 100) { auto_buf[i++] = '1'; auto_buf[i++] = '0'; auto_buf[i++] = '0'; }
            else if (pct >= 10) { auto_buf[i++] = (char)('0' + pct / 10); auto_buf[i++] = (char)('0' + pct % 10); }
            else { auto_buf[i++] = (char)('0' + pct); }
            auto_buf[i++] = '%';
            auto_buf[i] = '\0';
            text = auto_buf;
        }
        if (text != NULL && text[0] != '\0') {
            int16_t tw = (int16_t)syn_gfx_text_width(ctx->gfx, text);
            uint8_t fh = syn_gfx_text_height(ctx->gfx);
            int16_t tx = (int16_t)(x + (w - tw) / 2);
            int16_t ty = (int16_t)(y + (h - fh) / 2);
            /* Draw text in inverted color if fill is under the text */
            syn_gfx_text(ctx->gfx, tx, ty, text, fg);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Selectable  (inspired by Dear ImGui Selectable / microui mu_button with
 *              MU_OPT_NOFRAME)
 * Full-width clickable row — no border, only a highlight fill when focused.
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_selectable(SYN_IMGUI_Context *ctx, const char *label,
                            bool *selected,
                            int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(selected != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + 2 * ctx->style.padding);
    layout_resolve(ctx, &x, &y, &w, &h, default_h);

    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool touch_clicked = false;

    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool clicked = false;

    if (ctx->disabled_depth == 0 && (touch_clicked || (is_focused && ctx->btn_select))) {
        *selected = !(*selected);
        clicked = true;
    }

    /* Draw filled background when selected or focused */
    if (*selected) {
        syn_gfx_rect_fill(ctx->gfx, x, y, w, h, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding),
                     (int16_t)(y + (h - fh) / 2), label, ctx->style.bg);
    } else if (is_focused && ctx->disabled_depth == 0) {
        /* Dotted/outline highlight for focus without selection */
        syn_gfx_rect(ctx->gfx, x, y, w, h, fg);
        syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding),
                     (int16_t)(y + (h - fh) / 2), label, fg);
    } else {
        syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding),
                     (int16_t)(y + (h - fh) / 2), label, fg);
    }

    return clicked;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collapsing Header  (feature #5, inspired by all three libs)
 *
 * Renders "▸ Label" (collapsed) or "▾ Label" (expanded).
 * Toggles *expanded on click.  User gates the body:
 *   if (*expanded) { ... widgets ... }
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_collapsing_header(SYN_IMGUI_Context *ctx, const char *label,
                                   bool *expanded,
                                   int16_t x, int16_t y, int16_t w, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(label != NULL);
    SYN_ASSERT(expanded != NULL);

    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    int16_t default_h = (int16_t)(fh + 2 * ctx->style.padding);
    layout_resolve(ctx, &x, &y, &w, &h, default_h);

    ctx->next_id++;
    track_focus(ctx);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    bool touch_clicked = false;

    if (ctx->disabled_depth == 0) {
        touch_clicked = is_hit_test(ctx, x, y, w, h);
        if (touch_clicked) {
            ctx->focused_id = ctx->next_id;
        }
    }

    bool is_focused = (ctx->focused_id == ctx->next_id);
    bool toggled = false;

    if (ctx->disabled_depth == 0 && (touch_clicked || (is_focused && ctx->btn_select))) {
        *expanded = !(*expanded);
        toggled = true;
    }

    /* Draw focus outline */
    if (is_focused && ctx->disabled_depth == 0) {
        syn_gfx_rect_round(ctx->gfx, x, y, w, h, 3, ctx->style.highlight);
    }

    /* Draw arrow + label */
    int16_t ty = (int16_t)(y + (h - fh) / 2);
    const char *arrow = *expanded ? "-" : "+";
    syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding), ty, arrow, fg);
    int16_t arrow_w = (int16_t)syn_gfx_text_width(ctx->gfx, arrow);
    syn_gfx_text(ctx->gfx, (int16_t)(x + ctx->style.padding + arrow_w + 2), ty, label, fg);

    /* Separator line below */
    syn_gfx_hline(ctx->gfx, x, (int16_t)(y + h - 1), w, fg);

    return toggled;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Text Word-Wrap  (feature #6, inspired by microui mu_text)
 *
 * Renders multi-line word-wrapped text within width.
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_text_wrapped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y, int16_t w)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    uint8_t fh_tw = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_tw = (int16_t)(fh_tw + 1);
    int16_t lh_tw = 0;
    layout_resolve(ctx, &x, &y, &w, &lh_tw, default_h_tw);
    if (!ctx->layout.in_layout) {
        lh_tw = default_h_tw;
    }

    uint16_t fg = ctx->style.fg;
    uint8_t fh = syn_gfx_text_height(ctx->gfx);
    uint8_t fw = syn_gfx_font_width(ctx->gfx);
    int16_t cx = x;
    int16_t cy = y;

    if (fw == 0 || w <= 0) return;

    const char *p = text;
    while (*p) {
        /* Find end of next word */
        const char *word_start = p;
        while (*p && *p != ' ' && *p != '\n') p++;
        int16_t word_len = (int16_t)(p - word_start);
        int16_t word_w = (int16_t)(word_len * fw);

        /* Wrap if this word doesn't fit on the current line */
        if (cx + word_w > x + w && cx > x) {
            cx = x;
            cy = (int16_t)(cy + fh + 1);
        }

        /* Render character by character */
        for (const char *c = word_start; c < word_start + word_len; c++) {
            const char ch[2] = { *c, '\0' };
            syn_gfx_text(ctx->gfx, cx, cy, ch, fg);
            cx = (int16_t)(cx + fw);
        }

        /* Handle space / newline */
        if (*p == ' ') {
            cx = (int16_t)(cx + fw);
            p++;
        } else if (*p == '\n') {
            cx = x;
            cy = (int16_t)(cy + fh + 1);
            p++;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Layout Row  (feature #7, inspired by microui mu_layout_row)
 *
 * Define N columns with specific widths.  Subsequent widgets that use
 * auto-layout (all coords 0) will fill them left-to-right.
 * Positive width = fixed pixels.  Negative width = fill remainder.
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_layout_row(SYN_IMGUI_Context *ctx, int items,
                            const int16_t *widths, int16_t height)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(ctx->layout.in_layout);

    /* Start a new row */
    if (ctx->layout.row_h > 0) {
        ctx->layout.cy = (int16_t)(ctx->layout.cy + ctx->layout.row_h + ctx->style.spacing);
        ctx->layout.row_h = 0;
    }
    ctx->layout.cx = ctx->layout.origin_x;
    ctx->layout.same_line = false;

    /* Store row params in layout for widgets to consume */
    ctx->layout.row_items = items;
    ctx->layout.row_item_idx = 0;
    ctx->layout.row_height = height;
    if (widths != NULL && items > 0) {
        int max = items;
        if (max > SYN_IMGUI_MAX_ROW_COLS) max = SYN_IMGUI_MAX_ROW_COLS;
        for (int i = 0; i < max; i++) {
            ctx->layout.row_widths[i] = widths[i];
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Visibility Culling  (feature #8)
 *
 * Returns false if the widget rect is fully outside the scroll viewport.
 * Widgets should still increment next_id for focus tracking.
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_imgui_widget_visible(const SYN_IMGUI_Context *ctx,
                                int16_t y, int16_t h)
{
    SYN_ASSERT(ctx != NULL);
    if (!ctx->scroll.in_scroll) return true; /* no scroll = always visible */

    int16_t vp_top = ctx->scroll.vp_y;
    int16_t vp_bot = (int16_t)(ctx->scroll.vp_y + ctx->scroll.vp_h);

    /* Fully below viewport or fully above */
    if (y >= vp_bot || y + h <= vp_top) return false;
    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Text Clipped  (feature #9, inspired by microui mu_draw_control_text)
 *
 * Draws text, but clips it to a bounding rectangle using the canvas
 * clip rect mechanism.
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_text_clipped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y,
                              int16_t clip_x, int16_t clip_y,
                              int16_t clip_w, int16_t clip_h)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);

    /* text_clipped is a lower-level primitive used by other widgets; it does
     * NOT participate in the layout cursor since clip_x/y/w/h are explicit.
     * Callers that want layout advancement should call a higher-level widget. */
    SYN_Canvas *c = (SYN_Canvas *)ctx->gfx;

    /* Save current clip rect */
    int16_t old_cx = c->clip_x;
    int16_t old_cy = c->clip_y;
    int16_t old_cw = c->clip_w;
    int16_t old_ch = c->clip_h;

    /* Set clip to widget bounds */
    syn_canvas_set_clip(c, clip_x, clip_y, clip_w, clip_h);

    /* Draw text (canvas will clip at pixel level) */
    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    syn_gfx_text(ctx->gfx, x, y, text, fg);

    /* Restore clip rect */
    if (old_cw > 0 && old_ch > 0) {
        syn_canvas_set_clip(c, old_cx, old_cy, old_cw, old_ch);
    } else {
        syn_canvas_reset_clip(c);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Text Marquee
 *
 * Scrolling text label.  If the text fits, it's drawn statically.
 * If it overflows, it scrolls left with pauses at start and end.
 *
 * The caller owns the scroll state — just declare:
 *   static int16_t offset = 0;
 *
 * speed: pixels per call (1-2 typical).  Pause ticks at each end = 2/speed
 * cycles worth.
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_imgui_text_marquee(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t *offset,
                              int16_t x, int16_t y, int16_t w,
                              int16_t speed)
{
    SYN_ASSERT(ctx != NULL);
    SYN_ASSERT(text != NULL);
    SYN_ASSERT(offset != NULL);

    uint16_t fg = ctx->disabled_depth > 0 ? ctx->style.disabled : ctx->style.fg;
    int16_t text_w = (int16_t)syn_gfx_text_width(ctx->gfx, text);

    if (speed <= 0) speed = 1;

    /* Resolve geometry from layout cursor before any drawing */
    uint8_t fh_mq = syn_gfx_text_height(ctx->gfx);
    int16_t default_h_mq = (int16_t)(fh_mq + 2);
    int16_t lh_mq = 0;
    layout_resolve(ctx, &x, &y, &w, &lh_mq, default_h_mq);
    if (!ctx->layout.in_layout) {
        lh_mq = default_h_mq;
    }

    /* Text fits — draw statically, reset offset */
    if (text_w <= w) {
        syn_gfx_text(ctx->gfx, x, y, text, fg);
        *offset = 0;
        return;
    }

    /* Overflow amount (how far we need to scroll) */
    int16_t overflow = (int16_t)(text_w - w);

    /* Pause duration at each end (in calls).  ~30 calls at speed=1 */
    int16_t pause = (int16_t)(30 / speed);
    if (pause < 4) pause = 4;

    /* Total cycle: pause + scroll_right + pause + scroll_left */
    int16_t total_cycle = (int16_t)(pause + overflow + pause + overflow);

    /* Normalize offset into cycle */
    int16_t pos = *offset;
    if (pos < 0) pos = 0;
    if (pos >= total_cycle) pos = 0;

    /* Determine text_x shift */
    int16_t shift;
    if (pos < pause) {
        /* Phase 1: paused at start */
        shift = 0;
    } else if (pos < pause + overflow) {
        /* Phase 2: scrolling left */
        shift = (int16_t)(pos - pause);
    } else if (pos < pause + overflow + pause) {
        /* Phase 3: paused at end */
        shift = overflow;
    } else {
        /* Phase 4: scrolling right (back) */
        shift = (int16_t)(overflow - (pos - pause - overflow - pause));
    }

    /* Clip to widget bounds and draw */
    SYN_Canvas *c = (SYN_Canvas *)ctx->gfx;
    int16_t old_cx = c->clip_x;
    int16_t old_cy = c->clip_y;
    int16_t old_cw = c->clip_w;
    int16_t old_ch = c->clip_h;

    syn_canvas_set_clip(c, x, y, w, lh_mq);
    syn_gfx_text(ctx->gfx, (int16_t)(x - shift), y, text, fg);

    if (old_cw > 0 && old_ch > 0) {
        syn_canvas_set_clip(c, old_cx, old_cy, old_cw, old_ch);
    } else {
        syn_canvas_reset_clip(c);
    }

    /* Advance offset */
    *offset = (int16_t)(pos + speed);
    if (*offset >= total_cycle) *offset = 0;
}

#endif /* SYN_USE_IMGUI */
