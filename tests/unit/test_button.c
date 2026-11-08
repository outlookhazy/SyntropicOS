/**
 * @file test_button.c
 * @brief Unity tests for syn_button — full coverage & user behavior testing.
 */

#include "mocks/mock_port.h"
#include "syntropic/input/syn_button.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int btn_press_count = 0;
static int btn_release_count = 0;
static int btn_long_count = 0;
static int btn_repeat_count = 0;

static void btn_on_press(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_press_count++;
}
static void btn_on_release(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_release_count++;
}
static void btn_on_long(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_long_count++;
}
static void btn_on_repeat(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_repeat_count++;
}

static int btn_single_click_count = 0;
static int btn_double_click_count = 0;
static int btn_triple_click_count = 0;
static int btn_multi_click_count = 0;
static int combo_fired_count = 0;

static void btn_on_single_click(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_single_click_count++;
}
static void btn_on_double_click(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_double_click_count++;
}
static void btn_on_triple_click(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_triple_click_count++;
}
static void btn_on_multi_click(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    btn_multi_click_count++;
}
static void combo_on_fire(SYN_Button *b, void *ctx)
{
    (void)b;
    (void)ctx;
    combo_fired_count++;
}

static void reset_counts(void)
{
    btn_press_count = btn_release_count = btn_long_count = btn_repeat_count = 0;
    btn_single_click_count = btn_double_click_count = btn_triple_click_count =
        btn_multi_click_count = 0;
    combo_fired_count = 0;
}

/* ── Original test — ACTIVE_HIGH, preserved ──────────────────────────────── */

static void test_button(void)
{
    mock_tick_ms = 0;
    reset_counts();

    /* Pin 0 — we control it via gpio_states (from mock GPIO port) */
    mock_gpio_states[0] = 0; /* not pressed */

    SYN_Button btn;
    syn_button_init(&btn, 0, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_release(&btn, btn_on_release, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 500, NULL);

    /* No press — should stay idle */
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);
    TEST_ASSERT_FALSE(syn_button_is_pressed(&btn));

    /* Press the button */
    mock_gpio_states[0] = 1;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);

    /* Advance past debounce window */
    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);
    TEST_ASSERT_TRUE(syn_button_is_pressed(&btn));

    /* Hold for long press */
    mock_tick_advance(500);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);

    /* Release */
    mock_gpio_states[0] = 0;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_release_count);
    TEST_ASSERT_FALSE(syn_button_is_pressed(&btn));

    /* Bounce rejection: press then release before debounce */
    btn_press_count = 0;
    mock_gpio_states[0] = 1;
    mock_tick_advance(10);
    syn_button_update(&btn);
    mock_gpio_states[0] = 0; /* bounce off */
    mock_tick_advance(10);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);

    /* Event polling */
    btn.events = 0;
    mock_gpio_states[0] = 1;
    syn_button_update(&btn);
    mock_tick_advance(60);
    syn_button_update(&btn);
    uint8_t evts = syn_button_poll_events(&btn);
    TEST_ASSERT_TRUE(evts & SYN_BUTTON_EVT_PRESS);
    TEST_ASSERT_EQUAL_INT(0, syn_button_poll_events(&btn));
}

/* ── Test: ACTIVE_LOW polarity ───────────────────────────────────────────── */

static void test_button_active_low(void)
{
    mock_tick_ms = 0;
    reset_counts();

    /* Pin 1 — ACTIVE_LOW: GPIO_LOW = pressed */
    mock_gpio_states[1] = SYN_GPIO_HIGH; /* not pressed */

    SYN_Button btn;
    syn_button_init(&btn, 1, SYN_BUTTON_ACTIVE_LOW, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_release(&btn, btn_on_release, NULL);

    /* Not pressed — no event */
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);

    /* Press = drive LOW */
    mock_gpio_states[1] = SYN_GPIO_LOW;
    syn_button_update(&btn);

    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);
    TEST_ASSERT_TRUE(syn_button_is_pressed(&btn));

    /* Release = drive HIGH */
    mock_gpio_states[1] = SYN_GPIO_HIGH;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_release_count);
}

