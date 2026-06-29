/**
 * @file test_imgui.c
 * @brief Unity tests for the SyntropicOS IMGUI framework.
 */

#include "unity/unity.h"
#include "syntropic/syntropic.h"
#include "syntropic/display/syn_canvas.h"
#include "syntropic/ui/syn_imgui.h"

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
    syn_imgui_button(&ctx, "B1", 0, 0, 40, 15);   /* ID 1 */
    syn_imgui_button(&ctx, "B2", 0, 20, 40, 15);  /* ID 2 */
    syn_imgui_button(&ctx, "B3", 0, 40, 40, 15);  /* ID 3 */
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
    TEST_ASSERT_EQUAL_INT(3, ctx.active_id); /* Remains active */
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
    const char *opts[] = { "A", "B", "C" };
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
    int32_t telemetry[5] = { 10, 50, 90, 30, 70 };
    syn_canvas_clear(&canvas);
    syn_imgui_graph(&ctx, "Telemetry", telemetry, 5, 0, 100, 0, 0, 64, 32);
    /* Basic sanity check: data line drawn */
    TEST_ASSERT_TRUE(fb[0] != 0);

    /* 4. Gauge indicator test */
    syn_canvas_clear(&canvas);
    syn_imgui_gauge(&ctx, "Speed", 75, 0, 100, 32, 32, 16);
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { has_pixels = true; break; }
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
    s.fg      = SYN_COLOR_RED;
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
    TEST_ASSERT_EQUAL_INT(4, ctx.layout.cx);   /* back to origin_x */
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

    syn_imgui_button(&ctx, "A", 0, 0, 0, 0);  /* advances cy to 0+11+3=14 */
    /* same_line: next widget appends to the right, cy stays at 14 */
    syn_imgui_same_line(&ctx);
    int16_t cx_before = ctx.layout.cx;
    (void)cx_before;
    syn_imgui_button(&ctx, "B", 0, 0, 0, 0);  /* on same row as A */

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
    syn_imgui_button(&ctx, "Go", 0, 0, 30, 13);  /* ID 1 — gets focus via enc_delta */
    bool changed = syn_imgui_spinner(&ctx, "V", &val, 0, 100, 1, 40, 0, 80, 13);
    syn_imgui_end(&ctx);

    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_EQUAL_INT32(50, val);  /* unchanged */
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

    const char *tabs[] = { "Dash", "Set", "Info" };
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
    int32_t data[4] = { 10, 50, 80, 30 };
    syn_imgui_bar_chart(&ctx, "Stats", data, 4, 0, 100, 0, 0, 64, 32);

    /* Verify that some pixels were drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { has_pixels = true; break; }
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
    const uint8_t icon[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

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
        if (fb[i] != 0) { has_pixels = true; break; }
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
        if (fb[i] != 0) { has_pixels = true; break; }
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
        if (fb[i] != 0) { has_pixels = true; break; }
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
        if (fb[i] != 0) { has_pixels = true; break; }
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
        if (fb[i] != 0) { has_pixels = true; break; }
    }
    TEST_ASSERT_TRUE(has_pixels);

    /* Indeterminate mode (value < min) */
    syn_canvas_clear(&canvas);
    syn_imgui_progress_bar_ex(&ctx, -10, 0, 100, NULL, 0, 0, 100, 12);
    has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { has_pixels = true; break; }
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
        if (fb[i] != 0) { has_pixels = true; break; }
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
    syn_imgui_text_clipped(&ctx, "Very long text that overflows", 0, 0,
                             0, 0, 40, 14);

    /* Verify something was drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { has_pixels = true; break; }
    }
    TEST_ASSERT_TRUE(has_pixels);

    /* Verify no pixels past the clip boundary (column >= 40) */
    bool overflow = false;
    for (int16_t col = 40; col < 128; col++) {
        for (int16_t row = 0; row < 14; row++) {
            /* SSD1306: page = y/8, idx = page*width + x, bit = y%8 */
            size_t page = (size_t)row / 8;
            size_t idx  = page * 128 + (size_t)col;
            uint8_t bit = (uint8_t)(1 << (row % 8));
            if (fb[idx] & bit) {
                overflow = true;
                break;
            }
        }
        if (overflow) break;
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
    syn_imgui_text_marquee(&ctx, "This is a very long scrolling text for testing", &offset, 0, 0, 40, 1);
    TEST_ASSERT_TRUE(offset > 0);

    /* Call a few more times — offset should keep advancing */
    int16_t prev = offset;
    syn_canvas_clear(&canvas);
    syn_imgui_text_marquee(&ctx, "This is a very long scrolling text for testing", &offset, 0, 0, 40, 1);
    TEST_ASSERT_TRUE(offset > prev);

    /* Verify something was drawn */
    bool has_pixels = false;
    for (size_t i = 0; i < sizeof(fb); i++) {
        if (fb[i] != 0) { has_pixels = true; break; }
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
}

