

# File syn\_button.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_button.c**](syn__button_8c.md)

[Go to the documentation of this file](syn__button_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BUTTON) || SYN_USE_BUTTON

#if defined(SYN_USE_FSM) && !SYN_USE_FSM
#error "syn_button requires SYN_USE_FSM=1 (table-driven FSM)"
#endif

#include "../drivers/syn_gpio.h"
#include "../util/syn_assert.h"
#include "syn_button.h"

#include <string.h>

/* ── Internal FSM events ───────────────────────────────────────────────── */

enum {
    BTN_EV_RAW_PRESS = 0, 
    BTN_EV_RAW_RELEASE,   
    BTN_EV_DEBOUNCE_OK,   
    BTN_EV_LONG_PRESS,    
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

static bool button_read_raw(const SYN_Button *btn)
{
    if (btn == NULL)
        return false; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    SYN_GPIO_State level = syn_gpio_read(btn->pin);

    if (btn->polarity == (uint8_t)SYN_BUTTON_ACTIVE_LOW) {
        return level == SYN_GPIO_LOW;
    }
    return level == SYN_GPIO_HIGH;
}

static void button_fire_event(SYN_Button *btn, uint8_t evt, SYN_ButtonCallback cb, void *ctx)
{
    btn->events |= evt;
    if (cb != NULL) {
        cb(btn, ctx);
    }
}

/* ── FSM transition actions ─────────────────────────────────────────────── */

static void action_start_debounce(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = syn_port_get_tick_ms();
    btn->raw_pressed = true;
}

static void action_bounce_back(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = syn_port_get_tick_ms();
    btn->raw_pressed = false;
}

static void action_confirm_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = syn_port_get_tick_ms();
    btn->pressed = true;
    btn->repeat_tick = syn_port_get_tick_ms();
    btn->click_count++;

    button_fire_event(btn, SYN_BUTTON_EVT_PRESS, btn->on_press, btn->on_press_ctx);
}

static void action_release(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = syn_port_get_tick_ms();
    btn->click_tick = syn_port_get_tick_ms();
    btn->pressed = false;
    btn->raw_pressed = false;
    btn->in_combo = false;

    button_fire_event(btn, SYN_BUTTON_EVT_RELEASE, btn->on_release, btn->on_release_ctx);
}

static void action_long_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = syn_port_get_tick_ms();
    btn->repeat_tick = syn_port_get_tick_ms();
    btn->click_count = 0;

    button_fire_event(btn, SYN_BUTTON_EVT_LONG_PRESS, btn->on_long_press, btn->on_long_press_ctx);
}

/* ── FSM transition table ───────────────────────────────────────────────── */

static const SYN_FSM_Transition g_button_transitions[] = {
    /* IDLE: wait for press */
    {SYN_BUTTON_STATE_IDLE, BTN_EV_RAW_PRESS, SYN_BUTTON_STATE_DEBOUNCING, NULL,
     action_start_debounce},

    /* DEBOUNCING: bounce back or confirm */
    {SYN_BUTTON_STATE_DEBOUNCING, BTN_EV_RAW_RELEASE, SYN_BUTTON_STATE_IDLE, NULL,
     action_bounce_back},
    {SYN_BUTTON_STATE_DEBOUNCING, BTN_EV_DEBOUNCE_OK, SYN_BUTTON_STATE_PRESSED, NULL,
     action_confirm_press},

    /* PRESSED: release or escalate to held */
    {SYN_BUTTON_STATE_PRESSED, BTN_EV_RAW_RELEASE, SYN_BUTTON_STATE_IDLE, NULL, action_release},
    {SYN_BUTTON_STATE_PRESSED, BTN_EV_LONG_PRESS, SYN_BUTTON_STATE_HELD, NULL, action_long_press},

    /* HELD: release */
    {SYN_BUTTON_STATE_HELD, BTN_EV_RAW_RELEASE, SYN_BUTTON_STATE_IDLE, NULL, action_release},

    SYN_FSM_END};

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_button_init(SYN_Button *btn, SYN_GPIO_Pin pin, SYN_ButtonPolarity polarity,
                     uint16_t debounce_ms)
{
    SYN_ASSERT(btn != NULL);

    memset(btn, 0, sizeof(*btn));
    btn->pin = pin;
    btn->polarity = (uint8_t)polarity;
    btn->debounce_ms = debounce_ms;
    btn->double_click_ms = 250;
    btn->pressed = false;
    btn->raw_pressed = false;
    btn->click_count = 0;
    btn->state_tick = syn_port_get_tick_ms();

    syn_fsm_init(&btn->fsm, g_button_transitions, (SYN_FSM_State)SYN_BUTTON_STATE_IDLE, "btn");
    syn_fsm_set_context(&btn->fsm, btn);
}

/* ── Configuration ──────────────────────────────────────────────────────── */

void syn_button_set_click_window(SYN_Button *btn, uint16_t window_ms)
{
    SYN_ASSERT(btn != NULL);
    btn->double_click_ms = window_ms;
}

/* ── Callback registration ──────────────────────────────────────────────── */

void syn_button_on_press(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_press = cb;
    btn->on_press_ctx = ctx;
}

void syn_button_on_release(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_release = cb;
    btn->on_release_ctx = ctx;
}

void syn_button_on_long_press(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t hold_ms, void *ctx)
{
    btn->on_long_press = cb;
    btn->on_long_press_ctx = ctx;
    btn->long_press_ms = hold_ms;
}

void syn_button_on_repeat(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t interval_ms, void *ctx)
{
    btn->on_repeat = cb;
    btn->on_repeat_ctx = ctx;
    btn->repeat_ms = interval_ms;
}

