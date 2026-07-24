

# File syn\_slab.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_slab.h**](syn__slab_8h.md)

[Go to the documentation of this file](syn__slab_8h.md)


```C++

#ifndef SYN_SLAB_H
#define SYN_SLAB_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SLAB_MAX_CLASSES 8U

typedef struct {
    size_t   block_size;   
    size_t   total_blocks; 
    size_t   free_count;   
    uint8_t *start_ptr;    
    void    *freelist;     
} SYN_SlabClass;

typedef struct {
    uint8_t      *backing_memory;                
    size_t        total_memory_size;             
    SYN_SlabClass classes[SYN_SLAB_MAX_CLASSES]; 
    size_t        num_classes;                   
} SYN_SlabAllocator;

typedef struct {
    size_t total_bytes;     
    size_t allocated_bytes; 
    size_t total_allocs;    
} SYN_SlabStats;

SYN_Status syn_slab_init(SYN_SlabAllocator *slab,
                         void *backing_memory,
                         size_t total_size,
                         const size_t *block_sizes,
                         const size_t *blocks_per_class,
                         size_t num_classes);

void *syn_slab_alloc(SYN_SlabAllocator *slab, size_t size);

SYN_Status syn_slab_free(SYN_SlabAllocator *slab, void *ptr);

SYN_Status syn_slab_get_stats(const SYN_SlabAllocator *slab, SYN_SlabStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SLAB_H */
```