/* ── Test: repeat in PRESSED state ──────────────────────────────────────── */

static void test_button_repeat_in_pressed(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[2] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 2, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_repeat(&btn, btn_on_repeat, 200, NULL); /* repeat every 200ms */

    /* Press → debounce */
    mock_gpio_states[2] = 1;
    syn_button_update(&btn);
    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);

    /* Hold without long-press threshold — stay in PRESSED state */
    /* Advance past one repeat interval */
    mock_tick_advance(210);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_repeat_count); /* first repeat fired */

    mock_tick_advance(210);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(2, btn_repeat_count); /* second repeat */
}

/* ── Test: repeat in HELD state ─────────────────────────────────────────── */

static void test_button_repeat_in_held(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[3] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 3, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 300, NULL);
    syn_button_on_repeat(&btn, btn_on_repeat, 150, NULL);

    /* Press → debounce → confirm */
    mock_gpio_states[3] = 1;
    syn_button_update(&btn);
    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);

    /* Hold past long-press threshold → move to HELD state */
    mock_tick_advance(310);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);

    /* Now in HELD state — advance past repeat interval */
    mock_tick_advance(160);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_repeat_count);

    mock_tick_advance(160);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(2, btn_repeat_count);

    /* Release from HELD state */
    mock_gpio_states[3] = 0;
    syn_button_update(&btn);
    TEST_ASSERT_FALSE(syn_button_is_pressed(&btn));
}

/* ── Test: syn_button_service (batch update) ─────────────────────────────── */

static void test_button_service(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[4] = 0;
    mock_gpio_states[5] = 0;

    SYN_Button btns[2];
    syn_button_init(&btns[0], 4, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_init(&btns[1], 5, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btns[0], btn_on_press, NULL);
    syn_button_on_press(&btns[1], btn_on_press, NULL);

    /* Press both buttons */
    mock_gpio_states[4] = 1;
    mock_gpio_states[5] = 1;
    syn_button_service(btns, 2);

    mock_tick_advance(60);
    syn_button_service(btns, 2);

    /* Both should have fired press events */
    TEST_ASSERT_EQUAL_INT(2, btn_press_count);

    /* service with count=0 — should not crash */
    syn_button_service(btns, 0);
}

/* ── Test: Multi-click detection (single, double, triple, multi) ───────────── */

static void test_button_multi_clicks(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[6] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 6, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btn, 200);

    syn_button_on_single_click(&btn, btn_on_single_click, NULL);
    syn_button_on_double_click(&btn, btn_on_double_click, NULL);
    syn_button_on_triple_click(&btn, btn_on_triple_click, NULL);
    syn_button_on_multi_click(&btn, btn_on_multi_click, NULL);

    /* 1. Single click test: press, debounce, release, wait window expiry */
    mock_gpio_states[6] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[6] = 0;
    syn_button_update(&btn);

    /* Before window expiry — no single click yet */
    mock_tick_advance(100);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);

    /* Advance past 200ms window */
    mock_tick_advance(150);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_single_click_count);

    /* 2. Double click test */
    reset_counts();
    /* Tap 1 */
    mock_gpio_states[6] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[6] = 0;
    syn_button_update(&btn);

    /* Tap 2 within 200ms */
    mock_tick_advance(80);
    mock_gpio_states[6] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[6] = 0;
    syn_button_update(&btn);

    /* Advance past window */
    mock_tick_advance(210);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);
    TEST_ASSERT_EQUAL_INT(1, btn_double_click_count);

    /* 3. Triple click test */
    reset_counts();
    for (int i = 0; i < 3; i++) {
        mock_gpio_states[6] = 1;
        syn_button_update(&btn);
        mock_tick_advance(25);
        syn_button_update(&btn);
        mock_gpio_states[6] = 0;
        syn_button_update(&btn);
        mock_tick_advance(50);
    }
    mock_tick_advance(210);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_triple_click_count);

    /* 4. Multi-click (4 taps) test */
    reset_counts();
    for (int i = 0; i < 4; i++) {
        mock_gpio_states[6] = 1;
        syn_button_update(&btn);
        mock_tick_advance(25);
        syn_button_update(&btn);
        mock_gpio_states[6] = 0;
        syn_button_update(&btn);
        mock_tick_advance(50);
    }
    mock_tick_advance(210);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_multi_click_count);
}

