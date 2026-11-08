#include "syntropic/audio/syn_adpcm.h"
#include "unity/unity.h"

#include <string.h>

void test_adpcm_init_and_null_checks(void)
{
    SYN_ADPCM_State state;
    syn_adpcm_init(&state);
    TEST_ASSERT_EQUAL_INT16(0, state.predicted_sample);
    TEST_ASSERT_EQUAL_INT8(0, state.step_index);

    syn_adpcm_init(NULL);
    TEST_ASSERT_EQUAL_INT16(0, syn_adpcm_decode_sample(NULL, 0));
    TEST_ASSERT_EQUAL_UINT8(0, syn_adpcm_encode_sample(NULL, 100));
    TEST_ASSERT_EQUAL_UINT32(0, syn_adpcm_decode_block(NULL, NULL, NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(0, syn_adpcm_encode_block(NULL, NULL, NULL, 0));
}

void test_adpcm_encode_decode_sample_roundtrip(void)
{
    SYN_ADPCM_State enc_state, dec_state;
    syn_adpcm_init(&enc_state);
    syn_adpcm_init(&dec_state);

    int16_t input_pcm[16] = {0,   100, 250, 450, 700,  1000, 1200, 1100,
                             900, 600, 300, 100, -100, -300, -200, 0};

    for (int i = 0; i < 16; i++) {
        uint8_t nibble = syn_adpcm_encode_sample(&enc_state, input_pcm[i]);
        int16_t decoded = syn_adpcm_decode_sample(&dec_state, nibble);

        int32_t err = (int32_t)input_pcm[i] - (int32_t)decoded;
        if (err < 0) {
            err = -err;
        }
        TEST_ASSERT_TRUE(err < 2000);
    }
}

void test_adpcm_block_encode_decode(void)
{
    SYN_ADPCM_State enc_state, dec_state;
    syn_adpcm_init(&enc_state);
    syn_adpcm_init(&dec_state);

    int16_t pcm_in[32];
    for (int i = 0; i < 32; i++) {
        pcm_in[i] = (int16_t)(i * 100);
    }

    uint8_t adpcm_out[16];
    int16_t pcm_out[32];

    size_t enc_bytes = syn_adpcm_encode_block(&enc_state, pcm_in, adpcm_out, 32);
    TEST_ASSERT_EQUAL_UINT32(16, enc_bytes);

    size_t dec_samples = syn_adpcm_decode_block(&dec_state, adpcm_out, pcm_out, 32);
    TEST_ASSERT_EQUAL_UINT32(32, dec_samples);

    TEST_ASSERT_INT16_WITHIN(1500, pcm_in[0], pcm_out[0]);
    TEST_ASSERT_INT16_WITHIN(1500, pcm_in[31], pcm_out[31]);

    /* Test odd sample count block encoding & decoding (e.g. 5 samples) */
    syn_adpcm_init(&enc_state);
    syn_adpcm_init(&dec_state);
    uint8_t odd_adpcm[3] = {0};
    int16_t odd_pcm_out[5] = {0};
    size_t enc_odd = syn_adpcm_encode_block(&enc_state, pcm_in, odd_adpcm, 5);
    TEST_ASSERT_EQUAL_UINT32(3, enc_odd);
    size_t dec_odd = syn_adpcm_decode_block(&dec_state, odd_adpcm, odd_pcm_out, 5);
    TEST_ASSERT_EQUAL_UINT32(5, dec_odd);
}

void test_adpcm_clamping_and_bounds(void)
{
    SYN_ADPCM_State state;
    syn_adpcm_init(&state);
    state.step_index = 88;
    state.predicted_sample = 32000;

    /* Feed positive nibble (0x07) to exceed 32767 limit */
    for (int i = 0; i < 10; i++) {
        syn_adpcm_decode_sample(&state, 0x07);
    }
    TEST_ASSERT_EQUAL_INT16(32767, state.predicted_sample);
    TEST_ASSERT_EQUAL_INT8(88, state.step_index);

    /* Feed negative nibble (0x0F) to exceed -32768 limit */
    for (int i = 0; i < 20; i++) {
        syn_adpcm_decode_sample(&state, 0x0F);
    }
    TEST_ASSERT_EQUAL_INT16(-32768, state.predicted_sample);
    TEST_ASSERT_EQUAL_INT8(88, state.step_index);

    /* Feed 0 to bring step index down to 0 */
    for (int i = 0; i < 100; i++) {
        syn_adpcm_decode_sample(&state, 0x00);
    }
    TEST_ASSERT_EQUAL_INT8(0, state.step_index);
}

static void test_r5_adpcm_corrupt_step_index_clamped(void)
{
    SYN_ADPCM_State state;
    syn_adpcm_init(&state);

    /* Out-of-range step_index < 0 and > 88 must clamp BEFORE table lookup */
    state.step_index = -50;
    (void)syn_adpcm_decode_sample(&state, 0x00U);
    TEST_ASSERT_EQUAL_INT8(0, state.step_index);

    state.step_index = 127;
    (void)syn_adpcm_decode_sample(&state, 0x07U);
    TEST_ASSERT_EQUAL_INT8(88, state.step_index);

    state.step_index = -100;
    (void)syn_adpcm_encode_sample(&state, 500);
    TEST_ASSERT_TRUE(state.step_index >= 0 && state.step_index <= 88);

    state.step_index = 120;
    (void)syn_adpcm_encode_sample(&state, -500);
    TEST_ASSERT_TRUE(state.step_index >= 0 && state.step_index <= 88);
}

void run_adpcm_tests(void)
{
    RUN_TEST(test_adpcm_init_and_null_checks);
    RUN_TEST(test_adpcm_encode_decode_sample_roundtrip);
    RUN_TEST(test_adpcm_block_encode_decode);
    RUN_TEST(test_adpcm_clamping_and_bounds);
    RUN_TEST(test_r5_adpcm_corrupt_step_index_clamped);
}
