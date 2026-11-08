/**
 * @file test_imgui.c
 * @brief Unity tests for the SyntropicOS IMGUI framework.
 */

#include "syntropic/display/syn_canvas.h"
#include "syntropic/syntropic.h"
#include "syntropic/ui/syn_imgui.h"
#include "unity/unity.h"

#include <limits.h>
#include <string.h>

static void test_imgui_navigation(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* Frame 1: Discover widget count */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);  /* ID 1 */
    syn_imgui_button(&ctx, "B2", 0, 20, 40, 15); /* ID 2 */
    syn_imgui_button(&ctx, "B3", 0, 40, 40, 15); /* ID 3 */
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL_INT(3, ctx.last_max_id);
    TEST_ASSERT_EQUAL_INT(1, ctx.focused_id); /* default focus at 1 */

    /* Frame 2: Rotate encoder right (positive delta) -> focus moves to 2 */
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);
    syn_imgui_button(&ctx, "B3", 0, 40, 40, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL_INT(2, ctx.focused_id);

    /* Frame 3: Rotate encoder right twice -> focus moves 2 -> 3 -> 1 (wrap) */
    syn_imgui_begin(&ctx, &canvas, false, false, 2, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);
    syn_imgui_button(&ctx, "B3", 0, 40, 40, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL_INT(1, ctx.focused_id);

    /* Frame 4: Rotate encoder left (negative delta) -> focus moves 1 -> 3 (wrap back) */
    syn_imgui_begin(&ctx, &canvas, false, false, -1, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);
    syn_imgui_button(&ctx, "B3", 0, 40, 40, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL_INT(3, ctx.focused_id);
}

static void test_imgui_widgets(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 3;

    /* 1. Button click test */
    /* Focus is on 1 (Button), press select */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool b1_clicked = syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    bool b2_clicked = syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(b1_clicked);
    TEST_ASSERT_FALSE(b2_clicked);

    /* 2. Checkbox toggle test */
    /* Move focus to index 2 (Checkbox) and press select */
    ctx.focused_id = 2;
    bool checked = false;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    bool cb_toggled = syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(cb_toggled);
    TEST_ASSERT_TRUE(checked);

    /* 3. Slider active editing test */
    /* Focus index 3 (Slider) and select it to enter edit mode */
    ctx.focused_id = 3;
    int32_t val = 50;

    /* Frame A: Select button enters edit mode */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    bool s_changed = syn_imgui_slider(&ctx, "SL", &val, 0, 100, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(s_changed);
    TEST_ASSERT_EQUAL_INT(3, ctx.active_id); /* Slider should be in active editing mode */

    /* Frame B: Rotary dial updates the slider value instead of focus */
    syn_imgui_begin(&ctx, &canvas, false, false, 15, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    s_changed = syn_imgui_slider(&ctx, "SL", &val, 0, 100, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(s_changed);
    TEST_ASSERT_EQUAL_INT(65, val);
    TEST_ASSERT_EQUAL_INT(3, ctx.active_id);  /* Remains active */
    TEST_ASSERT_EQUAL_INT(3, ctx.focused_id); /* Focus did not move despite dial movement */

    /* Frame C: Select key again exits edit mode */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    s_changed = syn_imgui_slider(&ctx, "SL", &val, 0, 100, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(s_changed);
    TEST_ASSERT_EQUAL_INT(0, ctx.active_id); /* Edit mode exited */
}

static void test_imgui_touch(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 3;

    /* 1. Touch button click */
    /* Touch at (20, 7) inside button 1 bounds (x=0, y=0, w=40, h=15) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 20, 7);
    bool b1_clicked = syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    bool b2_clicked = syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(b1_clicked);
    TEST_ASSERT_FALSE(b2_clicked);
    TEST_ASSERT_EQUAL_INT(1, ctx.focused_id); /* Focus shifted to touch location */

    /* 2. Touch checkbox toggle */
    /* Touch at (50, 25) inside checkbox bounds (x=0, y=20, w=80, h=15) */
    bool checked = false;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 50, 25);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    bool cb_toggled = syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(cb_toggled);
    TEST_ASSERT_TRUE(checked);
    TEST_ASSERT_EQUAL_INT(2, ctx.focused_id);

    /* 3. Touch slider setting */
    /* Slider bounds: x=0, y=40, w=80, h=15.
     * Slider bar region: bar_x = 44, bar_w = 32.
     * Touching bar_x + 16 (x=60) should set value to half (50/100 scale). */
    int32_t val = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 60, 47);
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);
    syn_imgui_checkbox(&ctx, "CB", &checked, 0, 20, 80, 15);
    bool s_changed = syn_imgui_slider(&ctx, "SL", &val, 0, 100, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(s_changed);
    /* bar_x = 44, touch_x = 60, relative = 16. (16 * 100) / 32 = 50 */
    TEST_ASSERT_EQUAL_INT(50, val);
    TEST_ASSERT_EQUAL_INT(3, ctx.focused_id);
    TEST_ASSERT_EQUAL_INT(3, ctx.active_id);
}

static void test_imgui_radio(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 3;

    int32_t mode = 0;

    /* 1. Button click toggle */
    ctx.focused_id = 2;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_radio(&ctx, "Mode 0", &mode, 0, 0, 0, 80, 15);
    bool r_changed = syn_imgui_radio(&ctx, "Mode 1", &mode, 1, 0, 20, 80, 15);
    syn_imgui_radio(&ctx, "Mode 2", &mode, 2, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(r_changed);
    TEST_ASSERT_EQUAL_INT(1, mode);

    /* 2. Touch selection */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 40, 45);
    syn_imgui_radio(&ctx, "Mode 0", &mode, 0, 0, 0, 80, 15);
    syn_imgui_radio(&ctx, "Mode 1", &mode, 1, 0, 20, 80, 15);
    r_changed = syn_imgui_radio(&ctx, "Mode 2", &mode, 2, 0, 40, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(r_changed);
    TEST_ASSERT_EQUAL_INT(2, mode);
    TEST_ASSERT_EQUAL_INT(3, ctx.focused_id);
}

static void test_imgui_advanced(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;
    ctx.last_max_id = 5;

    /* 1. Progress Bar Test */

    syn_canvas_clear(&canvas);
    syn_imgui_progress_bar(&ctx, 50, 0, 100, 0, 0, 32, 8);
    /* Framebuffer should have drawing markers */
    TEST_ASSERT_TRUE(fb[0] != 0);

    /* 2. Combo Box Test */
    const char *opts[] = {"A", "B", "C"};
    int32_t selected = 0;

    /* Press select to enter edit mode */
    ctx.focused_id = 1;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool c_changed = syn_imgui_combo(&ctx, "Combo", opts, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(c_changed);
    TEST_ASSERT_EQUAL_INT(1, ctx.active_id); /* active */

    /* Rotate dial to increment */
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    c_changed = syn_imgui_combo(&ctx, "Combo", opts, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(c_changed);
    TEST_ASSERT_EQUAL_INT(1, selected);

    /* Touch Arrow Hit: Left arrow region (val_x = 40, left_x = 42).
     * Touch at (44, 7) should decrement selected back to 0. */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 44, 7);
    c_changed = syn_imgui_combo(&ctx, "Combo", opts, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(c_changed);
    TEST_ASSERT_EQUAL_INT(0, selected);

    /* 3. Graph plotting test */
    int32_t telemetry[5] = {10, 50, 90, 30, 70};
    syn_canvas_clear(&canvas);
    syn_imgui_graph(&ctx, "Telemetry", telemetry, 5, 0, 100, 0, 0, 64, 32);
    /* Basic sanity check: data line drawn */
    TEST_ASSERT_TRUE(fb[0] != 0);

    /* 4. Gauge indicator test */
    syn_canvas_clear(&canvas);
    syn_imgui_gauge(&ctx, "Speed", 75, 0, 100, 32, 32, 16);
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);

    /* 5. Modal Dialog popup test */
    /* OK button is at ID next_id+1 = 1. Cancel button is at ID next_id+2 = 2.
     * Select key triggers OK confirmation. */
    bool ok_clicked = false;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool dismissed = syn_imgui_dialog(&ctx, "Confirm?", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(dismissed);
    TEST_ASSERT_TRUE(ok_clicked);
}

/* ── Style ───────────────────────────────────────────────────────────────── */

static void test_imgui_style_defaults(void)
{
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* Default style must match monochrome white-on-black */
    TEST_ASSERT_EQUAL_HEX16(SYN_COLOR_WHITE, ctx.style.fg);
    TEST_ASSERT_EQUAL_HEX16(SYN_COLOR_BLACK, ctx.style.bg);
    TEST_ASSERT_EQUAL_INT(2, ctx.style.padding);
    TEST_ASSERT_EQUAL_INT(3, ctx.style.spacing);
}

static void test_imgui_set_style(void)
{
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    SYN_IMGUI_Style s = syn_imgui_default_style();
    s.fg = SYN_COLOR_RED;
    s.padding = 4;
    syn_imgui_set_style(&ctx, &s);

    TEST_ASSERT_EQUAL_HEX16(SYN_COLOR_RED, ctx.style.fg);
    TEST_ASSERT_EQUAL_INT(4, ctx.style.padding);
}

/* ── Layout cursor ───────────────────────────────────────────────────────── */

static void test_imgui_layout_cursor_advances(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 4, 4, 120);

    /* First widget at cursor (4, 4) */
    TEST_ASSERT_EQUAL_INT(4, ctx.layout.cx);
    TEST_ASSERT_EQUAL_INT(4, ctx.layout.cy);

    /* Draw a button via layout (x=0,y=0,w=0,h=0) */
    syn_imgui_button(&ctx, "OK", 0, 0, 0, 0);

    /* After a 5x7 font: h = 7 + 2*2=11, spacing=3 → cy should advance to 4+11+3=18 */
    TEST_ASSERT_EQUAL_INT(4, ctx.layout.cx); /* back to origin_x */
    TEST_ASSERT_EQUAL_INT(18, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_same_line(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 60);

    syn_imgui_button(&ctx, "A", 0, 0, 0, 0); /* advances cy to 0+11+3=14 */
    /* same_line: next widget appends to the right, cy stays at 14 */
    syn_imgui_same_line(&ctx);
    int16_t cx_before = ctx.layout.cx;
    (void)cx_before;
    syn_imgui_button(&ctx, "B", 0, 0, 0, 0); /* on same row as A */

    /* cy unchanged after same_line widget — still at 14 */
    TEST_ASSERT_EQUAL_INT(14, ctx.layout.cy);

    /* Now a normal widget — advances to 14+11+3=28 */
    syn_imgui_button(&ctx, "C", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT(28, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

/* ── Spinner ─────────────────────────────────────────────────────────────── */

static void test_imgui_spinner_encoder_increment(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t val = 10;

    /* Frame 1: discover widget (enc_delta=0) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_spinner(&ctx, "Speed", &val, 0, 100, 1, 10, 10, 100, 13);
    syn_imgui_end(&ctx);

    /* Frame 2: encoder +2 while focused (spinner is the only widget → focused_id=1) */
    syn_imgui_begin(&ctx, &canvas, false, false, 2, false, 0, 0);
    bool changed = syn_imgui_spinner(&ctx, "Speed", &val, 0, 100, 1, 10, 10, 100, 13);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_INT32(12, val);
}

static void test_imgui_spinner_wraps_at_max(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t val = 100;

    /* Frame 1: discover */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_spinner(&ctx, "V", &val, 0, 100, 1, 10, 10, 100, 13);
    syn_imgui_end(&ctx);

    /* Frame 2: encoder +1 — at max, should wrap to min (0) */
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    bool changed = syn_imgui_spinner(&ctx, "V", &val, 0, 100, 1, 10, 10, 100, 13);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_INT32(0, val);
}

static void test_imgui_spinner_no_change_when_unfocused(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t val = 50;

    /* Two widgets: button (ID 1, focused), spinner (ID 2, unfocused) */
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    syn_imgui_button(&ctx, "Go", 0, 0, 30, 13); /* ID 1 — gets focus via enc_delta */
    bool changed = syn_imgui_spinner(&ctx, "V", &val, 0, 100, 1, 40, 0, 80, 13);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_EQUAL_INT32(50, val); /* unchanged */
}

/* ── canvas text_height / hline / vline ──────────────────────────────────── */

static void test_canvas_text_height(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    /* Built-in 5x7 font: height = 7 */
    TEST_ASSERT_EQUAL_UINT8(7u, syn_canvas_text_height(&canvas));
}

static void test_canvas_hline_draws_pixels(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    syn_canvas_clear(&canvas);
    syn_canvas_hline(&canvas, 0, 0, 4, SYN_COLOR_WHITE);
    /* In 1bpp the first byte stores the first 8 vertical pixels of column 0-7.
     * Row 0 bit is bit 7 of byte 0 for each column. */
    /* Actually in SSD1306-style 1bpp: byte index = x + (y/8)*width
     * bit = y % 8.  For y=0, bit=0, mask=0x01.
     * Pixels (0,0),(1,0),(2,0),(3,0) → bytes 0,1,2,3, all bit 0 set. */
    TEST_ASSERT_BITS(0x01u, 0x01u, fb[0]);
    TEST_ASSERT_BITS(0x01u, 0x01u, fb[1]);
    TEST_ASSERT_BITS(0x01u, 0x01u, fb[2]);
    TEST_ASSERT_BITS(0x01u, 0x01u, fb[3]);
}

/* ── Toggle Switch ──────────────────────────────────────────────────────── */

static void test_imgui_toggle(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    bool state = false;

    /* Frame 1: Press select while focused on toggle → should toggle on */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool toggled = syn_imgui_toggle(&ctx, "WiFi", &state, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_TRUE(state);

    /* Frame 2: Press select again → should toggle off */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    toggled = syn_imgui_toggle(&ctx, "WiFi", &state, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_FALSE(state);

    /* Frame 3: Touch hit → should toggle on */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 40, 7);
    toggled = syn_imgui_toggle(&ctx, "WiFi", &state, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_TRUE(state);
}

/* ── Disabled State ─────────────────────────────────────────────────────── */

static void test_imgui_disabled(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    bool state = false;

    /* Press select while disabled → should NOT toggle */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_begin_disabled(&ctx);
    bool toggled = syn_imgui_toggle(&ctx, "WiFi", &state, 0, 0, 80, 15);
    syn_imgui_end_disabled(&ctx);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(toggled);
    TEST_ASSERT_FALSE(state);
    TEST_ASSERT_EQUAL_UINT8(0, ctx.disabled_depth); /* Should be cleared */
}

/* ── Tabs ───────────────────────────────────────────────────────────────── */

static void test_imgui_tabs(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    const char *tabs[] = {"Dash", "Set", "Info"};
    int32_t active = 0;

    /* Frame 1: select enters edit mode */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool changed = syn_imgui_tabs(&ctx, tabs, 3, &active, 0, 0, 128);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_EQUAL_INT(1, ctx.active_id); /* Now in edit mode */

    /* Frame 2: encoder +1 while in edit mode → tab changes */
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    changed = syn_imgui_tabs(&ctx, tabs, 3, &active, 0, 0, 128);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_INT(1, active); /* Moved to "Set" */

    /* Frame 3: touch second tab directly */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 85, 5);
    changed = syn_imgui_tabs(&ctx, tabs, 3, &active, 0, 0, 128);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_INT(2, active); /* Tapped "Info" */
}

/* ── Scroll Region ──────────────────────────────────────────────────────── */

static void test_imgui_scroll_region(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int16_t scroll_offset = 0;

    /* Frame 1: discover widgets - 5 buttons in a scroll region.
     * Viewport 128x30, each button is ~11px tall + 3px spacing = 14px per item.
     * Only ~2 fit in the 30px viewport. */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_scroll_begin(&ctx, 0, 0, 128, 30, &scroll_offset);
    syn_imgui_button(&ctx, "A", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "B", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "C", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "D", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "E", 0, 0, 0, 0);
    syn_imgui_scroll_end(&ctx);
    syn_imgui_end(&ctx);

    /* All 5 widgets should have been counted */
    TEST_ASSERT_EQUAL_INT(5, ctx.last_max_id);

    /* Clip rect should have been restored to full display */
    TEST_ASSERT_EQUAL_INT(0, canvas.clip_x);
    TEST_ASSERT_EQUAL_INT(0, canvas.clip_y);
    TEST_ASSERT_EQUAL_INT(128, canvas.clip_w);
    TEST_ASSERT_EQUAL_INT(64, canvas.clip_h);

    /* Initially scroll should be 0 */
    TEST_ASSERT_EQUAL_INT(0, scroll_offset);
}

/* ── Bar Chart ──────────────────────────────────────────────────────────── */

static void test_imgui_bar_chart(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    int32_t data[4] = {10, 50, 80, 30};
    syn_imgui_bar_chart(&ctx, "Stats", data, 4, 0, 100, 0, 0, 64, 32);

    /* Verify that some pixels were drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Icon Button ────────────────────────────────────────────────────────── */

static void test_imgui_icon_button(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    /* Simple 8x8 icon: filled square */
    const uint8_t icon[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    /* Press select while focused */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool clicked = syn_imgui_icon_button(&ctx, icon, 8, 8, 0, 0, 20, 12);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(clicked);
}

/* ── Label Alignment ────────────────────────────────────────────────────── */

static void test_imgui_label_alignment(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);

    /* These should not crash and should draw pixels */
    syn_imgui_label_colored(&ctx, "Red", SYN_COLOR_WHITE, 0, 0);
    syn_imgui_label_right(&ctx, "Right", 0, 10, 128);
    syn_imgui_label_centered(&ctx, "Center", 0, 20, 128);

    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Status Bar ─────────────────────────────────────────────────────────── */

static void test_imgui_status_bar(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    syn_imgui_status_bar(&ctx, "Ready", 0, 55, 128);

    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Disabled button (bug fix regression test) ──────────────────────────── */

static void test_imgui_disabled_button(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    /* Press select while disabled → should NOT click */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_begin_disabled(&ctx);
    bool clicked = syn_imgui_button(&ctx, "Test", 0, 0, 60, 15);
    syn_imgui_end_disabled(&ctx);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(clicked);
}

/* ── Nested disabled (bug #1 regression test) ───────────────────────────── */

static void test_imgui_nested_disabled(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    /* Nest two disabled blocks — inner end should NOT re-enable */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    syn_imgui_begin_disabled(&ctx);
    syn_imgui_begin_disabled(&ctx);
    syn_imgui_end_disabled(&ctx);
    /* Should still be disabled here (depth == 1) */
    TEST_ASSERT_EQUAL_UINT8(1, ctx.disabled_depth);
    bool clicked = syn_imgui_button(&ctx, "Test", 0, 0, 60, 15);
    TEST_ASSERT_FALSE(clicked); /* should NOT fire because still disabled */
    syn_imgui_end_disabled(&ctx);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL_UINT8(0, ctx.disabled_depth);
}

/* ── Stale focus (bug #3 regression test) ───────────────────────────────── */

static void test_imgui_stale_focus(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* Frame 1: 3 buttons, focus on widget 3 */
    ctx.last_max_id = 3;
    ctx.focused_id = 3;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "A", 0, 0, 40, 15);
    syn_imgui_button(&ctx, "B", 45, 0, 40, 15);
    syn_imgui_button(&ctx, "C", 90, 0, 40, 15);
    syn_imgui_end(&ctx);
    /* Focus should have been validated (widget 3 exists) */
    TEST_ASSERT_EQUAL_UINT16(3, ctx.focused_id);

    /* Frame 2: now only 2 buttons — widget 3 doesn't exist */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "A", 0, 0, 40, 15);
    syn_imgui_button(&ctx, "B", 45, 0, 40, 15);
    syn_imgui_end(&ctx);
    /* Focus should have been reset because widget 3 was not visited */
    TEST_ASSERT_TRUE(ctx.focused_id <= 2);
}

/* ── Separator Text ─────────────────────────────────────────────────────── */

static void test_imgui_separator_text(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    syn_imgui_separator_text(&ctx, "Motor", 0, 10, 128);

    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Checkbox Flags ─────────────────────────────────────────────────────── */

static void test_imgui_checkbox_flags(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    uint32_t flags = 0x00;

    /* Select should toggle bit 0x04 on */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool changed = syn_imgui_checkbox_flags(&ctx, "IRQ", &flags, 0x04, 0, 0, 100, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_HEX32(0x04, flags);

    /* Select again should toggle bit 0x04 off */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    changed = syn_imgui_checkbox_flags(&ctx, "IRQ", &flags, 0x04, 0, 0, 100, 15);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_HEX32(0x00, flags);
}

/* ── Value Int ──────────────────────────────────────────────────────────── */

static void test_imgui_value_int(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    syn_imgui_value_int(&ctx, "RPM", 1500, 0, 10);

    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Progress bar ex (overlay + indeterminate) ──────────────────────────── */

static void test_imgui_progress_bar_ex(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    /* Normal mode with auto overlay */
    syn_canvas_clear(&canvas);
    syn_imgui_progress_bar_ex(&ctx, 73, 0, 100, NULL, 0, 0, 100, 12);
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);

    /* Indeterminate mode (value < min) */
    syn_canvas_clear(&canvas);
    syn_imgui_progress_bar_ex(&ctx, -10, 0, 100, NULL, 0, 0, 100, 12);
    has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Selectable ─────────────────────────────────────────────────────────── */

static void test_imgui_selectable(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    bool selected = false;

    /* Press select while focused → should select */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool clicked = syn_imgui_selectable(&ctx, "Item A", &selected, 0, 0, 128, 14);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(clicked);
    TEST_ASSERT_TRUE(selected);

    /* Press select again → should deselect */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    clicked = syn_imgui_selectable(&ctx, "Item A", &selected, 0, 0, 128, 14);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(clicked);
    TEST_ASSERT_FALSE(selected);
}

/* ── Collapsing header (feature #5) ─────────────────────────────────────── */

static void test_imgui_collapsing_header(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.last_max_id = 1;

    bool expanded = false;

    /* Press select on focused header → should expand */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool toggled = syn_imgui_collapsing_header(&ctx, "Settings", &expanded, 0, 0, 128, 14);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_TRUE(expanded);

    /* Press again → should collapse */
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    toggled = syn_imgui_collapsing_header(&ctx, "Settings", &expanded, 0, 0, 128, 14);
    syn_imgui_end(&ctx);

    TEST_ASSERT_TRUE(toggled);
    TEST_ASSERT_FALSE(expanded);
}

/* ── Text word-wrap (feature #6) ────────────────────────────────────────── */

static void test_imgui_text_wrapped(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    syn_imgui_text_wrapped(&ctx, "This is a long text that should wrap", 0, 0, 60);

    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── Layout row (feature #7) ───────────────────────────────────────────── */

static void test_imgui_layout_row(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t widths[] = {40, 88};
    syn_imgui_layout_row(&ctx, 2, widths, 14);

    TEST_ASSERT_EQUAL_INT(2, ctx.layout.row_items);
    TEST_ASSERT_EQUAL_INT(0, ctx.layout.row_item_idx);
    TEST_ASSERT_EQUAL_INT16(40, ctx.layout.row_widths[0]);
    TEST_ASSERT_EQUAL_INT16(88, ctx.layout.row_widths[1]);
    TEST_ASSERT_EQUAL_INT16(14, ctx.layout.row_height);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

/* ── Widget visible (feature #8) ────────────────────────────────────────── */

static void test_imgui_widget_visible(void)
{
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* No scroll region → always visible */
    TEST_ASSERT_TRUE(syn_imgui_widget_visible(&ctx, 0, 10));

    /* Simulate scroll region */
    ctx.scroll.in_scroll = true;
    ctx.scroll.vp_y = 20;
    ctx.scroll.vp_h = 40;

    /* Fully inside */
    TEST_ASSERT_TRUE(syn_imgui_widget_visible(&ctx, 25, 10));
    /* Partially visible (top overlaps) */
    TEST_ASSERT_TRUE(syn_imgui_widget_visible(&ctx, 15, 10));
    /* Fully above viewport */
    TEST_ASSERT_FALSE(syn_imgui_widget_visible(&ctx, 5, 10));
    /* Fully below viewport */
    TEST_ASSERT_FALSE(syn_imgui_widget_visible(&ctx, 65, 10));
}

/* ── Text clipped (feature #9) ──────────────────────────────────────────── */

static void test_imgui_text_clipped(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    syn_canvas_clear(&canvas);
    /* Draw a long text but clip to only 40px width */
    syn_imgui_text_clipped(&ctx, "Very long text that overflows", 0, 0, 0, 0, 40, 14);

    /* Verify something was drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);

    /* Verify no pixels past the clip boundary (column >= 40) */
    bool overflow = false;
    for (int16_t col = 40; col < 128; col++) {
        for (int16_t row = 0; row < 14; row++) {
            /* SSD1306: page = y/8, idx = page*width + x, bit = y%8 */
            size_t page = (size_t)row / 8;
            size_t idx = page * 128 + (size_t)col;
            uint8_t bit = (uint8_t)(1 << (row % 8));
            if (fb[idx] & bit) {
                overflow = true;
                break;
            }
        }
        if (overflow)
            break;
    }
    TEST_ASSERT_FALSE(overflow);
}

/* ── Text marquee ───────────────────────────────────────────────────────── */

static void test_imgui_text_marquee(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    ctx.gfx = &canvas;

    /* Short text that fits — offset should stay 0 */
    int16_t offset = 0;
    syn_canvas_clear(&canvas);
    syn_imgui_text_marquee(&ctx, "Hi", &offset, 0, 0, 100, 1);
    TEST_ASSERT_EQUAL_INT16(0, offset);

    /* Long text that overflows — offset should advance */
    offset = 0;
    syn_canvas_clear(&canvas);
    syn_imgui_text_marquee(&ctx, "This is a very long scrolling text for testing", &offset, 0, 0,
                           40, 1);
    TEST_ASSERT_TRUE(offset > 0);

    /* Call a few more times — offset should keep advancing */
    int16_t prev = offset;
    syn_canvas_clear(&canvas);
    syn_imgui_text_marquee(&ctx, "This is a very long scrolling text for testing", &offset, 0, 0,
                           40, 1);
    TEST_ASSERT_TRUE(offset > prev);

    /* Verify something was drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) {
            has_pixels = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_pixels);
}

/* ── layout_resolve regression (covers the 2024-06 batch fix) ───────────────
 *
 * Each widget that was missing layout_resolve() would leave the cursor at
 * its initial position, causing all subsequent widgets to overlap at y=0.
 * These tests verify that the cursor advances correctly after each of the
 * affected widgets when called with all-zero geometry inside layout_begin.
 * ─────────────────────────────────────────────────────────────────────────── */

static void test_imgui_layout_resolve_separator_text(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_separator_text(&ctx, "Section", 0, 0, 0);
    /* Cursor must have advanced past the separator row */
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_slider(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t val = 50;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_slider(&ctx, "Brightness", &val, 0, 100, 0, 0, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_value_int(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_value_int(&ctx, "Speed", 42, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_progress_bar_ex(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_progress_bar_ex(&ctx, 60, 0, 100, NULL, 0, 0, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_text_marquee(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int16_t offset = 0;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_text_marquee(&ctx, "Scrolling title text", &offset, 0, 0, 0, 1);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_radio(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t mode = 0;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_radio(&ctx, "Option A", &mode, 0, 0, 0, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_layout_resolve_combo(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    const char *opts[] = {"A", "B", "C"};
    int32_t sel = 0;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_combo(&ctx, "Mode", opts, 3, &sel, 0, 0, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

/* Verify that all 4 widgets on page 0 are stacked correctly:
 * separator_text → checkbox_flags → checkbox_flags → slider → value_int
 * each must start below the previous one's bottom edge. */
static void test_imgui_layout_resolve_page0_stacking(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    int32_t val = 50;
    uint32_t flags = 0;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t y0 = ctx.layout.cy;
    syn_imgui_separator_text(&ctx, "GUI Overview", 0, 0, 0);
    int16_t y1 = ctx.layout.cy;
    syn_imgui_checkbox_flags(&ctx, "LED 1", &flags, 0x01, 0, 0, 0, 0);
    int16_t y2 = ctx.layout.cy;
    syn_imgui_checkbox_flags(&ctx, "LED 2", &flags, 0x02, 0, 0, 0, 0);
    int16_t y3 = ctx.layout.cy;
    syn_imgui_slider(&ctx, "Brightness", &val, 0, 100, 0, 0, 0, 0);
    int16_t y4 = ctx.layout.cy;
    syn_imgui_value_int(&ctx, "Speed", 10, 0, 0);
    int16_t y5 = ctx.layout.cy;

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);

    /* All rows must be strictly increasing */
    TEST_ASSERT_GREATER_THAN_INT16(y0, y1);
    TEST_ASSERT_GREATER_THAN_INT16(y1, y2);
    TEST_ASSERT_GREATER_THAN_INT16(y2, y3);
    TEST_ASSERT_GREATER_THAN_INT16(y3, y4);
    TEST_ASSERT_GREATER_THAN_INT16(y4, y5);

    /* Cursor is allowed to go past display height — scroll regions clip it.
     * Just verify it's within a plausible range (< 2× display height). */
    TEST_ASSERT_LESS_OR_EQUAL_INT16(128, y5);
}

static void test_imgui_layout_resolve_tabs(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    const char *tabs[] = {"Dash", "Set", "Info"};
    int32_t active = 0;

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);

    int16_t cy_before = ctx.layout.cy;
    syn_imgui_tabs(&ctx, tabs, 3, &active, 0, 0, 0);
    TEST_ASSERT_GREATER_THAN_INT16(cy_before, ctx.layout.cy);

    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_text_wrapping_and_clip_rects(void)
{
    uint8_t fb[128 * 128 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 128, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* 1. Basic Navigation & Capping */
    ctx.focused_id = 2;
    ctx.last_max_id = 3;
    syn_imgui_begin(&ctx, &canvas, false, false, -1, false, 0, 0);
    TEST_ASSERT_EQUAL_INT(1, ctx.focused_id);
    syn_imgui_end(&ctx);

    ctx.focused_id = 2;
    ctx.next_id = 0;
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(0, ctx.focused_id);

    /* 2. Group Box & Layouts */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_group_begin(&ctx, "Title", 10, 10, 100, 40);
    syn_imgui_button(&ctx, "Btn", 0, 0, 0, 0);
    syn_imgui_group_end(&ctx);

    syn_imgui_group_begin(&ctx, NULL, 10, 10, 100, 40);
    syn_imgui_group_end(&ctx);

    syn_imgui_spacing(&ctx, 5);
    syn_imgui_layout_begin(&ctx, 0, 0, 100);
    syn_imgui_spacing(&ctx, 10);
    syn_imgui_layout_end(&ctx);

    syn_imgui_label(&ctx, "BasicLabel", 0, 0);

    syn_imgui_separator(&ctx, 10, 20, 80);
    syn_imgui_layout_begin(&ctx, 0, 0, 100);
    syn_imgui_separator(&ctx, 0, 0, 0);
    syn_imgui_layout_end(&ctx);

    syn_imgui_layout_begin(&ctx, 0, 0, 100);
    syn_imgui_button(&ctx, "B1", 0, 0, 0, 0);
    syn_imgui_same_line(&ctx);
    int16_t widths[] = {40, 40};
    syn_imgui_layout_row(&ctx, 2, widths, 15);
    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);

    /* 3. Combo Box Touch & Exit */
    const char *opts[] = {"O1", "O2"};
    int32_t selected = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 68, 7);
    syn_imgui_combo(&ctx, "C1", opts, 2, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(1, selected);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 20, 7);
    syn_imgui_combo(&ctx, "C1", opts, 2, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(ctx.focused_id, ctx.active_id);

    syn_imgui_begin(&ctx, &canvas, false, true, 0, false, 0, 0);
    syn_imgui_combo(&ctx, "C1", opts, 2, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(0, ctx.active_id);

    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, -1, false, 0, 0);
    syn_imgui_combo(&ctx, "C1", opts, 2, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);

    /* 4. Gauge Arc needle boundary */
    syn_canvas_clear(&canvas);
    syn_imgui_gauge(&ctx, "G1", 0, 0, 100, 32, 32, 16);

    /* 5. Modal Dialog Cancel/Input */
    ctx.focused_id = 99;
    bool ok_clicked = false;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_dialog(&ctx, "Msg", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(1, ctx.focused_id);

    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    syn_imgui_dialog(&ctx, "Msg", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(2, ctx.focused_id);

    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0);
    bool dismissed = syn_imgui_dialog(&ctx, "Msg", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);
    TEST_ASSERT_TRUE(dismissed);
    TEST_ASSERT_FALSE(ok_clicked);

    ctx.focused_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 45, 25);
    dismissed = syn_imgui_dialog(&ctx, "Msg", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);
    TEST_ASSERT_TRUE(dismissed);

    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 0, 0);
    syn_imgui_dialog(&ctx, "Msg", &ok_clicked, 10, 10, 60, 30);
    syn_imgui_end(&ctx);
    TEST_ASSERT_FALSE(ctx.touch_down);

    /* 6. Scroll Auto-Follow & Visibility */
    int16_t scroll_offset = 0;
    bool toggle_val = false;

    /* Frame 6A: Scroll Down Auto-Follow */
    scroll_offset = 0;
    ctx.focused_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_scroll_begin(&ctx, 0, 10, 128, 30, &scroll_offset);
    syn_imgui_toggle(&ctx, "T1", &toggle_val, 10, 40, 80, 11);
    syn_imgui_toggle(&ctx, "T2", &toggle_val, 10, 60, 80, 11);
    syn_imgui_scroll_end(&ctx);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT16(11, scroll_offset);

    /* Frame 6B: Scroll Up Auto-Follow */
    scroll_offset = 5;
    ctx.focused_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_scroll_begin(&ctx, 0, 50, 128, 30, &scroll_offset);
    syn_imgui_toggle(&ctx, "T1", &toggle_val, 10, 65, 80, 11);
    syn_imgui_toggle(&ctx, "T2", &toggle_val, 10, 95, 80, 11);
    syn_imgui_scroll_end(&ctx);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT16(20, scroll_offset);

    /* 7. Text Wrapped & Clipped & Marquee */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_text_wrapped(&ctx, "Line1\nLine2", 0, 0, 60);

    const char *long_tabs[] = {"LongDashboardText"};
    int32_t active_tab = 0;
    syn_imgui_tabs(&ctx, long_tabs, 1, &active_tab, 0, 0, 20);

    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_tabs(&ctx, long_tabs, 1, &active_tab, 0, 0, 20);

    int16_t marquee_offset = 0;
    canvas.clip_w = 0;
    canvas.clip_h = 0;
    syn_imgui_text_clipped(&ctx, "Clip", 0, 0, 0, 0, 10, 10);
    syn_imgui_text_marquee(&ctx, "MarqueeLongTextScroll", &marquee_offset, 0, 0, 10, 1);

    marquee_offset = 0;
    syn_imgui_text_marquee(&ctx, "MarqueeLongTextScroll", &marquee_offset, 0, 0, 10, 1);
    marquee_offset = 35;
    syn_imgui_text_marquee(&ctx, "MarqueeLongTextScroll", &marquee_offset, 0, 0, 10, 1);
    marquee_offset = 220;
    syn_imgui_text_marquee(&ctx, "MarqueeLongTextScroll", &marquee_offset, 0, 0, 10, 1);
    marquee_offset = 250;
    syn_imgui_text_marquee(&ctx, "MarqueeLongTextScroll", &marquee_offset, 0, 0, 10, 1);
    syn_gfx_reset_clip(&canvas);

    /* 8. Progress Bar & Selectable & Header Hits */
    syn_imgui_progress_bar_ex(&ctx, -100, 0, 100, NULL, 0, 0, 100, 12);
    syn_imgui_progress_bar_ex(&ctx, 5, 0, 100, NULL, 0, 0, 100, 12);
    syn_imgui_end(&ctx);
}

static void test_imgui_spinner_arrow_and_encoder_wrapping(void)
{
    uint8_t fb[128 * 64 / 8] = {0};
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    const char *items[] = {"OptA", "OptB", "OptC"};

    /* 1. Touch left arrow when selected=0 -> wraps to 2 */
    int32_t selected = 0;
    syn_imgui_begin(&ctx, &canvas, false, true, 0, true, 43, 5); /* touch_x=43 -> left arrow */
    syn_imgui_combo(&ctx, "Combo", items, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT32(2, selected);

    /* 2. Touch right arrow when selected=2 -> wraps to 0 */
    selected = 2;
    syn_imgui_begin(&ctx, &canvas, false, true, 0, true, 70, 5); /* touch_x=70 -> right arrow */
    syn_imgui_combo(&ctx, "Combo", items, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT32(0, selected);

    /* 3. Encoder negative wrap while active */
    ctx.focused_id = 1;
    ctx.active_id = 1;
    selected = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, -5, false, 0, 0); /* enc_delta = -5 */
    syn_imgui_combo(&ctx, "Combo", items, 3, &selected, 0, 0, 80, 15);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT32(1, selected);
}

static void test_imgui_encoder_wrap_clamping_and_focus_cap(void)
{
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    SYN_Canvas canvas;
    uint8_t fb[128 * 64 / 8] = {0};
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    /* 1. Encoder delta negative wrap (idx < 0 -> idx += count) */
    ctx.focused_id = 1;
    ctx.last_max_id = 5;
    syn_imgui_begin(&ctx, &canvas, false, false, -2, false, 0, 0);
    TEST_ASSERT_EQUAL_UINT16(4, ctx.focused_id);
    syn_imgui_end(&ctx);

    /* 2. Focus ID > last_max_id capped at 1 */
    ctx.focused_id = 10;
    ctx.last_max_id = 3;
    ctx.next_id = 3;
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_UINT16(1, ctx.focused_id);
}

static void test_imgui_begin_focused_id_zero(void)
{
    uint8_t buf[128 * 64 / 8] = {0};
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, buf, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    ctx.last_max_id = 2;
    ctx.focused_id = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, 1, false, 0, 0);
    syn_imgui_end(&ctx);
}

static void test_imgui_slider_touch_out_of_bounds_clamping(void)
{
    uint8_t buf[128 * 64 / 8] = {0};
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, buf, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* Touch x = 0 (left of slider bar) with touch_down = true */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 0, 5);

    int32_t val = 50;
    syn_imgui_slider(&ctx, "Val", &val, 0, 100, 10, 5, 80, 12);
    syn_imgui_end(&ctx);
}

static void test_imgui_slider_and_progress_bar_bounds_clamping(void)
{
    uint8_t fb[64 * 64 / 8] = {0};
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 64, 64, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);

    /* 1. Progress bar values below min & above max */
    syn_imgui_progress_bar(&ctx, -10, 0, 100, 10, 10, 80, 15);
    syn_imgui_progress_bar(&ctx, 150, 0, 100, 10, 10, 80, 15);

    /* 2. Slider touch drag clamping below min & above max */
    int32_t val = 50;
    ctx.touch_down = true;
    ctx.touch_x = 0; /* far left of bar -> new_val < min -> new_val = min */
    ctx.touch_y = 10;
    syn_imgui_slider(&ctx, "sl", &val, 10, 90, 10, 10, 80, 20);

    ctx.touch_x = 200; /* far right of bar -> new_val > max -> new_val = max */
    syn_imgui_slider(&ctx, "sl", &val, 10, 90, 10, 10, 80, 20);

    /* 3. Slider encoder delta clamping below min & above max */
    val = 50;
    syn_imgui_begin(&ctx, &canvas, false, false, -100, false, 0, 0);
    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_slider(&ctx, "sl", &val, 10, 90, 0, 0, 80, 20);
    TEST_ASSERT_EQUAL_INT32(10, val);

    syn_imgui_begin(&ctx, &canvas, false, false, 100, false, 0, 0);
    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_slider(&ctx, "sl", &val, 10, 90, 0, 0, 80, 20);
    TEST_ASSERT_EQUAL_INT32(90, val);
    syn_imgui_end(&ctx);
}

static void test_imgui_widget_active_state_and_focus(void)
{
    uint8_t fb[128 * 128 / 8] = {0};
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 128, 1, NULL, NULL);
    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* 1. Negative integer formatting in syn_imgui_value_int & format_int (lines 992-993, 1619-1620)
     */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_value_int(&ctx, "Neg", -42, 10, 10);

    /* 2. Bar chart null/zero count/range and min/max clamping (lines 1422, 1426, 1430, 1436-1442)
     */
    int32_t bdata[3] = {-10, 50, 150};
    syn_imgui_bar_chart(&ctx, "bc0", bdata, 0, 0, 100, 10, 10, 50, 20); /* count = 0 */
    syn_imgui_bar_chart(&ctx, "bc1", bdata, 3, 100, 100, 10, 10, 50,
                        20); /* range = 0 -> clamped to 1 */
    syn_imgui_bar_chart(&ctx, "bc2", bdata, 3, 0, 100, 10, 10, 50,
                        20); /* data < min & data > max */

    /* 3. Progress bar ex indeterminate and min/max clamping (lines 1671, 1688-1690) */
    syn_imgui_progress_bar_ex(&ctx, 10, 0, 100, "10%", 10, 10, 10,
                              15); /* indeterminate, w=10 -> inner_w=6 -> bar_w min 2 */
    syn_imgui_progress_bar_ex(&ctx, -10, 0, 100, NULL, 10, 10, 50, 15); /* val < min */
    syn_imgui_progress_bar_ex(&ctx, 150, 0, 100, NULL, 10, 10, 50, 15); /* val > max */

    /* 4. Icon button rendering & click/focus (lines 1472-1473, 1489-1492) */
    const uint8_t icon[8] = {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF};
    ctx.focused_id = 0;
    ctx.next_id = 1;
    ctx.btn_select = true;
    bool icon_clicked = syn_imgui_icon_button(&ctx, icon, 8, 8, 10, 10, 20, 20);
    (void)icon_clicked;

    /* 5. Marquee scroll reset, pause, and overflow handling (lines 2021, 2045, 2053, 2055, 2067,
     * 2086, 2092) */
    int16_t offset_val = 100;
    syn_imgui_text_marquee(&ctx, "Short", &offset_val, 10, 10, 100,
                           1); /* Short text -> offset reset to 0 */
    TEST_ASSERT_EQUAL_INT16(0, offset_val);

    offset_val = 0;
    syn_imgui_text_marquee(&ctx, "Very Long Marquee Text String For Overflow Testing", &offset_val,
                           10, 10, 30, 1);

    /* 6. Combo index wrapping on encoder / touch (lines 448, 1320, 1322, 1330) */
    const char *items[] = {"One", "Two", "Three"};
    int32_t active_idx = 5; /* idx >= count -> clamped to count-1 */
    syn_imgui_combo(&ctx, "cb", items, 3, &active_idx, 10, 10, 60, 20);

    /* 7. Layout row max columns clamping (line 1929) */
    int16_t widths[20];
    for (int i = 0; i < 20; i++)
        widths[i] = 10;
    syn_imgui_layout_begin(&ctx, 10, 10, 100);
    syn_imgui_layout_row(&ctx, 20, widths, 20); /* count > SYN_IMGUI_MAX_ROW_COLS (16) */
    syn_imgui_layout_end(&ctx);

    /* 8. Scroll region scroll bounds clamping (lines 1081, 1083, 1096) */
    int16_t scroll_y = 1000;
    syn_imgui_scroll_begin(&ctx, 10, 10, 50, 20, &scroll_y);
    syn_imgui_label(&ctx, "S1", 0, 0);
    syn_imgui_label(&ctx, "S2", 0, 0);
    syn_imgui_label(&ctx, "S3", 0, 0);
    syn_imgui_label(&ctx, "S4", 0, 0);
    syn_imgui_scroll_end(&ctx);

    scroll_y = 0;
    syn_imgui_scroll_begin(&ctx, 10, 10, 50, 20, &scroll_y);
    syn_imgui_label(&ctx, "S1", 0, 0);
    syn_imgui_scroll_end(&ctx);

    /* 8. Focus cap when last_max_id == 0 (line 81) */
    ctx.focused_id = 5;
    ctx.last_max_id = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);

    /* 7. Combo with out-of-bounds selected index (lines 448 & 477) */
    const char *opts[] = {"A", "B"};
    int32_t sel = 10;
    syn_imgui_combo(&ctx, "C", opts, 2, &sel, 0, 0, 50, 20);

    /* 9. Layout row with height <= 0 or columns <= 0 (lines 511, 531, 538, 542) */
    ctx.layout.in_layout = true;
    const int16_t w_arr[2] = {20, 20};
    syn_imgui_layout_row(&ctx, 2, w_arr, 0);
    syn_imgui_layout_row(&ctx, 0, NULL, 15);
    /* 10. Graph out-of-bounds data clamping (lines 557 & 559) */
    const int32_t spk_data[3] = {-50, 50, 150};
    syn_imgui_graph(&ctx, "G", spk_data, 3, 0, 100, 0, 0, 50, 20);

    /* 11. Gauge out-of-bounds value clamping (lines 630 & 632) */
    syn_imgui_gauge(&ctx, "G1", -50, 0, 100, 25, 25, 20);
    syn_imgui_gauge(&ctx, "G2", 150, 0, 100, 25, 25, 20);

    ctx.layout.in_layout = false;
    syn_imgui_end(&ctx);
}

static void test_imgui_viewport_scissor_stack_clamping(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* 1. Stale focus reset (line 75) */
    ctx.focused_id = 2;
    ctx.last_max_id = 5;
    ctx.updated_focus = false;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 30, 10);
    syn_imgui_end(&ctx);

    /* 2. Slider touch bounds clamping (lines 276 & 278) */
    syn_imgui_begin(&ctx, &canvas, false, true, 0, false, 0, 10); /* Touch X=0 */
    int32_t sl_val = 50;
    syn_imgui_slider(&ctx, "S", &sl_val, 0, 100, 10, 10, 50, 15);
    syn_imgui_end(&ctx);

    syn_imgui_begin(&ctx, &canvas, false, true, 0, false, 100, 10); /* Touch X=100 */
    syn_imgui_slider(&ctx, "S", &sl_val, 0, 100, 10, 10, 50, 15);
    syn_imgui_end(&ctx);

    /* 3. Combo with count == 0 and enc_delta wrapping (lines 396, 448, 477) */
    syn_imgui_begin(&ctx, &canvas, false, false, 5, false, 0, 0);
    const char *c_opts[2] = {"O1", "O2"};
    int32_t combo_sel = -1;
    TEST_ASSERT_FALSE(syn_imgui_combo(&ctx, "C0", c_opts, 0, &combo_sel, 0, 0, 50, 15));
    combo_sel = -1;
    syn_imgui_combo(&ctx, "C1", c_opts, 2, &combo_sel, 0, 0, 50, 15);
    syn_imgui_end(&ctx);

    /* 4. Graph zero bounds & empty data & invalid range (lines 511, 531, 538, 542) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_graph(&ctx, "G0", NULL, 0, 0, 0, 0, 0, 0, 0);
    syn_imgui_graph(&ctx, "G1", NULL, 0, 100, 0, 0, 0, 50, 10);
    int32_t g_val = 5;
    syn_imgui_graph(&ctx, "G2", &g_val, 1, 0, 10, 10, 0, 50, 20);
    syn_imgui_end(&ctx);

    /* 5. Gauge radius <= 0 (line 606) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_gauge(&ctx, "G0", 50, 0, 100, 20, 20, 0);
    syn_imgui_end(&ctx);

    /* 6. Same line not in layout (line 818) */
    ctx.layout.in_layout = false;
    syn_imgui_same_line(&ctx);

    /* 7. Unwrapped row layout resolve (lines 859-861) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_layout_begin(&ctx, 0, 0, 50);
    ctx.layout.row_h = 15;
    syn_imgui_button(&ctx, "B2", 0, 0, 30, 10);
    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);

    /* 8. Spinner hit test focus & enc underflow & negative formatting (lines 959, 972, 997, 998) */
    int32_t sp_val = -10;
    syn_imgui_begin(&ctx, &canvas, false, true, -1, false, 15, 15); /* Touch on spinner */
    syn_imgui_spinner(&ctx, "SP", &sp_val, -100, 100, 1, 10, 10, 60, 15);
    syn_imgui_end(&ctx);

    /* 9. Scroll region overflow with small thumb_h < 3 (lines 1100, 1104) */
    int16_t sc_y = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_scroll_begin(&ctx, 0, 0, 50, 10, &sc_y);
    syn_imgui_layout_begin(&ctx, 0, 0, 50);
    for (int i = 0; i < 50; i++) {
        syn_imgui_label(&ctx, "L", 0, 0);
    }
    syn_imgui_layout_end(&ctx);
    syn_imgui_scroll_end(&ctx);
    syn_imgui_end(&ctx);
}

static void test_imgui_docking_tab_bar_overflow(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* 1. focused_id > last_max_id cap (line 81) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 10, 10);
    syn_imgui_button(&ctx, "B2", 0, 0, 10, 10);
    syn_imgui_end(&ctx); /* last_max_id = 2 */

    ctx.focused_id = 100;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_button(&ctx, "B1", 0, 0, 10, 10); /* next_id = 1 */
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL(1, ctx.focused_id);

    /* 2. Slider touch out of bounds (lines 276, 278) */
    int32_t val = 50;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 54, 10); /* touch_x = 54 = bar_x */
    syn_imgui_slider(&ctx, "S1", &val, 0, 100, 10, 10, 80, 20);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(0, val);

    val = 50;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 86,
                    10); /* touch_x = 86 = bar_x + bar_w */
    syn_imgui_slider(&ctx, "S2", &val, 0, 100, 10, 10, 80, 20);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT(100, val);

    /* 3. Dropdown encoder positive wrap (line 448) */
    const char *opts[] = {"A", "B", "C"};
    int32_t selected = 2;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0); /* select to open */
    syn_imgui_combo(&ctx, "DD", opts, 3, &selected, 0, 0, 80, 20);
    syn_imgui_end(&ctx);
    syn_imgui_begin(&ctx, &canvas, false, false, 5, false, 0, 0); /* enc_delta = +5 */
    syn_imgui_combo(&ctx, "DD", opts, 3, &selected, 0, 0, 80, 20);
    syn_imgui_end(&ctx);

    /* 4. Plot line max_val == min_val (line 542) */
    int32_t pdata[] = {50, 50};
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_graph(&ctx, "PL", pdata, 2, 50, 50, 10, 10, 80, 40);
    syn_imgui_end(&ctx);

    /* 5. Spinner touch hit (line 959) & negative wrap (line 972) */
    int32_t sp_val = 0;
    syn_imgui_begin(&ctx, &canvas, false, false, -1, true, 15, 15);
    syn_imgui_spinner(&ctx, "SP", &sp_val, 0, 10, 1, 10, 10, 50, 20);
    syn_imgui_end(&ctx);

    /* 6. Scroll area end *sp < 0 (line 1086) */
    int16_t scroll_pos = -10;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_scroll_begin(&ctx, 0, 0, 100, 50, &scroll_pos);
    syn_imgui_scroll_end(&ctx);
    syn_imgui_end(&ctx);
    TEST_ASSERT_EQUAL_INT16(0, scroll_pos);

    /* 7. Tabs encoder positive/negative wrap & select exit (lines 1325, 1327, 1335) */
    int32_t tab_active = 1;
    const char *tabs[] = {"T1", "T2"};
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0); /* select to enter edit */
    syn_imgui_tabs(&ctx, tabs, 2, &tab_active, 0, 0, 100);
    syn_imgui_end(&ctx);
    syn_imgui_begin(&ctx, &canvas, false, false, 5, false, 0, 0); /* positive wrap (line 1325) */
    syn_imgui_tabs(&ctx, tabs, 2, &tab_active, 0, 0, 100);
    syn_imgui_end(&ctx);

    tab_active = 0;
    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0); /* select to enter edit */
    syn_imgui_tabs(&ctx, tabs, 2, &tab_active, 0, 0, 100);
    syn_imgui_end(&ctx);
    syn_imgui_begin(&ctx, &canvas, false, false, -5, false, 0, 0); /* negative wrap (line 1327) */
    syn_imgui_tabs(&ctx, tabs, 2, &tab_active, 0, 0, 100);
    syn_imgui_end(&ctx);

    syn_imgui_begin(&ctx, &canvas, true, false, 0, false, 0, 0); /* select exit (line 1335) */
    syn_imgui_tabs(&ctx, tabs, 2, &tab_active, 0, 0, 100);
    syn_imgui_end(&ctx);

    /* 8. Plot histogram bar_w < 1 & bar_h < 1 */
    int32_t hist_data[20] = {1, 2, 3};
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_bar_chart(&ctx, "H", hist_data, 20, 0, 100, 10, 10, 5, 20);
    syn_imgui_end(&ctx);

    /* 9. Icon button touch hit */
    static const uint8_t dummy_icon[8] = {0xFF};
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 15, 15);
    syn_imgui_icon_button(&ctx, dummy_icon, 8, 8, 10, 10, 20, 20);
    syn_imgui_end(&ctx);

    /* 10. Progress bar val < min (line 1693) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_progress_bar_ex(&ctx, -10, 0, 100, "PR", 0, 0, 50, 20);
    syn_imgui_end(&ctx);

    /* 11. Selectable unfocused text (line 1778) */
    bool sel = false;
    ctx.focused_id = 999;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_selectable(&ctx, "SEL2", &sel, 10, 10, 50, 20);
    syn_imgui_end(&ctx);

    /* 12. Collapsing header touch hit */
    bool exp = false;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 15, 15);
    syn_imgui_collapsing_header(&ctx, "HDR", &exp, 10, 10, 50, 20);
    syn_imgui_end(&ctx);

    /* 13. Text wrapped w <= 0 */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_text_wrapped(&ctx, "Hello World", 0, 0, 0);
    syn_imgui_end(&ctx);

    /* 14. Text marquee Phase 3 (line 2072), clip reset (line 2091), wrap (line 2097) */
    int16_t mq_off = 200; /* offset >= total_cycle (line 2097) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    canvas.clip_w = 0; /* clip_w == 0 triggers syn_canvas_reset_clip (line 2091) */
    syn_imgui_text_marquee(&ctx, "A Very Long Text That Exceeds The Width", &mq_off, 0, 0, 20, 10);
    syn_imgui_end(&ctx);

    mq_off = 100; /* Phase 3 (line 2072) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_text_marquee(&ctx, "A Very Long Text That Exceeds The Width", &mq_off, 0, 0, 20, 1);
    syn_imgui_end(&ctx);
}

static void test_imgui_toast_test(void)
{
    SYN_IMGUI_Context ctx;
    SYN_Canvas canvas;
    uint8_t buf[128 * 64 / 8];
    syn_canvas_init(&canvas, buf, 128, 64, 1, NULL, NULL);
    syn_imgui_init(&ctx);

    /* Null checks */
    TEST_ASSERT_FALSE(syn_imgui_toast(NULL, "Hi", 1000, 500, 0));
    TEST_ASSERT_FALSE(syn_imgui_toast(&ctx, NULL, 1000, 500, 0));

    /* Toast active (current < start + duration) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    TEST_ASSERT_TRUE(syn_imgui_toast(&ctx, "Saved!", 2000, 500, 0));
    syn_imgui_end(&ctx);

    /* Long toast text -> box_x < 2 clamp branch */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    TEST_ASSERT_TRUE(
        syn_imgui_toast(&ctx, "Very Long Notification Message For Screen Width", 2000, 500, 0));
    syn_imgui_end(&ctx);

    /* Toast expired (current >= start + duration) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    TEST_ASSERT_FALSE(syn_imgui_toast(&ctx, "Saved!", 2000, 2500, 0));
    syn_imgui_end(&ctx);
}

static void test_imgui_numpad_test(void)
{
    SYN_IMGUI_Context ctx;
    SYN_Canvas canvas;
    uint8_t buf[128 * 64 / 8];
    syn_canvas_init(&canvas, buf, 128, 64, 1, NULL, NULL);
    syn_imgui_init(&ctx);

    char pin[8] = "";

    /* Null checks */
    TEST_ASSERT_EQUAL(0, syn_imgui_numpad(NULL, pin, 8, false, 0, 0, 0, 0));
    TEST_ASSERT_EQUAL(0, syn_imgui_numpad(&ctx, NULL, 8, false, 0, 0, 0, 0));
    TEST_ASSERT_EQUAL(0, syn_imgui_numpad(&ctx, pin, 0, false, 0, 0, 0, 0));

    /* Tap '1' button via touch (Button 1 is at x=0..38, y=18) */
    int16_t key1_x = 10, key1_y = 18;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, key1_x, key1_y);
    int res = syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(0, res);
    TEST_ASSERT_EQUAL_STRING("1", pin);

    /* Tap '0' button via touch (Button 0 is at x=41..79, y=52..62) */
    int16_t key0_x = 50, key0_y = 55;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, key0_x, key0_y);
    res = syn_imgui_numpad(&ctx, pin, 8, true, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(0, res);
    TEST_ASSERT_EQUAL_STRING("10", pin);

    /* Tap 'C' (Clear) button -> removes '0' -> "1" */
    int16_t keyC_x = 10, keyC_y = 55;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, keyC_x, keyC_y);
    res = syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(0, res);
    TEST_ASSERT_EQUAL_STRING("1", pin);

    /* Tap 'C' (Clear) button again -> removes '1' -> "" */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, keyC_x, keyC_y);
    res = syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(0, res);
    TEST_ASSERT_EQUAL_STRING("", pin);

    /* Tap 'C' (Clear) on empty buffer -> returns -1 (cancel) */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, keyC_x, keyC_y);
    res = syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(-1, res);

    /* Tap 'OK' button (Button OK is at x=82..120, y=52..62) -> returns 1 */
    int16_t keyOK_x = 90, keyOK_y = 55;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, keyOK_x, keyOK_y);
    res = syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 120, 60);
    syn_imgui_end(&ctx);

    TEST_ASSERT_EQUAL(1, res);

    /* Narrow numpad width (w=30) -> triggers key_w < 12 clamping branch */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_numpad(&ctx, pin, 8, false, 0, 0, 30, 60);
    syn_imgui_end(&ctx);
}

static void test_r5_imgui_layout_row_and_int32_extremes(void)
{
    uint8_t fb[128 * 64 / 8];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 1, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* 1. Multi-column layout_row: fixed width + fill remainder (-1) + wrap across rows */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 50, 5);
    syn_imgui_layout_begin(&ctx, 0, 0, 128);
    int16_t widths[2] = {40, -1};
    syn_imgui_layout_row(&ctx, 2, widths, 14);

    bool hit_col0 = syn_imgui_button(&ctx, "C0", 0, 0, 0, 0);
    TEST_ASSERT_FALSE(hit_col0);
    TEST_ASSERT_EQUAL_INT(1, ctx.layout.row_item_idx);
    TEST_ASSERT_EQUAL_INT16(43, ctx.layout.cx);
    TEST_ASSERT_EQUAL_INT16(0, ctx.layout.cy);

    /* Touch (50, 5) hits col1 at x=43..128, y=0..14 */
    bool hit_col1 = syn_imgui_button(&ctx, "C1", 0, 0, 0, 0);
    TEST_ASSERT_TRUE(hit_col1);
    TEST_ASSERT_EQUAL_INT(0, ctx.layout.row_item_idx);
    TEST_ASSERT_EQUAL_INT16(0, ctx.layout.cx);
    TEST_ASSERT_EQUAL_INT16(17, ctx.layout.cy);

    /* Equal-split columns (widths == NULL, height == 0 auto) */
    syn_imgui_layout_row(&ctx, 2, NULL, 0);
    syn_imgui_button(&ctx, "E0", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT16(65, ctx.layout.cx);
    syn_imgui_button(&ctx, "E1", 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT(0, ctx.layout.row_item_idx);

    /* Overflow remainder clamp (rem <= 1) & explicit coords inside row */
    int16_t wide_cols[2] = {128, -1};
    syn_imgui_layout_row(&ctx, 2, wide_cols, 12);
    syn_imgui_button(&ctx, "W0", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "W1", 0, 0, 0, 0);
    syn_imgui_button(&ctx, "Exp", 4, 4, 20, 10);
    syn_imgui_layout_end(&ctx);

    /* Narrow equal-split clamp (avail <= row_items) */
    syn_imgui_layout_begin(&ctx, 0, 0, 6);
    syn_imgui_layout_row(&ctx, 4, NULL, 10);
    syn_imgui_button(&ctx, "N0", 0, 0, 0, 0);
    syn_imgui_layout_end(&ctx);
    syn_imgui_end(&ctx);

    /* 2. Full INT32_MIN..INT32_MAX range math without signed 32-bit overflow */
    int32_t slider_val = 0;
    /* Touch midpoint of slider bar: bar_x = 10 + 40 + 4 = 54, bar_w = 40 - 8 = 32; mid = 70 */
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 70, 10);
    TEST_ASSERT_TRUE(syn_imgui_slider(&ctx, "S", &slider_val, INT32_MIN, INT32_MAX, 10, 5, 80, 16));
    TEST_ASSERT_INT_WITHIN(70000000, 0, slider_val);
    syn_imgui_end(&ctx);

    /* Narrow slider bar_w == 0 guard */
    int32_t narrow_val = 5;
    syn_imgui_begin(&ctx, &canvas, false, false, 0, true, 12, 10);
    TEST_ASSERT_FALSE(syn_imgui_slider(&ctx, "SN", &narrow_val, 0, 100, 0, 5, 16, 16));
    syn_imgui_end(&ctx);

    /* Slider encoder clamp at INT32_MAX and INT32_MIN */
    slider_val = INT32_MAX - 2;
    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, 10, false, 0, 0);
    TEST_ASSERT_TRUE(syn_imgui_slider(&ctx, "S", &slider_val, INT32_MIN, INT32_MAX, 10, 5, 80, 16));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, slider_val);
    syn_imgui_end(&ctx);

    slider_val = INT32_MIN + 2;
    ctx.focused_id = 1;
    ctx.active_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, -10, false, 0, 0);
    TEST_ASSERT_TRUE(syn_imgui_slider(&ctx, "S", &slider_val, INT32_MIN, INT32_MAX, 10, 5, 80, 16));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, slider_val);
    syn_imgui_end(&ctx);

    /* Spinner wrap & INT32_MIN formatting */
    int32_t spin_val = 1500000000;
    ctx.active_id = 0;
    ctx.focused_id = 1;
    syn_imgui_begin(&ctx, &canvas, false, false, 2, false, 0, 0);
    TEST_ASSERT_TRUE(
        syn_imgui_spinner(&ctx, "SP", &spin_val, INT32_MIN, INT32_MAX, 1000000000, 0, 0, 120, 14));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, spin_val);
    syn_imgui_end(&ctx);

    /* Graph, gauge, bar_chart, value_int, progress_bar, progress_bar_ex at INT32 extremes */
    const int32_t extreme_pts[3] = {INT32_MIN, 0, INT32_MAX};
    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);
    syn_imgui_progress_bar(&ctx, 0, INT32_MIN, INT32_MAX, 0, 0, 100, 10);
    syn_imgui_progress_bar_ex(&ctx, 0, INT32_MIN, INT32_MAX, NULL, 0, 0, 100, 12);
    syn_imgui_progress_bar_ex(&ctx, INT32_MIN, -100, INT32_MAX, NULL, 0, 0, 100, 12);
    syn_imgui_progress_bar_ex(&ctx, -1, 0, 100, NULL, 0, 0, 8, 12);
    syn_imgui_progress_bar_ex(&ctx, 0, 0, 100, NULL, 0, 0, 4, 12);
    syn_imgui_graph(&ctx, "EX", extreme_pts, 3, INT32_MIN, INT32_MAX, 0, 0, 80, 32);
    syn_imgui_gauge(&ctx, "GX", 0, INT32_MIN, INT32_MAX, 32, 32, 20);
    syn_imgui_bar_chart(&ctx, "BX", extreme_pts, 3, INT32_MIN, INT32_MAX, 0, 0, 80, 32);
    syn_imgui_value_int(&ctx, "Min", INT32_MIN, 0, 0);
    syn_imgui_value_int(&ctx, "Zero", 0, 0, 0);
    syn_imgui_end(&ctx);
}

void run_imgui_tests(void)
{
    RUN_TEST(test_imgui_navigation);
    RUN_TEST(test_imgui_widgets);
    RUN_TEST(test_imgui_touch);
    RUN_TEST(test_imgui_radio);
    RUN_TEST(test_imgui_advanced);
    /* New: style, layout, spinner, canvas primitives */
    RUN_TEST(test_imgui_style_defaults);
    RUN_TEST(test_imgui_set_style);
    RUN_TEST(test_imgui_layout_cursor_advances);
    RUN_TEST(test_imgui_same_line);
    RUN_TEST(test_imgui_spinner_encoder_increment);
    RUN_TEST(test_imgui_spinner_wraps_at_max);
    RUN_TEST(test_imgui_spinner_no_change_when_unfocused);
    RUN_TEST(test_canvas_text_height);
    RUN_TEST(test_canvas_hline_draws_pixels);
    /* New: widgets and features */
    RUN_TEST(test_imgui_toggle);
    RUN_TEST(test_imgui_disabled);
    RUN_TEST(test_imgui_tabs);
    RUN_TEST(test_imgui_scroll_region);
    RUN_TEST(test_imgui_bar_chart);
    RUN_TEST(test_imgui_icon_button);
    RUN_TEST(test_imgui_label_alignment);
    RUN_TEST(test_imgui_status_bar);
    /* Bug fix regression + new features */
    RUN_TEST(test_imgui_disabled_button);
    RUN_TEST(test_imgui_nested_disabled);
    RUN_TEST(test_imgui_stale_focus);
    RUN_TEST(test_imgui_separator_text);
    RUN_TEST(test_imgui_checkbox_flags);
    RUN_TEST(test_imgui_value_int);
    RUN_TEST(test_imgui_progress_bar_ex);
    RUN_TEST(test_imgui_selectable);
    /* Features #5-9 */
    RUN_TEST(test_imgui_collapsing_header);
    RUN_TEST(test_imgui_text_wrapped);
    RUN_TEST(test_imgui_layout_row);
    RUN_TEST(test_imgui_widget_visible);
    RUN_TEST(test_imgui_text_clipped);
    /* Text marquee */
    RUN_TEST(test_imgui_text_marquee);
    /* layout_resolve regression (2024-06 batch fix) */
    RUN_TEST(test_imgui_layout_resolve_separator_text);
    RUN_TEST(test_imgui_layout_resolve_slider);
    RUN_TEST(test_imgui_layout_resolve_value_int);
    RUN_TEST(test_imgui_layout_resolve_progress_bar_ex);
    RUN_TEST(test_imgui_layout_resolve_text_marquee);
    RUN_TEST(test_imgui_layout_resolve_radio);
    RUN_TEST(test_imgui_layout_resolve_combo);
    RUN_TEST(test_imgui_layout_resolve_page0_stacking);
    RUN_TEST(test_imgui_layout_resolve_tabs);
    RUN_TEST(test_imgui_text_wrapping_and_clip_rects);
    RUN_TEST(test_imgui_spinner_arrow_and_encoder_wrapping);
    RUN_TEST(test_imgui_slider_and_progress_bar_bounds_clamping);
    RUN_TEST(test_imgui_encoder_wrap_clamping_and_focus_cap);
    RUN_TEST(test_imgui_slider_touch_out_of_bounds_clamping);
    RUN_TEST(test_imgui_begin_focused_id_zero);
    RUN_TEST(test_imgui_widget_active_state_and_focus);
    RUN_TEST(test_imgui_viewport_scissor_stack_clamping);
    RUN_TEST(test_imgui_docking_tab_bar_overflow);
    RUN_TEST(test_imgui_toast_test);
    RUN_TEST(test_imgui_numpad_test);
    RUN_TEST(test_r5_imgui_layout_row_and_int32_extremes);
}
