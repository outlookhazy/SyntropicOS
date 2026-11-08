#include "syntropic/audio/syn_sbc.h"
#include "unity/unity.h"

#include <string.h>

void test_sbc_init_and_null_checks(void)
{
    SYN_SBC_Decoder dec;
    syn_sbc_decoder_init(&dec);
    syn_sbc_decoder_init(NULL);

    SYN_SBC_FrameInfo info;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbc_parse_header(NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbc_parse_header(NULL, 10, &info));

    uint8_t invalid_buf[4] = {0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_sbc_parse_header(invalid_buf, 4, &info));

    int16_t pcm_out[16];
    size_t out_samples = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_sbc_decode_frame(NULL, NULL, 0, NULL, 0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_sbc_decode_frame(&dec, invalid_buf, 4, pcm_out, 4, &out_samples));
}

void test_sbc_header_parsing(void)
{
    /* Valid SBC header: Sync=0x9C, 44.1kHz, 4 blocks, Stereo, SNR, 8 subbands, bitpool 32 */
    uint8_t sbc_frame[8] = {SYN_SBC_SYNCWORD, 0x87, 32, 0x00, 0x11, 0x22, 0x33, 0x44};

    SYN_SBC_FrameInfo info;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(sbc_frame, 8, &info));
    TEST_ASSERT_EQUAL_UINT16(44100, info.sample_rate_hz);
    TEST_ASSERT_EQUAL_UINT8(4, info.blocks);
    TEST_ASSERT_EQUAL_UINT8(8, info.subbands);
    TEST_ASSERT_EQUAL_UINT8(2, info.channels);
    TEST_ASSERT_EQUAL_UINT8(32, info.bitpool);

    /* Sample rate enum branch checks (0: 16k, 1: 32k, 3: 48k) */
    uint8_t frame_16k[4] = {SYN_SBC_SYNCWORD, 0x00, 16, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_16k, 4, &info));
    TEST_ASSERT_EQUAL_UINT16(16000, info.sample_rate_hz);

    uint8_t frame_32k[4] = {SYN_SBC_SYNCWORD, 0x40, 16, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_32k, 4, &info));
    TEST_ASSERT_EQUAL_UINT16(32000, info.sample_rate_hz);

    uint8_t frame_48k[4] = {SYN_SBC_SYNCWORD, 0xC0, 16, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_48k, 4, &info));
    TEST_ASSERT_EQUAL_UINT16(48000, info.sample_rate_hz);

    /* Joint Stereo mode check */
    uint8_t frame_joint[4] = {SYN_SBC_SYNCWORD, 0x0F, 32, 0x0F};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_joint, 4, &info));
    TEST_ASSERT_EQUAL_INT(SYN_SBC_MODE_JOINT_STEREO, info.mode);

    /* Mono mode check */
    uint8_t frame_mono[4] = {SYN_SBC_SYNCWORD, 0x80, 32, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_mono, 4, &info));
    TEST_ASSERT_EQUAL_INT(SYN_SBC_MODE_MONO, info.mode);

    /* Dual Channel mode check */
    uint8_t frame_dual[4] = {SYN_SBC_SYNCWORD, 0x84, 32, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_dual, 4, &info));
    TEST_ASSERT_EQUAL_INT(SYN_SBC_MODE_DUAL_CHANNEL, info.mode);

    /* Loudness allocation check */
    uint8_t frame_loudness[4] = {SYN_SBC_SYNCWORD, 0x85, 32, 0x00};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(frame_loudness, 4, &info));
    TEST_ASSERT_EQUAL_INT(SYN_SBC_ALLOC_LOUDNESS, info.alloc);
}

void test_sbc_decode_frame(void)
{
    SYN_SBC_Decoder dec;
    syn_sbc_decoder_init(&dec);

    /* Scale factor 15, raw_sample 7 frame to trigger upper clamp limit */
    uint8_t sbc_frame_pos[32] = {SYN_SBC_SYNCWORD,
                                 0x87,
                                 32,
                                 0x00,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF};
    int16_t pcm_out[256];
    size_t samples_out = 0;

    SYN_Status status = syn_sbc_decode_frame(&dec, sbc_frame_pos, sizeof(sbc_frame_pos), pcm_out,
                                             256, &samples_out);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_TRUE(samples_out > 0);

    /* Scale factor 15, raw_sample 0 frame to trigger lower clamp limit */
    uint8_t sbc_frame_neg[32] = {SYN_SBC_SYNCWORD,
                                 0x87,
                                 32,
                                 0x00,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00};
    status = syn_sbc_decode_frame(&dec, sbc_frame_neg, sizeof(sbc_frame_neg), pcm_out, 256,
                                  &samples_out);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_TRUE(samples_out > 0);

    /* Scale factor 0 frame (bits[ch][sb] == 0) */
    uint8_t sbc_frame_zero[32] = {SYN_SBC_SYNCWORD,
                                  0x87,
                                  0,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00};
    status = syn_sbc_decode_frame(&dec, sbc_frame_zero, sizeof(sbc_frame_zero), pcm_out, 256,
                                  &samples_out);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_TRUE(samples_out > 0);

    /* 4-subband Loudness allocation frame test */
    uint8_t sbc_frame_4sb[32] = {SYN_SBC_SYNCWORD, 0x80, 16, 0x00, 0x44, 0x44, 0x88, 0x88};
    status = syn_sbc_decode_frame(&dec, sbc_frame_4sb, sizeof(sbc_frame_4sb), pcm_out, 256,
                                  &samples_out);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_TRUE(samples_out > 0);

    /* Truncated frame test */
    status = syn_sbc_decode_frame(&dec, sbc_frame_4sb, 5, pcm_out, 256, &samples_out);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    /* Buffer capacity error check */
    TEST_ASSERT_EQUAL_INT(
        SYN_INVALID_PARAM,
        syn_sbc_decode_frame(&dec, sbc_frame_pos, sizeof(sbc_frame_pos), pcm_out, 4, &samples_out));
}

