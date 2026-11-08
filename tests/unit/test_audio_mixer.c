#include "syntropic/audio/syn_audio_mixer.h"
#include "unity/unity.h"

#include <string.h>

static SYN_Audio_Mixer g_mixer;

void test_audio_mixer_init_and_null_checks(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_audio_mixer_init(NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_init(&g_mixer));
    TEST_ASSERT_EQUAL_UINT16(SYN_AUDIO_GAIN_UNITY, g_mixer.master_volume_q15);

    int16_t out[16] = {0};
    TEST_ASSERT_EQUAL(0, syn_audio_mixer_render(NULL, out, 16));
    TEST_ASSERT_EQUAL(0, syn_audio_mixer_render(&g_mixer, NULL, 16));
    TEST_ASSERT_EQUAL(0, syn_audio_mixer_render(&g_mixer, out, 0));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_audio_mixer_stop(NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_audio_mixer_stop(&g_mixer, SYN_AUDIO_MIXER_MAX_CHANNELS));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_audio_mixer_set_channel_volume(NULL, 0, 1000));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_audio_mixer_set_channel_volume(
                                             &g_mixer, SYN_AUDIO_MIXER_MAX_CHANNELS, 1000));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_audio_mixer_set_master_volume(NULL, 1000));
}

void test_audio_mixer_play_invalid_params(void)
{
    int16_t buf[8] = {10, 20, 30, 40};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_audio_mixer_play(NULL, 0, buf, 4, SYN_AUDIO_GAIN_UNITY, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_audio_mixer_play(&g_mixer, 0, NULL, 4, SYN_AUDIO_GAIN_UNITY, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_audio_mixer_play(&g_mixer, 0, buf, 0, SYN_AUDIO_GAIN_UNITY, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_audio_mixer_play(&g_mixer, SYN_AUDIO_MIXER_MAX_CHANNELS, buf, 4,
                                           SYN_AUDIO_GAIN_UNITY, false));
}

void test_audio_mixer_single_channel_render(void)
{
    syn_audio_mixer_init(&g_mixer);
    int16_t pcm_in[4] = {1000, -2000, 3000, -4000};
    int16_t pcm_out[4] = {0};

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_audio_mixer_play(&g_mixer, 0, pcm_in, 4, SYN_AUDIO_GAIN_UNITY, false));
    TEST_ASSERT_EQUAL(4, syn_audio_mixer_render(&g_mixer, pcm_out, 4));

    TEST_ASSERT_EQUAL_INT16(1000, pcm_out[0]);
    TEST_ASSERT_EQUAL_INT16(-2000, pcm_out[1]);
    TEST_ASSERT_EQUAL_INT16(3000, pcm_out[2]);
    TEST_ASSERT_EQUAL_INT16(-4000, pcm_out[3]);
}

void test_audio_mixer_multi_channel_mixing_and_gain(void)
{
    syn_audio_mixer_init(&g_mixer);
    int16_t ch0_pcm[4] = {10000, 10000, 10000, 10000};
    int16_t ch1_pcm[4] = {20000, 20000, 20000, 20000};
    int16_t pcm_out[4] = {0};

    /* Channel 0 at 50% gain (16384 Q15), Channel 1 at 100% gain (32768 Q15) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_play(&g_mixer, 0, ch0_pcm, 4, 16384, false));
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_play(&g_mixer, 1, ch1_pcm, 4, 32768, false));

    TEST_ASSERT_EQUAL(4, syn_audio_mixer_render(&g_mixer, pcm_out, 4));

    /* 5000 + 20000 = 25000 */
    TEST_ASSERT_EQUAL_INT16(25000, pcm_out[0]);
    TEST_ASSERT_EQUAL_INT16(25000, pcm_out[1]);

    /* Change channel 0 volume */
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_set_channel_volume(&g_mixer, 0, 32768));
    /* Change master volume to 50% */
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_set_master_volume(&g_mixer, 16384));

    /* Re-play buffers */
    syn_audio_mixer_play(&g_mixer, 0, ch0_pcm, 4, 32768, false);
    syn_audio_mixer_play(&g_mixer, 1, ch1_pcm, 4, 32768, false);

    syn_audio_mixer_render(&g_mixer, pcm_out, 4);

    /* (10000 + 20000) * 0.5 = 15000 */
    TEST_ASSERT_EQUAL_INT16(15000, pcm_out[0]);
}

void test_audio_mixer_saturation_clipping(void)
{
    syn_audio_mixer_init(&g_mixer);
    int16_t ch0_pcm[2] = {25000, -25000};
    int16_t ch1_pcm[2] = {20000, -20000};
    int16_t pcm_out[2] = {0};

    syn_audio_mixer_play(&g_mixer, 0, ch0_pcm, 2, SYN_AUDIO_GAIN_UNITY, false);
    syn_audio_mixer_play(&g_mixer, 1, ch1_pcm, 2, SYN_AUDIO_GAIN_UNITY, false);

    syn_audio_mixer_render(&g_mixer, pcm_out, 2);

    /* 25000 + 20000 = 45000 -> clipped to 32767 */
    TEST_ASSERT_EQUAL_INT16(32767, pcm_out[0]);
    /* -25000 + -20000 = -45000 -> clipped to -32768 */
    TEST_ASSERT_EQUAL_INT16(-32768, pcm_out[1]);
}

