/**
 * @file syn_charlcd.c
 * @brief Generic Character LCD Driver (HD44780, ST7066, KS0066 over I2C PCF8574 or 4-Bit Parallel
 * GPIO).
 */

#include "syn_charlcd.h"

#include "../util/syn_assert.h"

#include <string.h>

#define PCF8574_BL 0x08 /**< Backlight bit mask */
#define PCF8574_EN 0x04 /**< Enable bit mask */
#define PCF8574_RW 0x02 /**< Read/Write bit mask */
#define PCF8574_RS 0x01 /**< Register Select bit mask */

/**
 * @brief Write byte to PCF8574 I2C I/O expander.
 * @param i2c Pointer to I2C interface.
 * @param dev_addr 7-bit device I2C address.
 * @param byte Data byte.
 */
static void write_i2c_byte(SYN_SoftI2C *i2c, uint8_t dev_addr, uint8_t byte)
{
    syn_soft_i2c_start(i2c);
    syn_soft_i2c_write(i2c, (uint8_t)(dev_addr << 1));
    syn_soft_i2c_write(i2c, byte);
    syn_soft_i2c_stop(i2c);
}

/**
 * @brief Send 4-bit nibble over I2C PCF8574.
 * @param lcd Pointer to LCD instance.
 * @param nibble 4-bit data nibble.
 * @param mode Control mode (RS/RW bit mask).
 */
static void send_i2c_nibble(SYN_CharLCD *lcd, uint8_t nibble, uint8_t mode)
{
    uint8_t data = (uint8_t)(nibble | mode | lcd->backlight_mask);
    write_i2c_byte(&lcd->i2c, lcd->i2c_addr, data);
    write_i2c_byte(&lcd->i2c, lcd->i2c_addr, (uint8_t)(data | PCF8574_EN));
    write_i2c_byte(&lcd->i2c, lcd->i2c_addr, (uint8_t)(data & ~PCF8574_EN));
}

/**
 * @brief Send byte as two 4-bit nibbles over I2C.
 * @param lcd Pointer to LCD instance.
 * @param val Byte value.
 * @param mode Control mode (RS/RW bit mask).
 */
static void send_i2c_byte(SYN_CharLCD *lcd, uint8_t val, uint8_t mode)
{
    send_i2c_nibble(lcd, val & 0xF0, mode);
    send_i2c_nibble(lcd, (uint8_t)((val << 4) & 0xF0), mode);
}

/**
 * @brief Send 4-bit nibble over parallel GPIO.
 * @param lcd Pointer to LCD instance.
 * @param nibble 4-bit data nibble.
 */
