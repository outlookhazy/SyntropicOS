/**
 * @file test_dma.c
 * @brief Unit tests for bare-metal safe DMA transaction engine (syn_dma).
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_dma.h"
#include "unity/unity.h"

#include <string.h>

static SYN_DMA g_dma;
static uint32_t s_cb_count = 0;
static SYN_DMA_Event s_last_event = (SYN_DMA_Event)0;

static void dma_test_callback(SYN_DMA *dma, SYN_DMA_Event event, void *user_ctx)
{
    (void)dma;
    (void)user_ctx;
    s_cb_count++;
    s_last_event = event;
}

static void dma_test_setup(void)
{
    s_cb_count = 0;
    s_last_event = (SYN_DMA_Event)0;
    mock_dma_start_count = 0;
    mock_dma_stop_count = 0;
    memset(mock_dma, 0, sizeof(mock_dma));

    SYN_DMA_Config cfg = {.channel_id = 0,
                          .dir = SYN_DMA_DIR_MEM_TO_MEM,
                          .data_size = SYN_DMA_SIZE_32BIT,
                          .src_inc = true,
                          .dst_inc = true,
                          .callback = dma_test_callback,
                          .user_ctx = NULL};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_init(&g_dma, &cfg));
}

void test_dma_init_invalid_params(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_init(NULL, NULL));

    SYN_DMA_Config bad_cfg = {
        .channel_id = 1, .data_size = (SYN_DMA_Size)3 /* Invalid data size */
    };
    SYN_DMA dma;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_init(&dma, &bad_cfg));
}

void test_dma_start_alignment_verification(void)
{
    dma_test_setup();
    uint32_t src_aligned[4] = {1, 2, 3, 4};
    uint32_t dst_aligned[4] = {0};

    /* Invalid params: NULL buffers or count == 0 */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(NULL, src_aligned, dst_aligned, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, NULL, dst_aligned, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, src_aligned, NULL, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, src_aligned, dst_aligned, 0));

    /* 32-bit transfer requires 4-byte aligned addresses */
    uint8_t *unaligned_src = ((uint8_t *)src_aligned) + 1;
    uint8_t *unaligned_dst = ((uint8_t *)dst_aligned) + 2;

    /* Unaligned src */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, unaligned_src, dst_aligned, 2));

    /* Unaligned dst */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, src_aligned, unaligned_dst, 2));

    /* Aligned src & dst */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&g_dma, src_aligned, dst_aligned, 4));
    TEST_ASSERT_EQUAL_INT(1, mock_dma_start_count);
    TEST_ASSERT_EQUAL_INT(1, dst_aligned[0]);
    TEST_ASSERT_EQUAL_INT(4, dst_aligned[3]);
}

void test_dma_busy_rejection(void)
{
    dma_test_setup();
    uint32_t src[2] = {10, 20};
    uint32_t dst[2] = {0};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&g_dma, src, dst, 2));
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_BUSY, syn_dma_get_state(&g_dma));

    /* Submitting another transfer while busy should return SYN_BUSY */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_dma_start(&g_dma, src, dst, 2));
}

void test_dma_isr_handler_events(void)
{
    dma_test_setup();
    uint32_t src[2] = {100, 200};
    uint32_t dst[2] = {0};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&g_dma, src, dst, 2));

    /* Simulate Half-Complete ISR event */
    syn_dma_isr_handler(&g_dma, SYN_DMA_EVENT_HALF_COMPLETE);
    TEST_ASSERT_EQUAL_INT(1, s_cb_count);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_EVENT_HALF_COMPLETE, s_last_event);

    /* Simulate Complete ISR event */
    syn_dma_isr_handler(&g_dma, SYN_DMA_EVENT_COMPLETE);
    TEST_ASSERT_EQUAL_INT(2, s_cb_count);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_EVENT_COMPLETE, s_last_event);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_COMPLETE, syn_dma_get_state(&g_dma));
    TEST_ASSERT_EQUAL_UINT32(1, g_dma.transfers_cnt);

    /* Reset state for error test */
    g_dma.state = SYN_DMA_STATE_IDLE;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&g_dma, src, dst, 2));

    /* Simulate Error ISR event */
    syn_dma_isr_handler(&g_dma, SYN_DMA_EVENT_ERROR);
    TEST_ASSERT_EQUAL_INT(3, s_cb_count);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_EVENT_ERROR, s_last_event);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_ERROR, syn_dma_get_state(&g_dma));
    TEST_ASSERT_EQUAL_UINT32(1, g_dma.errors_cnt);
}

