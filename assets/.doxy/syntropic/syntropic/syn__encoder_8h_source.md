

# File syn\_encoder.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_encoder.h**](syn__encoder_8h.md)

[Go to the documentation of this file](syn__encoder_8h.md)


```C++

#ifndef SYN_ENCODER_H
#define SYN_ENCODER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../dsp/syn_signal.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Encoder direction ──────────────────────────────────────────────────── */

typedef enum {
    SYN_ENCODER_NONE = 0,   
    SYN_ENCODER_CW   = 1,   
    SYN_ENCODER_CCW   = -1, 
} SYN_EncoderDir;

/* ── Encoder descriptor ─────────────────────────────────────────────────── */

typedef struct {
    SYN_GPIO_Pin  pin_a;            
    SYN_GPIO_Pin  pin_b;            
    int32_t        position;    
    int32_t        delta;       
    uint8_t        last_state;  
    int8_t         last_dir;    
    /* Optional: step multiplier (for encoders with detents every N states) */
    uint8_t        steps_per_detent; 
    int8_t         sub_count;        
    /* Statistics (optional) */
    SYN_Signal   *stats;            
} SYN_Encoder;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_encoder_init(SYN_Encoder *enc,
                       SYN_GPIO_Pin pin_a,
                       SYN_GPIO_Pin pin_b);

void syn_encoder_set_steps_per_detent(SYN_Encoder *enc, uint8_t spd);

void syn_encoder_update(SYN_Encoder *enc);

int32_t syn_encoder_get_delta(SYN_Encoder *enc);

static inline int32_t syn_encoder_position(const SYN_Encoder *enc)
{
    return enc->position;
}

static inline void syn_encoder_set_position(SYN_Encoder *enc, int32_t pos)
{
    enc->position = pos;
}

static inline SYN_EncoderDir syn_encoder_direction(const SYN_Encoder *enc)
{
    return (SYN_EncoderDir)enc->last_dir;
}

void syn_encoder_set_stats(SYN_Encoder *enc, SYN_Signal *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ENCODER_H */
```


