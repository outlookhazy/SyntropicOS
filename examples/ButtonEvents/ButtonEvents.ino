/**
 * SyntropicOS — ButtonEvents Example
 *
 * Demonstrates multi-click detection (single, double, triple, N-click),
 * long press, and multi-button combination taps (SYN_ButtonCombo).
 *
 * Architecture & File Organization:
 *   - Tab 1 (ButtonEvents.ino): Application code — button callbacks and click setup.
 *   - Tab 2 (sys_init.h):       OS infrastructure — scheduler, background loop, & serial logging.
 *
 * Supported Boards:
 *   - Arduino Uno, Mega, Nano, ESP32, STM32duino, RP2040 (Raspberry Pi Pico), etc.
 *
 * Documentation & Guides:
 *   - Getting Started:  https://outlookhazy.github.io/SyntropicOS/getting-started/
 *   - Button Module:    https://outlookhazy.github.io/SyntropicOS/modules/io/
 */

#include <SyntropicOS.h>
#include <syntropic/input/syn_button.h>
#include "sys_init.h"  /* Tab 2: Encapsulates OS scheduler & background update loop */

#ifndef BTN1_PIN
#define BTN1_PIN 2
#endif

#ifndef BTN2_PIN
#define BTN2_PIN 3
#endif

static SYN_Button btn1;
static SYN_Button btn2;
static SYN_ButtonCombo combo;
static SYN_Button *combo_btns[2];

/* ── Application Event Callbacks ────────────────────────────────────────── */

static void on_btn1_single(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    SYN_LOG_I("btn", "[BTN 1] Single Click");
}

static void on_btn1_double(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    SYN_LOG_I("btn", "[BTN 1] Double Click!");
}

static void on_btn1_triple(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    SYN_LOG_I("btn", "[BTN 1] Triple Click!!");
}

static void on_btn1_multi(SYN_Button *btn, void *ctx)
{
    (void)ctx;
    SYN_LOG_I("btn", "[BTN 1] Multi-Click! Taps: %d", syn_button_clicks(btn));
}

static void on_btn1_long(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    SYN_LOG_I("btn", "[BTN 1] Long Press Held >1000ms");
}

static void on_combo_press(SYN_Button *btn, void *ctx)
{
    (void)btn; (void)ctx;
    SYN_LOG_I("btn", "⚡ [COMBO] Button 1 + Button 2 Pressed Together!");
}

/* ── Background Button Update Loop (Passed to sys_init in Tab 2) ────────── */

static void update_buttons(void)
{
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    syn_button_combo_update(&combo);
}

/* ── Main Arduino Sketch Setup & Loop ───────────────────────────────────── */

void setup()
{
    /* 1. Initialize OS scheduler & background update thread (see sys_init.h in Tab 2) */
    sys_init(update_buttons);

    SYN_LOG_I("main", "SyntropicOS Multi-Button & Combo Example Ready");

    /* 2. Initialize buttons (Pin, Polarity = Active Low with internal pull-up, 50ms debounce) */
    syn_button_init(&btn1, BTN1_PIN, SYN_BUTTON_ACTIVE_LOW, 50);
    syn_button_init(&btn2, BTN2_PIN, SYN_BUTTON_ACTIVE_LOW, 50);

    /* 3. Set 250ms multi-click evaluation gap window */
    syn_button_set_click_window(&btn1, 250);

    /* 4. Register gesture callbacks for Button 1 */
    syn_button_on_single_click(&btn1, on_btn1_single, NULL);
    syn_button_on_double_click(&btn1, on_btn1_double, NULL);
    syn_button_on_triple_click(&btn1, on_btn1_triple, NULL);
    syn_button_on_multi_click(&btn1, on_btn1_multi, NULL);
    syn_button_on_long_press(&btn1, on_btn1_long, 1000, NULL);

    /* 5. Setup multi-button combination (BTN1 + BTN2 pressed together) */
    combo_btns[0] = &btn1;
    combo_btns[1] = &btn2;
    syn_button_combo_init(&combo, combo_btns, 2, on_combo_press, NULL);
}

void loop()
{
    /* 6. Run background OS scheduler (serves buttons, tasks, and system ticks) */
    sys_run();
}
