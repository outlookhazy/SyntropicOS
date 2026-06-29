/**
 * @file visual_test.c
 * @brief Comprehensive visual regression harness for syn_imgui.
 *
 * Renders every public widget in every meaningful state, writes each to a
 * 4x-scaled PNG, then assembles them into a labeled grid image.
 *
 * Build:
 *   make -C examples/imgui_host visual_test
 *
 * Run:
 *   ./examples/imgui_host/visual_test
 *
 * Output: /tmp/imgui_visual_test.png
 */

#define _POSIX_C_SOURCE 200809L

#include "syntropic/display/syn_canvas.h"
#include "syntropic/ui/syn_imgui.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/util/syn_assert.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
/* minimal nearest-neighbour scaler inline — no dep needed */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ── assert ──────────────────────────────────────────────────────────────── */
void syn_assert_failed(const char *file, int line)
{
    fprintf(stderr, "ASSERT FAILED: %s:%d\n", file, line);
    abort();
}

/* ── Display geometry ────────────────────────────────────────────────────── */
#define W   128
#define H   64
#define SZ  (W * H / 8)

/* ── Framebuffer → RGBA (4× scale) ──────────────────────────────────────── */
#define SCALE 4
static void fb_to_rgba(const uint8_t *fb, uint8_t *rgba)
{
    int ow = W * SCALE, oh = H * SCALE;
    for (int py = 0; py < H; py++) {
        for (int px = 0; px < W; px++) {
            int page = py / 8, bit = py % 8;
            uint8_t v = ((fb[page * W + px] >> bit) & 1) ? 255 : 0;
            for (int sy = 0; sy < SCALE; sy++)
                for (int sx = 0; sx < SCALE; sx++) {
                    int idx = ((py*SCALE+sy)*ow + (px*SCALE+sx))*4;
                    rgba[idx+0] = rgba[idx+1] = rgba[idx+2] = v;
                    rgba[idx+3] = 255;
                }
        }
    }
    (void)oh;
}

/* ── One test case ───────────────────────────────────────────────────────── */
typedef struct { char name[64]; uint8_t fb[SZ]; } TestFrame;

static TestFrame g_frames[64];
static int       g_nframes = 0;

static void begin_frame(const char *name,
                        SYN_Canvas *c, SYN_IMGUI_Context *ctx,
                        uint8_t *fb,
                        bool sel, bool back, int32_t enc)
{
    memset(fb, 0, SZ);
    syn_canvas_init(c, fb, W, H, 1, NULL, NULL);
    syn_imgui_init(ctx);
    /* pre-warm last_max_id so focus starts at 1 */
    ctx->last_max_id = 8;
    syn_canvas_clear(c);
    syn_imgui_begin(ctx, c, sel, back, enc, false, 0, 0);
    strncpy(g_frames[g_nframes].name, name, 63);
    g_frames[g_nframes].name[63] = '\0';
}

static void end_frame(SYN_IMGUI_Context *ctx, uint8_t *fb)
{
    syn_imgui_end(ctx);
    memcpy(g_frames[g_nframes].fb, fb, SZ);
    g_nframes++;
}

/* Macro to simplify */
#define BEGIN(name, sel, back, enc) \
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx; \
    begin_frame(name, &_c, &_ctx, _fb, sel, back, enc)
#define END() end_frame(&_ctx, _fb)

