

# File syn\_audio.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio.h**](syn__audio_8h.md)

[Go to the documentation of this file](syn__audio_8h.md)


```C++

#ifndef SYN_AUDIO_H
#define SYN_AUDIO_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SYN_Audio_OutputFn)(const int16_t *samples, size_t count, void *ctx);

typedef struct {
    int16_t *buf;              
    size_t buf_capacity;       
    size_t half_size;          
    uint32_t sample_rate_hz;   
    uint8_t channels;          
    SYN_Audio_OutputFn out_fn; 
    void *ctx;                 
} SYN_Audio_Config;

typedef struct {
    SYN_Audio_Config cfg;         
    volatile uint8_t active_half; 
    volatile size_t fill_pos;     
    volatile bool running;        
    uint32_t underrun_cnt;        
    uint32_t frames_played;       
} SYN_Audio;

SYN_Status syn_audio_init(SYN_Audio *audio, const SYN_Audio_Config *cfg);

SYN_Status syn_audio_start(SYN_Audio *audio);

SYN_Status syn_audio_stop(SYN_Audio *audio);

size_t syn_audio_feed(SYN_Audio *audio, const int16_t *samples, size_t count);

void syn_audio_isr_half(SYN_Audio *audio);

void syn_audio_isr_complete(SYN_Audio *audio);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUDIO_H */
```


