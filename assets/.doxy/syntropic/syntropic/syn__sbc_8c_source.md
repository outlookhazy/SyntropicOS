

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
    0,    0,    1,    2,    4,    6,    6,    3,    -4,   -14,  -27,  -40,  -49,  -50,  -39,  -16,
    19,   63,   108,  146,  166,  160,  121,  48,   -54,  -174, -295, -393, -445, -429, -327, -132,
    154,  518,  933,  1367, 1780, 2133, 2391, 2527, 2527, 2391, 2133, 1780, 1367, 933,  518,  154,
    -132, -327, -429, -445, -393, -295, -174, -54,  48,   121,  160,  166,  146,  108,  63,   19,
    -16,  -39,  -50,  -49,  -40,  -27,  -14,  -4,   3,    6,    6,    4,    2,    1,    0,    0};

static const int32_t syn_cos4[8][4] = {
    {23170, 23170, 23170, 23170},     {30274, 12540, -12540, -30274},
    {23170, -23170, -23170, 23170},   {12540, -30274, 30274, -12540},
    {-23170, -23170, -23170, -23170}, {-30274, -12540, 12540, 30274},
    {-23170, 23170, 23170, -23170},   {-12540, 30274, -30274, 12540}};

static const int32_t syn_cos8[16][8] = {
    {32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767},
    {32138, 27246, 18205, 6393, -6393, -18205, -27246, -32138},
    {30274, 12540, -12540, -30274, -30274, -12540, 12540, 30274},
    {27246, -6393, -32138, -18205, 18205, 32138, 6393, -27246},
    {23170, -23170, -23170, 23170, 23170, -23170, -23170, 23170},
    {18205, -32138, 6393, 27246, -27246, -6393, 32138, -18205},
    {12540, -30274, 30274, -12540, -12540, 30274, -30274, 12540},
    {6393, -18205, 27246, -32138, 32138, -27246, 18205, -6393},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {-6393, 18205, -27246, 32138, -32138, 27246, -18205, 6393},
    {-12540, 30274, -30274, 12540, 12540, -30274, 30274, -12540},
    {-18205, 32138, -6393, -27246, 27246, 6393, -32138, 18205},
    {-23170, 23170, 23170, -23170, -23170, 23170, 23170, -23170},
    {-27246, 6393, 32138, 18205, -18205, -32138, -6393, 27246},
    {-30274, -12540, 12540, 30274, 30274, 12540, -12540, -30274},
    {-32138, -27246, -18205, -6393, 6393, 18205, 27246, 32138}};

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

static void sbc_alloc_group(int8_t loudness[2][8], uint8_t ch_start, uint8_t ch_end, uint8_t sbs,
                            int32_t bitpool, int32_t bits[2][8])
{
    for (int level = 17; level >= -16 && bitpool > 0; level--) {
        for (uint8_t ch = ch_start; ch < ch_end; ch++) {
            for (uint8_t sb = 0U; sb < sbs; sb++) {
                if ((int)loudness[ch][sb] >= level && bits[ch][sb] < 16 && bitpool > 0) {
                    bits[ch][sb]++;
                    bitpool--;
                }
            }
        }
    }
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
    if (info->mode == SYN_SBC_MODE_DUAL_CHANNEL) {
        for (uint8_t ch = 0U; ch < chs; ch++) {
            sbc_alloc_group(loudness, ch, (uint8_t)(ch + 1U), sbs, bitpool, bits);
        }
    } else {
        sbc_alloc_group(loudness, 0U, chs, sbs, bitpool, bits);
    }
}
static uint16_t sbc_calc_frame_len(const SYN_SBC_FrameInfo *info)
{
    uint32_t sf_bits = 4U * (uint32_t)info->subbands * (uint32_t)info->channels;
    uint32_t audio_bits = 0U;

    if (info->mode == SYN_SBC_MODE_MONO || info->mode == SYN_SBC_MODE_DUAL_CHANNEL) {
        audio_bits = (uint32_t)info->blocks * (uint32_t)info->channels * (uint32_t)info->bitpool;
    } else {
        audio_bits = (uint32_t)info->blocks * (uint32_t)info->bitpool;
        if (info->mode == SYN_SBC_MODE_JOINT_STEREO) {
            audio_bits += (uint32_t)info->subbands;
        }
    }

    return (uint16_t)(4U + (sf_bits / 8U) + ((audio_bits + 7U) / 8U));
}

static uint8_t sbc_read_join_mask(BitReader *br, uint8_t subbands)
{
    uint8_t join_mask = 0U;
    for (uint8_t sb = 0U; sb + 1U < subbands; sb++) {
        if (get_bits(br, 1U) != 0U) {
            join_mask |= (uint8_t)(1U << sb);
        }
    }
    (void)get_bits(br, 1U); /* RFA bit */
    return join_mask;
}

