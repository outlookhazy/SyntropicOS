/**
 * @file test_vad.c
 * @brief Unit tests for Zero-Heap Acoustic Voice Activity Detector (syn_vad).
 */

#include "syntropic/audio/syn_vad.h"
#include "unity/unity.h"

#include <string.h>

void test_vad_init_and_validation(void)
{
    SYN_VAD vad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_vad_init(NULL, NULL));

    /* Default init */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_init(&vad, NULL));
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, syn_vad_get_state(&vad));
    TEST_ASSERT_EQUAL(160U, vad.cfg.frame_length);

    /* Zero-valued edge cases in cfg */
    SYN_VAD_Config zero_cfg = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_init(&vad, &zero_cfg));
    TEST_ASSERT_EQUAL(160U, vad.cfg.frame_length);
    TEST_ASSERT_EQUAL(1U, vad.cfg.attack_frames);

    /* Custom config init */
    SYN_VAD_Config cfg = {
        .sample_rate_hz = 8000U,
        .frame_length = 80U,
        .attack_frames = 3U,
        .hangover_frames = 8U,
        .sensitivity = SYN_VAD_SENSITIVITY_SENSITIVE,
        .initial_noise_floor = 100U,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_init(&vad, &cfg));
    TEST_ASSERT_EQUAL(80U, vad.cfg.frame_length);
    TEST_ASSERT_EQUAL(100U, vad.noise_floor);

    /* Sensitivity changes across all enum values */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_set_sensitivity(&vad, SYN_VAD_SENSITIVITY_AGGRESSIVE));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_set_sensitivity(&vad, SYN_VAD_SENSITIVITY_SENSITIVE));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_set_sensitivity(&vad, SYN_VAD_SENSITIVITY_NORMAL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_vad_set_sensitivity(NULL, SYN_VAD_SENSITIVITY_NORMAL));

    /* Reset */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_reset(&vad));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_vad_reset(NULL));
}

void test_vad_processing_and_state_machine(void)
{
    SYN_VAD vad;
    SYN_VAD_Config cfg = {
        .sample_rate_hz = 16000U,
        .frame_length = 160U,
        .attack_frames = 2U,
        .hangover_frames = 3U,
        .sensitivity = SYN_VAD_SENSITIVITY_NORMAL,
        .initial_noise_floor = 100U,
    };
    (void)syn_vad_init(&vad, &cfg);

    /* 1. Silence / Low background noise frame */
    int16_t silence_frame[160];
    for (size_t i = 0; i < 160; i++) {
        silence_frame[i] = (int16_t)((i % 2 == 0) ? 5 : -5);
    }

    SYN_VAD_Features feat;
    SYN_VAD_State st = syn_vad_process_frame(&vad, silence_frame, 160U, &feat);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, st);
    TEST_ASSERT_FALSE(feat.is_speech_instant);

    /* Process silence frames with zeros to trigger noise floor clamping */
    int16_t zero_frame[160] = {0};
    for (int i = 0; i < 50; i++) {
        (void)syn_vad_process_frame(&vad, zero_frame, 160U, NULL);
    }
    TEST_ASSERT_EQUAL(50U, vad.noise_floor);

    /* 2. Synthetic Speech Frame (High energy + moderate ZCR) */
    int16_t speech_frame[160];
    for (size_t i = 0; i < 160; i++) {
        speech_frame[i] = (int16_t)((i % 8 < 4) ? 2000 : -2000);
    }

    /* First speech frame: onset attack accumulating (attack_counter = 1 < 2), state still SILENCE
     */
    st = syn_vad_process_frame(&vad, speech_frame, 160U, &feat);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, st);
    TEST_ASSERT_TRUE(feat.is_speech_instant);

    /* Second speech frame: attack_counter = 2 >= 2 -> transitions to SPEECH */
    st = syn_vad_process_frame(&vad, speech_frame, 160U, &feat);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, st);

    /* 3. Hangover Transition: Speech stops -> frame is silence again */
    /* Frame 1 of silence: hangover_counter decrements (3 -> 2), state remains SPEECH */
    st = syn_vad_process_frame(&vad, silence_frame, 160U, NULL);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, st);

    /* Frame 2 of silence: hangover_counter (2 -> 1), state remains SPEECH */
    st = syn_vad_process_frame(&vad, silence_frame, 160U, NULL);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, st);

    /* Frame 3 of silence: hangover_counter (1 -> 0), state remains SPEECH */
    st = syn_vad_process_frame(&vad, silence_frame, 160U, NULL);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, st);

    /* Frame 4 of silence: hangover expired (0), state reverts to SILENCE */
    st = syn_vad_process_frame(&vad, silence_frame, 160U, NULL);
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, st);

    /* NULL / 0-length checks */
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE,
                      syn_vad_process_frame(NULL, silence_frame, 160U, NULL));
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, syn_vad_process_frame(&vad, NULL, 160U, NULL));
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, syn_vad_process_frame(&vad, silence_frame, 0U, NULL));
}

