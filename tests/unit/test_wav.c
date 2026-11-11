#include "syntropic/audio/syn_wav.h"
#include "unity/unity.h"

#include <string.h>

void test_wav_parse_null_and_bounds_checks(void)
{
    SYN_WAV_Info info;
    uint8_t dummy_buf[44] = {0};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wav_parse_header(NULL, 44, &info));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wav_parse_header(dummy_buf, 44, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_wav_parse_header(dummy_buf, 43, &info));
}

void test_wav_parse_pcm_valid_header(void)
{
    /* 44-byte standard PCM WAV header */
    uint8_t wav_hdr[44] = {
        'R',  'I',  'F', 'F', 36,  0,   0,   0,                /* ChunkSize = 36 */
        'W',  'A',  'V', 'E', 'f', 'm', 't', ' ', 16, 0, 0, 0, /* Subchunk1Size = 16 */
        1,    0,                                               /* AudioFormat = 1 (PCM) */
        1,    0,                                               /* NumChannels = 1 (Mono) */
        0x80, 0x3E, 0,   0,                                    /* SampleRate = 16000 Hz */
        0x00, 0x7D, 0,   0,                                    /* ByteRate = 32000 */
        2,    0,                                               /* BlockAlign = 2 */
        16,   0,                                               /* BitsPerSample = 16 */
        'd',  'a',  't', 'a', 100, 0,   0,   0                 /* Subchunk2Size = 100 bytes data */
    };

    SYN_WAV_Info info;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wav_parse_header(wav_hdr, sizeof(wav_hdr), &info));

    TEST_ASSERT_EQUAL_UINT16(SYN_WAV_FORMAT_PCM, info.audio_format);
    TEST_ASSERT_EQUAL_UINT16(1, info.num_channels);
    TEST_ASSERT_EQUAL_UINT32(16000, info.sample_rate);
    TEST_ASSERT_EQUAL_UINT32(32000, info.byte_rate);
    TEST_ASSERT_EQUAL_UINT16(2, info.block_align);
    TEST_ASSERT_EQUAL_UINT16(16, info.bits_per_sample);
    TEST_ASSERT_EQUAL_UINT32(44, info.data_offset);
    TEST_ASSERT_EQUAL_UINT32(100, info.data_size);
    TEST_ASSERT_EQUAL_UINT32(50, info.total_samples);
}

void test_wav_parse_ima_adpcm_header(void)
{
    uint8_t wav_hdr[44] = {
        'R',  'I',  'F', 'F', 36,  0,  0, 0, 'W', 'A',  'V',
        'E',  'f',  'm', 't', ' ', 16, 0, 0, 0,   0x11, 0, /* AudioFormat = 0x11 (IMA ADPCM) */
        1,    0,                                           /* NumChannels = 1 */
        0x40, 0x1F, 0,   0,                                /* SampleRate = 8000 Hz */
        0x00, 0x10, 0,   0,                                /* ByteRate */
        1,    0,                                           /* BlockAlign = 1 */
        4,    0,                                           /* BitsPerSample = 4 */
        'd',  'a',  't', 'a', 200, 0,  0, 0                /* Subchunk2Size = 200 bytes */
    };

    SYN_WAV_Info info;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wav_parse_header(wav_hdr, sizeof(wav_hdr), &info));
    TEST_ASSERT_EQUAL_UINT16(SYN_WAV_FORMAT_IMA_ADPCM, info.audio_format);
    TEST_ASSERT_EQUAL_UINT32(400, info.total_samples);
}

