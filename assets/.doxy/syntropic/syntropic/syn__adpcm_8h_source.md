

# File syn\_adpcm.h

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_adpcm.h**](syn__adpcm_8h.md)

[Go to the documentation of this file](syn__adpcm_8h.md)


```C++

#ifndef SYN_ADPCM_H
#define SYN_ADPCM_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t predicted_sample; 
    int8_t step_index;        
} SYN_ADPCM_State;

void syn_adpcm_init(SYN_ADPCM_State *state);

int16_t syn_adpcm_decode_sample(SYN_ADPCM_State *state, uint8_t nibble);

uint8_t syn_adpcm_encode_sample(SYN_ADPCM_State *state, int16_t sample);

size_t syn_adpcm_decode_block(SYN_ADPCM_State *state, const uint8_t *in, int16_t *out,
                              size_t sample_count);

size_t syn_adpcm_encode_block(SYN_ADPCM_State *state, const int16_t *in, uint8_t *out,
                              size_t sample_count);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ADPCM_H */
```


