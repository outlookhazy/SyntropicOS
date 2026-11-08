#include "syntropic/audio/syn_audio.h"
#include "unity/unity.h"

#include <string.h>

static int16_t g_pcm_buf[256];
static int g_out_cb_calls = 0;
static size_t g_last_cb_count = 0;

static void dummy_audio_output(const int16_t *samples, size_t count, void *ctx)
{
    (void)samples;
    (void)ctx;
    g_out_cb_calls++;
    g_last_cb_count = count;
}

void test_audio_init_and_null_checks(void)
{
    SYN_Audio audio;
    SYN_Audio_Config cfg;

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_audio_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_audio_init(&audio, NULL));

    memset(&cfg, 0, sizeof(cfg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_audio_init(&audio, &cfg));

    cfg.buf = g_pcm_buf;
    cfg.buf_capacity = 256;
    cfg.half_size = 128;
    cfg.sample_rate_hz = 44100;
    cfg.channels = 2;
    cfg.out_fn = dummy_audio_output;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_audio_init(&audio, &cfg));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_audio_start(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_audio_stop(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, syn_audio_feed(NULL, NULL, 0));
    syn_audio_isr_half(NULL);
    syn_audio_isr_complete(NULL);
}

void test_audio_start_stop_and_feed(void)
{
    SYN_Audio audio;
    SYN_Audio_Config cfg;
    cfg.buf = g_pcm_buf;
    cfg.buf_capacity = 256;
    cfg.half_size = 128;
    cfg.sample_rate_hz = 16000;
    cfg.channels = 1;
    cfg.out_fn = dummy_audio_output;

    syn_audio_init(&audio, &cfg);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_audio_start(&audio));
    TEST_ASSERT_EQUAL_INT(1, g_out_cb_calls);
    TEST_ASSERT_EQUAL_UINT32(128, g_last_cb_count);

    int16_t input_samples[256];
    for (int i = 0; i < 256; i++) {
        input_samples[i] = (int16_t)(i * 10);
    }

    size_t fed = syn_audio_feed(&audio, input_samples, 256);
    TEST_ASSERT_EQUAL_UINT32(256, fed);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_audio_stop(&audio));
}

void test_audio_isr_handlers(void)
{
    SYN_Audio audio;
    SYN_Audio_Config cfg;
    cfg.buf = g_pcm_buf;
    cfg.buf_capacity = 256;
    cfg.half_size = 128;
    cfg.sample_rate_hz = 48000;
    cfg.channels = 2;
    cfg.out_fn = dummy_audio_output;

    syn_audio_init(&audio, &cfg);
    syn_audio_start(&audio);

    int prev_calls = g_out_cb_calls;
    syn_audio_isr_half(&audio);
    TEST_ASSERT_EQUAL_INT(prev_calls + 1, g_out_cb_calls);

    syn_audio_isr_complete(&audio);
    TEST_ASSERT_EQUAL_INT(prev_calls + 2, g_out_cb_calls);
}

typedef struct {
    int16_t captured[2][4];
    const int16_t *ptrs[2];
    uint32_t calls;
} AudioFlushCapture;

static void capture_audio_output(const int16_t *samples, size_t count, void *ctx)
{
    AudioFlushCapture *cap = (AudioFlushCapture *)ctx;
    if (cap != NULL && cap->calls < 2U && count == 4U) {
        memcpy(cap->captured[cap->calls], samples, 4U * sizeof(int16_t));
        cap->ptrs[cap->calls] = samples;
    }
    if (cap != NULL) {
        cap->calls++;
    }
}

void test_r5_audio_double_buffer_flush_order(void)
{
    int16_t dbl_buf[8] = {0};
    AudioFlushCapture cap;
    memset(&cap, 0, sizeof(cap));

    SYN_Audio audio;
    SYN_Audio_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.buf = dbl_buf;
    cfg.buf_capacity = 8U;
    cfg.half_size = 4U;
    cfg.sample_rate_hz = 16000U;
    cfg.channels = 1U;
    cfg.out_fn = capture_audio_output;
    cfg.ctx = &cap;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_audio_init(&audio, &cfg));

    /* Feed while stopped: toggles active_half without invoking out_fn */
    int16_t pre_samples[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_UINT32(4U, syn_audio_feed(&audio, pre_samples, 4U));
    TEST_ASSERT_EQUAL_UINT32(0U, cap.calls);
    TEST_ASSERT_EQUAL_UINT8(1U, audio.active_half);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_audio_start(&audio));
    cap.calls = 0U;

    /* Feed exact first half: flushes half 0 immediately with fed samples, then switches to half 1
     */
    int16_t first_half[4] = {101, 102, 103, 104};
    TEST_ASSERT_EQUAL_UINT32(4U, syn_audio_feed(&audio, first_half, 4U));
    TEST_ASSERT_EQUAL_UINT32(1U, cap.calls);
    TEST_ASSERT_EQUAL_PTR(&dbl_buf[0], cap.ptrs[0]);
    TEST_ASSERT_EQUAL_INT16(101, cap.captured[0][0]);
    TEST_ASSERT_EQUAL_INT16(104, cap.captured[0][3]);
    TEST_ASSERT_EQUAL_UINT8(1U, audio.active_half);
    TEST_ASSERT_EQUAL_UINT32(0U, audio.fill_pos);

    /* Feed exact second half: flushes half 1 with fed samples, then switches back to half 0 */
    int16_t second_half[4] = {201, 202, 203, 204};
    TEST_ASSERT_EQUAL_UINT32(4U, syn_audio_feed(&audio, second_half, 4U));
    TEST_ASSERT_EQUAL_UINT32(2U, cap.calls);
    TEST_ASSERT_EQUAL_PTR(&dbl_buf[4], cap.ptrs[1]);
    TEST_ASSERT_EQUAL_INT16(201, cap.captured[1][0]);
    TEST_ASSERT_EQUAL_INT16(204, cap.captured[1][3]);
    TEST_ASSERT_EQUAL_UINT8(0U, audio.active_half);
}

void run_audio_tests(void)
{
    RUN_TEST(test_audio_init_and_null_checks);
    RUN_TEST(test_audio_start_stop_and_feed);
    RUN_TEST(test_audio_isr_handlers);
    RUN_TEST(test_r5_audio_double_buffer_flush_order);
}
