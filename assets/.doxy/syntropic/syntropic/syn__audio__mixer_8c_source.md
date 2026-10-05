

# File syn\_audio\_mixer.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio\_mixer.c**](syn__audio__mixer_8c.md)

[Go to the documentation of this file](syn__audio__mixer_8c.md)


```C++

#include "syn_audio_mixer.h"

#include <string.h>

static inline int16_t saturate_i32_to_i16(int32_t val)
{
    if (val > 32767) {
        return 32767;
    }
    if (val < -32768) {
        return -32768;
    }
    return (int16_t)val;
}

SYN_Status syn_audio_mixer_init(SYN_Audio_Mixer *mixer)
{
    if (mixer == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(mixer, 0, sizeof(SYN_Audio_Mixer));
    mixer->master_volume_q15 = SYN_AUDIO_GAIN_UNITY;

    return SYN_OK;
}

SYN_Status syn_audio_mixer_play(SYN_Audio_Mixer *mixer, uint8_t channel_idx, const int16_t *pcm_buf,
                                size_t sample_count, uint16_t volume_q15, bool loop)
{
    if ((mixer == NULL) || (pcm_buf == NULL) || (sample_count == 0U)) {
        return SYN_INVALID_PARAM;
    }

    if (channel_idx >= SYN_AUDIO_MIXER_MAX_CHANNELS) {
        return SYN_INVALID_PARAM;
    }

    SYN_Audio_Mixer_Channel *ch = &mixer->channels[channel_idx];
    ch->buf = pcm_buf;
    ch->sample_count = sample_count;
    ch->pos = 0U;
    ch->volume_q15 = volume_q15;
    ch->loop = loop;
    ch->active = true;

    return SYN_OK;
}

SYN_Status syn_audio_mixer_stop(SYN_Audio_Mixer *mixer, uint8_t channel_idx)
{
    if (mixer == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (channel_idx >= SYN_AUDIO_MIXER_MAX_CHANNELS) {
        return SYN_INVALID_PARAM;
    }

    mixer->channels[channel_idx].active = false;
    mixer->channels[channel_idx].pos = 0U;

    return SYN_OK;
}

SYN_Status syn_audio_mixer_set_channel_volume(SYN_Audio_Mixer *mixer, uint8_t channel_idx,
                                              uint16_t volume_q15)
{
    if (mixer == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (channel_idx >= SYN_AUDIO_MIXER_MAX_CHANNELS) {
        return SYN_INVALID_PARAM;
    }

    mixer->channels[channel_idx].volume_q15 = volume_q15;

    return SYN_OK;
}

SYN_Status syn_audio_mixer_set_master_volume(SYN_Audio_Mixer *mixer, uint16_t volume_q15)
{
    if (mixer == NULL) {
        return SYN_INVALID_PARAM;
    }

    mixer->master_volume_q15 = volume_q15;

    return SYN_OK;
}

size_t syn_audio_mixer_render(SYN_Audio_Mixer *mixer, int16_t *out_pcm, size_t sample_count)
{
    if ((mixer == NULL) || (out_pcm == NULL) || (sample_count == 0U)) {
        return 0U;
    }

    for (size_t s = 0U; s < sample_count; s++) {
        int32_t acc = 0;

        for (uint8_t i = 0U; i < SYN_AUDIO_MIXER_MAX_CHANNELS; i++) {
            SYN_Audio_Mixer_Channel *ch = &mixer->channels[i];
            if (!ch->active || (ch->buf == NULL)) {
                continue;
            }

            if (ch->pos >= ch->sample_count) {
                if (ch->loop) {
                    ch->pos = 0U;
                } else {
                    ch->active = false;
                    continue;
                }
            }

            int32_t sample = ch->buf[ch->pos++];
            int32_t scaled = (sample * (int32_t)ch->volume_q15) >> 15;
            acc += scaled;
        }

        /* Apply master volume scaling */
        int32_t master_scaled = (acc * (int32_t)mixer->master_volume_q15) >> 15;
        out_pcm[s] = saturate_i32_to_i16(master_scaled);
    }

    return sample_count;
}
```