/* ── Test: Combo button (simultaneous multi-button press) ─────────────── */

static void test_button_combo(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[7] = 0;
    mock_gpio_states[8] = 0;

    SYN_Button btn1, btn2;
    syn_button_init(&btn1, 7, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_init(&btn2, 8, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btn1, 200);
    syn_button_set_click_window(&btn2, 200);
    syn_button_on_single_click(&btn1, btn_on_single_click, NULL);
    syn_button_on_single_click(&btn2, btn_on_single_click, NULL);
    syn_button_on_long_press(&btn1, btn_on_long, 300, NULL);
    syn_button_on_long_press(&btn2, btn_on_long, 300, NULL);
    syn_button_on_repeat(&btn1, btn_on_repeat, 100, NULL);
    syn_button_on_repeat(&btn2, btn_on_repeat, 100, NULL);

    SYN_Button *combo_btns[2] = {&btn1, &btn2};
    SYN_ButtonCombo combo;
    syn_button_combo_init(&combo, combo_btns, 2, combo_on_fire, NULL);

    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo));

    /* Press btn1 only */
    mock_gpio_states[7] = 1;
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    mock_tick_advance(25);
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(0, combo_fired_count);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo));

    /* Now press btn2 as well */
    mock_gpio_states[8] = 1;
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    mock_tick_advance(25);
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    syn_button_combo_update(&combo);

    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo));

    /* Advance time by 1000ms while holding both buttons in combo state */
    for (int i = 0; i < 100; i++) {
        mock_tick_advance(10);
        syn_button_update(&btn1);
        syn_button_update(&btn2);
        syn_button_combo_update(&combo);
    }

    /* Long press and repeat should BE SUPPRESSED for member buttons during combo */
    TEST_ASSERT_EQUAL_INT(0, btn_long_count);
    TEST_ASSERT_EQUAL_INT(0, btn_repeat_count);

    /* Release btn1 and btn2 */
    mock_gpio_states[7] = 0;
    mock_gpio_states[8] = 0;
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo));

    /* Advance past click window (200ms) to ensure single clicks do NOT trigger */
    mock_tick_advance(210);
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);
}

/* ── Test: 3-Button Combo & Staggered Release ───────────────────────────── */

static void test_button_combo_3_buttons(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[10] = 0;
    mock_gpio_states[11] = 0;
    mock_gpio_states[12] = 0;

    SYN_Button b1, b2, b3;
    syn_button_init(&b1, 10, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_init(&b2, 11, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_init(&b3, 12, SYN_BUTTON_ACTIVE_HIGH, 20);

    syn_button_on_single_click(&b1, btn_on_single_click, NULL);
    syn_button_on_single_click(&b2, btn_on_single_click, NULL);
    syn_button_on_single_click(&b3, btn_on_single_click, NULL);
    syn_button_on_long_press(&b1, btn_on_long, 200, NULL);
    syn_button_on_long_press(&b2, btn_on_long, 200, NULL);
    syn_button_on_long_press(&b3, btn_on_long, 200, NULL);

    SYN_Button *trio[3] = {&b1, &b2, &b3};
    SYN_ButtonCombo combo3;
    syn_button_combo_init(&combo3, trio, 3, combo_on_fire, NULL);

    /* Press only b1 and b2 */
    mock_gpio_states[10] = 1;
    mock_gpio_states[11] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    mock_tick_advance(25);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    syn_button_combo_update(&combo3);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo3));
    TEST_ASSERT_EQUAL_INT(0, combo_fired_count);

    /* Press b3 to complete trio */
    mock_gpio_states[12] = 1;
    syn_button_update(&b3);
    mock_tick_advance(25);
    syn_button_update(&b3);
    syn_button_combo_update(&combo3);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo3));
    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);

    /* Hold trio for 500ms */
    mock_tick_advance(500);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    syn_button_combo_update(&combo3);
    TEST_ASSERT_EQUAL_INT(0, btn_long_count);

    /* Release b1 first while b2 and b3 remain held: combo stays latched until all release */
    mock_gpio_states[10] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    syn_button_combo_update(&combo3);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo3));

    /* Release b2 and b3 */
    mock_gpio_states[11] = 0;
    mock_gpio_states[12] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    syn_button_combo_update(&combo3);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo3));

    mock_tick_advance(250);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_update(&b3);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);
}

