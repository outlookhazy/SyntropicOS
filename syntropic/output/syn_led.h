/**
 * @file syn_led.h
 * @brief Non-blocking LED controller — blink, flash, patterns.
 *
 * Drives LEDs through GPIO with non-blocking blink/flash/pattern
 * sequencing. No timers or tasks required — just call
 * syn_led_update() from your main loop.
 *
 * @par Usage
 * @code
 *   static SYN_LED led;
 *   syn_led_init(&led, LED_PIN, SYN_LED_ACTIVE_HIGH);
 *   syn_led_blink(&led, 500, 500);     // 500ms on, 500ms off
 *   while (1) { syn_led_update(&led); }
 * @endcode
 * @ingroup syn_io
 */

#ifndef SYN_LED_H
#define SYN_LED_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── LED polarity ───────────────────────────────────────────────────────── */

/** @brief LED output polarity. */
typedef enum {
    SYN_LED_ACTIVE_HIGH = 0,  /**< LED on when GPIO high */
    SYN_LED_ACTIVE_LOW  = 1,  /**< LED on when GPIO low (common anode) */
} SYN_LEDPolarity;

/* ── LED mode ───────────────────────────────────────────────────────────── */

/** @brief LED operating mode. */
typedef enum {
    SYN_LED_MODE_OFF     = 0,  /**< LED is off                        */
    SYN_LED_MODE_ON      = 1,  /**< LED is on (steady)                */
    SYN_LED_MODE_BLINK   = 2,  /**< Continuous blink                  */
    SYN_LED_MODE_FLASH   = 3,  /**< Flash N times then stop           */
    SYN_LED_MODE_PATTERN = 4,  /**< Play a pattern string             */
} SYN_LEDMode;

/* ── LED descriptor ─────────────────────────────────────────────────────── */

/** @brief LED instance — pin, mode, blink/flash/pattern state. */
typedef struct {
    /* Configuration */
    SYN_GPIO_Pin       pin;            /**< GPIO pin                    */
    uint8_t             polarity;      /**< SYN_LEDPolarity             */

    /* State */
    uint8_t             mode;          /**< SYN_LEDMode                 */
    bool                lit;           /**< Current output state          */
    uint32_t            tick;          /**< Last transition tick           */

    /* Blink / flash */
    uint16_t            on_ms;         /**< On duration (ms)              */
    uint16_t            off_ms;        /**< Off duration (ms)             */
    uint8_t             flash_remain;  /**< Remaining flashes             */

    /* Pattern */
    const char         *pattern;       /**< Pattern string pointer        */
    uint8_t             pattern_idx;   /**< Current position in pattern   */
    uint16_t            pattern_unit;  /**< Base unit time (ms)           */
} SYN_LED;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize an LED descriptor.
 *
 * @param led       LED to initialize.
 * @param pin       GPIO pin number.
 * @param polarity  Active-high or active-low.
 */
void syn_led_init(SYN_LED *led, SYN_GPIO_Pin pin, SYN_LEDPolarity polarity);

/**
 * @brief Turn the LED on (steady).
 * @param led  LED instance.
 */
void syn_led_on(SYN_LED *led);

/**
 * @brief Turn the LED off.
 * @param led  LED instance.
 */
void syn_led_off(SYN_LED *led);

/**
 * @brief Toggle the LED (only meaningful in ON/OFF modes).
 * @param led  LED instance.
 */
void syn_led_toggle(SYN_LED *led);

/**
 * @brief Start a continuous blink.
 *
 * @param led     LED.
 * @param on_ms   On time in milliseconds.
 * @param off_ms  Off time in milliseconds.
 */
void syn_led_blink(SYN_LED *led, uint16_t on_ms, uint16_t off_ms);

/**
 * @brief Flash the LED N times, then turn off.
 *
 * @param led     LED.
 * @param on_ms   On time per flash (ms).
 * @param off_ms  Off time between flashes (ms).
 * @param count   Number of flashes.
 */
void syn_led_flash(SYN_LED *led, uint16_t on_ms, uint16_t off_ms,
                    uint8_t count);

/**
 * @brief Play a pattern string.
 *
 * Pattern characters:
 *   '.'  = short on  (1 unit)
 *   '-'  = long on   (3 units)
 *   ' '  = pause     (1 unit)
 *   '|'  = long pause (3 units)
 *
 * The pattern repeats indefinitely. Set a null or empty string to stop.
 *
 * @param led       LED.
 * @param pattern   Null-terminated pattern string.
 * @param unit_ms   Base time unit in milliseconds (e.g., 100).
 */
void syn_led_pattern(SYN_LED *led, const char *pattern, uint16_t unit_ms);

/**
 * @brief Update the LED state machine.
 *
 * Call from your main loop or a scheduler task.
 *
 * @param led  LED instance.
 */
void syn_led_update(SYN_LED *led);

/**
 * @brief Service an array of LEDs.
 * @param leds   LED array.
 * @param count  Number of LEDs.
 */
void syn_led_service(SYN_LED *leds, size_t count);

/**
 * @brief Check if the LED is currently lit.
 * @param led  LED instance.
 * @return true if lit.
 */
static inline bool syn_led_is_on(const SYN_LED *led)
{
    return led->lit;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_LED_H */
