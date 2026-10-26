

# File syn\_ppm.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_ppm.h**](syn__ppm_8h.md)

[Go to the documentation of this file](syn__ppm_8h.md)


```C++

#ifndef SYN_PPM_H
#define SYN_PPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_PPM_MAX_CHANNELS 12   
#define SYN_PPM_SYNC_MIN_US 2700U 
typedef struct {
    uint16_t channels[SYN_PPM_MAX_CHANNELS]; 
    uint8_t channel_count;                   
    uint8_t current_channel;                 
    bool in_frame;                           
    uint32_t frames_received;                
} SYN_PPM_Decoder;

SYN_Status syn_ppm_init(SYN_PPM_Decoder *ppm);

SYN_Status syn_ppm_process_pulse(SYN_PPM_Decoder *ppm, uint16_t pulse_us);

uint16_t syn_ppm_get_channel(const SYN_PPM_Decoder *ppm, uint8_t channel_idx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PPM_H */
```


