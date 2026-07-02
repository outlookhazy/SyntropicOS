

# File syn\_port\_dma.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dma.h**](syn__port__dma_8h.md)

[Go to the documentation of this file](syn__port__dma_8h.md)


```C++

#ifndef SYN_PORT_DMA_H
#define SYN_PORT_DMA_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DMA) || SYN_USE_DMA

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Transfer direction ────────────────────────────────────────────────── */

typedef enum {
    SYN_DMA_MEM_TO_PERIPH = 0,  
    SYN_DMA_PERIPH_TO_MEM = 1,  
    SYN_DMA_MEM_TO_MEM    = 2,  
} SYN_DMA_Dir;

/* ── Data width ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_DMA_WIDTH_8  = 1,  
    SYN_DMA_WIDTH_16 = 2,  
    SYN_DMA_WIDTH_32 = 4,  
} SYN_DMA_Width;

/* ── Callback ──────────────────────────────────────────────────────────── */

typedef void (*SYN_DMA_Callback)(uint8_t channel, SYN_Status result, void *ctx);

/* ── Channel configuration ─────────────────────────────────────────────── */

typedef struct {
    uint8_t          channel;    
    SYN_DMA_Dir      direction;  
    SYN_DMA_Width    width;      
    bool             src_incr;   
    bool             dst_incr;   
    SYN_DMA_Callback callback;   
    void            *user_data;  
} SYN_DMA_Config;

/* ── Port functions (user implements) ──────────────────────────────────── */

SYN_Status syn_port_dma_init(const SYN_DMA_Config *cfg);

SYN_Status syn_port_dma_start(uint8_t channel,
                               const volatile void *src,
                               volatile void *dst,
                               size_t count);

SYN_Status syn_port_dma_stop(uint8_t channel);

bool syn_port_dma_busy(uint8_t channel);

size_t syn_port_dma_remaining(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_DMA */

#endif /* SYN_PORT_DMA_H */
```


