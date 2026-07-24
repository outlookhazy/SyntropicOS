

# File syn\_wg.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.h**](syn__wg_8h.md)

[Go to the documentation of this file](syn__wg_8h.md)


```C++

#ifndef SYN_WG_H
#define SYN_WG_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "syn_sntp.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

#ifndef SYN_WG_MTU
#define SYN_WG_MTU  1420
#endif

#define SYN_WG_TRANSPORT_OVERHEAD  32

#define SYN_WG_REKEY_AFTER_TIME       120   
#define SYN_WG_REJECT_AFTER_TIME      180   
#define SYN_WG_REKEY_TIMEOUT            5   
#define SYN_WG_KEEPALIVE_TIMEOUT       10   
#define SYN_WG_MSG_INITIATION   1  
#define SYN_WG_MSG_RESPONSE     2  
#define SYN_WG_MSG_COOKIE       3  
#define SYN_WG_MSG_TRANSPORT    4  
#define SYN_WG_INITIATION_SIZE  148 
#define SYN_WG_RESPONSE_SIZE     92 
/* ── States ─────────────────────────────────────────────────────────────── */

typedef enum {
    SYN_WG_DISCONNECTED,     
    SYN_WG_HANDSHAKE_INIT,   
    SYN_WG_ESTABLISHED,      
} SYN_WgState;

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    uint8_t      private_key[32];       
    uint8_t      peer_public_key[32];   
    uint8_t      preshared_key[32];     
    SYN_SockAddr endpoint;              
    uint16_t     keepalive_interval_s;  
} SYN_WgConfig;

/* ── Session keys ───────────────────────────────────────────────────────── */

typedef struct {
    uint8_t  send_key[32];    
    uint8_t  recv_key[32];    
    uint64_t send_counter;    
    uint64_t recv_counter;    
    uint32_t recv_bitmap;     
    uint32_t sender_index;    
    uint32_t receiver_index;  
    uint32_t established_ms;  
} SYN_WgSession;

/* ── Client context ─────────────────────────────────────────────────────── */

typedef struct {
    SYN_WgConfig   config;          
    SYN_WgState    state;           
    SYN_Socket     udp_sock;        
    /* Time source */
    SYN_SNTP      *sntp;            
    /* Derived keys (computed once at init from config) */
    uint8_t        public_key[32];  
    /* Active session */
    SYN_WgSession  session;          
    /* Handshake state (scratch — only valid during handshake) */
    uint8_t        hs_ephemeral_priv[32]; 
    uint8_t        hs_chaining_key[32];   
    uint8_t        hs_hash[32];           
    /* Timers */
    uint32_t       last_sent_ms;     
    uint32_t       last_recv_ms;     
    uint32_t       last_handshake_ms;
    /* Caller-owned I/O buffers */
    uint8_t       *rx_buf;           
    size_t         rx_buf_size;      
    uint8_t       *tx_buf;           
    size_t         tx_buf_size;      
    void (*on_recv)(const uint8_t *ip_packet, size_t len, void *ctx);
    void           *user_ctx;        
} SYN_WG;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_wg_init(SYN_WG *wg, const SYN_WgConfig *config,
                 SYN_SNTP *sntp,
                 uint8_t *rx_buf, size_t rx_buf_size,
                 uint8_t *tx_buf, size_t tx_buf_size);

SYN_Status syn_wg_send(SYN_WG *wg, const uint8_t *ip_packet, size_t len);

SYN_PT_Status syn_wg_task(SYN_PT *pt, SYN_Task *task);

static inline bool syn_wg_is_established(const SYN_WG *wg)
{
    return wg->state == SYN_WG_ESTABLISHED;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_WG */

#endif /* SYN_WG_H */
```


