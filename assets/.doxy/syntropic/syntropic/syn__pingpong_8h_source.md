

# File syn\_pingpong.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pingpong.h**](syn__pingpong_8h.md)

[Go to the documentation of this file](syn__pingpong_8h.md)


```C++

#ifndef SYN_PINGPONG_H
#define SYN_PINGPONG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t  *buf[2];       
    size_t    size;          
    uint8_t   active_idx;   
    volatile bool data_ready; 
} SYN_PingPong;

static inline void syn_pingpong_init(SYN_PingPong *pp,
                                      uint8_t *buf_a,
                                      uint8_t *buf_b,
                                      size_t size)
{
    pp->buf[0]     = buf_a;
    pp->buf[1]     = buf_b;
    pp->size       = size;
    pp->active_idx = 0;
    pp->data_ready = false;
}

static inline uint8_t *syn_pingpong_active(const SYN_PingPong *pp)
{
    return pp->buf[pp->active_idx];
}

static inline uint8_t *syn_pingpong_ready_buf(const SYN_PingPong *pp)
{
    return pp->buf[1 - pp->active_idx];
}

static inline bool syn_pingpong_ready(const SYN_PingPong *pp)
{
    return pp->data_ready;
}

static inline void syn_pingpong_swap(SYN_PingPong *pp)
{
    pp->active_idx = (uint8_t)(1 - pp->active_idx);
    pp->data_ready = true;
}

static inline void syn_pingpong_consume(SYN_PingPong *pp)
{
    pp->data_ready = false;
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