/* ── Test: Helper Queries & NULL Parameters ─────────────────────────────── */

static void test_button_queries_and_null_checks(void)
{
    mock_tick_ms = 100;
    SYN_Button btn;
    syn_button_init(&btn, 13, SYN_BUTTON_ACTIVE_HIGH, 20);

    TEST_ASSERT_EQUAL_UINT32(0, syn_button_held_ms(&btn));
    TEST_ASSERT_EQUAL_UINT8(0, syn_button_clicks(&btn));
    TEST_ASSERT_FALSE(syn_button_combo_is_active(NULL));

    /* Combo update with NULL / empty list */
    SYN_ButtonCombo empty_combo;
    syn_button_combo_init(&empty_combo, NULL, 0, NULL, NULL);
    syn_button_combo_update(&empty_combo);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&empty_combo));

    SYN_Button *null_btn_list[2] = {NULL, NULL};
    SYN_ButtonCombo null_combo;
    syn_button_combo_init(&null_combo, null_btn_list, 2, NULL, NULL);
    syn_button_combo_update(&null_combo);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&null_combo));
}

/* ── Test: Long Press / Repeat Release Prevents Spurious Single Click ───── */

static void test_button_long_press_release_no_single_click(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[14] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 14, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btn, 250);
    syn_button_on_single_click(&btn, btn_on_single_click, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 300, NULL);
    syn_button_on_repeat(&btn, btn_on_repeat, 100, NULL);

    /* Press button */
    mock_gpio_states[14] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);

    /* Hold button past long press threshold (300ms) */
    mock_tick_advance(310);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);

    /* Hold button for 300ms more with repeats */
    mock_tick_advance(300);
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_repeat_count > 0);

    /* Release button */
    mock_gpio_states[14] = 0;
    syn_button_update(&btn);

    /* Advance past click window (300ms) */
    mock_tick_advance(300);
    syn_button_update(&btn);

    /* SINGLE CLICK MUST NOT FIRE AFTER A LONG PRESS / REPEAT GESTURE */
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);
}

/* ── Test: Polled Events & Immediate Click Window ───────────────────────── */

static void test_button_polled_events_and_immediate_click_window(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[15] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 15, SYN_BUTTON_ACTIVE_HIGH, 20);

    /* Set click window to 0 -> immediate single click on release */
    syn_button_set_click_window(&btn, 0);
    syn_button_on_single_click(&btn, btn_on_single_click, NULL);

    /* Tap button */
    mock_gpio_states[15] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[15] = 0;
    syn_button_update(&btn);

    /* With window = 0, single click fires immediately on next update in IDLE */
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_single_click_count);

    /* Verify polled event bitmask collects multiple events */
    btn.events = 0;
    mock_gpio_states[15] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[15] = 0;
    syn_button_update(&btn);

    uint8_t evts = syn_button_poll_events(&btn);
    TEST_ASSERT_TRUE((evts & SYN_BUTTON_EVT_PRESS) != 0);
    TEST_ASSERT_TRUE((evts & SYN_BUTTON_EVT_RELEASE) != 0);
    TEST_ASSERT_EQUAL_UINT8(0, syn_button_poll_events(&btn));
}

/* ── Test: Slow Double Tap Registers Two Single Clicks ──────────────────── */

static void test_button_behavior_slow_double_tap(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[16] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 16, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btn, 250);
    syn_button_on_single_click(&btn, btn_on_single_click, NULL);
    syn_button_on_double_click(&btn, btn_on_double_click, NULL);

    /* Tap 1 */
    mock_gpio_states[16] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[16] = 0;
    syn_button_update(&btn);

    /* Wait 300ms (gap > double_click_ms 250) */
    mock_tick_advance(300);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_single_click_count);

    /* Tap 2 */
    mock_gpio_states[16] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[16] = 0;
    syn_button_update(&btn);

    /* Wait 300ms */
    mock_tick_advance(300);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(2, btn_single_click_count);
    TEST_ASSERT_EQUAL_INT(0, btn_double_click_count);
}

