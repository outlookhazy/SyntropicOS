

# File syn\_vad.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_vad.h**](syn__vad_8h.md)

[Go to the documentation of this file](syn__vad_8h.md)


```C++

#ifndef SYN_VAD_H
#define SYN_VAD_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_VAD) || SYN_USE_VAD

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_VAD_DEFAULT_FRAME_LEN 160U     
#define SYN_VAD_DEFAULT_ATTACK_FRAMES 2U   
#define SYN_VAD_DEFAULT_HANGOVER_FRAMES 6U 
/* ── Enums ──────────────────────────────────────────────────────────────── */

typedef enum {
    SYN_VAD_STATE_SILENCE = 0, 
    SYN_VAD_STATE_SPEECH = 1   
} SYN_VAD_State;

typedef enum {
    SYN_VAD_SENSITIVITY_SENSITIVE = 0, 
    SYN_VAD_SENSITIVITY_NORMAL = 1,    
    SYN_VAD_SENSITIVITY_AGGRESSIVE = 2 
} SYN_VAD_Sensitivity;

/* ── Feature Extraction Representation ──────────────────────────────────── */

typedef struct {
    uint32_t energy;        
    uint16_t zcr;           
    uint32_t hf_energy;     
    uint32_t noise_floor;   
    bool is_speech_instant; 
} SYN_VAD_Features;

/* ── Configuration Descriptor ────────────────────────────────────────────── */

typedef struct {
    uint16_t sample_rate_hz;         
    uint16_t frame_length;           
    uint8_t attack_frames;           
    uint8_t hangover_frames;         
    SYN_VAD_Sensitivity sensitivity; 
    uint32_t initial_noise_floor;    
} SYN_VAD_Config;

/* ── VAD Context Structure ──────────────────────────────────────────────── */

typedef struct {
    SYN_VAD_Config cfg;   
    SYN_VAD_State state;  
    uint32_t noise_floor; 
    uint8_t attack_counter;   
    uint8_t hangover_counter; 
    uint32_t energy_threshold_multiplier; 
    uint16_t min_zcr_threshold;           
    uint16_t max_zcr_threshold;           
} SYN_VAD;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_vad_init(SYN_VAD *vad, const SYN_VAD_Config *cfg);

SYN_Status syn_vad_reset(SYN_VAD *vad);

SYN_VAD_State syn_vad_process_frame(SYN_VAD *vad, const int16_t *samples, size_t num_samples,
                                    SYN_VAD_Features *out_features);

SYN_VAD_State syn_vad_get_state(const SYN_VAD *vad);

SYN_Status syn_vad_set_sensitivity(SYN_VAD *vad, SYN_VAD_Sensitivity sensitivity);

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYN_USE_VAD) || SYN_USE_VAD */

#endif /* SYN_VAD_H */
```


