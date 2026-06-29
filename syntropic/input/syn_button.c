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

#include "syn_button.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Internal FSM events ───────────────────────────────────────────────── */

enum {
    BTN_EV_RAW_PRESS = 0,    /**< GPIO went active                       */
    BTN_EV_RAW_RELEASE,      /**< GPIO went inactive                     */
    BTN_EV_DEBOUNCE_OK,      /**< Debounce timer expired while still held */
    BTN_EV_LONG_PRESS,       /**< Long-press threshold reached           */
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Read the raw (debounce-agnostic) button state.
 * @param btn  Button instance.
 * @return true if the GPIO indicates pressed.
 */
static bool button_read_raw(const SYN_Button *btn)
{
    SYN_GPIO_State level = syn_port_gpio_read(btn->pin);
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
static void button_fire_event(SYN_Button *btn, uint8_t evt,
                              SYN_ButtonCallback cb, void *ctx)
{
    btn->events |= evt;
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
    btn->state_tick  = syn_port_get_tick_ms();
    btn->raw_pressed = true;
}

/**
 * @brief FSM action: raw release during debounce → return to idle.
 * @param ctx  Button (as void*).
 */
static void action_bounce_back(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick  = syn_port_get_tick_ms();
    btn->raw_pressed = false;
}

/**
 * @brief FSM action: debounce confirmed, fire press event.
 * @param ctx  Button (as void*).
 */
static void action_confirm_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick  = syn_port_get_tick_ms();
    btn->pressed     = true;
    btn->repeat_tick = syn_port_get_tick_ms();

    button_fire_event(btn, SYN_BUTTON_EVT_PRESS,
                      btn->on_press, btn->on_press_ctx);
}

/**
 * @brief FSM action: button released, fire release event.
 * @param ctx  Button (as void*).
 */
static void action_release(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick  = syn_port_get_tick_ms();
    btn->pressed     = false;
    btn->raw_pressed = false;

    button_fire_event(btn, SYN_BUTTON_EVT_RELEASE,
                      btn->on_release, btn->on_release_ctx);
}

/**
 * @brief FSM action: held long enough, fire long-press event.
 * @param ctx  Button (as void*).
 */
static void action_long_press(void *ctx)
{
    SYN_Button *btn = (SYN_Button *)ctx;
    btn->state_tick  = syn_port_get_tick_ms();
    btn->repeat_tick = syn_port_get_tick_ms();

    button_fire_event(btn, SYN_BUTTON_EVT_LONG_PRESS,
                      btn->on_long_press, btn->on_long_press_ctx);
}

/* ── FSM transition table ───────────────────────────────────────────────── */

/** @brief Button FSM transition table. */
static const SYN_FSM_Transition g_button_transitions[] = {
    /* IDLE: wait for press */
    { SYN_BUTTON_STATE_IDLE,       BTN_EV_RAW_PRESS,    SYN_BUTTON_STATE_DEBOUNCING, NULL, action_start_debounce },

    /* DEBOUNCING: bounce back or confirm */
    { SYN_BUTTON_STATE_DEBOUNCING, BTN_EV_RAW_RELEASE,  SYN_BUTTON_STATE_IDLE,       NULL, action_bounce_back    },
    { SYN_BUTTON_STATE_DEBOUNCING, BTN_EV_DEBOUNCE_OK,  SYN_BUTTON_STATE_PRESSED,    NULL, action_confirm_press  },

    /* PRESSED: release or escalate to held */
    { SYN_BUTTON_STATE_PRESSED,    BTN_EV_RAW_RELEASE,  SYN_BUTTON_STATE_IDLE,       NULL, action_release        },
    { SYN_BUTTON_STATE_PRESSED,    BTN_EV_LONG_PRESS,   SYN_BUTTON_STATE_HELD,       NULL, action_long_press     },

    /* HELD: release */
    { SYN_BUTTON_STATE_HELD,       BTN_EV_RAW_RELEASE,  SYN_BUTTON_STATE_IDLE,       NULL, action_release        },

    SYN_FSM_END
};

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_button_init(SYN_Button *btn,
                      SYN_GPIO_Pin pin,
                      SYN_ButtonPolarity polarity,
                      uint16_t debounce_ms)
{
    SYN_ASSERT(btn != NULL);

    memset(btn, 0, sizeof(*btn));
    btn->pin          = pin;
    btn->polarity     = (uint8_t)polarity;
    btn->debounce_ms  = debounce_ms;
    btn->pressed      = false;
    btn->raw_pressed  = false;
    btn->state_tick   = syn_port_get_tick_ms();

    syn_fsm_init(&btn->fsm, g_button_transitions,
                 (SYN_FSM_State)SYN_BUTTON_STATE_IDLE, "btn");
    syn_fsm_set_context(&btn->fsm, btn);
}

/* ── Callback registration ──────────────────────────────────────────────── */

void syn_button_on_press(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_press     = cb;
    btn->on_press_ctx = ctx;
}

void syn_button_on_release(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx)
{
    btn->on_release     = cb;
    btn->on_release_ctx = ctx;
}

void syn_button_on_long_press(SYN_Button *btn, SYN_ButtonCallback cb,
                               uint16_t hold_ms, void *ctx)
{
    btn->on_long_press     = cb;
    btn->on_long_press_ctx = ctx;
    btn->long_press_ms     = hold_ms;
}

void syn_button_on_repeat(SYN_Button *btn, SYN_ButtonCallback cb,
                           uint16_t interval_ms, void *ctx)
{
    btn->on_repeat     = cb;
    btn->on_repeat_ctx = ctx;
    btn->repeat_ms     = interval_ms;
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
        } else {
            /* Check long press threshold */
            if (btn->long_press_ms > 0 && elapsed >= btn->long_press_ms) {
                syn_fsm_dispatch(&btn->fsm, BTN_EV_LONG_PRESS);
            }
            /* Check repeat (stays in same state, no FSM transition) */
            if (btn->repeat_ms > 0 &&
                (now - btn->repeat_tick) >= btn->repeat_ms) {
                btn->repeat_tick = now;
                button_fire_event(btn, SYN_BUTTON_EVT_REPEAT,
                                  btn->on_repeat, btn->on_repeat_ctx);
            }
        }
    } else if (current == SYN_BUTTON_STATE_HELD) {
        if (!raw) {
            syn_fsm_dispatch(&btn->fsm, BTN_EV_RAW_RELEASE);
        } else {
            /* Check repeat in held state */
            if (btn->repeat_ms > 0 &&
                (now - btn->repeat_tick) >= btn->repeat_ms) {
                btn->repeat_tick = now;
                button_fire_event(btn, SYN_BUTTON_EVT_REPEAT,
                                  btn->on_repeat, btn->on_repeat_ctx);
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
