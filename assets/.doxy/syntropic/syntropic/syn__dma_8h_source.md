

# File syn\_dma.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dma.h**](syn__dma_8h.md)

[Go to the documentation of this file](syn__dma_8h.md)


```C++

#ifndef SYN_DMA_H
#define SYN_DMA_H

#include "../common/syn_defs.h"
#include "../port/syn_port_dma.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_DMA_STATE_IDLE = 0,    
    SYN_DMA_STATE_BUSY = 1,    
    SYN_DMA_STATE_COMPLETE = 2,
    SYN_DMA_STATE_ERROR = 3    
} SYN_DMA_State;

typedef enum {
    SYN_DMA_EVENT_HALF_COMPLETE = 0x01, 
    SYN_DMA_EVENT_COMPLETE      = 0x02, 
    SYN_DMA_EVENT_ERROR         = 0x04  
} SYN_DMA_Event;

struct SYN_DMA;

typedef void (*SYN_DMA_Callback)(struct SYN_DMA *dma, SYN_DMA_Event event, void *user_ctx);

typedef struct {
    uint8_t           channel_id;   
    SYN_DMA_Direction dir;          
    SYN_DMA_Size      data_size;    
    bool              src_inc;      
    bool              dst_inc;      
    SYN_DMA_Callback  callback;     
    void             *user_ctx;     
} SYN_DMA_Config;

typedef struct SYN_DMA {
    SYN_DMA_Config    cfg;          
    SYN_DMA_State     state;        
    const void       *current_src;  
    void             *current_dst;  
    size_t            current_len;  
    uint32_t          transfers_cnt;
    uint32_t          errors_cnt;   
} SYN_DMA;

SYN_Status syn_dma_init(SYN_DMA *dma, const SYN_DMA_Config *cfg);

SYN_Status syn_dma_start(SYN_DMA *dma, const void *src, void *dst, size_t count);

SYN_Status syn_dma_stop(SYN_DMA *dma);

SYN_DMA_State syn_dma_get_state(const SYN_DMA *dma);

void syn_dma_isr_handler(SYN_DMA *dma, SYN_DMA_Event event);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DMA_H */
```


