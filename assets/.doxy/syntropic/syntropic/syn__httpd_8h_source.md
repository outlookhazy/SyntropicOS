

# File syn\_httpd.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.h**](syn__httpd_8h.md)

[Go to the documentation of this file](syn__httpd_8h.md)


```C++

#ifndef SYN_HTTPD_H
#define SYN_HTTPD_H

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

/* ── HTTP methods ──────────────────────────────────────────────────────── */

typedef enum {
    SYN_HTTP_GET    = 0,             
    SYN_HTTP_POST   = 1,             
    SYN_HTTP_PUT    = 2,             
    SYN_HTTP_DELETE = 3,             
} SYN_HttpMethod;

/* ── Request (parsed, presented to handler) ────────────────────────────── */

typedef struct {
    SYN_HttpMethod  method;          
    const char     *path;            
    const char     *query;           
    uint32_t        content_length;  
    const char     *content_type;    
    const char     *headers;         
    SYN_Socket      client_sock;     
    size_t          body_consumed;   
    size_t          body_buffered_offset; 
    size_t          body_buffered_len;    
} SYN_HttpdRequest;

/* ── Response writer ───────────────────────────────────────────────────── */

typedef struct {
    SYN_Socket  sock;               
    uint8_t    *buf;                 
    size_t      buf_size;            
    bool        headers_sent;        
    bool        upgraded;            
} SYN_HttpdResponse;

/* ── Route handler ─────────────────────────────────────────────────────── */

typedef void (*SYN_HttpdHandler)(const SYN_HttpdRequest *req,
                                  SYN_HttpdResponse *resp,
                                  void *ctx);

/* ── Route entry ───────────────────────────────────────────────────────── */

typedef struct {
    SYN_HttpMethod    method;        
    const char       *path;          
    SYN_HttpdHandler  handler;       
    void             *ctx;           
} SYN_HttpdRoute;

/* ── Server instance ───────────────────────────────────────────────────── */

typedef struct {
    const SYN_HttpdRoute *routes;    
    size_t                route_count; 
    uint8_t              *work_buf;  
    size_t                work_buf_size; 
    uint16_t              port;      
    SYN_Socket            listener;  
    bool                  running;   
} SYN_Httpd;

/* ── Server API ────────────────────────────────────────────────────────── */

SYN_Status syn_httpd_init(SYN_Httpd *srv, uint16_t port,
                           const SYN_HttpdRoute *routes, size_t route_count,
                           uint8_t *work_buf, size_t work_buf_size);

SYN_PT_Status syn_httpd_task(SYN_PT *pt, SYN_Task *task);

SYN_Status syn_httpd_step(SYN_Httpd *srv);

void syn_httpd_stop(SYN_Httpd *srv);

/* ── Response helpers (used from handlers) ─────────────────────────────── */

void syn_httpd_status(const SYN_HttpdResponse *resp, int code, const char *reason);

void syn_httpd_header(const SYN_HttpdResponse *resp,
                       const char *name, const char *value);

void syn_httpd_body(SYN_HttpdResponse *resp, const void *data, size_t len);

void syn_httpd_body_str(SYN_HttpdResponse *resp, const char *str);

int syn_httpd_read_body(const SYN_HttpdRequest *req,
                         const SYN_HttpdResponse *resp,
                         void *buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HTTPD_H */
```