void test_dma_stop_and_null_checks(void)
{
    dma_test_setup();
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_stop(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_ERROR, syn_dma_get_state(NULL));

    syn_dma_isr_handler(NULL, SYN_DMA_EVENT_COMPLETE);

    /* Hardware port error trigger */
    g_dma.cfg.channel_id = 99; /* Out of range for mock hardware port */
    uint32_t src[1] = {1};
    uint32_t dst[1] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_dma_start(&g_dma, src, dst, 1));
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_ERROR, syn_dma_get_state(&g_dma));
    TEST_ASSERT_EQUAL_UINT32(1, g_dma.errors_cnt);

    g_dma.cfg.channel_id = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_stop(&g_dma));
    TEST_ASSERT_EQUAL_INT(1, mock_dma_stop_count);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_IDLE, syn_dma_get_state(&g_dma));
}

static void test_dma_ringbuf(void)
{
    SYN_DMA dma;
    SYN_DMA_Config cfg = {0};
    cfg.channel_id = 1;
    cfg.dir = SYN_DMA_DIR_PERIPH_TO_MEM;
    cfg.data_size = SYN_DMA_SIZE_8BIT;
    cfg.dst_inc = true;
    syn_dma_init(&dma, &cfg);

    uint8_t rx_buf[64] = {0};
    uint8_t dummy_periph[64] = {0xAA};

    SYN_DMA_RingBuf rbuf = {0};
    /* NULL guards */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dma_ringbuf_init(NULL, &dma, rx_buf, 64));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dma_ringbuf_start(NULL, &dummy_periph));
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_bytes_available(NULL));
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_read(NULL, rx_buf, 10));
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_read(&rbuf, NULL, 10));
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_read(&rbuf, rx_buf, 0));

    TEST_ASSERT_EQUAL(SYN_OK, syn_dma_ringbuf_init(&rbuf, &dma, rx_buf, sizeof(rx_buf)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_dma_ringbuf_start(&rbuf, &dummy_periph));

    /* Mock 10 bytes written into rx_buf */
    memcpy(rx_buf, "1234567890", 10);
    mock_dma[1].remaining = 54; /* 64 - 54 = 10 bytes written */
    TEST_ASSERT_EQUAL(10, syn_dma_ringbuf_bytes_available(&rbuf));

    uint8_t out[16] = {0};
    size_t read_cnt = syn_dma_ringbuf_read(&rbuf, out, 5);
    TEST_ASSERT_EQUAL(5, read_cnt);
    TEST_ASSERT_EQUAL(5, rbuf.tail);
    TEST_ASSERT_EQUAL(5, syn_dma_ringbuf_bytes_available(&rbuf));

    /* Test wraparound case where head < tail */
    rbuf.tail = 60;
    mock_dma[1].remaining = 59; /* head = 5, tail = 60 -> avail = 64 - 55 = 9 */
    TEST_ASSERT_EQUAL(9, syn_dma_ringbuf_bytes_available(&rbuf));

    /* Read across ring buffer boundary split */
    memcpy(&rx_buf[60], "ABCD", 4);
    memcpy(&rx_buf[0], "EFGH", 4);
    uint8_t wrap_out[16] = {0};
    size_t wrap_read = syn_dma_ringbuf_read(&rbuf, wrap_out, 8);
    TEST_ASSERT_EQUAL(8, wrap_read);
    TEST_ASSERT_EQUAL_MEMORY("ABCDEFGH", wrap_out, 8);

    /* Empty buffer read (head == tail) */
    mock_dma[1].remaining = 60; /* head = 64 - 60 = 4 == tail */
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_read(&rbuf, out, 5));
}

static void test_dma_ringbuf_counter_overflow_guard(void)
{
    SYN_DMA dma;
    SYN_DMA_Config cfg = {.channel_id = 1, .data_size = SYN_DMA_SIZE_8BIT};
    syn_dma_init(&dma, &cfg);

    uint8_t rx_buf[16] = {0};
    SYN_DMA_RingBuf rbuf;
    syn_dma_ringbuf_init(&rbuf, &dma, rx_buf, 16);

    /* Test remaining > capacity -> head returns 0 */
    mock_dma[1].remaining = 999;
    TEST_ASSERT_EQUAL(0, syn_dma_ringbuf_bytes_available(&rbuf));
}

