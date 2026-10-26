

# File syn\_sbc.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_sbc.h**](syn__sbc_8h.md)

[Go to the documentation of this file](syn__sbc_8h.md)


```C++

#ifndef SYN_SBC_H
#define SYN_SBC_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SBC_SYNCWORD 0x9CU

typedef enum {
    SYN_SBC_MODE_MONO = 0,
    SYN_SBC_MODE_DUAL_CHANNEL = 1,
    SYN_SBC_MODE_STEREO = 2,
    SYN_SBC_MODE_JOINT_STEREO = 3
} SYN_SBC_ChannelMode;

typedef enum { SYN_SBC_ALLOC_LOUDNESS = 0, SYN_SBC_ALLOC_SNR = 1 } SYN_SBC_AllocMethod;

typedef struct {
    uint8_t sample_rate_enum;  
    uint16_t sample_rate_hz;   
    SYN_SBC_ChannelMode mode;  
    uint8_t channels;          
    uint8_t blocks;            
    uint8_t subbands;          
    SYN_SBC_AllocMethod alloc; 
    uint8_t bitpool;           
    uint8_t join;              
    uint16_t frame_len;        
} SYN_SBC_FrameInfo;

typedef struct {
    SYN_SBC_FrameInfo info;         
    int32_t V[2][160];              
    int32_t scale_factors[2][8];    
    int32_t audio_sample[16][2][8]; 
} SYN_SBC_Decoder;

void syn_sbc_decoder_init(SYN_SBC_Decoder *dec);

SYN_Status syn_sbc_parse_header(const uint8_t *data, size_t len, SYN_SBC_FrameInfo *info);

SYN_Status syn_sbc_decode_frame(SYN_SBC_Decoder *dec, const uint8_t *in, size_t in_len,
                                int16_t *pcm_out, size_t pcm_cap, size_t *out_samples);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SBC_H */
```