/* ── Test: Double Tap and Hold Gesture ──────────────────────────────────── */

static void test_button_behavior_double_tap_and_hold(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[17] = 0;
    SYN_Button btn;
    syn_button_init(&btn, 17, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btn, 250);
    syn_button_on_single_click(&btn, btn_on_single_click, NULL);
    syn_button_on_double_click(&btn, btn_on_double_click, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 300, NULL);

    /* Tap 1 */
    mock_gpio_states[17] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);
    mock_gpio_states[17] = 0;
    syn_button_update(&btn);

    /* Tap 2 (pressed down within 100ms, then held past 300ms) */
    mock_tick_advance(100);
    mock_gpio_states[17] = 1;
    syn_button_update(&btn);
    mock_tick_advance(25);
    syn_button_update(&btn);

    /* Hold past long press threshold (300ms) */
    mock_tick_advance(310);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);

    /* Release */
    mock_gpio_states[17] = 0;
    syn_button_update(&btn);
    mock_tick_advance(300);
    syn_button_update(&btn);

    /* Double-click and single-click must NOT fire for tap-and-hold */
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);
    TEST_ASSERT_EQUAL_INT(0, btn_double_click_count);
}

/* ── Test: Overlapping Independent Buttons ──────────────────────────────── */

static void test_button_behavior_overlapping_independent_buttons(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[18] = 0;
    mock_gpio_states[19] = 0;

    SYN_Button btnA, btnB;
    syn_button_init(&btnA, 18, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_init(&btnB, 19, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_set_click_window(&btnA, 250);
    syn_button_set_click_window(&btnB, 250);
    syn_button_on_single_click(&btnA, btn_on_single_click, NULL);
    syn_button_on_single_click(&btnB, btn_on_single_click, NULL);

    /* Press Btn A */
    mock_gpio_states[18] = 1;
    syn_button_update(&btnA);
    syn_button_update(&btnB);

    /* 10ms later, Press Btn B */
    mock_tick_advance(10);
    mock_gpio_states[19] = 1;
    syn_button_update(&btnA);
    syn_button_update(&btnB);

    /* Advance 25ms */
    mock_tick_advance(25);
    syn_button_update(&btnA);
    syn_button_update(&btnB);

    /* Release Btn A */
    mock_gpio_states[18] = 0;
    syn_button_update(&btnA);

    /* Release Btn B */
    mock_tick_advance(20);
    mock_gpio_states[19] = 0;
    syn_button_update(&btnB);

    /* Wait for click window expiry */
    mock_tick_advance(300);
    syn_button_update(&btnA);
    syn_button_update(&btnB);

    /* Both buttons must register 1 single click each (total 2) */
    TEST_ASSERT_EQUAL_INT(2, btn_single_click_count);
}

/* ── Test: Combo Reactivation Cycle ─────────────────────────────────────── */

static void test_button_behavior_combo_reactivation_cycle(void)
{
    mock_tick_ms = 0;
    reset_counts();

    mock_gpio_states[20] = 0;
    mock_gpio_states[21] = 0;

    SYN_Button b1, b2;
    syn_button_init(&b1, 20, SYN_BUTTON_ACTIVE_HIGH, 20);
    syn_button_init(&b2, 21, SYN_BUTTON_ACTIVE_HIGH, 20);

    SYN_Button *pair[2] = {&b1, &b2};
    SYN_ButtonCombo combo;
    syn_button_combo_init(&combo, pair, 2, combo_on_fire, NULL);

    /* 1st Combo press */
    mock_gpio_states[20] = 1;
    mock_gpio_states[21] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);

    /* Release b1 while b2 is held: combo remains latched */
    mock_gpio_states[20] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo));

    /* Re-press b1 while b2 is still held: must NOT double-fire */
    mock_gpio_states[20] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);

    /* Release BOTH buttons to clear latch */
    mock_gpio_states[20] = 0;
    mock_gpio_states[21] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo));

    /* Full 2nd chord press reactivates and fires 2nd time */
    mock_gpio_states[20] = 1;
    mock_gpio_states[21] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(2, combo_fired_count);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo));
}

