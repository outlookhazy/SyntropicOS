/**
 * @file test_canvas.c
 * @brief Unity tests for syn_canvas.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/display/syn_canvas.h"

static int cvs_flush_n = 0;
static size_t cvs_flush_sz = 0;
static void cvs_flush(const uint8_t *b, size_t l, void *c)
    { (void)b; (void)c; cvs_flush_n++; cvs_flush_sz = l; }

static void test_canvas(void)
{
    uint8_t fb[32 * 16 / 8];
    SYN_Canvas c;
    memset(fb, 0, sizeof(fb));
    cvs_flush_n = 0;

    syn_canvas_init(&c, fb, 32, 16, 1, cvs_flush, NULL);
    TEST_ASSERT_EQUAL_INT(32, c.width);
    TEST_ASSERT_EQUAL_INT(16, c.height);

    /* Fill / clear */
    syn_canvas_fill(&c, 1);
    TEST_ASSERT_EQUAL_HEX8(0xFF, fb[0]);
    syn_canvas_clear(&c);
    TEST_ASSERT_EQUAL_HEX8(0x00, fb[0]);

    /* Pixel set */
    syn_canvas_pixel(&c, 0, 0, 1);
    TEST_ASSERT_TRUE((fb[0] & 0x01) != 0);

    /* Out-of-bounds pixels — should not crash */
    syn_canvas_pixel(&c, -1, 0, 1);
    syn_canvas_pixel(&c, 100, 100, 1);

    /* Horizontal line */
    syn_canvas_clear(&c);
    syn_canvas_line(&c, 0, 0, 31, 0, 1);
    int lok = 1;
    for (int x = 0; x < 32; x++) {
        if (!(fb[x] & 0x01)) { lok = 0; break; }
    }
    TEST_ASSERT_TRUE(lok);

    /* Filled rect */
    syn_canvas_clear(&c);
    syn_canvas_rect_fill(&c, 0, 0, 2, 2, 1);
    TEST_ASSERT_TRUE((fb[0] & 0x01) && (fb[1] & 0x01) &&
                     (fb[0] & 0x02) && (fb[1] & 0x02));

    /* Circle draws some pixels */
    syn_canvas_clear(&c);
    syn_canvas_circle(&c, 16, 8, 5, 1);
    int cpx = 0;
    for (size_t i = 0; i < sizeof(fb); i++) {
        for (int b = 0; b < 8; b++) {
            if (fb[i] & (1 << b)) cpx++;
        }
    }
    TEST_ASSERT_TRUE(cpx > 0);

    /* Text rendering */
    syn_canvas_clear(&c);
    syn_canvas_text(&c, 0, 0, "A", 1);
    TEST_ASSERT_TRUE(fb[0] != 0);
    TEST_ASSERT_EQUAL_INT(5, syn_canvas_text_width(&c, "A"));

    /* Flush callback */
    syn_canvas_flush(&c);
    TEST_ASSERT_EQUAL_INT(1, cvs_flush_n);
    TEST_ASSERT_EQUAL_size_t(sizeof(fb), cvs_flush_sz);

    /* Test syn_canvas_circle_fill */
    syn_canvas_clear(&c);
    syn_canvas_circle_fill(&c, 16, 8, 4, 1);
    /* Center (16,8) is at page 1, x=16, which is idx 48, bit 0x01 */
    TEST_ASSERT_TRUE(fb[48] & 0x01);
    
    /* Test syn_canvas_rect_round and filled version */
    syn_canvas_clear(&c);
    syn_canvas_rect_round(&c, 0, 0, 10, 10, 3, 1);
    /* Outer corner (0,0) should be 0 because it's rounded off */
    TEST_ASSERT_FALSE(fb[0] & 0x01);
    /* Inner pixel (3, 0) on top side should be 1 */
    TEST_ASSERT_TRUE(fb[3] & 0x01);

    syn_canvas_clear(&c);
    syn_canvas_rect_round_fill(&c, 0, 0, 10, 10, 3, 1);
    /* Center of filled round rect (5, 5) should be filled (page 0, x=5, bit 0x20) */
    TEST_ASSERT_TRUE(fb[5] & 0x20);

    /* Test syn_canvas_bitmap */
    syn_canvas_clear(&c);
    const uint8_t icon[2] = { 0xC0, 0x03 }; /* 11000000 00000011 */
    syn_canvas_bitmap(&c, 0, 0, icon, 16, 1, 1);
    /* check bits at (0,0), (1,0) are set, (2,0) is not, and (14,0), (15,0) are set.
     * All are on page 0 (y=0), so bit is 0x01. */
    TEST_ASSERT_TRUE(fb[0] & 0x01); /* x=0 */
    TEST_ASSERT_TRUE(fb[1] & 0x01); /* x=1 */
    TEST_ASSERT_FALSE(fb[2] & 0x01); /* x=2 */
    TEST_ASSERT_TRUE(fb[14] & 0x01); /* x=14 */
    TEST_ASSERT_TRUE(fb[15] & 0x01); /* x=15 */

    /* RGB565 mode */
    uint8_t fbc[8 * 8 * 2];
    SYN_Canvas cc;
    syn_canvas_init(&cc, fbc, 8, 8, 16, NULL, NULL);
    syn_canvas_clear(&cc);
    syn_canvas_pixel(&cc, 0, 0, SYN_COLOR_RED);
    TEST_ASSERT_EQUAL_HEX8(0xF8, fbc[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, fbc[1]);
    TEST_ASSERT_EQUAL_HEX16(SYN_COLOR_RED, syn_rgb565(255, 0, 0));
}

