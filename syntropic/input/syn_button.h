/**
 * @file syn_button.h
 * @brief Debounced button input with press/release/long-press/repeat.
 *
 * Pure polling — no interrupts required. Call syn_button_update()
 * (or syn_button_service() for an array) from your main loop or a
 * scheduler task. The module handles debouncing, edge detection, and
 * long-press timing internally.
 *
 * @par Usage
 * @code
 *   static SYN_Button btn;
 *   syn_button_init(&btn, PIN_BUTTON, SYN_BUTTON_ACTIVE_LOW, 50);
 *   syn_button_on_press(&btn, my_press_handler, NULL);
 *   syn_button_on_long_press(&btn, my_long_handler, 1000, NULL);
 *
 *   // Main loop:
 *   while (1) { syn_button_update(&btn); }
 *
 *   // Or in a protothread:
 *   PT_WAIT_BUTTON_PRESS(pt, &btn);
 * @endcode
 * @ingroup syn_io
 */

#ifndef SYN_BUTTON_H
#define SYN_BUTTON_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"
#include "../util/syn_fsm.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Button polarity ────────────────────────────────────────────────────── */

/** @brief Button polarity selector. */
typedef enum {
    SYN_BUTTON_ACTIVE_HIGH = 0,  /**< Pressed = GPIO high */
    SYN_BUTTON_ACTIVE_LOW  = 1,  /**< Pressed = GPIO low (pull-up) */
} SYN_ButtonPolarity;

/* ── Button state ───────────────────────────────────────────────────────── */

/** @brief Button FSM states. */
typedef enum {
    SYN_BUTTON_STATE_IDLE       = 0,  /**< Not pressed.         */
    SYN_BUTTON_STATE_DEBOUNCING = 1,  /**< Waiting for stable.  */
    SYN_BUTTON_STATE_PRESSED    = 2,  /**< Confirmed pressed.   */
    SYN_BUTTON_STATE_HELD       = 3,  /**< Held past threshold. */
} SYN_ButtonState;

/** @name Button event flags (bitmask)
 * @{
 */
#define SYN_BUTTON_EVT_PRESS       ((uint8_t)(1u << 0))  /**< Button pressed     */
#define SYN_BUTTON_EVT_RELEASE     ((uint8_t)(1u << 1))  /**< Button released    */
#define SYN_BUTTON_EVT_LONG_PRESS  ((uint8_t)(1u << 2))  /**< Long press detected */
#define SYN_BUTTON_EVT_REPEAT      ((uint8_t)(1u << 3))  /**< Auto-repeat fire   */
/** @} */

/* ── Callback type ──────────────────────────────────────────────────────── */

struct SYN_Button;

/**
 * @brief Button event callback.
 *
 * @param btn       The button that generated the event.
 * @param user_data User-provided context pointer.
 */
typedef void (*SYN_ButtonCallback)(struct SYN_Button *btn, void *user_data);

/* ── Button descriptor ──────────────────────────────────────────────────── */

/** @brief Button descriptor — owns the FSM, debounce, and callback state. */
typedef struct SYN_Button {
    /* Configuration */
    SYN_GPIO_Pin         pin;            /**< GPIO pin number               */
    uint8_t               polarity;       /**< SYN_ButtonPolarity            */
    uint16_t              debounce_ms;    /**< Debounce window (ms)          */
    uint16_t              long_press_ms;  /**< Long-press threshold (ms)     */
    uint16_t              repeat_ms;      /**< Auto-repeat interval (ms)     */

    /* State */
    SYN_FSM               fsm;            /**< Button FSM (uses syn_fsm)     */
    uint8_t               events;         /**< Pending events bitmask        */
    bool                  raw_pressed;    /**< Last raw GPIO reading         */
    bool                  pressed;        /**< Debounced pressed state       */
    uint32_t              state_tick;     /**< Tick when state was entered    */
    uint32_t              repeat_tick;    /**< Tick of last repeat event      */

    /* Callbacks */
    SYN_ButtonCallback   on_press;        /**< Press callback                */
    void                 *on_press_ctx;   /**< Press callback context        */
    SYN_ButtonCallback   on_release;      /**< Release callback              */
    void                 *on_release_ctx; /**< Release callback context      */
    SYN_ButtonCallback   on_long_press;   /**< Long-press callback           */
    void                 *on_long_press_ctx; /**< Long-press context         */
    SYN_ButtonCallback   on_repeat;       /**< Repeat callback               */
    void                 *on_repeat_ctx;  /**< Repeat callback context       */
} SYN_Button;