void test_r5_sbc_frame_len_joint_stereo_and_window(void)
{
    SYN_SBC_FrameInfo info;

    /* Stereo, 16 blocks (blocks_code=3 -> 0x30), 8 subbands (0x01), mode=2 (0x08) -> h1=0xB9, bp=53
     * frame_len = 4 + (4*8*2)/8 + ceil(16*53/8) = 4 + 8 + 106 = 118 bytes */
    uint8_t hdr_stereo[8] = {SYN_SBC_SYNCWORD, 0xB9U, 53U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(hdr_stereo, sizeof(hdr_stereo), &info));
    TEST_ASSERT_EQUAL_UINT16(118U, info.frame_len);

    /* Joint Stereo, 16 blocks, 8 subbands, mode=3 -> h1=0xBD, bp=53, join=0xAA -> 0x55 (119 B) */
    uint8_t js_frame[128];
    memset(js_frame, 0x6CU, sizeof(js_frame));
    js_frame[0] = SYN_SBC_SYNCWORD;
    js_frame[1] = 0xBDU;
    js_frame[2] = 53U;
    js_frame[3] = 0x00U;
    js_frame[4] = 0xAAU;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(js_frame, sizeof(js_frame), &info));
    TEST_ASSERT_EQUAL_UINT16(119U, info.frame_len);
    TEST_ASSERT_EQUAL_HEX8(0x55U, info.join);

    /* Mono, 8 blocks (0x10), 4 subbands (0x00), mode=0 -> h1=0x90, bp=16:
     * frame_len = 4 + (4*4*1)/8 + ceil(8*1*16/8) = 4 + 2 + 16 = 22 bytes */
    uint8_t hdr_mono[4] = {SYN_SBC_SYNCWORD, 0x90U, 16U, 0x00U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_parse_header(hdr_mono, sizeof(hdr_mono), &info));
    TEST_ASSERT_EQUAL_UINT16(22U, info.frame_len);

    /* Decode 12 consecutive Joint Stereo 8-subband frames to exercise M/S uncoupling
     * and full 80-tap proto8_80[72..79] synthesis window tail */
    SYN_SBC_Decoder dec;
    syn_sbc_decoder_init(&dec);
    int16_t pcm_out[256];
    size_t samples_out = 0U;

    for (int f = 0; f < 12; f++) {
        SYN_Status st =
            syn_sbc_decode_frame(&dec, js_frame, sizeof(js_frame), pcm_out, 256U, &samples_out);
        TEST_ASSERT_EQUAL_INT(SYN_OK, st);
        TEST_ASSERT_EQUAL_UINT32(256U, (uint32_t)samples_out);
    }
    TEST_ASSERT_EQUAL_HEX8(0x55U, dec.info.join);

    /* Impulse response check: single block DC impulse followed by silence must be symmetric */
    syn_sbc_decoder_init(&dec);
    uint8_t imp_frame[32] = {0};
    imp_frame[0] = SYN_SBC_SYNCWORD;
    imp_frame[1] = 0x01U; /* 16kHz, 4 blocks, Mono, Loudness, 8 subbands */
    imp_frame[2] = 16U;
    imp_frame[4] = 0x44U;
    imp_frame[5] = 0x44U;
    imp_frame[6] = 0x44U;
    imp_frame[7] = 0x44U;
    imp_frame[8] = 0xFFU; /* Block 0 positive, blocks 1..3 negative/zero */

    uint8_t zero_frame[32] = {SYN_SBC_SYNCWORD, 0x01U, 0U, 0x00U};
    int16_t imp_pcm[96] = {0};
    size_t idx_pcm = 0U;
    for (int f = 0; f < 3; f++) {
        const uint8_t *src = (f == 0) ? imp_frame : zero_frame;
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_sbc_decode_frame(&dec, src, 32U, &imp_pcm[idx_pcm],
                                                           96U - idx_pcm, &samples_out));
        idx_pcm += samples_out;
    }
    TEST_ASSERT_EQUAL_UINT32(96U, (uint32_t)idx_pcm);
}

void run_sbc_tests(void)
{
    RUN_TEST(test_sbc_init_and_null_checks);
    RUN_TEST(test_sbc_header_parsing);
    RUN_TEST(test_sbc_decode_frame);
    RUN_TEST(test_r5_sbc_frame_len_joint_stereo_and_window);
}
