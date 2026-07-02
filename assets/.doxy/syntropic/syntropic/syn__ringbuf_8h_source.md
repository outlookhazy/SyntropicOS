

# File syn\_ringbuf.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ringbuf.h**](syn__ringbuf_8h.md)

[Go to the documentation of this file](syn__ringbuf_8h.md)


```C++

#ifndef SYN_RINGBUF_H
#define SYN_RINGBUF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buf;       
    size_t   size;      
    volatile size_t head;  
    volatile size_t tail;  
} SYN_RingBuf;

void syn_ringbuf_init(SYN_RingBuf *rb, uint8_t *buf, size_t size);

void syn_ringbuf_reset(SYN_RingBuf *rb);

bool syn_ringbuf_put(SYN_RingBuf *rb, uint8_t byte);

bool syn_ringbuf_get(SYN_RingBuf *rb, uint8_t *byte);

bool syn_ringbuf_peek(const SYN_RingBuf *rb, uint8_t *byte);

bool syn_ringbuf_full(const SYN_RingBuf *rb);

bool syn_ringbuf_empty(const SYN_RingBuf *rb);

size_t syn_ringbuf_count(const SYN_RingBuf *rb);

size_t syn_ringbuf_free(const SYN_RingBuf *rb);

size_t syn_ringbuf_write(SYN_RingBuf *rb, const uint8_t *data, size_t len);

size_t syn_ringbuf_read(SYN_RingBuf *rb, uint8_t *data, size_t len);

size_t syn_ringbuf_peek_n(const SYN_RingBuf *rb, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RINGBUF_H */
```