static void send_gpio_nibble(SYN_CharLCD *lcd, uint8_t nibble)
{
    for (int i = 0; i < 4; i++) {
        bool bit = (nibble & (1u << i)) != 0;
        syn_port_gpio_write(lcd->d_pins[i], bit ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
    }

    /* Pulse Enable High then Low */
    syn_port_gpio_write(lcd->en_pin, SYN_GPIO_HIGH);
    syn_port_gpio_write(lcd->en_pin, SYN_GPIO_LOW);
}

/**
 * @brief Send byte as two 4-bit nibbles over parallel GPIO.
 * @param lcd Pointer to LCD instance.
 * @param val Byte value.
 * @param is_data True for data, false for command.
 */
static void send_gpio_byte(SYN_CharLCD *lcd, uint8_t val, bool is_data)
{
    syn_port_gpio_write(lcd->rs_pin, is_data ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
    send_gpio_nibble(lcd, (uint8_t)(val >> 4));
    send_gpio_nibble(lcd, (uint8_t)(val & 0x0F));
}

/**
 * @brief Send command byte to LCD controller.
 * @param lcd Pointer to LCD instance.
 * @param cmd Command byte.
 */
static void write_command(SYN_CharLCD *lcd, uint8_t cmd)
{
    if (lcd->mode == SYN_CHARLCD_MODE_I2C) {
        send_i2c_byte(lcd, cmd, 0);
    } else {
        send_gpio_byte(lcd, cmd, false);
    }
}

/**
 * @brief Write character data byte to LCD controller.
 * @param lcd Pointer to LCD instance.
 * @param data Character data byte.
 */
static void write_data(SYN_CharLCD *lcd, uint8_t data)
{
    if (lcd->mode == SYN_CHARLCD_MODE_I2C) {
        send_i2c_byte(lcd, data, PCF8574_RS);
    } else {
        send_gpio_byte(lcd, data, true);
    }
}

SYN_Status syn_charlcd_init_i2c(SYN_CharLCD *lcd, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                                uint8_t i2c_addr, uint8_t cols, uint8_t rows)
{
    SYN_ASSERT(lcd != NULL);
    SYN_ASSERT(cols > 0 && rows > 0);

    memset(lcd, 0, sizeof(*lcd));
    lcd->mode = SYN_CHARLCD_MODE_I2C;
    lcd->cols = cols;
    lcd->rows = rows;
    lcd->i2c_addr = i2c_addr;
    lcd->backlight_mask = PCF8574_BL;
    lcd->backlight = true;

    syn_soft_i2c_init(&lcd->i2c, scl, sda, 10);

    /* HD44780 4-Bit Mode Initialization Sequence */
    send_i2c_nibble(lcd, 0x30, 0);
    send_i2c_nibble(lcd, 0x30, 0);
    send_i2c_nibble(lcd, 0x30, 0);
    send_i2c_nibble(lcd, 0x20, 0); /* 4-bit mode */

    write_command(lcd, 0x28); /* 2-line mode, 5x8 dots */
    write_command(lcd, 0x0C); /* Display ON, Cursor OFF, Blink OFF */
    write_command(lcd, 0x06); /* Entry mode: auto-increment */
    syn_charlcd_clear(lcd);

    return SYN_OK;
}

SYN_Status syn_charlcd_init_gpio(SYN_CharLCD *lcd, SYN_GPIO_Pin rs, SYN_GPIO_Pin en,
                                 SYN_GPIO_Pin d4, SYN_GPIO_Pin d5, SYN_GPIO_Pin d6, SYN_GPIO_Pin d7,
                                 uint8_t cols, uint8_t rows)
{
    SYN_ASSERT(lcd != NULL);
    SYN_ASSERT(cols > 0 && rows > 0);

    memset(lcd, 0, sizeof(*lcd));
    lcd->mode = SYN_CHARLCD_MODE_GPIO_4BIT;
    lcd->cols = cols;
    lcd->rows = rows;
    lcd->backlight = true;

    lcd->rs_pin = rs;
    lcd->en_pin = en;
    lcd->d_pins[0] = d4;
    lcd->d_pins[1] = d5;
    lcd->d_pins[2] = d6;
    lcd->d_pins[3] = d7;

    syn_port_gpio_init(rs, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(en, SYN_GPIO_OUTPUT);
    for (int i = 0; i < 4; i++) {
        syn_port_gpio_init(lcd->d_pins[i], SYN_GPIO_OUTPUT);
    }

    syn_port_gpio_write(rs, SYN_GPIO_LOW);
    syn_port_gpio_write(en, SYN_GPIO_LOW);

    /* HD44780 4-Bit Mode Initialization Sequence */
    send_gpio_nibble(lcd, 0x03);
    send_gpio_nibble(lcd, 0x03);
    send_gpio_nibble(lcd, 0x03);
    send_gpio_nibble(lcd, 0x02); /* 4-bit mode */

    write_command(lcd, 0x28);
    write_command(lcd, 0x0C);
    write_command(lcd, 0x06);
    syn_charlcd_clear(lcd);

    return SYN_OK;
}

void syn_charlcd_clear(SYN_CharLCD *lcd)
{
    if (lcd == NULL) {
        return;
    }
    lcd->cursor_col = 0U;
    lcd->cursor_row = 0U;
    write_command(lcd, 0x01U); /* Clear display */
}

void syn_charlcd_set_cursor(SYN_CharLCD *lcd, uint8_t col, uint8_t row)
{
    if (lcd == NULL || lcd->cols == 0U || lcd->rows == 0U) {
        return;
    }

    static const uint8_t row_offsets[] = {0x00U, 0x40U, 0x14U, 0x54U};
    if (row >= lcd->rows) {
        row = (uint8_t)(lcd->rows - 1U);
    }
    if (row >= 4U) {
        row = 3U;
    }
    if (col >= lcd->cols) {
        col = (uint8_t)(lcd->cols - 1U);
    }

    lcd->cursor_col = col;
    lcd->cursor_row = row;
    write_command(lcd, (uint8_t)(0x80U | (col + row_offsets[row])));
}

void syn_charlcd_print(SYN_CharLCD *lcd, const char *str)
{
    if (lcd == NULL || str == NULL) {
        return;
    }

    while (*str != '\0') {
        write_data(lcd, (uint8_t)(*str++));
        if (lcd->cursor_col + 1U < lcd->cols) {
            lcd->cursor_col++;
        }
    }
}

void syn_charlcd_set_backlight(SYN_CharLCD *lcd, bool enable)
{
    if (lcd == NULL || lcd->mode != SYN_CHARLCD_MODE_I2C) {
        return;
    }
    lcd->backlight = enable;
    lcd->backlight_mask = enable ? PCF8574_BL : 0x00U;
    write_i2c_byte(&lcd->i2c, lcd->i2c_addr, lcd->backlight_mask);
}

void syn_charlcd_create_char(SYN_CharLCD *lcd, uint8_t slot, const uint8_t charmap[8])
{
    if (lcd == NULL || charmap == NULL) {
        return;
    }
    slot &= 0x07U;
    write_command(lcd, (uint8_t)(0x40U | (slot << 3U)));
    for (int i = 0; i < 8; i++) {
        write_data(lcd, charmap[i]);
    }
    syn_charlcd_set_cursor(lcd, lcd->cursor_col, lcd->cursor_row);
}
