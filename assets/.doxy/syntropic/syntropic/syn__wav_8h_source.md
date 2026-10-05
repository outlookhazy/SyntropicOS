

# File syn\_wav.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_wav.h**](syn__wav_8h.md)

[Go to the documentation of this file](syn__wav_8h.md)


```C++

#ifndef SYN_WAV_H
#define SYN_WAV_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_WAV_FORMAT_PCM 0x0001U       
#define SYN_WAV_FORMAT_IMA_ADPCM 0x0011U 
typedef struct {
    uint16_t audio_format;    
    uint16_t num_channels;    
    uint32_t sample_rate;     
    uint32_t byte_rate;       
    uint16_t block_align;     
    uint16_t bits_per_sample; 
    size_t data_offset;       
    uint32_t data_size;       
    uint32_t total_samples;   
} SYN_WAV_Info;

SYN_Status syn_wav_parse_header(const uint8_t *buffer, size_t buffer_size, SYN_WAV_Info *info);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WAV_H */
```


