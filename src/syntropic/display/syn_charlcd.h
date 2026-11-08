/**
 * @file syn_charlcd.h
 * @brief Generic Character LCD Driver (HD44780, ST7066, KS0066 over I2C PCF8574 or 4-Bit Parallel
 * GPIO).
 * @ingroup syn_display
 */

#ifndef SYN_CHARLCD_H
#define SYN_CHARLCD_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Character LCD Interface Mode.
 */
typedef enum {
    SYN_CHARLCD_MODE_I2C = 0,  /**< I2C PCF8574 backpack adapter */
    SYN_CHARLCD_MODE_GPIO_4BIT /**< Direct 4-bit parallel GPIO bus (RS, EN, D4-D7) */
} SYN_CharLCDMode;

/**
 * @brief Generic Character LCD Context.
 */
typedef struct {
    SYN_CharLCDMode mode;    /**< Interface mode (I2C or 4-bit GPIO) */
    uint8_t cols;            /**< Display columns (e.g. 16, 20) */
    uint8_t rows;            /**< Display rows (e.g. 2, 4) */
    uint8_t display_control; /**< Display ON/OFF control state */
    uint8_t display_mode;    /**< Entry mode state */
    bool backlight;          /**< True if backlight enabled */
    uint8_t cursor_col;      /**< Cached DDRAM cursor column */
    uint8_t cursor_row;      /**< Cached DDRAM cursor row */

    /* I2C Mode Context */
    SYN_SoftI2C i2c;        /**< Software I2C bus context */
    uint8_t i2c_addr;       /**< 7-bit I2C device address */
    uint8_t backlight_mask; /**< Backlight bit mask */

    /* 4-Bit GPIO Mode Context */
    SYN_GPIO_Pin rs_pin;    /**< Register select GPIO pin */
    SYN_GPIO_Pin en_pin;    /**< Enable pulse GPIO pin */
    SYN_GPIO_Pin d_pins[4]; /**< D4, D5, D6, D7 pins */
} SYN_CharLCD;

/**
 * @brief Initialize Character LCD in I2C PCF8574 Backpack mode.
 *
 * @param lcd      LCD context.
 * @param scl      I2C SCL GPIO pin.
 * @param sda      I2C SDA GPIO pin.
 * @param i2c_addr PCF8574 I2C address (e.g. 0x27 or 0x3F).
 * @param cols     Columns (e.g. 16 or 20).
 * @param rows     Rows (e.g. 2 or 4).
 * @return SYN_OK on success.
 */
SYN_Status syn_charlcd_init_i2c(SYN_CharLCD *lcd, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                                uint8_t i2c_addr, uint8_t cols, uint8_t rows);

/**
 * @brief Initialize Character LCD in Direct 4-Bit Parallel GPIO mode.
 *
 * @param lcd      LCD context.
 * @param rs       Register Select GPIO pin.
 * @param en       Enable GPIO pin.
 * @param d4       Data Bit 4 GPIO pin.
 * @param d5       Data Bit 5 GPIO pin.
 * @param d6       Data Bit 6 GPIO pin.
 * @param d7       Data Bit 7 GPIO pin.
 * @param cols     Columns (e.g. 16 or 20).
 * @param rows     Rows (e.g. 2 or 4).
 * @return SYN_OK on success.
 */
SYN_Status syn_charlcd_init_gpio(SYN_CharLCD *lcd, SYN_GPIO_Pin rs, SYN_GPIO_Pin en,
                                 SYN_GPIO_Pin d4, SYN_GPIO_Pin d5, SYN_GPIO_Pin d6, SYN_GPIO_Pin d7,
                                 uint8_t cols, uint8_t rows);

/**
 * @brief Clear LCD screen and reset cursor to home (0,0).
 *
 * @param lcd LCD context.
 */
void syn_charlcd_clear(SYN_CharLCD *lcd);

/**
 * @brief Set cursor position.
 *
 * @param lcd LCD context.
 * @param col Column (0 to cols-1).
 * @param row Row (0 to rows-1).
 */
void syn_charlcd_set_cursor(SYN_CharLCD *lcd, uint8_t col, uint8_t row);

/**
 * @brief Print an ASCII string at current cursor position.
 *
 * @param lcd LCD context.
 * @param str Null-terminated string.
 */
void syn_charlcd_print(SYN_CharLCD *lcd, const char *str);

/**
 * @brief Turn display backlight ON or OFF (I2C backpack mode).
 *
 * @param lcd    LCD context.
 * @param enable True for backlight ON.
 */
void syn_charlcd_set_backlight(SYN_CharLCD *lcd, bool enable);

/**
 * @brief Load a custom 5x8 pixel character into CGRAM.
 *
 * @param lcd   LCD context.
 * @param slot  CGRAM slot (0 to 7).
 * @param charmap Array of 8 bytes (each byte represents a 5-bit pixel row).
 */
void syn_charlcd_create_char(SYN_CharLCD *lcd, uint8_t slot, const uint8_t charmap[8]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CHARLCD_H */
