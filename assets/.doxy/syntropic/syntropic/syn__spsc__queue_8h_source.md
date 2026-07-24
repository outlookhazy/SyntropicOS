

# File syn\_spsc\_queue.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_spsc\_queue.h**](syn__spsc__queue_8h.md)

[Go to the documentation of this file](syn__spsc__queue_8h.md)


```C++

#ifndef SYN_SPSC_QUEUE_H
#define SYN_SPSC_QUEUE_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buffer;       
    size_t   elem_size;    
    size_t   capacity;     
    volatile size_t head;  
    volatile size_t tail;  
} SYN_SPSC_Queue;

SYN_Status syn_spsc_queue_init(SYN_SPSC_Queue *q, void *elem_buf, size_t elem_size, size_t capacity);

SYN_Status syn_spsc_queue_push(SYN_SPSC_Queue *q, const void *item);

SYN_Status syn_spsc_queue_pop(SYN_SPSC_Queue *q, void *out_item);

bool syn_spsc_queue_is_empty(const SYN_SPSC_Queue *q);

bool syn_spsc_queue_is_full(const SYN_SPSC_Queue *q);

size_t syn_spsc_queue_count(const SYN_SPSC_Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPSC_QUEUE_H */
```


