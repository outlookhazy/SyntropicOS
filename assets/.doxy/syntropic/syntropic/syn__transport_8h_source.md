

# File syn\_transport.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport.h**](syn__transport_8h.md)

[Go to the documentation of this file](syn__transport_8h.md)


```C++

#ifndef SYN_TRANSPORT_H
#define SYN_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Transport interface ────────────────────────────────────────────────── */

typedef struct {
    bool (*send)(const uint8_t *data, size_t len, void *ctx);

    bool (*recv)(uint8_t *data, size_t max_len, size_t *out_len, void *ctx);

    void *ctx;  
} SYN_Transport;

/* ── Convenience: send/receive via transport ────────────────────────────── */

static inline bool syn_transport_send(SYN_Transport *t,
                                        const uint8_t *data, size_t len)
{
    if (t == NULL || t->send == NULL) return false;
    return t->send(data, len, t->ctx);
}

static inline bool syn_transport_recv(SYN_Transport *t,
                                        uint8_t *data, size_t max_len,
                                        size_t *out_len)
{
    if (t == NULL || t->recv == NULL) return false;
    return t->recv(data, max_len, out_len, t->ctx);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_H */
```


