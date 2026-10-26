

# File syn\_vad.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_vad.c**](syn__vad_8c.md)

[Go to the documentation of this file](syn__vad_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_VAD) || SYN_USE_VAD

#include "../port/syn_port_system.h"
#include "syn_vad.h"

#include <string.h>

/* ── Internal Constants ─────────────────────────────────────────────────── */

#define SYN_VAD_MIN_NOISE_FLOOR 50U        
#define SYN_VAD_DEFAULT_INITIAL_NOISE 200U 
/* ── Sensitivity Configuration Helper ───────────────────────────────────── */

static void vad_apply_sensitivity(SYN_VAD *vad, SYN_VAD_Sensitivity sensitivity)
{
    vad->cfg.sensitivity = sensitivity;
    switch (sensitivity) {
    case SYN_VAD_SENSITIVITY_SENSITIVE:
        vad->energy_threshold_multiplier = 2U; /* 2x above noise floor */
        vad->min_zcr_threshold = 2U;
        vad->max_zcr_threshold = 120U;
        break;
    case SYN_VAD_SENSITIVITY_AGGRESSIVE:
        vad->energy_threshold_multiplier = 8U; /* 8x above noise floor */
        vad->min_zcr_threshold = 5U;
        vad->max_zcr_threshold = 90U;
        break;
    case SYN_VAD_SENSITIVITY_NORMAL:
    default:
        vad->energy_threshold_multiplier = 4U; /* 4x above noise floor */
        vad->min_zcr_threshold = 3U;
        vad->max_zcr_threshold = 100U;
        break;
    }
}

/* ── Public API Implementation ───────────────────────────────────────────── */

SYN_Status syn_vad_init(SYN_VAD *vad, const SYN_VAD_Config *cfg)
{
    if (vad == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(vad, 0, sizeof(*vad));

    if (cfg != NULL) {
        vad->cfg = *cfg;
    } else {
        vad->cfg.sample_rate_hz = 16000U;
        vad->cfg.frame_length = SYN_VAD_DEFAULT_FRAME_LEN;
        vad->cfg.attack_frames = SYN_VAD_DEFAULT_ATTACK_FRAMES;
        vad->cfg.hangover_frames = SYN_VAD_DEFAULT_HANGOVER_FRAMES;
        vad->cfg.sensitivity = SYN_VAD_SENSITIVITY_NORMAL;
        vad->cfg.initial_noise_floor = SYN_VAD_DEFAULT_INITIAL_NOISE;
    }

    if (vad->cfg.frame_length == 0U) {
        vad->cfg.frame_length = SYN_VAD_DEFAULT_FRAME_LEN;
    }
    if (vad->cfg.attack_frames == 0U) {
        vad->cfg.attack_frames = 1U;
    }
    if (vad->cfg.initial_noise_floor == 0U) {
        vad->cfg.initial_noise_floor = SYN_VAD_DEFAULT_INITIAL_NOISE;
    }

    vad->noise_floor = vad->cfg.initial_noise_floor;
    vad_apply_sensitivity(vad, vad->cfg.sensitivity);
    vad->state = SYN_VAD_STATE_SILENCE;
    return SYN_OK;
}

SYN_Status syn_vad_reset(SYN_VAD *vad)
{
    if (vad == NULL) {
        return SYN_INVALID_PARAM;
    }

    vad->state = SYN_VAD_STATE_SILENCE;
    vad->attack_counter = 0U;
    vad->hangover_counter = 0U;
    vad->noise_floor = vad->cfg.initial_noise_floor;
    return SYN_OK;
}

SYN_VAD_State syn_vad_process_frame(SYN_VAD *vad, const int16_t *samples, size_t num_samples,
                                    SYN_VAD_Features *out_features)
{
    if (vad == NULL || samples == NULL || num_samples == 0U) {
        return SYN_VAD_STATE_SILENCE;
    }

    uint64_t energy_acc = 0U;
    uint64_t hf_energy_acc = 0U;
    uint16_t zcr_count = 0U;
    int16_t prev_sample = samples[0];

    for (size_t i = 0U; i < num_samples; i++) {
        int16_t s = samples[i];
        int32_t s32 = (int32_t)s;
        energy_acc += (uint64_t)(s32 * s32);

        int32_t diff = (int32_t)s - (int32_t)prev_sample;
        hf_energy_acc += (uint64_t)(diff * diff);

        if ((s >= 0 && prev_sample < 0) || (s < 0 && prev_sample >= 0)) {
            zcr_count++;
        }
        prev_sample = s;
    }

    uint32_t frame_energy = (uint32_t)(energy_acc / num_samples);
    uint32_t hf_energy = (uint32_t)(hf_energy_acc / num_samples);

    /* Instantaneous Speech Trigger Evaluation */
    uint32_t threshold = vad->noise_floor * vad->energy_threshold_multiplier;
    bool instant_speech = false;

    if (frame_energy > threshold && zcr_count >= vad->min_zcr_threshold &&
        zcr_count <= vad->max_zcr_threshold) {
        instant_speech = true;
    }

    if (instant_speech) {
        if (vad->attack_counter < vad->cfg.attack_frames) {
            vad->attack_counter++;
        }
        if (vad->attack_counter >= vad->cfg.attack_frames) {
            vad->state = SYN_VAD_STATE_SPEECH;
            vad->hangover_counter = vad->cfg.hangover_frames;
        }
    } else {
        vad->attack_counter = 0U;

        /* Update background noise floor using Exponential Moving Average (alpha = 1/32) */
        vad->noise_floor = (vad->noise_floor * 31U + frame_energy) / 32U;
        if (vad->noise_floor < SYN_VAD_MIN_NOISE_FLOOR) {
            vad->noise_floor = SYN_VAD_MIN_NOISE_FLOOR;
        }

        if (vad->hangover_counter > 0U) {
            vad->hangover_counter--;
            vad->state = SYN_VAD_STATE_SPEECH;
        } else {
            vad->state = SYN_VAD_STATE_SILENCE;
        }
    }

    if (out_features != NULL) {
        out_features->energy = frame_energy;
        out_features->zcr = zcr_count;
        out_features->hf_energy = hf_energy;
        out_features->noise_floor = vad->noise_floor;
        out_features->is_speech_instant = instant_speech;
    }

    return vad->state;
}

SYN_VAD_State syn_vad_get_state(const SYN_VAD *vad)
{
    return (vad != NULL) ? vad->state : SYN_VAD_STATE_SILENCE;
}

SYN_Status syn_vad_set_sensitivity(SYN_VAD *vad, SYN_VAD_Sensitivity sensitivity)
{
    if (vad == NULL) {
        return SYN_INVALID_PARAM;
    }
    vad_apply_sensitivity(vad, sensitivity);
    return SYN_OK;
}

#endif /* !defined(SYN_USE_VAD) || SYN_USE_VAD */
```