/* ── R3-K07: Atomic button event consumption & PT macros ───────────────── */

static SYN_Button g_pt_btn;
static int g_btn_ptr_evals = 0;
static int g_btn_pt_stage = 0;

static SYN_Button *next_btn_ptr(void)
{
    g_btn_ptr_evals++;
    return &g_pt_btn;
}

static SYN_PT_Status pt_btn_wait_fn(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_WAIT_BUTTON_PRESS(pt, next_btn_ptr());
    g_btn_pt_stage = 1;
    PT_WAIT_BUTTON_RELEASE(pt, next_btn_ptr());
    g_btn_pt_stage = 2;
    PT_END(pt);
}

static SYN_PT_Status pt_btn_block_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BUTTON_BLOCK_PRESS(pt, task, &g_pt_btn);
    g_btn_pt_stage = 1;
    PT_BUTTON_BLOCK_RELEASE(pt, task, &g_pt_btn);
    g_btn_pt_stage = 2;
    PT_END(pt);
}

static void test_button_consume_event_and_pt_macros(void)
{
    mock_tick_ms = 0U;
    mock_critical_depth = 0;

    /* 1. NULL guards */
    TEST_ASSERT_EQUAL_UINT8(0U, syn_button_poll_events(NULL));
    TEST_ASSERT_FALSE(syn_button_consume_event(NULL, SYN_BUTTON_EVT_PRESS));

    /* 2. Atomic selective bit consumption & critical section balance */
    mock_gpio_states[22] = 0;
    syn_button_init(&g_pt_btn, 22, SYN_BUTTON_ACTIVE_HIGH, 20U);

    int enters_before = mock_critical_enter_count;
    TEST_ASSERT_FALSE(syn_button_consume_event(&g_pt_btn, SYN_BUTTON_EVT_PRESS));
    TEST_ASSERT_EQUAL_INT(enters_before + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* Generate press + release so multiple bits are pending simultaneously */
    mock_gpio_states[22] = 1;
    syn_button_update(&g_pt_btn);
    mock_tick_advance(25U);
    syn_button_update(&g_pt_btn);
    mock_gpio_states[22] = 0;
    syn_button_update(&g_pt_btn);

    TEST_ASSERT_TRUE(syn_button_consume_event(&g_pt_btn, SYN_BUTTON_EVT_PRESS));
    TEST_ASSERT_FALSE(syn_button_consume_event(&g_pt_btn, SYN_BUTTON_EVT_PRESS));
    /* Release bit was preserved when consuming press bit */
    enters_before = mock_critical_enter_count;
    TEST_ASSERT_EQUAL_UINT8(SYN_BUTTON_EVT_RELEASE, syn_button_poll_events(&g_pt_btn));
    TEST_ASSERT_EQUAL_INT(enters_before + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    TEST_ASSERT_EQUAL_UINT8(0U, syn_button_poll_events(&g_pt_btn));

    /* 3. PT_WAIT_BUTTON_PRESS / PT_WAIT_BUTTON_RELEASE single evaluation */
    SYN_PT pt;
    PT_INIT(&pt);
    g_btn_ptr_evals = 0;
    g_btn_pt_stage = 0;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_btn_wait_fn(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_btn_ptr_evals);

    g_pt_btn.events = (uint8_t)(SYN_BUTTON_EVT_PRESS | SYN_BUTTON_EVT_LONG_PRESS);
    TEST_ASSERT_EQUAL(PT_IDLE, pt_btn_wait_fn(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_btn_pt_stage);
    /* 1 eval for PRESS consume + 1 eval checking RELEASE (false) */
    TEST_ASSERT_EQUAL_INT(3, g_btn_ptr_evals);
    TEST_ASSERT_EQUAL_UINT8(SYN_BUTTON_EVT_LONG_PRESS, g_pt_btn.events);

    g_pt_btn.events |= SYN_BUTTON_EVT_RELEASE;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_btn_wait_fn(&pt));
    TEST_ASSERT_EQUAL_INT(2, g_btn_pt_stage);
    TEST_ASSERT_EQUAL_UINT8(SYN_BUTTON_EVT_LONG_PRESS, g_pt_btn.events);
    (void)syn_button_poll_events(&g_pt_btn);

    /* 4. PT_BUTTON_BLOCK_PRESS / PT_BUTTON_BLOCK_RELEASE with scheduler */
    SYN_Task task = {0};
    SYN_Sched sched;
    g_btn_pt_stage = 0;
    syn_task_create(&task, "btn_blk", pt_btn_block_fn, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&sched, &task, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);
    TEST_ASSERT_EQUAL_INT(0, g_btn_pt_stage);

    g_pt_btn.events |= SYN_BUTTON_EVT_PRESS;
    syn_task_resume(&task);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, g_btn_pt_stage);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);

    g_pt_btn.events |= SYN_BUTTON_EVT_RELEASE;
    syn_task_resume(&task);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, g_btn_pt_stage);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, task.state);
}