static void sbc_uncouple_joint_stereo(SYN_SBC_Decoder *dec)
{
    if (dec->info.mode != SYN_SBC_MODE_JOINT_STEREO) {
        return;
    }

    uint8_t sbs = dec->info.subbands;
    for (uint8_t sb = 0U; sb + 1U < sbs; sb++) {
        if ((dec->info.join & (uint8_t)(1U << sb)) != 0U) {
            for (uint8_t blk = 0U; blk < dec->info.blocks; blk++) {
                int32_t mid = dec->audio_sample[blk][0][sb];
                int32_t side = dec->audio_sample[blk][1][sb];
                dec->audio_sample[blk][0][sb] = mid + side;
                dec->audio_sample[blk][1][sb] = mid - side;
            }
        }
    }
}

static void sbc_unpack_samples(SYN_SBC_Decoder *dec, BitReader *br, int32_t bits[2][8])
{
    for (uint8_t blk = 0U; blk < dec->info.blocks; blk++) {
        for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
            for (uint8_t sb = 0U; sb < dec->info.subbands; sb++) {
                if (bits[ch][sb] > 0) {
                    uint32_t b = (uint32_t)bits[ch][sb];
                    uint32_t raw_sample = get_bits(br, (uint8_t)b);
                    uint32_t sf = (uint32_t)dec->scale_factors[ch][sb];
                    uint32_t levels = (1U << b) - 1U;
                    uint64_t num = (uint64_t)(2U * raw_sample + 1U) << sf;
                    int32_t val = (int32_t)(num / (uint64_t)levels) - (int32_t)(1U << sf);
                    dec->audio_sample[blk][ch][sb] = val;
                } else {
                    dec->audio_sample[blk][ch][sb] = 0;
                }
            }
        }
    }
}

static void sbc_synthesize_channel(SYN_SBC_Decoder *dec, uint8_t blk, uint8_t ch, int16_t *pcm_out)
{
    uint8_t sbs = dec->info.subbands;
    uint8_t chs = dec->info.channels;
    size_t blk_base = (size_t)blk * (size_t)sbs * (size_t)chs;

    for (int i = (20 * (int)sbs) - 1; i >= 2 * (int)sbs; i--) {
        dec->V[ch][i] = dec->V[ch][i - 2 * (int)sbs];
    }

    for (uint16_t i = 0U; i < 2U * (uint16_t)sbs; i++) {
        int32_t sum = 0;
        for (uint8_t sb = 0U; sb < sbs; sb++) {
            int32_t cos_val = (sbs == 4U) ? syn_cos4[i][sb] : syn_cos8[i][sb];
            sum += (int32_t)(((int64_t)dec->audio_sample[blk][ch][sb] * (int64_t)cos_val) >> 8);
        }
        dec->V[ch][i] = sum;
    }

    const int32_t *proto = (sbs == 4U) ? proto4_40 : proto8_80;
    for (uint8_t sb = 0U; sb < sbs; sb++) {
        int32_t pcm_val = 0;
        for (int idx = 0; idx < 10; idx++) {
            int32_t coef = proto[idx * (int)sbs + (int)sb];
            int32_t v_sample = dec->V[ch][idx * 2 * (int)sbs + (int)sb];
            pcm_val += (int32_t)(((int64_t)v_sample * (int64_t)coef) >> 8);
        }
        pcm_out[blk_base + (size_t)sb * (size_t)chs + (size_t)ch] = clamp_s16(pcm_val);
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

    info->join = 0U;
    if (info->mode == SYN_SBC_MODE_JOINT_STEREO && len >= 5U) {
        BitReader br = {data, len, 32U};
        info->join = sbc_read_join_mask(&br, info->subbands);
    }

    info->frame_len = sbc_calc_frame_len(info);

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

    size_t req_samples = (size_t)dec->info.blocks * (size_t)dec->info.subbands;
    req_samples *= (size_t)dec->info.channels;
    if (pcm_cap < req_samples) {
        return SYN_INVALID_PARAM;
    }

    BitReader br = {in, in_len, 32U}; /* Skip 4-byte header */
    if (dec->info.mode == SYN_SBC_MODE_JOINT_STEREO) {
        dec->info.join = sbc_read_join_mask(&br, dec->info.subbands);
    }

    /* 1. Unpack scale factors */
    for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
        for (uint8_t sb = 0U; sb < dec->info.subbands; sb++) {
            dec->scale_factors[ch][sb] = (int32_t)get_bits(&br, 4U);
        }
    }

    /* 2. Calculate bit allocation */
    int32_t bits[2][8];
    sbc_calculate_bits(&dec->info, dec->scale_factors, bits);

    /* 3. Unpack subband samples & Joint Stereo M/S uncoupling */
    sbc_unpack_samples(dec, &br, bits);
    sbc_uncouple_joint_stereo(dec);

    /* 4. Synthesis filterbank matrixing and windowing */
    for (uint8_t blk = 0U; blk < dec->info.blocks; blk++) {
        for (uint8_t ch = 0U; ch < dec->info.channels; ch++) {
            sbc_synthesize_channel(dec, blk, ch, pcm_out);
        }
    }

    *out_samples = req_samples;
    return SYN_OK;
}

#endif /* SYN_USE_SBC */
```


