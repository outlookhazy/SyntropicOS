/**
 * @file test_oled.c
 * @brief Unity tests for syn_oled module.
 */

#include "mocks/mock_port.h"
#include "syntropic/display/syn_oled.h"
#include "unity/unity.h"

static void test_oled_controllers(void)
{
    mock_port_reset();
    SYN_OLED oled;

    /* SSD1306 Initialization */
    SYN_Status st = syn_oled_init(&oled, 0, 1, 0x3C, 128, 64, SYN_OLED_SSD1306);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    uint8_t buf[128 * 64 / 8];
    SYN_Canvas c;
    syn_canvas_init(&c, buf, 128, 64, 1, syn_oled_canvas_flush_cb, &oled);

    syn_canvas_fill(&c, 1);
    syn_oled_flush_canvas(&oled, &c);

    /* Test control functions */
    syn_oled_set_contrast(&oled, 200);
    TEST_ASSERT_EQUAL_UINT8(200, oled.contrast);

    syn_oled_set_invert(&oled, true);
    TEST_ASSERT_TRUE(oled.inverted);

    syn_oled_set_display_on(&oled, false);
    TEST_ASSERT_FALSE(oled.display_on);

    /* SH1106 Initialization (with 2-pixel column shift) */
    st = syn_oled_init(&oled, 0, 1, 0x3C, 128, 64, SYN_OLED_SH1106);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT8(2, oled.col_offset);
    syn_oled_flush_canvas(&oled, &c);

    /* SSD1309 Initialization */
    st = syn_oled_init(&oled, 0, 1, 0x3C, 128, 64, SYN_OLED_SSD1309);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    /* SSD1306 128x32 Initialization (non-64 height COM pin config branch) */
    st = syn_oled_init(&oled, 0, 1, 0x3C, 128, 32, SYN_OLED_SSD1306);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_UINT16(32, oled.height);

    /* NULL safety guards */
    syn_oled_set_contrast(NULL, 0);
    syn_oled_set_invert(NULL, false);
    syn_oled_set_display_on(NULL, false);
    syn_oled_flush_canvas(NULL, NULL);
    syn_oled_canvas_flush_cb(NULL, 0, NULL);
}

static void test_r5_oled_flush_layout_and_callback(void)
{
    mock_port_reset();
    SYN_OLED oled;
    TEST_ASSERT_EQUAL(SYN_OK, syn_oled_init(&oled, 0, 1, 0x3C, 16, 10, SYN_OLED_SSD1306));

    /* 16x10 OLED -> 2 pages * 16 = 32 bytes */
    uint8_t fb[16 * 2];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 16, 10, 1, syn_oled_canvas_flush_cb, &oled);
    syn_canvas_clear(&c);
    syn_canvas_pixel(&c, 0, 0, 1);
    syn_canvas_pixel(&c, 15, 9, 1);
    TEST_ASSERT_EQUAL_HEX8(0x01U, fb[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02U, fb[16 + 15]);

    /* Flush via SYN_Canvas callback (syn_oled_canvas_flush_cb) */
    syn_canvas_flush(&c);

    /* Partial buffer & smaller-width canvas & 16-bpp rejection */
    syn_oled_canvas_flush_cb(fb, 8U, &oled);
    TEST_ASSERT_EQUAL_UINT16(8U, oled.flush_offset);
    syn_oled_canvas_flush_cb(&fb[8], 24U, &oled);
    TEST_ASSERT_EQUAL_UINT16(0U, oled.flush_offset);
    syn_oled_canvas_flush_cb(fb, 0U, &oled);
    oled.width = 0U;
    syn_oled_canvas_flush_cb(fb, 8U, &oled);
    oled.width = 16U;

    uint8_t small_fb[8 * 2];
    SYN_Canvas small_c;
    syn_canvas_init(&small_c, small_fb, 8, 10, 1, NULL, NULL);
    syn_canvas_clear(&small_c);
    syn_oled_flush_canvas(&oled, &small_c);

    uint8_t rgb_fb[8 * 8 * 2];
    SYN_Canvas rgb_c;
    syn_canvas_init(&rgb_c, rgb_fb, 8, 8, 16, NULL, NULL);
    syn_oled_flush_canvas(&oled, &rgb_c);

    syn_oled_set_invert(&oled, false);
    syn_oled_set_display_on(&oled, true);
}

void run_oled_tests(void)
{
    RUN_TEST(test_oled_controllers);
    RUN_TEST(test_r5_oled_flush_layout_and_callback);
}