static void r4_advance_tick_on_critical_cb(void)
{
    mock_tick_ms += 5U;
}

/** R4-K02: Single tick snapshot per update, HELD/repeat else-if, and NULL guards. */
static void test_r4_button_tick_snapshot_and_null_guards(void)
{
    /* 1. Exercise all public & inline NULL guards */
    syn_button_init(NULL, 0, SYN_BUTTON_ACTIVE_HIGH, 20U);
    syn_button_set_click_window(NULL, 200U);
    syn_button_on_press(NULL, btn_on_press, NULL);
    syn_button_on_release(NULL, btn_on_release, NULL);
    syn_button_on_long_press(NULL, btn_on_long, 300U, NULL);
    syn_button_on_repeat(NULL, btn_on_repeat, 100U, NULL);
    syn_button_on_single_click(NULL, btn_on_single_click, NULL);
    syn_button_on_double_click(NULL, btn_on_double_click, NULL);
    syn_button_on_triple_click(NULL, btn_on_triple_click, NULL);
    syn_button_on_multi_click(NULL, btn_on_multi_click, NULL);
    syn_button_combo_init(NULL, NULL, 2U, combo_on_fire, NULL);
    syn_button_combo_update(NULL);
    syn_button_update(NULL);
    syn_button_service(NULL, 2U);
    TEST_ASSERT_FALSE(syn_button_is_pressed(NULL));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_button_held_ms(NULL));
    TEST_ASSERT_EQUAL_UINT8(0U, syn_button_clicks(NULL));

    SYN_ButtonCombo bad_combo;
    syn_button_combo_init(&bad_combo, NULL, 2U, combo_on_fire, NULL);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)bad_combo.count);

    /* 2. Mid-update tick skew + coinciding long_press_ms == repeat_ms threshold */
    reset_counts();
    mock_tick_ms = 100U;
    mock_gpio_states[23] = 0;

    SYN_Button btn;
    syn_button_init(&btn, 23, SYN_BUTTON_ACTIVE_HIGH, 20U);
    syn_button_set_click_window(&btn, 200U);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_release(&btn, btn_on_release, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 150U, NULL);
    syn_button_on_repeat(&btn, btn_on_repeat, 150U, NULL);
    syn_button_on_single_click(&btn, btn_on_single_click, NULL);

    mock_critical_enter_cb = r4_advance_tick_on_critical_cb;

    /* Start debounce at t=100, confirm press at t=120 (cb advances mock_tick_ms to 125) */
    mock_gpio_states[23] = 1;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_UINT32(100U, btn.state_tick);

    mock_tick_ms = 120U;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_UINT32(120U, btn.state_tick);
    TEST_ASSERT_EQUAL_UINT32(120U, btn.repeat_tick);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);

    /* Release at t=130 (cb advances mock_tick_ms to 135): click_tick == 130, no underflow! */
    mock_tick_ms = 130U;
    mock_gpio_states[23] = 0;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_UINT32(130U, btn.click_tick);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);

    /* Re-press and hold to exact long_press_ms == repeat_ms == 150ms boundary:
     * else-if ensures ONLY long_press fires on transition tick (zero repeat double-fire) */
    mock_tick_ms = 140U;
    mock_gpio_states[23] = 1;
    syn_button_update(&btn);
    mock_tick_ms = 160U;
    syn_button_update(&btn);

    mock_tick_ms = 310U;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);
    TEST_ASSERT_EQUAL_INT(0, btn_repeat_count);
    TEST_ASSERT_EQUAL_UINT32(310U, btn.state_tick);
    TEST_ASSERT_EQUAL_UINT32(310U, btn.repeat_tick);

    mock_critical_enter_cb = NULL;
}

