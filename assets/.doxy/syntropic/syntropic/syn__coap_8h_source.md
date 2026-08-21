

# File syn\_coap.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.h**](syn__coap_8h.md)

[Go to the documentation of this file](syn__coap_8h.md)


```C++

#ifndef SYN_COAP_H
#define SYN_COAP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "../util/syn_backoff.h"
#include "syn_dtls.h"
#include "syn_transport.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COAP_VERSION 1 
#define COAP_TYPE_CON 0 
#define COAP_TYPE_NON 1 
#define COAP_TYPE_ACK 2 
#define COAP_TYPE_RST 3 
#define COAP_CODE_GET 1    
#define COAP_CODE_POST 2   
#define COAP_CODE_PUT 3    
#define COAP_CODE_DELETE 4 
#define COAP_RESP_CREATED 65    
#define COAP_RESP_DELETED 66    
#define COAP_RESP_VALID 67      
#define COAP_RESP_CHANGED 68    
#define COAP_RESP_CONTENT 69    
#define COAP_RESP_BAD_REQ 128   
#define COAP_RESP_UNAUTH 129    
#define COAP_RESP_BAD_OPT 130   
#define COAP_RESP_FORBIDDEN 131 
#define COAP_RESP_NOT_FOUND 132 
#define COAP_RESP_METHOD_NA 133 
#define COAP_RESP_NOT_ACC 134   
#define COAP_RESP_PRECOND_F 140 
#define COAP_RESP_REQ_TOO_L 141 
#define COAP_RESP_UNSUP_MED 143 
#define COAP_RESP_INTERNAL 160  
#define COAP_RESP_NOT_IMPL 161  
#define COAP_RESP_BAD_GATE 162  
#define COAP_RESP_SVC_UNAV 163  
#define COAP_RESP_GATE_TO 164   
#define COAP_RESP_PROXY_NA 165  
#define COAP_OPT_URI_HOST 3        
#define COAP_OPT_ETAG 4            
#define COAP_OPT_OBSERVE 6         
#define COAP_OPT_URI_PORT 7        
#define COAP_OPT_LOCATION_PATH 8   
#define COAP_OPT_URI_PATH 11       
#define COAP_OPT_CONTENT_FORMAT 12 
#define COAP_OPT_MAX_AGE 14        
#define COAP_OPT_URI_QUERY 15      
#define COAP_OPT_ACCEPT 17         
#define COAP_OPT_LOCATION_QUERY 20 
#define COAP_OPT_BLOCK2 23         
#define COAP_OPT_BLOCK1 27         
#define COAP_OPT_PROXY_URI 35      
#define COAP_OPT_PROXY_SCHEME 39   
#define COAP_OPT_SIZE1 60          
typedef struct {
    uint32_t num; 
    bool more;    
    uint8_t szx;  
} SYN_CoapBlock;

typedef struct {
    uint16_t num;       
    const uint8_t *val; 
    size_t len;         
} SYN_CoapOption;

size_t syn_coap_encode_block_opt(const SYN_CoapBlock *block, uint8_t buf[3]);

bool syn_coap_decode_block_opt(const uint8_t *opt_val, size_t opt_len, SYN_CoapBlock *block);

typedef struct {
    uint8_t type;           
    uint8_t code;           
    uint16_t msg_id;        
    uint8_t token_len;      
    uint8_t token[8];       
    const uint8_t *payload; 
    size_t payload_len;     
} SYN_CoapMsg;

/* Parser & Builder API */

SYN_Status syn_coap_parse(SYN_CoapMsg *msg, SYN_CoapOption *options, size_t max_options,
                          size_t *option_count, const uint8_t *buf, size_t buf_len);

size_t syn_coap_serialize(const SYN_CoapMsg *msg, const SYN_CoapOption *options,
                          size_t option_count, uint8_t *buf, size_t max_buf_len);

typedef struct {
    /* ── Input (caller fills before launching task) ── */
    SYN_SockAddr server_addr;          
    const SYN_CoapMsg *req_msg;        
    const SYN_CoapOption *req_options; 
    size_t req_option_count;           
    /* ── Output (filled by task on completion) ── */
    SYN_CoapMsg resp_msg;           
    SYN_CoapOption resp_options[8]; 
    size_t resp_option_count;       
    uint8_t resp_buf[256];          
    size_t resp_len;                
    /* ── Execution state (must survive across PT_YIELD) ── */
    SYN_Socket sock;     
    uint8_t tx_buf[256]; 
    size_t tx_len;       
    uint32_t start_ms;   
    SYN_Backoff backoff; 
    SYN_Status status;   
} SYN_CoapRequest;

SYN_PT_Status syn_coap_request_task(SYN_PT *pt, SYN_Task *task);

void syn_coap_request_init(SYN_CoapRequest *r, const SYN_SockAddr *server_addr,
                           const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries);

/* ── CoAP over Generic Transport (SYN_Transport) ────────────────────────── */

typedef struct {
    /* ── Input ── */
    SYN_Transport *transport;          
    const SYN_CoapMsg *req_msg;        
    const SYN_CoapOption *req_options; 
    size_t req_option_count;           
    /* ── Output ── */
    SYN_CoapMsg resp_msg;           
    SYN_CoapOption resp_options[8]; 
    size_t resp_option_count;       
    uint8_t resp_buf[256];          
    size_t resp_len;                
    /* ── Execution state ── */
    uint8_t tx_buf[256]; 
    size_t tx_len;       
    uint32_t start_ms;   
    SYN_Backoff backoff; 
    SYN_Status status;   
} SYN_CoapTransportRequest;

void syn_coap_transport_request_init(SYN_CoapTransportRequest *r, SYN_Transport *transport,
                                     const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries);

SYN_PT_Status syn_coap_transport_request_task(SYN_PT *pt, SYN_Task *task);

SYN_Status syn_coap_transport_send_request(SYN_Transport *transport, const SYN_CoapMsg *req,
                                           const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                           SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                           size_t max_resp_opts, size_t *resp_opt_cnt,
                                           uint8_t *resp_buf, size_t resp_buf_sz);

/* ── CoAP over DTLS 1.3 (coaps://) ───────────────────────────────────────── */

typedef struct {
    SYN_DTLS_Context dtls;        
    SYN_Transport dtls_transport; 
} SYN_CoapsClient;

bool syn_coaps_client_init(SYN_CoapsClient *client, const SYN_DTLS_Config *config,
                           SYN_Transport *underlying_transport, uint8_t *rx_buf, size_t rx_buf_size,
                           uint8_t *tx_buf, size_t tx_buf_size);

bool syn_coaps_client_handshake(SYN_CoapsClient *client);

SYN_Status syn_coaps_client_send_request(SYN_CoapsClient *client, const SYN_CoapMsg *req,
                                         const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                         SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                         size_t max_resp_opts, size_t *resp_opt_cnt,
                                         uint8_t *resp_buf, size_t resp_buf_sz);

#ifdef __cplusplus
}
#endif

#endif /* SYN_COAP_H */
```


