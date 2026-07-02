

# File syn\_can.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_can.h**](syn__can_8h.md)

[Go to the documentation of this file](syn__can_8h.md)


```C++

#ifndef SYN_CAN_H
#define SYN_CAN_H

#include "../common/syn_defs.h"
#include "../port/syn_port_can.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── CAN frame ──────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t id;            
    uint8_t  data[8];       
    uint8_t  dlc;           
    bool     extended;      
    bool     rtr;           
} SYN_CAN_Frame;

/* ── Callback ───────────────────────────────────────────────────────────── */

typedef void (*SYN_CAN_Callback)(const SYN_CAN_Frame *frame, void *ctx);

/* ── CAN instance ───────────────────────────────────────────────────────── */

typedef struct {
    uint8_t           port;        
    uint32_t          bitrate;     
    SYN_CAN_Callback on_rx;       
    void             *on_rx_ctx;   
    uint32_t          tx_count;    
    uint32_t          rx_count;    
    uint32_t          err_count;   
} SYN_CAN;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_can_init(SYN_CAN *can, uint8_t port, uint32_t bitrate);

bool syn_can_send(SYN_CAN *can, const SYN_CAN_Frame *frame);

void syn_can_poll(SYN_CAN *can);

void syn_can_on_receive(SYN_CAN *can, SYN_CAN_Callback cb, void *ctx);

void syn_can_set_filter(const SYN_CAN *can, uint32_t id, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CAN_H */
```