static void test_dma_periph_non_incrementing_transfers(void)
{
    SYN_DMA dma;
    uint32_t tx_buf[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    uint32_t periph_tx_reg = 0U;

    SYN_DMA_Config tx_cfg = {.channel_id = 0,
                             .dir = SYN_DMA_DIR_MEM_TO_PERIPH,
                             .data_size = SYN_DMA_SIZE_32BIT,
                             .src_inc = true,
                             .dst_inc = false,
                             .callback = dma_test_callback,
                             .user_ctx = NULL};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_init(&dma, &tx_cfg));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&dma, tx_buf, &periph_tx_reg, 4));
    TEST_ASSERT_EQUAL_UINT32(0x44U, periph_tx_reg);
    syn_dma_isr_handler(&dma, SYN_DMA_EVENT_COMPLETE);
    TEST_ASSERT_EQUAL_INT(SYN_DMA_STATE_COMPLETE, syn_dma_get_state(&dma));

    const uint32_t periph_rx_reg = 0xCAFEBABEU;
    uint32_t rx_buf[3] = {0};
    SYN_DMA_Config rx_cfg = {.channel_id = 0,
                             .dir = SYN_DMA_DIR_PERIPH_TO_MEM,
                             .data_size = SYN_DMA_SIZE_32BIT,
                             .src_inc = false,
                             .dst_inc = true,
                             .callback = NULL,
                             .user_ctx = NULL};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_init(&dma, &rx_cfg));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dma_start(&dma, &periph_rx_reg, rx_buf, 3));
    TEST_ASSERT_EQUAL_UINT32(0xCAFEBABEU, rx_buf[0]);
    TEST_ASSERT_EQUAL_UINT32(0xCAFEBABEU, rx_buf[2]);
    syn_dma_isr_handler(&dma, SYN_DMA_EVENT_COMPLETE);
}

static void test_r5_dma_ringbuf_full_wrap_drain(void)
{
    dma_test_setup();
    SYN_DMA dma;
    SYN_DMA_Config cfg = {0};
    cfg.channel_id = 1;
    cfg.dir = SYN_DMA_DIR_PERIPH_TO_MEM;
    cfg.data_size = SYN_DMA_SIZE_8BIT;
    cfg.dst_inc = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dma_init(&dma, &cfg));

    uint8_t rx_buf[64];
    uint8_t periph_byte = 0x5AU;
    SYN_DMA_RingBuf rbuf;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dma_ringbuf_init(&rbuf, &dma, rx_buf, sizeof(rx_buf)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_dma_ringbuf_start(&rbuf, &periph_byte));

    /* Before any byte arrives, remaining == capacity -> 0 bytes available */
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));

    /* Hardware fills entire 64B buffer so remaining == 0 */
    for (size_t i = 0; i < sizeof(rx_buf); i++) {
        rx_buf[i] = (uint8_t)(i + 1U);
    }
    mock_dma[1].remaining = 0U;
    TEST_ASSERT_EQUAL_UINT32(64U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));

    /* Read first 20 bytes, then remaining 44 bytes */
    uint8_t out[64];
    memset(out, 0, sizeof(out));
    TEST_ASSERT_EQUAL_UINT32(20U, (uint32_t)syn_dma_ringbuf_read(&rbuf, out, 20U));
    TEST_ASSERT_EQUAL_UINT32(44U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));
    TEST_ASSERT_EQUAL_UINT32(44U, (uint32_t)syn_dma_ringbuf_read(&rbuf, out + 20, 44U));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(rx_buf, out, sizeof(rx_buf));

    /* R5-S12: Drained full buffer with remaining == 0 must NOT replay 64B! */
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_dma_ringbuf_read(&rbuf, out, sizeof(out)));

    /* Subsequent hardware wrap writes 10 fresh bytes (remaining = 54), then fills to remaining = 0
     */
    for (size_t i = 0; i < sizeof(rx_buf); i++) {
        rx_buf[i] = (uint8_t)(0xA0U + i);
    }
    mock_dma[1].remaining = 54U;
    TEST_ASSERT_EQUAL_UINT32(10U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));
    /* Hardware finishes 2nd full 64B wrap before CPU reads -> must report 64B, not wedge at 0! */
    mock_dma[1].remaining = 0U;
    TEST_ASSERT_EQUAL_UINT32(64U, (uint32_t)syn_dma_ringbuf_bytes_available(&rbuf));

    mock_cache_invalidate_count = 0U;
    TEST_ASSERT_EQUAL_UINT32(64U, (uint32_t)syn_dma_ringbuf_read(&rbuf, out, sizeof(out)));
    TEST_ASSERT_TRUE(mock_cache_invalidate_count > 0U);
    TEST_ASSERT_EQUAL_UINT8(0xA0U, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0xA9U, out[9]);
}

void run_dma_tests(void)
{
    dma_test_setup();
    RUN_TEST(test_dma_init_invalid_params);
    RUN_TEST(test_dma_start_alignment_verification);
    RUN_TEST(test_dma_busy_rejection);
    RUN_TEST(test_dma_isr_handler_events);
    RUN_TEST(test_dma_stop_and_null_checks);
    RUN_TEST(test_dma_ringbuf);
    RUN_TEST(test_dma_ringbuf_counter_overflow_guard);
    RUN_TEST(test_dma_periph_non_incrementing_transfers);
    RUN_TEST(test_r5_dma_ringbuf_full_wrap_drain);
}