/* ── flush_partial ───────────────────────────────────────────────────────── */

/* Extended mock: also records the buf pointer and length */
static const uint8_t *partial_flush_buf = NULL;
static size_t         partial_flush_len = 0;
static int            partial_flush_n   = 0;
static void partial_flush_fn(const uint8_t *b, size_t l, void *ctx)
{
    (void)ctx;
    partial_flush_buf = b;
    partial_flush_len = l;
    partial_flush_n++;
}

static void test_canvas_flush_partial_basic(void)
{
    uint8_t fb[64];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 64, 8, 1, partial_flush_fn, NULL);

    partial_flush_buf = NULL;
    partial_flush_len = 0;
    partial_flush_n   = 0;

    /* Flush 16 bytes starting at offset 8 */
    syn_canvas_flush_partial(&c, 8u, 16u);

    TEST_ASSERT_EQUAL_INT(1, partial_flush_n);
    TEST_ASSERT_EQUAL_size_t(16u, partial_flush_len);
    /* buf pointer must point into framebuffer at +8 */
    TEST_ASSERT_EQUAL_PTR(fb + 8, partial_flush_buf);
}

static void test_canvas_flush_partial_clamps_to_buf_size(void)
{
    uint8_t fb[64];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 64, 8, 1, partial_flush_fn, NULL);

    partial_flush_n   = 0;
    partial_flush_len = 0;

    /* Ask for 32 bytes at offset 48 — only 16 remain, must clamp */
    syn_canvas_flush_partial(&c, 48u, 32u);

    TEST_ASSERT_EQUAL_INT(1, partial_flush_n);
    TEST_ASSERT_EQUAL_size_t(16u, partial_flush_len);  /* clamped */
}

static void test_canvas_flush_partial_offset_past_end_is_noop(void)
{
    uint8_t fb[64];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 64, 8, 1, partial_flush_fn, NULL);

    partial_flush_n = 0;

    /* Offset past the buffer — must not call flush_fn at all */
    syn_canvas_flush_partial(&c, 64u, 8u);
    syn_canvas_flush_partial(&c, 999u, 1u);

    TEST_ASSERT_EQUAL_INT(0, partial_flush_n);
}

static void test_canvas_flush_partial_simulated_chunked_loop(void)
{
    /* Simulate the coroutine pattern: flush a 64-byte buffer in 16-byte chunks */
    uint8_t fb[64];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 64, 8, 1, partial_flush_fn, NULL);

    partial_flush_n = 0;

    size_t pos   = 0u;
    size_t chunk = 16u;
    while (pos < c.buf_size) {
        syn_canvas_flush_partial(&c, pos, chunk);
        pos += chunk;
        /* PT_YIELD(pt) would go here in real code */
    }

    /* Expect exactly 4 flush_fn calls covering the whole buffer */
    TEST_ASSERT_EQUAL_INT(4, partial_flush_n);
    /* Last call covered bytes 48..63 — buf pointer at fb+48 */
    TEST_ASSERT_EQUAL_PTR(fb + 48, partial_flush_buf);
    TEST_ASSERT_EQUAL_size_t(16u, partial_flush_len);
}