/* ── Initialization ─────────────────────────────────────────────────────── */

/**
 * @brief Initialize a button descriptor.
 *
 * @param btn          Button to initialize.
 * @param pin          GPIO pin number.
 * @param polarity     Active-high or active-low.
 * @param debounce_ms  Debounce window in milliseconds (e.g., 50).
 */
void syn_button_init(SYN_Button *btn,
                      SYN_GPIO_Pin pin,
                      SYN_ButtonPolarity polarity,
                      uint16_t debounce_ms);

/* ── Callback registration ──────────────────────────────────────────────── */

/**
 * @brief Register a press callback.
 * @param btn  Button.
 * @param cb   Callback (or NULL to disable).
 * @param ctx  User context.
 */
void syn_button_on_press(SYN_Button *btn,
                          SYN_ButtonCallback cb, void *ctx);

/**
 * @brief Register a release callback.
 * @param btn  Button.
 * @param cb   Callback (or NULL to disable).
 * @param ctx  User context.
 */
void syn_button_on_release(SYN_Button *btn,
                            SYN_ButtonCallback cb, void *ctx);

/**
 * @brief Register a long-press callback.
 *
 * @param btn      Button.
 * @param cb       Callback (or NULL to disable).
 * @param hold_ms  Time the button must be held before firing (ms).
 * @param ctx      User context.
 */
void syn_button_on_long_press(SYN_Button *btn,
                               SYN_ButtonCallback cb,
                               uint16_t hold_ms,
                               void *ctx);

/**
 * @brief Register an auto-repeat callback.
 *
 * Fires repeatedly while the button is held, after the initial press.
 *
 * @param btn          Button.
 * @param cb           Callback (or NULL to disable).
 * @param interval_ms  Repeat interval in milliseconds.
 * @param ctx          User context.
 */
void syn_button_on_repeat(SYN_Button *btn,
                           SYN_ButtonCallback cb,
                           uint16_t interval_ms,
                           void *ctx);

/* ── Update / service ───────────────────────────────────────────────────── */

/**
 * @brief Update a single button's state machine.
 *
 * Call this from your main loop, a scheduler task, or a timer callback.
 * Reads the GPIO, runs debouncing, and fires callbacks as needed.
 *
 * @param btn  Button to update.
 */
void syn_button_update(SYN_Button *btn);

/**
 * @brief Service an array of buttons.
 * @param buttons  Array of buttons.
 * @param count    Number of buttons.
 */
void syn_button_service(SYN_Button *buttons, size_t count);

/* ── Query ──────────────────────────────────────────────────────────────── */

/**
 * @brief Is the button currently pressed (debounced)?
 * @param btn  Button.
 * @return true if pressed.
 */
static inline bool syn_button_is_pressed(const SYN_Button *btn)
{
    return btn->pressed;
}

/**
 * @brief How long has the button been in its current state (ms)?
 * @param btn  Button.
 * @return Duration in milliseconds.
 */
static inline uint32_t syn_button_held_ms(const SYN_Button *btn)
{
    return syn_port_get_tick_ms() - btn->state_tick;
}

/**
 * @brief Read and clear pending events (bitmask of SYN_BUTTON_EVT_*).
 *
 * Useful for polling instead of callbacks.
 *
 * @param btn  Button.
 * @return Bitmask of events that occurred since last poll.
 */
static inline uint8_t syn_button_poll_events(SYN_Button *btn)
{
    uint8_t e = btn->events;
    btn->events = 0;
    return e;
}

/* ── Protothread integration ────────────────────────────────────────────── */

/** Block until the button is pressed (debounced). */
#define PT_WAIT_BUTTON_PRESS(pt, btn) \
    PT_WAIT_UNTIL(pt, (btn)->events & SYN_BUTTON_EVT_PRESS); \
    (btn)->events &= (uint8_t)~SYN_BUTTON_EVT_PRESS

/** Block until the button is released. */
#define PT_WAIT_BUTTON_RELEASE(pt, btn) \
    PT_WAIT_UNTIL(pt, (btn)->events & SYN_BUTTON_EVT_RELEASE); \
    (btn)->events &= (uint8_t)~SYN_BUTTON_EVT_RELEASE

#ifdef __cplusplus
}
#endif

#endif /* SYN_BUTTON_H */