static void test_r5_button_combo_latch_until_all_released(void)
{
    mock_tick_ms = 0U;
    reset_counts();

    /* Initialize const-free mutable button descriptors */
    mock_gpio_states[24] = 0;
    mock_gpio_states[25] = 0;

    SYN_Button b1;
    SYN_Button b2;
    syn_button_init(&b1, 24, SYN_BUTTON_ACTIVE_HIGH, 20U);
    syn_button_init(&b2, 25, SYN_BUTTON_ACTIVE_HIGH, 20U);
    syn_button_set_click_window(&b1, 200U);
    syn_button_set_click_window(&b2, 200U);
    syn_button_on_single_click(&b1, btn_on_single_click, NULL);
    syn_button_on_single_click(&b2, btn_on_single_click, NULL);

    SYN_Button *pair[2] = {&b1, &b2};
    SYN_ButtonCombo combo;
    syn_button_combo_init(&combo, pair, 2U, combo_on_fire, NULL);

    /* Press both -> fires combo once */
    mock_gpio_states[24] = 1;
    mock_gpio_states[25] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25U);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo));

    /* Bounce b1 (release + re-press + release) while b2 stays held */
    mock_gpio_states[24] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&combo));

    mock_gpio_states[24] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25U);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_EQUAL_INT(1, combo_fired_count);
    TEST_ASSERT_TRUE(b1.in_combo);
    TEST_ASSERT_EQUAL_UINT8(0U, b1.click_count);

    /* Release both -> latch clears and zero spurious single clicks fire */
    mock_gpio_states[24] = 0;
    mock_gpio_states[25] = 0;
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&combo);
    TEST_ASSERT_FALSE(syn_button_combo_is_active(&combo));

    mock_tick_advance(250U);
    syn_button_update(&b1);
    syn_button_update(&b2);
    TEST_ASSERT_EQUAL_INT(0, btn_single_click_count);

    /* Also exercise combo activation with NULL callback */
    SYN_ButtonCombo silent_combo;
    syn_button_combo_init(&silent_combo, pair, 2U, NULL, NULL);
    mock_gpio_states[24] = 1;
    mock_gpio_states[25] = 1;
    syn_button_update(&b1);
    syn_button_update(&b2);
    mock_tick_advance(25U);
    syn_button_update(&b1);
    syn_button_update(&b2);
    syn_button_combo_update(&silent_combo);
    TEST_ASSERT_TRUE(syn_button_combo_is_active(&silent_combo));
}

void run_button_tests(void)
{
    RUN_TEST(test_button);
    RUN_TEST(test_button_active_low);
    RUN_TEST(test_button_repeat_in_pressed);
    RUN_TEST(test_button_repeat_in_held);
    RUN_TEST(test_button_service);
    RUN_TEST(test_button_multi_clicks);
    RUN_TEST(test_button_combo);
    RUN_TEST(test_button_combo_3_buttons);
    RUN_TEST(test_button_queries_and_null_checks);
    RUN_TEST(test_button_long_press_release_no_single_click);
    RUN_TEST(test_button_polled_events_and_immediate_click_window);
    RUN_TEST(test_button_behavior_slow_double_tap);
    RUN_TEST(test_button_behavior_double_tap_and_hold);
    RUN_TEST(test_button_behavior_overlapping_independent_buttons);
    RUN_TEST(test_button_behavior_combo_reactivation_cycle);
    RUN_TEST(test_button_consume_event_and_pt_macros);
    RUN_TEST(test_r4_button_tick_snapshot_and_null_guards);
    RUN_TEST(test_r5_button_combo_latch_until_all_released);
}
