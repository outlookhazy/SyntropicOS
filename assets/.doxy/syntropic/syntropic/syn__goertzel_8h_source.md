

# File syn\_goertzel.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_goertzel.h**](syn__goertzel_8h.md)

[Go to the documentation of this file](syn__goertzel_8h.md)


```C++

#ifndef SYN_GOERTZEL_H
#define SYN_GOERTZEL_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t coeff_q15; 
    int32_t s1;        
    int32_t s2;        
    size_t n_count;    
    size_t n_total;    
} SYN_Goertzel;

SYN_Status syn_goertzel_init(SYN_Goertzel *g, uint32_t sample_rate_hz, uint32_t target_freq_hz,
                             size_t block_size);

SYN_Status syn_goertzel_reset(SYN_Goertzel *g);

SYN_Status syn_goertzel_process_sample(SYN_Goertzel *g, int16_t sample);

size_t syn_goertzel_process_block(SYN_Goertzel *g, const int16_t *samples, size_t count);

uint64_t syn_goertzel_get_magnitude_sq(const SYN_Goertzel *g);

#ifdef __cplusplus
}
#endif

#endif /* SYN_GOERTZEL_H */
```


