

# File syn\_heartbeat.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_heartbeat.h**](syn__heartbeat_8h.md)

[Go to the documentation of this file](syn__heartbeat_8h.md)


```C++

#ifndef SYN_HEARTBEAT_H
#define SYN_HEARTBEAT_H

#include "../common/syn_defs.h"
#include "syn_router.h"
#include "../system/syn_errlog.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Error codes ────────────────────────────────────────────────────────── */

#define SYN_HB_ERR_PEER_LOST   0x0500  
#define SYN_HB_ERR_PEER_FOUND  0x0501  
/* ── Peer entry ─────────────────────────────────────────────────────────── */

typedef struct {
    uint8_t   node_id;      
    uint32_t  last_seen;    
    bool      alive;        
    bool      used;         
} SYN_HB_Peer;

/* ── Callbacks ──────────────────────────────────────────────────────────── */

typedef void (*SYN_HB_Callback)(uint8_t node_id, void *ctx);

/* ── Heartbeat instance ─────────────────────────────────────────────────── */

typedef struct {
    SYN_Router    *router;          
    SYN_HB_Peer  *peers;           
    uint8_t         peer_capacity;   
    uint8_t         peer_count;      
    uint32_t        interval_ms;     
    uint32_t        timeout_ms;      
    uint32_t        last_tx_tick;    
    SYN_HB_Callback on_peer_lost;    
    SYN_HB_Callback on_peer_found;   
    void            *cb_ctx;         
    SYN_ErrLog    *errlog;          
} SYN_Heartbeat;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_heartbeat_init(SYN_Heartbeat *hb, SYN_Router *router,
                           SYN_HB_Peer *peers, uint8_t peer_cap,
                           uint32_t interval_ms, uint32_t timeout_ms);

bool syn_heartbeat_add_peer(SYN_Heartbeat *hb, uint8_t node_id);

void syn_heartbeat_update(SYN_Heartbeat *hb);

void syn_heartbeat_on_peer_lost(SYN_Heartbeat *hb,
                                   SYN_HB_Callback cb, void *ctx);

void syn_heartbeat_on_peer_found(SYN_Heartbeat *hb,
                                    SYN_HB_Callback cb, void *ctx);

bool syn_heartbeat_peer_alive(const SYN_Heartbeat *hb, uint8_t node_id);

void syn_heartbeat_set_errlog(SYN_Heartbeat *hb, SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HEARTBEAT_H */
```


