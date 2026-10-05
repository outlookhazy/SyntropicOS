

# File syn\_foc\_encoder.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc\_encoder.h**](syn__foc__encoder_8h.md)

[Go to the documentation of this file](syn__foc__encoder_8h.md)


```C++

#ifndef SYN_FOC_ENCODER_H
#define SYN_FOC_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SYN_FOC_ENCODER_QUADRATURE = 0, 
    SYN_FOC_ENCODER_HALL = 1,       
    SYN_FOC_ENCODER_ABSOLUTE = 2    
} SYN_FOCEncoderType;

typedef struct {
    SYN_FOCEncoderType type; 
    uint8_t pole_pairs;      
    uint32_t cpr;            
    q16_t zero_offset_rad;   
    uint32_t sample_rate_hz; 
} SYN_FOCEncoderConfig;

typedef struct {
    SYN_FOCEncoderConfig config; 
    int32_t count;               
    int32_t prev_count;          
    q16_t elec_angle_rad;        
    q16_t elec_speed_rad_s;      
    uint8_t hall_state;          
} SYN_FOCEncoder;

SYN_Status syn_foc_encoder_init(SYN_FOCEncoder *enc, const SYN_FOCEncoderConfig *cfg);

void syn_foc_encoder_update_quadrature(SYN_FOCEncoder *enc, int32_t raw_count);

void syn_foc_encoder_update_hall(SYN_FOCEncoder *enc, bool hall_u, bool hall_v, bool hall_w);

void syn_foc_encoder_update_absolute(SYN_FOCEncoder *enc, uint16_t raw_angle_14bit);

q16_t syn_foc_encoder_get_elec_angle(const SYN_FOCEncoder *enc);

q16_t syn_foc_encoder_get_elec_speed(const SYN_FOCEncoder *enc);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_ENCODER_H */
```


