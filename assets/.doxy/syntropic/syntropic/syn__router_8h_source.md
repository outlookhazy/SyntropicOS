

# File syn\_router.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_router.h**](syn__router_8h.md)

[Go to the documentation of this file](syn__router_8h.md)


```C++

#ifndef SYN_ROUTER_H
#define SYN_ROUTER_H

#include "../common/syn_defs.h"
#include "syn_transport.h"
#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Packet format ──────────────────────────────────────────────────────── */

#ifndef SYN_ROUTER_MAX_PAYLOAD
#define SYN_ROUTER_MAX_PAYLOAD  64
#endif

#define SYN_ROUTER_HEADER_SIZE  6

typedef struct {
    uint8_t  src;           
    uint8_t  dst;           
    uint8_t  type;          
    uint8_t  seq;           
    uint8_t  flags;         
    uint8_t  len;           
    uint8_t  payload[SYN_ROUTER_MAX_PAYLOAD]; 
} SYN_Packet;

/* Packet flags */
#define SYN_PKT_FLAG_ACK_REQ  0x01  
#define SYN_PKT_FLAG_IS_ACK   0x02  
/* ── Built-in message types ─────────────────────────────────────────────── */

#define SYN_MSG_HEARTBEAT  0x00  
#define SYN_MSG_ACK        0x01  
#define SYN_MSG_DISCOVER   0x02  
/* ── Handler registration ───────────────────────────────────────────────── */

typedef void (*SYN_RouterHandlerFn)(const SYN_Packet *pkt, void *ctx);

typedef struct {
    uint8_t              type;     
    SYN_RouterHandlerFn handler;  
    void                *ctx;      
} SYN_RouterHandler;

/* ── Pending ACK tracking ───────────────────────────────────────────────── */

#ifndef SYN_ROUTER_MAX_PENDING
#define SYN_ROUTER_MAX_PENDING  4
#endif

typedef struct {
    uint8_t   dst;         
    uint8_t   seq;         
    uint8_t   type;        
    uint8_t   retries;     
    uint32_t  sent_tick;   
    uint8_t   payload[SYN_ROUTER_MAX_PAYLOAD]; 
    uint8_t   len;         
    bool      active;      
} SYN_PendingAck;

/* ── Router instance ────────────────────────────────────────────────────── */

typedef struct {
    uint8_t            node_id;        
    SYN_Transport    *transport;      
    /* Handler table */
    SYN_RouterHandler *handlers;      
    uint8_t             handler_count; 
    uint8_t             handler_cap;   
    /* Sequence counter */
    uint8_t             tx_seq;        
    /* Reliable delivery */
    SYN_PendingAck    *pending;       
    uint8_t             pending_cap;   
    uint16_t            ack_timeout_ms; 
    uint8_t             max_retries;   
    /* Stats */
    uint32_t            tx_count;      
    uint32_t            rx_count;      
    uint32_t            drop_count;    
} SYN_Router;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_router_init(SYN_Router *r, uint8_t node_id,
                       SYN_Transport *transport,
                       SYN_RouterHandler *handlers, uint8_t handler_cap);

void syn_router_enable_ack(SYN_Router *r, SYN_PendingAck *pending,
                              uint8_t cap, uint16_t timeout_ms,
                              uint8_t max_retries);

bool syn_router_register(SYN_Router *r, uint8_t type,
                            SYN_RouterHandlerFn handler, void *ctx);

bool syn_router_send(SYN_Router *r, uint8_t dst, uint8_t type,
                        const uint8_t *data, uint8_t len, bool reliable);

void syn_router_poll(SYN_Router *r);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ROUTER_H */
```