void test_audio_mixer_looping_and_stop(void)
{
    syn_audio_mixer_init(&g_mixer);
    int16_t ch0_pcm[2] = {100, 200};
    int16_t pcm_out[4] = {0};

    /* Play with loop = true */
    syn_audio_mixer_play(&g_mixer, 0, ch0_pcm, 2, SYN_AUDIO_GAIN_UNITY, true);
    syn_audio_mixer_render(&g_mixer, pcm_out, 4);

    TEST_ASSERT_EQUAL_INT16(100, pcm_out[0]);
    TEST_ASSERT_EQUAL_INT16(200, pcm_out[1]);
    TEST_ASSERT_EQUAL_INT16(100, pcm_out[2]);
    TEST_ASSERT_EQUAL_INT16(200, pcm_out[3]);

    /* Stop channel */
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_stop(&g_mixer, 0));
    memset(pcm_out, 0, sizeof(pcm_out));
    syn_audio_mixer_render(&g_mixer, pcm_out, 4);

    TEST_ASSERT_EQUAL_INT16(0, pcm_out[0]);
    TEST_ASSERT_EQUAL_INT16(0, pcm_out[1]);

    /* Non-looping playback completion */
    int16_t ch1_pcm[2] = {50, 60};
    syn_audio_mixer_play(&g_mixer, 0, ch1_pcm, 2, SYN_AUDIO_GAIN_UNITY, false);
    syn_audio_mixer_render(&g_mixer, pcm_out, 4);
    TEST_ASSERT_EQUAL_INT16(50, pcm_out[0]);
    TEST_ASSERT_EQUAL_INT16(60, pcm_out[1]);
    TEST_ASSERT_EQUAL_INT16(0, pcm_out[2]);
    TEST_ASSERT_EQUAL_INT16(0, pcm_out[3]);
    TEST_ASSERT_FALSE(g_mixer.channels[0].active);
}

void test_r5_audio_mixer_overflow_and_immediate_deactivation(void)
{
    syn_audio_mixer_init(&g_mixer);
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_set_master_volume(&g_mixer, 65535U));

    int16_t pos_pcm[4] = {32767, 32767, 32767, 32767};
    int16_t neg_pcm[4] = {-32768, -32768, -32768, -32768};
    int16_t out[4] = {0};

    for (uint8_t i = 0U; i < 4U; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_play(&g_mixer, i, pos_pcm, 4U, 65535U, false));
    }

    /* Render exact 4 samples: no 32-bit overflow, immediate channel deactivation on sample 4 */
    TEST_ASSERT_EQUAL(4U, syn_audio_mixer_render(&g_mixer, out, 4U));
    for (size_t i = 0U; i < 4U; i++) {
        TEST_ASSERT_EQUAL_INT16(32767, out[i]);
    }
    for (uint8_t i = 0U; i < 4U; i++) {
        TEST_ASSERT_FALSE(g_mixer.channels[i].active);
    }

    /* Negative full-scale multi-channel gain without 32-bit signed overflow */
    for (uint8_t i = 0U; i < 4U; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_play(&g_mixer, i, neg_pcm, 4U, 65535U, false));
    }
    TEST_ASSERT_EQUAL(4U, syn_audio_mixer_render(&g_mixer, out, 4U));
    for (size_t i = 0U; i < 4U; i++) {
        TEST_ASSERT_EQUAL_INT16(-32768, out[i]);
    }

    /* Defensive out-of-bounds pos guard coverage */
    TEST_ASSERT_EQUAL(SYN_OK, syn_audio_mixer_play(&g_mixer, 0U, pos_pcm, 4U, 32768U, false));
    g_mixer.channels[0].pos = 99U;
    memset(out, 0, sizeof(out));
    TEST_ASSERT_EQUAL(1U, syn_audio_mixer_render(&g_mixer, out, 1U));
    TEST_ASSERT_EQUAL_INT16(0, out[0]);
    TEST_ASSERT_FALSE(g_mixer.channels[0].active);
}

void run_audio_mixer_tests(void)
{
    RUN_TEST(test_audio_mixer_init_and_null_checks);
    RUN_TEST(test_audio_mixer_play_invalid_params);
    RUN_TEST(test_audio_mixer_single_channel_render);
    RUN_TEST(test_audio_mixer_multi_channel_mixing_and_gain);
    RUN_TEST(test_audio_mixer_saturation_clipping);
    RUN_TEST(test_audio_mixer_looping_and_stop);
    RUN_TEST(test_r5_audio_mixer_overflow_and_immediate_deactivation);
}
