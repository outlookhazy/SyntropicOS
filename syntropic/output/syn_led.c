#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_LED) || SYN_USE_LED

/**
 * @file syn_led.c
 * @brief LED controller implementation.
 */

#include "syn_led.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Internal helpers ───────────────────────────────────────────────────── */

/**
 * @brief Set the LED GPIO output based on polarity.
 * @param led  LED instance.
 * @param on   true to light the LED.
 */
static void led_set_output(SYN_LED *led, bool on)
{
    led->lit = on;
    SYN_GPIO_State level;
    if (led->polarity == (uint8_t)SYN_LED_ACTIVE_LOW) {
        level = on ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    } else {
        level = on ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    }
    syn_port_gpio_write(led->pin, level);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_led_init(SYN_LED *led, SYN_GPIO_Pin pin, SYN_LEDPolarity polarity)
{
    SYN_ASSERT(led != NULL);

    memset(led, 0, sizeof(*led));
    led->pin      = pin;
    led->polarity = (uint8_t)polarity;
    led->mode     = (uint8_t)SYN_LED_MODE_OFF;
    led->lit      = false;
    led_set_output(led, false);
}

void syn_led_on(SYN_LED *led)
{
    SYN_ASSERT(led != NULL);
    led->mode = (uint8_t)SYN_LED_MODE_ON;
    led_set_output(led, true);
}

void syn_led_off(SYN_LED *led)
{
    SYN_ASSERT(led != NULL);
    led->mode = (uint8_t)SYN_LED_MODE_OFF;
    led_set_output(led, false);
}

void syn_led_toggle(SYN_LED *led)
{
    SYN_ASSERT(led != NULL);
    if (led->lit) {
        syn_led_off(led);
    } else {
        syn_led_on(led);
    }
}

void syn_led_blink(SYN_LED *led, uint16_t on_ms, uint16_t off_ms)
{
    SYN_ASSERT(led != NULL);

    led->mode   = (uint8_t)SYN_LED_MODE_BLINK;
    led->on_ms  = on_ms;
    led->off_ms = off_ms;
    led->tick   = syn_port_get_tick_ms();
    led_set_output(led, true);
}

void syn_led_flash(SYN_LED *led, uint16_t on_ms, uint16_t off_ms,
                    uint8_t count)
{
    SYN_ASSERT(led != NULL);
    SYN_ASSERT(count > 0);

    led->mode         = (uint8_t)SYN_LED_MODE_FLASH;
    led->on_ms        = on_ms;
    led->off_ms       = off_ms;
    led->flash_remain = count;
    led->tick         = syn_port_get_tick_ms();
    led_set_output(led, true);
}

void syn_led_pattern(SYN_LED *led, const char *pattern, uint16_t unit_ms)
{
    SYN_ASSERT(led != NULL);

    if (pattern == NULL || pattern[0] == '\0') {
        syn_led_off(led);
        return;
    }

    led->mode         = (uint8_t)SYN_LED_MODE_PATTERN;
    led->pattern      = pattern;
    led->pattern_idx  = 0;
    led->pattern_unit = unit_ms;
    led->tick         = syn_port_get_tick_ms();
    led_set_output(led, false);
}

/* ── Update ─────────────────────────────────────────────────────────────── */

void syn_led_update(SYN_LED *led)
{
    SYN_ASSERT(led != NULL);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t elapsed = now - led->tick;

    switch ((SYN_LEDMode)led->mode) {

    case SYN_LED_MODE_OFF:
    case SYN_LED_MODE_ON:
        /* Static — nothing to do */
        break;

    case SYN_LED_MODE_BLINK:
        if (led->lit && elapsed >= led->on_ms) {
            led_set_output(led, false);
            led->tick = now;
        } else if (!led->lit && elapsed >= led->off_ms) {
            led_set_output(led, true);
            led->tick = now;
        }
        break;

    case SYN_LED_MODE_FLASH:
        if (led->lit && elapsed >= led->on_ms) {
            led_set_output(led, false);
            led->tick = now;
            led->flash_remain--;
            if (led->flash_remain == 0) {
                led->mode = (uint8_t)SYN_LED_MODE_OFF;
            }
        } else if (!led->lit && elapsed >= led->off_ms) {
            if (led->flash_remain > 0) {
                led_set_output(led, true);
                led->tick = now;
            }
        }
        break;

    case SYN_LED_MODE_PATTERN: {
        char ch = led->pattern[led->pattern_idx];
        uint16_t dur;

        switch (ch) {
        case '.':  /* short on */
            if (!led->lit) {
                led_set_output(led, true);
                led->tick = now;
            } else if (elapsed >= led->pattern_unit) {
                led_set_output(led, false);
                led->tick = now;
                led->pattern_idx++;
            }
            break;
        case '-':  /* long on */
            if (!led->lit) {
                led_set_output(led, true);
                led->tick = now;
            } else if (elapsed >= (uint32_t)(led->pattern_unit * 3u)) {
                led_set_output(led, false);
                led->tick = now;
                led->pattern_idx++;
            }
            break;
        case ' ':  /* short pause */
            dur = led->pattern_unit;
            if (elapsed >= dur) {
                led->pattern_idx++;
                led->tick = now;
            }
            break;
        case '|':  /* long pause */
            dur = (uint16_t)(led->pattern_unit * 3u);
            if (elapsed >= dur) {
                led->pattern_idx++;
                led->tick = now;
            }
            break;
        case '\0': /* end of pattern — loop */
            led->pattern_idx = 0;
            led->tick = now;
            break;
        default:   /* unknown char — skip */
            led->pattern_idx++;
            break;
        }
        break;
    }
    }
}

void syn_led_service(SYN_LED *leds, size_t count)
{
    SYN_ASSERT(leds != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        syn_led_update(&leds[i]);
    }
}

#endif /* SYN_USE_LED */
