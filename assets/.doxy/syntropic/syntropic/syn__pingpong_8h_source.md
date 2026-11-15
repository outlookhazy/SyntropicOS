

# File syn\_pingpong.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pingpong.h**](syn__pingpong_8h.md)

[Go to the documentation of this file](syn__pingpong_8h.md)


```C++


#ifndef SYN_PINGPONG_H
#define SYN_PINGPONG_H

#include "../common/syn_barrier.h"
#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buf[2];            
    size_t size;                
    volatile size_t active_idx; 
    volatile size_t data_ready; 
    volatile size_t overruns;   
} SYN_PingPong;

static inline void syn_pingpong_init(SYN_PingPong *pp, uint8_t *buf_a, uint8_t *buf_b, size_t size)
{
    pp->buf[0] = buf_a;
    pp->buf[1] = buf_b;
    pp->size = size;
    pp->active_idx = 0U;
    pp->data_ready = 0U;
    pp->overruns = 0U;
}

static inline uint8_t *syn_pingpong_active(const SYN_PingPong *pp)
{
    return pp->buf[SYN_LOAD_ACQUIRE(&pp->active_idx)];
}

static inline uint8_t *syn_pingpong_ready_buf(const SYN_PingPong *pp)
{
    return pp->buf[1U - SYN_LOAD_ACQUIRE(&pp->active_idx)];
}

static inline bool syn_pingpong_ready(const SYN_PingPong *pp)
{
    return SYN_LOAD_ACQUIRE(&pp->data_ready) != 0U;
}

static inline bool syn_pingpong_swap(SYN_PingPong *pp)
{
    if (SYN_LOAD_ACQUIRE(&pp->data_ready) != 0U) {
        size_t drops = SYN_LOAD_ACQUIRE(&pp->overruns);
        SYN_STORE_RELEASE(&pp->overruns, drops + 1U);
        return false;
    }
    size_t next_idx = 1U - SYN_LOAD_ACQUIRE(&pp->active_idx);
    SYN_STORE_RELEASE(&pp->active_idx, next_idx);
    SYN_STORE_RELEASE(&pp->data_ready, 1U);
    return true;
}

static inline void syn_pingpong_consume(SYN_PingPong *pp)
{
    SYN_STORE_RELEASE(&pp->data_ready, 0U);
}

static inline uint32_t syn_pingpong_overruns(const SYN_PingPong *pp)
{
    return (uint32_t)SYN_LOAD_ACQUIRE(&pp->overruns);
}

static inline size_t syn_pingpong_size(const SYN_PingPong *pp)
{
    return pp->size;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PINGPONG_H */
```


