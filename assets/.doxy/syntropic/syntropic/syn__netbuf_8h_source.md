

# File syn\_netbuf.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netbuf.h**](syn__netbuf_8h.md)

[Go to the documentation of this file](syn__netbuf_8h.md)


```C++

#ifndef SYN_NETBUF_H
#define SYN_NETBUF_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_NETBUF_HEADROOM_DEFAULT \
    32U 
typedef struct SYN_NetBuf {
    uint8_t *head;                
    uint8_t *data;                
    uint8_t *tail;                
    uint8_t *end;                 
    struct SYN_NetBuf *next_free; 
    volatile uint16_t ref_count;  
    uint16_t flags;               
    void *pool;                   
} SYN_NetBuf;

typedef struct {
    SYN_NetBuf *bufs;      
    uint8_t *raw_mem;      
    SYN_NetBuf *free_list; 
    size_t buf_count;      
    size_t buf_size;       
    size_t free_count;     
} SYN_NetBufPool;

SYN_Status syn_netbuf_pool_init(SYN_NetBufPool *pool, SYN_NetBuf *bufs, uint8_t *raw_mem,
                                size_t buf_count, size_t buf_size);

SYN_NetBuf *syn_netbuf_alloc(SYN_NetBufPool *pool, size_t headroom);

void syn_netbuf_ref(SYN_NetBuf *buf);

void syn_netbuf_free(SYN_NetBufPool *pool, SYN_NetBuf *buf);

uint8_t *syn_netbuf_push(SYN_NetBuf *buf, size_t len);

uint8_t *syn_netbuf_pull(SYN_NetBuf *buf, size_t len);

uint8_t *syn_netbuf_put(SYN_NetBuf *buf, size_t len);

size_t syn_netbuf_len(const SYN_NetBuf *buf);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NETBUF_H */
```


