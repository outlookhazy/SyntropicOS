

# File syn\_ramp.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ramp.h**](syn__ramp_8h.md)

[Go to the documentation of this file](syn__ramp_8h.md)


```C++

#ifndef SYN_RAMP_H
#define SYN_RAMP_H

#include "../common/syn_defs.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Ramp mode ──────────────────────────────────────────────────────────── */

typedef enum {
    SYN_RAMP_LINEAR    = 0,   
    SYN_RAMP_TRAPEZOID = 1,   
} SYN_RampMode;

/* ── Ramp instance ──────────────────────────────────────────────────────── */

typedef struct {
    int32_t  current;       
    int32_t  target;        
    int32_t  rate;          
    int32_t  velocity;      
    int32_t  accel;         
    int32_t  frac_accum;    
    uint8_t  frac_bits;     
    uint8_t  mode;          
    bool     done;          
} SYN_Ramp;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_ramp_init(SYN_Ramp *ramp, int32_t initial);

void syn_ramp_set_target(SYN_Ramp *ramp, int32_t target, int32_t rate);

void syn_ramp_set_target_trapezoid(SYN_Ramp *ramp, int32_t target,
                                    int32_t max_rate, int32_t accel);

void syn_ramp_set_target_trapezoid_fp(SYN_Ramp *ramp, int32_t target,
                                      int32_t max_rate, int32_t accel,
                                      uint8_t frac_bits);

int32_t syn_ramp_update(SYN_Ramp *ramp);

void syn_ramp_jump(SYN_Ramp *ramp, int32_t value);

static inline bool syn_ramp_done(const SYN_Ramp *ramp)
{
    return ramp->done;
}

static inline int32_t syn_ramp_value(const SYN_Ramp *ramp)
{
    return ramp->current;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_RAMP_H */
```


