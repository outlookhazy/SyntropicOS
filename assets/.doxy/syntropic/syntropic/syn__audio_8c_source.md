

# File syn\_audio.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio.c**](syn__audio_8c.md)

[Go to the documentation of this file](syn__audio_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_AUDIO) || SYN_USE_AUDIO

#include "../util/syn_assert.h"
#include "syn_audio.h"

#include <string.h>

SYN_Status syn_audio_init(SYN_Audio *audio, const SYN_Audio_Config *cfg)
{
    if (audio == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->buf == NULL || cfg->half_size == 0U || cfg->buf_capacity < (2U * cfg->half_size)) {
        return SYN_INVALID_PARAM;
    }

    memset(audio, 0, sizeof(SYN_Audio));
    audio->cfg = *cfg;
    audio->active_half = 0U;
    audio->fill_pos = 0U;
    audio->running = false;
    audio->underrun_cnt = 0U;
    audio->frames_played = 0U;

    memset(cfg->buf, 0, cfg->buf_capacity * sizeof(int16_t));

    return SYN_OK;
}

SYN_Status syn_audio_start(SYN_Audio *audio)
{
    if (audio == NULL || audio->cfg.buf == NULL) {
        return SYN_INVALID_PARAM;
    }

    audio->running = true;
    audio->active_half = 0U;
    audio->fill_pos = 0U;

    if (audio->cfg.out_fn != NULL) {
        audio->cfg.out_fn(&audio->cfg.buf[0], audio->cfg.half_size, audio->cfg.ctx);
    }

    return SYN_OK;
}

SYN_Status syn_audio_stop(SYN_Audio *audio)
{
    if (audio == NULL) {
        return SYN_INVALID_PARAM;
    }

    audio->running = false;
    return SYN_OK;
}

size_t syn_audio_feed(SYN_Audio *audio, const int16_t *samples, size_t count)
{
    if (audio == NULL || samples == NULL || count == 0U) {
        return 0U;
    }

    size_t written = 0U;
    size_t half_len = audio->cfg.half_size;
    int16_t *half_buf = &audio->cfg.buf[audio->active_half * half_len];

    while (written < count) {
        if (audio->fill_pos < half_len) {
            half_buf[audio->fill_pos++] = samples[written++];
        } else {
            /* Current half buffer full — switch to alternate half */
            audio->active_half = (uint8_t)(1U - audio->active_half);
            half_buf = &audio->cfg.buf[audio->active_half * half_len];
            audio->fill_pos = 0U;

            if (audio->running && audio->cfg.out_fn != NULL) {
                audio->cfg.out_fn(half_buf, half_len, audio->cfg.ctx);
            }
        }
    }

    return written;
}

void syn_audio_isr_half(SYN_Audio *audio)
{
    if (audio == NULL || !audio->running) {
        return;
    }

    audio->frames_played++;
    if (audio->cfg.out_fn != NULL) {
        audio->cfg.out_fn(&audio->cfg.buf[0], audio->cfg.half_size, audio->cfg.ctx);
    }
}

void syn_audio_isr_complete(SYN_Audio *audio)
{
    if (audio == NULL || !audio->running) {
        return;
    }

    audio->frames_played++;
    if (audio->cfg.out_fn != NULL) {
        audio->cfg.out_fn(&audio->cfg.buf[audio->cfg.half_size], audio->cfg.half_size,
                          audio->cfg.ctx);
    }
}

#endif /* SYN_USE_AUDIO */
```


