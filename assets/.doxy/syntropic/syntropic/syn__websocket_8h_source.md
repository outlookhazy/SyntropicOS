

# File syn\_websocket.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_websocket.h**](syn__websocket_8h.md)

[Go to the documentation of this file](syn__websocket_8h.md)


```C++


#ifndef SYN_WEBSOCKET_H
#define SYN_WEBSOCKET_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "syn_httpd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_WS_PENDING_RX_SIZE
#define SYN_WS_PENDING_RX_SIZE 64U 
#endif

typedef enum {
    SYN_WS_STATE_CLOSED,    
    SYN_WS_STATE_CONNECTED, 
} SYN_WebsocketState;

typedef struct {
    SYN_Socket sock;          
    SYN_WebsocketState state; 
    uint8_t rx_buf[128];      
    uint8_t rx_state;         
    uint32_t payload_len;     
    uint32_t bytes_read;      
    uint8_t mask_key[4];      
    bool masked;              
    bool is_server;           
    uint8_t opcode;           
    uint8_t pending_rx[SYN_WS_PENDING_RX_SIZE]; 
    uint16_t pending_rx_len;                    
    uint16_t pending_rx_pos;                    

    void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx);
    void *ctx; 
} SYN_WebsocketSession;

SYN_Status syn_websocket_upgrade(
    const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, SYN_WebsocketSession *ws,
    void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx), void *ctx);

SYN_Status syn_websocket_send(SYN_WebsocketSession *ws, uint8_t opcode, const void *data,
                              size_t len);

SYN_PT_Status syn_websocket_task(SYN_PT *pt, SYN_Task *task);

void syn_websocket_close(SYN_WebsocketSession *ws);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WEBSOCKET_H */
```