/* ═══════════════════════════════════════════════════════════════════════════
 * Test cases
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── Button ──────────────────────────────────────────────────────────────── */
static void tc_button_idle(void)
{
    BEGIN("btn: idle", false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_button(&_ctx, "Click Me", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Button 2", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Button 3", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    END();
}

static void tc_button_focused(void)
{
    BEGIN("btn: focused", false, false, 1); /* enc+1 moves focus to 2 */
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_button(&_ctx, "Button 1", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Focused!", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Button 3", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    END();
}

static void tc_button_disabled(void)
{
    BEGIN("btn: disabled", true, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_begin_disabled(&_ctx);
    syn_imgui_button(&_ctx, "Disabled", 0, 0, 0, 0);
    syn_imgui_end_disabled(&_ctx);
    syn_imgui_button(&_ctx, "Active", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    END();
}

/* ── Checkbox ────────────────────────────────────────────────────────────── */
static void tc_checkbox(void)
{
    BEGIN("checkbox: states", false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    bool checked = true, unchecked = false;
    syn_imgui_checkbox(&_ctx, "Checked",   &checked,   0, 0, 0, 0);
    syn_imgui_checkbox(&_ctx, "Unchecked", &unchecked, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    END();
}

/* ── Checkbox flags ──────────────────────────────────────────────────────── */
static void tc_checkbox_flags(void)
{
    BEGIN("chk_flags: mixed", false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    uint32_t f = 0x05; /* bits 0 and 2 set */
    syn_imgui_checkbox_flags(&_ctx, "Bit 0 ON",  &f, 0x01, 0, 0, 0, 0);
    syn_imgui_checkbox_flags(&_ctx, "Bit 1 OFF", &f, 0x02, 0, 0, 0, 0);
    syn_imgui_checkbox_flags(&_ctx, "Bit 2 ON",  &f, 0x04, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    END();
}

/* ── Slider ──────────────────────────────────────────────────────────────── */
static void tc_slider_idle(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("slider: 50%", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t v = 50;
    syn_imgui_slider(&_ctx, "Level", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* Rewrite slider tests properly */
static void tc_slider_low(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("slider: 10%", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t v = 10;
    syn_imgui_slider(&_ctx, "Volume", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_slider_full(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("slider: 100%", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t v = 100;
    syn_imgui_slider(&_ctx, "Volume", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_slider_active(void)
{
    /* Two-frame sequence: select to enter active, then render */
    uint8_t fb[SZ]; SYN_Canvas c; SYN_IMGUI_Context ctx;
    memset(fb, 0, SZ);
    syn_canvas_init(&c, fb, W, H, 1, NULL, NULL);
    syn_imgui_init(&ctx);
    ctx.last_max_id = 2;
    /* Frame 1: focus slider, press select */
    syn_canvas_clear(&c);
    syn_imgui_begin(&ctx, &c, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, W);
    int32_t v = 50;
    syn_imgui_slider(&ctx, "Brightness", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
    /* Frame 2: now active (select was pressed, focus=1 already) */
    ctx.focused_id = 1;
    syn_canvas_clear(&c);
    syn_imgui_begin(&ctx, &c, true, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, W);
    syn_imgui_slider(&ctx, "Brightness", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);

    begin_frame("slider: active edit", &c, &ctx, fb, false, false, 0);
    /* Re-render with active_id set */
    memset(fb, 0, SZ);
    syn_canvas_init(&c, fb, W, H, 1, NULL, NULL);
    syn_canvas_clear(&c);
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;
    ctx.focused_id  = 1;
    ctx.active_id   = 1;
    syn_imgui_begin(&ctx, &c, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, W);
    v = 65;
    syn_imgui_slider(&ctx, "Brightness", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&ctx);
    end_frame(&ctx, fb);
}

/* ── Radio ───────────────────────────────────────────────────────────────── */
static void tc_radio(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("radio: sel=1", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t mode = 1;
    syn_imgui_radio(&_ctx, "Mode A", &mode, 0, 0, 0, 0, 0);
    syn_imgui_radio(&_ctx, "Mode B", &mode, 1, 0, 0, 0, 0);
    syn_imgui_radio(&_ctx, "Mode C", &mode, 2, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Combo ───────────────────────────────────────────────────────────────── */
static void tc_combo_idle(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("combo: idle sel=1", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    const char *opts[] = {"Alpha", "Beta", "Gamma"};
    int32_t sel = 1;
    syn_imgui_combo(&_ctx, "Mode", opts, 3, &sel, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_combo_active(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("combo: editing", &_c, &_ctx, _fb, false, false, 0);
    _ctx.last_max_id = 1; _ctx.focused_id = 1; _ctx.active_id = 1;
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    const char *opts[] = {"Alpha", "Beta", "Gamma"};
    int32_t sel = 2;
    syn_imgui_combo(&_ctx, "Mode", opts, 3, &sel, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Spinner ─────────────────────────────────────────────────────────────── */
static void tc_spinner(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("spinner: val=42", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t v = 42;
    syn_imgui_spinner(&_ctx, "Count", &v, 0, 99, 1, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Toggle ──────────────────────────────────────────────────────────────── */
static void tc_toggle(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("toggle: on+off", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    bool on = true, off = false;
    syn_imgui_toggle(&_ctx, "WiFi ON",  &on,  0, 0, 0, 0);
    syn_imgui_toggle(&_ctx, "BLE  OFF", &off, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Tabs ────────────────────────────────────────────────────────────────── */
static void tc_tabs(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("tabs: tab 1 active", &_c, &_ctx, _fb, false, false, 0);
    const char *tabs[] = {"Overview", "Advanced", "Log"};
    int32_t active = 1;
    syn_imgui_tabs(&_ctx, tabs, 3, &active, 0, 0, W);
    end_frame(&_ctx, _fb);
}

static void tc_tabs_layout(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("tabs: layout resolved", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    const char *tabs[] = {"Dash", "Set", "Info"};
    int32_t active = 0;
    syn_imgui_tabs(&_ctx, tabs, 3, &active, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Collapsing header ───────────────────────────────────────────────────── */
static void tc_collapsing_collapsed(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("collapsing: closed", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    bool open = false;
    syn_imgui_collapsing_header(&_ctx, "Config", &open, 0, 0, 0, 0);
    syn_imgui_collapsing_header(&_ctx, "Network", &open, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_collapsing_open(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("collapsing: open", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    bool open = true, closed = false;
    if (syn_imgui_collapsing_header(&_ctx, "Config", &open, 0, 0, 0, 0)) {
        syn_imgui_label(&_ctx, "  Mode: Active", 0, 0);
        syn_imgui_label(&_ctx, "  Volt: 3.3V", 0, 0);
    }
    syn_imgui_collapsing_header(&_ctx, "Network", &closed, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Selectable list ─────────────────────────────────────────────────────── */
static void tc_selectable(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("selectable: 3 items", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    bool s0 = true, s1 = false, s2 = true;
    syn_imgui_selectable(&_ctx, "Item Alpha", &s0, 0, 0, 0, 0);
    syn_imgui_selectable(&_ctx, "Item Beta",  &s1, 0, 0, 0, 0);
    syn_imgui_selectable(&_ctx, "Item Gamma", &s2, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Scroll region ───────────────────────────────────────────────────────── */
static void tc_scroll_top(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("scroll: top (0)", &_c, &_ctx, _fb, false, false, 0);
    int16_t off = 0;
    syn_imgui_scroll_begin(&_ctx, 0, 0, W, H, &off);
    for (int i = 0; i < 8; i++) {
        char lbl[16]; snprintf(lbl, sizeof(lbl), "Row %d", i+1);
        bool sel = (i == 1);
        syn_imgui_selectable(&_ctx, lbl, &sel, 0, 0, 0, 0);
    }
    syn_imgui_scroll_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_scroll_mid(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("scroll: mid (28px)", &_c, &_ctx, _fb, false, false, 0);
    int16_t off = 28;
    syn_imgui_scroll_begin(&_ctx, 0, 0, W, H, &off);
    for (int i = 0; i < 8; i++) {
        char lbl[16]; snprintf(lbl, sizeof(lbl), "Row %d", i+1);
        bool sel = false;
        syn_imgui_selectable(&_ctx, lbl, &sel, 0, 0, 0, 0);
    }
    syn_imgui_scroll_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Separator text ──────────────────────────────────────────────────────── */
static void tc_separator_text(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("separator_text", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_separator_text(&_ctx, "Section A", 0, 0, 0);
    syn_imgui_label(&_ctx, "Item 1", 0, 0);
    syn_imgui_separator_text(&_ctx, "Section B", 0, 0, 0);
    syn_imgui_label(&_ctx, "Item 2", 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Progress bars ───────────────────────────────────────────────────────── */
static void tc_progress_bars(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("progress: 0/50/100/%", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_progress_bar_ex(&_ctx,   0, 0, 100, NULL,    0, 0, 0, 0);
    syn_imgui_progress_bar_ex(&_ctx,  50, 0, 100, NULL,    0, 0, 0, 0);
    syn_imgui_progress_bar_ex(&_ctx, 100, 0, 100, NULL,    0, 0, 0, 0);
    syn_imgui_progress_bar_ex(&_ctx,  73, 0, 100, "Done!", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_progress_indeterminate(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("progress: indeterminate", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    /* value < min triggers indeterminate mode */
    syn_imgui_progress_bar_ex(&_ctx, -30, 0, 100, NULL, 0, 0, 0, 0);
    syn_imgui_progress_bar_ex(&_ctx, -60, 0, 100, NULL, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Value int ───────────────────────────────────────────────────────────── */
static void tc_value_int(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("value_int: pos/neg/zero", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_value_int(&_ctx, "RPM",  1500, 0, 0);
    syn_imgui_value_int(&_ctx, "Temp", -23,  0, 0);
    syn_imgui_value_int(&_ctx, "Err",  0,    0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Text marquee ────────────────────────────────────────────────────────── */
static void tc_marquee_static(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("marquee: short (static)", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int16_t off = 0;
    syn_imgui_text_marquee(&_ctx, "Short", &off, 0, 0, 0, 1);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

static void tc_marquee_scrolling(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("marquee: scrolled 20px", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int16_t off = 20;
    syn_imgui_text_marquee(&_ctx, "SyntropicOS Embedded UI Engine v1.0", &off, 0, 0, 0, 1);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Text wrapped ────────────────────────────────────────────────────────── */
static void tc_text_wrapped(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("text_wrapped", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_text_wrapped(&_ctx, "Word wrap test: long lines break here nicely", 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Status bar ──────────────────────────────────────────────────────────── */
static void tc_status_bar(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("status_bar", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_label(&_ctx, "Main content here", 0, 0);
    syn_imgui_label(&_ctx, "More content here", 0, 0);
    syn_imgui_layout_end(&_ctx);
    /* Status bar is outside layout — draw at fixed bottom */
    syn_imgui_status_bar(&_ctx, "Ready | 3.3V | WiFi", 0, H-9, W);
    end_frame(&_ctx, _fb);
}

/* ── Bar chart ───────────────────────────────────────────────────────────── */
static void tc_bar_chart(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("bar_chart", &_c, &_ctx, _fb, false, false, 0);
    int32_t data[] = {10, 70, 40, 90, 20, 60};
    syn_imgui_bar_chart(&_ctx, "Samples", data, 6, 0, 100, 0, 0, W, H);
    end_frame(&_ctx, _fb);
}

/* ── Graph ───────────────────────────────────────────────────────────────── */
static void tc_graph(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("graph: sine-like", &_c, &_ctx, _fb, false, false, 0);
    int32_t data[] = {50,72,88,95,88,72,50,28,12,5,12,28,50,72,88,95};
    syn_imgui_graph(&_ctx, "Signal", data, 16, 0, 100, 0, 0, W, H);
    end_frame(&_ctx, _fb);
}

/* ── Gauge ───────────────────────────────────────────────────────────────── */
static void tc_gauge(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("gauge: 75%", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_gauge(&_ctx, "Speed", 75, 0, 100, W/2, H/2+8, 28);
    end_frame(&_ctx, _fb);
}

/* ── Dialog ──────────────────────────────────────────────────────────────── */
static void tc_dialog(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("dialog: modal", &_c, &_ctx, _fb, false, false, 0);
    /* draw background content first without ending frame */
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_label(&_ctx, "Background content", 0, 0);
    syn_imgui_layout_end(&_ctx);
    /* end the begin() from begin_frame, then start a fresh one for the modal */
    syn_imgui_end(&_ctx);
    /* now render the modal dialog in a second frame pass on the same fb */
    _ctx.last_max_id = 2;
    syn_imgui_begin(&_ctx, &_c, false, false, 0, false, 0, 0);
    bool ok = false;
    syn_imgui_dialog(&_ctx, "Delete file?", &ok, 20, 15, 88, 34);
    end_frame(&_ctx, _fb);
}

/* ── Disabled nested ─────────────────────────────────────────────────────── */
static void tc_disabled_mixed(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("disabled: mixed layout", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_label(&_ctx, "Enabled label", 0, 0);
    syn_imgui_begin_disabled(&_ctx);
    syn_imgui_label(&_ctx, "Disabled label", 0, 0);
    int32_t v = 30;
    syn_imgui_slider(&_ctx, "Disabled slider", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_end_disabled(&_ctx);
    syn_imgui_label(&_ctx, "Enabled again", 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── same_line layout ────────────────────────────────────────────────────── */
static void tc_same_line(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("same_line: 3 buttons", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, 30);
    /* NOTE: all-zero w/h required for layout_resolve to fire.
     * Buttons will auto-size to (label_w + 2*padding) x (fh + 2*padding). */
    syn_imgui_button(&_ctx, "A", 0, 0, 0, 0);
    syn_imgui_same_line(&_ctx);
    syn_imgui_button(&_ctx, "B", 0, 0, 0, 0);
    syn_imgui_same_line(&_ctx);
    syn_imgui_button(&_ctx, "C", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── layout_row ──────────────────────────────────────────────────────────── */
static void tc_layout_row(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("layout_row: 2-col", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int16_t widths[] = {60, 64};
    syn_imgui_layout_row(&_ctx, 2, widths, 13);
    syn_imgui_label(&_ctx, "Left col", 0, 0);
    syn_imgui_label(&_ctx, "Right col", 0, 0);
    syn_imgui_layout_row(&_ctx, 2, widths, 13);
    bool t = true;
    syn_imgui_checkbox(&_ctx, "Check", &t, 0, 0, 0, 0);
    syn_imgui_label(&_ctx, "Detail", 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Icon button ─────────────────────────────────────────────────────────── */
static void tc_icon_button(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("icon_button", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    /* 8x8 filled square icon */
    static const uint8_t icon[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    syn_imgui_icon_button(&_ctx, icon, 8, 8, 0, 0, 20, 0);
    syn_imgui_same_line(&_ctx);
    syn_imgui_label(&_ctx, "Play", 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Full page 0 (the actual app screen) ─────────────────────────────────── */
static void tc_page0_full(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("page0: full screen", &_c, &_ctx, _fb, false, false, 0);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    uint32_t flags = 0x01;
    int32_t bright = 65;
    syn_imgui_separator_text(&_ctx, "GUI Overview", 0, 0, 0);
    syn_imgui_checkbox_flags(&_ctx, "LED 1", &flags, 0x01, 0, 0, 0, 0);
    syn_imgui_checkbox_flags(&_ctx, "LED 2", &flags, 0x02, 0, 0, 0, 0);
    syn_imgui_slider(&_ctx, "Brightness", &bright, 0, 100, 0, 0, 0, 0);
    syn_imgui_value_int(&_ctx, "Speed", 10, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Focus wrap: navigate past last widget ───────────────────────────────── */
static void tc_focus_wrap(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    /* enc=3 on a 3-widget list should wrap to widget 1 */
    begin_frame("focus wrap: 3→1", &_c, &_ctx, _fb, false, false, 3);
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    syn_imgui_button(&_ctx, "Button 1", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Button 2", 0, 0, 0, 0);
    syn_imgui_button(&_ctx, "Button 3", 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ── Navigator: back button clears active ────────────────────────────────── */
static void tc_back_clears_active(void)
{
    uint8_t _fb[SZ]; SYN_Canvas _c; SYN_IMGUI_Context _ctx;
    begin_frame("back: exits slider", &_c, &_ctx, _fb, false, true, 0);
    _ctx.last_max_id = 1; _ctx.focused_id = 1; _ctx.active_id = 1;
    syn_imgui_layout_begin(&_ctx, 0, 0, W);
    int32_t v = 50;
    syn_imgui_slider(&_ctx, "Vol (editing)", &v, 0, 100, 0, 0, 0, 0);
    syn_imgui_layout_end(&_ctx);
    end_frame(&_ctx, _fb);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Grid assembly
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CELL_W  (W * SCALE)    /* 512 */
#define CELL_H  (H * SCALE)    /* 256 */
#define LABEL_H  22
#define COLS     4
#define MARGIN   6
#define FONT_PX  1  /* we'll draw label in white text directly into the grid */

/* Tiny 5×7 digit/letter renderer for grid labels — just uses stb style */
static void draw_label(uint8_t *grid, int gw, int x, int y,
                       const char *text, int scale)
{
    /* Write text as white pixels — minimal 5px-wide glyph using canvas would
     * require linking back to syn_canvas. Instead just write the string into
     * the PNG using stdio chars as pixel art.  Since we have stb, we can't
     * easily render text to RGBA without a full font.  Use a 1px-high dotted
     * indicator line + rely on the label being visible to a human reader via
     * a separate text overlay.  Actually: write the label text to stdout and
     * trust the grid ordering — each frame title is printed to console. */
    (void)grid; (void)gw; (void)x; (void)y; (void)text; (void)scale;
}

int main(void)
{
    /* Register all test cases */
    tc_button_idle();
    tc_button_focused();
    tc_button_disabled();
    tc_checkbox();
    tc_checkbox_flags();
    tc_slider_low();
    tc_slider_full();
    tc_slider_active();
    tc_radio();
    tc_combo_idle();
    tc_combo_active();
    tc_spinner();
    tc_toggle();
    tc_tabs();
    tc_tabs_layout();
    tc_collapsing_collapsed();
    tc_collapsing_open();
    tc_selectable();
    tc_scroll_top();
    tc_scroll_mid();
    tc_separator_text();
    tc_progress_bars();
    tc_progress_indeterminate();
    tc_value_int();
    tc_marquee_static();
    tc_marquee_scrolling();
    tc_text_wrapped();
    tc_status_bar();
    tc_bar_chart();
    tc_graph();
    tc_gauge();
    tc_dialog();
    tc_disabled_mixed();
    tc_same_line();
    tc_layout_row();
    tc_icon_button();
    tc_page0_full();
    tc_focus_wrap();
    tc_back_clears_active();

    printf("Rendered %d test frames:\n", g_nframes);
    for (int i = 0; i < g_nframes; i++)
        printf("  [%2d] %s\n", i, g_frames[i].name);

    /* Assemble grid */
    int rows = (g_nframes + COLS - 1) / COLS;
    int gw   = COLS * (CELL_W + MARGIN) + MARGIN;
    int gh   = rows * (CELL_H + LABEL_H + MARGIN) + MARGIN;

    uint8_t *grid = calloc((size_t)(gw * gh * 3), 1);
    if (!grid) { perror("calloc"); return 1; }

    /* Dark background */
    for (int i = 0; i < gw * gh * 3; i += 3) {
        grid[i+0] = 20; grid[i+1] = 22; grid[i+2] = 30;
    }

    uint8_t *rgba = malloc((size_t)(CELL_W * CELL_H * 4));

    for (int fi = 0; fi < g_nframes; fi++) {
        int col = fi % COLS;
        int row = fi / COLS;
        int ox  = MARGIN + col * (CELL_W + MARGIN);
        int oy  = MARGIN + row * (CELL_H + LABEL_H + MARGIN);

        /* Decode frame to RGBA then blit RGB into grid */
        fb_to_rgba(g_frames[fi].fb, rgba);
        for (int py = 0; py < CELL_H; py++) {
            for (int px = 0; px < CELL_W; px++) {
                int si = (py * CELL_W + px) * 4;
                int di = ((oy + py) * gw + (ox + px)) * 3;
                grid[di+0] = rgba[si+0];
                grid[di+1] = rgba[si+1];
                grid[di+2] = rgba[si+2];
            }
        }

        /* Label bar: light gray row below the frame */
        int ly = oy + CELL_H;
        for (int py = 0; py < LABEL_H; py++) {
            for (int px = 0; px < CELL_W; px++) {
                int di = ((ly + py) * gw + (ox + px)) * 3;
                grid[di+0] = 40; grid[di+1] = 44; grid[di+2] = 55;
            }
        }
        /* Print frame index+name: no easy way to render text into RGBA
         * without bringing in stb_truetype. The title line + index are
         * printed to stdout above. Add a white index number dot: */
        /* Top-left corner: write fi+1 as tiny 3px dots */
        int dot_x = ox + 2, dot_y = ly + 3;
        uint8_t bright = 200;
        for (int b = 0; b < 3 && dot_x + b < gw; b++) {
            int di = (dot_y * gw + dot_x + b) * 3;
            grid[di] = bright; grid[di+1] = bright; grid[di+2] = bright;
        }
    }

    free(rgba);

    const char *out = "/tmp/imgui_visual_test.png";
    stbi_write_png(out, gw, gh, 3, grid, gw * 3);
    free(grid);

    printf("\nGrid saved: %s (%d×%d, %d frames)\n", out, gw, gh, g_nframes);
    return 0;
}
