

# File syn\_audio\_mixer.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio\_mixer.h**](syn__audio__mixer_8h.md)

[Go to the documentation of this file](syn__audio__mixer_8h.md)


```C++

#ifndef SYN_AUDIO_MIXER_H
#define SYN_AUDIO_MIXER_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_AUDIO_MIXER_MAX_CHANNELS
#define SYN_AUDIO_MIXER_MAX_CHANNELS 4U 
#endif

#define SYN_AUDIO_GAIN_UNITY 32768U

typedef struct {
    const int16_t *buf;  
    size_t sample_count; 
    size_t pos;          
    uint16_t volume_q15; 
    bool active;         
    bool loop;           
} SYN_Audio_Mixer_Channel;

typedef struct {
    SYN_Audio_Mixer_Channel channels[SYN_AUDIO_MIXER_MAX_CHANNELS]; 
    uint16_t master_volume_q15;                                     
} SYN_Audio_Mixer;

SYN_Status syn_audio_mixer_init(SYN_Audio_Mixer *mixer);

SYN_Status syn_audio_mixer_play(SYN_Audio_Mixer *mixer, uint8_t channel_idx, const int16_t *pcm_buf,
                                size_t sample_count, uint16_t volume_q15, bool loop);

SYN_Status syn_audio_mixer_stop(SYN_Audio_Mixer *mixer, uint8_t channel_idx);

SYN_Status syn_audio_mixer_set_channel_volume(SYN_Audio_Mixer *mixer, uint8_t channel_idx,
                                              uint16_t volume_q15);

SYN_Status syn_audio_mixer_set_master_volume(SYN_Audio_Mixer *mixer, uint16_t volume_q15);

size_t syn_audio_mixer_render(SYN_Audio_Mixer *mixer, int16_t *out_pcm, size_t sample_count);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUDIO_MIXER_H */
```