static void test_r5_vad_full_scale_no_overflow(void)
{
    SYN_VAD vad;
    SYN_VAD_Config cfg = {
        .sample_rate_hz = 16000U,
        .frame_length = 8U,
        .attack_frames = 1U,
        .hangover_frames = 1U,
        .sensitivity = SYN_VAD_SENSITIVITY_AGGRESSIVE,
        .initial_noise_floor = 500000000U,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_init(&vad, &cfg));

    /* Full-scale rail-to-rail [-32768, +32767] triggers diff = 65535 (diff^2 > INT32_MAX) */
    const int16_t rail_frame[8] = {-32768, 32767, -32768, 32767, -32768, 32767, -32768, 32767};
    SYN_VAD_Features feat;
    SYN_VAD_State st = syn_vad_process_frame(&vad, rail_frame, 8U, &feat);

    /* High noise_floor * multiplier does NOT wrap 32-bit, and EMA noise_floor * 31ULL stays ~5e8 */
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE, st);
    TEST_ASSERT_FALSE(feat.is_speech_instant);
    TEST_ASSERT_TRUE(feat.hf_energy > 3000000000U);
    TEST_ASSERT_TRUE(vad.noise_floor > 480000000U);
}

static void test_vad_noise_floor_unpoisoned_and_zcr_scaled(void)
{
    SYN_VAD vad;
    SYN_VAD_Config cfg = {
        .sample_rate_hz = 16000U,
        .frame_length = 160U,
        .attack_frames = 1U,
        .hangover_frames = 1U,
        .sensitivity = SYN_VAD_SENSITIVITY_NORMAL,
        .initial_noise_floor = 100U,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_init(&vad, &cfg));

    /* Loud DC/hum (energy = 10,000,000 >> threshold 400, ZCR = 0) is rejected by ZCR,
     * and must NOT poison background noise_floor upward. */
    int16_t hum_frame[160];
    for (size_t i = 0U; i < 160U; i++) {
        hum_frame[i] = 3162;
    }
    for (int f = 0; f < 20; f++) {
        TEST_ASSERT_EQUAL(SYN_VAD_STATE_SILENCE,
                          syn_vad_process_frame(&vad, hum_frame, 160U, NULL));
    }
    TEST_ASSERT_EQUAL_UINT32(100U, vad.noise_floor);

    /* Moderate speech frame (amplitude 100 -> energy 10,000 > 400) still triggers speech */
    int16_t speech_160[160];
    for (size_t i = 0U; i < 160U; i++) {
        speech_160[i] = (int16_t)(((i % 8U) < 4U) ? 100 : -100);
    }
    SYN_VAD_Features feat;
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, syn_vad_process_frame(&vad, speech_160, 160U, &feat));
    TEST_ASSERT_TRUE(feat.is_speech_instant);

    /* 480-sample frame (30 ms @ 16 kHz) with same 25% ZCR density (119 crossings -> ~40 per 160)
     * must also classify as speech rather than failing raw 160-sample max_zcr_threshold (100). */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vad_reset(&vad));
    int16_t speech_480[480];
    for (size_t i = 0U; i < 480U; i++) {
        speech_480[i] = (int16_t)(((i % 8U) < 4U) ? 200 : -200);
    }
    TEST_ASSERT_EQUAL(SYN_VAD_STATE_SPEECH, syn_vad_process_frame(&vad, speech_480, 480U, &feat));
    TEST_ASSERT_TRUE(feat.is_speech_instant);
}

void run_vad_tests(void)
{
    RUN_TEST(test_vad_init_and_validation);
    RUN_TEST(test_vad_processing_and_state_machine);
    RUN_TEST(test_r5_vad_full_scale_no_overflow);
    RUN_TEST(test_vad_noise_floor_unpoisoned_and_zcr_scaled);
}
