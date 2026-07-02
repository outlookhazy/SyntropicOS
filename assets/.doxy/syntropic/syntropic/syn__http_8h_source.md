

# File syn\_http.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_http.h**](syn__http_8h.md)

[Go to the documentation of this file](syn__http_8h.md)


```C++

#ifndef SYN_HTTP_H
#define SYN_HTTP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Response info ──────────────────────────────────────────────────────── */

typedef struct {
    int          status_code;     
    uint32_t     content_length;  
    bool         chunked;         
    bool         connection_close; 
    char         location[128];   
} SYN_HttpResponse;

/* ── Request headers (optional) ─────────────────────────────────────────── */

typedef struct {
    const char *name;    
    const char *value;   
} SYN_HttpHeader;

/* ── Body callback ──────────────────────────────────────────────────────── */

typedef bool (*SYN_HttpBodyCallback)(const uint8_t *data, size_t len,
                                     void *ctx);

/* ── Cooperative Task API ────────────────────────────────────────────────── */

typedef enum {
    SYN_HTTP_STATE_IDLE,             
    SYN_HTTP_STATE_CONNECTING,       
    SYN_HTTP_STATE_SENDING_REQUEST,  
    SYN_HTTP_STATE_READING_HEADERS,  
    SYN_HTTP_STATE_READING_BODY,     
    SYN_HTTP_STATE_DONE,             
    SYN_HTTP_STATE_ERROR             
} SYN_HttpState;

typedef struct {
    SYN_HttpState state;             
    SYN_Socket    sock;              
    const char   *host;              
    uint16_t      port;              
    const char   *path;              
    const char   *method;            
    const char   *content_type;      
    const uint8_t *body;             
    size_t        body_len;          
    const SYN_HttpHeader *headers;   
    uint8_t       header_count;      
    SYN_HttpResponse resp;           
    SYN_HttpBodyCallback body_cb;    
    void         *cb_ctx;            
    uint8_t      *work_buf;          
    size_t        work_buf_size;     
    /* Internal tracking */
    int           hops;              
    char          cur_host[64];      
    char          cur_path[128];     
    uint16_t      cur_port;          
    size_t        buf_used;          
    size_t        buf_pos;           
    size_t        body_start;        
    /* Header reading */
    size_t        line_len;          
    uint32_t      header_timeout_ms; 
    /* Body reading */
    uint32_t      body_remaining;    
    bool          known_length;      
    uint32_t      body_timeout_ms;   
    /* Chunked body reading */
    uint32_t      chunk_remaining;   
    uint8_t       chunk_state;       
    char          chunk_line[32];    
    size_t        chunk_line_pos;    
    SYN_Status    status;            
} SYN_HttpClient;

SYN_Status syn_http_client_init(SYN_HttpClient *client,
                                const char *method,
                                const char *host, uint16_t port,
                                const char *path,
                                const char *content_type,
                                const uint8_t *body, size_t body_len,
                                const SYN_HttpHeader *headers, uint8_t header_count,
                                SYN_HttpBodyCallback body_cb, void *cb_ctx,
                                uint8_t *work_buf, size_t work_buf_size);

SYN_PT_Status syn_http_client_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HTTP_H */
```


