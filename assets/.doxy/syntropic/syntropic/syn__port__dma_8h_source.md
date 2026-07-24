

# File syn\_port\_dma.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dma.h**](syn__port__dma_8h.md)

[Go to the documentation of this file](syn__port__dma_8h.md)


```C++

#ifndef SYN_PORT_DMA_H
#define SYN_PORT_DMA_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_DMA_DIR_MEM_TO_MEM = 0, 
    SYN_DMA_DIR_MEM_TO_PERIPH = 1, 
    SYN_DMA_DIR_PERIPH_TO_MEM = 2  
} SYN_DMA_Direction;

typedef enum {
    SYN_DMA_SIZE_8BIT  = 1, 
    SYN_DMA_SIZE_16BIT = 2, 
    SYN_DMA_SIZE_32BIT = 4  
} SYN_DMA_Size;

typedef struct {
    uint8_t           channel_id;   
    SYN_DMA_Direction dir;          
    SYN_DMA_Size      data_size;    
    bool              src_inc;      
    bool              dst_inc;      
    const void       *src;          
    void             *dst;          
    size_t            count;        
} SYN_PortDmaTransfer;

SYN_Status syn_port_dma_start(const SYN_PortDmaTransfer *xfer);

SYN_Status syn_port_dma_stop(uint8_t channel_id);

bool syn_port_dma_is_busy(uint8_t channel_id);

void syn_port_cache_clean(const void *addr, size_t len);

void syn_port_cache_invalidate(void *addr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_DMA_H */
```


