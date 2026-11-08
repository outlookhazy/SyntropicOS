/**
 * @file test_charlcd.c
 * @brief Unity tests for syn_charlcd module.
 */

#include "mocks/mock_port.h"
#include "syntropic/display/syn_charlcd.h"
#include "unity/unity.h"

static void test_charlcd_i2c_mode(void)
{
    mock_port_reset();
    SYN_CharLCD lcd;

    SYN_Status st = syn_charlcd_init_i2c(&lcd, 0, 1, 0x27, 16, 2);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    syn_charlcd_set_cursor(&lcd, 2, 1);
    syn_charlcd_print(&lcd, "I2C Test");
    syn_charlcd_set_backlight(&lcd, true);
    syn_charlcd_set_backlight(&lcd, false);

    uint8_t glyph[8] = {0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00};
    syn_charlcd_create_char(&lcd, 0, glyph);
    syn_charlcd_clear(&lcd);

    /* NULL safety guards */
    syn_charlcd_clear(NULL);
    syn_charlcd_set_cursor(NULL, 0, 0);
    syn_charlcd_print(NULL, NULL);
    syn_charlcd_set_backlight(NULL, true);
    syn_charlcd_create_char(NULL, 0, NULL);
}

static void test_charlcd_gpio_mode(void)
{
    mock_port_reset();
    SYN_CharLCD lcd;

    SYN_Status st = syn_charlcd_init_gpio(&lcd, 0, 1, 2, 3, 4, 5, 20, 4);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    syn_charlcd_set_cursor(&lcd, 99, 99);
    syn_charlcd_print(&lcd, "GPIO Mode");
    syn_charlcd_clear(&lcd);
}

static void test_r5_charlcd_create_char_restores_ddram(void)
{
    mock_port_reset();
    SYN_CharLCD lcd;

    TEST_ASSERT_EQUAL(SYN_OK, syn_charlcd_init_gpio(&lcd, 0, 1, 2, 3, 4, 5, 16, 2));

    /* Position cursor at (col=3, row=1) and print 2 chars -> cursor advances to (5, 1) */
    syn_charlcd_set_cursor(&lcd, 3U, 1U);
    syn_charlcd_print(&lcd, "Hi");
    TEST_ASSERT_EQUAL_UINT8(5U, lcd.cursor_col);
    TEST_ASSERT_EQUAL_UINT8(1U, lcd.cursor_row);

    /* Load custom glyph into CGRAM; must restore DDRAM cursor (0x80 | (0x40 + 5) = 0xC5) */
    const uint8_t glyph[8] = {0x1FU, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x1FU, 0x0AU};
    syn_charlcd_create_char(&lcd, 2U, glyph);

    /* RS pin (0) must end LOW (command) with low nibble of 0xC5 (0x05) on D4..D7 (pins 2..5) */
    TEST_ASSERT_EQUAL_UINT8(5U, lcd.cursor_col);
    TEST_ASSERT_EQUAL_UINT8(1U, lcd.cursor_row);
    TEST_ASSERT_EQUAL_INT(SYN_GPIO_LOW, mock_gpio_states[0]);
    uint8_t low_nibble =
        (uint8_t)((mock_gpio_states[2] ? 1U : 0U) | (mock_gpio_states[3] ? 2U : 0U) |
                  (mock_gpio_states[4] ? 4U : 0U) | (mock_gpio_states[5] ? 8U : 0U));
    TEST_ASSERT_EQUAL_HEX8(0x05U, low_nibble);

    /* Print to right edge to exercise cursor_col clamp + rows > 4 clamp + cols/rows == 0 guard */
    syn_charlcd_set_cursor(&lcd, 15U, 0U);
    syn_charlcd_print(&lcd, "AB");
    TEST_ASSERT_EQUAL_UINT8(15U, lcd.cursor_col);

    SYN_CharLCD lcd_tall;
    TEST_ASSERT_EQUAL(SYN_OK, syn_charlcd_init_gpio(&lcd_tall, 0, 1, 2, 3, 4, 5, 20, 6));
    syn_charlcd_set_cursor(&lcd_tall, 5U, 5U);
    TEST_ASSERT_EQUAL_UINT8(3U, lcd_tall.cursor_row);

    lcd_tall.rows = 0U;
    syn_charlcd_set_cursor(&lcd_tall, 0U, 0U);
}

void run_charlcd_tests(void)
{
    RUN_TEST(test_charlcd_i2c_mode);
    RUN_TEST(test_charlcd_gpio_mode);
    RUN_TEST(test_r5_charlcd_create_char_restores_ddram);
}