/* ── clip rect ───────────────────────────────────────────────────────────── */

static void test_canvas_clip_rect(void)
{
    uint8_t fb[32 * 16 / 8];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 32, 16, 1, NULL, NULL);

    /* Default clip rect should be full display */
    TEST_ASSERT_EQUAL_INT(0,  c.clip_x);
    TEST_ASSERT_EQUAL_INT(0,  c.clip_y);
    TEST_ASSERT_EQUAL_INT(32, c.clip_w);
    TEST_ASSERT_EQUAL_INT(16, c.clip_h);

    /* Set clip rect to a 4x4 region at (8,4) */
    syn_canvas_clear(&c);
    syn_canvas_set_clip(&c, 8, 4, 4, 4);

    /* Pixel inside clip rect — should be written */
    syn_canvas_pixel(&c, 9, 5, 1);
    /* byte index = x + (y/8)*width = 9 + 0*32 = 9, bit = y%8 = 5 */
    TEST_ASSERT_TRUE(fb[9] & (1 << 5));

    /* Pixel outside clip rect — should NOT be written */
    syn_canvas_pixel(&c, 0, 0, 1);
    TEST_ASSERT_FALSE(fb[0] & 0x01);

    /* Pixel at clip boundary (x = 12, which is 8+4, exclusive) */
    syn_canvas_pixel(&c, 12, 5, 1);
    TEST_ASSERT_FALSE(fb[12] & (1 << 5));
}

static void test_canvas_clip_rect_reset(void)
{
    uint8_t fb[32 * 16 / 8];
    SYN_Canvas c;
    syn_canvas_init(&c, fb, 32, 16, 1, NULL, NULL);
    syn_canvas_clear(&c);

    /* Set a restrictive clip rect */
    syn_canvas_set_clip(&c, 8, 4, 4, 4);

    /* Pixel at (0,0) should be rejected */
    syn_canvas_pixel(&c, 0, 0, 1);
    TEST_ASSERT_FALSE(fb[0] & 0x01);

    /* Reset clip rect */
    syn_canvas_reset_clip(&c);

    /* Now pixel at (0,0) should work */
    syn_canvas_pixel(&c, 0, 0, 1);
    TEST_ASSERT_TRUE(fb[0] & 0x01);
}

/** syn_canvas_set_font — exercises lines 176-180 */
static void test_canvas_set_font(void)
{
    static uint8_t fb2[32];
    SYN_Canvas c;
    syn_canvas_init(&c, fb2, 32, 8, 1, cvs_flush, NULL);

    /* Set to the built-in font */
    syn_canvas_set_font(&c, &syn_font_5x7);
    TEST_ASSERT_EQUAL_PTR(&syn_font_5x7, c.font);

    /* Set to NULL — should revert to default */
    syn_canvas_set_font(&c, NULL);
    TEST_ASSERT_EQUAL_PTR(&syn_font_5x7, c.font);
}

/** 16-bit color canvas fill — exercises lines 195-199 (2-byte fill path) */
static void test_canvas_16bit_clear(void)
{
    static uint8_t fb16[64]; /* 32 pixels x 2 bytes each */
    SYN_Canvas c;
    /* bpp=16 for 16-bit display (buf_size must hold w*h*2 bytes) */
    syn_canvas_init(&c, fb16, 32, 1, 16, cvs_flush, NULL);

    /* Color > 0xFF triggers hi/lo byte path (lines 195-199) */
    syn_canvas_fill(&c, 0xF800u); /* Red in RGB565 */
    /* First two bytes should be 0xF8 and 0x00 */
    TEST_ASSERT_EQUAL_HEX8(0xF8u, fb16[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, fb16[1]);
}

void run_canvas_tests(void)
{
    RUN_TEST(test_canvas);
    RUN_TEST(test_canvas_flush_partial_basic);
    RUN_TEST(test_canvas_flush_partial_clamps_to_buf_size);
    RUN_TEST(test_canvas_flush_partial_offset_past_end_is_noop);
    RUN_TEST(test_canvas_flush_partial_simulated_chunked_loop);
    RUN_TEST(test_canvas_clip_rect);
    RUN_TEST(test_canvas_clip_rect_reset);
    RUN_TEST(test_canvas_set_font);
    RUN_TEST(test_canvas_16bit_clear);
}
