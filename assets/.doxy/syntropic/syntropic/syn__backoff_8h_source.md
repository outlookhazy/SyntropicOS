

# File syn\_backoff.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_backoff.h**](syn__backoff_8h.md)

[Go to the documentation of this file](syn__backoff_8h.md)


```C++

#ifndef SYN_BACKOFF_H
#define SYN_BACKOFF_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t min_ms;      
    uint32_t max_ms;      
    uint32_t current_ms;  
    uint8_t  attempts;    
    uint8_t  max_attempts; 
    uint8_t  factor;      
} SYN_Backoff;

void syn_backoff_init(SYN_Backoff *b, uint32_t min_ms, uint32_t max_ms, uint8_t factor, uint8_t max_attempts);

uint32_t syn_backoff_next_ms(SYN_Backoff *b);

bool syn_backoff_exhausted(const SYN_Backoff *b);

void syn_backoff_reset(SYN_Backoff *b);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BACKOFF_H */
```


