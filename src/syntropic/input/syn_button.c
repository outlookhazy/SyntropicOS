#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_BUTTON) || SYN_USE_BUTTON

#if defined(SYN_USE_FSM) && !SYN_USE_FSM
#error "syn_button requires SYN_USE_FSM=1 (table-driven FSM)"
#endif

/**
 * @file syn_button.c
 * @brief Debounced button implementation using syn_fsm.
 */

#include "../drivers/syn_gpio.h"
#include "../util/syn_assert.h"
#include "syn_button.h"

#include <string.h>

/* ── Internal FSM events ───────────────────────────────────────────────── */

enum {
    BTN_EV_RAW_PRESS = 0, /**< GPIO went active                       */
    BTN_EV_RAW_RELEASE,   /**< GPIO went inactive                     */
    BTN_EV_DEBOUNCE_OK,   /**< Debounce timer expired while still held */
    BTN_EV_LONG_PRESS,    /**< Long-press threshold reached           */
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Read the raw (debounce-agnostic) button state.
 * @param btn  Button instance.
 * @return true if the GPIO indicates pressed.
 */
static bool button_read_raw(const SYN_Button *btn)
{
    SYN_GPIO_State level = syn_gpio_read(btn->pin);

    if (btn->polarity == (uint8_t)SYN_BUTTON_ACTIVE_LOW) {
        return level == SYN_GPIO_LOW;
    }
    return level == SYN_GPIO_HIGH;
}

/**
 * @brief Set an event flag and invoke the callback.
 * @param btn  Button instance.
 * @param evt  Event bitmask.
 * @param cb   Callback (may be NULL).
 * @param ctx  User context for callback.
 */
static void button_fire_event(SYN_Button *btn, uint8_t evt, SYN_ButtonCallback cb, void *ctx)
{
    syn_port_enter_critical();
    btn->events |= evt;
    syn_port_exit_critical();
    if (cb != NULL) {
        cb(btn, ctx);
    }
}

/* ── FSM transition actions ─────────────────────────────────────────────── */

/**
 * @brief FSM action: start debounce timer on raw press.
 * @param ctx  Button (as void*).
 */
static void action_start_debounce(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = btn->now_tick;
    btn->raw_pressed = true;
}

/**
 * @brief FSM action: raw release during debounce → return to idle.
 * @param ctx  Button (as void*).
 */
static void action_bounce_back(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = btn->now_tick;
    btn->raw_pressed = false;
}

/**
 * @brief FSM action: debounce confirmed, fire press event.
 * @param ctx  Button (as void*).
 */
static void action_confirm_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = btn->now_tick;
    btn->pressed = true;
    btn->repeat_tick = btn->now_tick;
    btn->click_count++;

    button_fire_event(btn, SYN_BUTTON_EVT_PRESS, btn->on_press, btn->on_press_ctx);
}

/**
 * @brief FSM action: button released, fire release event.
 * @param ctx  Button (as void*).
 */
static void action_release(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = btn->now_tick;
    btn->click_tick = btn->now_tick;
    btn->pressed = false;
    btn->raw_pressed = false;
    btn->in_combo = false;

    button_fire_event(btn, SYN_BUTTON_EVT_RELEASE, btn->on_release, btn->on_release_ctx);
}

/**
 * @brief FSM action: held long enough, fire long-press event.
 * @param ctx  Button (as void*).
 */
static void action_long_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick = btn->now_tick;
    btn->repeat_tick = btn->now_tick;
    btn->click_count = 0;

    button_fire_event(btn, SYN_BUTTON_EVT_LONG_PRESS, btn->on_long_press, btn->on_long_press_ctx);
}

/* ── FSM transition table ───────────────────────────────────────────────── */

/** @brief Button FSM transition table. */
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
    if (btn == NULL) {
        return;
    }

    uint32_t now = syn_port_get_tick_ms();
    memset(btn, 0, sizeof(*btn));
    btn->pin = pin;
    btn->polarity = (uint8_t)polarity;
    btn->debounce_ms = debounce_ms;
    btn->double_click_ms = 250;
    btn->pressed = false;
    btn->raw_pressed = false;
    btn->click_count = 0;
    btn->state_tick = now;
    btn->now_tick = now;

    syn_fsm_init(&btn->fsm, g_button_transitions, (SYN_FSM_State)SYN_BUTTON_STATE_IDLE, "btn");
    syn_fsm_set_context(&btn->fsm, btn);
}

/* ── Configuration ──────────────────────────────────────────────────────── */

void syn_button_set_click_window(SYN_Button *btn, uint16_t window_ms)
{
    if (btn == NULL) {
        return;
    }
    btn->double_click_ms = window_ms;
}

/* ── Callback registration ──────────────────────────────────────────────── */

void syn_button_on_press(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_press = cb;
    btn->on_press_ctx = ctx;
}

void syn_button_on_release(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_release = cb;
    btn->on_release_ctx = ctx;
}

void syn_button_on_long_press(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t hold_ms, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_long_press = cb;
    btn->on_long_press_ctx = ctx;
    btn->long_press_ms = hold_ms;
}

void syn_button_on_repeat(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t interval_ms, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_repeat = cb;
    btn->on_repeat_ctx = ctx;
    btn->repeat_ms = interval_ms;
}