void test_wav_parse_invalid_magics_and_truncated(void)
{
    uint8_t bad_riff[44] = {'X', 'I', 'F', 'F'};
    SYN_WAV_Info info;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(bad_riff, 44, &info));

    uint8_t bad_wave[44] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'B', 'A', 'D', ' '};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(bad_wave, 44, &info));

    /* Truncated fmt chunk header */
    uint8_t trunc_fmt[44] = {'R', 'I', 'F', 'F', 36,  0,   0,   0, 'W', 'A',
                             'V', 'E', 'f', 'm', 't', ' ', 100, 0, 0,   0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(trunc_fmt, 44, &info));

    /* Truncated fmt chunk payload at offset 32 (buffer_size = 44) */
    uint8_t trunc_fmt_payload[44] = {
        'R', 'I', 'F', 'F', 36, 0, 0, 0, 'W', 'A', 'V', 'E', 'J', 'U', 'N', 'K', 12, 0, 0, 0,
        0,   0,   0,   0,   0,  0, 0, 0, 0,   0,   0,   0,   'f', 'm', 't', ' ', 16, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_wav_parse_header(trunc_fmt_payload, sizeof(trunc_fmt_payload), &info));

    /* Odd chunk size padding test + data chunk */
    uint8_t odd_chunk_wav[] = {
        'R', 'I', 'F',  'F',  60, 0, 0, 0, 'W', 'A', 'V', 'E', 'J',  'U',
        'N', 'K', 3,    0,    0,  0, 1, 2, 3,   0, /* 3 bytes payload + 1 pad byte */
        'f', 'm', 't',  ' ',  16, 0, 0, 0, 1,   0,   1,   0,   0x80, 0x3E,
        0,   0,   0x00, 0x7D, 0,  0, 2, 0, 16,  0,   'd', 'a', 't',  'a',
        10,  0,   0,    0,    0,  0, 0, 0, 0,   0,   0,   0,   0,    0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_wav_parse_header(odd_chunk_wav, sizeof(odd_chunk_wav), &info));

    /* Missing data chunk */
    uint8_t no_data_wav[] = {'R',  'I',  'F', 'F', 36,   0,    0, 0, 'W', 'A', 'V', 'E',
                             'f',  'm',  't', ' ', 16,   0,    0, 0, 1,   0,   1,   0,
                             0x80, 0x3E, 0,   0,   0x00, 0x7D, 0, 0, 2,   0,   16,  0,
                             'N',  'O',  'P', 'E', 4,    0,    0, 0, 0,   0,   0,   0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(no_data_wav, sizeof(no_data_wav), &info));

    /* Zero num_channels test (Line 90 guard) */
    uint8_t zero_chan_wav[] = {'R',  'I',  'F', 'F', 44,   0,    0, 0, 'W', 'A', 'V', 'E',
                               'f',  'm',  't', ' ', 16,   0,    0, 0, 1,   0,   0,   0,
                               0x80, 0x3E, 0,   0,   0x00, 0x7D, 0, 0, 2,   0,   0,   0,
                               'd',  'a',  't', 'a', 10,   0,    0, 0, 0,   0,   0,   0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_wav_parse_header(zero_chan_wav, sizeof(zero_chan_wav), &info));
    TEST_ASSERT_EQUAL_UINT32(0, info.total_samples);

    /* 4-bit PCM bits_per_sample test (division-by-zero protection when bits_per_sample < 8) */
    uint8_t four_bit_pcm_wav[] = {'R',  'I',  'F', 'F', 44,   0,    0, 0, 'W', 'A', 'V', 'E',
                                  'f',  'm',  't', ' ', 16,   0,    0, 0, 1,   0,   1,   0,
                                  0x80, 0x3E, 0,   0,   0x00, 0x7D, 0, 0, 1,   0,   4,   0,
                                  'd',  'a',  't', 'a', 10,   0,    0, 0, 0,   0,   0,   0};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_wav_parse_header(four_bit_pcm_wav, sizeof(four_bit_pcm_wav), &info));
    TEST_ASSERT_EQUAL_UINT32(0, info.total_samples);

    /* Oversized chunk size (0xFFFFFFFF) loop break test */
    uint8_t huge_chunk_wav[] = {'R',  'I',  'F', 'F', 44,   0,    0,    0,    'W', 'A', 'V', 'E',
                                'J',  'U',  'N', 'K', 0xFF, 0xFF, 0xFF, 0xFF, 0,   0,   0,   0,
                                'f',  'm',  't', ' ', 16,   0,    0,    0,    1,   0,   1,   0,
                                0x80, 0x3E, 0,   0,   0x00, 0x7D, 0,    0,    2,   0,   16,  0,
                                'd',  'a',  't', 'a', 10,   0,    0,    0,    0,   0,   0,   0};
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_wav_parse_header(huge_chunk_wav, sizeof(huge_chunk_wav), &info));
}

