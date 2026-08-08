

# File syn\_sbc.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_sbc.c**](syn__sbc_8c.md)

[Go to the documentation of this file](syn__sbc_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SBC) || SYN_USE_SBC

#include "../util/syn_assert.h"
#include "syn_sbc.h"

#include <string.h>

static const int32_t proto4_40[40] = {
    0,     209,  395,  482,  428,  209,  -158, -600, -998, -1251, -1251, -904, -188, 778,
    1878,  2989, 3975, 4707, 5088, 5088, 4707, 3975, 2989, 1878,  778,   -188, -904, -1251,
    -1251, -998, -600, -158, 209,  428,  482,  395,  209,  0,     0,     0};

static const int32_t proto8_80[80] = {
    0,     156,   311,   464,   613,   756,   891,   1017,  1132,  1234,  1321,  1391,
    1443,  1475,  1486,  1475,  1443,  1391,  1321,  1234,  1132,  1017,  891,   756,
    613,   464,   311,   156,   0,     -156,  -311,  -464,  -613,  -756,  -891,  -1017,
    -1132, -1234, -1321, -1391, -1443, -1475, -1486, -1475, -1443, -1391, -1321, -1234,
    -1132, -1017, -891,  -756,  -613,  -464,  -311,  -156,  0,     156,   311,   464,
    613,   756,   891,   1017,  1132,  1234,  1321,  1391,  1443,  1475,  1486,  1475};

static const int32_t syn_cos4[8][4] = {
    {23170, 23170, 23170, 23170},     {30274, 12540, -12540, -30274},
    {23170, -23170, -23170, 23170},   {12540, -30274, 30274, -12540},
    {-23170, -23170, -23170, -23170}, {-30274, -12540, 12540, 30274},
    {-23170, 23170, 23170, -23170},   {-12540, 30274, -30274, 12540}};

typedef struct {
    const uint8_t *data;
    size_t len;
    size_t bit_pos;
} BitReader;
static inline int16_t clamp_s16(int32_t val)
{
    if (val > 32767) {
        return 32767;
    }
    if (val < -32768) {
        return -32768;
    }
    return (int16_t)val;
}

static uint32_t get_bits(BitReader *br, uint8_t nbits)
{
    uint32_t val = 0U;
    for (uint8_t i = 0U; i < nbits; i++) {
        size_t byte_idx = br->bit_pos >> 3;
        if (byte_idx >= br->len) {
            break;
        }
        uint8_t bit_offset = (uint8_t)(7U - (br->bit_pos & 7U));
        uint8_t bit = (uint8_t)((br->data[byte_idx] >> bit_offset) & 1U);
        val = (val << 1) | bit;
        br->bit_pos++;
    }
    return val;
}

static void sbc_calculate_bits(const SYN_SBC_FrameInfo *info, int32_t scale_factors[2][8],
                               int32_t bits[2][8])
{
    static const int8_t offset4[4] = {-2, 0, 0, 0};
    static const int8_t offset8[8] = {-2, 0, 0, 0, 0, 0, 0, 1};
    int8_t loudness[2][8];

    uint8_t chs = info->channels;
    uint8_t sbs = info->subbands;

    memset(bits, 0, sizeof(int32_t) * 16);

    if (info->alloc == SYN_SBC_ALLOC_LOUDNESS) {
        for (uint8_t ch = 0; ch < chs; ch++) {
            for (uint8_t sb = 0; sb < sbs; sb++) {
                int8_t off = (sbs == 4U) ? offset4[sb] : offset8[sb];
                int32_t loud_val = (int32_t)scale_factors[ch][sb] - (int32_t)off;
                loudness[ch][sb] = (loud_val < 0) ? 0 : (int8_t)loud_val;
            }
        }
    } else {
        for (uint8_t ch = 0; ch < chs; ch++) {
            for (uint8_t sb = 0; sb < sbs; sb++) {
                loudness[ch][sb] = (int8_t)scale_factors[ch][sb];
            }
        }
    }

    int32_t bitpool = (int32_t)info->bitpool;

    for (int level = 16; level >= 0 && bitpool > 0; level--) {
        for (uint8_t ch = 0; ch < chs; ch++) {
            for (uint8_t sb = 0; sb < sbs; sb++) {
                if (loudness[ch][sb] >= level && bitpool > 0) {
                    bits[ch][sb]++;
                    bitpool--;
                }
            }
        }
    }
}
void syn_sbc_decoder_init(SYN_SBC_Decoder *dec)
{
    if (dec == NULL) {
        return;
    }
    memset(dec, 0, sizeof(SYN_SBC_Decoder));
}