void syn_button_on_single_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_single_click = cb;
    btn->on_single_click_ctx = ctx;
}

void syn_button_on_double_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_double_click = cb;
    btn->on_double_click_ctx = ctx;
}

void syn_button_on_triple_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_triple_click = cb;
    btn->on_triple_click_ctx = ctx;
}

void syn_button_on_multi_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_multi_click = cb;
    btn->on_multi_click_ctx = ctx;
}

/* ── Combo button functions ─────────────────────────────────────────────── */

void syn_button_combo_init(SYN_ButtonCombo *combo, const SYN_Button **buttons, size_t count,
                           SYN_ButtonCallback cb, void *ctx)
{
    SYN_ASSERT(combo != NULL);
    SYN_ASSERT(buttons != NULL || count == 0);

    memset(combo, 0, sizeof(*combo));
    combo->buttons = buttons;
    combo->count = count;
    combo->on_combo = cb;
    combo->on_combo_ctx = ctx;
    combo->active = false;
}

void syn_button_combo_update(SYN_ButtonCombo *combo)
{
    SYN_ASSERT(combo != NULL);

    if (combo->count == 0 || combo->buttons == NULL) {
        return;
    }

    bool all_pressed = true;
    for (size_t i = 0; i < combo->count; i++) {
        if (combo->buttons[i] == NULL || !combo->buttons[i]->pressed) {
            all_pressed = false;
            break;
        }
    }

    if (all_pressed && !combo->active) {
        combo->active = true;
        for (size_t i = 0; i < combo->count; i++) {
            if (combo->buttons[i] != NULL) {
                ((SYN_Button *)combo->buttons[i])->click_count = 0;
                ((SYN_Button *)combo->buttons[i])->in_combo = true;
            }
        }
        if (combo->on_combo != NULL) {
            combo->on_combo(NULL, combo->on_combo_ctx);
        }
    } else if (!all_pressed && combo->active) {
        combo->active = false;
    }
}

/* ── State machine update ───────────────────────────────────────────────── */

void syn_button_update(SYN_Button *btn)
{
    SYN_ASSERT(btn != NULL);

    bool raw = button_read_raw(btn);
    uint32_t now = syn_port_get_tick_ms();
    uint32_t elapsed = now - btn->state_tick;
    SYN_ButtonState current = (SYN_ButtonState)syn_fsm_state(&btn->fsm);

    /* Generate edge events from GPIO reading */
    if (current == SYN_BUTTON_STATE_IDLE && raw) {
        syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_PRESS);
    } else if (current == SYN_BUTTON_STATE_DEBOUNCING) {
        if (!raw) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_RELEASE);
        } else if (elapsed >= btn->debounce_ms) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_DEBOUNCE_OK);
        }
    } else if (current == SYN_BUTTON_STATE_PRESSED) {
        if (!raw) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_RELEASE);
        } else if (!btn->in_combo) {
            /* Check long press threshold */
            if (btn->long_press_ms > 0 && elapsed >= btn->long_press_ms) {
                syn_fsm_dispatch(&btn->fsm, BTN_EV_LONG_PRESS);
            }
            /* Check repeat (stays in same state, no FSM transition) */
            if (btn->repeat_ms > 0 && (now - btn->repeat_tick) >= btn->repeat_ms) {
                btn->repeat_tick = now;
                btn->click_count = 0;
                button_fire_event(btn, SYN_BUTTON_EVT_REPEAT, btn->on_repeat, btn->on_repeat_ctx);
            }
        }
    } else if (current == SYN_BUTTON_STATE_HELD) {
        if (!raw) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_RELEASE);
        } else if (!btn->in_combo) {
            /* Check repeat in held state */
            if (btn->repeat_ms > 0 && (now - btn->repeat_tick) >= btn->repeat_ms) {
                btn->repeat_tick = now;
                btn->click_count = 0;
                button_fire_event(btn, SYN_BUTTON_EVT_REPEAT, btn->on_repeat, btn->on_repeat_ctx);
            }
        }
    }

    /* Evaluate multi-click timeout window when button is back in IDLE */
    current = (SYN_ButtonState)syn_fsm_state(&btn->fsm);
    if (current == SYN_BUTTON_STATE_IDLE && btn->click_count > 0) {
        uint32_t idle_elapsed = now - btn->click_tick;
        if (btn->double_click_ms == 0 || idle_elapsed >= btn->double_click_ms) {
            uint8_t count = btn->click_count;
            btn->click_count = 0;

            if (count == 1) {
                button_fire_event(btn, SYN_BUTTON_EVT_SINGLE_CLICK, btn->on_single_click,
                                  btn->on_single_click_ctx);
            } else if (count == 2) {
                button_fire_event(btn, SYN_BUTTON_EVT_DOUBLE_CLICK, btn->on_double_click,
                                  btn->on_double_click_ctx);
            } else if (count == 3) {
                button_fire_event(btn, SYN_BUTTON_EVT_TRIPLE_CLICK, btn->on_triple_click,
                                  btn->on_triple_click_ctx);
            } else if (count >= 4) {
                button_fire_event(btn, SYN_BUTTON_EVT_MULTI_CLICK, btn->on_multi_click,
                                  btn->on_multi_click_ctx);
            }
        }
    }
}

void syn_button_service(SYN_Button *buttons, size_t count)
{
    SYN_ASSERT(buttons != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        syn_button_update(&buttons[i]);
    }
}

#endif /* SYN_USE_BUTTON */
```