void test_tier_a_wav_undersized_fmt_odd_pad_and_adpcm_overflow(void)
{
    SYN_WAV_Info info;

    /* Reject 'fmt ' chunk with chunk_size = 4 (< 16 minimum) instead of overreading next chunk */
    uint8_t short_fmt_wav[44];
    memset(short_fmt_wav, 0, sizeof(short_fmt_wav));
    memcpy(&short_fmt_wav[0], "RIFF", 4U);
    short_fmt_wav[4] = 36U;
    memcpy(&short_fmt_wav[8], "WAVEfmt ", 8U);
    short_fmt_wav[16] = 4U;
    short_fmt_wav[20] = 1U;
    short_fmt_wav[22] = 1U;
    memcpy(&short_fmt_wav[24], "data", 4U);
    short_fmt_wav[28] = 16U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(short_fmt_wav, sizeof(short_fmt_wav), &info));

    /* Odd-sized JUNK chunk ending right at buffer_size (missing pad byte) stops without wrap */
    uint8_t odd_tail_wav[45];
    memset(odd_tail_wav, 0, sizeof(odd_tail_wav));
    memcpy(&odd_tail_wav[0], "RIFF", 4U);
    odd_tail_wav[4] = 37U;
    memcpy(&odd_tail_wav[8], "WAVEJUNK", 8U);
    odd_tail_wav[16] = 25U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_wav_parse_header(odd_tail_wav, sizeof(odd_tail_wav), &info));

    /* IMA ADPCM with data_size = 0x80000000 saturates total_samples to UINT32_MAX instead of 0 */
    uint8_t adpcm_huge_wav[44];
    memset(adpcm_huge_wav, 0, sizeof(adpcm_huge_wav));
    memcpy(&adpcm_huge_wav[0], "RIFF", 4U);
    adpcm_huge_wav[4] = 36U;
    memcpy(&adpcm_huge_wav[8], "WAVEfmt ", 8U);
    adpcm_huge_wav[16] = 16U;
    adpcm_huge_wav[20] = 0x11U;
    adpcm_huge_wav[22] = 1U;
    adpcm_huge_wav[24] = 0x40U;
    adpcm_huge_wav[25] = 0x1FU;
    adpcm_huge_wav[29] = 0x10U;
    adpcm_huge_wav[32] = 1U;
    adpcm_huge_wav[34] = 4U;
    memcpy(&adpcm_huge_wav[36], "data", 4U);
    adpcm_huge_wav[43] = 0x80U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_wav_parse_header(adpcm_huge_wav, sizeof(adpcm_huge_wav), &info));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, info.total_samples);

    /* Stereo IMA ADPCM: 200 bytes = 400 4-bit samples across 2 channels = 200 sample frames */
    uint8_t adpcm_stereo_wav[44];
    memset(adpcm_stereo_wav, 0, sizeof(adpcm_stereo_wav));
    memcpy(&adpcm_stereo_wav[0], "RIFF", 4U);
    adpcm_stereo_wav[4] = 36U;
    memcpy(&adpcm_stereo_wav[8], "WAVEfmt ", 8U);
    adpcm_stereo_wav[16] = 16U;
    adpcm_stereo_wav[20] = 0x11U;
    adpcm_stereo_wav[22] = 2U; /* Stereo */
    adpcm_stereo_wav[24] = 0x40U;
    adpcm_stereo_wav[25] = 0x1FU;
    adpcm_stereo_wav[34] = 4U;
    memcpy(&adpcm_stereo_wav[36], "data", 4U);
    adpcm_stereo_wav[40] = 200U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_wav_parse_header(adpcm_stereo_wav, sizeof(adpcm_stereo_wav), &info));
    TEST_ASSERT_EQUAL_UINT32(200U, info.total_samples);

    /* Zero-channel IMA ADPCM must report 0 samples */
    adpcm_stereo_wav[22] = 0U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_wav_parse_header(adpcm_stereo_wav, sizeof(adpcm_stereo_wav), &info));
    TEST_ASSERT_EQUAL_UINT32(0U, info.total_samples);
}

void run_wav_tests(void)
{
    RUN_TEST(test_wav_parse_null_and_bounds_checks);
    RUN_TEST(test_wav_parse_pcm_valid_header);
    RUN_TEST(test_wav_parse_ima_adpcm_header);
    RUN_TEST(test_wav_parse_invalid_magics_and_truncated);
    RUN_TEST(test_tier_a_wav_undersized_fmt_odd_pad_and_adpcm_overflow);
}
