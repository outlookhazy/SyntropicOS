

# File syn\_cobs.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cobs.h**](syn__cobs_8h.md)

[Go to the documentation of this file](syn__cobs_8h.md)


```C++

#ifndef SYN_COBS_H
#define SYN_COBS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── One-shot encode / decode ───────────────────────────────────────────── */

size_t syn_cobs_encode(const void *src, size_t src_len, void *dst);

size_t syn_cobs_decode(const void *src, size_t src_len, void *dst);

/* ── Streaming decoder ──────────────────────────────────────────────────── */

struct SYN_COBS_Decoder;

typedef void (*SYN_COBS_PacketCallback)(const uint8_t *data, size_t len,
                                         void *ctx);

typedef struct SYN_COBS_Decoder {
    uint8_t                 *buf;       
    size_t                   buf_size;  
    size_t                   idx;       
    SYN_COBS_PacketCallback callback;  
    void                    *ctx;      
} SYN_COBS_Decoder;

void syn_cobs_decoder_init(SYN_COBS_Decoder *dec,
                            uint8_t *buf, size_t buf_size,
                            SYN_COBS_PacketCallback callback,
                            void *ctx);

void syn_cobs_decoder_feed(SYN_COBS_Decoder *dec, uint8_t byte);

void syn_cobs_decoder_reset(SYN_COBS_Decoder *dec);

#ifdef __cplusplus
}
#endif

#endif /* SYN_COBS_H */
```


