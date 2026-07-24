

# File syn\_dds.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dds.h**](syn__dds_8h.md)

[Go to the documentation of this file](syn__dds_8h.md)


```C++

#ifndef SYN_DDS_H
#define SYN_DDS_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_DDS_SINE     = 0, 
    SYN_DDS_TRIANGLE = 1, 
    SYN_DDS_SAWTOOTH = 2, 
    SYN_DDS_SQUARE   = 3, 
    SYN_DDS_NOISE    = 4  
} SYN_DDS_Waveform;

typedef struct {
    SYN_DDS_Waveform type;         
    uint32_t         phase;        
    uint32_t         phase_step;   
    q16_t            amplitude;    
    q16_t            offset;       
    uint16_t         duty_q16;     
    uint32_t         rand_state;   
} SYN_DDS;

SYN_Status syn_dds_init(SYN_DDS *dds, SYN_DDS_Waveform type, uint32_t freq_hz, uint32_t sample_rate_hz);

SYN_Status syn_dds_set_freq(SYN_DDS *dds, uint32_t freq_hz, uint32_t sample_rate_hz);

SYN_Status syn_dds_set_gain(SYN_DDS *dds, q16_t amplitude, q16_t offset);

SYN_Status syn_dds_set_duty(SYN_DDS *dds, float duty_pct);

q16_t syn_dds_step(SYN_DDS *dds);

SYN_Status syn_dds_fill_q16(SYN_DDS *dds, q16_t *buf, size_t count);

SYN_Status syn_dds_fill_u16(SYN_DDS *dds, uint16_t *buf, size_t count, uint16_t dac_center, uint16_t dac_span);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DDS_H */
```