void syn_button_on_single_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_single_click = cb;
    btn->on_single_click_ctx = ctx;
}

void syn_button_on_double_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_double_click = cb;
    btn->on_double_click_ctx = ctx;
}

void syn_button_on_triple_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_triple_click = cb;
    btn->on_triple_click_ctx = ctx;
}

void syn_button_on_multi_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    if (btn == NULL) {
        return;
    }
    btn->on_multi_click = cb;
    btn->on_multi_click_ctx = ctx;
}

/* ── Combo button functions ─────────────────────────────────────────────── */

/**
 * @brief Reset click count and mark currently pressed combo buttons as suppressed.
 * @param combo Button combo instance.
 */
static void combo_suppress_pressed(SYN_ButtonCombo *combo)
{
    for (size_t i = 0; i < combo->count; i++) {
        if (combo->buttons[i]->pressed) {
            combo->buttons[i]->click_count = 0U;
            combo->buttons[i]->in_combo = true;
        }
    }
}

void syn_button_combo_init(SYN_ButtonCombo *combo, SYN_Button *const *buttons, size_t count,
                           SYN_ButtonCallback cb, void *ctx)
{
    if (combo == NULL) {
        return;
    }

    memset(combo, 0, sizeof(*combo));
    combo->buttons = buttons;
    combo->count = (buttons != NULL) ? count : 0U;
    combo->on_combo = cb;
    combo->on_combo_ctx = ctx;
    combo->active = false;
}

void syn_button_combo_update(SYN_ButtonCombo *combo)
{
    if (combo == NULL || combo->count == 0U || combo->buttons == NULL) {
        return;
    }

    bool all_pressed = true;
    bool any_pressed = false;
    for (size_t i = 0; i < combo->count; i++) {
        if (combo->buttons[i] == NULL) {
            return;
        }
        if (combo->buttons[i]->pressed) {
            any_pressed = true;
        } else {
            all_pressed = false;
        }
    }

    if (combo->active) {
        if (!any_pressed) {
            combo->active = false;
        } else {
            combo_suppress_pressed(combo);
        }
        return;
    }

    if (all_pressed) {
        combo->active = true;
        combo_suppress_pressed(combo);
        if (combo->on_combo != NULL) {
            combo->on_combo(NULL, combo->on_combo_ctx);
        }
    }
}

/* ── State machine update ───────────────────────────────────────────────── */

/**
 * @brief Evaluate accumulated multi-click gestures once button returns to IDLE.
 * @param btn  Button instance.
 * @param now  Snapshot tick for current update.
 */
static void button_eval_multiclick(SYN_Button *btn, uint32_t now)
{
    if ((SYN_ButtonState)syn_fsm_state(&btn->fsm) != SYN_BUTTON_STATE_IDLE ||
        btn->click_count == 0U) {
        return;
    }

    uint32_t idle_elapsed = now - btn->click_tick;
    if (btn->double_click_ms != 0U && idle_elapsed < btn->double_click_ms) {
        return;
    }

    uint8_t count = btn->click_count;
    btn->click_count = 0U;

    if (count == 1U) {
        button_fire_event(btn, SYN_BUTTON_EVT_SINGLE_CLICK, btn->on_single_click,
                          btn->on_single_click_ctx);
    } else if (count == 2U) {
        button_fire_event(btn, SYN_BUTTON_EVT_DOUBLE_CLICK, btn->on_double_click,
                          btn->on_double_click_ctx);
    } else if (count == 3U) {
        button_fire_event(btn, SYN_BUTTON_EVT_TRIPLE_CLICK, btn->on_triple_click,
                          btn->on_triple_click_ctx);
    } else {
        button_fire_event(btn, SYN_BUTTON_EVT_MULTI_CLICK, btn->on_multi_click,
                          btn->on_multi_click_ctx);
    }
}

void syn_button_update(SYN_Button *btn)
{
    if (btn == NULL) {
        return;
    }

    bool raw = button_read_raw(btn);
    uint32_t now = syn_port_get_tick_ms();
    btn->now_tick = now;
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
            /* Check long press threshold; else check auto-repeat */
            if (btn->long_press_ms > 0U && elapsed >= btn->long_press_ms) {
                syn_fsm_dispatch(&btn->fsm, BTN_EV_LONG_PRESS);
            } else if (btn->repeat_ms > 0U && (now - btn->repeat_tick) >= btn->repeat_ms) {
                btn->repeat_tick = now;
                btn->click_count = 0U;
                button_fire_event(btn, SYN_BUTTON_EVT_REPEAT, btn->on_repeat, btn->on_repeat_ctx);
            }
        }
    } else if (current == SYN_BUTTON_STATE_HELD) {
        if (!raw) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_RELEASE);
        } else if (!btn->in_combo && btn->repeat_ms > 0U &&
                   (now - btn->repeat_tick) >= btn->repeat_ms) {
            btn->repeat_tick = now;
            btn->click_count = 0U;
            button_fire_event(btn, SYN_BUTTON_EVT_REPEAT, btn->on_repeat, btn->on_repeat_ctx);
        }
    }

    button_eval_multiclick(btn, now);
}

void syn_button_service(SYN_Button *buttons, size_t count)
{
    if (buttons == NULL) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        syn_button_update(&buttons[i]);
    }
}

#endif /* SYN_USE_BUTTON */