SYN_Status syn_sbc_parse_header(const uint8_t *data, size_t len, SYN_SBC_FrameInfo *info)
{
    if (data == NULL || info == NULL || len < 4U) {
        return SYN_INVALID_PARAM;
    }

    if (data[0] != SYN_SBC_SYNCWORD) {
        return SYN_ERROR;
    }

    uint8_t h1 = data[1];
    uint8_t h2 = data[2];

    info->sample_rate_enum = (h1 >> 6) & 0x03U;
    switch (info->sample_rate_enum) {
    case 0:
        info->sample_rate_hz = 16000U;
        break;
    case 1:
        info->sample_rate_hz = 32000U;
        break;
    case 2:
        info->sample_rate_hz = 44100U;
        break;
    default:
        info->sample_rate_hz = 48000U;
        break;
    }

    uint8_t blocks_code = (h1 >> 4) & 0x03U;
    info->blocks = (uint8_t)(4U * (blocks_code + 1U));

    info->mode = (SYN_SBC_ChannelMode)((h1 >> 2) & 0x03U);
    info->channels = (info->mode == SYN_SBC_MODE_MONO) ? 1U : 2U;

    info->alloc = (SYN_SBC_AllocMethod)((h1 >> 1) & 0x01U);
    info->subbands = ((h1 & 0x01U) != 0U) ? 8U : 4U;
    info->bitpool = h2;

    info->join = (info->mode == SYN_SBC_MODE_JOINT_STEREO) ? data[3] : 0U;

    /* Calculate total frame length according to Bluetooth A2DP specification */
    uint16_t header_len = 4U;

    uint32_t bits_per_frame = 0;
    if (info->mode == SYN_SBC_MODE_MONO || info->mode == SYN_SBC_MODE_DUAL_CHANNEL) {
        bits_per_frame =
            (uint32_t)info->blocks * (uint32_t)info->channels * (uint32_t)info->bitpool;
    } else {
        bits_per_frame = (uint32_t)info->blocks * (uint32_t)info->bitpool;
        if (info->mode == SYN_SBC_MODE_JOINT_STEREO) {
            bits_per_frame += (uint32_t)info->subbands;
        }
    }

    info->frame_len = (uint16_t)(header_len + (bits_per_frame + 7U) / 8U);

    return SYN_OK;
}

SYN_Status syn_sbc_decode_frame(SYN_SBC_Decoder *dec, const uint8_t *in, size_t in_len,
                                int16_t *pcm_out, size_t pcm_cap, size_t *out_samples)
{
    if (dec == NULL || in == NULL || pcm_out == NULL || out_samples == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_Status status = syn_sbc_parse_header(in, in_len, &dec->info);
    if (status != SYN_OK) {
        return status;
    }

    size_t req_samples =
        (size_t)dec->info.blocks * (size_t)dec->info.subbands * (size_t)dec->info.channels;
    if (pcm_cap < req_samples) {
        return SYN_INVALID_PARAM;
    }

    BitReader br = {in, in_len, 32U}; /* Skip 4-byte header */

    /* 1. Unpack scale factors */
    for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
        for (uint8_t sb = 0U; sb < dec->info.subbands; sb++) {
            dec->scale_factors[ch][sb] = (int32_t)get_bits(&br, 4U);
        }
    }

    /* 2. Calculate bit allocation */
    int32_t bits[2][8];
    sbc_calculate_bits(&dec->info, dec->scale_factors, bits);

    /* 3. Unpack subband samples & reconstruct audio samples */
    for (uint8_t blk = 0U; blk < dec->info.blocks; blk++) {
        for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
            for (uint8_t sb = 0U; sb < dec->info.subbands; sb++) {
                if (bits[ch][sb] > 0) {
                    uint32_t raw_sample = get_bits(&br, (uint8_t)bits[ch][sb]);
                    int32_t sf = dec->scale_factors[ch][sb];
                    int32_t b = bits[ch][sb];
                    int32_t levels = (1 << b) - 1;
                    int32_t val =
                        (int32_t)(((uint64_t)(2U * raw_sample + 1U) << sf) / (uint32_t)levels) -
                        (1 << sf);
                    dec->audio_sample[blk][ch][sb] = val;
                } else {
                    dec->audio_sample[blk][ch][sb] = 0;
                }
            }
        }
    }

    /* 4. Synthesis filterbank matrixing and windowing */
    size_t pcm_idx = 0U;
    uint8_t sbs = dec->info.subbands;

    for (uint8_t blk = 0U; blk < dec->info.blocks; blk++) {
        for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
            /* Shift V vector by 2*subbands */
            for (int i = (10 * (int)sbs) - 1; i >= 2 * (int)sbs; i--) {
                dec->V[ch][i] = dec->V[ch][i - 2 * (int)sbs];
            }

            /* Perform IDCT synthesis matrixing into V[ch][0..2*subbands-1] */
            for (uint8_t i = 0U; i < 2U * sbs; i++) {
                int32_t sum = 0;
                for (uint8_t sb = 0U; sb < sbs; sb++) {
                    int32_t cos_val = (sbs == 4U) ? syn_cos4[i][sb] : proto8_80[i * 8 + sb];
                    sum += (dec->audio_sample[blk][ch][sb] * cos_val) >> 8;
                }
                dec->V[ch][i] = sum;
            }

            /* Windowing with proto window to compute final PCM sample */
            for (uint8_t sb = 0U; sb < sbs; sb++) {
                int32_t pcm_val = 0;
                for (int idx = 0; idx < 10; idx++) {
                    int32_t coef = (sbs == 4U) ? proto4_40[idx * 4 + sb] : proto8_80[idx * 8 + sb];
                    pcm_val += (dec->V[ch][idx * 2 * (int)sbs + (int)sb] * coef) >> 8;
                }
                pcm_out[pcm_idx++] = clamp_s16(pcm_val);
            }
        }
    }

    *out_samples = pcm_idx;
    return SYN_OK;
}

#endif /* SYN_USE_SBC */
```


